#include <sstream>
#include<iomanip>
#include "KWP2000_Message.hpp"
#include "VecStream.hpp"

KWP2000Message::KWP2000Message(std::vector<uint8_t> &rawMessage) : rawMessage
{
	
}

bool KWP2000Message::parse(std::vector<uint8_t> &rawMessage)
{
	VecStream stream(rawMessage);

	try
	{
		format = stream.next();
		mode = format >> 6;
		int len = format & 0b00111111;

		switch (mode)
		{
		case 0:
			// No address information
			break;
		case 1:
			// Exception mode (CARB)
			// TODO
			return false;
			break;
		case 2:
		case 3:
			// Address information included
			target = stream.next();
			source = stream.next();
			break;
		}

		if (len == 0) // Additional byte length included
		{ 
			length = len = stream.next();
		}

		payload.reserve(len);
		for (int i = 0; i < len; i++)
		{
			payload.push_back(stream.next());
		}

		checksum = stream.next();
		return true;
	}
	catch (std::out_of_range e) {
		return false;
	}
}

uint8_t KWP2000Message::calcChecksum() {
	uint8_t acc = format;
	acc += target.value_or(0);
	acc += source.value_or(0);
	acc += length.value_or(0);
	for (uint8_t b : payload) {
		acc += b;
	}
	return acc;
}

std::string KWP2000Message::print() {
	std::stringstream ret;

	ret << std::uppercase << std::setw(2) << std::setfill('0') << std::hex;

	if (isValid)
		ret << "VALID ";
	else
		ret << "INVALID ";
	
	if (mode > 1)
	{
		if (mode == 2)
		{
			ret << "(PHY ";
		}
		else
		{
			ret << "(FUN ";
		}
		ret << +source.value_or(0xFF) << " -> " << +target.value_or(0xFF) << ")  ";
	}

	ret << "[ ";

	for (uint8_t &v : payload)
	{
		ret << +v << " ";
	}

	ret << "]  ";
	ret << +checksum;
	
	return ret.str();
}


KWP2000Message::~KWP2000Message()
{

}