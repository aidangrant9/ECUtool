#pragma once

#include "Message.hpp"
#include "Buffer.hpp"

class Logger
{
public:
	Logger &instance()
	{
		static Logger instance;
		return instance;
	}

private:
	Logger();
	~Logger();

	Buffer<Message> messages
};