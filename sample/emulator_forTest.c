#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#include <unistd.h>

#include "IotClient.h"
#include "IotClient_ErrCode.h"
//*********************************************************************************************************************
// GLOBAL AREA
//*********************************************************************************************************************
IotClient* client;

#define SITEID 		"CB00000000"
#define THINGNAME	"TEST_TYPE.ITA001"
#define AUTHCODE	""

#define PORTAL_ID	"test@samsung.com"
#define PORTAL_PW	"test1!"

void onMessageArrive(void* context, OCPMessage* message);
void onConnectionClosed(void* context, char* cause);
void do_testcase(int select);
//*********************************************************************************************************************
// MAIN FUNCTION IMPLE
//*********************************************************************************************************************
int main(int argc, char** argv)
{
	fprintf(stdout, "==============================================================\n");
	fprintf(stdout, ">>>>>>>>>>>>>>>>>>> Starting Emulator.........................\n");
	// set the thing information
	// option 1) set auth code, then don't need id/pw.
	// option 2) don't know auth code, then should set id/pw.

	client = createIotClient(AUTHCODE, PORTAL_ID, PORTAL_PW, SITEID, THINGNAME);
	if(client != NULL)
	{
		client->setCustomMessageListener(onMessageArrive, onConnectionClosed);

		int select = 0;
		while(select != -1)
		{
			fprintf(stdout, "==============================================================\n");
			fprintf(stdout, "[EMULATOR]\n");
			fprintf(stdout, "==============================================================\n");
			fprintf(stdout, " (-1) EXIT \n");
			fprintf(stdout, "--------------------------------------------------------------\n");
			fprintf(stdout, " ( 1) Connect \n");
			fprintf(stdout, " ( 2) Root Thing 활성화\n");
			fprintf(stdout, " ( 3) Leaf Thing 등록\n");
			fprintf(stdout, " ( 5) Root Thing 정보 저장 \n");
			fprintf(stdout, " ( 6) Leaf Thing 정보 저장\n");
			fprintf(stdout, " ( 8) Active Firmware 정보 수신 \n");
			fprintf(stdout, " ( 9) request signed data of firmware \n");
			fprintf(stdout, " (10) Firmware 완료 신호 전송 \n");
			fprintf(stdout, " (11) disconnect \n");
			fprintf(stdout, " (13) WBC Encrypted MC 값 요청 \n");
			fprintf(stdout, " (15) activate leaf thing \n");
			fprintf(stdout, " (90) File Uploda Uri 요청 \n");
			fprintf(stdout, " (91) File Uploda Complete 전송 \n");
			fprintf(stdout, " (92) Root 정보 저장 Sync \n");
			fprintf(stdout, " (93) Leaf 정보 저장 Sync \n");
			fprintf(stdout, "------------------------------------------------\n");
			fprintf(stdout, " SELECT: ");
			fscanf(stdin, "%d", &select);
			fprintf(stdout, "\n==============================================================\n");
			do_testcase(select);
		}

		// disconnect
		client->disconnect();
		// free resources
		destroyIotClient(client);
	} 
	else
	{
		fprintf(stderr, "[ERR] Failed to create Iot client.\n");
		exit(EXIT_FAILURE);		
	}

	fprintf(stdout, ">>>>>>>>>>>>>>>>>>> Terminating Emulator.....\n");
	fprintf(stdout, "====================================================\n");
	return 0;
}

//*********************************************************************************************************************
// LISTENERS
//*********************************************************************************************************************
void onMessageArrive(void* context, OCPMessage* message)
{
	if(message)
	{
		fprintf(stderr, "\n[<<<] msgType : %s\n", message->msgType);
		//fprintf(stderr, "[<<<] msgId   : %s\n", message->msgId);
		fprintf(stderr, "[<<<] msgCode : %s\n", message->msgCode);
		fprintf(stderr, "[<<<] resCode : %s\n", message->resCode);
		fprintf(stderr, "[<<<] encType : %s\n", message->encType);
		fprintf(stderr, "[<<<] data    : %s\n", (char*)message->data);

		if(strcmp((char*)message->resCode, OCPMESSAGE_RES_OK))
		{ // not "200"
			fprintf(stderr, "[<<<] res code is not ok.\n");
		}
		else
		{
			if(!strcmp(OCPMESSAGE_MSGCODE_PRESAUTHPROCESS_RES, message->msgCode)) {
				fprintf(stderr, "[<<<] PRESAUTHPROCESS_RES\n");
			} else if(!strcmp(OCPMESSAGE_MSGCODE_AUTHPROCESS_RES, message->msgCode)) {
				fprintf(stderr, "[<<<] AUTHPROCESS_RES\n");
			} else if(!strcmp(OCPMESSAGE_MSGCODE_SEND_HEARTBEAT_RES, message->msgCode)) {
				fprintf(stderr, "[<<<] SEND_HEARTBEAT_RES\n");
			} else if(!strcmp(OCPMESSAGE_MSGCODE_ACTIVATE_ROOTTHING_RES, message->msgCode)) {
				fprintf(stderr, "[<<<] ACTIVATE_ROOTTHING_RES\n");
			} else if(!strcmp(OCPMESSAGE_MSGCODE_REGISTER_EDGETHING_RES, message->msgCode)) {
				fprintf(stderr, "[<<<] REGISTER_EDGETHING_RES\n");
			} else if(!strcmp(OCPMESSAGE_MSGCODE_INACTIVATE_THING_RES, message->msgCode)) {
				fprintf(stderr, "[<<<] INACTIVATE_THING_RES\n");
			} else if(!strcmp(OCPMESSAGE_MSGCODE_BULK_INSERT_RES, message->msgCode)) {
				fprintf(stderr, "[<<<] BULK_INSERT_RES\n");
			} else if(!strcmp(OCPMESSAGE_MSGCODE_REQUEST_ACTIVEFIRMWARE_RES, message->msgCode)) {
				fprintf(stderr, "[<<<] REQUEST_ACTIVEFIRMWARE_RES\n");
				FRMInfo* info = (FRMInfo*)(message->data);
				if(info != NULL) {
					fprintf(stdout, "fileUri:%s\n", info->fileUri);
					fprintf(stdout, "type:%s\n", info->type);
					fprintf(stdout, "version:%s\n", info->version);
					fprintf(stdout, "modelName:%s\n", info->modelName);
					}
			} else if(!strcmp(OCPMESSAGE_MSGCODE_NOTI_PASSIVEFIRMWARE_REQ, message->msgCode)) {
				fprintf(stderr, "[<<<] NOTI_PASSIVEFIRMWARE_REQ\n");
				FRMInfo* info = (FRMInfo*)(message->data);
				if(info != NULL) {
					fprintf(stdout, "fileUri:%s\n", info->fileUri);
					fprintf(stdout, "type:%s\n", info->type);
					fprintf(stdout, "version:%s\n", info->version);
					fprintf(stdout, "modelName:%s\n", info->modelName);
					fprintf(stdout, "upgradeId:%s\n", info->upgradeId);
					}
			} else if(!strcmp(OCPMESSAGE_MSGCODE_SEND_FIRMWARERESULT_RES, message->msgCode)) {
				fprintf(stderr, "[<<<] SEND_FIRMWARERESULT_RES\n");
			} else if(!strcmp(OCPMESSAGE_MSGCODE_GET_SIGNEDFIRMWARE_RES, message->msgCode)) {
				fprintf(stderr, "[<<<] GET_SIGNEDFIRMWARE_RES\n");
			} else if(!strcmp(OCPMESSAGE_MSGCODE_NOTIREQUEST_REQ, message->msgCode)) {
				fprintf(stderr, "[<<<] NOTIREQUEST_REQ\n");
			} else if(!strcmp(OCPMESSAGE_MSGCODE_BASIC_PROVISIONING, message->msgCode)) {
				fprintf(stderr, "[<<<] Provision Message\n");
			} else if(!strcmp(OCPMESSAGE_MSGCODE_REGISTER_THING_ATTRIBUTE_RES, message->msgCode)) {
				fprintf(stderr, "[<<<] REGISTER_THING_ATTRIBUTE_RES\n");
			} else if(!strcmp(OCPMESSAGE_MSGCODE_BASIC_ATTR_GROUP, message->msgCode)) {
				fprintf(stderr, "[<<<] BASIC_ATTR_GROUP\n");
			} else if(!strcmp(OCPMESSAGE_MSGCODE_REQUEST_UPLOADURI_RES, message->msgCode)) {
				fprintf(stderr, "[<<<] REQUEST_UPLOADURI_RES\n");
				fprintf(stdout, "upload file uri:%s\n", (char*)message->data);
			} else if(!strcmp(OCPMESSAGE_MSGCODE_SEND_UPLOADRESULT_RES, message->msgCode)) {
				fprintf(stderr, "[<<<] SEND_UPLOADRESULT_RES\n");
			} else if(!strcmp(OCPMESSAGE_MSGCODE_WBC_MC_RES, message->msgCode)) {
				fprintf(stderr, "[<<<] REQUEST_WBC_MC_RES\n");
				fprintf(stdout, "encryptedMC: (%s)\n", (char*)message->data);
			} else {
				fprintf(stderr, "[<<<] don't know this message. code(%s) data(%s)\n", message->msgCode, (char*)(message->data));
			}
		}
        	
		freeMessage(message);	// should free the used message
	}
}

void onConnectionClosed(void* context, char* cause)
{ // when the connection is closed, this will be called.
	if(cause != NULL && strlen(cause) != 0)
		fprintf(stdout, "[<<<] Connection Closed : %s\n", cause);
}

//*********************************************************************************************************************
// PRIVATE FUNCTIONS
//*********************************************************************************************************************
void do_testcase(int select)
{
	// if( select < -1 || select > 10 )
	// 	fprintf(stdout, "select between (-1 ~ 10)\n");
	// else
	{
		switch(select)
		{
//------------------------------------------------------------------------------------------------------------------
			case 1:		// connect
				if(client->connect() == ERR_OK)
					fprintf(stdout, "[RESULT:OK] Connected. \n");
				else
					fprintf(stdout, "[RESULT:NOT OK] Not Connected. \n");
				break;
//------------------------------------------------------------------------------------------------------------------
			case 2:		// activate root thing
			{
				fprintf(stdout, "(Activate a Root Thing)\n");
				
				char model[SHORT_STR_LEN] = {0, };
				fprintf(stdout, "Input model name:");
				fscanf(stdin, "%s", model);
				
				char uniqueNum[SHORT_STR_LEN] = {0, };
				fprintf(stdout, "Input unique number:");
				fscanf(stdin, "%s", uniqueNum);
				
				if(client->activateThing(model, uniqueNum) != ERR_OK)
					fprintf(stdout, "[RESULT:NOT OK] Failed to activate a root thing.\n");
			}	
			break;
//------------------------------------------------------------------------------------------------------------------
			case 3:		// register leaf thing
			{
				fprintf(stdout, "(Register a Leaf Thing)\n");

				char thingName[SHORT_STR_LEN] = {0, };
				fprintf(stdout, "Input leaf thing name:");
				fscanf(stdin, "%s", thingName);

				char model[SHORT_STR_LEN] = {0, };
				fprintf(stdout, "Input model name:");
				fscanf(stdin, "%s", model);

				char uniqueNum[SHORT_STR_LEN] = {0, };
				fprintf(stdout, "Input unique number:");
				fscanf(stdin, "%s", uniqueNum);

				if(client->registerLeafThing(thingName, model, uniqueNum) != ERR_OK)
					fprintf(stdout, "[RESULT:NOT OK] Failed to register a leaf thing.\n");
			}
			break;
//------------------------------------------------------------------------------------------------------------------
			case 5:		// send attributes
			{
				fprintf(stdout, "(Send Attributes of Root Thing)\n");

				char dataStr[LONG_STR_LEN] = {0, };
				char text[SHORT_STR_LEN] = {0, };
				int latitude = 0, longitude = 0;

				fprintf(stdout, "Input the value of integer:");
				fscanf(stdin, "%d", &latitude);
				fprintf(stdout, "Input the value of string:");
				fscanf(stdin, "%s", text);
				char* baseStr = "{\"integerA\":%d,\"stringA\":\"%s\"}";
				sprintf(dataStr, baseStr, latitude, text);

				// send attributes data for Root thing
				client->sendAttributes(NULL, dataStr);
				fprintf(stdout, "[>>>] Send (%s)\n", dataStr);
			}
			break;
//------------------------------------------------------------------------------------------------------------------
			case 6:	// send attributes of leaf thing
			{
				fprintf(stdout, "(Send Attributes of Leaf Thing)\n");

				char thingName[SHORT_STR_LEN] = {0, };
				fprintf(stdout, "Input leaf thing name:");
				fscanf(stdin, "%s", thingName);

				char dataStr[LONG_STR_LEN] = {0, };
				char text[SHORT_STR_LEN] = {0, };
				int temperature = 0;
				fprintf(stdout, "Input text (string):");
				fscanf(stdin, "%s", text);
				fprintf(stdout, "Input temperature (integer):");
				fscanf(stdin, "%d", &temperature);

				char* baseStr = "{\"text\":\"%s\",\"temperature\":%d}";
				sprintf(dataStr, baseStr, text, temperature);

				// send attributes data for Root thing
				client->sendAttributesForLeaf(thingName, NULL, dataStr);
				fprintf(stdout, "[>>>] Send (%s)\n", dataStr);
			}
			break;
//------------------------------------------------------------------------------------------------------------------
			case 8:		// Active firmware latest version
			{
				fprintf(stdout, "(Request Firmware Latest Version Information)\n");
				char modelName[SHORT_STR_LEN] = {0, };
				fprintf(stdout, "Input model name:");
				fscanf(stdin, "%s", modelName);

				char firmwareType[SHORT_STR_LEN] = {0, };
				fprintf(stdout, "Input firmware type:");
				fscanf(stdin, "%s", firmwareType);

				if(client->requestFirmwareLatestVersion(modelName, firmwareType) != ERR_OK)
					fprintf(stdout, "[RESULT:NOT OK] Failed to request firmware information.\n");
			}
			break;
//------------------------------------------------------------------------------------------------------------------
			case 9:		// request signed data of firmware
				break;
//------------------------------------------------------------------------------------------------------------------
			case 10:		// tell firmware upgrade is completed
			{
				fprintf(stdout, "(Firmware Upgrade Complete 신호 전송)\n");
				char type[SHORT_STR_LEN] = {0, };
				printf("Input firmware type:");
				fscanf(stdin, "%s", type);
				char version[SHORT_STR_LEN] = {0, };
				printf("Input version:");
				fscanf(stdin, "%s", version);

				if(client->requestFirmwareUpgradeComplete(THINGNAME, type, version) != ERR_OK)
					fprintf(stdout, "[RESULT:NOT OK] Failed to send upgrade complete signal.\n");
			}
			break;
//------------------------------------------------------------------------------------------------------------------
			case 13:
			{
				fprintf(stdout, "(WBC Encrypted MC 요청)\n");
				
				if(client->requestWbcMc() != ERR_OK)
					fprintf(stdout, "[RESULT:NOT OK] Failed to send request of wbc encrypted mc.\n");
			}
			break;
//------------------------------------------------------------------------------------------------------------------
			case 90:
			{
				fprintf(stdout, "(File Upload Uri 요청 신호 전송)\n");

				if(client->requestFileUploadUri() != ERR_OK)
					fprintf(stdout, "[RESULT:NOT OK] Failed to send request of file upload uri signal.\n");
			}
			break;
//------------------------------------------------------------------------------------------------------------------
			case 91:
			{
				fprintf(stdout, "(File Upload Complete 신호 전송)\n");
				char uri[SHORT_STR_LEN] = {0, };
				printf("Input upload uri:");
				fscanf(stdin, "%s", uri);
				char fileName[SHORT_STR_LEN] = {0, };
				printf("Input file name:");
				fscanf(stdin, "%s", fileName);

				if(client->requestFileUploadComplete(fileName, uri) != ERR_OK)
					fprintf(stdout, "[RESULT:NOT OK] Failed to send file upload complete signal.\n");
			}
			break;
//------------------------------------------------------------------------------------------------------------------
			case 92:		// send attributes
			{
				fprintf(stdout, "(Send Attributes of Root Thing)\n");

				char dataStr[LONG_STR_LEN] = {0, };

				int temperature = 0;
				int humidity = 0;

				fprintf(stdout, "Input Temperature attribute (integer):");
				fscanf(stdin, "%d", &temperature);
				fprintf(stdout, "Input Humidity attribute (integer):");
				fscanf(stdin, "%d", &humidity);

				// can see these attributes on "Thing Model Management" menu in Service Portal
				char* baseStr = "{\"temperature\": %d, \"humidity\": %d}";   // for SampleModel.120

				// put values into base string
				sprintf(dataStr, baseStr, temperature, humidity);

				// send attributes data for Root thing
				int rc = client->sendAttributesSync(NULL, dataStr);
				fprintf(stdout, "[>>>] Send (%s), ret(%d)\n", dataStr, rc);
			}
			break;
//------------------------------------------------------------------------------------------------------------------
			case 93:	// send attributes of leaf thing
			{
				fprintf(stdout, "(Send Attributes of Leaf Thing)\n");


				char thingName[SHORT_STR_LEN] = {0, };
				fprintf(stdout, "Input leaf thing name:");
				fscanf(stdin, "%s", thingName);

				char dataStr[LONG_STR_LEN] = {0, };

				int temperature = 0;
				int humidity = 0;
				fprintf(stdout, "Input Temperature attribute (integer):");
				fscanf(stdin, "%d", &temperature);
				fprintf(stdout, "Input Humidity attribute (integer):");
				fscanf(stdin, "%d", &humidity);

				// can see these attributes on "Thing Model Management" menu in Service Portal
				char* baseStr = "{\"temperature\": %d, \"humidity\": %d}";

				// put values into base string
				sprintf(dataStr, baseStr, temperature, humidity);

				// send attributes data for Root thing
				int rc = client->sendAttributesSyncForLeaf(thingName, NULL, dataStr);
				fprintf(stdout, "[>>>] Send (%s), ret(%d)\n", dataStr, rc);
			}
			break;
//------------------------------------------------------------------------------------------------------------------
			case 11:		// disconnect
				client->disconnect();
				break;
//------------------------------------------------------------------------------------------------------------------
#if 0
			case 15:		// activate leaf thing
				break;
#endif
//------------------------------------------------------------------------------------------------------------------
			case -1: 	// (exit) : do nothing
				break;
			default:
				fprintf(stdout, "you selected a wrong number.\n");
				break;
//------------------------------------------------------------------------------------------------------------------
		}	
	}
	
}

