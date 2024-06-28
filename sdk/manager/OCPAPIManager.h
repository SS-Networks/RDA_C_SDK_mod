/**
 * 
 * 
 * @author mw50.jeong
 * @since 2018. 2. 1.
 */

#ifndef _OCPApiManager_
#define _OCPApiManager_

#include "OCPManager.h"

// connection info ip string 
#define OCPCONNECTIONINFO_IP 			"ip"
// connection info port string 
#define OCPCONNECTIONINFO_PORT			"port"
// connection info sslOpts string 
#define OCPCONNECTIONINFO_SSLOPTS		"sslOpts"
// connection info msgHeaderType string 
#define OCPCONNECTIONINFO_MSGHEADERTYPE	"msgHeaderType"
// connection info encType string 
#define OCPCONNECTIONINFO_ENCTYPE		"encType"
// connection info authCode string 
#define OCPCONNECTIONINFO_AUTHCODE		"authCode"
// connection info struct 
typedef struct {
  char ip[OCPMANAGER_PROPERTY_LEN];
  char port[OCPMANAGER_PROPERTY_LEN];
  char sslOpts;
  char msgHeaderType[OCPMANAGER_PROPERTY_LEN];
  char encType[OCPMANAGER_PROPERTY_LEN];
  char authCode[OCPMANAGER_PROPERTY_LEN];  
}OCPConnectInfo;
// connection info initiailzer
#define OCPCONNECTINFO_INITIALIZER { {0,}, {0,}, '0', {0,}, {0,}, {0,} }
// create manager
int OCPAPIManager_create(OCPManager* handle, OCPManager_properties* properties);
// destroy manager
void OCPAPIManager_destroy(OCPManager* handle);
// get userToken
int OCPAPIManager_do_login(OCPManager handle, char* userId, char* userkey);
// check login status
int OCPAPIManager_islogined(OCPManager handle);
// get connection info for ita
//OCPConnectInfo OCPAPIManager_get_connectioninfo_ita(OCPManager handle, char* siteId, char* thingName, char* thingModuleType);
OCPConnectInfo OCPAPIManager_get_connectioninfo_ita(OCPManager handle, char* siteId, char* thingName, char* thingModuleType, char* messageHeaderType);
// get all rootthing in site
char* OCPAPIManager_get_all_rootthings(OCPManager handle, char* siteId, unsigned int limit, unsigned int offset);
// get direct(one depth) edgething in thing 
char* OCPAPIManager_get_direct_edgethings(OCPManager handle, char* siteId, char* thingName, unsigned int limit, unsigned int offset);
// get all(ignore depth) edgething in thing 
char* OCPAPIManager_get_all_edgethings(OCPManager handle, char* siteId, char* thingName);
// get thing attributes 
char* OCPAPIManager_get_thing_attributes(OCPManager handle, char* siteId, char* thingName);

char* OCPAPIManager_do_custom(OCPManager handle, char* service, char* dataString);

#endif