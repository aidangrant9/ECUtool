#pragma once

#include "Command.hpp"
#include "Logger.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct ScriptCommand : public Command
{
	explicit ScriptCommand(const std::string &name, const size_t repeatInMilliseconds)
		: Command(name, repeatInMilliseconds)
	{
		type = Type::Script;
	}

	std::filesystem::path mainScript;
	std::filesystem::path globalsDirectory;

	std::string identifier() override { return std::string{ "SCRIPT" }; }

	std::string toJson() override
	{
		json output;
		output["name"] = name;
		output["repeatInterval"] = repeatInMilliseconds;
		output["visible"] = visible;
		output["type"] = identifier();
		return output.dump(4);
	}

	virtual bool run(std::shared_ptr<Connection> connection, std::string arguments, const std::stop_token &st)
	{
		Logger &logger = Logger::instance();

		sol::state lua;

		lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::string, sol::lib::table);

		lua.new_usertype<Logger>("log",
			"info", [this](Logger *e, const std::string msg) {e->addMessage(Message{ msg, this->name });},
			"error", [this](Logger *e, const std::string msg) {e->addErrorMessage(Message{ msg, this->name });}
		);

		lua["log"] = &logger;

		lua_State *L = lua.lua_state();

		struct StopTokenState 
		{
			const std::stop_token &token;
			explicit StopTokenState(const std::stop_token &t) : token(t) {}
		};

		void *stateMemory = lua_newuserdata(L, sizeof(StopTokenState));
		new (stateMemory) StopTokenState(st);
		lua_setfield(L, LUA_REGISTRYINDEX, "STOP_TOKEN_STATE");

		lua_sethook(L, [](lua_State *L, lua_Debug *ar) {
			(void)ar;

			lua_getfield(L, LUA_REGISTRYINDEX, "STOP_TOKEN_STATE");
			if (lua_isuserdata(L, -1)) {
				StopTokenState *state = static_cast<StopTokenState *>(lua_touserdata(L, -1));

				if (state && state->token.stop_requested()) {
					luaL_error(L, "Script execution interrupted");
				}
			}
			lua_pop(L, 1);
			}, LUA_MASKCOUNT, 5);


		connection->bindToLua(lua);

		if (std::filesystem::exists(globalsDirectory))
		{
			for (auto &file : std::filesystem::recursive_directory_iterator(globalsDirectory))
			{
				if (file.is_regular_file() && file.path().extension() == ".lua")
				{
					sol::protected_function_result result = lua.safe_script_file(file.path().string(), sol::script_pass_on_error);
					if (!result.valid())
					{
						sol::error err = result;
						logger.addErrorMessage(Message{ std::string(err.what()), name });
						return false;
					}
				}
			}
		}

		if (std::filesystem::exists(mainScript))
		{
			sol::protected_function_result result = lua.safe_script_file(mainScript.string(), sol::script_pass_on_error);
			if (!result.valid())
			{
				sol::error err = result;
				logger.addErrorMessage(Message{ std::string(err.what()), name });
				return false;
			}
		}

		sol::protected_function entryFunc = lua["entry"];
		if (!entryFunc.valid())
		{
			logger.addErrorMessage(Message{ "No entry() function found", name });
			return false;
		}

		sol::protected_function_result res = entryFunc(arguments);
		if (!res.valid())
		{
			sol::error err = res;
			logger.addErrorMessage(Message{std::string(err.what()), name});
			return false;
		}

		if (res.get_type() == sol::type::boolean) 
		{
			return res.get<bool>(0);
		}

		return true;
	}
};