#pragma once

#include <cstdint>
#include <vector>
#include <string>

class Message {
public:
	// Is a valid message (at the data link layer)
	bool isValid;

	// Payload of the message
	std::vector<uint8_t> payload;

	// Should parse the message, initialising member variables appropriately, return whether the message is valid
	virtual bool parse(std::vector<uint8_t> &rawMessage) = 0;

	// Should print the message in a human-readable format for logging
	virtual std::string print() = 0;
};