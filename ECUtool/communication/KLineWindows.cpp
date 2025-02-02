#include "KLineWindows.hpp"

#include <iostream>
#include <chrono>
#include "VecStream.hpp"

KLineWindows::KLineWindows(std::string &portName, size_t baudRate, size_t byteSize, Parity parity, StopBits stopBits)
	: SerialConnection(portName, baudRate, byteSize, parity, stopBits)
{}

KLineWindows::KLineWindows(std::string &portName, size_t baudRate, size_t byteSize, Parity parity, StopBits stopBits, InitMode initMode, AddressingMode addressingMode,
	uint8_t sourceAddress, uint8_t targetAddress)
	: SerialConnection(portName, baudRate, byteSize, parity, stopBits), initMode { initMode }, addressingMode { addressingMode }, sourceAddress { sourceAddress }, targetAddress { targetAddress }
{}

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

	COMMTIMEOUTS readTimeouts = { 20, 0, 0, p4, 0 };

	// Configure port timeouts
	if (!SetCommTimeouts(hCom, &readTimeouts))
	{
		changeConnectionStatus(ConnectionStatus::Error, "Failed to set com timeouts");
		return;
	}


	if (!initMode.has_value())
	{
		return true;
	}
	else if (initMode.value() == InitMode::FastInit)
	{
		if (!setPortConfiguration(this->baudRate, this->byteSize, this->parity, this->stopBits))
		{
			changeConnectionStatus(ConnectionStatus::Error, "Failed to set port configuration");
			return false;
		}

		// Wake up pattern
		std::this_thread::sleep_for(std::chrono::milliseconds(300)); // T_idle
		EscapeCommFunction(hCom, SETBREAK);
		std::this_thread::sleep_for(std::chrono::milliseconds(25)); // T_inil
		EscapeCommFunction(hCom, CLRBREAK);
		std::this_thread::sleep_for(std::chrono::milliseconds(25)); // T_wup - T_inil

		// Send StartCommunicationMessage

		if (!(addressingMode.has_value() && sourceAddress.has_value() && targetAddress.has_value()))
		{
			changeConnectionStatus(ConnectionStatus::Error, "StartCommunication addressing information undefined");
			return false;
		}

		uint8_t fmt{};

		if (addressingMode.value() == AddressingMode::Functional)
		{
			fmt = 0xC1;
		}
		else
		{
			fmt = 0x81;
		}

		uint8_t tgt = targetAddress.value();
		uint8_t src = sourceAddress.value();
		uint8_t cs{ fmt + tgt + src + 0x81 };

		std::vector<uint8_t> startCommunicationRequest{ fmt, tgt, src, 0x81, cs };
		
		DWORD bytesWritten = 0;

		if (!WriteFile(hCom, startCommunicationRequest.data(), startCommunicationRequest.size(), &bytesWritten, 0))
		{
			changeConnectionStatus(ConnectionStatus::Error, "Error sending StartCommunicationRequest");
			return false;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(p2));

		std::vector<uint8_t> messageRead{};
		uint8_t buf[265];
		DWORD read = 0;

		if (!ReadFile(hCom, buf, 265, &read, 0))
		{
			changeConnectionStatus(ConnectionStatus::Error, "Error reading StartCommunicationResponse");
			return false;
		}

		if (read)
		{
			for (int i = 0; i < read; i++)
			{
				messageRead.push_back(buf[i]);
			}
		}

		VecStream stream(messageRead);

		try
		{
			uint8_t format = stream.next();
			uint8_t mode = format >> 6;
			uint8_t len = format & 0b00111111;

			if (mode > 1)
			{
				uint8_t r_tgt = stream.next();
				uint8_t r_src = stream.next();

				// Check addresses are correct
				if (r_tgt != src || r_src != tgt)
				{
					changeConnectionStatus(ConnectionStatus::Error, "Wrong addresses in StartCommunicationResponse");
					return false;
				}
			}

			// Parse additional byte
			if (len == 0)
			{
				len = stream.next();
			}

			// Check response identifier
			if (stream.next() != 0xC1)
			{
				changeConnectionStatus(ConnectionStatus::Error, "Negative response in StartCommunicationResponse");
				return false;
			}

			this->keyByte1 = stream.next();
			this->keyByte2 = stream.next();

			uint8_t r_cs{};
			for (int i = 0; i < messageRead.size() - 1; i++)
			{
				r_cs += messageRead[i];
			}

			if (stream.next() != r_cs)
			{
				changeConnectionStatus(ConnectionStatus::Error, "Checksum error on StartCommunicationResponse");
				return false;
			}
		}
		catch (std::out_of_range e)
		{
			changeConnectionStatus(ConnectionStatus::Error, "Error parsing StartCommunicationResponse");
			return false;
		}

	}
	else if (initMode.value() == InitMode::FiveBaud)
	{
		if (!setPortConfiguration(5, 8, Parity::None, StopBits::OneStopBit))
		{
			changeConnectionStatus(ConnectionStatus::Error, "Failed to set port configuration for Five Baud initialisation");
			return false;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(300)); // w5

		if (!(addressingMode.has_value() && sourceAddress.has_value() && targetAddress.has_value()))
		{
			changeConnectionStatus(ConnectionStatus::Error, "StartCommunication addressing information undefined");
			return false;
		}

		uint8_t address = targetAddress.value();

		DWORD bytesWritten{ 0 };

		if (!WriteFile(hCom, &address, 1, &bytesWritten, 0))
		{
			changeConnectionStatus(ConnectionStatus::Error, "Error sending address byte");
			return false;
		}

		if (!setPortConfiguration(this->baudRate, this->byteSize, this->parity, this->stopBits))
		{
			changeConnectionStatus(ConnectionStatus::Error, "Failed to set port configuration for Five Baud initialisation");
			return false;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(60));

		uint8_t synchByte{};
		DWORD read = 0;

		if (!ReadFile(hCom, &synchByte, 1, &read, 0))
		{
			changeConnectionStatus(ConnectionStatus::Error, "Error reading Synch Byte");
			return false;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(5));

		uint8_t keyBytes[2];

		if (!ReadFile(hCom, keyBytes, 2, &read, 0))
		{
			changeConnectionStatus(ConnectionStatus::Error, "Error reading KeyBytes");
			return false;
		}

		keyByte1 = keyBytes[0];
		keyByte2 = keyBytes[1];

		std::this_thread::sleep_for(std::chrono::milliseconds(25));

		uint8_t inversion = ~keyBytes[1];

		if (!WriteFile(hCom, &inversion, 1, &bytesWritten, 0))
		{
			changeConnectionStatus(ConnectionStatus::Error, "Error sending KeyByte inversion");
			return false;
		}


		std::this_thread::sleep_for(std::chrono::milliseconds(25));

		uint8_t addrInversion {};

		if (!ReadFile(hCom, &addrInversion, 1, &read, 0))
		{
			changeConnectionStatus(ConnectionStatus::Error, "Error reading address inversion");
			return false;
		}

		if (address != ~addrInversion)
		{
			changeConnectionStatus(ConnectionStatus::Error, "Address inversion doesn't match address");
			return false;
		}


		std::this_thread::sleep_for(std::chrono::milliseconds(p3));
	}
	else
	{
		changeConnectionStatus(ConnectionStatus::Error, "Initialisation mode not implemented");
		return false;
	}

	//Check keybytes if they have been set
	if (keyByte1.has_value() && keyByte2.has_value())
	{
		// check parity bit
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

	COMMTIMEOUTS readTimeouts = { 20, 0, 0, p4, 0 };

	// Configure port timeouts
	if (!SetCommTimeouts(hCom, &readTimeouts))
	{
		changeConnectionStatus(ConnectionStatus::Error, "Failed to set com timeouts");
		return;
	}

	// Set requested port configuration
	if (!setPortConfiguration(this->baudRate, this->byteSize, this->parity, this->stopBits))
	{
		changeConnectionStatus(ConnectionStatus::Error, "Failed to set port configuration");
		return;
	}

	PurgeComm(hCom, PURGE_RXCLEAR | PURGE_TXCLEAR);	

	// Set connection status to connected
	changeConnectionStatus(ConnectionStatus::Connected);

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