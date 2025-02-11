#pragma once

#include "../communication/SerialConnection.hpp"
#include "Command.hpp"
#include "Message.hpp"
#include <optional>
#include <list>
#include <vector>
#include <filesystem>
#include <functional>
#include <mutex>

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

	void notifyCommandsView();
	void notifyMessagesView();

	void setMessageViewCallback(std::function<void()> cb);
	void setCommandViewCallback(std::function<void()> cb);
	
	void addCommand(std::shared_ptr<Command> &c);
	void addMessage(Message &m);
	const std::vector<Message> &getMessages();
	
	void removeCommand(Command &c);
	const std::vector<std::shared_ptr<Command>> &getCommands();

private:
	std::vector<std::shared_ptr<Command>> definedCommands{};
	std::vector<Message> outputMessages{};
	std::mutex messageMutex{};

	std::shared_ptr<SerialConnection>   connection{};
	std::filesystem::path projectRoot{};
	std::function<void()> commandsViewCallback{};
	std::function<void()> messageViewCallback {};
};