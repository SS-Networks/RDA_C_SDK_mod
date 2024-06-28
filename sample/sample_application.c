#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#include <unistd.h>
#include <semaphore.h>

#include "IotClient.h"

//*********************************************************************************************************************
// GLOBAL AREA
//*********************************************************************************************************************
IotClient* client;

#define SITEID 		"CB00000000"
#define THINGNAME 	"TestModel.001"

#define AUTHCODE 	""
#define PORTAL_ID	"insator"
#define PORTAL_PW	"ocplab1!"

void onMessageArrive(void* context, OCPMessage* message);
void onConnectionClosed(void* context, char* cause);
void do_testcase(int select);

static int initialize_app();
static void uninitialize_app();
//*********************************************************************************************************************
// QUEUE DEFINITION
//*********************************************************************************************************************
#define MAX_QUEUE_SIZE 1000
typedef struct _node {
	void* message;
	struct _node* next;
} node;
typedef node* p_node;
typedef struct _queue {
	int size;
	p_node first;
	p_node last;
} queue;

queue* create_queue(void);
void destroy_queue(queue* q_ptr);
void add_queue(queue* q_ptr, void* message);
void delete_queue(queue* q_ptr);

static queue* send_queue;
static queue* receive_queue;

static void* thread_receiver(void* args);
static void* thread_sender(void* args);
static int thread_flag = 1;

static void* thread_connection(void* args);
static int connection_flag = 1;

static sem_t* sem_ptr;
static int timeout = 5;
static struct timespec ts;
//*********************************************************************************************************************
// MAIN FUNCTION IMPLE
//*********************************************************************************************************************
int main(int argc, char** argv)
{
	// set the thing information
	// option 1) set auth code, then don't need id/pw.
	// option 2) don't know auth code, then should set id/pw.

	IotClient* client = createIotClient(authCode, userId, userPw, SITEID, THINGNAME);
	if(client != NULL)
	{
		client->setCustomMessageListener(onMessageArrive, onConnectionClosed);
		if( client->connect() == 0 )
		{ // connected
			int select = 0;
			while(select != -1)
			{
				fprintf(stdout, "================================================\n");
				fprintf(stdout, "[SAMPLE EMULATOR]\n");
				fprintf(stdout, "================================================\n");
				fprintf(stdout, " (-1) EXIT \n");
				fprintf(stdout, "------------------------------------------------\n");
				fprintf(stdout, " ( 0) connect \n");
				fprintf(stdout, " ( 1) disconnect \n");
				fprintf(stdout, " ( 2) send Heart Beat \n");
				fprintf(stdout, " ( 3) activate root thing \n");
				fprintf(stdout, " ( 4) register leaf thing \n");
				fprintf(stdout, " ( 5) activate leaf thing \n");
				fprintf(stdout, " ( 6) request firmware latest version \n");
				fprintf(stdout, " ( 7) request signed data of firmware \n");
				fprintf(stdout, " ( 8) tell firmware upgrade is completed \n");
				fprintf(stdout, " ( 9) send Attributes \n");
				fprintf(stdout, " (10) send Attributes of leaf thing \n");
				fprintf(stdout, " SELECT: ");
				fscanf(stdin, "%d", &select);
				fprintf(stdout, "================================================\n");
				do_testcase(select);
			}

			// disconnect
			client->disconnect();
			// free resources
			destroyIotClient(client);
		}
		else
		{ // not connected
			fprintf(stderr, "Failed to connect.\n");
			exit(EXIT_FAILURE);
		}
	} 
	else
	{
		fprintf(stderr, "Failed to create Iot client.\n");
		exit(EXIT_FAILURE);		
	}

	return 0;
}

//*********************************************************************************************************************
// LISTENERS
//*********************************************************************************************************************
void onMessageArrive(void* context, OCPMessage* message)
{
	if(message)
	{
    	fprintf(stderr, "<< receive message->msgType : %s\n", message->msgType);
        fprintf(stderr, "<< receive message->msgId : %s\n", message->msgId);
        fprintf(stderr, "<< receive message->msgCode : %s\n", message->msgCode);
        fprintf(stderr, "<< receive message->resCode : %s\n", message->resCode);
        fprintf(stderr, "<< receive message->encType : %s\n", message->encType);
	}
}

void onConnectionClosed(void* context, char* cause)
{ // when the connection is closed, this will be called.
	if(cause != NULL && strlen(cause) != 0)
		fprintf(stdout, "[Connection Closed] %s\n", cause);
}
//*********************************************************************************************************************
// QUEUE IMPLEMENTATION
//*********************************************************************************************************************
queue* create_queue(void)
{
	queue* q = (queue*)malloc(sizeof(queue));
	q->size = 0;
	q->first = NULL;
	q->last = NULL;
	return q;
}
void destroy_queue(queue* q_ptr)
{
	while(q_ptr->size != 0)
		delete_queue(q_ptr);
	free(q_ptr);
	q_ptr = NULL;
}
void add_queue(queue* q_ptr, void* message)
{
	if(q_ptr->size >= MAX_QUEUE_SIZE) {
		return;
	}

	p_node new = (p_node) malloc(sizeof(node));
	new->message = message;
	new->next = NULL;

	if(q_ptr->size == 0) {
		q_ptr->first = new;
		q_ptr->last = new;
	} else {
		q_ptr->last->next = new;
		q_ptr->last = new;
	}

	(q_ptr->size)++;
}
void delete_queue(queue* q_ptr)
{
	if(q_ptr->size != 0) {
		p_node temp = q_ptr->first;
		q_ptr->first = temp->next;
		free(temp);
		temp = NULL;
		(q_ptr->size)--;
	}
}
//*********************************************************************************************************************
// STATIC FUNCTIONS
//*********************************************************************************************************************
static int initialize_app()
{
	receive_queue = create_queue();
	pthread_t	thread_r;
	if(pthread_create(&thread_r, NULL, thread_receiver, NULL) != 0) return 0;
	if(pthread_detach(thread_r) != 0) return 0;

	send_queue = create_queue();
	pthread_t 	thread_s;
	if(pthread_create(&thread_s, NULL, thread_sender, NULL) != 0) return 0;
	if(pthread_detach(thread_s) != 0) return 0;

	sem_ptr = malloc(sizeof(sem_t));
	if(sem_init(sem_ptr, 0, 0) != 0) return 0;

	pthread_t 	thread_cn;
	if(pthread_create(&thread_cn, NULL, thread_connection, NULL) != 0) return 0;
	if(pthread_detach(thread_cn) != 0) return 0;

	return 1;
}

static void uninitialize_app() 
{
	thread_flag = 0;
	while(send_queue->size > 0) {
		OCPMessage* message = (OCPMessage *) send_queue->first->message;
		if(message != NULL) freeMessage(message);
		delete_queue(send_queue);
	}
	while(receive_queue->size > 0) {
		OCPMessage* message = (OCPMessage *) receive_queue->first->message;
		if(message != NULL) freeMessage(message);
		delete_queue(receive_queue);
	}

	sem_destroy(sem_ptr);
	free(sem_ptr);
}
static void* thread_receiver(void* args)
{
	while(thread_flag) 
	{
		if(receive_queue->size > 0)
		{
			OCPMessage* message = (OCPMessage *) receive_queue->first->message;
			if(message)
			{
				if(!strcmp(OCPMESSAGE_MSGCODE_PRESAUTHPROCESS_RES, message->msgCode)) {
		        	fprintf(stderr, "<< PRESAUTHPROCESS_RES\n");
		        } else if(!strcmp(OCPMESSAGE_MSGCODE_AUTHPROCESS_RES, message->msgCode)) {
		        	fprintf(stderr, "<< AUTHPROCESS_RES\n");
		        } else if(!strcmp(OCPMESSAGE_MSGCODE_SEND_HEARTBEAT_RES, message->msgCode)) {
		        	fprintf(stderr, "<< SEND_HEARTBEAT_RES\n");
		        } else if(!strcmp(OCPMESSAGE_MSGCODE_ACTIVATE_ROOTTHING_RES, message->msgCode)) {
		        	fprintf(stderr, "<< ACTIVATE_ROOTTHING_RES\n");
		        } else if(!strcmp(OCPMESSAGE_MSGCODE_REGISTER_EDGETHING_RES, message->msgCode)) {
		        	fprintf(stderr, "<< REGISTER_EDGETHING_RES\n");
		        } else if(!strcmp(OCPMESSAGE_MSGCODE_INACTIVATE_THING_RES, message->msgCode)) {
		        	fprintf(stderr, "<< INACTIVATE_THING_RES\n");
		        } else if(!strcmp(OCPMESSAGE_MSGCODE_BULK_INSERT_RES, message->msgCode)) {
		        	fprintf(stderr, "<< BULK_INSERT_RES\n");
		        } else if(!strcmp(OCPMESSAGE_MSGCODE_REQUEST_ACTIVEFIRMWARE_RES, message->msgCode)) {
		        	fprintf(stderr, "<< REQUEST_ACTIVEFIRMWARE_RES\n");
		        	fprintf(stderr, "data:%s\n", (char*)(message->data));
		        } else if(!strcmp(OCPMESSAGE_MSGCODE_NOTI_PASSIVEFIRMWARE_REQ, message->msgCode)) {
		        	fprintf(stderr, "<< NOTI_PASSIVEFIRMWARE_REQ\n");
		        	fprintf(stderr, "data:%s\n", (char*)(message->data));
		        } else if(!strcmp(OCPMESSAGE_MSGCODE_SEND_FIRMWARERESULT_RES, message->msgCode)) {
		        	fprintf(stderr, "<< SEND_FIRMWARERESULT_RES\n");
		        } else if(!strcmp(OCPMESSAGE_MSGCODE_GET_SIGNEDFIRMWARE_RES, message->msgCode)) {
		        	fprintf(stderr, "<< GET_SIGNEDFIRMWARE_RES\n");
		        } else if(!strcmp(OCPMESSAGE_MSGCODE_NOTIREQUEST_REQ, message->msgCode)) {
		        	fprintf(stderr, "<< NOTIREQUEST_REQ\n");
		        } else {
		        	fprintf(stderr, "<< don't know this message. code(%s)\n", message->msgCode);
		        }

			}
		}
	}
}
int fail_cnt = 0;
static void* thread_sender(void* args)
{
	while(thread_flag)
	{
		if(send_queue->size > 0) 
		{
			OCPMessage* message = (OCPMessage *) send_queue->first->message;
			char* res = sendMessage(message);
			if(res != NULL && !strcmp(res, message->msgId))
            {
                // fprintf(stderr, "%s\n", "thread_producer");
                OCPManager_free_message(message);
                delete_queue(send_queue);
            }
            else
            {
            	fprintf(stderr, "** send fail [%d]", fail_cnt++);
            }

            if(fail_cnt > 3)
            {
            	fail_cnt = 0;
            	OCPManager_free_message(message);
                delete_queue(send_queue);	
            }
		}
	}
}
//*********************************************************************************************************************
// PRIVATE FUNCTIONS
//*********************************************************************************************************************
void do_testcase(int select)
{
	if( select < -1 || select > 10 )
		fprintf(stdout, "select between (-1 ~ 10)\n");
	else
	{
		switch(select)
		{
			case -1: 	// do nothing
				break;
			case 0:		// connect
				if(client->connect() == 0) {
					printf(">> connected.\n");
				}
				break;
			case 1:		// disconnect
				client->disconnect();
				break;
			case 2:		// send heart beat
				client->sendHeartBeat();
				break;
			case 3:		// activate root thing
				printf("(Activate a Root Thing)\n");
				char model[SHORT_STR_LEN] = {0, };
				fprintf(stderr, "Input model name:");
				fscanf(stdin, "%s", model);
				char uniqueNum[SHORT_STR_LEN] = {0, };
				fprintf(stderr, "Input unique number:");
				fscanf(stdin, "%s", uniqueNum);
				if(client->activateThing(model, uniqueNum) != 0) {
					printf("failed to activate a root thing.\n");
				}
				break;
			case 4:		// register leaf thing
				break;
			case 5:		// activate leaf thing
				break;
			case 6:		// request firmware latest version
				break;
			case 7:		// request signed data of firmware
				break;
			case 8:		// tell firmware upgrade is completed
				break;
			case 9:		// send attributes
			printf("send attributes.\n");
				char* msgCode = NULL;
				// can see these attributes on "Thing Model Management" menu in Service Portal
				char* baseStr = "{\"temperature\": %d, \"humidity\": %d}";

				// put values into base string
				char dataStr[LONG_STR_LEN] = {0, };
				sprintf(dataStr, baseStr, 1004, 2003);

				// send attributes data for Root thing
				client->sendAttributes(msgCode, dataStr);

				break;
			case 10:	// send attributes of leaf thing
				break;
			default:
				break;
		}	
	}
	
}

