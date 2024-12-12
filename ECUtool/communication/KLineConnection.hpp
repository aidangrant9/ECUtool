#pragma once

#include "SerialConnection.hpp"

#include <string>
#include <Windows.h>
#include <thread>

class KLineConnection : public SerialConnection
{
private:
	HANDLE hCom = NULL;
	std::string portName;
	int baudRate;
	std::thread pollThread;

	virtual void poll();

public:
	KLineConnection(std::string portName, int baudRate);
	~KLineConnection();

	virtual void connect() override;
	virtual void disconnect() override;
};

