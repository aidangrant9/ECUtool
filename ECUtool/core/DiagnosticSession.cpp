#include "DiagnosticSession.hpp"
#include <functional>
#include "RawCommand.hpp"
#include "ScriptCommand.hpp"
#include <fstream>

void DiagnosticSession::setConnection(std::shared_ptr<Connection> newConnection)
{
	connection = newConnection;

	std::function<void(const Connection::ConnectionStatus previous, const Connection::ConnectionStatus current)> statusCng =
		std::bind(&DiagnosticSession::handleStatusChange, this, std::placeholders::_1, std::placeholders::_2);

	connection->registerStatusCallback(statusCng);

	if (statusChanged)
		statusChanged(Connection::ConnectionStatus::Disconnected);
}

void DiagnosticSession::connect()
{
	commandExecutor = std::shared_ptr<CommandExecutor>(new CommandExecutor(this, std::shared_ptr<Connection>(connection), projectRoot));

	if (connection.get() != nullptr)
		connection->connect();
}

void DiagnosticSession::disconnect()
{
	if (connection.get() != nullptr)
		connection->disconnect();

	commandExecutor = nullptr;
}

void DiagnosticSession::openProject(const std::filesystem::path &path)
{
	projectRoot = path;

	if (commandsResetStart)
		commandsResetStart();
	definedCommands.clear();

	if (commandsResetEnd)
		commandsResetEnd();
	
	if (!std::filesystem::exists(projectRoot / "commands"))
	{
		std::filesystem::create_directory(projectRoot / "commands");
	}
	else
	{
		for (auto &file : std::filesystem::directory_iterator(projectRoot / "commands"))
		{
			if (file.is_regular_file() && file.path().extension() == ".json")
			{
				std::ifstream commandJson(file.path());
				if (commandJson.is_open())
				{
					std::stringstream buf{};
					buf << commandJson.rdbuf();
					std::shared_ptr<Command> c = loadCommandFromJson(buf.str());
					if (c.get() != nullptr)
					{
						addCommand(c);
					}
				}
			}
		}
	}

	if (!std::filesystem::exists(projectRoot / "scripts"))
	{
		std::filesystem::create_directory(projectRoot / "scripts");
	}

	if (!std::filesystem::exists(projectRoot / "globals.lua"))
	{
		// create global.lua here
		std::ofstream globalScript(projectRoot / "globals.lua");
	}
}

void DiagnosticSession::saveProject()
{
	for (auto &p : definedCommands)
	{
		std::ofstream file(projectRoot / "commands" / std::string{ p->name + std::string{ ".json" } }, std::ios::trunc);
		if (file.is_open())
		{
			file << p->toJson();
		}
	}


}

void DiagnosticSession::setCommandsResetStart(std::function<void()> cb)
{
	commandsResetStart = cb;
}

void DiagnosticSession::setCommandsResetEnd(std::function<void()> cb)
{
	commandsResetEnd = cb;
}

void DiagnosticSession::setStatusChanged(std::function<void(std::optional<Connection::ConnectionStatus>)> statusChanged)
{
	this->statusChanged = statusChanged;
}

std::shared_ptr<Command> DiagnosticSession::loadCommandFromJson(std::string input)
{
	try 
	{
		json obj = json::parse(input);

		std::string name = obj["name"];
		std::size_t repeatInterval = obj["repeatInterval"];
		std::string type = obj["type"];

		if (type == "RAW")
		{
			std::vector<uint8_t> data = obj["data"];
			return std::shared_ptr<Command>(new RawCommand(name, repeatInterval, data));
		}
		if (type == "SCRIPT")
		{
			return std::shared_ptr<Command>(new ScriptCommand(name, repeatInterval));
		}
	}
	catch (std::exception e)
	{
		return std::shared_ptr<Command>(nullptr);
	}

	return std::shared_ptr<Command>(nullptr);
}

void DiagnosticSession::addCommand(std::shared_ptr<Command> c)
{
	if (commandsResetStart)
		commandsResetStart();
	definedCommands.push_back(c);
	if (commandsResetEnd)
		commandsResetEnd();

	initialiseScript(c);
}

void DiagnosticSession::initialiseScript(std::shared_ptr<Command> c)
{
	if (auto p = dynamic_cast<ScriptCommand *>(c.get()))
	{
		if (!std::filesystem::exists(projectRoot / "scripts" / std::string{ c->name + ".lua" }))
		{
			std::ofstream globalScript(projectRoot / "scripts" / std::string{ c->name + ".lua" });
		}
	}
}

bool DiagnosticSession::editCommand(int idx, std::shared_ptr<Command> &c)
{
	if (idx < definedCommands.size())
	{
		if (commandsResetStart)
			commandsResetStart();
		definedCommands[idx] = c;
		if (commandsResetEnd)
			commandsResetEnd();
		initialiseScript(c);
		return true;
	}

	return false;
}

bool DiagnosticSession::removeCommand(int idx)
{
	if (idx < definedCommands.size())
	{
		if (commandsResetStart)
			commandsResetStart();
		definedCommands.erase(definedCommands.begin() + idx);
		if (commandsResetEnd)
			commandsResetEnd();
		return true;
	}
	return false;
}

const std::vector<std::shared_ptr<Command>> &DiagnosticSession::getCommands()
{
	return definedCommands;
}

void DiagnosticSession::queueCommand(std::shared_ptr<Command> c)
{
	if (commandExecutor.get() != nullptr)
	{
		commandExecutor->queueCommand(c);
	}
}

void DiagnosticSession::handleStatusChange(const Connection::ConnectionStatus status, const Connection::ConnectionStatus current)
{
	if (connection.get() == nullptr)
	{
		if (statusChanged)
			statusChanged(std::nullopt);

		//addMessage(std::shared_ptr<Message>(Message(Message::MessageType::Info, "<>")))

		return;
	}
	else
	{
		if (statusChanged)
			statusChanged(std::optional<Connection::ConnectionStatus>(current));
		return;
	}
}