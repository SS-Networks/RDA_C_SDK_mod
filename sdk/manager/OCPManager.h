/**
 * 
 * 
 * @author mw50.jeong
 * @since 2018. 2. 1.
 */

#ifndef _OCPManager_
#define _OCPManager_

#include "OCPMessage.h"
#include "OCPTypes.h"

// log trace for manager
#define OCPMANAGER_TRACE          "OCPMANAGER"
// manager pointer
typedef void* OCPManager;
// manager properties struct
typedef struct 
{
    char sslOpts;
    char host[OCPMANAGER_PROPERTY_LEN];
    char port[OCPMANAGER_PROPERTY_LEN];
    char trustStore[OCPMANAGER_PROPERTY_LEN];
    int timeout;              //sec
    char msgHeaderType[OCPMANAGER_PROPERTY_LEN];
    int minCompressionSize;
    char protocolType[OCPMANAGER_PROPERTY_LEN]; 
    char siteId[OCPMANAGER_PROPERTY_LEN];
    char thingName[OCPMANAGER_PROPERTY_LEN];
    char keyStore[OCPMANAGER_PROPERTY_LEN];
    char keyPassword[OCPMANAGER_PROPERTY_LEN];
    char publishTopic[OCPMANAGER_PROPERTY_LEN];
    char subscribeTopic[OCPMANAGER_PROPERTY_LEN];    
    int keepAliveInterval;    //sec
    char authCode[OCPMANAGER_PROPERTY_LEN];
}OCPManager_properties;
// manager properties initializer
#define OCPMANAGER_PROPERTIES_INITIALIZER { OCPSSLOPTS_TCP, {0,}, {0,}, {0,}, 10, OCPMESSAGECONVERTER_JSON, 0, OCPPROTOCOL_MQTT, {0,}, {0,}, {0,}, {0,}, "ocp/dataBus", "ocp/", 60, {0,} }
// manager callback on messsage arrive
typedef void OCPManager_onMessageArrive(void* context, OCPMessage* message);
// managercallback on messsage arrive
typedef void OCPManager_onConnectionClosed(void* context, char* cause);
// manager callabck struct 
typedef struct 
{
  OCPManager_onMessageArrive *ma;
  OCPManager_onConnectionClosed *cl;
}OCPManager_callback;
// manager callback initiailzer 
#define OCPMANAGER_CALLBACK_INITIALIZER { NULL, NULL }
// free ocpmessage 
void OCPManager_free_message(OCPMessage* message);
// print ocpmessage
void OCPManager_print_message(OCPMessage* message);

#endif