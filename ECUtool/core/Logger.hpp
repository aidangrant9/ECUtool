#pragma once

#include "Message.hpp"
#include "Buffer.hpp"

#include <mutex>
#include <unordered_map>
#include <functional>

#define BUFFER_SIZE 200

/*
	Class for handling logging and log filtering between core and GUI
	All public methods should obtain publicMutex as this is a thread safe interface
*/
class Logger
{
private:
	Logger() = default;
	~Logger() = default;
	Logger(const Logger &) = delete;
	Logger &operator=(const Logger &) = delete;

	bool isVisible(const Message msg)
	{
		if (!sourceFilter.contains(msg.source))
			sourceFilter[msg.source] = true;

		if (msg.system)
		{
			if (sourceFilter["__SYSTEM__RESERVED__VISIBILITY"])
				return true;
			else
				return false;
		}

		return sourceFilter[msg.source];
	}

	Buffer<std::shared_ptr<Message>> messages{ BUFFER_SIZE };
	std::mutex publicMutex{};
	std::unordered_map<std::string, bool> sourceFilter{};
	std::function<void(std::shared_ptr<Message> msg)> messageCallback{};

public:
	static Logger &instance()
	{
		static Logger instance;
		return instance;
	}

	// Add messages to the buffer & forward to GUI individually
	void addMessage(const Message msg, bool system = false)
	{
		std::lock_guard<std::mutex> lock(publicMutex);

		std::shared_ptr<Message> m = std::make_shared<Message>(msg);

		messages.push(m);

		if (system)
			m->system = true;

		// If its a new source, then we just set it visible
		if (isVisible(*m.get()))
		{
			if (messageCallback)
				messageCallback(m);
		}
	}


	void addErrorMessage(const Message msg, bool system = false)
	{
		std::lock_guard<std::mutex> lock(publicMutex);

		std::shared_ptr<Message> m = std::make_shared<Message>(msg);
		m->msg = "<#e63946>" + m->msg;

		messages.push(m);

		if (system)
			m->system = true;

		if (messageCallback)
			messageCallback(m);
	}

	// Get all messages
	std::vector<std::shared_ptr<Message>> getMessages(bool filtered)
	{
		std::lock_guard<std::mutex> lock(publicMutex);

		std::vector<std::shared_ptr<Message>> ret{};

		for (int i = 0; i < messages.size(); i++)
		{
			if (filtered)
			{
				if(isVisible(*messages[i].get()))
					ret.push_back(messages[i]);
			}
			else
				ret.push_back(messages[i]);
		}

		return ret;
	}

	void clearLogs()
	{
		std::lock_guard<std::mutex> lock(publicMutex);
		messages = Buffer<std::shared_ptr<Message>>{ BUFFER_SIZE };
	}

	void setSourceVisible(std::string source, bool visible)
	{
		std::lock_guard<std::mutex> lock(publicMutex);
		sourceFilter[source] = visible;
	}

	void setMessageCallback(const std::function<void(std::shared_ptr<Message>)> cb)
	{
		std::lock_guard<std::mutex> lock(publicMutex);
		messageCallback = cb;
	}


	static std::string stringFromDataVec(std::vector<uint8_t> data)
	{
		std::ostringstream o;
		o << std::hex << std::uppercase << std::setfill('0');

		for (size_t i = 0; i < data.size(); ++i)
		{
			o << std::setw(2) << static_cast<int>(data[i]);

			if (i < data.size() - 1)
				o << " ";
		}
		return o.str();
	}

	static std::vector<uint8_t> dataVecFromString(std::string input)
	{
		input.erase(std::remove(input.begin(), input.end(), ' '), input.end());
		std::vector<uint8_t> ret{};

		try
		{
			if (input.length() % 2 != 0)
			{
				input = input;
			}
			for (int i = 0; i < input.length(); i += 2)
			{
				std::string byte = input.substr(i, 2);
				ret.push_back(std::stoi(byte, nullptr, 16));
			}
		}
		catch (std::invalid_argument)
		{
			return {};
		}

		return ret;
	}

};