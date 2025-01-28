#pragma once

#include <optional>
#include "DataLinkMessage.hpp"

class KWP2000Message : public DataLinkMessage
{
protected:
	// KWP2000 specific fields
	uint8_t format; // Format byte
	uint8_t mode; // Addressing mode
	std::optional<uint8_t> target; // Target address
	std::optional<uint8_t> source; // Source address
	std::optional<uint8_t> length; // Length byte of message
	uint8_t checksum; // Supplied checksum

public:
	KWP2000Message(const std::vector<uint8_t> &rawMessage);
	virtual ~KWP2000Message() override;
	virtual uint8_t calcChecksum();
	virtual std::vector<uint8_t> &getPayload() override;
	virtual std::vector<uint8_t> &getRaw() override;
	virtual std::string &toString() override;

	KWP2000Message(const KWP2000Message &m);
	KWP2000Message &operator=(const KWP2000Message &m);

	KWP2000Message(KWP2000Message &&m);
	KWP2000Message &operator=(KWP2000Message &&m);

	bool parse(std::vector<uint8_t> &rawMessage);
};