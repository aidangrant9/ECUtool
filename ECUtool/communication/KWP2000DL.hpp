#pragma once

#include "Connection.hpp"
#include <thread>
#include <chrono>

using namespace serial;

class KWP2000DL : public Connection
{
public:
	enum class AddressingMode
	{
		Functional,
		Physical,
	};

	KWP2000DL(std::string &portName, uint32_t baudRate, bytesize_t byteSize, parity_t parity, stopbits_t stopBits, flowcontrol_t flowControl, bool echoCancellation,
		AddressingMode addressingMode, uint8_t sourceAddress, uint8_t targetAddress);
	~KWP2000DL() override;

	virtual void connect()    override;
	virtual void disconnect() override;

protected:
	// Port options
	std::string portName{};
	uint32_t baudRate{};
	bytesize_t byteSize{};
	parity_t parity{};
	stopbits_t stopBits{};
	bool echoCancellation{ true };

	// Initialisation options
	std::optional<AddressingMode> addressingMode{};
	std::optional<uint8_t>        sourceAddress{};
	std::optional<uint8_t>        targetAddress{};

	// Connection parameters
	std::optional<uint8_t> keyByte1{};
	std::optional<uint8_t> keyByte2{};

	// Serial port connection
	Serial connection;

	// Connection IO
	std::vector<std::vector<uint8_t>> sentMessages{};
	std::vector<uint8_t>              readMessages{};
	
	virtual std::string name() { return std::string{ "KWP2000DL" }; }
	virtual void bindToLua(sol::state &s) override;

	virtual bool hasValidChecksum(const std::vector<uint8_t> &data);

	// Blocking
	virtual void wakeUpPattern();
	virtual void sendFiveBaudAddress(uint8_t address);

	virtual void writeWithDelay(const std::vector<uint8_t> msg, const uint32_t msDelay);
	virtual std::vector<uint8_t> readFrameMatch(const uint32_t timeout, std::function<int(std::vector<uint8_t>)> matchingFn);
	virtual void write(const std::vector<uint8_t> msg);
	virtual std::vector<uint8_t> read();
};