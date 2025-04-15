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

void CommandExecutor::queueOrUnqueueCommand(std::shared_ptr<Command> c, std::string arguments)
{
	std::lock_guard<std::mutex> lock(commandQueueMutex);

	if (removeCommandFromQueues(c))
	{
		toggleCommandActive(c, false);
		return;
	}

	toggleCommandActive(c, true);

	if (c->repeatInMilliseconds > 0)
	{
		repeatingCommands.push_back({ c, steady_clock::now() - milliseconds(c->repeatInMilliseconds), arguments });
	}
	else
	{
		nonRepeatingCommands.push_back({ c, arguments });
	}
}

bool CommandExecutor::removeCommandFromQueues(std::shared_ptr<Command> c)
{
	for (auto it = repeatingCommands.begin(); it != repeatingCommands.end(); ++it)
	{
		if (std::get<0>(*it).get() == c.get())
		{
			repeatingCommands.erase(it);
			return true;
		}
	}

	for (auto it = nonRepeatingCommands.begin(); it != nonRepeatingCommands.end(); ++it)
	{
		if (it->first.get() == c.get())
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
        std::string argsToPass{};

        {
            commandQueueMutex.lock();
            auto now = steady_clock::now();

            // Check repeating commands first.
            for (auto it = repeatingCommands.begin(); it != repeatingCommands.end(); ++it)
            {
                auto elapsed = duration_cast<milliseconds>(now - std::get<1>(*it));
                if (elapsed.count() > std::get<0>(*it)->repeatInMilliseconds)
                {
                    // Update last run time and select command to run.
                    std::get<1>(*it) = now;
                    toRun = std::get<0>(*it);
                    argsToPass = std::get<2>(*it);
                    break;
                }
            }

            // If no repeating command is ready, check non-repeating commands.
            if (!toRun && !nonRepeatingCommands.empty())
            {
                toRun = nonRepeatingCommands.front().first;
                argsToPass = nonRepeatingCommands.front().second;
                nonRepeatingCommands.pop_front();
            }
            commandQueueMutex.unlock();
        }

        // If we have a command to run, execute it with the provided arguments.
        if (toRun && connection->getStatus() == Connection::ConnectionStatus::Connected)
        {
            bool successful = toRun->run(connection, argsToPass, st);
            if (!successful)
            {
                std::lock_guard<std::mutex> lock(commandQueueMutex);
                removeCommandFromQueues(toRun);
                toggleCommandActive(toRun, false);
            }

            // If command is non-repeating, mark it as inactive.
            if (toRun->repeatInMilliseconds <= 0)
            {
                toggleCommandActive(toRun, false);
            }
        }

        // Small sleep to avoid busy waiting.
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
