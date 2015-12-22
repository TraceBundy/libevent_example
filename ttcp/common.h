#pragma once
#include <string.h>
#include <sys/types.h>
#include <string>
struct SessionMessage
{
    int32_t length;
    int32_t number;
};

struct PayloadMessage
{
    int32_t length;
    char data[0];
};


struct Options
{
	int16_t port;
	int32_t number;
	int32_t length;
	bool transmit, receive;
	std::string host;
	Options() :port(0), number(0),
		length(0), transmit(false), receive(false)
	{}
};

bool parseCommandLine(int argc, char *argv[], Options &option);
