#ifndef CODEC_H
#define CODEC_H

#include <string>


enum ParseResult {
	SUB_CMD,
	UNSUB_CMD,
	PUB_CMD,
	UNDIFINED
};
ParseResult parseMessage(const char *info, size_t len,
	std::string &topic, std::string &content);
#endif