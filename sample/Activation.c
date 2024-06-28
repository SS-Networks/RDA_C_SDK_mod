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
	char* userId = "test@samsung.com";
	char* userPw = "test1!";

	char* siteId = "CB00000000";
	char* thingName = "TestModel.001";

	IotClient* client = createIotClient(authCode, userId, userPw, "CB00000000", "TestModel.001");
	if(client != NULL)
	{
		if( client->connect() == ERR_OK )
		{ // connected
			//=====================================================================================
			// Activate a root thing
			client->activateThing("TestModel", "001");

			//=====================================================================================
		}
		// destroy resources
		destroyIotClient(client);
	}

	return 0;
}