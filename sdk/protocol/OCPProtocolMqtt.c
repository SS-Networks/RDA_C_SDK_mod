/*========================================================
 * 시스템  명
 * 프로그램ID
 * 프로그램명
 * 개　　　요
 * 변 경 일자
=========================================================*/

#include "OCPProtocolMqtt.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <MQTTClient.h>
#include <MQTTAsync.h>

//
#define MQTT_TCP_URI 	"tcp://%s:%s";
//
#define MQTT_SSL_URI 	"ssl://%s:%s";

typedef struct
{
	char sslOpts;
	char uri[OCPMANAGER_PROPERTY_LEN];
	char siteId[OCPMANAGER_PROPERTY_LEN];
	char thingName[OCPMANAGER_PROPERTY_LEN];	
	char publishTopic[OCPMANAGER_PROPERTY_LEN];
	char subscribeTopic[OCPMANAGER_PROPERTY_LEN];
	char username[OCPMANAGER_PROPERTY_LEN];
	char password[OCPMANAGER_PROPERTY_LEN];
	int messageQos;
	int keepAliveInterval;
	int timeout;
	char trustStore[OCPMANAGER_PROPERTY_LEN];
	char keyStore[OCPMANAGER_PROPERTY_LEN];
	char keyPassword[OCPMANAGER_PROPERTY_LEN];
	void* context; 
	OCPProtocol_onConnect* cn;
	OCPProtocol_onDisconnect* dc;
	OCPProtocol_onMessageArrive* ma;
	OCPProtocol_onConnectionClosed* cl;
	MQTTAsync mqttAsync;
	// MQTTAsync_connectOptions *connOpts;
	MQTTAsync_connectOptions connOpts;
	MQTTAsync_SSLOptions sslOptions;
}IOCPMqtt;

static void MQTT_connectionLost(void* context, char* cause);
static int MQTT_messageArrived(void* context, char* topicName, int topicLen, MQTTAsync_message* message);
static void MQTT_deliveryComplete(void* context, MQTTAsync_token dt);
static void onConnectSuccess(void* context, MQTTAsync_successData* response);
static void onConnectFailure(void* context, MQTTAsync_failureData* response);
static void onDisconnectSuccess(void* context, MQTTAsync_successData* response);
static void onDisconnectFailure(void* context, MQTTAsync_failureData* response);
static void onSubscribeSuccess(void* context, MQTTAsync_successData* response);
static void onSubscribeFailure(void* context, MQTTAsync_failureData* response);
static void onPublishSuccess(void* context, MQTTAsync_successData* response);
static void onPublishFailure(void* context, MQTTAsync_failureData* response);

/**
 * 
 * 
 * @param 
 * @return 
 */
int OCPProtocolMqtt_create(void** handle, void* properties, void* context, OCPProtocol_callback* callback)
{
	if(*handle != NULL || properties == NULL)
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPProtocolMqtt_create] handle is not null or properties is null\n");
		return OCPSDK_FAILURE;
	}
	
	OCPProtocol_properties property = *(OCPProtocol_properties*)properties;
	if(property.host == NULL || property.port == NULL) 
	{ 
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPProtocolMqtt_create] host | port is null \n");
		return OCPSDK_FAILURE;
	}

	IOCPMqtt *protocol = NULL;
	protocol = malloc(sizeof(IOCPMqtt));
	memset(protocol, '\0', sizeof(IOCPMqtt));
	*handle = protocol;

	protocol->sslOpts = property.sslOpts;

 	char* uriformat = NULL;
 	if(protocol->sslOpts == OCPSSLOPTS_TCP)
 	{
 		uriformat = MQTT_TCP_URI;
 	}
 	else if(protocol->sslOpts <= OCPSSLOPTS_TWOWAY)
 	{
 		uriformat = MQTT_SSL_URI;
 	}
 	else
 	{
 		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPProtocolMqtt_create] unsupported sslOpts : [%c]\n", protocol->sslOpts);
 		OCPProtocolMqtt_destroy(handle);
		return OCPSDK_FAILURE;
 	} 
	memset(protocol->uri, 0, sizeof(char) * strlen(uriformat) + sizeof(char) * strlen(property.host) + sizeof(char) * strlen(property.port) + 1);
	sprintf(protocol->uri, uriformat, property.host, property.port);
	memset(protocol->siteId, 0x00, sizeof(char) * strlen(property.siteId) + 1);
	strncpy(protocol->siteId, property.siteId, strlen(property.siteId));
	memset(protocol->thingName, 0x00, sizeof(char) * strlen(property.thingName) + 1);
	strncpy(protocol->thingName, property.thingName, strlen(property.thingName));
	memset(protocol->publishTopic, 0x00, sizeof(char) * strlen(property.publishTopic) + 1);
	strncpy(protocol->publishTopic, property.publishTopic, strlen(property.publishTopic));
	memset(protocol->subscribeTopic, 0x00, sizeof(char) * strlen(property.subscribeTopic) + 1);
	strncpy(protocol->subscribeTopic, property.subscribeTopic, strlen(property.subscribeTopic));
	protocol->messageQos = 0;
	protocol->keepAliveInterval = property.keepAliveInterval;
	protocol->timeout = property.timeout;
	memset(protocol->trustStore, 0x00, sizeof(char) * strlen(property.trustStore) + 1);
	strncpy(protocol->trustStore, property.trustStore, strlen(property.trustStore));
	memset(protocol->keyStore, 0x00, sizeof(char) * strlen(property.keyStore) + 1);
	strncpy(protocol->keyStore, property.keyStore, strlen(property.keyStore));
	memset(protocol->keyPassword, 0x00, sizeof(char) * strlen(property.keyPassword) + 1);
	strncpy(protocol->keyPassword, property.keyPassword, strlen(property.keyPassword));

	protocol->context = context;

	protocol->cn = callback->cn;
	protocol->dc = callback->dc;
	protocol->ma = callback->ma;
	protocol->cl = callback->cl;

	// char clientId[OCPMANAGER_PROPERTY_LEN];
	memset(protocol->username, 0, sizeof(protocol->username));
	sprintf(protocol->username, "%s+%s", protocol->siteId, protocol->thingName);

	memset(protocol->password, 0, sizeof(protocol->password));
	memcpy(protocol->password, property.password, strlen(property.password));

	int rc;
	if ((rc = MQTTAsync_create(&(protocol->mqttAsync), protocol->uri, protocol->username, MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTASYNC_SUCCESS)
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPProtocolMqtt_create] MQTTAsync_create rc : [%d]\n", rc);
		OCPProtocolMqtt_destroy(handle);
		return OCPSDK_FAILURE;
	}

	rc = MQTTAsync_setCallbacks(protocol->mqttAsync, protocol, MQTT_connectionLost, MQTT_messageArrived, MQTT_deliveryComplete);
	if (MQTTASYNC_SUCCESS != rc)
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPProtocolMqtt_create] MQTTAsync_setCallbacks rc : [%d]\n", rc);
		OCPProtocolMqtt_destroy(handle);
		return OCPSDK_FAILURE;
	}

	FILE* fp = NULL;
	MQTTAsync_SSLOptions	sslOpts = MQTTAsync_SSLOptions_initializer;
// fprintf(stderr, "--------------------- x\n");			
	if (protocol->sslOpts == OCPSSLOPTS_TCP)
	{
// fprintf(stderr, "--------------------- 0\n");		
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPProtocolMqtt_connect] protocol->sslOpts : [%c]\n", protocol->sslOpts);
	}
	else if (protocol->sslOpts == OCPSSLOPTS_ONEWAY)
	{
// fprintf(stderr, "--------------------- 1\n");				
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPProtocolMqtt_connect] protocol->sslOpts : [%c]\n", protocol->sslOpts);
		sslOpts.sslVersion = MQTT_SSL_VERSION_TLS_1_2;		
		if((fp =fopen(protocol->trustStore, "r")) == 0)
		{
			OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPProtocolMqtt_connect] trustStore [%s] open error, support only public certification\n", protocol->trustStore);
			sslOpts.enableServerCertAuth = 1;
			if(fp != NULL) { fclose(fp); }
		}
		else
		{
			OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPProtocolMqtt_connect] trustStore [%s] open, support selfsigned certification\n", protocol->trustStore);
			sslOpts.trustStore = protocol->trustStore;
			sslOpts.enableServerCertAuth = 0;
			if(fp != NULL) { fclose(fp); }
		}
	}
	else if (protocol->sslOpts == OCPSSLOPTS_TWOWAY)
	{
// fprintf(stderr, "--------------------- 2\n");				
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPProtocolMqtt_connect] protocol->sslOpts : [%c]\n", protocol->sslOpts);
		if((fp =fopen(protocol->trustStore, "r")) == 0)
		{
			OCPUilts_log(OCPPROTOCOL_TRACE, "[[OCPProtocolMqtt_connect] trustStore [%s] open error, support only public certification\n", protocol->trustStore);
			sslOpts.enableServerCertAuth = 1;
			if(fp != NULL) { fclose(fp); }
		}
		else
		{
			OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPProtocolMqtt_connect] trustStore [%s] open, support selfsigned certification\n", protocol->trustStore);
			sslOpts.trustStore = protocol->trustStore;	
			sslOpts.enableServerCertAuth = 1;
			if(fp != NULL) { fclose(fp); }
		}

		if((fp =fopen(protocol->keyStore, "r")) == 0)
		{
			OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPProtocolMqtt_connect] keyStore [%s] open error, support only public certification\n", protocol->keyStore);
			sslOpts.enableServerCertAuth = 1;
			if(fp != NULL) { fclose(fp); }
		}
		else
		{
			OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPProtocolMqtt_connect] keyStore [%s] open, support selfsigned certification\n", protocol->keyStore);
			sslOpts.keyStore = protocol->keyStore;
			sslOpts.enableServerCertAuth = 0;
			if(fp != NULL) { fclose(fp); }
		}
		
		sslOpts.privateKeyPassword = protocol->keyPassword;
	}
	else
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPProtocolMqtt_connect] unsupported sslOpts : [%c]\n", protocol->sslOpts);
		return OCPSDK_FAILURE;
	}

	memcpy(&(protocol->sslOptions), &sslOpts, sizeof(MQTTAsync_SSLOptions));

	return OCPSDK_SUCCESS;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
int OCPProtocolMqtt_destroy(void** handle)
{
	IOCPMqtt *protocol = *handle;

	if(protocol == NULL)
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPProtocolMqtt_destroy] [protocol] is null > already destroy \n");
		return OCPSDK_SUCCESS;
	}

	if(protocol->mqttAsync != NULL)
	{
		MQTTAsync_destroy(&protocol->mqttAsync);
		protocol->mqttAsync = NULL;
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
int OCPProtocolMqtt_connect(void* handle)
{
	IOCPMqtt *protocol = handle;
	if(protocol == NULL || protocol->mqttAsync == NULL)
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPProtocolMqtt_connect] [protocol | mqttAsync] is null \n");
		return OCPSDK_FAILURE;
	}

	if(MQTTAsync_isConnected(protocol->mqttAsync)) //connected : 1  //disconnected 0
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPProtocolMqtt_connect] MQTTAsync_isConnected [!0] > already connect\n");
		return OCPSDK_SUCCESS;
	}

	//FILE* fp = NULL;
	MQTTAsync_connectOptions connectOpts = MQTTAsync_connectOptions_initializer;	

	if (protocol->sslOpts == OCPSSLOPTS_TCP)
	{
		connectOpts.ssl = NULL;
	}
	else
	{
		// connectOpts.ssl = &sslOpts;
		connectOpts.ssl = &(protocol->sslOptions);
	}

	connectOpts.keepAliveInterval = protocol->keepAliveInterval;
	connectOpts.connectTimeout = protocol->timeout;
	connectOpts.cleansession = 0;

	connectOpts.automaticReconnect = 1;	// will try to reconnect when disconnected

	if(strlen(protocol->password) > 0)
	{
		connectOpts.username = protocol->username;
		connectOpts.password = protocol->password;
	}

	connectOpts.onSuccess = onConnectSuccess;
	connectOpts.onFailure = onConnectFailure;
	connectOpts.context = protocol;

	int rc;
	if ((rc = MQTTAsync_connect(protocol->mqttAsync, &connectOpts)) != MQTTASYNC_SUCCESS)
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPProtocolMqtt_connect] MQTTAsync_connect rc : [%d]\n", rc);
		return OCPSDK_FAILURE;
	}

	return OCPSDK_SUCCESS;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
int OCPProtocolMqtt_subscribe(void* handle)
{
	IOCPMqtt *protocol = handle;
	if(protocol == NULL || protocol->mqttAsync == NULL)
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPProtocolMqtt_subscribe] [protocol | mqttAsync] is null\n");
		return OCPSDK_FAILURE;
	}

	if(!MQTTAsync_isConnected(protocol->mqttAsync)) //connected : 1 
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPProtocolMqtt_subscribe] MQTTAsync_isConnected [!1]\n");
		return OCPSDK_FAILURE;
	}

	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
	opts.onSuccess = onSubscribeSuccess;
	opts.onFailure = onSubscribeFailure;
	opts.context = protocol;

	//char topic[OCPMANAGER_PROPERTY_LEN];
	char topic[3072] = {0, };
	memset(topic, 0, sizeof(topic));
	sprintf(topic, "%s%s/%s", protocol->subscribeTopic, protocol->siteId, protocol->thingName);

	int rc = OCPSDK_FAILURE;
	if ((rc = MQTTAsync_subscribe(protocol->mqttAsync, topic, protocol->messageQos, &opts)) != MQTTASYNC_SUCCESS)
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPProtocolMqtt_subscribe] MQTTAsync_subscribe rc : [%d]\n", rc);
		return OCPSDK_FAILURE;
	}
	return OCPSDK_SUCCESS;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
int OCPProtocolMqtt_disconnect(void* handle)
{
	IOCPMqtt *protocol = handle;
	if(protocol == NULL || protocol->mqttAsync == NULL)
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPProtocolMqtt_disconnect] [protocol | mqttAsync] is null\n");
		return OCPSDK_FAILURE;
	}

	if(!MQTTAsync_isConnected(protocol->mqttAsync)) //connected : 1  //disconnected
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPProtocolMqtt_disconnect] MQTTAsync_isConnected [!1] > already disconnect\n");
		return OCPSDK_SUCCESS;
	}

	int rc;
	MQTTAsync_disconnectOptions opts = MQTTAsync_disconnectOptions_initializer;
	opts.onSuccess = onDisconnectSuccess;
	opts.onFailure = onDisconnectFailure;
	opts.context = protocol;
	if ((rc = MQTTAsync_disconnect(protocol->mqttAsync, &opts)) != MQTTASYNC_SUCCESS)
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPProtocolMqtt_disconnect] MQTTAsync_disconnect rc : [%d]\n", rc);
		return OCPSDK_FAILURE;
	}	
	return OCPSDK_SUCCESS;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
int OCPProtocolMqtt_isconnected(void* handle)
{
	IOCPMqtt *protocol = handle;
	if(protocol == NULL || protocol->mqttAsync == NULL)
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPProtocolMqtt_isconnected] [protocol | mqttAsync] is null\n");
		return OCPSDK_FAILURE;
	}

	return MQTTAsync_isConnected(protocol->mqttAsync);
}

/**
 * 
 * 
 * @param 
 * @return 
 */
int OCPProtocolMqtt_send(void* handle, char* payload, int payloadlen)
{
	IOCPMqtt *protocol = handle;
	if(protocol == NULL  || protocol->mqttAsync == NULL || payload == NULL)
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPProtocolMqtt_send] [protocol | mqttAsync] is null\n");
		return OCPSDK_FAILURE;
	}

	if(!MQTTAsync_isConnected(protocol->mqttAsync))
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPProtocolMqtt_send] MQTTAsync_isConnected [!1]\n");
		return OCPSDK_FAILURE;
	}

	int rc = MQTTASYNC_SUCCESS;
	MQTTAsync_message  pubmsg = MQTTAsync_message_initializer;
	MQTTAsync_responseOptions ropts = MQTTAsync_responseOptions_initializer;

	ropts.onSuccess = onPublishSuccess;
	ropts.onFailure = onPublishFailure;
	ropts.context = protocol;

	pubmsg.qos = protocol->messageQos;
	pubmsg.retained = 0;
	pubmsg.payload = (void*)payload;
	pubmsg.payloadlen = payloadlen;

	if ((rc = MQTTAsync_sendMessage(protocol->mqttAsync, protocol->publishTopic, &pubmsg, &ropts)) != MQTTASYNC_SUCCESS)
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[OCPProtocolMqtt_send] MQTTAsync_sendMessage rc : [%d]\n", rc);
		return rc;
	}

	return OCPSDK_SUCCESS;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
static void MQTT_connectionLost(void* context, char* cause)
{
	OCPUilts_log(OCPPROTOCOL_TRACE, "[MQTT_connectionLost] [%s+%s] cause [%s]\n", ((IOCPMqtt *) context)->siteId, ((IOCPMqtt *) context)->thingName, cause);
	if(*(((IOCPMqtt *) context)->cl))
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[MQTT_connectionLost] called cl\n");
		(*(((IOCPMqtt *) context)->cl))(((IOCPMqtt *) context)->context, cause);	
	}
}

/**
 * 
 * 
 * @param 
 * @return 
 */
static int MQTT_messageArrived(void* context, char* topicName, int topicLen, MQTTAsync_message* message)
{
	if(topicName == NULL || \
		message == NULL || message->payload == NULL || message->payloadlen <= 0 || message->payloadlen >= 104857600)
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[MQTT_messageArrived] make message FAIL, topic or message is null\n");
		return OCPSDK_FAILURE;
	}

	// fprintf(stderr, "-- MQTT_messageArrived topicName: %s \n", topicName);
	// fprintf(stderr, "-- MQTT_messageArrived message->payloadlen: %d \n", message->payloadlen);
	// fprintf(stderr, "-- MQTT_messageArrived message: %s \n", message->payload);
	// int i = 0;
	// for(i = 0; i < message->payloadlen; i++)
	// {
	// 	fprintf(stderr, "%02x", ((char*)message->payload)[i]);
	// }
	// fprintf(stderr, "\n");

	if(strlen(message->payload) <= 2)
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[MQTT_messageArrived] pong message\n");
		return OCPSDK_FAILURE;
	}

	if(*(((IOCPMqtt *) context)->ma))
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[MQTT_messageArrived] called ma\n");

		char* payload = malloc(message->payloadlen + 1);
		memset(payload, 0, sizeof(char) * message->payloadlen + 1);
		memcpy(payload, message->payload, message->payloadlen);

		(*(((IOCPMqtt *) context)->ma))(((IOCPMqtt *) context)->context, payload);	
	}

	// 콜백이 설정되었느냐 아니냐와 상관없이 message 와 topicName 은 free 하는게 맞음
	MQTTAsync_freeMessage(&message);
	MQTTAsync_free(topicName);
 	
	return OCPSDK_SUCCESS;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
static void MQTT_deliveryComplete(void* context, MQTTAsync_token dt)
{
	OCPUilts_log(OCPPROTOCOL_TRACE, "[MQTT_deliveryComplete]\n");
}

/**
 * 
 * 
 * @param 
 * @return 
 */
static void onConnectSuccess(void* context, MQTTAsync_successData* response)
{
	OCPUilts_log(OCPPROTOCOL_TRACE, "[onConnectSuccess] response [%s]\n", ((IOCPMqtt *) context)->siteId, ((IOCPMqtt *) context)->thingName, response == NULL ? NULL : response->alt.connect.serverURI);
	if(*(((IOCPMqtt *) context)->cn))
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "@@ onConnectSuccess call cn\n");
		(*(((IOCPMqtt *) context)->cn))(((IOCPMqtt *) context)->context);	
	}
}

/**
 * 
 * 
 * @param 
 * @return 
 */
static void onConnectFailure(void* context, MQTTAsync_failureData* response)
{
	OCPUilts_log(OCPPROTOCOL_TRACE, "[onConnectFailure] response [%s]\n", ((IOCPMqtt *) context)->siteId, ((IOCPMqtt *) context)->thingName, response == NULL ? NULL : response->message);

	if(*(((IOCPMqtt *) context)->cn))
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[onConnectFailure] call cn \n");
		(*(((IOCPMqtt *) context)->cn))(((IOCPMqtt *) context)->context);	
	}
}

/**
 * 
 * 
 * @param 
 * @return 
 */
static void onDisconnectSuccess(void* context, MQTTAsync_successData* response)
{
	OCPUilts_log(OCPPROTOCOL_TRACE, "[onDisconnectSuccess] response [%s]\n", ((IOCPMqtt *) context)->siteId, ((IOCPMqtt *) context)->thingName, response == NULL ? NULL : response->alt.connect.serverURI);
	if(*(((IOCPMqtt *) context)->dc))
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "@@ onDisconnectSuccess call cn\n");
		(*(((IOCPMqtt *) context)->dc))(((IOCPMqtt *) context)->context);	
	}
}

/**
 * 
 * 
 * @param 
 * @return 
 */
static void onDisconnectFailure(void* context, MQTTAsync_failureData* response)
{
	OCPUilts_log(OCPPROTOCOL_TRACE, "[onDisconnectFailure] response [%s]\n", ((IOCPMqtt *) context)->siteId, ((IOCPMqtt *) context)->thingName, response == NULL ? NULL : response->message);

	if(*(((IOCPMqtt *) context)->dc))
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[onDisconnectFailure] call cn \n");
		(*(((IOCPMqtt *) context)->dc))(((IOCPMqtt *) context)->context);	
	}
}

/**
 * 
 * 
 * @param 
 * @return 
 */
static void onSubscribeSuccess(void* context, MQTTAsync_successData* response)
{
	OCPUilts_log(OCPPROTOCOL_TRACE, "[onSubscribeSuccess] [%s+%s] response [%d]\n", ((IOCPMqtt *) context)->siteId, ((IOCPMqtt *) context)->thingName, response->alt.qos);
	if(*(((IOCPMqtt *) context)->cn))
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[onSubscribeSuccess] call cn \n");
		(*(((IOCPMqtt *) context)->cn))(((IOCPMqtt *) context)->context);	
	}
}

/**
 * 
 * 
 * @param 
 * @return 
 */
static void onSubscribeFailure(void* context, MQTTAsync_failureData* response)
{
	OCPUilts_log(OCPPROTOCOL_TRACE, "[onSubscribeFailure] [%s+%s] response [%s]\n", ((IOCPMqtt *) context)->siteId, ((IOCPMqtt *) context)->thingName, response->message);
	if(*(((IOCPMqtt *) context)->cn))
	{
		OCPUilts_log(OCPPROTOCOL_TRACE, "[onSubscribeFailure] call cn \n");
		(*(((IOCPMqtt *) context)->cn))(((IOCPMqtt *) context)->context);	
	}
}

/**
 * 
 * 
 * @param 
 * @return 
 */
static void onPublishSuccess(void* context, MQTTAsync_successData* response)
{
	OCPUilts_log(OCPPROTOCOL_TRACE, "[onPublishSuccess]\n");
}

/**
 * 
 * 
 * @param 
 * @return 
 */
static void onPublishFailure(void* context, MQTTAsync_failureData* response)
{
	OCPUilts_log(OCPPROTOCOL_TRACE, "[onPublishFailure][%s+%s] response [%s]\n", ((IOCPMqtt *) context)->siteId, ((IOCPMqtt *) context)->thingName, response->message);
}