/*========================================================
 * 시스템  명
 * 프로그램ID
 * 프로그램명
 * 개　　　요
 * 변 경 일자
=========================================================*/

#include "OCPThingManager.h"
#include "OCPProtocolMqtt.h"
#include "OCPProtocolCurl.h"
#include "OCPMessageConverterJson.h"
#include "OCPMessageConverterDelimeter.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//
static void onConnectClient(void* context);
//
static void onDisconnectClient(void* context);
//
static void onMessageArriveClient(void* context, char* response);
//
static void onConnectionClosedClient(void* context, char* cause);
//
static OCPMessage* ocpGenerateMessage(OCPManager handle
	, char* msgType, char* funcType, char* msgId, char* msgCode, char* tId
	, char* encType, char* dataformat, void* data, int datalen, char* resCode);

typedef struct 
{
	void* context;
	OCPManager_onMessageArrive* ma;
	OCPManager_onConnectionClosed* cl;
	char version[OCPMANAGER_PROPERTY_LEN];
	char siteId[OCPMANAGER_PROPERTY_LEN];
	char thingName[OCPMANAGER_PROPERTY_LEN];
	int timeout;
	char protocolType[OCPMANAGER_PROPERTY_LEN];
	void* protocol;
	char msgHeaderType[OCPMANAGER_PROPERTY_LEN];
	char authCode[OCPMANAGER_PROPERTY_LEN];
	int minCompressionSize;
	char authToken[OCPMANAGER_PROPERTY_LEN];	
 	sem_t* sem;
}IOCPManager;

/**
 * 
 * 
 * @param 
 * @return 
 */
int OCPThingManager_create(OCPManager* handle, OCPManager_callback* callback, OCPManager_properties* properties)
{
 	if(*handle != NULL || properties == NULL)
 	{
 		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_create] [*handle] is not null or [properties] is null \n");
 		return OCPSDK_FAILURE;
 	}

 	IOCPManager *ocpManager = NULL;
 	ocpManager = malloc(sizeof(IOCPManager));
 	memset(ocpManager, '\0', sizeof(IOCPManager));
 	*handle = ocpManager; 	

 	//set callback
	ocpManager->context = *handle;
	if(callback != NULL)
	{
		ocpManager->ma = callback->ma;
		ocpManager->cl = callback->cl;
	}
	else
	{
		ocpManager->ma = NULL;
		ocpManager->cl = NULL;
	}

	//version
	memset(ocpManager->version, 0, sizeof(ocpManager->version));
 	memcpy(ocpManager->version, OCPSDK_VERSION, strlen(OCPSDK_VERSION));

	//siteId
	memset(ocpManager->siteId, 0, sizeof(ocpManager->siteId));
	strncpy(ocpManager->siteId, properties->siteId, strlen(properties->siteId));

	//thingName
	memset(ocpManager->thingName, 0, sizeof(ocpManager->thingName));
	strncpy(ocpManager->thingName, properties->thingName, strlen(properties->thingName));

	//timeout
	ocpManager->timeout = properties->timeout;

	//protocol
	//protocolType
	memset(ocpManager->protocolType, 0, sizeof(ocpManager->protocolType));
 	memcpy(ocpManager->protocolType, properties->protocolType, strlen(properties->protocolType));

	ocpManager->protocol = NULL;
	if(ocpManager->protocol != NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_create] [ocpManager->protocol] is not null \n");
		OCPThingManager_destroy(handle);
		//return OCPSDK_FAILURE;
		return (4); 	// ERR_PROTOCOL_STILL_EXIST
	}
 	else if(!strcmp(OCPPROTOCOL_MQTT, ocpManager->protocolType))
 	{
 		OCPProtocol_properties protocolProperty = OCPPROTOCOL_PROPERTIES_INITIALIZER;	
		//sslOpts
		protocolProperty.sslOpts = properties->sslOpts;
		//host
		memset(protocolProperty.host, 0x00, sizeof(protocolProperty.host));
		strncpy(protocolProperty.host, properties->host, strlen(properties->host));
		//port
		memset(protocolProperty.port, 0x00, sizeof(protocolProperty.port));
		strncpy(protocolProperty.port, properties->port, strlen(properties->port));
		//trustStore
		memset(protocolProperty.trustStore, 0x00, sizeof(protocolProperty.trustStore));
		strncpy(protocolProperty.trustStore, properties->trustStore, strlen(properties->trustStore));
		//keyStore
		memset(protocolProperty.keyStore, 0x00, sizeof(protocolProperty.keyStore));
		strncpy(protocolProperty.keyStore, properties->keyStore, strlen(properties->keyStore));
		//keyPassword
		memset(protocolProperty.keyPassword, 0x00, sizeof(protocolProperty.keyPassword));
		strncpy(protocolProperty.keyPassword, properties->keyPassword, strlen(properties->keyPassword));
		//keepAliveInterval
		protocolProperty.keepAliveInterval = properties->keepAliveInterval;
		//timeout
		protocolProperty.timeout = properties->timeout;
		//siteId
		memset(protocolProperty.siteId, 0x00, sizeof(protocolProperty.siteId));
		strncpy(protocolProperty.siteId, properties->siteId, strlen(properties->siteId));
		//thingName
		memset(protocolProperty.thingName, 0x00, sizeof(protocolProperty.thingName));
		strncpy(protocolProperty.thingName, properties->thingName, strlen(properties->thingName));
		//publishTopic
		memset(protocolProperty.publishTopic, 0x00, sizeof(protocolProperty.publishTopic));
		strncpy(protocolProperty.publishTopic, properties->publishTopic, strlen(properties->publishTopic));
		//subscribeTopic
		memset(protocolProperty.subscribeTopic, 0x00, sizeof(protocolProperty.subscribeTopic));
		strncpy(protocolProperty.subscribeTopic, properties->subscribeTopic, strlen(properties->subscribeTopic));
		//messageQos = 1 fixed 
		//password
		memset(protocolProperty.password, 0, sizeof(protocolProperty.password));
		strncpy(protocolProperty.password, properties->authCode, strlen(properties->authCode));

		OCPProtocol_callback cb;
		cb.cn = onConnectClient;
		cb.dc = onDisconnectClient;
 		cb.ma = onMessageArriveClient;
 		cb.cl = onConnectionClosedClient;
 		if(OCPSDK_SUCCESS != OCPProtocolMqtt_create(&ocpManager->protocol, &protocolProperty, ocpManager, &cb))
		{
			OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_create] [OCPProtocolMqtt_create] fail \n");			
			OCPThingManager_destroy(handle);
			//return OCPSDK_FAILURE;
			return (5);	// ERR_CREATE_MQTT_FAIL
		}
 	}
 	else
 	{
 		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_create] unsupported [protocolType : %d] \n", ocpManager->protocolType);
 		OCPThingManager_destroy(handle);
		//return OCPSDK_FAILURE;
		return (6);	// ERR_UNSUPPORTED_PROTOCOL
 	}

 	// create message converter
 	memset(ocpManager->msgHeaderType, 0, sizeof(ocpManager->msgHeaderType));
 	memcpy(ocpManager->msgHeaderType, properties->msgHeaderType, strlen(properties->msgHeaderType));

 	memset(ocpManager->authCode, 0, sizeof(ocpManager->authCode));
 	memcpy(ocpManager->authCode, properties->authCode, strlen(properties->authCode));

 	ocpManager->minCompressionSize = properties->minCompressionSize;

	//authToken
	memset(ocpManager->authToken, 0, sizeof(ocpManager->authToken));

	ocpManager->sem = OCPThread_create_sem();
	
	return OCPSDK_SUCCESS;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
void OCPThingManager_destroy(OCPManager* handle)
{
	IOCPManager *ocpManager = *handle;
	if(ocpManager == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "@@ OCPProtocolMqtt_destroy FAIL, because [ocpManager] is null > already destroy @@\n");
		return;
	}

	memset(ocpManager->version, 0, sizeof(ocpManager->version));
	ocpManager->ma = NULL;
	ocpManager->cl = NULL;
	if(ocpManager->protocol != NULL)
 	{		
		OCPProtocolMqtt_destroy(&ocpManager->protocol);
 		ocpManager->protocol = NULL;
 	}

	if(ocpManager->sem != NULL)
	{
		OCPThread_destroy_sem(ocpManager->sem);	
		ocpManager->sem = NULL;
	}

	if(ocpManager != NULL)
	{
		free(ocpManager);
		ocpManager = NULL;
	}

	*handle = NULL;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
int OCPThingManager_set_authToken(OCPManager handle, char* authToken)
{
	IOCPManager *ocpManager = handle;
	if(ocpManager == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_set_authToken] [ocpManager] is null \n");			
		return OCPSDK_FAILURE;
	}

	if(authToken == NULL || strlen(authToken) < 1)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_set_authToken] [authToken] is null \n");			
		return OCPSDK_FAILURE;	
	}

	memset(ocpManager->authToken, 0, 1024 * sizeof(char));
	memcpy(ocpManager->authToken, authToken, strlen(authToken));	

	return OCPSDK_SUCCESS;
}

int OCPThingManager_set_callback(OCPManager handle, OCPManager_callback* callback)
{
	IOCPManager *ocpManager = handle;
	if(ocpManager == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_set_callback] [ocpManager] is null \n");			
		return OCPSDK_FAILURE;
	}
	if(callback != NULL)
	{
		ocpManager->ma = callback->ma;
		ocpManager->cl = callback->cl;
	}
	return OCPSDK_SUCCESS;
}

char* OCPThingManager_get_message_header_type(OCPManager handle)
{
	IOCPManager *ocpManager = handle;
	if(ocpManager == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_get_message_header_type] [ocpManager] is null \n");			
		return NULL;
	}

	return ocpManager->msgHeaderType;
}

char* OCPThingManager_get_thing_name(OCPManager handle)
{
	IOCPManager *ocpManager = handle;
	if(ocpManager == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_get_thing_name] [ocpManager] is null \n");			
		return NULL;
	}

	return ocpManager->thingName;
}

char* OCPThingManager_get_auth_code(OCPManager handle)
{
	IOCPManager *ocpManager = handle;
	if(ocpManager == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_get_auth_code] [ocpManager] is null \n");			
		return NULL;
	}

	return ocpManager->authCode;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
int OCPThingManager_isconnected(OCPManager handle)
{
	IOCPManager *ocpManager = handle;
	if(ocpManager == NULL || ocpManager->protocol == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_isconnected] [ocpManager || ocpManager->protocol] is null \n");			
		return OCPSDK_FAILURE;
	}

	int rc = 0;
	if(ocpManager->protocolType == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_connect] [ocpManager->protocolType] is null \n");			
		return OCPSDK_FAILURE;
	}
	else if(!strcmp(OCPPROTOCOL_MQTT, ocpManager->protocolType)) //mqtt
	{
		rc = OCPProtocolMqtt_isconnected(ocpManager->protocol);
		if(OCPSDK_SUCCESS != rc)
		{
			OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_isconnected] [%s+%s] [OCPProtocolMqtt_isconnected] rc [%d] \n", ocpManager->siteId, ocpManager->thingName, rc);			
			return OCPSDK_FAILURE;
		}
		//send heartbeat & sync ? 
	}
	else
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_connect] unsupported [protocolType : %d] \n", ocpManager->protocolType);
		return OCPSDK_FAILURE;
	}
	return OCPSDK_SUCCESS;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
int OCPThingManager_connect(OCPManager handle)
{
	IOCPManager *ocpManager = handle;
	if(ocpManager == NULL || ocpManager->protocol == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_connect] [ocpManager || ocpManager->protocol] is null \n");			
		//return OCPSDK_FAILURE;
		return 8;	// ERR_PROTOCOL_NOT_CREATED
	}

	if(OCPSDK_SUCCESS == OCPThingManager_isconnected(ocpManager))
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_connect] [OCPThingManager_isconnected] success > already connect \n");			
		return OCPSDK_SUCCESS;
	}

	int rc;
	if(ocpManager->protocolType == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_connect] [ocpManager->protocolType] is null \n");			
		//return OCPSDK_FAILURE;
		return 9;	// ERR_PROTOCOL_TYPE_NOT_SET
	}
	else if(!strcmp(OCPPROTOCOL_MQTT, ocpManager->protocolType)) //mqtt
	{
		rc = OCPProtocolMqtt_connect(ocpManager->protocol);
		if(OCPSDK_SUCCESS != rc)
		{
			OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_connect] [%s+%s] [OCPProtocolMqtt_connect] fail, rc : [%d]  \n", ocpManager->siteId, ocpManager->thingName, rc);
			//return OCPSDK_FAILURE;
			return 10;	// ERR_MQTT_CONNECT_FAIL
		}
		OCPThread_wait_sem(ocpManager->sem, ocpManager->timeout);
// fprintf(stderr, "@@ timeout : %d\n", ocpManager->timeout);
		//subscribe
		if(OCPSDK_SUCCESS != (rc = OCPProtocolMqtt_subscribe(ocpManager->protocol)))
		{
			OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_connect] [%s+%s] [OCPProtocolMqtt_subscribe] fail, rc : [%d]  \n", ocpManager->siteId, ocpManager->thingName, rc);
			//return OCPSDK_FAILURE;
			return 11; 	// ERR_MQTT_SUBSCRIBE_FAIL
		}
		OCPThread_wait_sem(ocpManager->sem, ocpManager->timeout);
	}
	else
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_connect] unsupported [protocolType : %d] \n", ocpManager->protocolType);
		//return OCPSDK_FAILURE;
		return 6;	// ERR_UNSUPPORTED_PROTOCOL
	}
	ocpManager->authToken[0] = '\0';

	return OCPSDK_SUCCESS;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
int OCPThingManager_disconnect(OCPManager handle)
{
	IOCPManager *ocpManager = handle;
	if(ocpManager == NULL || ocpManager->protocol == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_disconnect] [ocpManager || ocpManager->protocol] is null \n");			
		//return OCPSDK_FAILURE;
		return 8;	// ERR_PROTOCOL_NOT_CREATED
	}

	//socket disconnect 
	if(OCPSDK_SUCCESS != OCPThingManager_isconnected(ocpManager))
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_disconnect] [OCPThingManager_isconnected] != OCPSDK_SUCCESS > already disconnect \n");			
		return OCPSDK_SUCCESS;
	}

	// int rc;
	if(ocpManager->protocolType == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_disconnect] [ocpManager->protocolType] is null \n");			
		//return OCPSDK_FAILURE;
		return 9;	// ERR_PROTOCOL_TYPE_NOT_SET
	}
	else if(!strcmp(OCPPROTOCOL_MQTT, ocpManager->protocolType)) 
	{
		if(OCPSDK_SUCCESS != OCPProtocolMqtt_disconnect(ocpManager->protocol))
		{
			OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_disconnect] [OCPProtocolMqtt_disconnect] != OCPSDK_SUCCESS \n");			
			//return OCPSDK_FAILURE;
			return 12; 	// ERR_MQTT_DISCONNECT_FAIL
		}
	}
	else
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_disconnect] unsupported [protocolType : %d] \n", ocpManager->protocolType);
		//return OCPSDK_FAILURE;
		return 6;	// ERR_UNSUPPORTED_PROTOCOL
	}

	OCPThread_wait_sem(ocpManager->sem, ocpManager->timeout);

	return OCPSDK_SUCCESS;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
OCPPreauthorize_result OCPThingManager_parse_preauthorize_result(OCPManager handle, char* authType, char* response)
{
	OCPPreauthorize_result preauthroize_result = OCPPREAUTHORIZE_RESULT_INITIALIZER;

	IOCPManager *ocpManager = handle;
	if(ocpManager == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_parse_preauthorize_result] [ocpManager] is null \n");
		return preauthroize_result;
	}	

	ocp_json_t* object = OCPMessageConverterJson_fromCharPointer(response);
	if(object == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_parse_preauthorize_result] OCPMessageConverterJson_fromCharPointer  OBJECT fail\n");		
		return preauthroize_result;
	}

	if(!strcmp(OCPAUTHTYPE_WBC, authType))
	{
		char* apiKey = OCPMessageConverterJson_getStringValue(object, "apiKey");
		if(apiKey == NULL)
		{
			OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_parse_preauthorize_result] OCPMessageConverterJson_getStringValue  apiKey fail\n");					
			OCPMessageConverterJson_delete(object);
			return preauthroize_result;
		}
		memset(preauthroize_result.apiKey, 0, sizeof(char) * strlen(apiKey));
		memcpy(preauthroize_result.apiKey, apiKey, strlen(apiKey));

		char* issueTimestamp = OCPMessageConverterJson_getStringValue(object, "issueTimestamp");
		if(issueTimestamp == NULL)
		{
			OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_parse_preauthorize_result] OCPMessageConverterJson_getStringValue  issueTimestamp fail\n");								
			OCPMessageConverterJson_delete(object);
			return preauthroize_result;
		}
		memset(preauthroize_result.issueTimestamp, 0, sizeof(char) * strlen(issueTimestamp));
		memcpy(preauthroize_result.issueTimestamp, issueTimestamp, strlen(issueTimestamp));

		char* expireDate = OCPMessageConverterJson_getStringValue(object, "expireDate");
		if(expireDate == NULL)
		{
			OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_parse_preauthorize_result] OCPMessageConverterJson_getStringValue  expireDate fail\n");											
			OCPMessageConverterJson_delete(object);
			return preauthroize_result;
		}
		memset(preauthroize_result.expireDate, 0, sizeof(char) * strlen(expireDate));
		memcpy(preauthroize_result.expireDate, expireDate, strlen(expireDate));

		char* apiSecretKey = OCPMessageConverterJson_getStringValue(object, "apiSecretKey");
		if(apiSecretKey == NULL)
		{
			OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_parse_preauthorize_result] OCPMessageConverterJson_getStringValue  apiSecretKey fail\n");														
			OCPMessageConverterJson_delete(object);
			return preauthroize_result;
		}
		memset(preauthroize_result.apiSecretKey, 0, sizeof(char) * strlen(apiSecretKey));
		memcpy(preauthroize_result.apiSecretKey, apiSecretKey, strlen(apiSecretKey));

		char* mc = OCPMessageConverterJson_getStringValue(object, "mc");
		if(mc == NULL)
		{
			OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_parse_preauthorize_result] OCPMessageConverterJson_getStringValue  mc fail\n");																	
			OCPMessageConverterJson_delete(object);
			return preauthroize_result;
		}
		memset(preauthroize_result.data, 0, sizeof(char) * 16);
		memcpy(preauthroize_result.data, mc, strlen(mc));
	}
	else if(!strcmp(OCPAUTHTYPE_ITA, authType))
	{
		char* authCode = OCPMessageConverterJson_getStringValue(object, "authCode");
		if(authCode == NULL)
		{
			OCPMessageConverterJson_delete(object);
			return preauthroize_result;
		}
		memset(preauthroize_result.data, 0, sizeof(char) * strlen(authCode));
		memcpy(preauthroize_result.data, authCode, strlen(authCode));
	}
	else if(!strcmp(OCPAUTHTYPE_SSL, authType))
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_parse_preauthorize_result] [authType] OCPAUTHTYPE_SSL \n");
	}
	else
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_parse_preauthorize_result] unsupported [authType %s] \n", authType);
	}
	
	OCPMessageConverterJson_delete(object);

	return preauthroize_result;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
OCPAuthorize_result OCPThingManager_parse_authorize_result(OCPManager handle, char* response)
{
	OCPAuthorize_result authorize_result = OCPAUTHORIZE_RESULT_INITIALIZER;

	IOCPManager *ocpManager = handle;
	if(ocpManager == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_parse_authorize_result] [ocpManager] is null \n");
		return authorize_result;
	}	

	ocp_json_t* object = OCPMessageConverterJson_fromCharPointer(response);
	if(object == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_parse_authorize_result] [OCPMessageConverterJson_fromCharPointer object] is null \n");
		return authorize_result;
	}

	char* encType = OCPMessageConverterJson_getStringValue(object, "encType");
	if(encType == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_parse_authorize_result] [OCPMessageConverterJson_getStringValue encType] is null \n");
		return authorize_result;
	}
	memset(authorize_result.encType, 0, sizeof(char) * strlen(encType));
	memcpy(authorize_result.encType, encType, strlen(encType));

	char* authToken = OCPMessageConverterJson_getStringValue(object, "authToken");
	if(authToken == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_parse_authorize_result] [OCPMessageConverterJson_getStringValue authToken] is null \n");
		return authorize_result;
	}
	memset(authorize_result.authToken, 0, sizeof(char) * strlen(authToken));
	memcpy(authorize_result.authToken, authToken, strlen(authToken));

	OCPMessageConverterJson_delete(object);

	return authorize_result;
}

char* OCPThingManager_generate_authorize_data(OCPManager handle, char* authCode)
{
	char* authroize_format = "{\"authCode\":\"%s\"}";

	char* data = malloc(strlen(authroize_format) + strlen(authCode)+ 1); 
	memset(data, 0, sizeof(char) * strlen(authroize_format) + sizeof(char) * strlen(authCode) + 1);
	sprintf(data, authroize_format, authCode);

    return data; 
}

/**
 * 
 * 
 * @param 
 * @return 
 */
static void onConnectClient(void* context)
{
	//called by connect & subscribe
 	IOCPManager *ocpManager = (IOCPManager *) context;
 	if(ocpManager == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[onConnectClient] [ocpManager] is null \n");			
		return;
	}

	if (!OCPThread_check_sem(ocpManager->sem)){
  		OCPThread_post_sem(ocpManager->sem);
	}
}

/**
 * 
 * 
 * @param 
 * @return 
 */
static void onDisconnectClient(void* context)
{
	//called by disconnect
 	IOCPManager *ocpManager = (IOCPManager *) context;
 	if(ocpManager == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[onDisconnectClient] [ocpManager] is null \n");			
		return;
	}

	if (!OCPThread_check_sem(ocpManager->sem)) {
  		OCPThread_post_sem(ocpManager->sem);
	}
}

/**
 * 
 * 
 * @param 
 * @return 
 */
static void onMessageArriveClient(void* context, char* response)
{
 	IOCPManager *ocpManager = (IOCPManager *) context;
 	if(ocpManager == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[onMessageArriveClient] [ocpManager] is null \n");		
		return;
	}

	OCPMessage* message = NULL;
	
	// step1 : response convert to message
	if(response != NULL)
	{
		if(!strcmp(OCPMESSAGECONVERTER_JSON, ocpManager->msgHeaderType))
		{
			message = OCPMessageConverterJson_messageFromPayload(response);
		}
		else if(!strcmp(OCPMESSAGECONVERTER_DELIMETER, ocpManager->msgHeaderType))
		{
			message = OCPMessageConverterDelimeter_messageFromPayload(response, ocpManager->minCompressionSize);
		}
		else
		{
			OCPUilts_log(OCPMANAGER_TRACE, "[onMessageArriveClient] unsupported msgHeaderType [%d] \n", ocpManager->msgHeaderType);
		}

 		free(response);
 		response = NULL;
	}

	if(*(ocpManager->ma) != NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[onMessageArriveClient] call ma\n");
		if(message != NULL)
		{
			(*(ocpManager->ma))(NULL, message);
		}
		else
		{
			(*(ocpManager->ma))(NULL, NULL);	
		}
	} 
}

/**
 * 
 * 
 * @param 
 * @return 
 */
static void onConnectionClosedClient(void* context, char* cause)
{
	OCPUilts_log(OCPMANAGER_TRACE, "@@ onConnectionClosedClient cause : %s @@\n", cause);
// fprintf(stderr, "%s\n", "onConnectionClosedClient start");	
	IOCPManager *ocpManager = (IOCPManager *) context;
 	if(ocpManager == NULL)
	{
// fprintf(stderr, "%s\n", "onConnectionClosedClient ocpManager is NULL ");			
		OCPUilts_log(OCPMANAGER_TRACE, "@@ onConnectionClosedClient FAIL, because ocpManager is NULL @@\n");
		return;
	}

	// if(*(((IOCPMqtt *) context)->cl))
 	if(*(ocpManager->cl) != NULL)
	// if(*(ocpManager->ma) != NULL)
 	{
// fprintf(stderr, "%s\n", "onConnectionClosedClient call cl ");			 		
 		OCPUilts_log(OCPMANAGER_TRACE, "@@ onConnectionClosedClient call cl @@\n");
		(*(ocpManager->cl))(ocpManager, cause);
 	}
}

/**
 * 
 * 
 * @param 
 * @return 
 */
char* OCPThingManager_sendMessage(OCPManager handle, OCPMessage* message)
{
	IOCPManager *ocpManager = handle;
	if(ocpManager == NULL || ocpManager->protocol == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_sendMessage] handle or handle->protocol is NULL \n");
		return NULL;
	}
	//mesasge convert to payload 
	char* payload = NULL;
	int payloadlen = 0;
	if(!strcmp(OCPMESSAGECONVERTER_JSON, ocpManager->msgHeaderType))
	{
		OCPMessageConverterJson_messageToPayload(message, &payload, &payloadlen); 
	}
	else if(!strcmp(OCPMESSAGECONVERTER_DELIMETER, ocpManager->msgHeaderType))
	{
		// 여기에 ,, 그 압축 여부를 써야 한다 
		payload = OCPMessageConverterDelimeter_messageToPayload(message, &payloadlen, ocpManager->minCompressionSize);
	}
	else
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_sendMessage] Unsupported msgHeaderType [%d]\n", ocpManager->msgHeaderType);
		return NULL;
	}

	//payload send
	if(!strcmp(OCPPROTOCOL_MQTT, ocpManager->protocolType)) //mqtt
	{
		if(OCPSDK_SUCCESS != OCPProtocolMqtt_send(ocpManager->protocol, (char*)payload, payloadlen))
		{
			OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_sendMessage] OCPProtocolMqtt_send return [!OCPSDK_SUCCESS] \n");
			if(payload != NULL) {
				free(payload);
				payload = NULL;
			}
			return NULL;
		}
	}
	else
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_sendMessage] Unsupported protocoltype [%d] \n", ocpManager->protocolType);
		if(payload != NULL) {
			free(payload);
			payload = NULL;
		}
		return NULL;
	}

	if(payload != NULL) 
	{
		free(payload);
		payload = NULL;
	}

	return message->msgId;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
OCPMessage* ocpGenerateMessage(OCPManager handle, char* msgType, char* funcType, char* msgId, char* msgCode, char* tId, char* encType, char* dataformat, void* data, int datalen, char* resCode)
{
	IOCPManager *ocpManager = handle;
	if(ocpManager == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[ocpGenerateMessage] [handle] is NULL \n");
		return NULL;
	}

	OCPMessage message = OCPMESSAGE_INITIALIZER;	

	char* value = NULL;
	int len = 0; 

	value = ocpManager->version;
	len = value == NULL ? 0 : strlen(value);
	// strncpy(message.version, value, len);
	memset(message.version, 0, sizeof(message.version));
	memcpy(message.version, value, len);

	value = msgType;
	len = value == NULL ? 0 : strlen(value);
	// strncpy(message.msgType, value, len);
	memset(message.msgType, 0, sizeof(message.msgType));
	memcpy(message.msgType, value, len);

	value = funcType;
	len = value == NULL ? 0 : strlen(value);
	// strncpy(message.funcType, value, len);
	memset(message.funcType, 0, sizeof(message.funcType));
	memcpy(message.funcType, value, len);

	value = ocpManager->siteId;
	len = value == NULL ? 0 : strlen(value);
	// strncpy(message.siteId, value, len);
	memset(message.siteId, 0, sizeof(message.siteId));
	memcpy(message.siteId, value, len);

	value = ocpManager->thingName;
	len = value == NULL ? 0 : strlen(value);
	// strncpy(message.thingName, value, len);
	memset(message.thingName, 0, sizeof(message.thingName));
	memcpy(message.thingName, value, len);

	value = tId;
	len = value == NULL ? 0 : strlen(value);
	// strncpy(message.tId, value, len);
	memset(message.tId, 0, sizeof(message.tId));
	memcpy(message.tId, value, len);

	value = msgCode;
	len = value == NULL ? 0 : strlen(value);
	// strncpy(message.msgCode, value, len);
	memset(message.msgCode, 0, sizeof(message.msgCode));
	memcpy(message.msgCode, value, len);

	value = msgId;
	len = value == NULL ? 0 : strlen(value);
	// strncpy(message.msgId, value, len);
	memset(message.msgId, 0, sizeof(message.msgId));
	memcpy(message.msgId, value, len);

	message.msgDate = OCPUtils_getCurrentTime();

	// message.resCode[0] = 0;
	value = resCode;
	len = value == NULL ? 0 : strlen(value);
	memset(message.resCode, 0, sizeof(message.resCode));
	memcpy(message.resCode, value, len);

	// message.resMsg[0] = 0;
	value = "";
	len = value == NULL ? 0 : strlen(value);
	memset(message.resMsg, 0, sizeof(message.resMsg));
	memcpy(message.resMsg, value, len);

	value = dataformat;
	len = value == NULL ? 0 : strlen(value);
	// strncpy(message.dataFormat, value, len);
	memset(message.dataFormat, 0, sizeof(message.dataFormat));
	memcpy(message.dataFormat, value, len);

	value = "0";
	len = value == NULL ? 0 : strlen(value);
	// strncpy(message.severity, value, len);
	memset(message.severity, 0, sizeof(message.severity));
	memcpy(message.severity, value, len);

	value = encType;
	len = value == NULL ? 0 : strlen(value);
	// strncpy(message.encType, value, len);	
	memset(message.encType, 0, sizeof(message.encType));
	memcpy(message.encType, value, len);

	value = ocpManager->authToken;
	len = value == NULL ? 0 : strlen(value);
	// strncpy(message.authToken, value, len);
	memset(message.authToken, 0, sizeof(message.authToken));
	memcpy(message.authToken, value, len);

	// if(data == NULL || datalen < 1)
	// {
	// 	message.data = NULL;
	// 	message.datalen = 0;
	// }
	// else
	// {
	// 	message.data = malloc(datalen + 1);
	// 	memset(message.data, 0, sizeof(char) * datalen + 1);
	// 	memcpy(message.data, data, datalen);			
	// 	message.datalen = datalen;
	// }

	OCPMessage* messageptr = malloc(sizeof(OCPMessage));
	memset(messageptr, 0, sizeof(OCPMessage));
	memcpy(messageptr, &message, sizeof(OCPMessage));

	if(data == NULL || datalen < 1)
	{
		messageptr->data = NULL;
		messageptr->datalen = 0;
	}
	else
	{
		messageptr->data = malloc(datalen + 1);
		memset(messageptr->data, 0, sizeof(char) * datalen + 1);
		memcpy(messageptr->data, data, datalen);			
		messageptr->datalen = datalen;
	}

	return messageptr;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
OCPMessage* OCPThingManager_get_preauthorize_message(OCPManager handle)
{
	IOCPManager *ocpManager = handle;
	if(ocpManager == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_get_preauthorize_message] [handle] is NULL \n");
		return NULL;
	}

	char* msgType = OCPMESSAGE_MSGTYPE_REQUEST;
	char* funcType = "001";
	char* msgId = OCPUtils_generateUUID(ocpManager);
	char* msgCode = OCPMESSAGE_MSGCODE_PRESAUTHPROCESS_REQ;	
	char* tId = "";
	char* encType = OCPENCTYPE_PLAIN;	
	char* dataformat = OCPDATAFORMAT_JSON;			
	char* data = "{\"body\":\"dummy\"}";	
	int datalen = strlen(data);
	char* resCode = "";

	OCPMessage* message = ocpGenerateMessage(ocpManager, msgType, funcType, msgId, msgCode, tId, encType, dataformat, data, datalen, resCode);
	if(msgId != NULL) {
		free(msgId);
	}
	return message;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
OCPMessage* OCPThingManager_get_authorize_message(OCPManager handle, char* encType, char* dataformat, void* data, int datalen)
{
	IOCPManager *ocpManager = handle;
	if(ocpManager == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_get_authorize_message] handle is NULL \n");
		return NULL;
	}

	char* msgType = OCPMESSAGE_MSGTYPE_REQUEST;
	char* funcType = "002";
	char* msgId = OCPUtils_generateUUID(ocpManager);
	char* msgCode = OCPMESSAGE_MSGCODE_AUTHPROCESS_REQ;
	char* tId = ocpManager->thingName;
	char* resCode = "";

	OCPMessage* message = ocpGenerateMessage(ocpManager, msgType, funcType, msgId, msgCode, tId, encType, dataformat, data, datalen, resCode);
	if(msgId != NULL) {
		free(msgId);
	}
	return message;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
OCPMessage* OCPThingManager_get_heartbeat_message(OCPManager handle)
{
	IOCPManager *ocpManager = handle;
	if(ocpManager == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_get_heartbeat_message] handle is NULL \n");
		return NULL;
	}

	char* msgType = OCPMESSAGE_MSGTYPE_REQUEST;
	char* funcType = "003";
	char* msgId = OCPUtils_generateUUID(ocpManager);
	char* msgCode = OCPMESSAGE_MSGCODE_SEND_HEARTBEAT_REQ;	
	char* tId = "";
	char* encType = OCPENCTYPE_PLAIN;	
	char* dataformat = OCPDATAFORMAT_JSON;		
	char* data = "{\"body\":\"dummy\"}";	
	int datalen = strlen(data);
	char* resCode = "";

	OCPMessage* message = ocpGenerateMessage(ocpManager, msgType, funcType, msgId, msgCode, tId, encType, dataformat, data, datalen, resCode);
	if(msgId != NULL) {free(msgId);}
	return message;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
OCPMessage* OCPThingManager_get_activate_rootthing_message(OCPManager handle, char* encType, char* dataformat, void* data, int datalen)
{
	IOCPManager *ocpManager = handle;
	if(ocpManager == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_get_activate_rootthing_message] handle is NULL \n");
		return NULL;
	}

	if(data == NULL || datalen < 1)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_get_activate_rootthing_message] data is NULL or datalen less than 1 \n");
		return NULL;
	}

	char* msgType = OCPMESSAGE_MSGTYPE_REQUEST;
	char* funcType = "011";
	char* msgId = OCPUtils_generateUUID(ocpManager);
	char* msgCode = OCPMESSAGE_MSGCODE_ACTIVATE_ROOTTHING_REQ;
	char* tId = ocpManager->thingName;
	char* resCode = "";

	OCPMessage* message = ocpGenerateMessage(ocpManager, msgType, funcType, msgId, msgCode, tId, encType, dataformat, data, datalen, resCode);
	if(msgId != NULL) {free(msgId);}
	return message;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
OCPMessage* OCPThingManager_get_register_edgething_message(OCPManager handle, char* encType, char* dataformat, void* data, int datalen, char* edgeThingName)
{
	IOCPManager *ocpManager = handle;
	if(ocpManager == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_get_register_edgething_message] handle is NULL \n");
		return NULL;
	}

	if(data == NULL || datalen < 1)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_get_register_edgething_message] data is NULL or datalen less than 1 \n");
		return NULL;
	}

 	char* msgType = OCPMESSAGE_MSGTYPE_REQUEST;
 	char* funcType = "011";
 	char* msgId = OCPUtils_generateUUID(ocpManager);
 	char* msgCode = OCPMESSAGE_MSGCODE_REGISTER_EDGETHING_REQ;
 	char* tId = edgeThingName;
 	char* resCode = "";

	OCPMessage* message = ocpGenerateMessage(ocpManager, msgType, funcType, msgId, msgCode, tId, encType, dataformat, data, datalen, resCode);
	if(msgId != NULL) {free(msgId);}
	return message;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
OCPMessage* OCPThingManager_get_inactivate_thing_message(OCPManager handle, char* thingName)
{
	IOCPManager *ocpManager = handle;
	if(ocpManager == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_get_inactivate_thing_message] handle is NULL \n");
		return NULL;
	}

	if(thingName == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_get_inactivate_thing_message] thingName is NULL \n");
		return NULL;
	}

 	char* msgType = OCPMESSAGE_MSGTYPE_REQUEST;
	char* funcType = "012";
	char* msgId = OCPUtils_generateUUID(ocpManager);
	char* msgCode = OCPMESSAGE_MSGCODE_INACTIVATE_THING_REQ;	
	char* tId = thingName;
	char* encType = OCPENCTYPE_PLAIN;
	char* dataformat = OCPDATAFORMAT_JSON;	
	char* data = "{\"body\":\"dummy\"}";	
	int datalen = strlen(data);
	char* resCode = "";

	OCPMessage* message = ocpGenerateMessage(ocpManager, msgType, funcType, msgId, msgCode, tId, encType, dataformat, data, datalen, resCode);
	if(msgId != NULL) {free(msgId);}
	return message;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
OCPMessage* OCPThingManager_get_register_data_message(OCPManager handle, char* encType, char* dataformat, void* data, int datalen, char* thingName, char* msgCode)
{
	IOCPManager *ocpManager = handle;
	if(ocpManager == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_get_register_data_message] handle is NULL \n");
		return NULL;
	}

	if(data == NULL || datalen < 1)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_get_register_data_message] data is NULL or datalen less than 1 \n");
		return NULL;
	}

	if(thingName == NULL || msgCode == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_get_register_data_message] thingName or msgCode is NULL \n");
		return NULL;
	}

 	char* msgType = OCPMESSAGE_MSGTYPE_REQUEST;
 	char* funcType = "021";
 	char* msgId = OCPUtils_generateUUID(ocpManager);
 	char* tId = thingName;
 	char* resCode = "";

	OCPMessage* message = ocpGenerateMessage(ocpManager, msgType, funcType, msgId, msgCode, tId, encType, dataformat, data, datalen, resCode);
	if(msgId != NULL) {free(msgId);}
	return message;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
OCPMessage* OCPThingManager_get_bulk_insert_message(OCPManager handle, char* encType, char* dataformat, void* data, int datalen)
{
	IOCPManager *ocpManager = handle;
	if(ocpManager == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_get_bulk_insert_message] [handle] is NULL \n");
		return NULL;
	}

	if(data == NULL || datalen < 1)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_get_bulk_insert_message] [data] is NULL or [datalen] < 1 \n");
		return NULL;
	}

 	char* msgType = OCPMESSAGE_MSGTYPE_REQUEST;
 	char* funcType = "021";
 	char* msgId = OCPUtils_generateUUID(ocpManager);
 	char* msgCode = OCPMESSAGE_MSGCODE_BULK_INSERT_REQ;
 	char* tId = ocpManager->thingName;
 	char* resCode = "";

	OCPMessage* message = ocpGenerateMessage(ocpManager, msgType, funcType, msgId, msgCode, tId, encType, dataformat, data, datalen, resCode);
	if(msgId != NULL) {free(msgId);}
	return message;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
OCPMessage* OCPThingManager_get_request_activefirmware_message(OCPManager handle, char* encType, char* dataformat, void* data, int datalen, char* thingName)
{
	IOCPManager *ocpManager = handle;
	if(ocpManager == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_get_request_activefirmware_message] handle is NULL \n");
		return NULL;
	}

	if(data == NULL || datalen < 1)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_get_request_activefirmware_message] data is NULL or datalen less than 1 \n");
		return NULL;
	}

	if(thingName == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_get_request_activefirmware_message] thingName is NULL \n");
		return NULL;
	}

 	char* msgType = OCPMESSAGE_MSGTYPE_REQUEST;
 	char* funcType = "042";
 	char* msgId = OCPUtils_generateUUID(ocpManager);
 	char* msgCode = OCPMESSAGE_MSGCODE_REQUEST_ACTIVEFIRMWARE_REQ;
	char* tId = thingName;
	char* resCode = "";

	OCPMessage* message = ocpGenerateMessage(ocpManager, msgType, funcType, msgId, msgCode, tId, encType, dataformat, data, datalen, resCode);
	if(msgId != NULL) {free(msgId);}
	return message;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
OCPMessage* OCPThingManager_get_response_passivefirmware_message(OCPManager handle, char* encType, char* dataformat, void* data, int datalen, char* msgId, char* thingName)
{
	IOCPManager *ocpManager = handle;
	if(ocpManager == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_get_response_passivefirmware_message] handle is NULL \n");
		return NULL;
	}

	if(data == NULL || datalen < 1)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_get_response_passivefirmware_message] data is NULL or datalen less than 1 \n");
		return NULL;
	}

	if(thingName == NULL || msgId == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_get_response_passivefirmware_message] thingName or msgId is NULL \n");
		return NULL;
	}
	char* msgType = OCPMESSAGE_MSGTYPE_ANSWER;
	char* funcType = "042";
	char* msgCode = OCPMESSAGE_MSGCODE_NOTI_PASSIVEFIRMWARE_RES;
	char* tId = thingName;
	char* resCode = "";

	OCPMessage* message = ocpGenerateMessage(ocpManager, msgType, funcType, msgId, msgCode, tId, encType, dataformat, data, datalen, resCode);

	return message;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
OCPMessage* OCPThingManager_get_signedfirmware_message(OCPManager handle, char* encType, char* dataformat, void* data, int datalen)
{
	IOCPManager *ocpManager = handle;
	if(ocpManager == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_get_signedfirmware_message] handle is NULL \n");
		return NULL;
	}

	if(data == NULL || datalen < 1)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_get_signedfirmware_message] data is NULL or datalen less than 1 \n");
		return NULL;
	}
	
 	char* msgType = OCPMESSAGE_MSGTYPE_REQUEST;
 	char* funcType = "042";
 	char* msgId = OCPUtils_generateUUID(ocpManager);
 	char* msgCode = OCPMESSAGE_MSGCODE_GET_SIGNEDFIRMWARE_REQ;
	char* tId = ocpManager->thingName;
	char* resCode = "";
	
	OCPMessage* message = ocpGenerateMessage(ocpManager, msgType, funcType, msgId, msgCode, tId, encType, dataformat, data, datalen, resCode);
	if(msgId != NULL) {free(msgId);}
	return message;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
OCPMessage* OCPThingManager_get_firmware_upgraderesult_message(OCPManager handle, char* encType, char* dataformat, void* data, int datalen, char* thingName, char* resCode)
{
	IOCPManager *ocpManager = handle;
	if(ocpManager == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_get_firmware_upgraderesult_message] handle is NULL \n");
		return NULL;
	}

	if(data == NULL || datalen < 1)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_get_firmware_upgraderesult_message] data is NULL or datalen less than 1 \n");
		return NULL;
	}

	if(thingName == NULL )
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_get_firmware_upgraderesult_message] thingName is NULL \n");
		return NULL;
	}

 	char* msgType = OCPMESSAGE_MSGTYPE_REQUEST;
 	char* funcType = "042";
 	char* msgId = OCPUtils_generateUUID(ocpManager);
 	char* msgCode = OCPMESSAGE_MSGCODE_SEND_FIRMWARERESULT_REQ;
	char* tId = thingName;
	// char* resCode = "";

	OCPMessage* message = ocpGenerateMessage(ocpManager, msgType, funcType, msgId, msgCode, tId, encType, dataformat, data, datalen, resCode);
	if(msgId != NULL) {free(msgId);}
	return message;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
OCPMessage* OCPThingManager_get_response_notirequest_message(OCPManager handle, char* encType, char* dataformat, void* data, int datalen, char* msgId)
{
	IOCPManager *ocpManager = handle;
	if(ocpManager == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_get_response_notirequest_message] handle is NULL \n");
		return NULL;
	}

	if(data == NULL || datalen < 1)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_get_response_notirequest_message] data is NULL or datalen less than 1 \n");
		return NULL;
	}

	if(msgId == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_get_response_notirequest_message] msgId is NULL \n");
		return NULL;
	}

 	char* msgType = OCPMESSAGE_MSGTYPE_ANSWER;
 	char* funcType = "030";
 	char* msgCode = OCPMESSAGE_MSGCODE_NOTIREQUEST_RES;
	char* tId = "";
	char* resCode = "";

	OCPMessage* message = ocpGenerateMessage(ocpManager, msgType, funcType, msgId, msgCode, tId, encType, dataformat, data, datalen, resCode);

	return message;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
OCPMessage* OCPThingManager_get_answer_message(OCPManager handle, char* encType, char* dataformat, void* data, int datalen, char* msgId, char* msgCode)
{
	IOCPManager *ocpManager = handle;
	if(ocpManager == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_get_answer_message] handle is NULL \n");
		return NULL;
	}

	if(data == NULL || datalen < 1)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_get_answer_message] data is NULL or datalen less than 1 \n");
		return NULL;
	}

	if(msgId == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_get_answer_message] msgId is NULL \n");
		return NULL;
	}

 	char* msgType = OCPMESSAGE_MSGTYPE_ANSWER;
 	char* funcType = "030";
 	// char* msgCode = OCPMESSAGE_MSGCODE_NOTIREQUEST_RES;
	char* tId = ocpManager->thingName;
	char* resCode = "";

	OCPMessage* message = ocpGenerateMessage(ocpManager, msgType, funcType, msgId, msgCode, tId, encType, dataformat, data, datalen, resCode);

	return message;
}

OCPMessage* OCPThingManager_get_fileuploaduri_request_message(OCPManager handle, char* encType, char* dataformat, void* data, int datalen, char* thingName)
{
	IOCPManager *ocpManager = handle;
	if(ocpManager == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_get_request_activefirmware_message] handle is NULL \n");
		return NULL;
	}

	// if(data == NULL || datalen < 1)
	// {
	// 	OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_get_request_activefirmware_message] data is NULL or datalen less than 1 \n");
	// 	return NULL;
	// }

	if(thingName == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_get_request_activefirmware_message] thingName is NULL \n");
		return NULL;
	}

 	char* msgType = OCPMESSAGE_MSGTYPE_REQUEST;
 	char* funcType = "042";
 	char* msgId = OCPUtils_generateUUID(ocpManager);
 	char* msgCode = OCPMESSAGE_MSGCODE_REQUEST_UPLOADURI_REQ;
	char* tId = thingName;
	char* resCode = "";

	OCPMessage* message = ocpGenerateMessage(ocpManager, msgType, funcType, msgId, msgCode, tId, encType, dataformat, data, datalen, resCode);
	if(msgId != NULL) {free(msgId);}
	return message;
}

OCPMessage* OCPThingManager_get_fileupload_result_message(OCPManager handle, char* encType, char* dataformat, void* data, int datalen, char* thingName)
{
	IOCPManager *ocpManager = handle;
	if(ocpManager == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_get_request_activefirmware_message] handle is NULL \n");
		return NULL;
	}

	if(data == NULL || datalen < 1)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_get_request_activefirmware_message] data is NULL or datalen less than 1 \n");
		return NULL;
	}

	if(thingName == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_get_request_activefirmware_message] thingName is NULL \n");
		return NULL;
	}

 	char* msgType = OCPMESSAGE_MSGTYPE_REQUEST;
 	char* funcType = "042";
 	char* msgId = OCPUtils_generateUUID(ocpManager);
 	char* msgCode = OCPMESSAGE_MSGCODE_SEND_UPLOADRESULT_REQ;
	char* tId = thingName;
	char* resCode = "";

	OCPMessage* message = ocpGenerateMessage(ocpManager, msgType, funcType, msgId, msgCode, tId, encType, dataformat, data, datalen, resCode);
	if(msgId != NULL) {free(msgId);}
	return message;
}

OCPMessage* OCPThingManager_get_wbc_mc(OCPManager handle, char* encType, char* dataformat, void* data, int datalen, char* thingName)
{
	IOCPManager *ocpManager = handle;
	if(ocpManager == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_get_request_activefirmware_message] handle is NULL \n");
		return NULL;
	}

	if(data == NULL || datalen < 1)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_get_request_activefirmware_message] data is NULL or datalen less than 1 \n");
		return NULL;
	}

	if(thingName == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPThingManager_get_request_activefirmware_message] thingName is NULL \n");
		return NULL;
	}

	char* msgType = OCPMESSAGE_MSGTYPE_REQUEST;
	char* funcType = "042";
	char* msgId = OCPUtils_generateUUID(ocpManager);
	char* msgCode = OCPMESSAGE_MSGCODE_WBC_MC_REQ;
	char* tId = thingName;
	char* resCode = "";

	OCPMessage* message = ocpGenerateMessage(ocpManager, msgType, funcType, msgId, msgCode, tId, encType, dataformat, data, datalen, resCode);
	if(msgId != NULL) {free(msgId);}
	return message;
}
