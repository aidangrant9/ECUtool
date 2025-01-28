#pragma once

#include "SerialConnection.hpp"

#include <string>
#include <thread>
#include <atomic>

class KLineConnection : public SerialConnection
{
protected:
	enum class InitMode
	{
		CARB = 1,
		FiveBaud,
		FastInit,
	};

	InitMode mode;

	std::thread pollThread;
	struct PlatformSpecifics;
	virtual void poll(PlatformSpecifics);
public:
	KLineConnection(std::string &portName, size_t baudRate, size_t byteSize, Parity parity, StopBits stopBits, InitMode mode);
	~KLineConnection() override;

	virtual int connect() override;
	virtual int disconnect() override;
};

 