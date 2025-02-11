#include <vector>
#include <string>
#include <iostream>

#ifdef _WIN32
#include <Windows.h>

#define PORT_COUNT 255

std::vector<std::string> getPortNames()
{
	std::vector<ULONG> portNumbers;
	portNumbers.resize(PORT_COUNT);
	ULONG portsFound = 0;
	GetCommPorts(portNumbers.data(), PORT_COUNT, &portsFound);
	portNumbers.resize(portsFound);

	std::vector<std::string> comChoices;
	comChoices.reserve(portsFound);

	for (ULONG &i : portNumbers)
	{
		comChoices.push_back(std::string{"COM"} + std::to_string(i));
	}

	return comChoices;
}

#endif

#ifdef _UNIX
	//put mac&linux fn here

std::vector<std::string> getPortNames()
{
	return {};
}
#endif
