#include "KWP2000DL.hpp"
#include <thread>
#include "VecStream.hpp"
#include <algorithm>

using namespace std;

KLine::KLine(std::string &portName, uint32_t baudRate, bytesize_t byteSize, parity_t parity, stopbits_t stopBits, flowcontrol_t flowControl, bool echoCancellation,
	AddressingMode addressingMode, uint8_t sourceAddress, uint8_t targetAddress)
	: connection{Serial("", baudRate, Timeout(), byteSize, parity, stopBits, flowControl)},
	portName{ portName }, baudRate{ baudRate }, byteSize{ byteSize }, parity{ parity }, stopBits{ stopBits }, echoCancellation {echoCancellation},
	addressingMode{ addressingMode }, sourceAddress{ sourceAddress }, targetAddress{ targetAddress }
{
}

KLine::~KLine()
{
	disconnect();
}

void KLine::connect()
{
	if (connection.isOpen())
	{
		try
		{
			connection.close();
		}
		catch (...)
		{
			logger.addErrorMessage(Message{ "Please start a new connection", name() });
			return;
		}
	}

	try
	{
		connection.setPort(portName);
		connection.open();
		changeConnectionStatus(ConnectionStatus::Connected);
	}
	catch (...)
	{
		logger.addErrorMessage(Message{ "Failed to open port", name() });
		return;
	}
}

void KLine::disconnect()
{
	if (connection.isOpen())
	{
		connection.close();
	}

	changeConnectionStatus(ConnectionStatus::Disconnected);
}

void KLine::bindToLua(sol::state &s)
{
	s.new_usertype<KLine>("connection",
		"name", &KLine::name,
		"write", &KLine::write,
		"writeWithDelay", &KLine::writeWithDelay,
		"read", &KLine::read,
		"readTimeout", &KLine::readWithTimeout,
		"readFrameMatch", &KLine::readFrameMatch,
		"sleep", [this](uint32_t ms){busyLoop(std::chrono::milliseconds(ms));},
		"keyByte", [this]() {return keyByte1.value_or(-1);},
		"functionalAddressing", [this]() {return addressingMode == AddressingMode::Functional;},
		"sourceAddress", [this]() {return sourceAddress;},
		"targetAddress", [this]() {return targetAddress;},
		"wakeUpPattern", &KLine::wakeUpPattern,
		"sendFiveBaudAddress", &KLine::sendFiveBaudAddress);

	s["connection"] = this;
}

std::string KLine::getStatusString()
{
	if (connectionStatus == ConnectionStatus::Disconnected)
		return name() + " | Disconnected";

	std::string am = addressingMode == AddressingMode::Physical ? "PHYSICAL" : "FUNCTIONAL";
	std::string opts{};
	
	switch (parity)
	{
	case parity_even:
		opts += "EVEN";
		break;
	case parity_odd:
		opts += "ODD";
		break;
	case parity_space:
		opts += "SPACE";
		break;
	case parity_mark:
		opts += "MARK";
		break;
	case parity_none:
		opts += "NONE";
		break;
	}

	switch (stopBits)
	{
	case stopbits_one:
		opts += " | SB 1";
		break;
	case stopbits_one_point_five:
		opts += " | SB 1.5";
		break;
	case stopbits_two:
		opts += " | SB 2";
		break;
	}

	std::ostringstream oss;
	oss << name()
		<< " | Connected | " << portName
		<< " | " << baudRate
		<< " | " << am
		<< " " << std::uppercase << std::hex << +sourceAddress
		<< " " << std::uppercase << std::hex << +targetAddress
		<< " | " << +byteSize << " BIT | " << opts;

	return oss.str();
}

bool KLine::hasValidChecksum(const std::vector<uint8_t> &data)
{
	uint8_t cs = 0x0;
	for (int i = 0; i < data.size() - 1; i++)
	{
		cs += data[i];
	}
	return cs == data[data.size() - 1];
}

void KLine::wakeUpPattern()
{
	if (!connection.isOpen())
		return

	connection.setBreak(true);
	busyLoop(std::chrono::milliseconds(25));
	connection.setBreak(false);
	busyLoop(std::chrono::milliseconds(25));
}

void KLine::sendFiveBaudAddress(uint8_t address)
{
	if (!connection.isOpen())
		return;

	uint8_t addressParity = 0x0;

	if (addressingMode != AddressingMode::Functional)
	{
		for (int i = 0; i < 7; i++)
			addressParity ^= ((address >> i) & 0b1);

		if (!addressParity)
			address |= 0b10000000; // Add bit to make odd parity
	}

	std::chrono::time_point t1 = std::chrono::steady_clock::now();
	connection.setBreak(true);
	std::this_thread::sleep_for(std::chrono::milliseconds(200)); // Start bit

	// Send address byte LSB first
	for (int i = 0; i < 8; i++)
	{
		int deviation = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t1).count() - (200 + (200 * (i)));
		uint8_t toSend = (address >> i) & 0b1;
		if (toSend)
			connection.setBreak(false);
		else
			connection.setBreak(true);
		std::this_thread::sleep_for(std::chrono::milliseconds(200 - deviation));
	}

	int deviation = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t1).count() - 1800;
	// Send stop bit
	connection.setBreak(false);
	std::this_thread::sleep_for(std::chrono::milliseconds(200 - deviation));
}

void KLine::writeWithDelay(const std::vector<uint8_t> msg, const uint32_t msDelay)
{
	if (!connection.isOpen())
		return;

	if (msg.size() < 1)
		return;

	// Reset connection state to await response
	readMessages.clear();
	sentMessages.clear();

	for (int i = 0; i < msg.size() - 1; i++)
	{
		connection.write(&msg[i], 1);
		busyLoop(std::chrono::milliseconds(msDelay));
	}
	
	connection.write(&msg[msg.size() - 1], 1);
	
	if(echoCancellation)
		sentMessages.push_back(msg);

	logWrite(msg);

	return;
}

std::vector<uint8_t> KLine::readFrameMatch(const uint32_t timeout, std::function<int(std::vector<uint8_t>)> matchingFn)
{
#define BYTE_TOLERANCE 3

	if (!connection.isOpen())
	{
		changeConnectionStatus(ConnectionStatus::Disconnected, "Connection closed");
		return {};
	}

	Timeout t = Timeout(0, 1, 0, 0, 0);
	connection.setTimeout(t);
	
	chrono::time_point<chrono::steady_clock> start = chrono::steady_clock::now();

	while (chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - start).count() < timeout)
	{
		connection.read(readMessages, 1024);

		// Run echo cancellation
		if (echoCancellation && !sentMessages.empty())
		{
			const std::vector<uint8_t> &toCancel = sentMessages.front();
			if (!toCancel.empty())
			{
				for (size_t i = 0; i < readMessages.size(); i++)
				{
					if (i + toCancel.size() <= readMessages.size() && readMessages[i] == toCancel[0])
					{
						bool matches = true;
						for (size_t j = 0; j < toCancel.size(); j++)
						{
							if (readMessages[i + j] != toCancel[j])
							{
								matches = false;
								break;
							}
						}

						if (matches)
						{
							readMessages.erase(readMessages.begin(), readMessages.begin() + i + toCancel.size());
							sentMessages.erase(sentMessages.begin());
							break;
						}
					}
				}
			}
		}

		// Run frame matching
		for (int i = 0; i <= std::min(readMessages.size(), static_cast<size_t>(BYTE_TOLERANCE)); i++)
		{
			int frameLen = matchingFn(std::vector(readMessages.begin() + i, readMessages.end()));
			if (frameLen > 0 && (i + static_cast<size_t>(frameLen)) <= readMessages.size())
			{
				if(i > 0)
					readMessages.erase(readMessages.begin(), readMessages.begin() + i);

				std::vector<uint8_t> ret = std::vector<uint8_t>(readMessages.begin(), readMessages.begin() + frameLen);
				readMessages.erase(readMessages.begin(), readMessages.begin() + frameLen);

				logRead(ret);
				return ret;
			}
		}
	}

	return {};
}

void KLine::write(const std::vector<uint8_t> msg)
{
	writeWithDelay(msg, 0);
}

std::vector<uint8_t> KLine::read()
{
	return readWithTimeout(2000);
}

std::vector<uint8_t> KLine::readWithTimeout(uint32_t timeout)
{
	if (!connection.isOpen())
	{
		changeConnectionStatus(ConnectionStatus::Disconnected, "Connection closed");
		return {};
	}


	std::vector<uint8_t> read{};
	Timeout t = Timeout(0, timeout, 0, 0, 0);
	connection.setTimeout(t);
	connection.read(read, 10000);

	if (echoCancellation && !sentMessages.empty())
	{
		std::vector<uint8_t> toCancel = sentMessages[0];

		try
		{
			for (int i = 0; i < read.size(); i++)
			{
				if (read[i] == toCancel[0])
				{
					bool matches = true;
					for (int j = 0; j < toCancel.size(); j++)
					{
						if (toCancel[j] != read[i + j])
						{
							matches = false;
							break;
						}
					}

					if (matches)
					{
						read.erase(read.begin(), read.begin() + i + toCancel.size());
						sentMessages.erase(sentMessages.begin() + 0);
						break;
					}
				}
			}
		}
		catch (...)
		{
		}
	}

	logRead(read);

	return read;
}
