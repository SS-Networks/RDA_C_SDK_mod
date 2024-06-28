/**
 * 
 * 
 * @author mw50.jeong
 * @since 2018. 2. 1.
 */

#ifndef _OCPMessageConverter_Json_
#define _OCPMessageConverter_Json_

#include "OCPMessageConverter.h"

// json_t typedef 
typedef void ocp_json_t;
// convert message to payload 
void OCPMessageConverterJson_messageToPayload(OCPMessage* message, char** payload, int* payloadlen);
// convert message from payload 
OCPMessage* OCPMessageConverterJson_messageFromPayload(char* payload);
// convert json object to char pointer 
void OCPMessageConverterJson_toCharPointer(void* object, char** data);
// convert json object from char pointer 
ocp_json_t* OCPMessageConverterJson_fromCharPointer(const char* jsonString);
// set new json object 
ocp_json_t* OCPMessageConverterJson_object();
// delet json object 
void OCPMessageConverterJson_delete(ocp_json_t* object);
// set string to json object
void OCPMessageConverterJson_setStringValue(ocp_json_t* object, const char *key, char* value);
// get string from json object
char* OCPMessageConverterJson_getStringValue(ocp_json_t* object, const char* key);
// get int from json object
int OCPMessageConverterJson_getIntValue(ocp_json_t* object, const char* key);

#endif



