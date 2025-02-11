#pragma once

#include "Command.hpp"
#include <vector>

struct RawCommand : public Command
{
	explicit RawCommand(const std::string &name, const std::vector<uint8_t> &rawMsg);
	
	~RawCommand() = default;

	std::vector<uint8_t> msg{};
};