#include "KWP2000GRC.hpp"

KWP2000GRC::KWP2000GRC(std::string &portName, uint32_t baudRate, bytesize_t byteSize, parity_t parity, stopbits_t stopBits, flowcontrol_t flowControl, bool echoCancellation,
	uint8_t sourceAddress, uint8_t targetAddress)
	: connection{ Serial("", baudRate, Timeout(), byteSize, parity, stopBits, flowControl) },
	portName{ portName }, baudRate{ baudRate }, byteSize{ byteSize }, parity{ parity }, stopBits{ stopBits }, echoCancellation{ echoCancellation },
	sourceAddress{ sourceAddress }, targetAddress{ targetAddress }
{}

KWP2000GRC::~KWP2000GRC()
{
	disconnect();
}

void KWP2000GRC::connect()
{
	connection.setPort(portName);
	try
	{
		connection.open();
		changeConnectionStatus(Connection::ConnectionStatus::Connected);
	}
	catch (...)
	{
		notifyMessageCallback(Message{ Message::MessageType::Error, "Failed to open port", name() });
	}
}

void KWP2000GRC::disconnect()
{
	if (connection.isOpen())
	{
		try
		{
			connection.close();
			changeConnectionStatus(Connection::ConnectionStatus::Disconnected);
		}
		catch(...)
		{
			notifyMessageCallback(Message{ Message::MessageType::Error, "Failed to close connection", name() });
		}
	}
}

bool KWP2000GRC::write(const std::vector<uint8_t> msg, const uint32_t msDelay)
{
	DataMessage<uint8_t> datamsg = { msg };

	if (!connection.isOpen())
		return false;

	try
	{
		for (uint8_t m : datamsg.data)
		{
			busyLoop(std::chrono::milliseconds(msDelay));
			if (connection.write(&m, 1) != 1)
			{
				notifyMessageCallback(Message{ Message::MessageType::Error, std::format("Failed to write message {:d}", datamsg.id), name() });
				return false;
			}
		}
		if(echoCancellation)
			sent.push_back(datamsg);
		notifyDataSentCallback(datamsg);
	}
	catch (PortNotOpenedException e)
	{
		changeConnectionStatus(Connection::ConnectionStatus::Disconnected, "Port not open during write");
	}
	catch (...)
	{
		notifyMessageCallback(Message{ Message::MessageType::Error, std::format("Failed to write message {:d}", datamsg.id), name() });
		return false;
	}
	return true;
}

std::vector<uint8_t> KWP2000GRC::read()
{
	if (!connection.isOpen())
		return {};

	Timeout t = Timeout(0, 2000, 0, 0, 0);
	connection.setTimeout(t);

	std::vector<uint8_t> read{};
	try
	{
		connection.read(read, 1024);
	}
	catch (PortNotOpenedException e)
	{
		changeConnectionStatus(Connection::ConnectionStatus::Disconnected, "Port not open during read");
	}
	catch (...)
	{
		notifyMessageCallback(Message{ Message::MessageType::Error, "Failed to read message", name() });
	}

	if (echoCancellation && !sent.empty())
	{
		auto toCancel = sent.front();
		
		try 
		{
			for (int i = 0; i < read.size(); i++)
			{
				if (read[i] == toCancel.data[0])
				{
					bool matches = true;
					for (int j = 0; j < toCancel.data.size(); j++)
					{
						if (toCancel.data[j] != read[i + j])
						{
							matches = false;
							break;
						}
					}

					if (matches)
					{
						read.erase(read.begin(), read.begin() + i + toCancel.data.size());
						sent.pop_front();
						break;
					}
				}
			}
		}
		catch (...)
		{}
	}

	// Remove leading zero bytes
	while (!read.empty() && read[0] == 0)
	{
		read.erase(read.begin());
	}

	return read;
}

void KWP2000GRC::bindToLua(sol::state &s)
{
	s.new_usertype<KWP2000GRC>("connection",
		"name", &KWP2000GRC::name,
		"write", &KWP2000GRC::write,
		"read", &KWP2000GRC::read,
		"busyLoop", [this](uint32_t ms) {busyLoop(std::chrono::milliseconds(ms));},
		"sourceAddress", [this]() {return sourceAddress;},
		"targetAddress", [this]() {return targetAddress;});

	s["connection"] = this;
}

void KWP2000GRC::wakeUpPattern()
{
}

bool KWP2000GRC::sendFiveBaudAddress(uint8_t address, bool functional)
{
	return false;
}

bool KWP2000GRC::writeWithInnerByteDelay(const std::vector<uint8_t> &data, uint32_t delay)
{
	return false;
}

