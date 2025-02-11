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
	explicit Command(const std::string &name)
		: name(name)
	{}

	virtual ~Command() = default;
	
	std::string name;
};