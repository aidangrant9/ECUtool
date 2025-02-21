#include "CommandExecutor.hpp"
#include "ScriptCommand.hpp"
#include "RawCommand.hpp"
#include "../communication/DataMessage.hpp"
#include "sol/sol.hpp"
#include <QDebug>
#include <fstream>


CommandExecutor::CommandExecutor(DiagnosticSession *session, std::shared_ptr<SerialConnection> connection, std::filesystem::path workpath)
	: workpath(workpath)
{
	if (connection.get() == nullptr)
		throw std::runtime_error("CommandExecutor cannot be initialised without a connection");
	else
		this->connection = connection;

	if (session == nullptr)
		throw std::runtime_error("CommandExecutor cannot be initialised without a DiagnosticSession");
	else
		this->session = session;

	workThread = std::jthread(&CommandExecutor::work, this);
}

CommandExecutor::~CommandExecutor()
{
	workThread.request_stop();
	workThread.join();
}

void CommandExecutor::queueCommand(std::shared_ptr<Command> c)
{
	if (c->repeatInMilliseconds > 0)
	{
		repeatingCommands.push_back(std::make_pair<>(c, std::chrono::steady_clock::now()));
	}
	else
	{
		regularCommands.push_front(c);
	}
}

void CommandExecutor::runLua(std::filesystem::path file)
{
	/*
	sol::state lua;

	lua.open_libraries(sol::lib::base, sol::lib::string, sol::lib::table);

	lua.new_usertype<DiagnosticSession>("DiagnosticSession",
		"errorMessage", &DiagnosticSession::errorMessage,
		"infoMessage", &DiagnosticSession::infoMessage,
		"readOrTimeout", &DiagnosticSession::readOrTimeout,
		"send", &DiagnosticSession::send);

	lua["session"] = session;

	std::filesystem::path globalsPath = workpath / "global.lua";
	std::filesystem::path scriptPath = workpath / "scripts" / file.filename();  // Use file.filename() to get just the filename

	sol::load_result globals = lua.load_file(globalsPath.string());
	sol::load_result script = lua.load_file(scriptPath.string());

	//session->infoMessage("Loading globals from: " + globalsPath.string(), "Lua");
	//session->infoMessage("Loading script from: " + scriptPath.string(), "Lua");
	

	if (!globals.valid() || !script.valid())
	{
		session->errorMessage("Failed to load script", file.filename().string());
		return;
	}

	sol::protected_function entryFunc = lua["entry"];

	if (!entryFunc.valid())
	{
		session->errorMessage("No entry() defined in the script", file.filename().string());
		return;
	}

	auto res = entryFunc();



	auto result = lua.safe_script("entry()");

	if (!result.valid())
	{
		sol::error err = result;
		session->errorMessage(err.what(), scriptPath.filename().string());
	}


	return;
	*/

	sol::state lua;

	lua.open_libraries(sol::lib::base, sol::lib::string, sol::lib::table);

	lua.new_usertype<DiagnosticSession>("DiagnosticSession",
		"errorMessage", &DiagnosticSession::errorMessage,
		"infoMessage", &DiagnosticSession::infoMessage,
		"readOrTimeout", &DiagnosticSession::readOrTimeout,
		"send", &DiagnosticSession::send);

	lua["session"] = session;

	std::filesystem::path globalsPath = workpath / "global.lua";
	std::filesystem::path scriptPath = workpath / "scripts" / file.filename();  // Get filename only

	// Load globals.lua
	sol::protected_function_result globalsResult = lua.safe_script_file(globalsPath.string(), sol::script_pass_on_error);
	if (!globalsResult.valid()) {
		sol::error err = globalsResult;
		session->errorMessage("Error loading globals.lua: " + std::string(err.what()), globalsPath.filename().string());
		return;
	}

	// Load script.lua
	sol::protected_function_result scriptResult = lua.safe_script_file(scriptPath.string(), sol::script_pass_on_error);
	if (!scriptResult.valid()) {
		sol::error err = scriptResult;
		session->errorMessage("Error loading script: " + std::string(err.what()), scriptPath.filename().string());
		return;
	}

	// Get entry function safely
	sol::protected_function entryFunc = lua["entry"];
	if (!entryFunc.valid()) {
		session->errorMessage("No entry() function found in script", scriptPath.filename().string());
		return;
	}

	// Execute entry function safely
	sol::protected_function_result res = entryFunc();
	if (!res.valid()) {
		sol::error err = res;
		session->errorMessage("Error executing entry(): " + std::string(err.what()), scriptPath.filename().string());
	}
}

void CommandExecutor::work()
{
	std::stop_token st = workThread.get_stop_token();

	while (true)
	{
		if (st.stop_requested())
		{
			// cleanup
			return;
		}

		// Command to run
		std::shared_ptr<Command> toRun = nullptr;

		commandMutex.lock();
		// Try to get due repeating command
		if (!repeatingCommands.empty())
		{
			for (auto &repeatedCommand : repeatingCommands)
			{
				auto &lastRun = repeatedCommand.second;
				auto differenceInRuntime = std::chrono::steady_clock::now() - lastRun;
				std::chrono::milliseconds msSinceLastRun = std::chrono::duration_cast<std::chrono::milliseconds>(differenceInRuntime);

				if (msSinceLastRun.count() > repeatedCommand.first->repeatInMilliseconds - queueTimeEpsilon)
				{
					toRun = repeatedCommand.first;
					repeatedCommand.second = std::chrono::steady_clock::now();
					break;
				}
			}
		}

		// Get regular command if no repeating command
		if (toRun.get() == nullptr && !regularCommands.empty())
		{
			toRun = regularCommands.back();
			regularCommands.pop_back();
		}
		commandMutex.unlock();


		// If we have a command to run
		if (toRun.get() != nullptr)
		{
			if (auto script = dynamic_cast<ScriptCommand *>(toRun.get()))
			{
				std::filesystem::path scriptPath = workpath / "scripts" / (script->name + ".lua");
				if (std::filesystem::exists(scriptPath))
				{
					runLua(scriptPath);
				}
				else
				{
					session->errorMessage("Couldn't find " + scriptPath.string() + " , no such file exists", "CommandExecutor");
				}
			}
			if (auto raw = dynamic_cast<RawCommand *>(toRun.get()))
			{
				DataMessage<uint8_t> toSend(raw->msg);
				connection->write(toSend);
			}
		}


		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}
