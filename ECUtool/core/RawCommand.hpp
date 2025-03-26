#pragma once

#include "Command.hpp"
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct RawCommand : public Command
{
	explicit RawCommand(const std::string &name, const size_t repeatInMilliseconds, const std::vector<uint8_t> &rawMsg)
		: Command(name, repeatInMilliseconds), msg(rawMsg)
	{
		type = Type::Raw;
	}

	std::vector<uint8_t> msg{};

	std::string identifier() override { return std::string{ "RAW" }; }

	std::string toJson() override
	{
		json output;
		output["name"] = name;
		output["repeatInterval"] = repeatInMilliseconds;
		output["type"] = identifier();
		output["visible"] = visible;
		output["data"] = msg;
		return output.dump(4);
	}

	virtual bool run(std::shared_ptr<Connection> connection)
	{
		connection->write(msg);
		connection->read();
		return true;
	}
};