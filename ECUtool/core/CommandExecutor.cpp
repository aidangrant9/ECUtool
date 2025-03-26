#include "CommandExecutor.hpp"
#include "ScriptCommand.hpp"
#include "RawCommand.hpp"
#include "sol/sol.hpp"
#include <fstream>
#include "Logger.hpp"

using namespace std::chrono;

CommandExecutor::CommandExecutor(std::shared_ptr<Connection> connection, std::function<void()> commandStatusChangedCb)
	: connection(connection), commandStatusChangedCb(commandStatusChangedCb)
{
	workThread = std::jthread(&CommandExecutor::work, this);
}

CommandExecutor::~CommandExecutor()
{
	workThread.request_stop();
	workThread.join();
}

void CommandExecutor::queueOrUnqueueCommand(std::shared_ptr<Command> c)
{
	std::lock_guard<std::mutex> lock(commandQueueMutex);
	
	// Command already in queue
	if (removeCommandFromQueues(c))
	{
		toggleCommandActive(c, false);
		return;
	}

	// Command not already queued
	toggleCommandActive(c, true);

	if (c->repeatInMilliseconds > 0)
		repeatingCommands.push_back(std::make_pair<>(c, std::chrono::steady_clock::now() - std::chrono::milliseconds(c->repeatInMilliseconds)));
	else
		nonRepeatingCommands.push_back(c);
}

bool CommandExecutor::removeCommandFromQueues(std::shared_ptr<Command> c)
{
	for (auto it = repeatingCommands.begin(); it != repeatingCommands.end(); it++)
	{
		// Command already queued
		if ((*it).first.get() == c.get())
		{
			repeatingCommands.erase(it);
			return true;
		}
	}

	for (auto it = nonRepeatingCommands.begin(); it != nonRepeatingCommands.end(); it++)
	{
		// Command already queued
		if ((*it).get() == c.get())
		{
			nonRepeatingCommands.erase(it);
			return true;
		}
	}

	return false;
}

void CommandExecutor::toggleCommandActive(std::shared_ptr<Command> c, bool active)
{
	c->active = active;
	commandStatusChangedCb();
}

void CommandExecutor::work()
{
	std::stop_token st = workThread.get_stop_token();

	while (true)
	{
		if (st.stop_requested())
		{
			return;
		}

		std::shared_ptr<Command> toRun{};
		{
			std::lock_guard<std::mutex> lock(commandQueueMutex);

			// Check repeating commands first
			auto now = steady_clock::now();
			for (auto it = repeatingCommands.begin(); it != repeatingCommands.end(); ++it)
			{
				auto elapsed = duration_cast<milliseconds>(now - it->second);
				if (elapsed.count() > it->first->repeatInMilliseconds)
				{
					it->second = now;  // Update last run time
					toRun = it->first;
					break;
				}
			}

			// If no repeating command ready, check non-repeating
			if (!toRun && !nonRepeatingCommands.empty())
			{
				toRun = nonRepeatingCommands.front();
				nonRepeatingCommands.pop_front();
			}
		}


		// If we have a command to run
		if (toRun)
		{
			bool successful = toRun->run(connection);
			if (!successful)
			{
				commandQueueMutex.lock();
				removeCommandFromQueues(toRun);
				commandQueueMutex.unlock();
				toggleCommandActive(toRun, false);
			}

			if (toRun->repeatInMilliseconds <= 0)
			{
				toggleCommandActive(toRun, false);
			}
		}


		// Avoid busy looping
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}
