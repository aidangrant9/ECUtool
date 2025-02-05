#pragma once

#include "../communication/SerialConnection.hpp"
#include "Command.hpp"
#include <optional>
#include <list>
#include <vector>

class DiagnosticSession
{
public:
	DiagnosticSession();
	~DiagnosticSession();

	void setConnection(std::shared_ptr<SerialConnection> newConnection);
	void sendRawMessage(std::vector<uint8_t> &msg);



	void addCommand(Command &c)
	{
		definedCommands.push_back(std::make_unique<Command>(c));
	}

	void removeCommand(Command &c)
	{
		const std::string name = c.getName();
		for (auto &n : definedCommands)
		{
			if (name == n->getName())
			{

			}
		}
	}

private:
	std::list<std::unique_ptr<Command>> definedCommands{};
	std::shared_ptr<SerialConnection>     connection {};
}