#pragma once

#include "../communication/SerialConnection.hpp"
#include <vector>
#include <string>
#include <cstdint>
#include <functional>

class Command
{
public:
	explicit Command(const std::string name, const std::shared_ptr<SerialConnection> connection)
		: name(name), connection(connection)
	{}

	virtual ~Command() = 0;

	virtual void execute(std::function<void(const std::string &result)> executionStopped) = 0;
	
	const std::string &getName()
	{
		return name;
	}
private:
	std::string name;
	std::shared_ptr<SerialConnection> connection;
};