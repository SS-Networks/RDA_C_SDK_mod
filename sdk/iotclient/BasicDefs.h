#ifndef _BASICDEF_H
#define _BASICDEF_H

#include "IotClient.h"
//========================================================================================================================
// message strings
//========================================================================================================================
// activate root thing
#define ACTIVATE_ROOT 		"{\"modelName\":\"%s\",\"uniqueNum\":\"%s\"}"
// register leaf(edge) thing
#define REGISTER_EDGE 		"{\"modelName\":\"%s\",\"uniqueNum\":\"%s\",\"parentThingId\":\"%s\"}"
// request active firmware latest version information
#define REQ_ACTIVE_FRM	 	"{\"requestList\":[{\"modelName\":\"%s\",\"type\":\"%s\"}]}"
// response firmware upgrader result
#define FRM_UPGRADE_RES 	"{\"type\":\"%s\",\"version\":\"%s\"}"
// request signed data of firmware
#define REQ_SIGNED_FRM    	"{\"fileUri\":\"%s\"}"
// response of passive firmware notification
#define RES_NOTI_PASS_FRM	"{\"upgradeId\":\"%s\"}"
// for authorization
#define AUTHORIZE_MSG		"{\"authCode\":\"%s\"}"
// file upload complete
#define FILE_UPLOAD_DONE	"{\"fileName\":\"%s\",\"fileUri\":\"%s\"}"
// request wbc mc
#define REQ_WBC_MC          "{\"thingId\":\"%s\",\"encType\":\"4\"}"
//========================================================================================================================
// string definitions for property
//========================================================================================================================
#define AUTHTYPE            "authType"
// host name
#define HOST                "host"
// OAG server port
#define API_PORT            "apiPort"
// OAG server ssl option
#define API_SSLOPTS 	    "apiSslOpts"
// OAG server certificate
#define API_TRUSTSTORE      "apiTrustStore"
// MQTT server port
#define THING_PORT          "thingPort"
// MQTT server ssl option
#define THING_SSLOPTS       "thingSslOpts"
// MQTT server certificate
#define THING_TRUSTSTORE    "thingTrustStore"
// message header type
#define MSGHEADERTYPE       "msgHeaderType"
// thing's certificate
#define KEY_STORE           "keyStore"
// password of key store file
#define KEY_PASSWORD        "keyPassword"
// aes128 used or not
#define DATA_ENCRYPTION     "dataEncryption"


// default property file name.
#define PROPERTY_FILENAME 		"biot_client.properties"
// send heart-beat signal every KEEP_ALIVE_TERM_SECONDS seconds
#define KEEP_ALIVE_TERM_SECONDS		120
// connection module type
#define MODULETYPE      	"mqtt" 

//========================================================================================================================
// typedef
//========================================================================================================================
typedef enum {false, true} bool;
typedef void IotClient_MsgListener(void* context, OCPMessage* response);
typedef void IotClient_ConnClosed(void* context, char* cause);
typedef struct _connectionInfo {
    char portalID[SHORT_STR_LEN];
    char portalPW[SHORT_STR_LEN];
    char authCode[SHORT_STR_LEN];
    char siteId[SHORT_STR_LEN];
    char thingName[SHORT_STR_LEN];	// root thing's name
} ConnInfo;

//========================================================================================================================
// functions provided by pointer to user application
//========================================================================================================================
iot_status_code 	iotclient_connect(void);
void 				iotclient_disconnect();
void 				iotclient_setCustomMessageListener(void (*messageArrive)(void*, OCPMessage*), void (*connectionClosed)(void*, char*));
void 				iotclient_sendAttributes(char* msgCode, char* dataStr);
void 				iotclient_sendAttributesInDelimiter(char* msgCode, char* dataStr);
iot_status_code 	iotclient_sendAttributesSync(char* msgCode, char* dataStr);
void 				iotclient_sendAttributesForLeaf(char* leafThingName, char* msgCode, char* dataStr);
void 				iotclient_sendAttributesForLeafInDelimiter(char* leafThingName, char* msgCode, char* dataStr);
iot_status_code 	iotclient_sendAttributesSyncForLeaf(char* leafThingName, char* msgCode, char* dataStr);
iot_status_code 	iotclient_activateThing(char* modelName, char* uniqueNum);
iot_status_code 	iotclient_registerLeafThing(char* leafThingName, char* modelName, char* uniqueNum);
iot_status_code 	iotclient_activateThingForLeaf(char* leafThingName, char* modelName, char* uniqueNum);
iot_status_code 	iotclient_requestFirmwareLatestVersion(char* modelName, char* firmwareType);
iot_status_code 	iotclient_requestFirmwareUpgradeComplete(char* thingName, char* firmwareType, char* version);
iot_status_code 	iotclient_requestFirmwareSignData(char* fileUri);
iot_status_code 	iotclient_requestFileUploadUri();
iot_status_code 	iotclient_requestFileUploadComplete(char* fileName, char* fileUri);
iot_status_code     iotclient_requestWbcMc();
void 				iotclient_sendHeartBeat(void);

//========================================================================================================================
// function only used internally
//========================================================================================================================
Properties*     readProperties();
FRMInfo*        getFirmwareInfoFromMessage(void* data);

void 	        getKeyNValue_1(char* source, char* key, char* value, char* delimeter);
void 	        getKeyNValue_2(char* source, char* key, char* value);
int 	        getNumFromKey(char* key);
bool 	        checkPropertiesAreOkay(Properties* ptr);
int 	        getAuthCodeWithIDPW(char* auth_code, Properties* properties);
char* 	        getEncryptionType();

void 	        preListener(void* context, OCPMessage* message);
void 	        preHandlerConnClosed(void* context, char* cause);

OCPMessage*     makeMessage(int type, char* original_data, char* thingName, char* msgCode);








#endif // _BASICDEF_H