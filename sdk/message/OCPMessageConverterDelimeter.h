/**
 * 
 * 
 * @author mw50.jeong
 * @since 2018. 2. 1.
 */

#ifndef _OCPMessageConverter_Delimeter_
#define _OCPMessageConverter_Delimeter_

#include "OCPMessageConverter.h"

// convert message to payload 
char* OCPMessageConverterDelimeter_messageToPayload(OCPMessage* message, int *payloadlen, int minCompressionSize);
// convert message from payload 
OCPMessage* OCPMessageConverterDelimeter_messageFromPayload(char* payload, int minCompressionSize);

#endif