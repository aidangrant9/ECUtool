#pragma once

#include <vector>
#include <stdexcept>

template <typename T>
class Buffer
{
public:
	explicit Buffer(size_t capacity)
		: capacity{capacity}, head{0}, c_size{0}
	{
		buffer.resize(capacity);
	}

	void push(const T &elem)
	{
		buffer[head] = elem;
		head = (head + 1) % capacity;
		
		if (c_size != capacity)
		{
			c_size++;
		}
	}

	size_t size() const
	{
		return c_size;
	}

	T &operator[](const size_t idx)
	{
		if (idx >= c_size)
			throw std::out_of_range("index out of range");

		return buffer[(head + idx + capacity - c_size) % capacity];
	}


private:
	std::vector<T> buffer{};
	size_t head;
	size_t c_size;
	size_t capacity;
};