/*========================================================
 * 시스템  명
 * 프로그램ID
 * 프로그램명
 * 개　　　요
 * 변 경 일자
=========================================================*/

#include "OCPManager.h"
#include "OCPUtils.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * 
 * 
 * @param 
 * @return 
 */
void OCPManager_free_message(OCPMessage* message)
{
	if(message != NULL) 
	{ 
		if(message->data != NULL) 
		{ 
			free(message->data); 
			message->data = NULL; 
		}

		free(message); 
		message = NULL;
	}
}

/**
 * 
 * 
 * @param 
 * @return 
 */
void OCPManager_print_message(OCPMessage* message)
{
	OCPUilts_log(OCPMANAGER_TRACE, "[OCPManager_message_to_charpointer]\n");
	fprintf(stderr, ". message->version : %s\n", message->version);
	fprintf(stderr, ". message->msgType : %s\n", message->msgType);
	fprintf(stderr, ". message->funcType : %s\n", message->funcType);
	fprintf(stderr, ". message->siteId : %s\n", message->siteId);
	fprintf(stderr, ". message->thingName : %s\n", message->thingName);
	fprintf(stderr, ". message->tId : %s\n", message->tId);
	fprintf(stderr, ". message->msgCode : %s\n", message->msgCode);
	fprintf(stderr, ". message->msgId : %s\n", message->msgId);
	fprintf(stderr, ". message->msgDate : %llu\n", message->msgDate);
	fprintf(stderr, ". message->resCode : %s\n", message->resCode);
	fprintf(stderr, ". message->resMsg : %s\n", message->resMsg);
	fprintf(stderr, ". message->dataFormat : %s\n", message->dataFormat);
	fprintf(stderr, ". message->severity : %s\n", message->severity);
	fprintf(stderr, ". message->encType : %s\n", message->encType);
	fprintf(stderr, ". message->datalen : %llu\n", message->datalen);
	fprintf(stderr, ". message->data : %s\n", (char*)message->data);
	fprintf(stderr, ". message->authToken : %s\n", message->authToken);
}