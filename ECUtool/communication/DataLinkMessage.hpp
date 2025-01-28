#pragma once

#include <cstdint>
#include <vector>
#include <string>

class DataLinkMessage
{
protected:
	std::vector<uint8_t> payload;
	std::vector<uint8_t> raw;
public:
	virtual ~DataLinkMessage() = 0;

	// Is a valid message
	virtual bool valid() = 0;

	// Payload of the message
	virtual std::vector<uint8_t> &getPayload() = 0;

	// Raw message
	virtual std::vector<uint8_t> &getRaw() = 0;

	// Should print the message in a human-readable format for logging
	virtual std::string &toString() = 0;
};