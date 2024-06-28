/*========================================================
 * 시스템  명
 * 프로그램ID
 * 프로그램명
 * 개　　　요
 * 변 경 일자
=========================================================*/

#include "OCPMessageConverterDelimeter.h"
#include "OCPMessageCompressor.h"
#include "OCPUtils.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

/**
 * 
 */
#define OCPMESSAGECONVERTERDELIMETER_HEADER_COUNT	17

static int payloadCharAppend(int payloadlen, char** payload, int valuelen, char* value, char delemeter);
static int payloadSizeTAppend(char** payload, unsigned long long num, char delemeter);
static char *strsep_custom(char **stringp, const char *delim);
static unsigned long long atoi_custom(char *str);

/**
 * 
 * 
 * @param 
 * @return 
 */
char* OCPMessageConverterDelimeter_messageToPayload(OCPMessage* message, int *payloadlen, int minCompressionSize)
{
	OCPUilts_log(OCPMESSAGECONVERTER_TRACE, "@@ OCPMessageConverterDelimeter_messageToPayload START @@\n");

	char* payload = NULL;
	int res = 0;

	// 0 : need compress 	
	int compress = OCPMessageCompressor_needCompress(message->msgCode, message->datalen, minCompressionSize);
	if(!compress)
	{
// fprintf(stderr, "------------ needCompress message->msgCode : %s, message->datalen: %lld, minCompressionSize : %d\n", message->msgCode, message->datalen, minCompressionSize);
OCPUilts_log(OCPMESSAGECONVERTER_TRACE, "@@ before message->encType : %s\n", message->encType);
		OCPMessageCompressor_convert_encType(message->encType, strlen(message->encType), 1);
OCPUilts_log(OCPMESSAGECONVERTER_TRACE, "@@ after  message->encType : %s\n", message->encType);		
		OCPMessageCompressor_compress(&(message->data), (int *)&(message->datalen));
	}

// fprintf(stderr, "------------ %s\n", "1");	
	res = payloadCharAppend(res, &payload, message->version == NULL ? 0 : strlen(message->version), message->version, '|');
// fprintf(stderr, "--- %s\n", "2");
	res = payloadCharAppend(res, &payload, message->msgType == NULL ? 0 : strlen(message->msgType), message->msgType, '|');
// fprintf(stderr, "--- %s\n", "3");
	res = payloadCharAppend(res, &payload, message->funcType == NULL ? 0 : strlen(message->funcType), message->funcType, '|');
// fprintf(stderr, "--- %s\n", "4");
	res = payloadCharAppend(res, &payload, message->siteId == NULL ? 0 : strlen(message->siteId), message->siteId, '|');
// fprintf(stderr, "--- %s\n", "5");
	res = payloadCharAppend(res, &payload, message->thingName == NULL ? 0 : strlen(message->thingName), message->thingName, '|');
// fprintf(stderr, "--- %s\n", "6");
	res = payloadCharAppend(res, &payload, message->tId == NULL ? 0 : strlen(message->tId), message->tId, '|');
// fprintf(stderr, "--- %s\n", "7");
	res = payloadCharAppend(res, &payload, message->msgCode == NULL ? 0 : strlen(message->msgCode), message->msgCode, '|');
// fprintf(stderr, "--- %s\n", "8");
	res = payloadCharAppend(res, &payload, message->msgId == NULL ? 0 : strlen(message->msgId), message->msgId, '|');
// fprintf(stderr, "--- %s\n", "9");
	res = payloadSizeTAppend(&payload, message->msgDate, '|');
// fprintf(stderr, "--- %s\n", "10");
	res = payloadCharAppend(res, &payload, message->resCode == NULL ? 0 : strlen(message->resCode), message->resCode, '|');
// fprintf(stderr, "--- %s\n", "11");
	res = payloadCharAppend(res, &payload, message->resMsg == NULL ? 0 : strlen(message->resMsg), message->resMsg, '|');
// fprintf(stderr, "--- %s\n", "12");
	res = payloadCharAppend(res, &payload, message->dataFormat == NULL ? 0 : strlen(message->dataFormat), message->dataFormat, '|');
// fprintf(stderr, "--- %s\n", "13");	
	res = payloadCharAppend(res, &payload, message->severity == NULL ? 0 : strlen(message->severity), message->severity, '|');
// fprintf(stderr, "--- %s\n", "14");
	// compress mode 면 바꿔줘야 해 
	// 
// fprintf(stderr, "------------ payload : %s\n", payload);
	res = payloadCharAppend(res, &payload, message->encType == NULL ? 0 : strlen(message->encType), message->encType, '|');
// fprintf(stderr, "--- %s\n", "15");
	res = payloadCharAppend(res, &payload, message->authToken == NULL ? 0 : strlen(message->authToken), message->authToken, '|');
// fprintf(stderr, "--- %s\n", "16");
	res = payloadSizeTAppend(&payload, message->datalen, '|');
// fprintf(stderr, "--- %s\n", "17");
// fprintf(stderr, "------------ payload : %s\n", payload);	
	char* tmp = (char*)message->data;
	res = payloadCharAppend(res, &payload, message->data == NULL ? 0 : message->datalen, tmp, 0);	
// fprintf(stderr, "------------ payload : %s\n", payload);	

// int i = 0;
// fprintf(stderr, "@@@@@@@ message->datalen : %llu\n", message->datalen);	
// fprintf(stderr, "@@@@@@@ message->data : \n");
// for(i = 0; i < message->datalen; i++)
// {
// 	fprintf(stderr, "%02x", ((char*)message->data)[i]);
// 	// fprintf(stderr, "%c", ((char*)message->data)[i]);
// }
// fprintf(stderr, "\n");

// fprintf(stderr, "--- %s\n", "18");
	*payloadlen = res - 1; //delimeter size 빼기 위해서 처리 
	// OCPUilts_log(OCPMESSAGECONVERTER_TRACE, "@@ OCPMessageConverterDelimeter_messageToPayload END \npayload : %s @@\n", payload);

// fprintf(stderr, "@@@@@@@ *payloadlen : %d\n", *payloadlen);	
// fprintf(stderr, "@@@@@@@ payload : \n");
// for(i = 0; i < *payloadlen; i++)
// {
// 	fprintf(stderr, "%02x", payload[i]);
// 	// fprintf(stderr, "%c", ((char*)message->data)[i]);
// }
// fprintf(stderr, "\n");	
	OCPUilts_log(OCPMESSAGECONVERTER_TRACE, "@@ message->encType : %s\n", message->encType);
	//OCPUilts_log(OCPMESSAGECONVERTER_TRACE, "@@ payload : %s\n", payload);

	return payload;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
OCPMessage* OCPMessageConverterDelimeter_messageFromPayload(char* payload, int minCompressionSize)
{
	OCPUilts_log(OCPMESSAGECONVERTER_TRACE, "@@ OCPMessageConverterDelimeter_messageFromPayload START \npayload : [%s] @@\n", payload);

	OCPMessage message = OCPMESSAGE_INITIALIZER;

	if(payload == NULL || strlen(payload) < 1)
	{
		OCPUilts_log(OCPMESSAGECONVERTER_TRACE, "@@ OCPMessageConverterDelimeter_messageFromPayload FAIL, because [payload] is null | payloadlen is [0] @@\n");
		return NULL;
	}

	// 0 : need compression 	
	int decompress = !0;

	char *temp;
	char *ptr = NULL;
	temp = payload;
	char* ptrArr[OCPMESSAGECONVERTERDELIMETER_HEADER_COUNT];
	int count = 0;

	// ptr = strsep_custom(&temp, "|");
	// while(ptr != NULL && count < OCPMESSAGECONVERTERDELIMETER_HEADER_COUNT) 
	while(count < OCPMESSAGECONVERTERDELIMETER_HEADER_COUNT) 
	{
		if(count ==  16)
		{
			ptrArr[count++] = temp;
			break;
		}
		ptr = strsep_custom(&temp, "|");
		ptrArr[count++] = ptr;
	}
	// 0 ~ 16 까지 돌면 돼 

	int i = 0;
	char* value;
	int len;
	for(i = 0; i < count; i++)
	{
		value = ptrArr[i];
		len = 0;
		if(i == 0)
		{
			if(value != NULL) 
			{
				len = strlen(value);
				strncpy(message.version, value, len);
			}
			else
			{
				message.version[0] = 0;
			}
		}
		else if(i == 1)
		{
			if(value != NULL)	
			{
				len = strlen(value);
				strncpy(message.msgType, value, len);
			}
			else 
			{
				message.msgType[0] = 0;
			}
		}
		else if(i == 2)
		{
			if(value != NULL)	
			{
				len = strlen(value);
				strncpy(message.funcType, value, len);
			}
			else 
			{
				message.funcType[0] = 0;
			}
		}
		else if(i == 3)
		{
			if(value != NULL)	
			{
				len = strlen(value);
				strncpy(message.siteId, value, len);
			}
			else 
			{
				message.siteId[0] = 0;
			}
		}
		else if(i == 4)
		{
			if(value != NULL)	
			{
				len = strlen(value);
				strncpy(message.thingName, value, len);
			}
			else 
			{
				message.thingName[0] = 0;
			}
		}
		else if(i == 5)
		{
			if(value != NULL)	
			{
				len = strlen(value);
				strncpy(message.tId, value, len);
			}
			else 
			{
				message.tId[0] = 0;
			}
		}
		else if(i == 6)
		{
			if(value != NULL)	
			{
				len = strlen(value);
				strncpy(message.msgCode, value, len);
			}
			else 
			{
				message.msgCode[0] = 0;
			}
		}
		else if(i == 7)
		{
			if(value != NULL)	
			{
				len = strlen(value);
				strncpy(message.msgId, value, len);
			}
			else 
			{
				message.msgId[0] = 0;
			}
		}
		else if(i == 8)
		{
			if(value != NULL)
			{
				message.msgDate = atoi_custom(value);
			}
			else
			{
				message.msgDate = 0;
			}
		}
		else if(i == 9)
		{
			if(value != NULL)	
			{
				len = strlen(value);
				strncpy(message.resCode, value, len);
			}
			else 
			{
				message.resCode[0] = 0;
			}
		}
		else if(i == 10)
		{
			if(value != NULL)	
			{
				len = strlen(value);
				strncpy(message.resMsg, value, len);
			}
			else 
			{
				message.resMsg[0] = 0;
			}
		}
		else if(i == 11)
		{
			if(value != NULL)	
			{
				len = strlen(value);
				strncpy(message.dataFormat, value, len);
			}
			else 
			{
				message.dataFormat[0] = 0;
			}
		}
		else if(i == 12)
		{
			if(value != NULL)	
			{
				len = strlen(value);
				strncpy(message.severity, value, len);
			}
			else 
			{
				message.severity[0] = 0;
			}
		}
		else if(i == 13)
		{
			if(value != NULL)	
			{
				len = strlen(value);
				strncpy(message.encType, value, len);
			}
			else 
			{
				message.encType[0] = 0;
			}
			decompress = OCPMessageCompressor_needDecompress(message.encType);
			if(!decompress)
			{
				OCPMessageCompressor_convert_encType(message.encType, strlen(message.encType), 0);
			}

		}
		else if(i == 14)
		{
			if(value != NULL)	
			{
				len = strlen(value);
				strncpy(message.authToken, value, len);
			}
			else 
			{
				message.authToken[0] = 0;
			}
		}
		else if(i == 15)
		{
			
			if(value != NULL)
			{
				message.datalen = atoi_custom(value);
			}
			else
			{
				message.datalen = 0;
			}
		}
		else if(i == 16)
		{
			if(value != NULL && message.datalen > 0)
			{
				len = message.datalen;
				message.data = malloc(len + 1);
				memset(message.data, 0, sizeof(char) * len + 1);
				memcpy(message.data, value, len);
			}
			else 
			{
				message.data = NULL;
			}
			if(!decompress)
			{
				OCPMessageCompressor_decompress(&(message.data), (int*)&(message.datalen));
			}
		}
		else
		{
			OCPUilts_log(OCPMESSAGECONVERTER_TRACE, "@@ OCPMessageConverterDelimeter_messageFromPayload FAIL, because parse error index [%d] @@\n", i);
			return NULL;
		}
	}

	OCPUilts_log(OCPMESSAGECONVERTER_TRACE, "@@ OCPMessageConverterDelimeter_messageFromPayload END @@\n");
	// return message;
	OCPMessage* messageptr = malloc(sizeof(OCPMessage));
	memset(messageptr, 0, sizeof(OCPMessage));
	memcpy(messageptr, &message, sizeof(OCPMessage));

// fprintf(stderr, "[OCPMessageConverterDelimeter_messageFromPayload] &message : %p\n", &message);	
// fprintf(stderr, "[OCPMessageConverterDelimeter_messageFromPayload] messageptr : %p\n", messageptr);		
	return messageptr;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
static int payloadCharAppend(int payloadlen, char** payload, int valuelen, char* value, char delemeter)
{
	if(*payload == NULL)
	{
		*payload = malloc(1);
		*payload[0] = 0x00;
	}

	// int payloadlen;
	// int valuelen;
	int newlen;

	if(value == NULL)
	{
		value = "";
	}

	// payloadlen = strlen(*payload);
	// valuelen = strlen(value) * sizeof(char);
	newlen = payloadlen + valuelen;
	
	char* temp = *payload;
	*payload = realloc(*payload, newlen + 2);
	if(*payload == NULL)
	{
		free(temp);
		OCPUilts_log(OCPMESSAGECONVERTER_TRACE, "@@ payloadCharAppend FAIL, because realloc payload is NULL @@\n");
		return 0;
	}

	memcpy(*payload + payloadlen, value, valuelen);
	(*payload)[newlen] = delemeter;
	(*payload)[newlen + 1] = 0;

	return newlen + 1;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
static int payloadSizeTAppend(char** payload, unsigned long long num, char delemeter)
{
	if(*payload == NULL)
	{
		*payload = malloc(1);
		*payload[0] = 0x00;
	}

	int payloadlen, valuelen, newlen;

	char value[32];
	value[0] = '\0';
	int res = sprintf(value, "%llu", num);
	if(res <= 0){
		value[0] = '\0';
	}

	payloadlen = strlen(*payload);
	valuelen = strlen(value) * sizeof(char);
	newlen = payloadlen + valuelen;

	char* temp = *payload;
	*payload = realloc(*payload, newlen + 2);
	if(*payload == NULL)
	{
		free(temp);
		OCPUilts_log(OCPMESSAGECONVERTER_TRACE, "@@ payloadSizeTAppend FAIL, because realloc payload is NULL @@\n");
		return 0;
	}

	memcpy(*payload + payloadlen, value, valuelen);
	(*payload)[newlen] = delemeter;
	(*payload)[newlen + 1] = '\0';

	return strlen(*payload);
}

/**
 * 
 * 
 * @param 
 * @return 
 */
static char *strsep_custom(char **stringp, const char *delim)
{
	char *ptr = *stringp;

	if(ptr == NULL) 
	{
		OCPUilts_log(OCPMESSAGECONVERTER_TRACE, "@@ strsep_custom FAIL, because ptr is NULL @@\n");
		return NULL;
	}

	while(**stringp) 
	{
		if(strchr(delim, **stringp) != NULL) {
			**stringp = 0x00;
			(*stringp)++;
			return ptr;
		}
		(*stringp)++;
	}
	*stringp = NULL;
	return ptr;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
static unsigned long long atoi_custom(char *str){     
    unsigned long long tot=0; 
    char* ptr = str;
    while(*ptr)
    { 
        tot = tot*10 + *ptr - '0';  
        ptr++;   
    } 
    return tot; 
} 

