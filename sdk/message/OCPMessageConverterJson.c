/*========================================================
 * 시스템  명
 * 프로그램ID
 * 프로그램명
 * 개　　　요
 * 변 경 일자
=========================================================*/

#include "OCPMessageConverterJson.h"
#include "OCPUtils.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <jansson.h>

//
static char* getObjectStrValue(json_t* object, char* key);
//
static char* getCharStrValue(json_t* object, char* key);
//
static double getDoubleValue(json_t* object, char* key);

/**
 * 
 * 
 * @param 
 * @return 
 */
void OCPMessageConverterJson_messageToPayload(OCPMessage* message, char** payload, int* payloadlen)
{
	json_t* object = json_object();
	json_object_set_new(object, OCPMESSAGE_KEY_VERSION, json_string(message->version));
	json_object_set_new(object, OCPMESSAGE_KEY_SID, json_string(message->siteId));
	json_object_set_new(object, OCPMESSAGE_KEY_TPID, json_string(message->thingName));
	json_object_set_new(object, OCPMESSAGE_KEY_ENCTYPE, json_string(message->encType));
	json_object_set_new(object, OCPMESSAGE_KEY_AUTHTOKEN, json_string(message->authToken));
	json_object_set_new(object, OCPMESSAGE_KEY_MSGTYPE, json_string(message->msgType));
	json_object_set_new(object, OCPMESSAGE_KEY_FUNCTYPE, json_string(message->funcType));
	json_object_set_new(object, OCPMESSAGE_KEY_MSGID, json_string(message->msgId));
	json_object_set_new(object, OCPMESSAGE_KEY_MSGCODE, json_string(message->msgCode));
	json_object_set_new(object, OCPMESSAGE_KEY_DATAFORMAT, json_string(message->dataFormat));
	json_object_set_new(object, OCPMESSAGE_KEY_DATA, json_string((char *)message->data));
	json_object_set_new(object, OCPMESSAGE_KEY_TID, json_string(message->tId));
	json_object_set_new(object, OCPMESSAGE_KEY_MSGDATE, json_real(message->msgDate));
	json_object_set_new(object, OCPMESSAGE_KEY_RESCODE, json_string(message->resCode));
	json_object_set_new(object, OCPMESSAGE_KEY_RESMSG, json_string(message->resMsg));
	json_object_set_new(object, OCPMESSAGE_KEY_SEVERITY, json_string(message->severity));
	
	char* dumps = json_dumps(object, 0);
	json_decref(object);

	if(dumps == NULL)
	{
		OCPUilts_log(OCPMESSAGECONVERTER_TRACE, "[OCPMessageConverterJson_messageToPayload] json_dumps is NULL\n");
		return;
	}

	int len = strlen(dumps);
	*payload = realloc(*payload, len + 1);
	if(*payload == NULL)
	{
		OCPUilts_log(OCPMESSAGECONVERTER_TRACE, "[OCPMessageConverterJson_messageToPayload] realloc *payload is NULL\n");
		return;
	}
	memset(*payload, 0, len * sizeof(char) + 1);
	memcpy(*payload, dumps, len);

	len = strlen(*payload);
	*payloadlen = len;
	free(dumps);
	OCPUilts_log(OCPMESSAGECONVERTER_TRACE, "[OCPMessageConverterJson_messageToPayload] end \n- *payload : %s\n", *payload);
}

/**
 * 
 * 
 * @param 
 * @return 
 */
OCPMessage* OCPMessageConverterJson_messageFromPayload(char* payload)
{
	OCPUilts_log(OCPMESSAGECONVERTER_TRACE, "[OCPMessageConverterJson_messageFromPayload] start \n- payload : %s\n", payload);

	OCPMessage message = OCPMESSAGE_INITIALIZER;

	if(payload == NULL || strlen(payload) < 1)
	{
		OCPUilts_log(OCPMESSAGECONVERTER_TRACE, "[OCPMessageConverterJson_messageFromPayload] payload is NULL | payloadlen < 1\n");
		return NULL;
	}

	json_t* object = NULL;
	json_error_t error;
	
	object = json_loads(payload, 0, &error);
	if (NULL == object) 
	{
		OCPUilts_log(OCPMESSAGECONVERTER_TRACE, "[OCPMessageConverterJson_messageFromPayload] json_loads is NULL %d:%s @@\n", error.line, error.text);
		return NULL;
	}

	char* value = NULL;
	int len = 0;

	value = getCharStrValue(object, OCPMESSAGE_KEY_VERSION);
	memset(message.version, 0, sizeof(message.version));
	if(value != NULL)	
	{
		len = strlen(value);
		if(len >= OCPMESSAGE_LEN_VERSION) { len = OCPMESSAGE_LEN_VERSION - 1; }
		memcpy(message.version, value, len);
	}

	value = getCharStrValue(object, OCPMESSAGE_KEY_MSGTYPE);
	memset(message.msgType, 0, sizeof(message.msgType));
	if(value != NULL)	
	{
		len = strlen(value);
		if(len >= OCPMESSAGE_LEN_MSGTYPE) { len = OCPMESSAGE_LEN_MSGTYPE - 1; }
		memcpy(message.msgType, value, len);
	}

	value = getCharStrValue(object, OCPMESSAGE_KEY_FUNCTYPE);
	memset(message.funcType, 0, sizeof(message.funcType));
	if(value != NULL)	
	{
		len = strlen(value);
		if(len >= OCPMESSAGE_LEN_FUNCTYPE) { len = OCPMESSAGE_LEN_FUNCTYPE - 1; }
		memcpy(message.funcType, value, len);
	}

	value = getCharStrValue(object, OCPMESSAGE_KEY_SID);
	memset(message.siteId, 0, sizeof(message.siteId));
	if(value != NULL)	
	{
		len = strlen(value);
		if(len >= OCPMESSAGE_LEN_SID) { len = OCPMESSAGE_LEN_SID - 1; }
		memcpy(message.siteId, value, len);
	}

	value = getCharStrValue(object, OCPMESSAGE_KEY_TPID);
	memset(message.thingName, 0, sizeof(message.thingName));
	if(value != NULL)	
	{
		len = strlen(value);
		if(len >= OCPMESSAGE_LEN_TPID) { len = OCPMESSAGE_LEN_TPID - 1; }
		memcpy(message.thingName, value, len);
	}

	value = getCharStrValue(object, OCPMESSAGE_KEY_TID);
	memset(message.tId, 0, sizeof(message.tId));
	if(value != NULL)	
	{
		len = strlen(value);
		if(len >= OCPMESSAGE_LEN_TID) { len = OCPMESSAGE_LEN_TID - 1; }
		memcpy(message.tId, value, len);
	}

	value = getCharStrValue(object, OCPMESSAGE_KEY_MSGCODE);
	memset(message.msgCode, 0, sizeof(message.msgCode));
	if(value != NULL)	
	{
		len = strlen(value);
		if(len >= OCPMESSAGE_LEN_MSGCODE) { len = OCPMESSAGE_LEN_MSGCODE - 1; }
		memcpy(message.msgCode, value, len);
	}

	value = getCharStrValue(object, OCPMESSAGE_KEY_MSGID);
	memset(message.msgId, 0, sizeof(message.msgId));
	if(value != NULL)	
	{
		len = strlen(value);
		if(len >= OCPMESSAGE_LEN_MSGID) { len = OCPMESSAGE_LEN_MSGID - 1; }
		memcpy(message.msgId, value, len);
	}

	message.msgDate = (size_t) getDoubleValue(object, OCPMESSAGE_KEY_MSGDATE);

	value = getCharStrValue(object, OCPMESSAGE_KEY_RESCODE);
	memset(message.resCode, 0, sizeof(message.resCode));
	if(value != NULL)	
	{
		len = strlen(value);
		if(len >= OCPMESSAGE_LEN_RESCODE) { len = OCPMESSAGE_LEN_RESCODE - 1; }
		memcpy(message.resCode, value, len);
	}

	value = getCharStrValue(object, OCPMESSAGE_KEY_RESMSG);
	memset(message.resMsg, 0, sizeof(message.resMsg));
	if(value != NULL)	
	{
		len = strlen(value);
		if(len >= OCPMESSAGE_LEN_RESMSG) { len = OCPMESSAGE_LEN_RESMSG - 1; }
		memcpy(message.resMsg, value, len);
	}

	value = getCharStrValue(object, OCPMESSAGE_KEY_DATAFORMAT);
	memset(message.dataFormat, 0, sizeof(message.dataFormat));
	if(value != NULL)	
	{
		len = strlen(value);
		if(len >= OCPMESSAGE_LEN_DATAFORMAT) { len = OCPMESSAGE_LEN_DATAFORMAT - 1; }
		memcpy(message.dataFormat, value, len);
	}

	value = getCharStrValue(object, OCPMESSAGE_KEY_SEVERITY);
	memset(message.severity, 0, sizeof(message.severity));
	if(value != NULL)	
	{
		len = strlen(value);
		if(len >= OCPMESSAGE_LEN_SEVERITY) { len = OCPMESSAGE_LEN_SEVERITY - 1; }
		memcpy(message.severity, value, len);
	}

	value = getCharStrValue(object, OCPMESSAGE_KEY_ENCTYPE);
	memset(message.encType, 0, sizeof(message.encType));
	if(value != NULL)	
	{
		len = strlen(value);
		if(len >= OCPMESSAGE_LEN_ENCTYPE) { len = OCPMESSAGE_LEN_ENCTYPE - 1; }
		memcpy(message.encType, value, len);
	}

	value = getCharStrValue(object, OCPMESSAGE_KEY_AUTHTOKEN);
	memset(message.authToken, 0, sizeof(message.authToken));
	if(value != NULL)	
	{
		len = strlen(value);
		if(len >= OCPMESSAGE_LEN_AUTHTOKEN) { len = OCPMESSAGE_LEN_AUTHTOKEN - 1; }
		memcpy(message.authToken, value, len);
	}

	if(!strcmp(OCPENCTYPE_PLAIN, message.encType))
	{
		message.data = getObjectStrValue(object, OCPMESSAGE_KEY_DATA);
		if(message.data){
			message.datalen = strlen(message.data);
		}
		else{
			message.datalen = 0;
		}
	}
	else
	{
		value = getCharStrValue(object, OCPMESSAGE_KEY_DATA);
		if(value != NULL)	
		{
			len = strlen(value);
			message.data = malloc(len + 1);
			memset(message.data, 0, len * sizeof(char) + 1);
			memcpy(message.data, value, len);
			message.datalen = len;
		}
		else 
		{
			message.data = NULL;
			message.datalen = 0;
		}
	}

	if(object != NULL)
	{
		json_decref(object);
		object = NULL;
	}

	OCPUilts_log(OCPMESSAGECONVERTER_TRACE, "@@ OCPMessageConverterJson_messageFromPayload END @@\n");
	// return message;

	// return message;
	OCPMessage* messageptr = malloc(sizeof(OCPMessage));
	memset(messageptr, 0, sizeof(OCPMessage));
	memcpy(messageptr, &message, sizeof(OCPMessage));

// fprintf(stderr, "@@@@@ &message : %p\n", &message);	
// fprintf(stderr, "@@@@@ messageptr : %p\n", messageptr);		
	return messageptr;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
void OCPMessageConverterJson_toCharPointer(void* object, char** data)
{
	OCPUilts_log(OCPMESSAGECONVERTER_TRACE, "@@ OCPMessageConverterJson_toCharPointer START @@\n");

	if(object == NULL)
	{
		OCPUilts_log(OCPMESSAGECONVERTER_TRACE, "@@ OCPMessageConverterJson_toCharPointer FAIL, because [object] is null @@\n");
		return;
	}

	char* dumps = json_dumps((json_t*)object, 0);
	if(dumps == NULL)
	{
		OCPUilts_log(OCPMESSAGECONVERTER_TRACE, "@@ OCPMessageConverterJson_toCharPointer FAIL, because json_dumps is NULL @@\n");
		return;
	}

	int len = strlen(dumps);
	char* temp = *data;
	*data = realloc(*data, len + 1);
	if(*data == NULL)
	{
		free(temp);
		OCPUilts_log(OCPMESSAGECONVERTER_TRACE, "@@ OCPMessageConverterJson_toCharPointer FAIL, because realloc payload is NULL @@\n");
		return;
	}
	memset(*data, 0, sizeof(char) * len + 1);
	memcpy(*data, dumps, len);
	free(dumps);

	OCPUilts_log(OCPMESSAGECONVERTER_TRACE, "@@ OCPMessageConverterJson_toCharPointer END @@\n");
}

/**
 * 
 * 
 * @param 
 * @return 
 */
ocp_json_t* OCPMessageConverterJson_fromCharPointer(const char* jsonString)
{
	OCPUilts_log(OCPMESSAGECONVERTER_TRACE, "@@ OCPMessageConverterJson_fromCharPointer START @@\n");
	json_t* object = NULL;

	json_error_t error;

	if (NULL == (object = json_loads(jsonString, 0, &error))) {
		OCPUilts_log(OCPMESSAGECONVERTER_TRACE, "@@ OCPMessageConverterJson_fromCharPointer FAIL because json_loads is NULL, line %d:%s @@\n", error.line, error.text);
		return NULL;
	}

	OCPUilts_log(OCPMESSAGECONVERTER_TRACE, "@@ OCPMessageConverterJson_fromCharPointer END @@\n");
	return object;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
ocp_json_t* OCPMessageConverterJson_object()
{
	return json_object();
}

/**
 * 
 * 
 * @param 
 * @return 
 */
void OCPMessageConverterJson_delete(ocp_json_t* object)
{
	json_decref((json_t *)object);
}

/**
 * 
 * 
 * @param 
 * @return 
 */
void OCPMessageConverterJson_setStringValue(ocp_json_t* object, const char *key, char* value)
{
	json_object_set_new((json_t*)object, key, json_string(value));
}

/**
 * 
 * 
 * @param 
 * @return 
 */
char* OCPMessageConverterJson_getStringValue(ocp_json_t* object, const char* key)
{
	OCPUilts_log(OCPMESSAGECONVERTER_TRACE, "@@ OCPMessageConverterJson_getStringValue START @@\n");
	char* value = NULL;
	if (object == NULL) 
	{
		OCPUilts_log(OCPMESSAGECONVERTER_TRACE, "@@ OCPMessageConverterJson_getStringValue FAIL because object is NULL @@\n");
		return NULL;
	}

	json_t* json = NULL;
	if (NULL == (json = json_object_get(object, key))) 
	{
		OCPUilts_log(OCPMESSAGECONVERTER_TRACE, "@@ OCPMessageConverterJson_getStringValue FAIL because json_object_get is NULL @@\n");
		return NULL;
	}
	if (NULL == (value = (char*)json_string_value(json))) 
	{
		OCPUilts_log(OCPMESSAGECONVERTER_TRACE, "@@ OCPMessageConverterJson_getStringValue FAIL because json_string_value is NULL @@\n");
		return NULL;
	}

	OCPUilts_log(OCPMESSAGECONVERTER_TRACE, "@@ OCPMessageConverterJson_getStringValue END @@\n");
	return value;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
int OCPMessageConverterJson_getIntValue(ocp_json_t* object, const char* key)
{
	int value = 0;
	
	if (object == NULL) 
	{
		OCPUilts_log(OCPMESSAGECONVERTER_TRACE, "@@ getIntValue FAIL because object is NULL @@\n");
		return value;
	}

	json_t* json = NULL;
	if (NULL == (json = json_object_get(object, key))) 
	{
		OCPUilts_log(OCPMESSAGECONVERTER_TRACE, "@@ getIntValue FAIL because json_object_get is NULL @@\n");
		return value;
	}

	value = (int)json_integer_value(json);
	return value;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
static char* getObjectStrValue(json_t* object, char* key)
{
	char* value = NULL;

	if (object == NULL) 
	{
		OCPUilts_log(OCPMESSAGECONVERTER_TRACE, "@@ getObjectStrValue FAIL because object is NULL @@\n");
		return NULL;
	}

	json_t* json = NULL;
	if (NULL == (json = json_object_get(object, key))) 
	{
		OCPUilts_log(OCPMESSAGECONVERTER_TRACE, "@@ getObjectStrValue FAIL because json_object_get is NULL @@\n");
		return NULL;
	}

	value = json_dumps(json, 0);
	return value;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
static char* getCharStrValue(json_t* object, char* key)
{
	char* value = NULL;
	
	if (object == NULL) 
	{
		OCPUilts_log(OCPMESSAGECONVERTER_TRACE, "@@ getCharStrValue FAIL because object is NULL @@\n");
		return NULL;
	}

	json_t* json = NULL;
	if (NULL == (json = json_object_get(object, key))) 
	{
		OCPUilts_log(OCPMESSAGECONVERTER_TRACE, "@@ getCharStrValue FAIL because json_object_get is NULL @@\n");
		return NULL;
	}

	if (NULL == (value = (char*)json_string_value(json))) 
	{
		OCPUilts_log(OCPMESSAGECONVERTER_TRACE, "@@ getCharStrValue FAIL because json_string_value is NULL @@\n");
		return NULL;
	}

	return value;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
static double getDoubleValue(json_t* object, char* key)
{
	double value = 0;
	
	if (object == NULL) 
	{
		OCPUilts_log(OCPMESSAGECONVERTER_TRACE, "@@ getDoubleValue FAIL because object is NULL @@\n");
		return value;
	}

	json_t* json = NULL;
	if (NULL == (json = json_object_get(object, key))) 
	{
		OCPUilts_log(OCPMESSAGECONVERTER_TRACE, "@@ getDoubleValue FAIL because json_object_get is NULL @@\n");
		return value;
	}

	value = (double)json_real_value(json);
	return value;
}