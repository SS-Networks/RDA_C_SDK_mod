/**
 * 
 * 
 * @author mw50.jeong
 * @since 2018. 2. 1.
 */

#ifndef _OCP_Protocol_Mqtt_
#define _OCP_Protocol_Mqtt_

#include "OCPProtocol.h"

// create protocol
int OCPProtocolMqtt_create(void** handle, void* properties, void* context, OCPProtocol_callback* callback);
// destroy protocol
int OCPProtocolMqtt_destroy(void** handle);
// connect
int OCPProtocolMqtt_connect(void* handle);
// subscribe
int OCPProtocolMqtt_subscribe(void* handle);
// disconnect
int OCPProtocolMqtt_disconnect(void* handle);
// check connect status
int OCPProtocolMqtt_isconnected(void* handle);
// send message
int OCPProtocolMqtt_send(void* handle, char* payload, int payloadlen);

#endif