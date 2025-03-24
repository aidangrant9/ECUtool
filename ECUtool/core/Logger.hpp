#pragma once

#include "Message.hpp"
#include "Buffer.hpp"

#include <mutex>
#include <unordered_map>
#include <functional>

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

	bool isVisible(const Message &msg)
	{
		if (!sourceFilter.contains(msg.source))
			sourceFilter[msg.source] = true;

		return sourceFilter[msg.source];
	}

	Buffer<std::shared_ptr<Message>> messages{ 1000 };
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
	void addMessage(const Message &msg)
	{
		std::lock_guard<std::mutex> lock(publicMutex);

		std::shared_ptr<Message> m = std::make_shared<Message>(msg);

		messages.push(m);

		// If its a new source, then we just set it visible
		if (isVisible(msg))
		{
			if (messageCallback)
				messageCallback(m);
		}
	}


	void addErrorMessage(const Message &msg)
	{
		std::lock_guard<std::mutex> lock(publicMutex);

		std::shared_ptr<Message> m = std::make_shared<Message>(msg);
		m->msg = "<#e63946>" + m->msg;

		messages.push(m);

		if (messageCallback)
			messageCallback(m);
	}

	// Get all messages
	std::vector<std::shared_ptr<Message>> &getMessages(bool filtered)
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

	void setSourceVisible(std::string &source, bool visible)
	{
		std::lock_guard<std::mutex> lock(publicMutex);
		sourceFilter[source] = visible;
	}

	void setMessageCallback(const std::function<void(std::shared_ptr<Message>)> &cb)
	{
		std::lock_guard<std::mutex> lock(publicMutex);
		messageCallback = cb;
	}
};