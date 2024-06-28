// default header of iotclient
#include "BasicDefs.h"
// queue and thread definitions
#include "queue.h"
// basic header of OCP Manager
#include "OCPManager.h"
// thing manager
#include "OCPThingManager.h"
// api manager for using open api
#include "OCPAPIManager.h"
// aes128
#include "OCPSecurityAES.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <sys/time.h>
#include <unistd.h>
#include <jansson.h>
#include <pthread.h>
//==================================================================================
#define ISNULLOREMPTY(ptr) 		((ptr == NULL) || (strlen(ptr) == 0))
//==================================================================================
// Global variables
//----------------------------------------------------------------------------------
OCPManager 			thingManager;
OCPManager 			apiManager;
OCPManager_callback cb;
//----------------------------------------------------------------------------------
OCPSecurityAES		securityAES;
//----------------------------------------------------------------------------------
ConnInfo 			connInfo;
Properties*			_properties;
//----------------------------------------------------------------------------------
bool 				bAES128Used;
//==================================================================================
// Semaphores
//----------------------------------------------------------------------------------
struct 					timespec ts;
int 					sem_timeout = 5;

sem_t* 					sem_forConn;
sem_t*					sem_forSync;

unsigned char 			auth_response[1024];	// for auth data
int 					sync_resCode;


IotClient_MsgListener 	*listenerOfApplication;
IotClient_ConnClosed	*connClosedOfApplication;
//==================================================================================
// for connection thread
//----------------------------------------------------------------------------------
int 	thread_all_flag = 1;
int 	thread_conn_flag = 0;

void* 	thread_sender(void* args);
void* 	thread_receiver(void* args);
void* 	thread_keep_alive(void* args);

QUEUE*	send_queue;
QUEUE*	receive_queue;
//==================================================================================

//==================================================================================
IotClient* createIotClient(char* authCode, char* portalID, char* portalPW, char* siteId, char* thingName)
{
	// 1. create iotclient
	IotClient* client = malloc(sizeof(IotClient));
	if (NULL == client) {
		fprintf(stderr, "[ERR] Creating IotClient has failed.\n");
		exit(EXIT_FAILURE);
	}
	// 2. get properties from file
	_properties = NULL;
	client->properties = _properties = readProperties();
	// 3. check if they are okay
	if(!checkPropertiesAreOkay(client->properties)) {
		fprintf(stderr, "[ERR] please check the property file (%s)\n", PROPERTY_FILENAME);
		exit(EXIT_FAILURE);
	}
	fprintf(stdout, "[OK] Property file has no problem.\n");
	// 4. check site id, thing name
	if(ISNULLOREMPTY(siteId) || ISNULLOREMPTY(thingName)) {
		fprintf(stderr, "[ERR] Site ID, Thing Name should not be empty.\n");
		exit(EXIT_FAILURE);
	} else {
		fprintf(stderr, "[OK] Site ID (%s), Thing Name (%s)\n", siteId, thingName);
		strcpy(connInfo.siteId, siteId);
		strcpy(connInfo.thingName, thingName);
	}

	// 5. check connection info (authcode, id, pw)
	bAES128Used = false;
	int authType = atoi(client->properties->authType);
	if( authType == 5 )
	{ // ITA
		if( !ISNULLOREMPTY(authCode) ) {
			// auth code 있음 -> don't care id/pw
			strcpy(connInfo.authCode, authCode);
		} else { 
			// no auth code
			if(ISNULLOREMPTY(portalID) || ISNULLOREMPTY(portalPW)) {
				fprintf(stderr, "[ERR] Enter correct authCode or fill portal's id and password.\n");
				exit(EXIT_FAILURE);
			} else {
				strcpy(connInfo.portalID, portalID);
				strcpy(connInfo.portalPW, portalPW);

				// get auth code using API manager
				char auth_code[SHORT_STR_LEN] = {0, };
				int ret = getAuthCodeWithIDPW(auth_code, client->properties);
				if(ret == -1) { // error case
					fprintf(stderr, "[ERR] can't get the authCode from API server with id(%s), pw(%s)\n", connInfo.portalID, connInfo.portalPW);
					exit(EXIT_FAILURE);
				}
				strcpy(connInfo.authCode, auth_code);
			}
		}

		fprintf(stderr, "[OK] id:%s, pw:%s, authcode:%s\n", connInfo.portalID, connInfo.portalPW, connInfo.authCode);

		// make security
		securityAES = NULL;
		int dataEncryption = atoi(client->properties->dataEncryption);
		if(dataEncryption != 0) { // AES128
			OCPSecurityAES_properties security_properties = OCPSECURITYAES_PROPERTIES_INITIALIZER;
			security_properties.aes_type = 128;
			memcpy(security_properties.auth_code, connInfo.authCode, sizeof(security_properties.auth_code)); 
			if(1 != OCPSecurityAES_create(&securityAES, &security_properties)) {
				fprintf(stderr, "[ERR] Can't make AES Security. \n");
		        exit(EXIT_FAILURE);
			}
			bAES128Used = true;
			fprintf(stderr, "[OK] Creating AES Security is done.\n");
		}
	}
	// 6. init semaphores
	{
		sem_forConn = malloc(sizeof(sem_t));
		if(sem_init(sem_forConn, 0, 0) != 0) {
			fprintf(stderr, "[ERR] Can't make a semaphore for connection authorization.\n");
			exit(EXIT_FAILURE);
		}

		sem_forSync = malloc(sizeof(sem_t));
		if(sem_init(sem_forSync, 0, 0) != 0) {
			fprintf(stderr, "[ERR] Can't make a semaphore for send sync.\n");
			exit(EXIT_FAILURE);
		}
	}
	// 7. run connection thread
	{
		pthread_t	thread_cn;
		if(pthread_create(&thread_cn, NULL, thread_keep_alive, NULL) != 0) {
			fprintf(stderr, "[ERR] can't create connection thread.\n");
			exit(EXIT_FAILURE);
		}
		if(pthread_detach(thread_cn) != 0) {
			fprintf(stderr, "[ERR] can't run connection thread.\n");
			exit(EXIT_FAILURE);
		}
		thread_all_flag = 1;
		thread_conn_flag = 0;
	}
	// 8. make queues
	{
		receive_queue = create_queue();
		if(receive_queue == NULL) {
			fprintf(stderr, "[ERR] can't create receive queue.\n");
			exit(EXIT_FAILURE);
		}
		send_queue = create_queue();
		if(send_queue == NULL) {
			fprintf(stderr, "[ERR] can't create send queue.\n");
			exit(EXIT_FAILURE);
		}
	}
	// 9. run sender/receiver
	{
		pthread_t 	thread_sn, thread_rcv;
		if( pthread_create(&thread_sn, 	NULL, thread_sender, 	NULL) != 0 ||
			pthread_create(&thread_rcv, NULL, thread_receiver, 	NULL) != 0 )
		{
			fprintf(stderr, "[ERR] can't create receiver/sender thread.\n");
			exit(EXIT_FAILURE);
		}
		if(pthread_detach(thread_sn) != 0) {
			fprintf(stderr, "[ERR] can't start sender thread.\n");
			exit(EXIT_FAILURE);
		}
		if(pthread_detach(thread_rcv) != 0) {
			fprintf(stderr, "[ERR] can't start receiver thread.\n");
			exit(EXIT_FAILURE);
		}
	}
	// 10. initialize varibales and pointers
	{
		// init managers
		thingManager = NULL;
		apiManager = NULL;

		// init call back
		cb.ma = preListener;
		cb.cl = preHandlerConnClosed;
		listenerOfApplication = NULL;
		connClosedOfApplication = NULL;

		// set function pointers with real ones.
		client->connect = iotclient_connect;
		client->disconnect = iotclient_disconnect;
		client->setCustomMessageListener = iotclient_setCustomMessageListener;
		client->sendAttributes = iotclient_sendAttributes;
		client->sendAttributesInDelimiter = iotclient_sendAttributesInDelimiter;
		client->sendAttributesSync = iotclient_sendAttributesSync;
		client->sendAttributesForLeaf = iotclient_sendAttributesForLeaf;
		client->sendAttributesForLeafInDelimiter = iotclient_sendAttributesForLeafInDelimiter;
		client->sendAttributesSyncForLeaf = iotclient_sendAttributesSyncForLeaf;
		client->activateThing = iotclient_activateThing;
		client->registerLeafThing = iotclient_registerLeafThing;
		client->activateThingForLeaf = iotclient_activateThingForLeaf;
		client->requestFirmwareLatestVersion = iotclient_requestFirmwareLatestVersion;
		client->requestFirmwareUpgradeComplete = iotclient_requestFirmwareUpgradeComplete;
		client->requestFirmwareSignData = iotclient_requestFirmwareSignData;
		client->requestFileUploadUri = iotclient_requestFileUploadUri;
		client->requestFileUploadComplete = iotclient_requestFileUploadComplete;
		client->requestWbcMc = iotclient_requestWbcMc;
		//client->sendHeartBeat = iotclient_sendHeartBeat;
	}

	fprintf(stderr, "[OK] Creating IotClient is done.\n");
	return client;
}

void destroyIotClient(IotClient* client)
{
	// thread termination
	thread_all_flag = 0;
	// security 
	if(bAES128Used) {
		OCPSecurityAES_destory(&securityAES);
		fprintf(stdout, "[OK] AES security is destroyed.\n");
	}
	// connection
	if(client != NULL) {
		iotclient_disconnect();
		if(client->properties != NULL) {
			free(client->properties);
			client->properties = NULL;
			_properties = NULL;
		}
		free(client);
		client = NULL;
		fprintf(stdout, "[OK] Iot Client is destroyed.\n");
	}
	// thing manager
	if(thingManager != NULL) {
		OCPThingManager_destroy(&thingManager);
		cb.ma = NULL;
		cb.cl = NULL;
		thingManager = NULL;
		fprintf(stdout, "[OK] ThingManager is destroyed.\n");
	}
	// destroy semaphore
	if(sem_forConn != NULL)	{
		sem_destroy(sem_forConn);
		free(sem_forConn);
		fprintf(stdout, "[OK] semaphore for connection is cleared normally.\n");
	}
	if(sem_forSync != NULL) {
		sem_destroy(sem_forSync);
		free(sem_forSync);
		fprintf(stdout, "[OK] semaphore for sendSync is cleared normally.\n");
	}
	// clear queue
	if(receive_queue != NULL) 	destroy_queue(receive_queue);
	if(send_queue != NULL) 		destroy_queue(send_queue);
}

iot_status_code iotclient_connect(void)
{
	int authType = -1;
	if(thingManager == NULL)
	{ // if former thing manager is not there, create a new one
		authType = atoi(_properties->authType);

		OCPManager_properties _prop = OCPMANAGER_PROPERTIES_INITIALIZER;
		
		// mqtt server 를 위한 설정
		_prop.sslOpts = (authType == 5) ? (_properties->thingSslOpts)[0] : '2';

		// 공통 for both ITA and SSL
		strncpy(_prop.host, _properties->host, strlen(_properties->host));
		strncpy(_prop.port, _properties->thingPort, strlen(_properties->thingPort));
		strncpy(_prop.siteId, connInfo.siteId, strlen(connInfo.siteId));
		strncpy(_prop.thingName, connInfo.thingName, strlen(connInfo.thingName));
		strncpy(_prop.trustStore, _properties->thingTrustStore, strlen(_properties->thingTrustStore));
		if(authType == 5) { // ita
			strncpy(_prop.authCode, connInfo.authCode, strlen(connInfo.authCode));
		} else { // ssl
			strncpy(_prop.keyStore, _properties->keyStore, strlen(_properties->keyStore));
			strncpy(_prop.keyPassword, _properties->keyPassword, strlen(_properties->keyPassword));
		}

		strncpy(_prop.msgHeaderType, _properties->msgHeaderType, strlen(_properties->msgHeaderType));
		//fprintf(stderr, "[DEBUG] sslopts:%c, authType:%d, authCode:%s, thingName:%s\n", _prop.sslOpts, authType, _prop.authCode, _prop.thingName);

		// compression is not used (2020-09-16)
		//_prop.minCompressionSize = 10; // set the default value

		if(listenerOfApplication == NULL) fprintf(stderr, "[WARN] your message arrive function pointer is NULL.\n");
		if(connClosedOfApplication == NULL) fprintf(stderr, "[WARN] your connection closed function pointer is NULL.\n");
		int rc = 0;
		if( OCPSDK_SUCCESS != (rc = OCPThingManager_create(&thingManager, &cb, &_prop)) ) {
			fprintf(stderr, "[ERR] can't make Thing Manager.\n");
			return (iot_status_code)rc;
		}
	}

	int rc = 0;
	if(OCPSDK_SUCCESS != OCPThingManager_isconnected(thingManager))
	{ // only if it's disconnected currently
		if(OCPSDK_SUCCESS != (rc = OCPThingManager_connect(thingManager)))
		{
			fprintf(stderr, "[ERR] can't connect to server with Thing Manager (%d)\n", rc);
			return (iot_status_code)rc;
		}

		// authorize 과정에는 암호화 없음
		OCPMessage* message = NULL;
		char auth_data[1024] = {0, };
		if(authType == 5) { // ITA
			sprintf(auth_data, AUTHORIZE_MSG, connInfo.authCode);
			message = OCPThingManager_get_authorize_message(
				thingManager, OCPENCTYPE_PLAIN, OCPDATAFORMAT_JSON, auth_data, strlen(auth_data));
		} else if(authType == 6) { // two-way SSL
			message = OCPThingManager_get_authorize_message(
				thingManager, OCPENCTYPE_PLAIN,	OCPDATAFORMAT_JSON, NULL, 0);
		} else { 
			// wrong case, will not come here ever.
			return ERR_WRONG_AUTHTYPE;	// 13
		}

		if(message != NULL) {
			fprintf(stdout, "[DEBUG] added AUTHORIZE MSG into send queue.\n");
			add_queue(send_queue, message);
			// wait response
			int rc = 0;
			if(clock_gettime(CLOCK_REALTIME, &ts) != -1)
			{
				ts.tv_sec += sem_timeout;
				rc = sem_timedwait(sem_forConn, &ts);
			}

			if(rc == 0) 
			{ // got response, set auth token
				OCPAuthorize_result authorize_result = OCPThingManager_parse_authorize_result(thingManager, auth_response);
				rc = OCPThingManager_set_authToken(thingManager, authorize_result.authToken);
				if(OCPSDK_SUCCESS != rc) {
					fprintf(stderr, "[ERR] Authorize is Failed.\n");
					exit(EXIT_FAILURE);
				}
			} else if(rc == -1) {
				fprintf(stderr, "[ERR] Couldn't get authorization response from server.\n");
				exit(EXIT_FAILURE);
			}

			fprintf(stderr, "[OK] Authorization is done.\n");
		}
	}

	thread_conn_flag = 1;

	return ERR_OK;
}

void iotclient_disconnect()
{
	thread_conn_flag = 0;
	if(thingManager != NULL) {
		if(OCPSDK_SUCCESS == OCPThingManager_isconnected(thingManager)) { // connected
			OCPThingManager_disconnect(thingManager);
			fprintf(stderr, "[OK] Disconnected.\n");
		}
	}
}

void iotclient_setCustomMessageListener(void (*messageArrive)(void*, OCPMessage*), void (*connectionClosed)(void*, char*))
{
	if(messageArrive != NULL) {
		fprintf(stderr, "[OK] set the callback function (message arrive)\n");
		listenerOfApplication = messageArrive;
	} else { // null
		fprintf(stderr, "[WARN] your message arrive function pointer is NULL.\n");
		listenerOfApplication = NULL;
	}
	if(connectionClosed != NULL) {
		fprintf(stderr, "[OK] set the callback function (connection closed)\n");
		connClosedOfApplication = connectionClosed;
	} else { // null
		fprintf(stderr, "[WARN] your connection closed function pointer is NULL.\n");
		connClosedOfApplication = NULL;
	}
}

// msg code : NULL ? "Basic-AttrGroup" default 로 사용함
// dataStr : Json data 를 string 으로 나열
void iotclient_sendAttributes(char* msgCode, char* dataStr)
{
	if(msgCode == NULL || strlen(msgCode) == 0) {
		fprintf(stderr, "[WARN][sendAttributes] msgCode is NULL. will use default code (Basic-AttrGroup).\n");
		msgCode = OCPMESSAGE_MSGCODE_BASIC_ATTR_GROUP;
	}
	if(dataStr == NULL || strlen(dataStr) == 0) {
		fprintf(stderr, "[ERR][sendAttributes] dataStr is NULL. can't send a message.\n");
		return;
	}
	// make BIOT message
	OCPMessage* message = makeMessage(1, dataStr, connInfo.thingName, msgCode);
	// send it
	if(message != NULL) {
		add_queue(send_queue, message);
		fprintf(stdout, "[DEBUG] added sendAttributes message into send queue.\n");
	} else {
		fprintf(stderr, "[ERR] can't make the register root attribute message.\n");

	}
}

void iotclient_sendAttributesInDelimiter(char* msgCode, char* dataStr)
{
	if(msgCode == NULL || strlen(msgCode) == 0) {
		fprintf(stderr, "[WARN][sendAttributes] msgCode is NULL. will use default code (Basic-AttrGroup).\n");
		msgCode = OCPMESSAGE_MSGCODE_BASIC_ATTR_GROUP;
	}
	if(dataStr == NULL || strlen(dataStr) == 0) {
		fprintf(stderr, "[ERR][sendAttributes] dataStr is NULL. can't send a message.\n");
		return;
	}
	// make BIOT message
	OCPMessage* message = makeMessage(9, dataStr, connInfo.thingName, msgCode);
	// send it
	if(message != NULL) {
		add_queue(send_queue, message);
		fprintf(stdout, "[DEBUG] added sendAttributes message into send queue.\n");
	} else {
		fprintf(stderr, "[ERR] can't make the register root attribute message.\n");

	}
}

iot_status_code iotclient_sendAttributesSync(char* msgCode, char* dataStr)
{
	if(msgCode == NULL || strlen(msgCode) == 0) {
		fprintf(stderr, "[WARN][sendAttributesSync] msgCode is NULL. will use default code (Basic-AttrGroup).\n");
		msgCode = OCPMESSAGE_MSGCODE_BASIC_ATTR_GROUP;
	}
	if(dataStr == NULL || strlen(dataStr) == 0) {
		fprintf(stderr, "[ERR][sendAttributesSync] dataStr is NULL. can't send a message.\n");
		return ERR_INVALID_PARAMETER;
	}
	// make BIOT message
	OCPMessage* message = makeMessage(1, dataStr, connInfo.thingName, msgCode);
	// send it
	if(message != NULL) {
		add_queue(send_queue, message);
		fprintf(stdout, "[DEBUG] added sendAttributesSync message into send queue.\n");
		// wait response
		int rc = -1;
		if(clock_gettime(CLOCK_REALTIME, &ts) != -1)
		{
			ts.tv_sec += sem_timeout;
			rc = sem_timedwait(sem_forSync, &ts);
		}

		if(rc == 0)
		{ // 성공
			fprintf(stdout, "[DEBUG][sendAttributesSync] got response normally. (%d)\n", sync_resCode);
			return (iot_status_code)sync_resCode;
		} else if(rc == -1) {
			// 실패 timed out
			fprintf(stderr, "[ERR][sendAttributesSync] timed out when waiting the response.");
			return ERR_TIMEOUT_RESPONSE;
		}
	} else {
		fprintf(stderr, "[ERR] can't make the register root attribute sync message.\n");
	}

	return ERR_CANT_MAKE_MESSAGE;	// here is the error case
}

void iotclient_sendAttributesForLeaf(char* leafThingName, char* msgCode, char* dataStr)
{
	if(msgCode == NULL || strlen(msgCode) == 0) {
		fprintf(stderr, "[ERR][sendAttributesForLeaf] msgCode is NULL. will use default code (Basic-AttrGroup).\n");
		msgCode = OCPMESSAGE_MSGCODE_BASIC_ATTR_GROUP;
	}
	if(dataStr == NULL || strlen(dataStr) == 0) {
		fprintf(stderr, "[ERR][sendAttributesForLeaf] dataStr is NULL. can't send a message.\n");
		return;
	}
	// make BIOT message	
	OCPMessage* message = makeMessage(1, dataStr, leafThingName, msgCode);
	// send it
	if(message != NULL) {
		add_queue(send_queue, message);
		fprintf(stdout, "[DEBUG] added sendAttributesForLeaf message into send queue.\n");
	} else {
		fprintf(stderr, "[ERR] can't make the register leaf attribute message.\n");
	}
}

void iotclient_sendAttributesForLeafInDelimiter(char* leafThingName, char* msgCode, char* dataStr)
{
	if(msgCode == NULL || strlen(msgCode) == 0) {
		fprintf(stderr, "[ERR][sendAttributesForLeaf] msgCode is NULL. will use default code (Basic-AttrGroup).\n");
		msgCode = OCPMESSAGE_MSGCODE_BASIC_ATTR_GROUP;
	}
	if(dataStr == NULL || strlen(dataStr) == 0) {
		fprintf(stderr, "[ERR][sendAttributesForLeaf] dataStr is NULL. can't send a message.\n");
		return;
	}
	// make BIOT message	
	OCPMessage* message = makeMessage(9, dataStr, leafThingName, msgCode);
	// send it
	if(message != NULL) {
		add_queue(send_queue, message);
		fprintf(stdout, "[DEBUG] added sendAttributesForLeaf message into send queue.\n");
	} else {
		fprintf(stderr, "[ERR] can't make the register leaf attribute message.\n");
	}
}

iot_status_code iotclient_sendAttributesSyncForLeaf(char* leafThingName, char* msgCode, char* dataStr)
{
	if(msgCode == NULL || strlen(msgCode) == 0) {
		fprintf(stderr, "[ERR][sendAttributesSyncForLeaf] msgCode is NULL. will use default code (Basic-AttrGroup).\n");
		msgCode = OCPMESSAGE_MSGCODE_BASIC_ATTR_GROUP;
	}
	if(dataStr == NULL || strlen(dataStr) == 0) {
		fprintf(stderr, "[ERR][sendAttributesSyncForLeaf] dataStr is NULL. can't send a message.\n");
		return ERR_INVALID_PARAMETER;
	}
	// make BIOT message	
	OCPMessage* message = makeMessage(1, dataStr, leafThingName, msgCode);
	// send it
	if(message != NULL) {
		add_queue(send_queue, message);
		fprintf(stdout, "[DEBUG] added sendAttributesSyncForLeaf message into send queue.\n");
		// wait response
		int rc = -1;
		if(clock_gettime(CLOCK_REALTIME, &ts) != -1)
		{
			ts.tv_sec += sem_timeout;
			rc = sem_timedwait(sem_forSync, &ts);
		}

		if(rc == 0)
		{ // 성공
			fprintf(stdout, "[DEBUG][sendAttributesSyncForLeaf] got response normally. (%d)\n", sync_resCode);
			return (iot_status_code)sync_resCode;
		} else if(rc == -1) {
			// 실패 timed out
			fprintf(stderr, "[ERR][sendAttributesSyncForLeaf] timed out when waiting the response.");
			return ERR_TIMEOUT_RESPONSE;
		}
	} else {
		fprintf(stderr, "[ERR] can't make the register leaf attribute sync message.\n");
	}

	return ERR_CANT_MAKE_MESSAGE;	// here is the error case
}

iot_status_code iotclient_activateThing(char* modelName, char* uniqueNum)
{
	if(modelName == NULL || strlen(modelName) == 0) {
		fprintf(stderr, "[ERR][activateThing] modelName is NULL. should not be empty.\n");
		return ERR_INVALID_PARAMETER;
	}
	if(uniqueNum == NULL || strlen(uniqueNum) == 0) {
		fprintf(stderr, "[ERR][activateThing] uniqueNum is NULL. should not be empty.\n");
		return ERR_INVALID_PARAMETER;
	}
	fprintf(stderr, "[DEBUG][activateThing] encryption:%s\n", _properties->dataEncryption);
	// make data string
	char data[LONG_STR_LEN] = {0, };
	sprintf(data, ACTIVATE_ROOT, modelName, uniqueNum);
	// make BIOT message
	OCPMessage* message = makeMessage(2, data, NULL, NULL);
	// send it
	if(message != NULL) {
		add_queue(send_queue, message);
		fprintf(stdout, "[DEBUG] added activateThing message into send queue.\n");
	} else {
		fprintf(stderr, "[ERR] can't make activate root thing message.\n");
		return ERR_CANT_MAKE_MESSAGE;
	}

	return ERR_OK;
}

iot_status_code iotclient_registerLeafThing(char* leafThingName, char* modelName, char* uniqueNum)
{
	if(leafThingName == NULL || strlen(leafThingName) == 0) {
		fprintf(stderr, "[ERR][registerLeafThing] leafThingName is NULL. should not be empty.\n");
		return ERR_INVALID_PARAMETER;
	}
	if(modelName == NULL || strlen(modelName) == 0) {
		fprintf(stderr, "[ERR][registerLeafThing] modelName is NULL. should not be empty.\n");
		return ERR_INVALID_PARAMETER;
	}
	if(uniqueNum == NULL || strlen(uniqueNum) == 0) {
		fprintf(stderr, "[ERR][registerLeafThing] uniqueNum is NULL. should not be empty.\n");
		return ERR_INVALID_PARAMETER;
	}
	// make data string
	char data[LONG_STR_LEN] = {0, };
	sprintf(data, REGISTER_EDGE, modelName, uniqueNum, connInfo.thingName);
	// make BIOT message
	OCPMessage* message = makeMessage(3, data, leafThingName, NULL);
	// send it
	if(message != NULL) {
		add_queue(send_queue, message);
		fprintf(stdout, "[DEBUG] added registerLeafThing message into send queue.\n");
	} else {
		fprintf(stderr, "[ERR] can't make register leaf thing message.\n");
		return ERR_CANT_MAKE_MESSAGE;
	}

	return ERR_OK;
}

iot_status_code iotclient_activateThingForLeaf(char* leafThingName, char* modelName, char* uniqueNum)
{
	if(leafThingName == NULL || strlen(leafThingName) == 0) {
		fprintf(stderr, "[ERR][activateThingForLeaf] leafThingName is NULL. should not be empty.\n");
		return ERR_INVALID_PARAMETER;
	}
	if(modelName == NULL || strlen(modelName) == 0) {
		fprintf(stderr, "[ERR][activateThingForLeaf] modelName is NULL. should not be empty.\n");
		return ERR_INVALID_PARAMETER;
	}
	if(uniqueNum == NULL || strlen(uniqueNum) == 0) {
		fprintf(stderr, "[ERR][activateThingForLeaf] uniqueNum is NULL. should not be empty.\n");
		return ERR_INVALID_PARAMETER;
	}
	// make data string
	char data[LONG_STR_LEN] = {0, };
	sprintf(data, REGISTER_EDGE, modelName, uniqueNum, connInfo.thingName);
	// make BIOT Message
	OCPMessage* message = makeMessage(3, data, leafThingName, NULL);
	// send it
	if(message != NULL) {
		add_queue(send_queue, message);
		fprintf(stdout, "[DEBUG] added activateThingForLeaf message into send queue.\n");
	} else {
		fprintf(stderr, "[ERR] can't make activate leaf thing message.\n");
		return ERR_CANT_MAKE_MESSAGE;
	}

	return ERR_OK;
}

iot_status_code iotclient_requestFirmwareLatestVersion(char* modelName, char* firmwareType)
{
	if(modelName == NULL || strlen(modelName) == 0) {
		fprintf(stderr, "[ERR][requestFirmwareLatestVersion] modelName is NULL. should not be empty.\n");
		return ERR_INVALID_PARAMETER;
	}
	if(firmwareType == NULL || strlen(firmwareType) == 0) {
		fprintf(stderr, "[ERR][requestFirmwareLatestVersion] firmwareType is NULL. should not be empty.\n");
		return ERR_INVALID_PARAMETER;
	}
	// make data string
	char data[LONG_STR_LEN] = {0, };
	sprintf(data, REQ_ACTIVE_FRM, modelName, firmwareType);
	// make BIOT Message
	OCPMessage* message = makeMessage(4, data, connInfo.thingName, NULL);
	// send it
	if(message != NULL) {
		add_queue(send_queue, message);
		fprintf(stdout, "[DEBUG] added requestFirmwareLatestVersion message into send queue.\n");
	} else {
		fprintf(stderr, "[ERR] can't make request Active Firmware message.\n");
		return ERR_CANT_MAKE_MESSAGE;
	}

	return ERR_OK;
}

iot_status_code iotclient_requestFirmwareUpgradeComplete(char* thingName, char* firmwareType, char* version)
{
	if(thingName == NULL || strlen(thingName) == 0) {
		fprintf(stderr, "[ERR][requestFirmwareUpgradeComplete] thingName is NULL. should not be empty.\n");
		return ERR_INVALID_PARAMETER;
	}
	if(firmwareType == NULL || strlen(firmwareType) == 0) {
		fprintf(stderr, "[ERR][requestFirmwareUpgradeComplete] firmwareType is NULL. should not be empty.\n");
		return ERR_INVALID_PARAMETER;
	}
	if(version == NULL || strlen(version) == 0) {
		fprintf(stderr, "[ERR][requestFirmwareUpgradeComplete] version is NULL. should not be empty.\n");
		return ERR_INVALID_PARAMETER;
	}
	// make data string
	char data[LONG_STR_LEN] = {0, };
	sprintf(data, FRM_UPGRADE_RES, firmwareType, version);
	// make BIOT Message
	OCPMessage* message = makeMessage(5, data, thingName, NULL);
	// send it
	if(message != NULL) {
		add_queue(send_queue, message);
		fprintf(stdout, "[DEBUG] added requestFirmwareUpgradeComplete message into send queue.\n");
	} else {
		fprintf(stderr, "[ERR] can't make Firmware Complete message.\n");
		return ERR_CANT_MAKE_MESSAGE;
	}

	return ERR_OK;
}

iot_status_code iotclient_requestFirmwareSignData(char* fileUri)
{
	if(fileUri == NULL || strlen(fileUri) == 0) {
		fprintf(stderr, "[ERR][requestFirmwareSignData] fileUri is NULL. should not be empty.\n");
		return ERR_INVALID_PARAMETER;
	}
	fprintf(stderr, "[DEBUG][requestFirmwareSignData] uri:(%s)\n", fileUri);
	// make data string
	char data[LONG_STR_LEN] = {0, };
	sprintf(data, REQ_SIGNED_FRM, fileUri);
	// make BIOT Message
	OCPMessage* message = makeMessage(6, data, NULL, NULL);
	// send it
	if(message != NULL) {
		add_queue(send_queue, message);
		fprintf(stdout, "[DEBUG] added requestFirmwareSignData message into send queue.\n");
	} else {
		fprintf(stderr, "[ERR] can't make Request Firmware Sign Data message.\n");
		return ERR_CANT_MAKE_MESSAGE;
	}

	return ERR_OK;
}

iot_status_code iotclient_requestFileUploadUri()
{
	// make BIOT Message
	OCPMessage* message = OCPThingManager_get_fileuploaduri_request_message(
		thingManager, (bAES128Used) ? OCPENCTYPE_AES128 : OCPENCTYPE_PLAIN, OCPDATAFORMAT_JSON, NULL, 0, connInfo.thingName);
	// send it
	if(message != NULL) {
		add_queue(send_queue, message);
		fprintf(stdout, "[DEBUG] added requestFileUploadUri message into send queue.\n");
	} else {
		fprintf(stderr, "[ERR] can't make request File Upload Uri.\n");
		return ERR_CANT_MAKE_MESSAGE;
	}

	return ERR_OK;
}

iot_status_code iotclient_requestFileUploadComplete(char* fileName, char* fileUri)
{
	if(fileName == NULL || strlen(fileName) == 0) {
		fprintf(stderr, "[ERR][requestFileUploadComplete] fileName is NULL. should not be empty.\n");
		return ERR_INVALID_PARAMETER;
	}
	if(fileUri == NULL || strlen(fileUri) == 0) {
		fprintf(stderr, "[ERR][requestFileUploadComplete] fileUri is NULL. should not be empty.\n");
		return ERR_INVALID_PARAMETER;
	}
	fprintf(stderr, "[DEBUG][requestFileUploadComplete] uri:(%s) name(%s)\n", fileUri, fileName);

	char data[LONG_STR_LEN] = {0, };
	sprintf(data, FILE_UPLOAD_DONE, fileName, fileUri);
	// make BIOT Message
	OCPMessage* message = makeMessage(7, data, connInfo.thingName, NULL);
	// send it
	if(message != NULL) {
		add_queue(send_queue, message);
		fprintf(stdout, "[DEBUG] added iotclient_requestFileUploadComplete message into send queue.\n");
	} else {
		fprintf(stderr, "[ERR] can't make request File Upload Complete message.\n");
		return ERR_CANT_MAKE_MESSAGE;
	}

	return ERR_OK;
}

iot_status_code iotclient_requestWbcMc()
{
	char data[LONG_STR_LEN] = {0, };
	sprintf(data, REQ_WBC_MC, connInfo.thingName);
	// make BIOT message
	OCPMessage* message = makeMessage(8, data, connInfo.thingName, NULL);
	// send it
	if(message != NULL) {
		add_queue(send_queue, message);
		fprintf(stdout, "[DEBUG] added requestWbcMc message into send queue.\n");
	} else {
		fprintf(stderr, "[ERR] can't make request wbc mc message.\n");
		return ERR_CANT_MAKE_MESSAGE;
	}

	return ERR_OK;
}

void iotclient_sendHeartBeat()
{
	OCPMessage* message = OCPThingManager_get_heartbeat_message(thingManager);
	if(message != NULL) {
		add_queue(send_queue, message);
		fprintf(stdout, "[DEBUG] added sendHeartBeat message into send queue.\n");
	} else {
		fprintf(stderr, "[ERR] can't make Heart Beat message.\n");
	}	
}

void freeMessage(OCPMessage* message) 
{
	OCPManager_free_message(message);
}

/*****************************************************************************
 * 1차 Listener before giving it to application
******************************************************************************/
void preListener(void* context, OCPMessage* message)
{
	if(message) {
		struct tm *tm;
		struct timeval tv;
		gettimeofday(&tv, NULL);
		tm = localtime(&tv.tv_sec);
		fprintf(stdout, "[<<<] [%d/%d/%d %d:%d:%d] (%s)\n",
			tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, message->msgCode);

		add_queue(receive_queue, (void*)message);
	}
}

void preHandlerConnClosed(void* context, char* cause)
{ // when the connection is closed, this will be called.
	thread_conn_flag = 0;
	if(cause != NULL && strlen(cause) != 0)
		fprintf(stdout, "[<<<] Connection Closed : %s\n", cause);
	if(connClosedOfApplication != NULL)
		connClosedOfApplication(context, cause);

	// TODO 
	// re-connect 시도 아예 connection 을 새로 만들것
	iotclient_connect();	// is this enough ??
}

/*****************************************************************************
 * Property 관련 함수들(단순반복)
******************************************************************************/
Properties* readProperties()
{
	Properties* properties = malloc(sizeof(Properties));
	if(NULL == properties) {
	    fprintf(stderr, "[ERR] Failed to make properties.\n");
	    exit(EXIT_FAILURE);
	}

	FILE* fp;
	char* line = NULL;
	size_t len = 0;
	ssize_t read;

	fp = fopen(PROPERTY_FILENAME, "r");
	if(NULL == fp) {
	    fprintf(stderr, "[ERR] Failed to read property file. (%s)\n", PROPERTY_FILENAME);
	    exit(EXIT_FAILURE);
	}

	while((read = getline(&line, &len, fp)) != -1) {
		if(line[0] == 0 || line[0] == '#') 		continue; 	// comment
		if(line[0] == ' ' || line[0] == '\n')	continue;	// blank case #1
		if(line[0] == '\r' && line[1] == '\n') 	continue;	// blank case #2

		char key[128] = {0, };
		char value[128] = {0, };
		getKeyNValue_1(line, key, value, "=");

		fprintf(stdout, "[Property] key(%s) value(%s)\n", key, value);

		switch(getNumFromKey(key)) {
			case 1:	// AUTHTYPE
			 	strcpy(properties->authType, value);
			 	break;
			case 2:	// HOST
				strcpy(properties->host, value);
				break;
			case 3:	// API_PORT
				strcpy(properties->apiPort, value);
				break;
			case 4:	// API_SSLOPTS
				strcpy(properties->apiSslOpts, value);
				break;
			case 5:	// API_TRUSTSTORE
				strcpy(properties->apiTrustStore, value);
				break;
			case 6:	// THING_PORT
				strcpy(properties->thingPort, value);
				break;
			case 7:	// THING_SSLOPTS
			 	strcpy(properties->thingSslOpts, value);
			 	break;
			case 8:	// THING_TRUSTSTORE
			 	strcpy(properties->thingTrustStore, value);
			 	break;
			case 9:	// MSGHEADERTYPE
				strcpy(properties->msgHeaderType, value);
				break;
			case 10:// KEY_STORE
				strcpy(properties->keyStore, value);
				break;
			case 11:// KEY_PASSWORD
				strcpy(properties->keyPassword, value);
				break;
			case 12:// DATA_ENCRYPTION
				strcpy(properties->dataEncryption, value);
				break;
			default:
			break;
		}
	}

	return properties;
}

void getKeyNValue_1(char* source, char* key, char* value, char* delimeter)
{
	// key 먼저 가져오기
	char* ptr = strtok(source, delimeter);
	strcpy(key, ptr);
	// 두번째, 즉 value 가져오기
	ptr = strtok(NULL, "=");
	// 뒤에 \r 이랑 \n 이 있는지 확인
	int len = strlen(ptr);
	int minus = 0;
	if(ptr[len-2] == '\r' && ptr[len-1] == '\n')
		minus += 2;
	else if(ptr[len-1] == '\n' || ptr[len-1] == '\r')
		minus += 1;
	// value 가져오기
	strncpy(value, ptr, len - minus);

	//fprintf(stderr, "[DEBUG] source:%s, key:%s, value:%s\n", source, key, value);
}

void getKeyNValue_2(char* source, char* key, char* value)
{
	char* temp;
	// 원본 문자열 복사
	temp = strdup(source);
	// key 를 빼오기 위해 원본 문자열 길이 측정
	int ori_len = strlen(temp);
	// strchr 로 = 의 우측 부분 받아옴
	char* ptr = strchr(temp, '=');
	// 원본과 한번 처리한 문자열 길이 비교
	int after_len = strlen(ptr);
	// key 복사
	strncpy(key, source, ori_len - after_len);
	// 맨 뒤에 \r 이랑 \n 있는지 체크해서 value 도 복사
	int minus = 1;
	if(ptr[after_len-2] == '\r' && ptr[after_len-1] == '\n')
		minus += 2;
	else if(ptr[after_len-1] == '\n' || ptr[after_len-1] == '\r')
		minus += 1;
	strncpy(value, ptr + 1, after_len - minus);
	// temp free
	if(temp != NULL) free(temp);
}

#define IFKEYIS(value) 		(strcmp(key, value) == 0)
int getNumFromKey(char* key)
{
	if (IFKEYIS(AUTHTYPE))
		return 1;
	else if (IFKEYIS(HOST))
		return 2;
	else if (IFKEYIS(API_PORT))
		return 3;
	else if (IFKEYIS(API_SSLOPTS))
		return 4;
	else if (IFKEYIS(API_TRUSTSTORE))
		return 5;
	else if (IFKEYIS(THING_PORT))
		return 6;
	else if (IFKEYIS(THING_SSLOPTS))
		return 7;
	else if (IFKEYIS(THING_TRUSTSTORE))
		return 8;
	else if (IFKEYIS(MSGHEADERTYPE))
		return 9;
	else if (IFKEYIS(KEY_STORE))
		return 10;
	else if (IFKEYIS(KEY_PASSWORD))
		return 11;
	else if (IFKEYIS(DATA_ENCRYPTION))
		return 12;
	else
		return 0;
}

bool checkPropertiesAreOkay(Properties* ptr)
{
	bool ret = true;

	char authType = '-';
	// check authType
	if(ISNULLOREMPTY(ptr->authType)) {
		fprintf(stderr, "[ERR] auth type should not be empty.\n");
		ret = false;
	} else if(strlen(ptr->authType) > 1) {
		fprintf(stderr, "[ERR] auth type should be 5 or 6. (%s)\n", ptr->authType);
		ret = false;
	} else {
		authType = (ptr->authType)[0];
		if(!(authType == '5' || authType == '6')) {
			fprintf(stderr, "[ERR] auth Type should be 5 or 6. (%c)\n", authType);
			authType = '-';
			ret = false;
		}
	}

	// check host
	if(ISNULLOREMPTY(ptr->host)) {
		fprintf(stderr, "[ERR] Host should not be empty.\n");
		ret = false;
	}

	// check apiPort
	if(ISNULLOREMPTY(ptr->apiPort)) {
		fprintf(stderr, "[ERR] API Port should not be empty.\n");
		ret = false;
	} else {
		int num = atoi(ptr->apiPort);
		if(num == 0) {
			fprintf(stderr, "[ERR] API Port should be a number. (%s)\n", ptr->apiPort);
			ret = false;
		}
	}

	// check apiSslOpts
	char apiOpt = '-';
	if(ISNULLOREMPTY(ptr->apiSslOpts)) {
		fprintf(stderr, "[ERR] API SSLOPTS should not be empty.\n");
		ret = false;
	} else if(strlen(ptr->apiSslOpts) > 1) {
		fprintf(stderr, "[ERR] API SSLOPTS should be 1 or 2 (%s)\n", ptr->apiSslOpts);
		ret = false;
	} else {
		apiOpt = (ptr->apiSslOpts)[0];
		if(!(apiOpt == '0' || apiOpt == '1')) {
			fprintf(stderr, "[ERR] API SSLOPTS should be 1 or 2 (%c)\n", apiOpt);
			apiOpt = '-';
			ret = false;
		}
	}

	// check apiTrustStore
	if(ISNULLOREMPTY(ptr->apiTrustStore) && apiOpt == '1') {
		fprintf(stderr, "[ERR] API Trust Store should not be empty.\n");
		ret = false;
	}

	// check thingPort
	if(ISNULLOREMPTY(ptr->thingPort)) {
		fprintf(stderr, "[ERR] THING Port should not be empty.\n");
		ret = false;
	} else {
		int num = atoi(ptr->thingPort);
		if(num == 0) {
			fprintf(stderr, "[ERR] THING Port should be a number. (%s)\n", ptr->thingPort);
			ret = false;
		}
	}

	// check thingSslOpts
	char thingOpt = '-';
	if(ISNULLOREMPTY(ptr->thingSslOpts)) {
		fprintf(stderr, "[ERR] THING SSLOPTS should not be empty.\n");
		ret = false;
	} else if(strlen(ptr->thingSslOpts) > 1) {
		fprintf(stderr, "[ERR] THING SSLOPTS should be 0 or 1 or 2. (%s)\n", ptr->thingSslOpts);
		ret = false;
	} else {
		thingOpt = (ptr->thingSslOpts)[0];
		if(thingOpt < '0' || thingOpt > '2') {
			fprintf(stderr, "[ERR] THING SSLOPTS should be 0 or 1 or 2 (%c)\n", thingOpt);
			thingOpt = '-';
			ret = false;
		}
	}

	// check thingTrustStore
	if(ISNULLOREMPTY(ptr->thingTrustStore) && (thingOpt == '1' || thingOpt == '2')) {
		fprintf(stderr, "[ERR] THING Trust Store should not be empty.\n");
		ret = false;
	}

	// check msgHeaderType
	if(ISNULLOREMPTY(ptr->msgHeaderType)) {
		fprintf(stderr, "[ERR] Msg Header Type should not be empty.\n");
		ret = false;
	} else if(strlen(ptr->msgHeaderType) > 1) {
		fprintf(stderr, "[ERR] Msg header Type should be J or D. (%s)\n", ptr->msgHeaderType);
		ret = false;
	} else {
		char msgHT = (ptr->msgHeaderType)[0];
		if (msgHT != 'D' && msgHT != 'J') {
			fprintf(stderr, "[ERR] Msg Header Type should be J or D (%c)\n", msgHT);
			ret = false;
		}
	}

	// check keyStore
	bool bKeyStore = false;
	if(ISNULLOREMPTY(ptr->keyStore)) {
		if(thingOpt == '2') {
			fprintf(stderr, "[ERR] KEY STORE should not be empty.\n");
			ret = false;
		}
	} else {
		if(thingOpt == '2')
			bKeyStore = true;
	}

	// check keyPassword
	if(ISNULLOREMPTY(ptr->keyPassword) && bKeyStore) {
		fprintf(stderr, "[WARN] KEY PASSWORD is empty. If it's not right, check it please.\n");
		//ret = false;
	}

	// check dataEncryption
	if(ISNULLOREMPTY(ptr->dataEncryption)) {
		fprintf(stderr, "[ERR] DATA ENCRYPTION should not be empty.\n");
		ret = false;
	} else if(strlen(ptr->dataEncryption) > 1) {
		fprintf(stderr, "[ERR] auth type should be 0 or 1. (%s)\n", ptr->dataEncryption);
		ret = false;
	} else {
		char enc = (ptr->dataEncryption)[0];
		if(enc != '0' && enc != '1') {
			fprintf(stderr, "[ERR] auth type should be 0 or 1. (%c)\n", enc);
			ret = false;
		}
	}

	return ret;
}

/*****************************************************************************
 * Private 함수들
******************************************************************************/
// id/pw 를 가지고 api manager 를 사용해 open api 로 auth code 를 얻어온다
int getAuthCodeWithIDPW(char* auth_code, Properties* properties)
{
	OCPManager_properties prop = OCPMANAGER_PROPERTIES_INITIALIZER;

	int authType = atoi(properties->authType);
	prop.sslOpts = (authType == 5) ? (properties->apiSslOpts)[0] : '2';
	
	strncpy(prop.host, properties->host, strlen(properties->host));
	strncpy(prop.port, properties->apiPort, strlen(properties->apiPort));
	
	strncpy(prop.trustStore, properties->apiTrustStore, strlen(properties->apiTrustStore));

	apiManager = NULL;
	if(OCPSDK_SUCCESS != OCPAPIManager_create(&apiManager, &prop)) {
		fprintf(stderr, "[ERR] can't make API manager. ssl option(%c), host(%s), port(%s)\n",
			prop.sslOpts, prop.host, prop.port);
		return (-1);
	}
	if(OCPSDK_SUCCESS != OCPAPIManager_do_login(apiManager, connInfo.portalID, connInfo.portalPW)) {
		fprintf(stderr, "[ERR] can't login to API server. id(%s), pw(%s)\n", connInfo.portalID, connInfo.portalPW);
		return (-1);
	}
	
	OCPConnectInfo connectionInfo = OCPAPIManager_get_connectioninfo_ita(apiManager, 
		connInfo.siteId, connInfo.thingName, MODULETYPE, properties->msgHeaderType);
	if(strlen(connectionInfo.authCode) == 0) {
		fprintf(stderr, "[ERR] can't get auth code from API server.\n");
		return (-1);
	}
	
	// fprintf(stderr, "[DEBUG] SSL option:%c\n", connectionInfo.sslOpts);
	// fprintf(stderr, "[DEBUG] msg header type:%s\n", connectionInfo.msgHeaderType);
	// fprintf(stderr, "[DEBUG] encryption type:%s\n", connectionInfo.encType);
	// fprintf(stderr, "[DEBUG] auth code:%s\n", connectionInfo.authCode);

	strcpy(auth_code, connectionInfo.authCode);

	OCPAPIManager_destroy(&apiManager);
	apiManager = NULL;

	fprintf(stderr, "[DEBUG] got auth code with id/pw (%s)\n", auth_code);
	return (0);
}

/*****************************************************************************
 * 보낼 메시지 만들어내는 공용 private 함수
******************************************************************************/
OCPMessage* makeMessage(int type, char* original_data, char* thingName, char* msgCode)
{
	char* encType = (bAES128Used) ? OCPENCTYPE_AES128 : OCPENCTYPE_PLAIN;
	unsigned char* data = NULL;
	int data_len = 0;

	//fprintf(stdout, "[DEBUG][makeMessage] data:(%s) dataFormat(%s)\n", original_data, dfFormat);

	if(bAES128Used) { // should be encrypted
		data = OCPSecurityAES_encrypt_to_string(securityAES, _properties->msgHeaderType, original_data, strlen(original_data), &data_len);
	} else { // encryption is not needed
		data = original_data;
		data_len = strlen(data);
	}

	if(data == NULL || data_len == 0) {
		fprintf(stderr, "[ERR][makeMessage] data is NULL. type(%d)\n", type);
		return NULL;
	}

	switch(type) 
	{
		// iotclient_sendAttributes
		// iotclient_sendAttributesSync
		// iotclient_sendAttributesForLeaf
		// iotclient_sendAttributesSyncForLeaf
		case 1:
		{
			return OCPThingManager_get_register_data_message(thingManager, encType, OCPDATAFORMAT_JSON, data, data_len, thingName, msgCode);
		}
		break;
		// iotclient_sendAttributesInDelimiter
		case 9:
		{
			return OCPThingManager_get_register_data_message(thingManager, encType, OCPDATAFORMAT_DELIMETER, data, data_len, thingName, msgCode);
		}
		break;
		// iotclient_activateThing
		case 2:
		{
			return OCPThingManager_get_activate_rootthing_message(thingManager, encType, OCPDATAFORMAT_JSON, data, data_len);
		}
		break;
		// iotclient_registerLeafThing
		// iotclient_activateThingForLeaf
		case 3:
		{
			return OCPThingManager_get_register_edgething_message(thingManager, encType, OCPDATAFORMAT_JSON, data, data_len, thingName);
		}
		break;
		// iotclient_requestFirmwareLatestVersion
		case 4:
		{
			return OCPThingManager_get_request_activefirmware_message(thingManager, encType, OCPDATAFORMAT_JSON, data, data_len, thingName);
		}
		break;
		// iotclient_requestFirmwareUpgradeComplete
		case 5:
		{
			return OCPThingManager_get_firmware_upgraderesult_message(thingManager, encType, OCPDATAFORMAT_JSON, data, data_len, thingName, "200");
		}
		break;
		// iotclient_requestFirmwareSignData
		case 6:
		{
			return OCPThingManager_get_signedfirmware_message(thingManager, encType, OCPDATAFORMAT_JSON, data, data_len);
		}
		break;
		// iotclient_requestFileUploadComplete
		case 7:
		{
			return OCPThingManager_get_fileupload_result_message(thingManager, encType, OCPDATAFORMAT_JSON, data, data_len, thingName);
		}
		break;
		// iotclient_requestWbcMc
		case 8:
		{
			return OCPThingManager_get_wbc_mc(thingManager, encType, OCPDATAFORMAT_JSON, data, data_len, thingName);
		}
		break;
		default:
		{
			fprintf(stderr, "[ERR][makeMessage] Wrong Type (%d)\n", type);
		}
		break;
	}

	return NULL;	// here is the error case
}

/*****************************************************************************
 * JSON Parsing 함수들
******************************************************************************/
/* Response of Request of ACTIVE FIRMWARE Latest Version
	{
		"resultList": 
		[
			{
				"fileUri":"http://local.insator.io:8081/contentsService?accessKey=e4e158b8-0173-1000-0000-a651fb559a16&downloadKey=9a4c66a6-0174-1000-0000-61f32aa39085",
		  		"modelName":"SampleModel",
		  		"type":"patch",
		  		"version":"3.0"
		  	}
		]
	}
 */

/* Notification of PASSIVE FIRMWARE
	{
		"requestList":
		[
			{
				"fileUri":"http://local.insator.io:8081/contentsService?accessKey=e4e158b8-0173-1000-0000-a651fb559a16&downloadKey=9a5adc5c-0174-1000-0000-aa2a827ce464",
				"type":"patch",
				"modelName":"SampleModel",
				"version":"3.0"
			}
		],
		"upgradeId":"U000002",
		"upgradeOption":"FOR"
	}
 */
/* Response of Request of WBC MC value
	{
		"thingId":"TEST_TYPE.ITA001",
		"encType":"4",
		"encryptedMC":"KHjaHzIxusY7r1kWf11PCrz5PWM+33xT3hHOsPJrP70xGZWJX0LbQOhHmV/RLRN6"
	}
 */
// json 에서 firmware info struct 를 추출한다
FRMInfo* getFirmwareInfoFromMessage(void* data)
{
	if(data == NULL) return NULL;

	char* value = NULL;
	json_t* object = NULL;
	json_error_t error;

	object = json_loads((char*)data, 0, &error);	// 전체 json load
	if(object == NULL) return NULL;

	size_t size;
	const char *key;
	json_t *value_1;
	size = json_object_size(object); // object 개수 : 1개일거임 resultList 와 value

	FRMInfo* info = (FRMInfo*)malloc(sizeof(FRMInfo));
	if(info == NULL)
		return NULL;
	else { // upgradId 의 알수없는 오류 때문에 미리 초기화 진행
		memset(info->fileUri, 0x00, strlen(info->fileUri));
		memset(info->modelName, 0x00, strlen(info->modelName));
		memset(info->type, 0x00, strlen(info->type));
		memset(info->version, 0x00, strlen(info->version));
		memset(info->upgradeId, 0x00, strlen(info->upgradeId));
	}

	json_t *requestList, *resultList, *upgradeId;
	const char *text;

	requestList = json_object_get(object, "requestList");
	resultList = json_object_get(object, "resultList");

	json_t* temp = (requestList != NULL) ? requestList : resultList;
	json_t *value_2;
	if(temp != NULL) {
			json_object_foreach(json_array_get(temp, 0), key, value_2) {
			// key: 아래 4개, value_2: string
			const char* strValue = json_string_value(value_2);
			if(!strcmp(key, "fileUri") && strValue != NULL) 	memcpy(info->fileUri, strValue, strlen(strValue));
			if(!strcmp(key, "modelName") && strValue != NULL)	memcpy(info->modelName, strValue, strlen(strValue));
			if(!strcmp(key, "type") && strValue != NULL) 		memcpy(info->type, strValue, strlen(strValue));
			if(!strcmp(key, "version") && strValue != NULL) 	memcpy(info->version, strValue, strlen(strValue));
		}
	}
	upgradeId = json_object_get(object, "upgradeId");
	if(upgradeId != NULL) {
		const char* id = json_string_value(upgradeId);
		memcpy(info->upgradeId, id, strlen(id));
	}
	return info;
}

char* getUploadUriFromMessage(void* data)
{
	if(data == NULL) return NULL;

	char* value = NULL;
	json_t* object = NULL;
	json_error_t error;

	object = json_loads((char*)data, 0, &error);
	if(object == NULL) return NULL;

	size_t size;
	const char *key;
	json_t *value_1;
	size = json_object_size(object); // 2개일거임 uploadUri, downloadUri

	json_t *uploadUri, *downloadUri;
	char* ret_text = NULL;

	uploadUri = json_object_get(object, "uploadUri");
	if(uploadUri != NULL) {
		const char* id_1 = json_string_value(uploadUri);
		printf("uploadUri:%s\n", id_1);

		int length = strlen(id_1);
		ret_text = (char*)malloc(length + 1);
		memset(ret_text, 0x00, length + 1);
		memcpy(ret_text, id_1, length);
	}
	downloadUri = json_object_get(object, "downloadUri");
	if(downloadUri != NULL) {
		const char* id_2 = json_string_value(downloadUri);
		printf("downloadUri:%s\n", id_2);
	}

	return ret_text;
}

const char* getEncryptedMCFromMessage(void* data)
{
	if(data == NULL) return NULL;

	json_error_t error;
	json_t* object = NULL;
	object = json_loads((char*)data, 0, &error);
	if(object == NULL) return NULL;

	json_t *value = json_object_get(object, "encryptedMC");
	if(value != NULL) {
		const char* v_2 = json_string_value(value);
		//fprintf(stdout, "[OK] Got encryptedMC, its value is (%s)\n", v_2);
		return v_2; 
	} else {
		fprintf(stderr, "[ERR] Can't get encryptedMC from message.\n");
		return NULL;
	}
}

/*****************************************************************************
 * Thread - send Q 에 있는 메시지를 send 한다
******************************************************************************/
int fail_count = 0;
void* thread_sender(void* args)
{
	while(thread_all_flag)
	{
		if(send_queue->size > 0)
		{
			OCPMessage* message = (OCPMessage*) (send_queue->first->message);

			char* res = OCPThingManager_sendMessage(thingManager, message);
			if(res != NULL && !strcmp(res, message->msgId))
			{
				fprintf(stdout, "[>>>] sended %s\n", message->msgCode);
				OCPManager_free_message(message);
				delete_queue(send_queue);
			}
			else
			{
				fprintf(stderr, "[ERR] send fail [%d]\n", fail_count++);
			}

			if(fail_count > 3)
			{
				fprintf(stderr, "[ERR] failed to send 3 times (%s)\n", message->msgCode);
				fail_count = 0;
				OCPManager_free_message(message);
				delete_queue(send_queue);
			}
		}
		sleep(1);
	}
}

/*****************************************************************************
 * Thread - receive Q 에 있는 메시지를 처리하고 Q 에서 지운다
******************************************************************************/
void* thread_receiver(void* args)
{
	while(thread_all_flag)
	{
		if(receive_queue->size > 0)
		{
			OCPMessage* message = (OCPMessage*)(receive_queue->first->message);
			if(message)
			{
//==========================================================================================================
//			Application 에게 줄 필요없이 iot client 가 처리하고 끝내는 경우
//==========================================================================================================
				if(!strcmp(message->msgCode, OCPMESSAGE_MSGCODE_AUTHPROCESS_RES))
				{ // AUTH_RESPONSE 는 암호화 안되어 있고, iot client 가 처리하고 끝
					memset(auth_response, 0, 1024);
					memcpy(auth_response, message->data, (int)strlen(message->data));

					int semval = -1;
					sem_getvalue(sem_forConn, &semval);
					if(semval == 0)
						sem_post(sem_forConn);
					
					OCPManager_free_message(message);
					fprintf(stderr, "[<<<] pre-process OCPMESSAGE_MSGCODE_AUTHPROCESS_RES\n");
				}
				else
				{
//==========================================================================================================
//			Application 에 돌려주기 전에 선처리해야 하는 메세지들을 전처리한다.
//==========================================================================================================
					int 	msgType = 0;
					char* 	msgName = NULL;
					// 아래 if-else 에 있는 메세지들은 선처리 후에 application 에게 건네준다
					if( !strcmp(message->msgCode, OCPMESSAGE_MSGCODE_REQUEST_ACTIVEFIRMWARE_RES) ) {
						msgName = "REQUEST_ACTIVEFIRMWARE_RES";
						msgType = 1;
					} else if( !strcmp(message->msgCode, OCPMESSAGE_MSGCODE_NOTI_PASSIVEFIRMWARE_REQ) )	{
						msgName = "NOTI_PASSIVEFIRMWARE_REQ";
						msgType = 2;
					} else if( !strcmp(message->msgCode, OCPMESSAGE_MSGCODE_BASIC_ATTR_GROUP) ) {
						msgName = "BASIC_ATTR_GROUP";
						msgType = 3;
					} else if( !strcmp(message->msgCode, OCPMESSAGE_MSGCODE_BASIC_PROVISIONING) ) {
						msgName = "BASIC_PROVISIONING";
						msgType = 4;
					} else if( !strcmp(message->msgCode, OCPMESSAGE_MSGCODE_REQUEST_UPLOADURI_RES) ) {
						msgName = "REQUEST_UPLOADURI_RES";
						msgType = 5;
					} else if(!strcmp(message->msgCode, OCPMESSAGE_MSGCODE_REGISTER_THING_ATTRIBUTE_RES)) {
						// send attribute 의 response 인 경우 sync call 일 수 있음
						sync_resCode = atoi(message->resCode);
						int semval = -1;
						sem_getvalue(sem_forSync, &semval);
						if(semval == 0) {
							sem_post(sem_forSync);
						}
						// 이 경우, sem_post 만 하고, 다른 전처리는 필요없으므로 msgType 설정안하고 application 에게 메시지 by-pass
					} else if(!strcmp(message->msgCode, OCPMESSAGE_MSGCODE_NOTIREQUEST_REQ)) {
						// NOTI REQUEST 가 들어오면 response 를 줘야함. 그리고 application 에게 by-pass
						msgName = "NOTIREQUEST_REQ";
						msgType = 6;
					} else if(!strcmp(message->msgCode, OCPMESSAGE_MSGCODE_WBC_MC_RES)) {
						// Wbc MC Response 가 들어오면, encryptedMC 를 떼어내서 data 에 다시 설정
						msgName = "WBC_MC_RES";
						msgType = 7;
					} else {
						fprintf(stdout, "[<<<] %s\n", message->msgCode);
					}					
				//==========================================================================================================
				// 1. SDK 의 encType 이 "3" > bAES128Used
				// 2. message->data 가 NULL 이 아님 (body data 가 있다는 의미)
				// 3. 도착한 메시지의 encType 이 "3"
				//==========================================================================================================
					if(bAES128Used && (message->data) != NULL && !strcmp((char*)message->encType, "3"))
					{ // message->data 를 복호화 먼저
	// fprintf(stderr, "[<<<!] msgType : %s\n", message->msgType);
    // fprintf(stderr, "[<<<!] msgId   : %s\n", message->msgId);
    // fprintf(stderr, "[<<<!] msgCode : %s\n", message->msgCode);
    // fprintf(stderr, "[<<<!] resCode : %s\n", message->resCode);
    // fprintf(stderr, "[<<<!] encType : %s\n", message->encType);
    // fprintf(stderr, "[<<<!] data(%d): %s\n", (int)message->datalen, (char*)message->data);
						char* encrypted_data = (char*)(message->data);
						int decrypted_len = 0;
						unsigned char* decrypted = OCPSecurityAES_decrypt(
							securityAES, _properties->msgHeaderType, encrypted_data, (int)message->datalen, &decrypted_len);
							//securityAES, _properties->msgHeaderType, encrypted_data, strlen(encrypted_data), &decrypted_len);
						if(decrypted != NULL) {
							fprintf(stdout, "\n[<<<] encrypted %s\n", msgName);
							free(encrypted_data);
							message->data = decrypted;
							// fprintf(stdout, "[DEBUG] decrypted data:%s\n", decrypted);
						} else {
							fprintf(stdout, "[<<<] encrypted %s, but message is broken\n", msgName);
							OCPManager_free_message(message);
							msgType = -1;
						}
					}

				//==========================================================================================================
				// (msgType : 1) Active Firmware Response, body 의 firmware 정보 json 을 풀어서 FRMInfo 로 돌려줘야함
				// (msgType : 2) Passive Firmware Request, body 의 firmware 정보 json 을 풀어서 FRMInfo 로 돌려줘야함 upgradeId 있음
				// (msgType : 3) basic attr group "Q", answer 보내야함
				// (msgType : 4) basic provisioning "Q", answer 보내야함
				// (msgType : 7) wbc mc response, body 의 json 을 풀어서 encryptedMC 를 돌려줘야함
				//==========================================================================================================
					if(msgType > 0)
					{
						fprintf(stdout, "[<<<] %s\n", msgName);
						if(msgType == 1 || msgType == 2) {
						// 2. json 을 풀어서 FRMInfo 로 돌려줄거임
							FRMInfo* info = getFirmwareInfoFromMessage(message->data);
							if(info == NULL) {
								fprintf(stdout, "[<<<] %s, can't get json object\n", msgName);
								OCPManager_free_message(message);
								msgType = -1;
							} else {
								free(message->data);
								message->data = (void*)info;
							}
						} else if(msgType == 5) {
							char* uri = getUploadUriFromMessage(message->data);
							if(uri == NULL) {
								fprintf(stdout, "[<<<] %s, can't get json object\n", msgName);
							} else {
								free(message->data);
								message->data = (void*)uri;
							}
						} else if(msgType == 7) {
							const char* mc = getEncryptedMCFromMessage(message->data);
							if(mc == NULL) {
								fprintf(stdout, "[<<<] %s, can't get json object\n", msgName);
							} else {
								free(message->data);
								message->data = (void*)mc;
							}
						}

				//==========================================================================================================
				// (msgType : 2) 의 경우에는 response 를 줘야 함
				//==========================================================================================================
						if(msgType == 2) {
							// upgradeId 를 가져옴 > data string 을 만들어냄
							char data[LONG_STR_LEN] = {0, };
							sprintf(data, RES_NOTI_PASS_FRM, ((FRMInfo*)(message->data))->upgradeId);
							// make BIOT response message 
							char* msgId = (char*)(message->msgId);
							OCPMessage* send_msg = NULL;
							if(bAES128Used && !strcmp((char*)message->encType, "3")) {
								int enc_data_len = 0;
								unsigned char* enc_data = OCPSecurityAES_encrypt_to_string(securityAES, _properties->msgHeaderType, data, strlen(data), &enc_data_len);
								send_msg = OCPThingManager_get_response_passivefirmware_message(
									thingManager, OCPENCTYPE_AES128, OCPDATAFORMAT_JSON, enc_data, enc_data_len, msgId, connInfo.thingName);
							} else {
								send_msg = OCPThingManager_get_response_passivefirmware_message(
									thingManager, OCPENCTYPE_PLAIN, OCPDATAFORMAT_JSON, data, strlen(data), msgId, connInfo.thingName);
							}
							// send it
							add_queue(send_queue, send_msg);
							fprintf(stdout, "[DEBUG] PASSIVE FIRMWARE NOTIFY Response is added (%s)\n", data);
						}
				//==========================================================================================================
				// 공통으로 get_answer_mesage 를 사용하는 경우
				// (msgType : 3) response 줘야 함 msgCode : Basic-AttrGroup
				// (msgType : 4) response 줘야 함 msgCode : Basic-Provisioning
				//==========================================================================================================
						else if(msgType == 3 || msgType == 4)
						{
							char* msgId = (char*)(message->msgId);
							char* msgCode = message->msgCode;
							char* data = "{}";

							OCPMessage* send_msg = NULL;
							if(bAES128Used && !strcmp((char*)message->encType, "3")) {
								int enc_data_len = 0;
								unsigned char* enc_data = OCPSecurityAES_encrypt_to_string(securityAES, _properties->msgHeaderType, data, strlen(data), &enc_data_len);
								send_msg = OCPThingManager_get_answer_message(
									thingManager, OCPENCTYPE_AES128, OCPDATAFORMAT_JSON, enc_data, enc_data_len, msgId, msgCode);
							} else {
								send_msg = OCPThingManager_get_answer_message(
									thingManager, OCPENCTYPE_AES128, OCPDATAFORMAT_JSON, data, strlen(data), msgId, msgCode);
							}

							add_queue(send_queue, send_msg);
							fprintf(stdout, "[DEBUG] A Response is added (%s)\n", msgCode);
						}
				//==========================================================================================================
				// 이 경우 NOTI REQUEST RES 를 보내야함
				// (msgType : 6) NOTI REQUEST REQ
				//==========================================================================================================
						else if(msgType == 6)
						{
							char* msgId = (char*)(message->msgId);
							char* msgCode = message->msgCode;
							char* data = (char*)message->data;

							OCPMessage* send_msg = NULL;
							if(bAES128Used && !strcmp((char*)message->encType, "3")) {
								int enc_data_len = 0;
								unsigned char* enc_data = OCPSecurityAES_encrypt_to_string(securityAES, _properties->msgHeaderType, data, strlen(data), &enc_data_len);
								send_msg = OCPThingManager_get_response_notirequest_message(
									thingManager, OCPENCTYPE_AES128, OCPDATAFORMAT_JSON, enc_data, enc_data_len, msgId);
							} else {
								send_msg = OCPThingManager_get_response_notirequest_message(
									thingManager, OCPENCTYPE_PLAIN, OCPDATAFORMAT_JSON, data, strlen(data), msgId);
							}

							add_queue(send_queue, send_msg);
							fprintf(stdout, "[DEBUG] NOTIREQUEST_RES is added (%s)\n", data);
						}
				//==========================================================================================================
					}

				//==========================================================================================================
				// 전처리를 마친 message 를 application 에게 전달한다.
				// 전처리 중에 문제가 생긴 경우 msgType = -1 로 설정된다. 이 경우 전달하지 않음.
				//==========================================================================================================
					if(listenerOfApplication != NULL && msgType >= 0)
						listenerOfApplication(args, message);
//==========================================================================================================			
				}


//==========================================================================================================			
			} // end of if message
			delete_queue(receive_queue);
		} // end of if receive queue size

		sleep(1);
	}
}

/*****************************************************************************
 * 주기적으로 alive signal 을 보내는 thread
******************************************************************************/
// 약 2분마다 heart beat signal 을 보낸다.
void* thread_keep_alive(void* args)
{
	while(thread_all_flag) {
		if(thread_conn_flag) { // connected
			iotclient_sendHeartBeat();
		}
		sleep(KEEP_ALIVE_TERM_SECONDS);
	}
	fprintf(stdout, "[OK] heart beat thread is terminated.\n");
}
