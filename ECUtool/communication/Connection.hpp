#pragma once

#include <vector>
#include <cstdint>
#include <queue>
#include <mutex>
#include <functional>
#include "DataMessage.hpp"
#include "../core/Message.hpp"
#include "../../serial/include/serial/serial.h"


/*
	Abstract class for serial connections
*/

using namespace serial;

class Connection
{
public:
	enum class ConnectionStatus
	{
		Disconnected,
		Connected,
	};

	Connection()          = default;
	virtual ~Connection() = default;

	virtual void write(const DataMessage<uint8_t> &toWrite)
	{
		std::lock_guard<std::mutex> lock{ writeMutex };
		writeQueue.push_back(toWrite);
	}

	void registerDataRecieveCallback(std::function<void(const DataMessage<uint8_t> &data)> &cb)
	{
		dataRecieveCallback = cb;
	}

	void registerDataSentCallback(std::function<void(const DataMessage<uint8_t> &data)> &cb)
	{
		dataSentCallback = cb;
	}

	void registerMessageCallback(std::function<void(const Message &msg)> &cb)
	{
		messageCallback = cb;
	}

	void registerStatusCallback(std::function<void(const ConnectionStatus previous, const ConnectionStatus current)> &cb)
	{
		statusCallback = cb;
	}

	bool notifyDataRecieveCallback(const DataMessage<uint8_t> &data)
	{
		if (dataRecieveCallback)
		{
			dataRecieveCallback(data);
			return true;
		}
		return false;
	}

	bool notifyDataSentCallback(const DataMessage<uint8_t> &data)
	{
		if (dataSentCallback)
		{
			dataSentCallback(data);
			return true;
		}
		return false;
	}

	bool notifyMessageCallback(const Message &msg)
	{
		if (messageCallback)
		{
			messageCallback(msg);
			return true;
		}
		return false;
	}

	bool notifyStatusCallback(const ConnectionStatus previous, const ConnectionStatus current)
	{
		if (statusCallback)
		{
			statusCallback(previous, current);
			return true;
		}
		return false;
	}


	bool changeConnectionStatus(const ConnectionStatus status)
	{
		bool r = notifyStatusCallback(connectionStatus, status);
		this->connectionStatus = status;
		return r;
	}

	bool changeConnectionStatus(const ConnectionStatus status, const std::string errorMessage)
	{
		return changeConnectionStatus(status) && notifyMessageCallback(Message{Message::MessageType::Error, errorMessage, name()});
	}

	virtual std::string name() = 0;
	virtual void connect() = 0;
	virtual void disconnect() = 0;
protected:
	ConnectionStatus connectionStatus{ ConnectionStatus::Disconnected };

	// Callbacks for GUI
	std::function<void(const DataMessage<uint8_t> &data)>   dataRecieveCallback{};
	std::function<void(const DataMessage<uint8_t> &data)>   dataSentCallback{};
	std::function<void(const Message &msg)>                 messageCallback{};
	std::function<void(const ConnectionStatus previous, const ConnectionStatus current)> statusCallback{};

	std::deque<DataMessage<uint8_t>> writeQueue{};
	std::mutex writeMutex{};
};

