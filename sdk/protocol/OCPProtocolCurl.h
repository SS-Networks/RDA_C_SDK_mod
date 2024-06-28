/**
 * 
 * 
 * @author mw50.jeong
 * @since 2018. 2. 1.
 */

#ifndef _OCP_Protocol_Curl_
#define _OCP_Protocol_Curl_

#include "OCPProtocol.h"

// create protocol
int OCPProtocolCurl_create(void** handle, void* properties, void* context);
// destroy protocol
int OCPProtocolCurl_destroy(void** handle);
// send message
char* OCPProtocolCurl_send(void* handle, char* service, char* headers, char* payload);

#endif
