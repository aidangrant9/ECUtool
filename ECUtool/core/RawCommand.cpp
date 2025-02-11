#include "RawCommand.hpp"

RawCommand::RawCommand(const std::string &name, const std::vector<uint8_t> &rawMsg) : Command(name), msg(rawMsg)
{
}
