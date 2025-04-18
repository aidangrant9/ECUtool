#pragma once

#include "../communication/Connection.hpp"
#include "Command.hpp"
#include <thread>
#include <filesystem>
#include <mutex>
#include <atomic>
#include <chrono>
#include <functional>

class CommandExecutor
{
public:
	explicit CommandExecutor(std::shared_ptr<Connection> connection, std::function<void()> commandStatusChangedCb);
	~CommandExecutor();
	void queueOrUnqueueCommand(std::shared_ptr<Command> c, std::string arguments);
private:
	void work();
	bool removeCommandFromQueues(std::shared_ptr<Command> c);
	void toggleCommandActive(std::shared_ptr<Command> c, bool active);

	Logger &logger = Logger::instance();

	std::shared_ptr<Connection> connection{};

	std::jthread workThread{};

	// Commands waiting to be run
	std::deque<std::tuple<std::shared_ptr<Command>, std::chrono::time_point<std::chrono::steady_clock>, std::string>> repeatingCommands{};
	std::deque<std::pair<std::shared_ptr<Command>, std::string>> nonRepeatingCommands{};
	std::mutex commandQueueMutex{};

	// For GUI to know when we change the status of a command
	std::function<void()> commandStatusChangedCb{};
};