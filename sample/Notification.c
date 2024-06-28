#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "IotClient.h"
#include "IotClient_ErrCode.h"

void onMessageArrive(void* context, OCPMessage* message);

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

	IotClient* client = createIotClient(authCode, userId, userPw, siteId, thingName);
	if(client != NULL)
	{
		// listener should be set before connect
		client->setCustomMessageListener(onMessageArrive, NULL);

		if( client->connect() == ERR_OK )
		{ // connected
			printf(">> Connected.\n");
		}
		// destroy resources
		destroyIotClient(client);
	}

	return 0;
}

void onMessageArrive(void* context, OCPMessage* message)
{
	if(message)
	{
		if(strcmp(message->msgCode, OCPMESSAGE_MSGCODE_SEND_HEARTBEAT_RES) == 0)
		{ // received a response of heart beat signal
			printf("<< heart beat response\n");
		}
		else if(strcmp(message->msgCode, OCPMESSAGE_MSGCODE_NOTIREQUEST_REQ) == 0)
		{ // received a notification request
			printf("<< notification request\n");
			printf("<< data:%s\n", (char*)(message->data));
		}
		else
		{ // not defined message

		}
		freeMessage(message);
	}
}