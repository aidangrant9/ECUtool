#include "DiagnosticSession.hpp"
#include <functional>
#include "RawCommand.hpp"
#include "ScriptCommand.hpp"
#include <fstream>
#include <unordered_set>

DiagnosticSession::DiagnosticSession(std::function<void(const Connection::ConnectionStatus status, const std::string message)> statusChangedCb)
	: statusChanged(statusChangedCb)
{
}

void DiagnosticSession::setConnection(std::shared_ptr<Connection> newConnection)
{
	commandExecutor = nullptr;

	connection = newConnection;
	connection->registerStatusCallback(statusChanged);
	connection->disconnect();
}

void DiagnosticSession::connect()
{
	commandExecutor = std::make_shared<CommandExecutor>(connection, [=]() {if (commandsResetStart) commandsResetStart();if (commandsResetEnd) commandsResetEnd();});

	if (connection.get() != nullptr)
		connection->connect();
}

void DiagnosticSession::disconnect()
{
	commandExecutor = nullptr;

	if (connection.get() != nullptr)
		connection->disconnect();
}

void DiagnosticSession::openProject(const std::filesystem::path path)
{
	projectRoot = path;

	// Clear commands
	if (commandsResetStart)
		commandsResetStart();
	commands.clear();
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

	if (!std::filesystem::exists(projectRoot / "globals"))
	{
		std::filesystem::create_directory(projectRoot / "globals");
	}
}

void DiagnosticSession::saveProject()
{
	std::unordered_set<std::string> currentNames;
	for (auto &cmd : commands) currentNames.insert(cmd->name);

	for (auto &file : std::filesystem::directory_iterator(projectRoot / "commands")) {
		if (file.is_regular_file() && file.path().extension() == ".json") {
			std::string name = file.path().stem().string();
			if (!currentNames.count(name)) {
				std::filesystem::remove(file.path());
			}
		}
	}

	for (auto &file : std::filesystem::directory_iterator(projectRoot / "scripts")) {
		if (file.is_regular_file() && file.path().extension() == ".lua") {
			std::string name = file.path().stem().string();
			if (!currentNames.count(name)) {
				std::filesystem::remove(file.path());
			}
		}
	}

	for (auto &cmd : commands) {
		std::ofstream(projectRoot / "commands" / (cmd->name + ".json")) << cmd->toJson();

		if (auto sc = dynamic_cast<ScriptCommand *>(cmd.get())) {
			auto scriptPath = projectRoot / "scripts" / (cmd->name + ".lua");
			if (!exists(scriptPath)) std::ofstream(scriptPath).close();
		}
	}
}

std::shared_ptr<Command> DiagnosticSession::loadCommandFromJson(std::string input)
{
	try 
	{
		json obj = json::parse(input);

		std::string name = obj["name"];
		std::size_t repeatInterval = obj["repeatInterval"];
		std::string type = obj["type"];
		bool visible = obj["visible"];

		if (type == "RAW")
		{
			std::vector<uint8_t> data = obj["data"];
			auto cmd = std::shared_ptr<Command>(new RawCommand(name, repeatInterval, data));
			cmd->visible = visible;
			return cmd;
		}
		if (type == "SCRIPT")
		{
			auto cmd = std::make_shared<ScriptCommand>(name, repeatInterval);
			cmd->mainScript = projectRoot / "scripts" / (name + ".lua");
			cmd->globalsDirectory = projectRoot / "globals";
			cmd->visible = visible;
			return cmd;
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
	commands.push_back(c);
	logger.setSourceVisible(c->name, c->visible);
	if (commandsResetEnd)
		commandsResetEnd();

	initialiseScript(c);
}

void DiagnosticSession::initialiseScript(std::shared_ptr<Command> c)
{
	if (auto p = dynamic_cast<ScriptCommand *>(c.get()))
	{
		p->mainScript = projectRoot / "scripts" / (c->name + ".lua");
		p->globalsDirectory = projectRoot / "globals";

		if (!std::filesystem::exists(p->mainScript))
		{
			std::ofstream scriptFile(p->mainScript);
			if (scriptFile.is_open())
			{
				scriptFile << "function entry()\n\n" << "end\n";
			}
		}
	}
}

bool DiagnosticSession::editCommand(int idx, std::shared_ptr<Command> &c)
{
	if (idx < commands.size())
	{
		if (commandsResetStart)
			commandsResetStart();
		commands[idx] = c;
		if (commandsResetEnd)
			commandsResetEnd();
		initialiseScript(c);
		return true;
	}

	return false;
}

bool DiagnosticSession::removeCommand(int idx)
{
	if (idx < commands.size())
	{
		if (commandsResetStart)
			commandsResetStart();
		commands.erase(commands.begin() + idx);
		if (commandsResetEnd)
			commandsResetEnd();
		return true;
	}
	return false;
}

std::vector<std::shared_ptr<Command>> DiagnosticSession::getCommands()
{
	return commands;
}

void DiagnosticSession::setCommandsResetStart(std::function<void()> cb)
{
	commandsResetStart = cb;
}

void DiagnosticSession::setCommandsResetEnd(std::function<void()> cb)
{
	commandsResetEnd = cb;
}

void DiagnosticSession::queueOrUnqueueCommand(std::shared_ptr<Command> c, std::string arguments)
{
	if (commandExecutor.get() != nullptr && connection->getStatus() == Connection::ConnectionStatus::Connected)
	{
		commandExecutor->queueOrUnqueueCommand(c, arguments);
	}
}
