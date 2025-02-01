#include "KLineWindows.hpp"

#include <iostream>
#include <chrono>

KLineWindows::KLineWindows(std::string &portName, size_t baudRate, size_t byteSize, Parity parity, StopBits stopBits, InitMode mode)
	: SerialConnection(portName, baudRate, byteSize, parity, stopBits), mode{ mode }
{
}

KLineWindows::~KLineWindows()
{
	this->disconnect();
}

void KLineWindows::disconnect()
{
	// TODO
}

void KLineWindows::connect()
{
	std::wstring comName = std::wstring(portName.begin(), portName.end());

	hCom = CreateFile(comName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

	if (hCom == INVALID_HANDLE_VALUE)
	{
		changeConnectionStatus(ConnectionStatus::Error, "Failed to create Windows file handle");
		return;
	}

	this->workThread = std::jthread(&KLineWindows::poll, this);
}


/// <summary>
/// Handles intialisation of the data link
/// </summary>
/// <returns>Success</returns>
bool KLineWindows::initialise()
{
	SecureZeroMemory(&dcb, sizeof(DCB));
	dcb.DCBlength = sizeof(DCB);

	if (!GetCommState(hCom, &dcb))
	{
		changeConnectionStatus(ConnectionStatus::Error, "Failed to get com state");
		return false;
	}


	if (mode == InitMode::None)
	{
		return true;
	}
	else if (mode == InitMode::FastInit)
	{
		if (!setPortConfiguration(this->baudRate, this->byteSize, this->parity, this->stopBits))
		{
			changeConnectionStatus(ConnectionStatus::Error, "Failed to set port configuration");
			return;
		}

		// Wake up pattern
		std::this_thread::sleep_for(std::chrono::milliseconds(300)); // T_idle
		EscapeCommFunction(hCom, SETBREAK);
		std::this_thread::sleep_for(std::chrono::milliseconds(25)); // T_inil
		EscapeCommFunction(hCom, CLRBREAK);
		std::this_thread::sleep_for(std::chrono::milliseconds(25)); // T_wup - T_inil

		// Send StartCommunicationMessage
		write({});
	}
	else
	{
		changeConnectionStatus(ConnectionStatus::Error, "Initialisation mode not implemented");
		return false;
	}

	return true;
}


/// <summary>
/// Set the port configuration DCB
/// </summary>
/// <param name="baudRate"></param>
/// <param name="byteSize"></param>
/// <param name="parity"></param>
/// <param name="stopBits"></param>
/// <returns>Success</returns>
bool KLineWindows::setPortConfiguration(size_t baudRate, size_t byteSize, Parity parity, StopBits stopBits)
{
	if (!GetCommState(hCom, &dcb))
	{
		changeConnectionStatus(ConnectionStatus::Error, "Failed to get com state");
		return false;
	}

	dcb.BaudRate = baudRate;
	dcb.ByteSize = byteSize;

	switch (parity)
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

	switch (stopBits)
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
		changeConnectionStatus(ConnectionStatus::Error, "Failed to set com state");
		return false;
	}

	return true;
}


/// <summary>
/// Main work thread
/// </summary>
void KLineWindows::poll()
{
	std::stop_token st = workThread.get_stop_token();

	// Initialise the connection
	if (!initialise())
	{
		changeConnectionStatus(ConnectionStatus::Error, "Failed to initialise connection");
		return;
	}

	if (!setPortConfiguration(this->baudRate, this->byteSize, this->parity, this->stopBits))
	{
		changeConnectionStatus(ConnectionStatus::Error, "Failed to set port configuration");
		return;
	}


	PurgeComm(hCom, PURGE_RXCLEAR | PURGE_TXCLEAR);	

	COMMTIMEOUTS readTimeouts = { 20, 0, 0, p4, 0 };

	if (!SetCommTimeouts(hCom, &readTimeouts))
	{
		changeConnectionStatus(ConnectionStatus::Error, "Failed to set com timeouts");
		return;
	}

	while (true)
	{
		if (st.stop_requested())
		{
			// Polling stops here
			return;
		}

		// Check if there is a message to send
		writeMutex.lock();
		if (!writeQueue.empty())
		{
			std::vector<uint8_t> toSend = writeQueue.back();
			DWORD bytesWritten = 0;

			if (!WriteFile(hCom, toSend.data(), toSend.size(), &bytesWritten, 0))
			{
				notifyErrorCallback("Error sending message");
			}
		}
		writeMutex.unlock();

		std::this_thread::sleep_for(std::chrono::milliseconds(p2));

		std::vector<uint8_t> messageRead{};

		while (true)
		{
			uint8_t buf[265];
			DWORD read = 0;
			
			if (!ReadFile(hCom, buf, 265, &read, 0))
			{
				notifyErrorCallback("Error reading message");
				break;
			}

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
			notifyDataCallback(messageRead);
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(p3));
	}
}