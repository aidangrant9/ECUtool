#include "KLineConnection.hpp"

#include <Windows.h>
#include <iostream>
#include <chrono>

KLineConnection::KLineConnection(std::string &portName, size_t baudRate, size_t byteSize, Parity parity, StopBits stopBits, InitMode mode)
	: SerialConnection(portName, baudRate, byteSize, parity, stopBits), mode{ mode }
{
}

KLineConnection::~KLineConnection()
{
	this->disconnect();
}

int KLineConnection::disconnect()
{
	// TODO
}

int KLineConnection::connect()
{
	HANDLE hCom;
	DCB dcb;
	BOOL fSuccess;

	std::wstring comName = std::wstring(portName.begin(), portName.end());

	hCom = CreateFile(comName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

	if (hCom == INVALID_HANDLE_VALUE)
	{
		return -1;
	}

	SecureZeroMemory(&dcb, sizeof(DCB));
	dcb.DCBlength = sizeof(DCB);

	if (!GetCommState(hCom, &dcb))
	{
		return -1;
	}

	dcb.BaudRate = this->baudRate;
	dcb.ByteSize = this->byteSize;

	switch (this->parity)
	{
	case Parity::Even:
		dcb.Parity = EVENPARITY;
		break;
	case Parity::Odd:
		dcb.Parity = ODDPARITY;
		break;
	case Parity::Mark:
		dcb.Parity = MARKPARITY;
		break;
	case Parity::Space:
		dcb.Parity = SPACEPARITY;
		break;
	default:
		dcb.Parity = NOPARITY;
		break;
	}

	switch (this->stopBits)
	{
	case StopBits::OneStopBit:
		dcb.StopBits = ONESTOPBIT;
		break;
	case StopBits::One5StopBit:
		dcb.StopBits = ONE5STOPBITS;
		break;
	case StopBits::TwoStopBit:
		dcb.StopBits = TWOSTOPBITS;
		break;
	default:
		dcb.StopBits = ONESTOPBIT;
		break;
	}

	if (!SetCommState(hCom, &dcb))
	{
		return -1;
	}

	this->handle = reinterpret_cast<uint64_t>(hCom);
	this->connectionStatus = ConnectionStatus::Open;
	this->pollThread = std::thread(&KLineConnection::poll, this);

	return 0;
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
		p1: 0
		p2: 25
		p3: 55
		p4: 5

		Timings in ms
	*/

	PurgeComm(hCom, PURGE_RXCLEAR | PURGE_TXCLEAR);


	int p1 = 0, p2 = 25, p3 = 55, p4 = 5;
	
	COMMTIMEOUTS readTimeouts = { p1, 1, 0, 1, 0 };

	while (true)
	{
		// Check if there is a message to send
		writeMutex.lock();
		if (!writeQueue.empty())
		{
			std::vector<uint8_t> toSend = writeQueue.back();
			writeQueue.pop_back();
			DWORD bytesWritten = 0;

			// No send delay
			WriteFile(hCom, toSend.data(), toSend.size(), &bytesWritten, 0);

			/*
			for (uint8_t b : toSend)
			{
				WriteFile(hCom, &b, 1, &bytesWritten, 0);
				std::this_thread::sleep_for(std::chrono::milliseconds(p4));
			}
			*/
			

			std::this_thread::sleep_for(std::chrono::milliseconds(p2));
		}
		writeMutex.unlock();

		if (!SetCommTimeouts(this->hCom, &readTimeouts))
		{
			std::cerr << "SetCommTimeouts failed with err " << GetLastError() << std::endl;;
		}

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

		std::this_thread::sleep_for(std::chrono::milliseconds(p3));
	}
}