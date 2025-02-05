#pragma once

#include "../communication/SerialConnection.hpp"
#include <vector>
#include <string>
#include <cstdint>
#include <functional>

class Command
{
public:
	explicit Command(const std::string name)
		: name(name)
	{}

	virtual ~Command() = 0;

	virtual void execute(std::shared_ptr<SerialConnection> connection, std::function<void(const std::string &result)> executionStopped) = 0;
	
	const std::string &getName()
	{
		return name;
	}
private:
	std::string name;
};