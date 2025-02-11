#pragma once

#include "Command.hpp"

struct ScriptCommand : public Command
{
	explicit ScriptCommand(const std::string &name, const size_t repeatInMilliseconds, const std::string &scriptFileName)
		: Command(name, repeatInMilliseconds), scriptFileName(scriptFileName)
	{}

	std::string identifier() override { return std::string{ "SCR" }; }

	std::string scriptFileName;
};