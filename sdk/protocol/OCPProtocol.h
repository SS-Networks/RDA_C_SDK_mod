/**
 * 
 * 
 * @author mw50.jeong
 * @since 2018. 2. 1.
 */

#ifndef _OCPProtocol_
#define _OCPProtocol_
#include "OCPTypes.h"
#include "OCPUtils.h"

// log trace for protocol
#define OCPPROTOCOL_TRACE           "OCPPROTOCOL"
// protocol properties struct
typedef struct
{
    char sslOpts;
    char host[OCPMANAGER_PROPERTY_LEN];
    char port[OCPMANAGER_PROPERTY_LEN];
    char trustStore[OCPMANAGER_PROPERTY_LEN];
    char keyStore[OCPMANAGER_PROPERTY_LEN];
    char keyPassword[OCPMANAGER_PROPERTY_LEN];
    int timeout;
    int keepAliveInterval;
    char siteId[OCPMANAGER_PROPERTY_LEN];
    char thingName[OCPMANAGER_PROPERTY_LEN];
    char publishTopic[OCPMANAGER_PROPERTY_LEN];
    char subscribeTopic[OCPMANAGER_PROPERTY_LEN];
    int messageQos;    
    char password[OCPMANAGER_PROPERTY_LEN];
}OCPProtocol_properties;
// protocol properties initializer
#define OCPPROTOCOL_PROPERTIES_INITIALIZER { '0', {0,}, {0,}, {0,}, {0,}, {0,}, 0, 0, {0,}, {0,}, {0,}, {0,}, 0, {0,} }
// ocpprotocol callback on connect
typedef void OCPProtocol_onConnect(void* context);
// ocpprotocol callback on disconnect
typedef void OCPProtocol_onDisconnect(void* context);
// ocpprotocol callback on message arrive
typedef void OCPProtocol_onMessageArrive(void* context, char* response);
// ocpprotocol callback on connectino closed
typedef void OCPProtocol_onConnectionClosed(void* context, char* cause);
// ocpprotocol callback struct
typedef struct 
{
  OCPProtocol_onConnect *cn;
  OCPProtocol_onDisconnect *dc;
  OCPProtocol_onMessageArrive *ma;
  OCPProtocol_onConnectionClosed *cl;
}OCPProtocol_callback;

#endif