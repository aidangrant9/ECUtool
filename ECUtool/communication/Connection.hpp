#pragma once

#include <vector>
#include <cstdint>
#include <queue>
#include <mutex>
#include <functional>
#include "sol/sol.hpp"
#include "DataMessage.hpp"
#include "../core/Message.hpp"
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

	Connection()          = default;
	virtual ~Connection() = default;

	virtual std::vector<uint8_t> read() = 0;
	virtual void write(const std::vector<uint8_t> data) = 0;
	virtual std::string name() = 0;
	virtual void connect() = 0;
	virtual void disconnect() = 0;

	void registerStatusCallback(std::function<void(const ConnectionStatus previous, const ConnectionStatus current)> &cb)
	{
		statusCallback = cb;
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
		logger.addErrorMessage(Message{ errorMessage, name()});
		return changeConnectionStatus(status);
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
			"sleep", [this](uint32_t ms) {busyLoop(std::chrono::milliseconds(ms));});
	}

protected:
	ConnectionStatus connectionStatus{ ConnectionStatus::Disconnected };

	// Callbacks for GUI
	std::function<void(const ConnectionStatus previous, const ConnectionStatus current)> statusCallback{};

	Logger &logger = Logger::instance();
};

