#pragma once

#include <vector>
#include <stdexcept>

template <typename T>
class Buffer
{
public:
	explicit Buffer(size_t capacity)
		: capacity{capacity}, head{0}, size{0}
	{
		buffer.resize(capacity);
	}

	void push(const T &elem)
	{
		buffer[head] = elem;
		head = (head + 1) % capacity;
		
		if (size != capacity)
		{
			size++;
		}
	}

	size_t size() const
	{
		return size;
	}

	T &operator[](const size_t idx)
	{
		if (idx >= size)
			throw std::out_of_range("index out of range");

		return buffer[(head + idx + capacity - size) % capacity];
	}


private:
	std::vector<T> buffer{};
	size_t head;
	size_t size;
	size_t capacity;
};