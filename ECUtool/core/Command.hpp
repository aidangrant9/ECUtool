#pragma once

#include "../communication/SerialConnection.hpp"
#include <vector>
#include <string>
#include <cstdint>
#include <functional>
#include <optional>

struct Command
{
	enum class Type
	{
		Raw,
		Script,
	};

	explicit Command(const std::string &name, const std::size_t repeatInMilliseconds)
		: name(name), repeatInMilliseconds(repeatInMilliseconds)
	{}

	virtual ~Command() = default;
	
	virtual std::string identifier() = 0;
	virtual std::string toJson() = 0;
	
	Type type;
	std::string name;
	std::size_t repeatInMilliseconds;
};