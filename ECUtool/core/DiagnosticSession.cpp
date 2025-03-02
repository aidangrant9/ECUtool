#include "DiagnosticSession.hpp"
#include <functional>
#include "RawCommand.hpp"
#include "ScriptCommand.hpp"
#include <fstream>

void DiagnosticSession::setConnection(std::shared_ptr<Connection> newConnection)
{
	connection = newConnection;

	std::function<void(const DataMessage<uint8_t> &cb)> dataRcv = std::bind(&DiagnosticSession::handleOnDataRecieved, this, std::placeholders::_1);
	std::function<void(const DataMessage<uint8_t> &data)> dataSnt = std::bind(&DiagnosticSession::handleOnDataSent, this, std::placeholders::_1);
	std::function<void(const Message &msg)> msgRcv = std::bind(&DiagnosticSession::handleMessage, this, std::placeholders::_1);
	std::function<void(const Connection::ConnectionStatus previous, const Connection::ConnectionStatus current)> statusCng =
		std::bind(&DiagnosticSession::handleStatusChange, this, std::placeholders::_1, std::placeholders::_2);

	connection->registerDataRecieveCallback(dataRcv);
	connection->registerDataSentCallback(dataSnt);
	connection->registerMessageCallback(msgRcv);
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

	if (messageResetStart)
		messageResetStart();

	outputMessages.clear();

	if (messageResetEnd)
		messageResetEnd();

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


void DiagnosticSession::setMessageResetStart(std::function<void()> cb)
{
	messageResetStart = cb;
}

void DiagnosticSession::setCommandsResetStart(std::function<void()> cb)
{
	commandsResetStart = cb;
}

void DiagnosticSession::setMessageResetEnd(std::function<void()> cb)
{
	messageResetEnd = cb;
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

void DiagnosticSession::addMessage(std::shared_ptr<Message> m)
{
	messageMutex.lock();
	if (messageResetStart)
		messageResetStart();
	outputMessages.push_back(m);
	if (messageResetEnd)
		messageResetEnd();
	messageMutex.unlock();
}

const std::vector<std::shared_ptr<Message>> &DiagnosticSession::getMessages()
{
	return outputMessages;
}

std::string DiagnosticSession::stringFromDataVec(std::vector<uint8_t> data)
{
	std::ostringstream o;
	o << std::hex << std::uppercase << std::setfill('0');

	for (size_t i = 0; i < data.size(); ++i)
	{
		o << std::setw(2) << static_cast<int>(data[i]);

		if (i < data.size() - 1)
			o << " ";
	}
	return o.str();
}

std::vector<uint8_t> DiagnosticSession::dataVecFromString(std::string input)
{
	input.erase(std::remove(input.begin(), input.end(), ' '), input.end());
	std::vector<uint8_t> ret{};
	if (input.length() % 2 != 0)
	{
		input = input;
	}
	for (int i = 0; i < input.length(); i += 2)
	{
		std::string byte = input.substr(i, 2);
		ret.push_back(std::stoi(byte, nullptr, 16));
	}

	return ret;
}


void DiagnosticSession::errorMessage(const std::string &msg, const std::string &src)
{
	addMessage(std::shared_ptr<Message>(new Message{ Message::MessageType::Error, "< div style = \"color:#e76f51\">" + msg + "</div>", src}));
}

void DiagnosticSession::dataRecievedMessage(const DataMessage<uint8_t> &msg, const std::string &src)
{
	std::string contents = stringFromDataVec(msg.data);
	addMessage(std::shared_ptr<Message>(new Message{ Message::MessageType::Data, "<div style=\"color:#118ab2\">Res. </div>" + contents,
		src, msg.id, {std::make_pair(std::string{"Data"}, contents)}}));
}

void DiagnosticSession::dataSentMessage(const DataMessage<uint8_t> &msg, const std::string &src)
{
	std::string contents = stringFromDataVec(msg.data);
	addMessage(std::shared_ptr<Message>(new Message{ Message::MessageType::Data, "<div style=\"color:#06d6a0\">Req. </div>" + contents,
		src, msg.id, {std::make_pair(std::string{"Data"}, contents)} }));
}

void DiagnosticSession::successMessage(const std::string &msg, const std::string &src)
{
	addMessage(std::shared_ptr<Message>(new Message{ Message::MessageType::Info, "<div style=\"color:#2a9d8f\">" + msg + "</div>", src}));
}

void DiagnosticSession::infoMessage(const std::string &msg, const std::string &src)
{
	addMessage(std::shared_ptr<Message>(new Message{ Message::MessageType::Info, msg, src}));
}

std::vector<uint8_t> DiagnosticSession::readOrTimeout(int ms)
{
	auto startTime = std::chrono::steady_clock::now();
	std::vector<uint8_t> data{};
	incomingQueue.clear();

	while (true)
	{
		incomingMutex.lock();
		if (!incomingQueue.empty())
		{
			data = incomingQueue.front().data;
			incomingQueue.pop_front();
		}
		incomingMutex.unlock();

		if (!data.empty())
		{
			return data;
		}

		if (std::chrono::steady_clock::now() - startTime > std::chrono::milliseconds(ms))
		{
			return {};
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	
}

void DiagnosticSession::send(std::vector<uint8_t> data)
{
	if (connection)
	{
		connection->write(data);
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

void DiagnosticSession::handleOnDataSent(const DataMessage<uint8_t> &data)
{
	dataSentMessage(data, "DiagnosticSession");
}

void DiagnosticSession::handleOnDataRecieved(const DataMessage<uint8_t> &data)
{
	incomingMutex.lock();
	incomingQueue.push_front(data);
	incomingMutex.unlock();

	dataRecievedMessage(data, "DiagnosticSession");
}

// Needs cleaning
void DiagnosticSession::handleMessage(const Message &msg)
{
	switch (msg.type)
	{
	case Message::MessageType::Error:
		errorMessage(msg.msg, msg.source);
		break;
	case Message::MessageType::Info:
		infoMessage(msg.msg, msg.source);
		break;
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