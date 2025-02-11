#pragma once

#include "Command.hpp"
#include <vector>

struct RawCommand : public Command
{
	explicit RawCommand(const std::string &name, const size_t repeatInMilliseconds, const std::vector<uint8_t> &rawMsg)
		: Command(name, repeatInMilliseconds), msg(rawMsg)
	{}

	std::string identifier() override { return std::string{ "RAW" }; }
	
	~RawCommand() = default;

	std::vector<uint8_t> msg{};
};