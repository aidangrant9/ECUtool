#pragma once

#include "Command.hpp"

struct ScriptCommand : public Command
{
	explicit ScriptCommand(const std::string &name, const std::string &scriptFileName)
};