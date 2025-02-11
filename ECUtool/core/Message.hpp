#pragma once

#include <string>
#include <tuple>
#include <vector>

struct Message
{
	enum class MessageType
	{
		Error,
		Info,
		Data,
		Unspecified
	};

	Message() = default;

	Message(const MessageType type, const std::string &msg, const std::string &src)
		: msg(msg), type(type), source(source)
	{}

	Message(const MessageType type, const std::string &msg)
		: msg(msg), type(type)
	{}

	Message(const std::string &msg)
		: msg(msg)
	{}

	Message(const std::string &msg, const std::string &src)
		: msg(msg), source(src)
	{}

	Message(const MessageType type, const std::string &msg, const std::vector<std::tuple<std::string, std::string>> &formats)
		: msg(msg), type(type), formats(formats)
	{}

	Message(const MessageType type, const std::string &msg, uint64_t id, const std::vector<std::tuple<std::string, std::string>> &formats)
		: msg(msg), type(type), formats(formats), id(id)
	{}

	Message(const MessageType type, const std::string &msg, const std::string &src, const std::vector<std::tuple<std::string, std::string>> &formats)
		: msg(msg), type(type), formats(formats), source(src)
	{}

	Message(const MessageType type, const std::string &msg, uint64_t id, const std::string &src, const std::vector<std::tuple<std::string, std::string>> &formats)
		: msg(msg), type(type), formats(formats), source(src), id(id)
	{}

	std::string source{"?"};
	uint64_t id;
	std::string msg {};
	MessageType type { MessageType::Unspecified };
	std::vector<std::tuple<std::string, std::string>> formats {};
};