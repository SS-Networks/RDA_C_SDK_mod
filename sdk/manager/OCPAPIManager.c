/*========================================================
 * 시스템  명
 * 프로그램ID
 * 프로그램명
 * 개　　　요
 * 변 경 일자
=========================================================*/

#include "OCPAPIManager.h"
#include "OCPUtils.h"
#include "OCPProtocolCurl.h"
#include "OCPMessageConverterJson.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//
#define HTTP_HEADER_AUTHORIZATION	"Authorization: UL %s"
//
#define ITA_USER_LOGIN 				"/v1.1/main/action/itaUserLogin"
//
#define ITA_THING_AUTHCODE 			"/v1.1/sites/%s/things/%s/module/%s/itaThingAuthCode?msgHeaderType=%s" //뒤에 추가로 더 밑여야 한다으아 
//
#define ROOT_THING_LIST 			"/v1.0/sites/%s/thingNames?type=root&limit=%u&offset=%u"
//
#define ALL_EDGE_THINGS 			"/v1.0/sites/%s/things/%s/allEdges"
//
#define DIRECT_EDGE_THINGS 			"/v1.0/sites/%s/things/%s/edges?limit=%u&offset=%u"
//
#define THING_ATTRIBUTES 			"/v1.0/sites/%s/things/%s/attrsInfo"
//
typedef struct 
{
 	void* protocol;
 	char userToken[OCPMANAGER_PROPERTY_LEN];
}IOCPAPIManager;

/**
 * 
 * 
 * @param 
 * @return 
 */
int OCPAPIManager_create(OCPManager* handle, OCPManager_properties* properties)
{
	if(*handle != NULL || properties == NULL)
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPAPIManager_create] *handle is not null or properties is null\n");
		return OCPSDK_FAILURE;
	}

	IOCPAPIManager *ocpAPIManager = NULL;
 	ocpAPIManager = malloc(sizeof(IOCPAPIManager));
 	memset(ocpAPIManager, '\0', sizeof(IOCPAPIManager));
 	*handle = ocpAPIManager;

	OCPProtocol_properties moduleProperty = OCPPROTOCOL_PROPERTIES_INITIALIZER ;

	//host
	memset(moduleProperty.host, 0, sizeof(moduleProperty.host));
	memcpy(moduleProperty.host, properties->host, strlen(properties->host));
	//port
	memset(moduleProperty.port, 0, sizeof(moduleProperty.port));
	memcpy(moduleProperty.port, properties->port, strlen(properties->port));
	//sslopts
	moduleProperty.sslOpts = properties->sslOpts;
	//trustStore
	memset(moduleProperty.trustStore, 0, sizeof(moduleProperty.trustStore));
	memcpy(moduleProperty.trustStore, properties->trustStore, strlen(properties->trustStore));
	//keyStore
	memset(moduleProperty.keyStore, 0, sizeof(moduleProperty.keyStore));
	memcpy(moduleProperty.keyStore, properties->keyStore, strlen(properties->keyStore));
	//keyPassword
	memset(moduleProperty.keyPassword, 0, sizeof(moduleProperty.keyPassword));
	memcpy(moduleProperty.keyPassword, properties->keyPassword, strlen(properties->keyPassword));
	//timeout
	moduleProperty.timeout = properties->timeout;
	//protocol
	ocpAPIManager->protocol = NULL;
	int rc = OCPProtocolCurl_create(&ocpAPIManager->protocol, &moduleProperty, ocpAPIManager);
 	if(rc != OCPSDK_SUCCESS)
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPAPIManager_create] OCPProtocolCurl_create rc : [%d]\n", rc);
		OCPAPIManager_destroy(handle);
		return OCPSDK_FAILURE;
	}

	memset(ocpAPIManager->userToken, 0, sizeof(ocpAPIManager->userToken));
	return OCPSDK_SUCCESS;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
void OCPAPIManager_destroy(OCPManager* handle)
{
	IOCPAPIManager *ocpAPIManager = *handle;
	if(ocpAPIManager == NULL)
	{
		OCPUilts_log(OCPMANAGER_TRACE, "[OCPAPIManager_destroy] ocpAPIManager is null > already destroy\n");
		return;
	}
	
	if(ocpAPIManager->protocol != NULL)
 	{		
 		OCPProtocolCurl_destroy(&ocpAPIManager->protocol);
 		ocpAPIManager->protocol = NULL;
 	}

 	memset(ocpAPIManager->userToken, 0, sizeof(ocpAPIManager->userToken));

	if(ocpAPIManager != NULL)
	{
		free(ocpAPIManager);
		ocpAPIManager = NULL;
	}

	*handle = NULL;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
int OCPAPIManager_islogined(OCPManager handle)
{
	IOCPAPIManager *ocpAPIManager = handle;
	if(ocpAPIManager == NULL || ocpAPIManager->protocol == NULL)
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPAPIManager_islogined] ocpAPIManager | ocpAPIManager->protocol is NULL\n");
		return OCPSDK_FAILURE;
	}

	if(ocpAPIManager->userToken[0] == 0)
	{
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
int OCPAPIManager_do_login(OCPManager handle, char* userId, char* userkey)
{
	IOCPAPIManager *ocpAPIManager = handle;
	if(ocpAPIManager == NULL || ocpAPIManager->protocol == NULL)
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPAPIManager_do_login] ocpAPIManager | ocpAPIManager->protocol is NULL\n");
		return OCPSDK_FAILURE;
	}

	if(userId == NULL || userkey == NULL)
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPAPIManager_do_login] userId | userkey is NULL\n");
		return OCPSDK_FAILURE;
	}

	char* dataformat = "{\"userId\":\"%s\",\"userPassword\":\"%s\"}";
	char* hashed = NULL;
	OCPUtils_getHash(userkey, &hashed);
	// char* dataString = malloc(strlen(dataformat) + strlen(userId) + strlen(hashed) + 1);
	char dataString[OCPMANAGER_PROPERTY_LEN] = {0, };
	sprintf(dataString, dataformat, userId, hashed);

	char* result = OCPProtocolCurl_send(ocpAPIManager->protocol, ITA_USER_LOGIN, HTTP_HEADER_AUTHORIZATION, dataString);
	// if(dataString != NULL){
	// 	free(dataString);
	// 	dataString = NULL;
	// }

	OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPAPIManager_do_login] OCPProtocolCurl_send result : %s \n", result);
	if(result == NULL)
	{
		if(hashed != NULL) 
		{
			free(hashed);
			hashed = NULL;
		}
		// if(dataString != NULL) 
		// {
		// 	free(dataString);
		// 	dataString = NULL;
		// }
		if(result != NULL) 
		{
			free(result);
			result = NULL;
		}
		return OCPSDK_FAILURE;
	}

	ocp_json_t* object = OCPMessageConverterJson_fromCharPointer(result);
	if(object == NULL)
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPAPIManager_do_login] OCPMessageConverterJson_fromCharPointer return NULL\n");
		if(hashed != NULL) {
			free(hashed);
			hashed = NULL;
		}
		// if(dataString != NULL) {
		// 	free(dataString);
		// 	dataString = NULL;
		// }
		if(result != NULL) {
			free(result);
			result = NULL;
		}
		return OCPSDK_FAILURE;
	}

	char* token = OCPMessageConverterJson_getStringValue(object, "authToken");
	if(token == NULL)
	{
		if(hashed != NULL) {
			free(hashed);
			hashed = NULL;
		}
		// if(dataString != NULL) {
		// 	free(dataString);
		// 	dataString = NULL;
		// }
		if(result != NULL) {
			free(result);
			result = NULL;
		}
		if(object != NULL) {
			OCPMessageConverterJson_delete(object);
			object = NULL;
		}
		return OCPSDK_FAILURE;
	}

	
	memset(ocpAPIManager->userToken, 0, sizeof(ocpAPIManager->userToken));
	memcpy(ocpAPIManager->userToken, token, strlen(token));

	if(object != NULL) {
		OCPMessageConverterJson_delete(object);
		object = NULL;
	}	
	if(hashed != NULL) {
		free(hashed);
		hashed = NULL;
	}
	if(result != NULL)
	{
		free(result);
		result = NULL;
	}

	return OCPSDK_SUCCESS;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
OCPConnectInfo OCPAPIManager_get_connectioninfo_ita(OCPManager handle, char* siteId, char* thingName, char* thingModuleType, char* messageHeaderType)
{
	OCPConnectInfo connectionInfo = OCPCONNECTINFO_INITIALIZER;
	IOCPAPIManager *ocpAPIManager = handle;
	if(ocpAPIManager == NULL || ocpAPIManager->protocol == NULL)
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPAPIManager_get_connectioninfo_ita] ocpAPIManager | ocpAPIManager->protocol is NULL\n");
		return connectionInfo;
	}

	if(ocpAPIManager->userToken[0] == 0 || siteId == NULL || thingName == NULL || thingModuleType == NULL || messageHeaderType == NULL)
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPAPIManager_get_connectioninfo_ita] userToken | siteId | thingName | thingModuleType | messageHeaderType  is NULL\n");
		return connectionInfo;
	}

	char *header = malloc(strlen(HTTP_HEADER_AUTHORIZATION) + strlen(ocpAPIManager->userToken) + 1);
	memset(header, 0, sizeof(char) * strlen(HTTP_HEADER_AUTHORIZATION) + sizeof(char) * strlen(ocpAPIManager->userToken) + 1);
	sprintf(header, HTTP_HEADER_AUTHORIZATION, ocpAPIManager->userToken);

	char *service = malloc(strlen(ITA_THING_AUTHCODE) + strlen(siteId) + strlen(thingName) + strlen(thingModuleType) + strlen(messageHeaderType) + 1);
	memset(service, 0, sizeof(char) * strlen(ITA_THING_AUTHCODE) + sizeof(char) * strlen(siteId) + sizeof(char) * strlen(thingName) + sizeof(char) * strlen(thingModuleType) + sizeof(char) * strlen(messageHeaderType) + 1);
	sprintf(service, ITA_THING_AUTHCODE, siteId, thingName, thingModuleType, messageHeaderType);

	char* result = OCPProtocolCurl_send(ocpAPIManager->protocol, service, header, NULL);
	if(result == NULL)
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPAPIManager_get_connectioninfo_ita] OCPProtocolCurl_send return NULL\n");
		if(service != NULL)
		{
			free(service);
			service = NULL;
		}
		if(header != NULL)
		{
			free(header);
			header = NULL;
		}
		return connectionInfo;
	}
	
 	ocp_json_t* object = OCPMessageConverterJson_fromCharPointer(result);

 	if(object == NULL)
 	{
 		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPAPIManager_get_connectioninfo_ita] OCPMessageConverterJson_fromCharPointer return NULL\n");

		if(service != NULL)
		{
			free(service);
			service = NULL;
		}
		if(header != NULL)
		{
			free(header);
			header = NULL;
		}
		if(result != NULL)
		{
			free(result);
			result = NULL;
		}
		return connectionInfo;
 	}

 	char* ip = OCPMessageConverterJson_getStringValue(object, OCPCONNECTIONINFO_IP);
 	if(ip != NULL)
 	{
 		memset(connectionInfo.ip, 0, sizeof(connectionInfo.ip));
		memcpy(connectionInfo.ip, ip, strlen(ip));
 	}

	char* port = OCPMessageConverterJson_getStringValue(object, OCPCONNECTIONINFO_PORT);
	if(port != NULL)
	{
		memset(connectionInfo.port, 0, sizeof(connectionInfo.port));
		memcpy(connectionInfo.port, port, strlen(port));	
	}

	char sslOpts = OCPMessageConverterJson_getIntValue(object, OCPCONNECTIONINFO_SSLOPTS) + 48;
	connectionInfo.sslOpts = sslOpts;
	
	char* msgHeaderType = OCPMessageConverterJson_getStringValue(object, OCPCONNECTIONINFO_MSGHEADERTYPE);
	if(msgHeaderType != NULL)
	{
		memset(connectionInfo.msgHeaderType, 0, sizeof(connectionInfo.msgHeaderType));
		memcpy(connectionInfo.msgHeaderType, msgHeaderType, strlen(msgHeaderType));	
	}
	
	char* encType = OCPMessageConverterJson_getStringValue(object, OCPCONNECTIONINFO_ENCTYPE);
	if(encType != NULL)
	{
		memset(connectionInfo.encType, 0, sizeof(connectionInfo.encType));
		memcpy(connectionInfo.encType, encType, strlen(encType));	
	}

	char* authCode = OCPMessageConverterJson_getStringValue(object, OCPCONNECTIONINFO_AUTHCODE);
	if(authCode != NULL)
	{
		memset(connectionInfo.authCode, 0, sizeof(connectionInfo.authCode));
		memcpy(connectionInfo.authCode, authCode, strlen(authCode));	
	}

	if(service != NULL){
		free(service);
		service = NULL;
	}

	if(header != NULL){
		free(header);
		header = NULL;
	}

	if(result != NULL){
		free(result);
		result = NULL;
	}

	if(object != NULL){
		OCPMessageConverterJson_delete(object);
		object = NULL;
	}

	return connectionInfo;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
char* OCPAPIManager_get_all_rootthings(OCPManager handle, char* siteId, unsigned int limit, unsigned int offset)
{
	IOCPAPIManager *ocpAPIManager = handle;
	if(ocpAPIManager == NULL || ocpAPIManager->protocol == NULL)
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPAPIManager_get_rootthings] OCPAPIManager is not initialize\n");
		return NULL;
	}

	if(ocpAPIManager->userToken[0] == 0 || siteId == NULL)
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPAPIManager_get_rootthings] status is logout or siteId  is NULL\n");
		return NULL;
	}

	char limitStr[32] = {0, };
	char offsetStr[32] = {0, };

	int res = sprintf(limitStr, "%u", limit);
	if(res <= 0){
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPAPIManager_get_rootthings] invalid limit \n");
		return NULL;
	}

	res = sprintf(offsetStr, "%u", offset);
	if(res <= 0){
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPAPIManager_get_rootthings] invalid offset \n");
		return NULL;
	}

	int limitLen = strlen(limitStr);
	int offsetLen = strlen(offsetStr);

	char *authorization = malloc(strlen(HTTP_HEADER_AUTHORIZATION) + strlen(ocpAPIManager->userToken) + 1);
	memset(authorization, 0, sizeof(char) * strlen(HTTP_HEADER_AUTHORIZATION) + sizeof(char) * strlen(ocpAPIManager->userToken) + 1);
	sprintf(authorization, HTTP_HEADER_AUTHORIZATION, ocpAPIManager->userToken);

	char *service = malloc(strlen(ROOT_THING_LIST) + strlen(siteId) + limitLen + offsetLen + 1);
	memset(service, 0, sizeof(char) * strlen(ROOT_THING_LIST) + sizeof(char) * strlen(siteId) + sizeof(char) * limitLen + sizeof(char) * offsetLen + 1);
	sprintf(service, ROOT_THING_LIST, siteId, limit, offset);

	char* result = OCPProtocolCurl_send(ocpAPIManager->protocol, service, authorization, NULL);

	if(service != NULL)
	{
		free(service);
		service = NULL;
	}
	if(authorization != NULL)
	{
		free(authorization);
		authorization = NULL;
	}
	if(result == NULL)
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPAPIManager_get_rootthings] OCPProtocolCurl_send return NULL\n");
		return NULL;
	}

	return result;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
char* OCPAPIManager_get_all_edgethings(OCPManager handle, char* siteId, char* thingName)
{
	IOCPAPIManager *ocpAPIManager = handle;
	if(ocpAPIManager == NULL || ocpAPIManager->protocol == NULL)
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPAPIManager_get_all_edgeThings] OCPAPIManager is not initialize\n");
		return NULL;
	}

	if(ocpAPIManager->userToken[0] == 0 || siteId == NULL || thingName == NULL)
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPAPIManager_get_all_edgeThings] status is logout or siteId | thingName  is NULL\n");
		return NULL;
	}

	int siteIdLen = strlen(siteId);
	int thingNameLen = strlen(thingName);

	char *authorization = malloc(strlen(HTTP_HEADER_AUTHORIZATION) + strlen(ocpAPIManager->userToken) + 1);
	memset(authorization, 0, sizeof(char) * strlen(HTTP_HEADER_AUTHORIZATION) + sizeof(char) * strlen(ocpAPIManager->userToken) + 1);
	sprintf(authorization, HTTP_HEADER_AUTHORIZATION, ocpAPIManager->userToken);

	char *service = malloc(strlen(ALL_EDGE_THINGS) + siteIdLen + thingNameLen + 1);
	memset(service, 0, sizeof(char) * strlen(ALL_EDGE_THINGS) + sizeof(char) * siteIdLen + sizeof(char) * thingNameLen + 1);
	sprintf(service, ALL_EDGE_THINGS, siteId, thingName);

	char* result = OCPProtocolCurl_send(ocpAPIManager->protocol, service, authorization, NULL);

	if(service != NULL)
	{
		free(service);
		service = NULL;
	}
	if(authorization != NULL)
	{
		free(authorization);
		authorization = NULL;
	}

	if(result == NULL)
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPAPIManager_get_all_edgeThings] OCPProtocolCurl_send return NULL\n");
		return NULL;
	}

	return result;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
char* OCPAPIManager_get_direct_edgethings(OCPManager handle, char* siteId, char* thingName, unsigned int limit, unsigned int offset)
{
	IOCPAPIManager *ocpAPIManager = handle;
	if(ocpAPIManager == NULL || ocpAPIManager->protocol == NULL)
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPAPIManager_get_direct_edgethings] OCPAPIManager is not initialize\n");
		return NULL;
	}

	if(ocpAPIManager->userToken[0] == 0 || siteId == NULL || thingName == NULL)
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPAPIManager_get_direct_edgethings] status is logout or [siteId | thingName] is NULL\n");
		return NULL;
	}

	int siteIdLen = strlen(siteId);
	int thingNameLen = strlen(thingName);

	char limitStr[32] = {0, };
	char offsetStr[32] = {0, };

	int res = sprintf(limitStr, "%u", limit);
	if(res <= 0){
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPAPIManager_get_direct_edgethings] invalid limit \n");
		return NULL;
	}

	res = sprintf(offsetStr, "%u", offset);
	if(res <= 0){
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPAPIManager_get_direct_edgethings] invalid offset \n");
		return NULL;
	}

	int limitLen = strlen(limitStr);
	int offsetLen = strlen(offsetStr);

	char *authorization = malloc(strlen(HTTP_HEADER_AUTHORIZATION) + strlen(ocpAPIManager->userToken) + 1);
	memset(authorization, 0, sizeof(char) * strlen(HTTP_HEADER_AUTHORIZATION) + sizeof(char) * strlen(ocpAPIManager->userToken) + 1);
	sprintf(authorization, HTTP_HEADER_AUTHORIZATION, ocpAPIManager->userToken);

	char *service = malloc(strlen(DIRECT_EDGE_THINGS) + siteIdLen + thingNameLen + limitLen + offsetLen + 1);
	memset(service, 0, sizeof(char) * strlen(DIRECT_EDGE_THINGS) + sizeof(char) * siteIdLen + sizeof(char) * thingNameLen + sizeof(char) * limitLen + sizeof(char) * offsetLen + 1);
	sprintf(service, DIRECT_EDGE_THINGS, siteId, thingName, limit, offset);

	char* result = OCPProtocolCurl_send(ocpAPIManager->protocol, service, authorization, NULL);
	
	if(service != NULL)
	{
		free(service);
		service = NULL;
	}
	if(authorization != NULL)
	{
		free(authorization);
		authorization = NULL;
	}
	
	if(result == NULL)
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPAPIManager_get_direct_edgethings] OCPProtocolCurl_send return NULL\n");
		return NULL;
	}

	return result;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
char* OCPAPIManager_get_thing_attributes(OCPManager handle, char* siteId, char* thingName)
{
	IOCPAPIManager *ocpAPIManager = handle;
	if(ocpAPIManager == NULL || ocpAPIManager->protocol == NULL)
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPAPIManager_get_attributes] OCPAPIManager is not initialize\n");
		return NULL;
	}

	if(ocpAPIManager->userToken[0] == 0 || siteId == NULL || thingName == NULL)
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPAPIManager_get_attributes] status is logout or siteId | thingName  is NULL\n");
		return NULL;
	}

	int siteIdLen = strlen(siteId);
	int thingNameLen = strlen(thingName);

	char *authorization = malloc(strlen(HTTP_HEADER_AUTHORIZATION) + strlen(ocpAPIManager->userToken) + 1);
	memset(authorization, 0, sizeof(char) * strlen(HTTP_HEADER_AUTHORIZATION) + sizeof(char) * strlen(ocpAPIManager->userToken) + 1);
	sprintf(authorization, HTTP_HEADER_AUTHORIZATION, ocpAPIManager->userToken);

	char *service = malloc(strlen(THING_ATTRIBUTES) + siteIdLen + thingNameLen + 1);
	memset(service, 0, sizeof(char) * strlen(THING_ATTRIBUTES) + sizeof(char) * siteIdLen + sizeof(char) * thingNameLen + 1);
	sprintf(service, THING_ATTRIBUTES, siteId, thingName);

	char* result = OCPProtocolCurl_send(ocpAPIManager->protocol, service, authorization, NULL);

	if(service != NULL)
	{
		free(service);
		service = NULL;
	}
	if(authorization != NULL)
	{
		free(authorization);
		authorization = NULL;
	}

	if(result == NULL)
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPAPIManager_get_attributes] OCPProtocolCurl_send return NULL\n");		
		return NULL;
	}

	return result;
}










char* OCPAPIManager_do_custom(OCPManager handle, char* service, char* dataString)
{
	IOCPAPIManager *ocpAPIManager = handle;
	if(ocpAPIManager == NULL || ocpAPIManager->protocol == NULL)
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPAPIManager_do_login] ocpAPIManager | ocpAPIManager->protocol is NULL\n");
		return OCPSDK_FAILURE;
	}

	// char dataString[1024] = {0, };
	// sprintf(dataString, "{\"thingModelTypeName\": \"%s\", \"thingModelTypeClassifyCode\": \"BASIC\"}", thingModelTypeName);

	char *authorization = malloc(strlen(HTTP_HEADER_AUTHORIZATION) + strlen(ocpAPIManager->userToken) + 1);
	memset(authorization, 0, sizeof(char) * strlen(HTTP_HEADER_AUTHORIZATION) + sizeof(char) * strlen(ocpAPIManager->userToken) + 1);
	sprintf(authorization, HTTP_HEADER_AUTHORIZATION, ocpAPIManager->userToken);

	// char *service = malloc(strlen(THING_ATTRIBUTES) + siteIdLen + thingNameLen + 1);
	// memset(service, 0, sizeof(char) * strlen(THING_ATTRIBUTES) + sizeof(char) * siteIdLen + sizeof(char) * thingNameLen + 1);
	// sprintf(service, THING_ATTRIBUTES, siteId, thingName);

	char* result = OCPProtocolCurl_send(ocpAPIManager->protocol, service, authorization, dataString);
	
	if(authorization != NULL)
	{
		free(authorization);
		authorization = NULL;
	}

	if(result == NULL)
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPAPIManager_get_attributes] OCPProtocolCurl_send return NULL\n");		
		return NULL;
	}

	return result;
}
