#pragma once

#include <optional>
#include "Message.hpp"

class KWP2000Message : public Message
{
public:
	// KWP2000 specific fields

	uint8_t format; // Format byte
	uint8_t mode; // Addressing mode

	std::optional<uint8_t> target; // Target address
	std::optional<uint8_t> source; // Source address
	std::optional<uint8_t> length; // Length byte of message (If you want the length of the payload use payload.size())

	uint8_t checksum; // Supplied checksum

	KWP2000Message(std::vector<uint8_t> &rawMessage);
	virtual uint8_t calcChecksum();

	virtual bool parse(std::vector<uint8_t> &rawMessage) override;
	virtual std::string print() override;
};