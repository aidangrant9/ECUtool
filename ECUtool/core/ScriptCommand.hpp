#pragma once

#include "Command.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct ScriptCommand : public Command
{
	explicit ScriptCommand(const std::string &name, const size_t repeatInMilliseconds)
		: Command(name, repeatInMilliseconds)
	{
		type = Type::Script;
	}

	std::string toJson() override
	{
		json output;
		output["name"] = name;
		output["repeatInterval"] = repeatInMilliseconds;
		output["type"] = identifier();
		return output.dump(4);
	}

	std::string identifier() override { return std::string{ "SCRIPT" }; }

};