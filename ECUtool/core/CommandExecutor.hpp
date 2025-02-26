#pragma once

#include "../communication/Connection.hpp"
#include "DiagnosticSession.hpp"
#include "Command.hpp"
#include <thread>
#include <filesystem>
#include <mutex>
#include <atomic>
#include <chrono>

class DiagnosticSession;

class CommandExecutor
{
public:
	explicit CommandExecutor(DiagnosticSession *session, std::shared_ptr<Connection> connection, std::filesystem::path workpath);
	~CommandExecutor();

	void queueCommand(std::shared_ptr<Command> c);
	void runLua(std::filesystem::path file);
private:
	void work();

	int queueTimeEpsilon = 20;
	std::filesystem::path workpath{};
	std::shared_ptr<Connection> connection{};
	DiagnosticSession *session{};
	std::thread workThread{};
	std::atomic<bool> shouldStop{ false };
	std::mutex commandMutex{};
	std::deque<std::pair<std::shared_ptr<Command>,
		std::chrono::time_point<std::chrono::steady_clock>>> repeatingCommands{};
	std::deque<std::shared_ptr<Command>> regularCommands  {};
};