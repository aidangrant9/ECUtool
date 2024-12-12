#pragma once

/*
	Base class for serial connections
*/

#include <functional>
#include <vector>
#include <cstdint>
#include <queue>
#include <mutex>
#include <iostream>

class SerialConnection
{
	using Callback = std::function<void(std::vector<uint8_t> *sc)>;
protected:
	std::vector<Callback> callbacks;
	std::deque<std::vector<uint8_t>> writeQueue;
	std::mutex writeMutex;
public:
	int connected = 0;

	virtual void registerCallback(Callback cb)
	{
		callbacks.push_back(cb);
	}

	virtual void doCallbacks(std::vector<uint8_t> &sc)
	{
		for (Callback cb : this->callbacks)
		{
			cb(&sc);
		}
	}

	virtual void writeMessage(std::vector<uint8_t> toWrite)
	{
		writeMutex.lock();
		writeQueue.push_front(toWrite);
		writeMutex.unlock();
	}

	virtual void connect() = 0;
	virtual void disconnect() = 0;
};

