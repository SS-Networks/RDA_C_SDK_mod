#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "IotClient.h"
#include "IotClient_ErrCode.h"

int main(int argc, char** argv)
{
	// set the thing information
	// option 1) set auth code, then don't need id/pw.
	// option 2) don't know auth code, then should set id/pw.
	char* authCode = "";
	char* userId = "";
	char* userPw = "";

	char* siteId = "CB00000000";
	char* thingName = "999999.1324";

	IotClient* client = createIotClient(authCode, userId, userPw, siteId, thingName);
	if(client != NULL)
	{
		if( client->connect() == ERR_OK )
			printf("successfully connected.\n");
		else
			printf("can't be connected.\n");

		// destroy resources
		destroyIotClient(client);
	}

	return 0;
}
