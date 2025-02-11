#pragma once

#include <cstdint>
#include <vector>
#include <mutex>


template <typename T>
class DataMessage
{
public:
	DataMessage(const std::vector<T> &data) : data(data), id(genID())
	{}

	const std::vector<T> data;
	const uint64_t id;

private:
	uint64_t genID()
	{
		idMutex.lock();
		uint64_t ret = globalID++;
		idMutex.unlock();
		return ret;
	}

	inline static uint64_t globalID{ 0 };
	inline static std::mutex idMutex{};
};