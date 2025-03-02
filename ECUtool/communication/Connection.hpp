#pragma once

#include <vector>
#include <cstdint>
#include <queue>
#include <mutex>
#include <functional>
#include "sol/sol.hpp"
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

	virtual void writeVec(const std::vector<uint8_t> &toWrite)
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
		readMutex.lock();
		readQueue.push_back(data);
		readMutex.unlock();

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
	
	// Will burn CPU
	virtual std::vector<uint8_t> readOrTimeout(uint32_t ms)
	{
		std::chrono::time_point t1 = std::chrono::steady_clock::now();
		readMutex.lock();
		readQueue.clear();
		readMutex.unlock();
		std::vector<uint8_t> ret{};

		while (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t1).count() < ms)
		{
			readMutex.lock();
			if (!readQueue.empty())
			{
				ret = readQueue.front().data;
				readQueue.pop_front();
			}
			readMutex.unlock();

			if (ret.size() > 0)
				break;
		}

		return ret;
	}

	virtual void busyLoop(std::chrono::steady_clock::duration e)
	{
		std::chrono::time_point t1 = std::chrono::steady_clock::now();
		while (true)
		{
			if (std::chrono::steady_clock::now() - t1 > e)
				return;
		}
	}

	virtual void bindToLua(sol::state &s)
	{
		s.new_usertype<Connection>("connection",
			"name", &Connection::name,
			"write", [this](std::vector<uint8_t> m) {write(m);},
			"readOrTimeout", &Connection::readOrTimeout,
			"busyLoop", [this](uint32_t ms) {busyLoop(std::chrono::milliseconds(ms));});
	}

protected:
	ConnectionStatus connectionStatus{ ConnectionStatus::Disconnected };

	// Callbacks for GUI
	std::function<void(const DataMessage<uint8_t> &data)>   dataRecieveCallback{};
	std::function<void(const DataMessage<uint8_t> &data)>   dataSentCallback{};
	std::function<void(const Message &msg)>                 messageCallback{};
	std::function<void(const ConnectionStatus previous, const ConnectionStatus current)> statusCallback{};

	std::deque<DataMessage<uint8_t>> readQueue{};
	std::mutex readMutex{};

	std::deque<DataMessage<uint8_t>> writeQueue{};
	std::mutex writeMutex{};
};

