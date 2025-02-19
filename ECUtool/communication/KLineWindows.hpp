#pragma once

#include "SerialConnection.hpp"

#include <string>
#include <thread>
#include <optional>
#include <Windows.h>


class KLine : public SerialConnection
{
public:
	enum class InitMode
	{
		CARB = 1,
		FiveBaud,
		FastInit,
		None,
	};

	enum class AddressingMode
	{
		Functional,
		Physical,
	};

protected:

	// Initialisation options
	std::optional<InitMode>       initMode       {};
	std::optional<AddressingMode> addressingMode {};
	std::optional<uint8_t>        sourceAddress  {};
	std::optional<uint8_t>        targetAddress  {};

	// Connection parameters

	std::optional<uint8_t> keyByte1 {};
	std::optional<uint8_t> keyByte2 {};

	size_t p1{20}; // Maxmimum time in between ECU response bytes
	size_t p2{25}; // Time between messages
	size_t p3{55}; // Time between ECU response and new request
	size_t p4{5};  // Inner byte time for requests

	// Thread for async IO
	std::jthread workThread;

	// Communications resource handle
	HANDLE hCom{ INVALID_HANDLE_VALUE };
	DCB dcb{};

	virtual std::string name() { return std::string{ "K-Line" }; }
	virtual void poll();
	virtual bool initialise();
	virtual bool setPortConfiguration(size_t baudRate, size_t byteSize, Parity parity, StopBits stopBits);
public:
	KLine(std::string &portName, size_t baudRate, size_t byteSize, Parity parity, StopBits stopBits);
	KLine(std::string &portName, size_t baudRate, size_t byteSize, Parity parity, StopBits stopBits, InitMode initMode, AddressingMode addressingMode,
		uint8_t sourceAddress, uint8_t targetAddress);
	~KLine() override;

	virtual void connect()    override;
	virtual void disconnect() override;
};

 