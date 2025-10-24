#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "IotClient.h"
#include "IotClient_ErrCode.h"

/**
 * A sample of connecting objects and transmitting data using the ITA method
 *
 *
 * After accessing the PoC, search for the objects created in the Objects > Object Management menu and enter the information below.
 * Create an authentication object using the entered information and receive the object through the IoTClient class provided by the SDK.
 * Connect and activate the object to switch it to a state where it can receive data.
 * To transmit the collected data, enter values that match the attribute name and attribute type of the object to create data in JSON format.
 * Transmit the data through the object connected to the PoC.
 */

int main(int argc, char **argv) {

	if (argc < 7) {
        printf("Usage: %s <authCode> <userId> <userPw> <siteId> <thingName> <jsonData> \n", argv[0]);
        return 1;
    }

    char* authCode  = argv[1];
    char* userId    = argv[2];
    char* userPw    = argv[3];
	
    char* siteId    = argv[4];
    char* thingName = argv[5];

    char *jsonStr   = argv[6];

    // Utilize the basic information of the object as parameters.
    IotClient *client = createIotClient(authCode, userId, userPw, siteId, thingName);
    if (client != NULL) {
        if (client->connect() == ERR_OK) { // connected

            //Create data in JSON format and transmit it once every 10 seconds.
            char *msgCode = NULL;

            client->sendAttributes(msgCode, jsonStr);
            // note: https://github.com/RDA-DATA/RDA_C_SDK/blob/main/sdk/iotclient/IotClient.c#L1481
            // message is sent using "void* thread_sender(void* args)" function
            //  which is a while loop with sleep(1) interval
            //  so, just to be safe, sleep 2 seconds:
            sleep(2);
        }
        destroyIotClient(client);
    }

    return 0;
}