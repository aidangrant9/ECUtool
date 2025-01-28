#pragma once

/*
	Abstract class for serial connections
*/

#include <vector>
#include <cstdint>
#include <queue>
#include <mutex>
#include <optional>
#include <functional>
#include <unordered_map>

class SerialConnection
{
public:
	enum class ConnectionStatus
	{
		Open,
		Closed
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

	virtual ~SerialConnection() = 0;

	virtual int write(const std::vector<uint8_t>& toWrite)
	{
		std::lock_guard<std::mutex> lock{ writeMutex };
		writeQueue.push_back(toWrite);
	}

	// Returns latest message or nullopt
	virtual std::optional<std::vector<uint8_t>&> peek()
	{
		std::lock_guard<std::mutex> lock{ readMutex };
		if (readQueue.empty())
		{
			return std::nullopt;
		}
		return readQueue.front();
	}

	// Returns ID of function callback needed to deregister callback
	virtual uint64_t registerCallback(const std::function<void(std::vector<uint8_t>)> cb)
	{
		std::lock_guard<std::mutex> lock{ cbMutex };
		callbacks[callbackID++] = cb;
	}

	// Deregisters callback
	virtual int deregisterCallback(const uint64_t id)
	{
		std::lock_guard<std::mutex> lock{ cbMutex };
		callbacks.erase(id);
	}

	virtual int notifyCallbacks(const std::vector<uint8_t> &msg)
	{
		std::lock_guard<std::mutex> lock{ cbMutex };
		for (auto & [id, cb] : callbacks)
		{
			cb(msg);
		}
	}

	virtual int connect() = 0;
	virtual int disconnect() = 0;
	virtual ConnectionStatus status() = 0;


protected:
	SerialConnection(std::string &portName, size_t baudRate, size_t byteSize, Parity parity, StopBits stopBits) :
		portName{portName}, baudRate{baudRate}, byteSize{byteSize}, parity{parity}, stopBits{stopBits}, connectionStatus{ConnectionStatus::Closed}
	{
	}

	std::unordered_map<uint64_t, std::function<void(std::vector<uint8_t>)>> callbacks{};
	uint64_t callbackID = 0;

	std::deque<std::vector<uint8_t>> writeQueue{};
	std::deque<std::vector<uint8_t>> readQueue{};

	std::mutex writeMutex;
	std::mutex readMutex;
	std::mutex cbMutex;

	ConnectionStatus connectionStatus;

	// Platform agnostic port configuration
	std::string portName;
	size_t baudRate;
	size_t byteSize;
	Parity parity;
	StopBits stopBits;
};

