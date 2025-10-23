#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "IotClient.h"
#include "IotClient_ErrCode.h"

/**
 * A sample of connecting objects registered in the PoC using the ITA method
 *
 *
 * After accessing the PoC, search for the objects created in the Objects > Object Management menu and enter the information below.
 * Create an authentication object using the entered information and receive the object through the IoTClient class provided by the SDK.
 * Connect and activate the device object.
 */

int main(int argc, char** argv)
{
	// TODO: add Connection.example_run.sh file
	if (argc < 6) {
        printf("Usage: %s <authCode> <userId> <userPw> <siteId> <thingName>\n", argv[0]);
        return 1;
    }

    char* authCode  = argv[1];
    char* userId    = argv[2];
    char* userPw    = argv[3];
	
    char* siteId    = argv[4];
    char* thingName = argv[5];

    printf("authCode: %s\n", authCode);
    printf("userId: %s\n", userId);
    printf("userPw: %s\n", userPw);
    printf("siteId: %s\n", siteId);
    printf("thingName: %s\n", thingName);

    // Utilize the basic information of the object as parameters.
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
