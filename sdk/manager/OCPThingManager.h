/**
 * 
 * 
 * @author mw50.jeong
 * @since 2018. 2. 1.
 */

#ifndef _OCPThingManager_
#define _OCPThingManager_

#include "OCPManager.h"

// preauthorize result sturct 
typedef struct 
{
    char apiKey[1024];
    char apiSecretKey[1024];
    char issueTimestamp[1024]; 
    char expireDate[1024];
    char data[1024];
}OCPPreauthorize_result;
// preauthorize result initializer
#define OCPPREAUTHORIZE_RESULT_INITIALIZER { {0,}, {0,}, {0,}, {0,}, {0,} }
// authorize result struct 
typedef struct 
{
    char encType[1024];
    char authToken[1024];
}OCPAuthorize_result;
// authorize result initializer 
#define OCPAUTHORIZE_RESULT_INITIALIZER { {0,}, {0,} }
// create manager 
int OCPThingManager_create(OCPManager* handle, OCPManager_callback* callback, OCPManager_properties* properties);
// destroy manager 
void OCPThingManager_destroy(OCPManager* handle);
// set authtoken
int OCPThingManager_set_authToken(OCPManager handle, char* authToken);
int OCPThingManager_set_callback(OCPManager handle, OCPManager_callback* callback);
char* OCPThingManager_get_message_header_type(OCPManager handle);
char* OCPThingManager_get_thing_name(OCPManager handle);
char* OCPThingManager_get_auth_code(OCPManager handle);

// check connect status 
int OCPThingManager_isconnected(OCPManager handle);
// connect 
int OCPThingManager_connect(OCPManager handle);
// disconnect 
int OCPThingManager_disconnect(OCPManager handle);
// parse preauthorize_result (for wbc)
OCPPreauthorize_result OCPThingManager_parse_preauthorize_result(OCPManager handle, char* authType, char* response);
// parse authorize result
OCPAuthorize_result OCPThingManager_parse_authorize_result(OCPManager handle, char* response);
// generate authroize data for ita 
char* OCPThingManager_generate_authorize_data(OCPManager handle, char* authCode);
// get ocpmessage preauthorize
OCPMessage* OCPThingManager_get_preauthorize_message(OCPManager handle);
// get ocpmessage authorize
OCPMessage* OCPThingManager_get_authorize_message(OCPManager handle, char* encType, char* dataformat, void* data, int datalen);
// get ocpmessage heartbeat
OCPMessage* OCPThingManager_get_heartbeat_message(OCPManager handle);
// get ocpmessage active rootthing
OCPMessage* OCPThingManager_get_activate_rootthing_message(OCPManager handle, char* encType, char* dataformat, void* data, int datalen);
// get ocpmessage register edgething
OCPMessage* OCPThingManager_get_register_edgething_message(OCPManager handle, char* encType, char* dataformat, void* data, int datalen, char* edgeThingName);
// get ocpmessage inactive thing 
OCPMessage* OCPThingManager_get_inactivate_thing_message(OCPManager handle, char* thingName);
// get ocpmessage register data
OCPMessage* OCPThingManager_get_register_data_message(OCPManager handle, char* encType, char* dataformat, void* data, int datalen, char* thingName, char* msgCode);
// get ocpmessage bulk insert
OCPMessage* OCPThingManager_get_bulk_insert_message(OCPManager handle, char* encType, char* dataformat, void* data, int datalen);
// get ocpmessage request active firmware
OCPMessage* OCPThingManager_get_request_activefirmware_message(OCPManager handle, char* encType, char* dataformat, void* data, int datalen, char* thingName);
// get ocpmessage response passive firmware
OCPMessage* OCPThingManager_get_response_passivefirmware_message(OCPManager handle, char* encType, char* dataformat, void* data, int datalen, char* msgId, char* thingName);
// get ocpmessage sigend firmware
OCPMessage* OCPThingManager_get_signedfirmware_message(OCPManager handle, char* encType, char* dataformat, void* data, int datalen);
// get ocpmessage firmware upgrade result
OCPMessage* OCPThingManager_get_firmware_upgraderesult_message(OCPManager handle, char* encType, char* dataformat, void* data, int datalen, char* thingName, char* resCode);
// get ocpmessage response noti request
OCPMessage* OCPThingManager_get_response_notirequest_message(OCPManager handle, char* encType, char* dataformat, void* data, int datalen, char* msgId);
// get ocpmessage answer message
OCPMessage* OCPThingManager_get_answer_message(OCPManager handle, char* encType, char* dataformat, void* data, int datalen, char* msgId, char* msgCode);

OCPMessage* OCPThingManager_get_fileuploaduri_request_message(OCPManager handle, char* encType, char* dataformat, void* data, int datalen, char* thingName);

OCPMessage* OCPThingManager_get_fileupload_result_message(OCPManager handle, char* encType, char* dataformat, void* data, int datalen, char* thingName);

OCPMessage* OCPThingManager_get_wbc_mc(OCPManager handle, char* encType, char* dataformat, void* data, int datalen, char* thingName);
// send message 
char* OCPThingManager_sendMessage(OCPManager handle, OCPMessage *message);

#endif