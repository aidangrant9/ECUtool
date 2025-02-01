#pragma once

#include "SerialConnection.hpp"

#include <string>
#include <thread>
#include <Windows.h>


class KLineWindows : public SerialConnection
{
protected:
	enum class InitMode
	{
		CARB = 1,
		FiveBaud,
		FastInit,
		None,
	};

	// ISO14230 Initialisation mode 
	InitMode mode;

	// Thread for async IO
	std::jthread workThread;

	// Communications resource handle
	HANDLE hCom{ INVALID_HANDLE_VALUE };
	DCB dcb{};

	virtual void poll();
	virtual bool initialise();
	virtual bool setPortConfiguration(size_t baudRate, size_t byteSize, Parity parity, StopBits stopBits);
public:
	KLineWindows(std::string &portName, size_t baudRate, size_t byteSize, Parity parity, StopBits stopBits, InitMode mode);
	~KLineWindows() override;

	virtual void connect()    override;
	virtual void disconnect() override;
};

 