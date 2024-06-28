#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "IotClient.h"
#include "IotClient_ErrCode.h"

int main(int argc, char** argv)
{
	char* authCode =  "adf022d0824dc28c";
	char* userId = "";
	char* userPw = "";

	char* siteId =  "C000000037";
	char* thingName = "999999.1324";

	IotClient* client = createIotClient(authCode, userId, userPw, siteId, thingName);
	if(client != NULL)
	{
		if( client->connect() == ERR_OK )
			printf("successfully connected.\n");
		else
			printf("can't be connected.\n");

		while(1);
		destroyIotClient(client);
	}

	return 0;
}
