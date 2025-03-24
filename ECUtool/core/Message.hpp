#pragma once

#include <string>
#include <utility>
#include <vector>
#include <chrono>
#include <iomanip>

struct Message
{
	Message(const std::string &msg = "",
		const std::string &src = "")
		: msg(msg), source(src)
	{
		auto now = std::chrono::system_clock::now();
		auto now_time_t = std::chrono::system_clock::to_time_t(now);
		std::tm now_tm = *std::localtime(&now_time_t);
		auto duration = now.time_since_epoch();
		auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() % 1000;
		std::ostringstream oss;
		oss << std::setw(2) << std::setfill('0') << now_tm.tm_min << ":"
			<< std::setw(2) << std::setfill('0') << now_tm.tm_sec << "."
			<< std::setw(2) << std::setfill('0') << millis / 10;
		timeString = oss.str();
	}

	std::string timeString{};
	std::string source{};
	std::string msg {};
};