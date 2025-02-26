#pragma once

#include "../communication/Connection.hpp"
#include "Command.hpp"
#include "Message.hpp"
#include <optional>
#include <list>
#include <vector>
#include <filesystem>
#include <functional>
#include <mutex>
#include <thread>
#include <nlohmann/json.hpp>
#include "CommandExecutor.hpp"

class CommandExecutor;

using json = nlohmann::json;

class DiagnosticSession
{
public:
	DiagnosticSession() = default;
	~DiagnosticSession() = default;

	void setConnection(std::shared_ptr<Connection> newConnection);
	void connect();
	void disconnect();

	void openProject(const std::filesystem::path &path);
	void saveProject();

	void handleOnDataSent(const DataMessage<uint8_t> &data);
	void handleOnDataRecieved(const DataMessage<uint8_t> &data);
	void handleMessage(const Message &msg);
	void handleStatusChange(const Connection::ConnectionStatus previous, const Connection::ConnectionStatus current);

	// Qt model specific
	void setMessageResetStart(std::function<void()> cb);
	void setCommandsResetStart(std::function<void()> cb);
	void setMessageResetEnd(std::function<void()> cb);
	void setCommandsResetEnd(std::function<void()> cb);

	void setStatusChanged(std::function<void(std::optional<Connection::ConnectionStatus>)> statusChanged);
	
	std::shared_ptr<Command> loadCommandFromJson(std::string input);
	void addCommand(std::shared_ptr<Command> c);
	bool editCommand(int idx, std::shared_ptr<Command> &c);
	bool removeCommand(int idx);
	const std::vector<std::shared_ptr<Command>> &getCommands();
	void queueCommand(std::shared_ptr<Command> c);
	void initialiseScript(std::shared_ptr<Command> c);

	std::string stringFromDataVec(std::vector<uint8_t>);
	std::vector<uint8_t> dataVecFromString(std::string input);
	void addMessage(std::shared_ptr<Message> c);
	const std::vector<std::shared_ptr<Message>> &getMessages();

	// Helper message functions
	void errorMessage(const std::string &msg, const std::string &src);
	void dataRecievedMessage(const DataMessage<uint8_t> &msg, const std::string &src);
	void dataSentMessage(const DataMessage<uint8_t> &msg, const std::string &src);
	void successMessage(const std::string &msg, const std::string &src);
	void infoMessage(const std::string &msg, const std::string &src);

	std::vector<uint8_t> readOrTimeout(int ms);
	void send(std::vector<uint8_t> data);

private:
	std::deque<DataMessage<uint8_t>> incomingQueue{};
	std::mutex incomingMutex{};


	std::shared_ptr<CommandExecutor> commandExecutor{};
	std::vector<std::shared_ptr<Command>> definedCommands{};
	std::vector<std::shared_ptr<Message>> outputMessages{};
	std::mutex messageMutex{};
	std::filesystem::path projectRoot{};

	std::shared_ptr<Connection>   connection{};

	std::function<void()> commandsResetStart{};
	std::function<void()> commandsResetEnd{};
	std::function<void()> messageResetStart {};
	std::function<void()> messageResetEnd{};
	std::function<void(std::optional<Connection::ConnectionStatus>)> statusChanged{};
};