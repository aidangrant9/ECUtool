#pragma once

#include "Connection.hpp"
#pragma once

#include <thread>
#include <chrono>

using namespace serial;

class KWP2000GRC : public Connection
{
public:
	KWP2000GRC(std::string &portName, uint32_t baudRate, bytesize_t byteSize, parity_t parity, stopbits_t stopBits, flowcontrol_t flowControl, bool echoCancellation,
		uint8_t sourceAddress, uint8_t targetAddress);
	~KWP2000GRC() override;

	virtual void connect()    override;
	virtual void disconnect() override;

	virtual bool write(const std::vector<uint8_t> msg, const uint32_t msDelay);
	virtual std::vector<uint8_t> read();

protected:
	// Port options
	std::string portName{};
	uint32_t baudRate{};
	bytesize_t byteSize{};
	parity_t parity{};
	stopbits_t stopBits{};
	bool echoCancellation{ true };

	uint8_t targetAddress{ 0 };
	uint8_t sourceAddress{ 0 };

	// Queue for echo cancellation
	std::deque<DataMessage<uint8_t>> sent{};

	// Serial port connection
	Serial connection;

	virtual std::string name() { return std::string{ "KWP2000GRC" }; }
	virtual void bindToLua(sol::state &s) override;

	virtual void wakeUpPattern();
	virtual bool sendFiveBaudAddress(uint8_t address, bool functional);
	virtual bool writeWithInnerByteDelay(const std::vector<uint8_t> &data, uint32_t delay);
};