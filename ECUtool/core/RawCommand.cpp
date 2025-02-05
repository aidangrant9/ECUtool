#include "RawCommand.hpp"
#include <iostream>

RawCommand::RawCommand(std::string name, std::vector<uint8_t> rawMsg)
	: Command(name), msg(rawMsg)
{}

void RawCommand::execute(std::function<void(const std::string & result)> executionStopped)
{
	

	
}

