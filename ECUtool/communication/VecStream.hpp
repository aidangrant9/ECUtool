#pragma once

#include <vector>
#include <stdexcept>

/*
	Utility class for parsing, treats a vector as a stream, handles iteration
	and will throw an exception when the requested byte is out of range.
*/

template <typename T>
class VecStream
{
private:
	const std::vector<T> &data;
	size_t head;

public:
	VecStream(const std::vector<T> &data) : data{ data }, head{ 0 } {}

	T next()
	{
		if (head < data.size())
		{
			return data.at(head++);
		}
		else
		{
			throw std::out_of_range("Requested element was out of range");
		}
	}

	T peek() const
	{
		if (head < data.size())
		{
			return data.at(head);
		}
		else
		{
			throw std::out_of_range("Requested element was out of range");
		}
	}

	void setHead(size_t nh)
	{
		if (nh < data.size())
		{
			head = nh;
		}
		else
		{
			throw std::out_of_range("Head is out of range of data");
		}
	}

	bool hasNext() const
	{
		return head < data.size();
	}
};