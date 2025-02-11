#pragma once

#include "../communication/SerialConnection.hpp"
#include <vector>
#include <string>
#include <cstdint>
#include <functional>
#include <optional>

struct Command
{
public:
	explicit Command(const std::string &name, const std::size_t repeatInMilliseconds)
		: name(name), repeatInMilliseconds(repeatInMilliseconds)
	{}

	virtual ~Command() = default;
	
	virtual std::string identifier() { return std::string{ "?" }; }
	
	std::string name;
	std::size_t repeatInMilliseconds;
};