#pragma once

#include "Connection.hpp"
#include <atomic>

using namespace serial;

class KWP2000DL : public Connection
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

	struct TimingParams
	{
		uint32_t P1_MIN_DEFAULT{ 0 };
		uint32_t P1_MIN_LOWER_LIMIT{ 0 };
		uint32_t P1_MAX_DEFAULT{ 20 };
		uint32_t P1_MAX_UPPER_LIMIT{ 20 };
		uint32_t P2_MIN_DEFAULT{ 25 };
		uint32_t P2_MIN_LOWER_LIMIT{ 0 };
		uint32_t P2_MAX_DEFAULT{ 50 };
		uint32_t P2_MAX_UPPER_LIMIT{ 50 };
		uint32_t P3_MIN_DEFAULT{ 55 };
		uint32_t P3_MIN_LOWER_LIMIT{ 0 };
		uint32_t P3_MAX_DEFAULT{ 5000 };
		uint32_t P3_MAX_UPPER_LIMIT{ UINT32_MAX };
		uint32_t P4_MIN_DEFAULT{ 5 };
		uint32_t P4_MIN_LOWER_LIMIT{ 0 };
		uint32_t P4_MAX_DEFAULT{ 20 };
		uint32_t P4_MAX_UPPER_LIMIT{ 20 };
	};

	KWP2000DL(std::string &portName, uint32_t baudRate, bytesize_t byteSize, parity_t parity, stopbits_t stopBits, flowcontrol_t flowControl, bool echoCancellation,
		InitMode initMode, AddressingMode addressingMode, uint8_t sourceAddress, uint8_t targetAddress);
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
	std::optional<InitMode>       initMode{};
	std::optional<AddressingMode> addressingMode{};
	std::optional<uint8_t>        sourceAddress{};
	std::optional<uint8_t>        targetAddress{};

	// Connection parameters
	std::optional<uint8_t> keyByte1{};
	std::optional<uint8_t> keyByte2{};

	TimingParams timingParams{};

	// Thread for async IO
	std::thread workThread;
	std::atomic<bool> shouldStop{ false };

	// Queue for echo cancellation
	std::deque<DataMessage<uint8_t>> sent{};

	// Serial port connection
	Serial connection;
	
	virtual std::string name() { return std::string{ "KWP2000DL" }; }
	virtual void startWork();
	virtual void poll();
	virtual bool initialise();

	virtual bool fiveBaudInit();
	virtual bool fastInit();
	virtual bool writeWithInnerByteDelay(const std::vector<uint8_t> &data, uint32_t delay);
	virtual bool hasValidChecksum(const std::vector<uint8_t> &data);
	virtual void configureBasedOnKeyByte();
};