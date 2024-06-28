#ifndef _IOTCLIENT_H
#define _IOTCLIENT_H

// short string length
#define SHORT_STR_LEN    128
// long string length
#define LONG_STR_LEN    1024

#include "OCPManager.h"
#include "OCPMessage.h"
#include "IotClient_ErrCode.h"
/*****************************************************************************
- Firmware Information
- If request latest firmware information, will receive this information via message Arrive function
- But not implemented yet
******************************************************************************/
typedef struct _FRMInfo {
    char   version[SHORT_STR_LEN];
    char   fileUri[LONG_STR_LEN];
    char   modelName[SHORT_STR_LEN];
    char   type[SHORT_STR_LEN];
    char   upgradeId[SHORT_STR_LEN];
} FRMInfo;

/*****************************************************************************
- Properties used when connecting to BIOT server.
- Please refer to README.md file for detailed information
******************************************************************************/
typedef struct _properties {
    char authType       [SHORT_STR_LEN];
    char host           [SHORT_STR_LEN];
    
    char apiPort        [SHORT_STR_LEN];
    char apiSslOpts     [SHORT_STR_LEN];
    char apiTrustStore  [LONG_STR_LEN];

    char thingPort      [SHORT_STR_LEN];
    char thingSslOpts   [SHORT_STR_LEN];
    char thingTrustStore[LONG_STR_LEN];

    char msgHeaderType  [SHORT_STR_LEN];
    
    char keyStore       [LONG_STR_LEN];
    char keyPassword    [SHORT_STR_LEN];
    
    char dataEncryption [SHORT_STR_LEN];
} Properties;

/*****************************************************************************
- IotClient
- how to use, refer to the samples
******************************************************************************/
typedef struct _iotclient {

    // Properties of this client
    Properties*     properties;

    // connect to server. before this, should create iot client.
    iot_status_code (*connect)(void);
    
    // disconnect the connection
    void            (*disconnect)(void);
    
    // set the callback function pointers. should do this before connect. both of these can be NULL.
    // messageArrive : will be called when a message comes from the server
    // connectionClosed : will be called when the connection is closed
    void            (*setCustomMessageListener)(void (*messageArrive)(void*, OCPMessage*), void (*connectionClosed)(void*, char*));
    
    // send attribute values of Root Thing to the server
    void            (*sendAttributes)(char* msgCode, char* dataStr);

    void            (*sendAttributesInDelimiter)(char* msgCode, char* dataStr);

    // send attribute values of Root Thing to the server and wait for the response
    // it's not working yet in C SDK
    iot_status_code (*sendAttributesSync)(char* msgCode, char* dataStr);
    
    // send attribute values of Leaf Thing to the server
    void            (*sendAttributesForLeaf)(char* leafThingName, char* msgCode, char* dataStr);

    void            (*sendAttributesForLeafInDelimiter)(char* leafThingName, char* msgCode, char* dataStr);

    // send attribute values of Leaf Thing to the server and wait for the response
    // it's not working yet in C SDK
    iot_status_code (*sendAttributesSyncForLeaf)(char* leafThingName, char* msgCode, char* dataStr);
    
    // activate a Root Thing
    iot_status_code (*activateThing)(char* modelName, char* uniqueNum);
    
    // register a Leaf Thing
    iot_status_code (*registerLeafThing)(char* leafThingName, char* modelName, char* uniqueNum);
    
    // activate a Leaf Thing
    iot_status_code (*activateThingForLeaf)(char* leafThingName, char* modelName, char* uniqueNum);
    
    // Active Firmware Upgrade. requests latest firmware information to the server.
    iot_status_code (*requestFirmwareLatestVersion)(char* modelName, char* firmwareType);
    
    // after firmware upgrade, tells that it's done to the server
    iot_status_code (*requestFirmwareUpgradeComplete)(char* thingName, char* firmwareType, char* version);
    
    // request the signed data of firmware file with file uri
    iot_status_code (*requestFirmwareSignData)(char* fileUri);
    
    // not working yet in C SDK
    iot_status_code (*requestFileUploadUri)(void);
    
    // not working yet in C SDK
    iot_status_code (*requestFileUploadComplete)(char* fileName, char* fileUri);

    iot_status_code (*requestWbcMc)(void);

} IotClient;

/*****************************************************************************
- Global Functions 
******************************************************************************/

// create IotClient using property and more information
// 1) should have a proper 'property file' in right path.
// 2) when using ITA, should put auth code or (id and password)
// 3) when using SSL, forget about (auth code, id, password)
// 4) But should put site id, thing name always.
IotClient*  createIotClient(char* authCode, char* portalID, char* portalPW, char* siteId, char* thingName);

// it will destroy IotClient. Also free memories of client itself.
void        destroyIotClient(IotClient* client);

// free OCPMessage
// when receives a message from the server -> handle it -> free the message using this
void        freeMessage(OCPMessage* message);

#endif // _IOTCLIENT_H
