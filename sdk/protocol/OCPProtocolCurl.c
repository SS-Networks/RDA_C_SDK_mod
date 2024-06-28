/*========================================================
 * 시스템  명
 * 프로그램ID
 * 프로그램명
 * 개　　　요
 * 변 경 일자
=========================================================*/

#include "OCPProtocolCurl.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <curl/curl.h>	

//
#define CURL_HTTP_URI 	"http://%s:%s"
//
#define CURL_HTTPS_URI 	"https://%s:%s"
//
#define STRING_INITIALIZER { NULL, 0}

typedef struct{
	char* ptr;
	size_t len;
} String;

static void init_string(String *s);
static size_t write_func(void *ptr, size_t size, size_t nmemb, String *s);

typedef struct
{
	char sslOpts;
	char uri[OCPMANAGER_PROPERTY_LEN];
	char trustStore[OCPMANAGER_PROPERTY_LEN];
	void* context;
} IOCPCurl;

/**
 * 
 * 
 * @param 
 * @return 
 */
int OCPProtocolCurl_create(void** handle, void* properties, void* context)
{
	if(*handle != NULL || properties == NULL)
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPProtocolCurl_create] [handle] is not null or [properties] is null \n");
		return OCPSDK_FAILURE;
	}

	OCPProtocol_properties property = *(OCPProtocol_properties*)properties;
	if(property.host == NULL || property.port == NULL)
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPProtocolCurl_create] [host | port] is null\n");
		return OCPSDK_FAILURE;
	}

	IOCPCurl *protocol = malloc(sizeof(IOCPCurl));
	memset(protocol, '\0', sizeof(IOCPCurl));
	*handle = protocol;

	protocol->sslOpts = property.sslOpts;
 	char* uriformat = NULL;
 	if(protocol->sslOpts == OCPSSLOPTS_TCP)
 	{
 		uriformat = CURL_HTTP_URI;
 	}
 	else if(protocol->sslOpts <= OCPSSLOPTS_TWOWAY)
 	{
 		uriformat = CURL_HTTPS_URI;
 	}
 	else
 	{
 		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPProtocolCurl_create] unsupported sslOpts : [%c]\n", protocol->sslOpts);
 		OCPProtocolCurl_destroy(handle);
		return OCPSDK_FAILURE;
 	}

	memset(protocol->uri, 0, sizeof(protocol->uri));
	sprintf(protocol->uri, uriformat, property.host, property.port);

	memset(protocol->trustStore, 0, sizeof(protocol->trustStore));
	strncpy(protocol->trustStore, property.trustStore, strlen(property.trustStore));

	protocol->context = context;

 	return OCPSDK_SUCCESS;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
int OCPProtocolCurl_destroy(void** handle)
{
	IOCPCurl *protocol = *handle;
	if(protocol == NULL)
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPProtocolCurl_destroy] [protocol] is null > already destroy \n");
		return OCPSDK_SUCCESS;
	}

	if(protocol != NULL)
	{
		free(protocol);
		protocol = NULL;
	}

	*handle = NULL;

	return OCPSDK_SUCCESS;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
char* OCPProtocolCurl_send(void* handle, char* service, char* headers, char* payload)
{
	IOCPCurl *protocol = handle;
	if(protocol == NULL){
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPProtocolCurl_send] [protocol] is null \n");
		return NULL;
	}

	CURL *curl;
	CURLcode res;

	struct curl_slist _chunk;
	struct curl_slist *chunk = (&_chunk);	
	//struct curl_slist *chunk = NULL;

	curl_global_init(CURL_GLOBAL_ALL);
	
	curl = curl_easy_init();
	String s = STRING_INITIALIZER;
	if(curl != NULL) {		
		
		chunk = curl_slist_append((struct curl_slist *)NULL, headers);
		chunk = curl_slist_append(chunk, "content-type:application/json");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

		if(protocol->sslOpts > OCPSSLOPTS_TCP)
		{
			FILE* fp = NULL;
			if((fp =fopen(protocol->trustStore, "r")) == 0)
			{
				OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPProtocolCurl_send] file [%s] open error, support only public certification\n", protocol->trustStore);
				if(fp != NULL) { fclose(fp); }
			}
			else
			{
				OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPProtocolCurl_send] file [%s] open, support selfsigned certification\n", protocol->trustStore);
				
				curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
				curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

				curl_easy_setopt(curl, CURLOPT_CAINFO, protocol->trustStore);


				/*char buf[256] = {0, };
				char certm[2048] = {0, };
				char* p = certm;
				while( feof(fp) == 0 )
			    {
			    	memset(buf, 0, 256 * sizeof(char));
			        fgets(buf, 256, fp);
			        strcat(p, buf);
			       	p = p + strlen(buf);
			    }

			    curl_easy_setopt(curl, CURLOPT_SSL_CTX_DATA, certm);*/
				
				if(fp != NULL) { fclose(fp); }
			}
		}

		char *uri = malloc(strlen(protocol->uri) + strlen(service) + 1);
		memset(uri, 0, sizeof(char) * (strlen(protocol->uri) + strlen(service)));
		sprintf(uri, "%s%s", protocol->uri, service);		
		curl_easy_setopt(curl, CURLOPT_URL, uri);	
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPProtocolCurl_send] curl_easy_setopt uri : [%s]\n", uri);	
		free(uri);

		if(payload != NULL)
		{
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload);
		}

		init_string(&s);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_func);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
		
		res = curl_easy_perform(curl);

		if(res != CURLE_OK)
		{
			OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPProtocolCurl_send] curl_easy_perform() FAIL res [%s] \n", curl_easy_strerror(res));
			if(s.ptr)
		 	{
		 		free(s.ptr);
		 		s.ptr = NULL;
		 	}
			curl_easy_cleanup(curl);
			curl_slist_free_all(chunk);
			curl_global_cleanup();
			return NULL;
		}

		curl_easy_cleanup(curl);
		curl_slist_free_all(chunk);
	}
 	curl_global_cleanup();

 	return s.ptr;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
static void init_string(String *s) 
{
	s->len = 0;
	s->ptr = malloc(s->len + 1);
	if (s->ptr == NULL) 
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[init_string] malloc() FAIL \n");
		return;		
	}
	s->ptr[0] = '\0';
}

/**
 * 
 * 
 * @param 
 * @return 
 */
static size_t write_func(void *ptr, size_t size, size_t nmemb, String *s)
{
	size_t new_len = s->len + size*nmemb;
	s->ptr = realloc(s->ptr, new_len+1);
	if (s->ptr == NULL) {
		OCPUilts_log(OCPPROTOCOL_TRACE, "[write_func] realloc() FAIL\n");
		return -1;
	}
	memcpy(s->ptr+s->len, ptr, size*nmemb);
	s->ptr[new_len] = '\0';
	s->len = new_len;

	return size*nmemb;
}