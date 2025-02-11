#pragma once

#include <vector>
#include <cstdint>
#include <queue>
#include <mutex>
#include <functional>
#include "DataMessage.hpp"
#include "../core/Message.hpp"

/*
	Abstract class for serial connections
*/

class SerialConnection
{
public:
	enum class ConnectionStatus
	{
		Disconnected,
		Connected,
		Error,
	};

	enum class StopBits
	{
		OneStopBit,
		One5StopBit,
		TwoStopBit,
	};

	enum class Parity
	{
		None,
		Even,
		Odd,
		Mark,
		Space,
	};

	virtual ~SerialConnection() = default;

	virtual void write(const DataMessage<uint8_t> &toWrite)
	{
		std::lock_guard<std::mutex> lock{ writeMutex };
		writeQueue.push_front(toWrite);
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

	void registerStatusCallback(std::function<void(const ConnectionStatus status)> &cb)
	{
		statusCallback = cb;
	}

	bool notifyDataRecieveCallback(const DataMessage<uint8_t> &data)
	{
		try
		{
			dataRecieveCallback(data);
			return true;
		}
		catch (std::bad_function_call e)
		{
			return false;
		}
	}

	bool notifyDataSentCallback(const DataMessage<uint8_t> &data)
	{
		try
		{
			dataSentCallback(data);
			return true;
		}
		catch (std::bad_function_call e)
		{
			return false;
		}
	}

	bool notifyMessageCallback(const Message &msg)
	{
		try
		{
			messageCallback(msg);
			return true;
		}
		catch (std::bad_function_call e)
		{
			return false;
		}
	}

	bool notifyStatusCallback(const ConnectionStatus status)
	{
		try
		{
			statusCallback(status);
			return true;
		}
		catch (std::bad_function_call e)
		{
			return false;
		}
	}

	/// <summary>
	/// Changes the internal connection status and notifies listeners
	/// </summary>
	/// <param name="status">New connection status</param>
	/// <returns>Callback run successfully</returns>
	bool changeConnectionStatus(const ConnectionStatus status)
	{
		this->connectionStatus = status;
		return notifyStatusCallback(status);
	}

	/// <summary>
	/// Changes the internal connection status and produces an error message
	/// </summary>
	/// <param name="status">New connection status</param>
	/// <param name="errorMessage">Error message for callback</param>
	/// <returns>Callbacks run successfully</returns>
	bool changeConnectionStatus(const ConnectionStatus status, const std::string errorMessage)
	{
		return changeConnectionStatus(status) && notifyMessageCallback(Message{Message::MessageType::Error, errorMessage});
	}

	virtual void connect() = 0;
	virtual void disconnect() = 0;
protected:
	SerialConnection(std::string &portName, size_t baudRate, size_t byteSize, Parity parity, StopBits stopBits) :
		portName{portName}, baudRate{baudRate}, byteSize{byteSize}, parity{parity}, stopBits{stopBits}
	{
	}

	ConnectionStatus connectionStatus{ ConnectionStatus::Disconnected };

	// Callbacks for GUI
	std::function<void(const DataMessage<uint8_t> &data)>   dataRecieveCallback{};
	std::function<void(const DataMessage<uint8_t> &data)>   dataSentCallback{};
	std::function<void(const Message &msg)>                 messageCallback{};
	std::function<void(const ConnectionStatus status)>      statusCallback{};

	std::deque<DataMessage<uint8_t>> writeQueue{};
	std::mutex writeMutex{};

	// Platform agnostic port configuration
	std::string portName{};
	size_t baudRate{};
	size_t byteSize{};
	Parity parity{};
	StopBits stopBits{};
};

