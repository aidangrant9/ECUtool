#pragma once

#include "../communication/SerialConnection.hpp"
#include "Command.hpp"
#include <optional>
#include <list>
#include <vector>
#include <filesystem>
#include <functional>

class DiagnosticSession
{
public:
	DiagnosticSession() = default;
	~DiagnosticSession() = default;

	void setConnection(std::shared_ptr<SerialConnection> newConnection);
	void setProjectRoot(const std::filesystem::path &path);

	void handleOnDataSent(const DataMessage<uint8_t> &data);
	void handleOnDataRecieved(const DataMessage<uint8_t> &data);
	void handleMessage(const Message &msg);
	void handleStatusChange(const SerialConnection::ConnectionStatus status);



	void setMessageViewCallback(std::function<void()> &cb);
	void setCommandViewCallback(std::function<void()> &cb);
	void addCommand(Command &c);
	void removeCommand(Command &c);
	const std::list<std::unique_ptr<Command>> &getCommands();

private:
	std::list<std::unique_ptr<Command>> definedCommands{};
	std::shared_ptr<SerialConnection>   connection{};
	std::filesystem::path projectRoot{};
	std::function<void()> commandsViewCallback{};
	std::function<void()> messageViewCallback {};
};