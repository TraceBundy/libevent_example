#include "Codec.h"
#include <string.h>


static const char* getTopic(const char *ptr, std::string &topic){
	while (*ptr != '\0' && *ptr == ' ') ++ptr;
	while (*ptr != '\0' && *ptr != ' ') {
		topic += *ptr;
		++ptr;
	}
	return ++ptr;
}
static void getContent(const char *ptr, std::string &content){
	while (*ptr != '\0' && *ptr != ' '){
		content += *ptr;
		++ptr;
	}
	content += '\n';
}
ParseResult parseMessage(const char *info, size_t len,
	std::string &topic, std::string &content){
	const char *ptr = info;
	if (strncasecmp(info, "sub", 3) == 0){
		ptr += 3;
		getTopic(ptr, topic);
		return SUB_CMD;
	} else if (strncasecmp(info, "unsub", 5) == 0){
		ptr += 5;
		getTopic(ptr, topic);
		return UNSUB_CMD;
	} else if (strncasecmp(info, "pub", 3) == 0){
		ptr += 3;
		ptr = getTopic(ptr, topic);
		getContent(ptr, content);
		
		return PUB_CMD;
	} else {
		return UNDIFINED;
	}
}