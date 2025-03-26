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

using json = nlohmann::json;

class DiagnosticSession
{
public:
	explicit DiagnosticSession(std::function<void(const Connection::ConnectionStatus status, const std::string message)> statusChangedCb);
	~DiagnosticSession() = default;

	// Connection methods
	void setConnection(std::shared_ptr<Connection> newConnection);
	void connect();
	void disconnect();

	// Project methods
	void openProject(const std::filesystem::path path);
	void saveProject();

	// Command methods
	std::shared_ptr<Command> loadCommandFromJson(std::string input);
	void addCommand(std::shared_ptr<Command> c);
	bool editCommand(int idx, std::shared_ptr<Command> &c);
	bool removeCommand(int idx);
	void queueOrUnqueueCommand(std::shared_ptr<Command> c);
	void initialiseScript(std::shared_ptr<Command> c);
	std::vector<std::shared_ptr<Command>> getCommands();

	// Qt callback
	void setCommandsResetStart(std::function<void()> cb);
	void setCommandsResetEnd(std::function<void()> cb);

private:
	// Internal state
	std::shared_ptr<CommandExecutor> commandExecutor{};
	std::shared_ptr<Connection>   connection{};
	std::vector<std::shared_ptr<Command>> commands{};
	std::filesystem::path projectRoot{};

	// Qt GUI callbacks
	std::function<void()> commandsResetStart{};
	std::function<void()> commandsResetEnd{};
	std::function<void(const Connection::ConnectionStatus status, const std::string message)> statusChanged{};

	Logger &logger = Logger::instance();
};