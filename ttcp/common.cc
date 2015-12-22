#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
bool parseCommandLine(int argc, char *argv[], Options &option)
{
	if (argc == 1)
	{
		printf("ttcp - r port\n");
		printf("ttcp -t ip port number length\n");
		return false;
	}
	if (strcasecmp("-t", argv[1]) == 0 && argc == 6)
	{
		option.transmit = true;
		option.host = argv[2];
		option.port = atoi(argv[3]);
		option.number = atoi(argv[4]);
		option.length = atoi(argv[5]);
	}
	else if (strcasecmp("-r", argv[1]) == 0 && argc == 3)
	{
		option.receive = true;
		option.port = atoi(argv[2]);
	}
	else
	{
		printf("ttcp - r port\n");
		printf("ttcp -t ip port number length\n");
		return false;
	}
	return true;
}