#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "IotClient.h"
#include "IotClient_ErrCode.h"

void onMessageArrive(void* context, OCPMessage* message);
char* parseFileUriFromData(char* data) { return NULL; }

IotClient* client;

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

	client = createIotClient(authCode, userId, userPw, siteId, thingName);
	if(client != NULL)
	{
		// listener should be set before connect
		client->setCustomMessageListener(onMessageArrive, NULL);

		if( client->connect() == ERR_OK )
		{ // connected
			printf("Connected.\n");
			//=====================================================================================
			if( client->requestFirmwareLatestVersion("TestModel", "patch") == ERR_OK )
				printf("ACTIVE FIRMWARE REQUEST 전송 완료\n");
			else
				printf("ACTIVE FIRMWARE REQUEST 전송 실패\n");
			//=====================================================================================
		}
		else
			printf("Not Connected.\n");

		// destroy resources
		destroyIotClient(client);
	}

	return 0;
}

void onMessageArrive(void* context, OCPMessage* message)
{
	if(message)
	{
		char* thing_name = "TestModel.001";

		if(strcmp(message->msgCode, OCPMESSAGE_MSGCODE_REQUEST_ACTIVEFIRMWARE_RES) == 0)
		{ // response of "Request Active Firmware Latest Version"

			FRMInfo* info = (FRMInfo*)(message->data);
			printf("fileUri : %s\n", info->fileUri);
			printf("type : %s\n", info->type);
			printf("version : %s\n", info->version);
			printf("modelName : %s\n", info->modelName);
		}
		else if(strcmp(message->msgCode, OCPMESSAGE_MSGCODE_NOTI_PASSIVEFIRMWARE_REQ) == 0)
		{ // request of passive firmware upgrade notification has come from the server

			FRMInfo* info = (FRMInfo*)(message->data);
			printf("fileUri : %s\n", info->fileUri);
			printf("type : %s\n", info->type);
			printf("version : %s\n", info->version);
			printf("modelName : %s\n", info->modelName);
			printf("upgradeId : %s\n", info->upgradeId);

			// say it's done to server.
			client->requestFirmwareUpgradeComplete(thing_name, "patch", "3.0");
		}
		else
		{ // not defined message

		}

		freeMessage(message);
	}
}
