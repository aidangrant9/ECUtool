#pragma once

#include <vector>
#include <cstdint>
#include <queue>
#include <mutex>
#include <functional>
#include "sol/sol.hpp"
#include "../core/Logger.hpp"
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

	Connection()
	{}

	virtual ~Connection() = default;

	virtual std::vector<uint8_t> read() = 0;
	virtual void write(const std::vector<uint8_t> data) = 0;
	virtual std::string name() = 0;
	virtual void connect() = 0;
	virtual void disconnect() = 0;

	void registerStatusCallback(std::function<void(const ConnectionStatus status, const std::string message)> cb)
	{
		statusCallback = cb;
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
			"read", [this](){ read(); },
			"sleep", [this](uint32_t ms) {busyLoop(std::chrono::milliseconds(ms));},
			"setGlobalState", &Connection::setGlobalState,
		    "getGlobalState", &Connection::getGlobalState);
	}

	virtual ConnectionStatus getStatus()
	{
		notifyStatusCallback(connectionStatus, getStatusString());
		return connectionStatus;
	}


	virtual void logRead(std::vector<uint8_t> data)
	{
		logger.addMessage(Message{ "READ: " + Logger::stringFromDataVec(data), name() }, true);
	}


	virtual void logWrite(std::vector<uint8_t> data)
	{
		logger.addMessage(Message{ "WRITE: " + Logger::stringFromDataVec(data), name() }, true);
	}


	void setGlobalState(std::string key, std::string value)
	{
		globalStrings[key] = value;
	}

	std::string getGlobalState(std::string key)
	{
		if (globalStrings.contains(key))
			return globalStrings[key];
		else
			return "";
	}

protected:
	bool notifyStatusCallback(const ConnectionStatus status, const std::string message)
	{
		if (statusCallback)
		{
			statusCallback(status, message);
			return true;
		}
		return false;
	}

	virtual std::string getStatusString()
	{
		return connectionStatus == ConnectionStatus::Connected ? "Connected" : "Disconnected";
	}

	bool changeConnectionStatus(const ConnectionStatus status)
	{
		connectionStatus = status;
		bool r = notifyStatusCallback(connectionStatus, getStatusString());
		return r;
	}

	bool changeConnectionStatus(const ConnectionStatus status, const std::string errorMessage)
	{
		logger.addErrorMessage(Message{ errorMessage, name() });
		return changeConnectionStatus(status);
	}

	ConnectionStatus connectionStatus{ ConnectionStatus::Disconnected };

	// Callbacks for GUI
	std::function<void(const ConnectionStatus status, const std::string message)> statusCallback{};

	Logger &logger = Logger::instance();

	// Global Script Value Store
	std::unordered_map<std::string, std::string> globalStrings{};
};

