#include "KLineConnection.hpp"

#include <iostream>
#include <chrono>

KLineConnection::KLineConnection(std::string portName, int baudRate) : portName(portName), baudRate(baudRate)
{
}

KLineConnection::~KLineConnection()
{
	this->disconnect();
}

void KLineConnection::disconnect()
{
	// TODO
}

/*
	Majority of the code from microsoft's documentation
	https://learn.microsoft.com/en-us/windows/win32/devio/configuring-a-communications-resource
*/

void KLineConnection::connect()
{
	DCB dcb;
	BOOL fSuccess;
	DWORD dwEvtMask;

	std::wstring comName = std::wstring(portName.begin(), portName.end());

	this->hCom = CreateFile(comName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

	if (this->hCom == INVALID_HANDLE_VALUE)
	{
		std::cerr << "CreateFile failed with err " << GetLastError() << std::endl;
		return;
	}

	SecureZeroMemory(&dcb, sizeof(DCB));
	dcb.DCBlength = sizeof(DCB);

	if (!GetCommState(this->hCom, &dcb))
	{
		std::cerr << "GetCommState failed with err " << GetLastError() << std::endl;
		return;
	}

	dcb.BaudRate = baudRate;
	dcb.ByteSize = 8;
	dcb.Parity = ODDPARITY;
	dcb.StopBits = ONESTOPBIT;

	if (!SetCommState(this->hCom, &dcb))
	{
		std::cerr << "SetCommState failed with err " << GetLastError() << std::endl;
		return;
	}

	this->pollThread = std::thread(&KLineConnection::poll, this);
	this->connected = 1;
}

void KLineConnection::poll()
{
	/*
		Timing restrictions
		p1:	Inter byte for ecu response 
		p2: Time between tester request and ecu response or multiple responses
		p3: Time between end of ecu response and start of new tester request
		p4:	Inter byte time for tester request

		p3min > p4min
		
		in case of detecting by timeout:
		p2min > p4max
		p2min > p1max


		defaults:
		p1: 20
		p2: 50
		p3: 5000
		p4: 20

		Timings in ms
	*/

	COMMTIMEOUTS readTimeouts = { 20, 1, 0, 1, 0 };
	

	while (true)
	{
		// Check if there is a message to send
		writeMutex.lock();
		if (!writeQueue.empty())
		{
			std::vector<uint8_t> toSend = writeQueue.back();
			writeQueue.pop_back();
			DWORD bytesWritten = 0;

			for (uint8_t b : toSend)
			{
				WriteFile(hCom, &b, 1, &bytesWritten, 0);
			}
		}
		writeMutex.unlock();

		if (!SetCommTimeouts(this->hCom, &readTimeouts))
		{
			std::cerr << "SetCommTimeouts failed with err " << GetLastError() << std::endl;;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(30));

		std::vector<uint8_t> messageRead{};

		while (true)
		{
			uint8_t buf[265];
			DWORD read = 0;
			ReadFile(hCom, buf, 265, &read, 0);

			if (read) 
			{
				for (int i = 0; i < read; i++)
				{
					messageRead.push_back(buf[i]);
				}
			}
			else
			{
				break;
			}
		}

		if (messageRead.size() > 0)
		{
			this->doCallbacks(messageRead);
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
}