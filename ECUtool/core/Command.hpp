#pragma once

#include "../communication/Connection.hpp"
#include <vector>
#include <string>
#include <cstdint>
#include <functional>
#include <optional>
#include <stop_token>

struct Command
{
	enum class Type
	{
		Raw,
		Script,
	};

	explicit Command(const std::string &name, const std::size_t repeatInMilliseconds)
		: name(name), repeatInMilliseconds(repeatInMilliseconds), active(false), visible(true)
	{}

	virtual ~Command() = default;

	virtual bool run(std::shared_ptr<Connection> connection, std::string arguments, const std::stop_token &st) = 0;
	
	virtual std::string identifier() = 0;
	virtual std::string toJson() = 0;
	
	Type type;
	std::string name;
	std::size_t repeatInMilliseconds;
	bool active;
	bool visible;
};