#pragma once

#include "Command.hpp"
#include <vector>

class RawCommand : public Command
{
public:
	explicit RawCommand(std::string name, std::vector<uint8_t> rawMsg);
	virtual void execute(std::shared_ptr<SerialConnection> connection, std::function<void(const std::string &result)> executionStopped) override;

	std::vector<uint8_t> msg{};
};