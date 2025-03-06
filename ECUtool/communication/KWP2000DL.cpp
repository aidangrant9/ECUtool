#include "KWP2000DL.hpp"
#include <thread>
#include "VecStream.hpp"
#include <QApplication>


using namespace std;

KWP2000DL::KWP2000DL(std::string &portName, uint32_t baudRate, bytesize_t byteSize, parity_t parity, stopbits_t stopBits, flowcontrol_t flowControl, bool echoCancellation,
	InitMode initMode, AddressingMode addressingMode, uint8_t sourceAddress, uint8_t targetAddress)
	: connection{Serial("", baudRate, Timeout(), byteSize, parity, stopBits, flowControl)},
	portName{ portName }, baudRate{ baudRate }, byteSize{ byteSize }, parity{ parity }, stopBits{ stopBits }, echoCancellation {echoCancellation},
	initMode{ initMode }, addressingMode{ addressingMode }, sourceAddress{ sourceAddress }, targetAddress{ targetAddress }
{}

KWP2000DL::~KWP2000DL()
{
	disconnect();
}

void KWP2000DL::connect()
{
	if (connection.isOpen())
	{
		try
		{
			connection.close();
		}
		catch (...)
		{
			notifyMessageCallback(Message{ Message::MessageType::Error, "Please start a new connection", name() });
			return;
		}
	}

	try
	{
		connection.setPort(portName);
		connection.open();
	}
	catch (...)
	{
		notifyMessageCallback(Message{ Message::MessageType::Error, "Failed to open port", name() });
		return;
	}

	writeQueue.clear(); // Clear anything in the Queue
	workThread = std::jthread(&KWP2000DL::workStart, this); // Start communication
}

void KWP2000DL::disconnect()
{
	if (connectionStatus == ConnectionStatus::Connected)
	{
		workThread.request_stop();
		workThread.join();
		if (connection.isOpen())
		{
			connection.close();
		}
	}

	changeConnectionStatus(ConnectionStatus::Disconnected);
}

void KWP2000DL::workStart()
{
	try
	{
		poll();
	}
	catch (...)
	{
		changeConnectionStatus(ConnectionStatus::Disconnected, "Connection Interrupted");
		return;
	}
}

void KWP2000DL::bindToLua(sol::state &s)
{
	s.new_usertype<KWP2000DL>("connection",
		"name", &KWP2000DL::name,
		"write", &KWP2000DL::writeVec,
		"readOrTimeout", &KWP2000DL::readOrTimeout,
		"busyLoop", [this](uint32_t ms){busyLoop(std::chrono::milliseconds(ms));},
		"keyByte", [this]() {return keyByte1.value_or(-1);},
		"functionalAddressing", [this]() {return addressingMode == AddressingMode::Functional;},
		"sourceAddress", [this]() {return sourceAddress.value_or(0);},
		"targetAddress", [this]() {return targetAddress.value_or(0);});

	s["connection"] = this;
}

#ifndef NOT_STRICT_TIMING
void KWP2000DL::poll()
{
	std::stop_token st = workThread.get_stop_token();

	// Initialise the connection
	if (!initialise())
	{
		changeConnectionStatus(ConnectionStatus::Disconnected, "Failed to initialise connection");
		return;
	}

	connection.flush();

	deque<DataMessage<uint8_t>> toSend{};

	changeConnectionStatus(ConnectionStatus::Connected);

	while (true) // Polling loop
	{
		if (st.stop_requested()) // Todo: can add StopConnection?
		{
			return;
		}
		else if (!connection.isOpen())
		{
			changeConnectionStatus(ConnectionStatus::Disconnected, "Connection Interrupted");
			return;
		}

		writeMutex.lock();
		if (!writeQueue.empty())
		{
			toSend.push_back(writeQueue.front());
			writeQueue.pop_front();
		}
		writeMutex.unlock();

		bool didWrite = false;

		if (!toSend.empty())
		{
			DataMessage messageToSend = toSend.front();
			connection.flush();

			if (!writeWithInnerByteDelay(messageToSend.data, timingParams.P4_MIN_DEFAULT))
			{
				notifyMessageCallback(Message{ Message::MessageType::Error, format("Failed to send message <{:d}>", messageToSend.id), name()});
			}
			else
			{
				didWrite = true;
				toSend.pop_front(); // Remove message from send queue
			}
			if (echoCancellation)
			{
				Timeout t = Timeout(timingParams.P4_MIN_DEFAULT, timingParams.P4_MAX_DEFAULT, 0, 0, 0);
				connection.setTimeout(t);
				vector<uint8_t> echo{};
				if (connection.read(echo, messageToSend.data.size()) != messageToSend.data.size()) // Try to read echo
				{
					notifyMessageCallback(Message{ Message::MessageType::Error, format("Failed to read message <{:d}> echo", messageToSend.id), name() });
					connection.flush();
				}
				else if (echo != messageToSend.data)
				{
					notifyMessageCallback(Message{ Message::MessageType::Error, format("Message <{:d}> echo mismatch", messageToSend.id), name() });
					connection.flush();
				}
				else 
				{
					notifyDataSentCallback(messageToSend);
				}
			}
			else
			{
				notifyDataSentCallback(messageToSend);
			}
		}

		if (didWrite)
		{
			Timeout t = Timeout(timingParams.P1_MAX_DEFAULT, timingParams.P2_MAX_DEFAULT, 0, 0, 0);
			connection.setTimeout(t);
		}
		else
		{
			Timeout t = Timeout(timingParams.P1_MAX_DEFAULT, timingParams.P1_MAX_DEFAULT, 0, 0, 0);
			connection.setTimeout(t);
		}

		bool doRead = true;
		bool didRead = false;
		while (doRead)
		{
			vector<uint8_t> msgTotal{};

			if (connection.read(msgTotal, 260) > 0)
			{
				Timeout t = Timeout(timingParams.P1_MAX_DEFAULT, timingParams.P2_MAX_DEFAULT, 0, 0, 0);
				connection.setTimeout(t);
				didRead = true;
				notifyDataRecieveCallback(DataMessage<uint8_t>{msgTotal});
			}
			else
			{
				doRead = false;
			}
		}

		if (didRead)
		{
			busyLoop(chrono::milliseconds(timingParams.P3_MIN_DEFAULT - timingParams.P2_MAX_DEFAULT));
		}
	}
}
#endif

#ifdef NOT_STRICT_TIMING
void KWP2000DL::poll()
{
	std::stop_token st = workThread.get_stop_token();

	// Initialise the connection
	if (!initialise())
	{
		changeConnectionStatus(ConnectionStatus::Disconnected, "Failed to initialise connection");
		return;
	}

	connection.flush();

	deque<DataMessage<uint8_t>> toSend{};
	deque<DataMessage<uint8_t>> sent{};

	changeConnectionStatus(ConnectionStatus::Connected);

	Timeout t = Timeout(timingParams.P1_MAX_DEFAULT, timingParams.P1_MAX_DEFAULT, 0, 0, 0);
	connection.setTimeout(t);

	while (true) // Polling loop
	{
		if (st.stop_requested()) // Todo: can add StopConnection?
		{
			return;
		}
		else if (!connection.isOpen())
		{
			changeConnectionStatus(ConnectionStatus::Disconnected, "Connection Interrupted");
			return;
		}

		writeMutex.lock();
		if (!writeQueue.empty())
		{
			toSend.push_back(writeQueue.front());
			writeQueue.pop_front();
		}
		writeMutex.unlock();


		if (!toSend.empty())
		{
			DataMessage messageToSend = toSend.front();
			connection.flush();

			//if (connection.write(messageToSend.data) != messageToSend.data.size())
			if (!writeWithInnerByteDelay(messageToSend.data, timingParams.P2_MIN_DEFAULT))
			{
				notifyMessageCallback(Message{ Message::MessageType::Error, format("Failed to send message <{:d}>", messageToSend.id), name() });
			}
			else
			{
				if (echoCancellation)
				{
					sent.push_front(messageToSend);
				}
				else
				{
					notifyDataSentCallback(messageToSend);
				}
			}
			toSend.pop_front();
		}
		
		std::vector<uint8_t> read{};

		if (connection.read(read, 260) > 0)
		{
			if (echoCancellation)
			{
				if (!sent.empty() && read == sent.front().data)
				{
					notifyDataSentCallback(sent.front());
					sent.pop_front();
				}
				else
				{
					notifyDataRecieveCallback(read);
				}
			}
			else
			{
				notifyDataRecieveCallback(read);
			}
		}

	}
}
#endif

bool KWP2000DL::initialise()
{
	if (initMode == InitMode::FiveBaud)
	{
		if (!fiveBaudInit())
			return false;
	}
	else if (initMode == InitMode::FastInit)
	{
		if (!fastInit())
			return false;
	}

	if (initMode != InitMode::None) // Should successfully have Key Byte 1
	{
		configureBasedOnKeyByte();
	}

	// Post initialisation setup
	connection.setBaudrate(baudRate);
	connection.setParity(parity);
	connection.setBytesize(byteSize);
	connection.setStopbits(stopBits);

	return true;
}

bool KWP2000DL::fiveBaudInit()
{
#define W1_MIN 60
#define W1_MAX 300
#define W2_MIN 5
#define W2_MAX 20
#define W3_MIN 0
#define W3_MAX 20
#define W4_MIN 25
#define W4_MAX 50
#define W5_MIN 300
#define EXPECTED_KW2 0x8F

	// Configure port
	try 
	{
		connection.setBaudrate(baudRate);
		connection.setBytesize(bytesize_t::eightbits);
		connection.setStopbits(stopbits_t::stopbits_one);
		connection.setParity(parity_t::parity_none);
		connection.setFlowcontrol(flowcontrol_t::flowcontrol_none);
	}
	catch (...)
	{
		notifyMessageCallback(Message{ Message::MessageType::Error, "Failed to configure port for 5-Baud Init", name() });
		return false;
	}

	this_thread::sleep_for(chrono::milliseconds(W5_MIN)); // IDLE

	uint8_t address = targetAddress.value_or(0x33);
	uint8_t addressParity = 0x0;

	if (addressingMode.value() == AddressingMode::Physical)
	{
		for (int i = 0; i < 7; i++)
			addressParity ^= ((address >> i) & 0b1);

		if (!addressParity)
			address |= 0b10000000; // Add bit to make odd parity
	}

	chrono::time_point t1 = chrono::steady_clock::now();
	connection.setBreak(true);
	this_thread::sleep_for(chrono::milliseconds(200)); // Start bit

	// Send address byte LSB first
	for (int i = 0; i < 8; i++)
	{
		int deviation = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - t1).count() - (200 + (200 * (i)));
		uint8_t toSend = (address >> i) & 0b1;
		qDebug() << toSend;
		if (toSend)
			connection.setBreak(false);
		else
			connection.setBreak(true);
		this_thread::sleep_for(chrono::milliseconds(200 - deviation));
	}

	int deviation = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - t1).count() - 1800;
	// Send stop bit
	connection.setBreak(false);
	this_thread::sleep_for(chrono::milliseconds(200 - deviation));

	chrono::time_point t2 = chrono::steady_clock::now();
	notifyMessageCallback(Message{ Message::MessageType::Info, format("Send took +{:d}ms", chrono::duration_cast<chrono::milliseconds>(t2 - t1).count() - 2000), name() });
	notifyMessageCallback(Message{ Message::MessageType::Info, format("Send within +{:.2f}%", 100*(chrono::duration_cast<chrono::milliseconds>(t2 - t1).count()-2000)/2000.f), name() });

	// Clear line
	connection.flush();
	Timeout t = Timeout{ timingParams.P1_MAX_DEFAULT, W1_MAX, 0, 0, 0 };
	connection.setTimeout(t);

	vector<uint8_t> syncByte{};

	if (connection.read(syncByte, 1) != 1) // Try to read sync byte
	{
		notifyMessageCallback(Message{ Message::MessageType::Error, "Failed to recieve sync byte", name()});
		return false;
	}
	else if (syncByte[0] != 0x55)
	{
		notifyMessageCallback(Message{ Message::MessageType::Error, format("Expected sync byte 0x55 but got 0x{:x}, is baud rate correct?", syncByte[0]), name() });
		return false;
	}

	t = Timeout{ timingParams.P1_MAX_DEFAULT, W2_MAX, 0, 0, 0 };
	connection.setTimeout(t);

	vector<uint8_t> kw1{};

	if (connection.read(kw1, 1) != 1) // Try to read Key Byte 1
	{
		notifyMessageCallback(Message{ Message::MessageType::Error, "Failed to recieve Key Byte 1", name() });
		return false;
	}
	else
	{
		keyByte1 = kw1[0];
	}

	t = Timeout{ timingParams.P1_MAX_DEFAULT, W3_MAX, 0, 0, 0 };
	connection.setTimeout(t);

	vector<uint8_t> kw2{};

	if (connection.read(kw2, 1) != 1) // Try to read Key Byte 2
	{
		notifyMessageCallback(Message{ Message::MessageType::Error, "Failed to recieve Key Byte 2", name() });
		return false;
	}
	else if (kw2[0] != EXPECTED_KW2)
	{
		notifyMessageCallback(Message{ Message::MessageType::Error, format("Expected Key Byte 2 0x{:x} but got 0x{:x}", EXPECTED_KW2, kw2[0]), name() });
		return false;
	}
	else
	{
		keyByte2 = kw2[0];
	}

	this_thread::sleep_for(chrono::milliseconds(W4_MIN));
	if (connection.write(vector<uint8_t>{static_cast<uint8_t>(~kw2[0])}) != 1) // Send Key Byte 2 inversion
	{
		notifyMessageCallback(Message{ Message::MessageType::Error, "Failed to send Key Byte 2 Inversion", name() });
		return false;
	}
	else
	{
		if (echoCancellation)
		{
			vector<uint8_t> sentByte{};
			if (connection.read(sentByte, 1) != 1)
			{
				notifyMessageCallback(Message{ Message::MessageType::Error, "Failed to read Key Byte 2 Inversion echo", name() });
				return false;
			}
			else if (sentByte[0] != ~kw2[0])
			{
				notifyMessageCallback(Message{ Message::MessageType::Error, format("Expected Key Byte 2 Inversion echo 0x{:x} but got 0x{:x}", ~kw2[0], sentByte[0]), name() });
				// Probably shouldnt exit here
			}
		}
	}

	t = Timeout{ timingParams.P1_MAX_DEFAULT, W4_MAX, 0, 0, 0 };
	connection.setTimeout(t);
	
	vector<uint8_t> invertedAddress{};

	if (connection.read(invertedAddress, 1) != 1) // Try to read inverted address
	{
		notifyMessageCallback(Message{ Message::MessageType::Error, "Failed to read inverted address byte", name() });
		return false;
	}
	else if (invertedAddress[0] != ~address)
	{
		notifyMessageCallback(Message{ Message::MessageType::Error, format("Expected inverted address byte 0x{:x} but got 0x{:x}", ~address, invertedAddress[0]), name()});
		return false;
	}

	notifyMessageCallback(Message{ Message::MessageType::Info, format("5-Baud Init successful, got Key Byte 0x{:x}", keyByte1.value()), name() });
	return true;
}

bool KWP2000DL::fastInit()
{
	if (baudRate != 10400)
	{
		notifyMessageCallback(Message{ Message::MessageType::Info, "Baud rate not in spec for fast init!" , name() });
	}

	this_thread::sleep_for(chrono::milliseconds(W5_MIN)); // Idle for reconnect

	chrono::time_point t1 = chrono::steady_clock::now();
	// Send wake-up pattern
	connection.setBreak(true);
	busyLoop(chrono::milliseconds(25));
	connection.setBreak(false);
	busyLoop(chrono::milliseconds(25));

	notifyMessageCallback(Message{ Message::MessageType::Info, format("WUP took {:d}ms", chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - t1).count()), name() });

	if (!(addressingMode.has_value() && sourceAddress.has_value() && targetAddress.has_value()))
	{
		notifyMessageCallback(Message{ Message::MessageType::Error, "Addressing information not available", name() });
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
	uint8_t cs{};
	cs += fmt + tgt + src + 0x81;

	vector<uint8_t> startCommunicationRequest{ fmt, tgt, src, 0x81, cs };

	connection.flush(); // flush buffer
	
	if (!writeWithInnerByteDelay(startCommunicationRequest, timingParams.P4_MIN_DEFAULT))
	{
		notifyMessageCallback(Message{ Message::MessageType::Error, "Failed to send StartCommunication request", name() });
		return false;
	}
	
	Timeout t = Timeout(timingParams.P4_MIN_DEFAULT, timingParams.P2_MAX_DEFAULT, 0, 0, 0);
	connection.setTimeout(t);

	vector<uint8_t> startCommunicationRequestEcho{};

	if (echoCancellation)
	{
		if (!connection.read(startCommunicationRequestEcho, 5)) // Read echo if possible
		{
			notifyMessageCallback(Message{ Message::MessageType::Error, "Failed to read StartCommunication echo", name() });
			return false;
		}
		else if (startCommunicationRequest != startCommunicationRequestEcho)
		{
			notifyMessageCallback(Message{ Message::MessageType::Error, "StartCommunication Request echo mismatch", name() });
			return false;
		}
	}

	t = Timeout(timingParams.P1_MAX_DEFAULT, timingParams.P2_MAX_DEFAULT, 0, 0, 0);
	connection.setTimeout(t);

	vector<uint8_t> startCommunicationResponse{};

	if (!connection.read(startCommunicationResponse, 7))
	{
		notifyMessageCallback(Message{ Message::MessageType::Error, "Failed to read StartCommunicationResponse", name() });
		return false;
	}
	
	if (startCommunicationResponse[1] != sourceAddress.value() || startCommunicationResponse[2] != targetAddress.value())
	{
		notifyMessageCallback(Message{ Message::MessageType::Error, format("Expected target address 0x{:x} and source address 0x{:x} but got target address 0x{:x} and source address 0x{:x}",
			sourceAddress.value(), targetAddress.value(), startCommunicationResponse[1], startCommunicationResponse[2]), name()});
		return false;
	}
	else if (startCommunicationResponse[3] != 0xC1)
	{
		notifyMessageCallback(Message{ Message::MessageType::Error, format("Expected StartCommunication Response service ID 0xc1, got 0x{:x}",
			startCommunicationResponse[3]), name()});
		return false;
	}
	else if (startCommunicationResponse[5] != EXPECTED_KW2)
	{
		notifyMessageCallback(Message{ Message::MessageType::Error, format("Expected Key Byte 2 0x{:x} but got 0x{:x}", EXPECTED_KW2, startCommunicationResponse[5]), name()});
		return false;
	}
	else if (!hasValidChecksum(startCommunicationResponse))
	{
		notifyMessageCallback(Message{ Message::MessageType::Error, "StartCommunicationResponse has invalid checksum", name() });
		return false;
	}

	keyByte1 = startCommunicationResponse[4];

	notifyMessageCallback(Message{ Message::MessageType::Info, format("Fast Init successful, got Key Byte 0x{:x}", keyByte1.value()), name() });
	return true;
}

bool KWP2000DL::writeWithInnerByteDelay(const std::vector<uint8_t> &data, uint32_t delay)
{
	chrono::time_point t1 = chrono::steady_clock::now();
	for (int i = 0; i < data.size() - 1; i++)
	{
		if (connection.write(&data[i], 1) != 1)
			return false;
		if(delay > 0)
			busyLoop(chrono::milliseconds(delay));
	}
	if (connection.write(&data[data.size() - 1], 1) != 1)
		return false;
	chrono::time_point t2 = chrono::steady_clock::now();
	int ms = chrono::duration_cast<chrono::milliseconds>(t2 - t1).count();
	notifyMessageCallback(Message{ Message::MessageType::Info, format("sent in {:d}ms", ms), name() });
	return true;
}

bool KWP2000DL::hasValidChecksum(const std::vector<uint8_t> &data)
{
	uint8_t cs = 0x0;
	for (int i = 0; i < data.size() - 1; i++)
	{
		cs += data[i];
	}
	return cs == data[data.size() - 1];
}

void KWP2000DL::configureBasedOnKeyByte()
{
	if ((keyByte1.value() & 0b00010000) != 0x0)
	{ // Extended timing set
		timingParams.P1_MIN_DEFAULT = 0;
		timingParams.P1_MIN_LOWER_LIMIT = 0;
		timingParams.P1_MAX_DEFAULT = 20;
		timingParams.P1_MAX_UPPER_LIMIT = 20;
		timingParams.P2_MIN_DEFAULT = 0;
		timingParams.P2_MIN_LOWER_LIMIT = 0;
		timingParams.P2_MAX_DEFAULT = 1000;
		timingParams.P2_MAX_UPPER_LIMIT = 1000;
		timingParams.P3_MIN_DEFAULT = 0;
		timingParams.P3_MIN_LOWER_LIMIT = 0;
		timingParams.P3_MAX_DEFAULT = 5000;
		timingParams.P3_MAX_UPPER_LIMIT = UINT32_MAX;
		timingParams.P4_MIN_DEFAULT = 5;
		timingParams.P4_MIN_LOWER_LIMIT = 0;
		timingParams.P4_MAX_DEFAULT = 20;
		timingParams.P4_MAX_UPPER_LIMIT = 20;
	}
	else
	{ // Normal timing set
		timingParams.P1_MIN_DEFAULT = 0 ;
		timingParams.P1_MIN_LOWER_LIMIT = 0 ;
		timingParams.P1_MAX_DEFAULT = 20;
		timingParams.P1_MAX_UPPER_LIMIT = 20;
		timingParams.P2_MIN_DEFAULT = 25;
		timingParams.P2_MIN_LOWER_LIMIT = 0;
		timingParams.P2_MAX_DEFAULT = 50;
		timingParams.P2_MAX_UPPER_LIMIT = 50;
		timingParams.P3_MIN_DEFAULT = 55;
		timingParams.P3_MIN_LOWER_LIMIT = 0;
		timingParams.P3_MAX_DEFAULT = 5000;
		timingParams.P3_MAX_UPPER_LIMIT = UINT32_MAX;
		timingParams.P4_MIN_DEFAULT = 5;
		timingParams.P4_MIN_LOWER_LIMIT = 0;
		timingParams.P4_MAX_DEFAULT = 20;
		timingParams.P4_MAX_UPPER_LIMIT = 20;
	}
}
