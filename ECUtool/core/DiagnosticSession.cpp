#include "DiagnosticSession.hpp"
#include <functional>

void DiagnosticSession::setConnection(std::shared_ptr<SerialConnection> newConnection)
{
	connection = newConnection;

	std::function<void(const DataMessage<uint8_t> &cb)> dataRcv = std::bind(&DiagnosticSession::handleOnDataRecieved, this, std::placeholders::_1);
	std::function<void(const DataMessage<uint8_t> &data)> dataSnt = std::bind(&DiagnosticSession::handleOnDataSent, this, std::placeholders::_1);
	std::function<void(const Message &msg)> msgRcv = std::bind(&DiagnosticSession::handleMessage, this, std::placeholders::_1);
	std::function<void(const SerialConnection::ConnectionStatus status)> statusCng = std::bind(&DiagnosticSession::handleStatusChange, this, std::placeholders::_1);

	connection->registerDataRecieveCallback(dataRcv);
	connection->registerDataSentCallback(dataSnt);
	connection->registerMessageCallback(msgRcv);
	connection->registerStatusCallback(statusCng);

}

void DiagnosticSession::setMessageViewCallback(std::function<void()> cb)
{
	messageViewCallback = cb;
}

void DiagnosticSession::setCommandViewCallback(std::function<void()> cb)
{
	commandsViewCallback = cb;
}

void DiagnosticSession::addCommand(std::shared_ptr<Command> &c)
{
	definedCommands.push_back(c);
	notifyCommandsView();
}

void DiagnosticSession::addMessage(Message &m)
{
	messageMutex.lock();
	outputMessages.push_back(m);
	messageMutex.unlock();
}

const std::vector<Message> &DiagnosticSession::getMessages()
{
	return outputMessages;
}

void DiagnosticSession::removeCommand(Command &c)
{
	for (int i = 0; i < definedCommands.size(); i++)
	{
		if (definedCommands[i].get() == &c)
		{
			definedCommands.erase(definedCommands.begin() + i);
			break;
		}
	}

	notifyCommandsView();
}

const std::vector<std::shared_ptr<Command>> &DiagnosticSession::getCommands()
{
	return definedCommands;
}

void DiagnosticSession::handleOnDataSent(const DataMessage<uint8_t> &data)
{

}

void DiagnosticSession::handleOnDataRecieved(const DataMessage<uint8_t> &data)
{

}

void DiagnosticSession::handleMessage(const Message &msg)
{

}

void DiagnosticSession::handleStatusChange(const SerialConnection::ConnectionStatus status)
{

}

void DiagnosticSession::notifyCommandsView()
{
	if (commandsViewCallback)
	{
		commandsViewCallback();
	}
}

void DiagnosticSession::notifyMessagesView()
{
	if (messageViewCallback)
	{
		messageViewCallback();
	}
}
