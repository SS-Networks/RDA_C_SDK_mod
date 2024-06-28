#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#include <unistd.h>

#include "IotClient.h"
#include "IotClient_ErrCode.h"

#define DATAFORMAT_JSON			"application/json"
#define DATAFORMAT_DELIMITER	"application/x-delimiter"

IotClient* client;

void onMessageArrive(void* context, OCPMessage* message);
void onConnectionClosed(void* context, char* cause);

int main(int argc, char** argv)
{
	char* modelName = "SDK_TEST";
	char* uniqueNum = "ITA";
	char* thingName = "SDK_TEST.ITA";

	char* leafThingName = "SDK_TEST.EDGE002";
	char* leafUniqueNum = "EDGE002";

	client = createIotClient("", "insator", "ocplab1!", "CB00000000", thingName);
	if(client != NULL)
	{
		client->setCustomMessageListener(onMessageArrive, onConnectionClosed);
		// Connect
		if(client->connect() == ERR_OK)
		{
			printf("[RESULT:OK] Connected.\n");
			int rc = 0;
			
			// activate root thing
			if( (rc = client->activateThing(modelName, uniqueNum)) != ERR_OK)
				printf("[RESULT:NOT OK] Root Thing Activation 실패\n");
			else
				printf("[RESULT:OK] Root Thing Activation 성공\n");
			sleep(4);

			// register end thing
			if( (rc = client->registerLeafThing(leafThingName, modelName, leafUniqueNum)) != ERR_OK)
				printf("[RESULT:NOT OK] Register END Thing Activation 실패\n");
			else
				printf("[RESULT:OK] Register END Thing Activation 성공\n");
			sleep(4);

			char* rootData = "{\"stringA\":\"Root_stringA\",\"integerA\":2020,\"stringB\":\"Root_stringB\"}";
			// send attributes
			if( (rc = client->sendAttributes(NULL, rootData, DATAFORMAT_JSON)) != ERR_OK)
				printf("[RESULT:NOT OK] Root - Send Attibutes 실패\n");
			else
				printf("[RESULT:OK] Root - Send Attibutes 성공\n");
			sleep(4);

			char* endData = "{\"stringA\":\"End_stringA\",\"integerA\":2021,\"stringB\":\"End_stringB\"}";
			// send end attributes
			if( (rc = client->registerLeafThing(NULL, endData, DATAFORMAT_JSON)) != ERR_OK)
				printf("[RESULT:NOT OK] End - Send Attibutes 실패\n");
			else
				printf("[RESULT:OK] END - Send Attibutes 성공\n");
			sleep(4);		
		}
		else
		{
			printf("[RESULT:NOT OK] Not Connected.\n");
		}

		client->disconnect();
		destroyIotClient(client);
	}

	return 0;
}

void onMessageArrive(void* context, OCPMessage* message)
{
	if(message)
	{
		printf("\n[<<<] msgType : %s\n", message->msgType);
		printf("[<<<] msgCode : %s\n", message->msgCode);
		printf("[<<<] resCode : %s\n", message->resCode);
		printf("[<<<] encType : %s\n", message->encType);
		printf("[<<<] data    : %s\n", (char*)message->data);

		if(strcmp((char*)message->resCode, OCPMESSAGE_RES_OK))
		{ // not "200"
			printf("[<<<] res code is not ok.\n");
		}
		else
		{
			if(!strcmp(OCPMESSAGE_MSGCODE_PRESAUTHPROCESS_RES, message->msgCode)) {
				printf("[<<<] PRESAUTHPROCESS_RES\n");
			} else if(!strcmp(OCPMESSAGE_MSGCODE_AUTHPROCESS_RES, message->msgCode)) {
				printf("[<<<] AUTHPROCESS_RES\n");
			} else if(!strcmp(OCPMESSAGE_MSGCODE_SEND_HEARTBEAT_RES, message->msgCode)) {
				printf("[<<<] SEND_HEARTBEAT_RES\n");
			} else if(!strcmp(OCPMESSAGE_MSGCODE_ACTIVATE_ROOTTHING_RES, message->msgCode)) {
				printf("[<<<] ACTIVATE_ROOTTHING_RES\n");
			} else if(!strcmp(OCPMESSAGE_MSGCODE_REGISTER_EDGETHING_RES, message->msgCode)) {
				printf("[<<<] REGISTER_EDGETHING_RES\n");
			} else if(!strcmp(OCPMESSAGE_MSGCODE_INACTIVATE_THING_RES, message->msgCode)) {
				printf("[<<<] INACTIVATE_THING_RES\n");
			} else if(!strcmp(OCPMESSAGE_MSGCODE_BULK_INSERT_RES, message->msgCode)) {
				printf("[<<<] BULK_INSERT_RES\n");
			} else if(!strcmp(OCPMESSAGE_MSGCODE_REQUEST_ACTIVEFIRMWARE_RES, message->msgCode)) {
				printf("[<<<] REQUEST_ACTIVEFIRMWARE_RES\n");
				FRMInfo* info = (FRMInfo*)(message->data);
				if(info != NULL) {
					printf("fileUri:%s\n", info->fileUri);
					printf("type:%s\n", info->type);
					printf("version:%s\n", info->version);
					printf("modelName:%s\n", info->modelName);
					}
			} else if(!strcmp(OCPMESSAGE_MSGCODE_NOTI_PASSIVEFIRMWARE_REQ, message->msgCode)) {
				printf("[<<<] NOTI_PASSIVEFIRMWARE_REQ\n");
				FRMInfo* info = (FRMInfo*)(message->data);
				if(info != NULL) {
					printf("fileUri:%s\n", info->fileUri);
					printf("type:%s\n", info->type);
					printf("version:%s\n", info->version);
					printf("modelName:%s\n", info->modelName);
					printf("upgradeId:%s\n", info->upgradeId);
					}
			} else if(!strcmp(OCPMESSAGE_MSGCODE_SEND_FIRMWARERESULT_RES, message->msgCode)) {
				printf("[<<<] SEND_FIRMWARERESULT_RES\n");
			} else if(!strcmp(OCPMESSAGE_MSGCODE_GET_SIGNEDFIRMWARE_RES, message->msgCode)) {
				printf("[<<<] GET_SIGNEDFIRMWARE_RES\n");
			} else if(!strcmp(OCPMESSAGE_MSGCODE_NOTIREQUEST_REQ, message->msgCode)) {
				printf("[<<<] NOTIREQUEST_REQ\n");
			} else if(!strcmp(OCPMESSAGE_MSGCODE_BASIC_PROVISIONING, message->msgCode)) {
				printf("[<<<] Provision Message\n");
			} else if(!strcmp(OCPMESSAGE_MSGCODE_REGISTER_THING_ATTRIBUTE_RES, message->msgCode)) {
				printf("[<<<] REGISTER_THING_ATTRIBUTE_RES\n");
			} else if(!strcmp(OCPMESSAGE_MSGCODE_BASIC_ATTR_GROUP, message->msgCode)) {
				printf("[<<<] BASIC_ATTR_GROUP\n");
			} else if(!strcmp(OCPMESSAGE_MSGCODE_REQUEST_UPLOADURI_RES, message->msgCode)) {
				printf("[<<<] REQUEST_UPLOADURI_RES\n");
				printf("upload file uri:%s\n", (char*)message->data);
			} else if(!strcmp(OCPMESSAGE_MSGCODE_SEND_UPLOADRESULT_RES, message->msgCode)) {
				printf("[<<<] SEND_UPLOADRESULT_RES\n");
			} else {
				printf("[<<<] don't know this message. code(%s) data(%s)\n", message->msgCode, (char*)(message->data));
			}
		}
        	
		freeMessage(message);	// should free the used message
	}
}

void onConnectionClosed(void* context, char* cause)
{ // when the connection is closed, this will be called.
	if(cause != NULL && strlen(cause) != 0)
		printf("[<<<] Connection Closed : %s\n", cause);
}