/*========================================================
 * 시스템  명
 * 프로그램ID
 * 프로그램명
 * 개　　　요
 * 변 경 일자
=========================================================*/

#include "OCPUtils.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>

#include "sha256.h"

// TRACE LOG BUFFER
#define OCP_SDK_TRACE_BUFSIZE 8192

/**
 * 
 * 
 * @param 
 * @return 
 */
void OCPUilts_log(char* target, char* fmt, ...)
{
	//char* envval = NULL;
	//FILE* trace_destination = stderr;

	// code inspection : getenv 사용금지
	// OCPUtils 은 현재 사용되지 않으며, 다른 유틸로 대체예정 
	/* if ((envval = getenv("OCP_SDK_TRACE")) != NULL && strlen(envval) > 0){
		if (!strcmp(envval, "ON") || !strcmp(envval, target)){
			char buf[OCP_SDK_TRACE_BUFSIZE] = {0, };
			memset(buf, 0, OCP_SDK_TRACE_BUFSIZE * sizeof(char));
			struct tm *tm;
			struct timeval tv;
			gettimeofday( &tv, NULL );
			tm = localtime( &tv.tv_sec );
			sprintf(buf, "[%d/%d/%d %d:%d:%d][%s] ", tm->tm_year +1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, target);
			int prefixLen = strlen(buf);

			int rc = 0;
			va_list args = NULL;
			va_start(args, fmt);
			rc = vsprintf(buf + prefixLen, fmt, args);
			va_end(args);

			fprintf(trace_destination, "%s", buf);
		}
	}*/
}

/**
 * 
 * 
 * @param 
 * @return 
 */
void OCPUtils_getHash(char* plain, char** hash)
{
	if(plain == NULL || strlen(plain) < 1)
	{
		return;
	}

	CSha256 csha256 = {0, };
	Byte md[SHA256_DIGEST_SIZE] = {0, };
	Sha256_Init(&csha256);
	Sha256_Update(&csha256, (Byte*)plain, strlen(plain));
	Sha256_Final(&csha256, md);

	*hash = (char*) malloc (2 * SHA256_DIGEST_SIZE + 1);
	memset(*hash, 0, sizeof(char)*(2 * SHA256_DIGEST_SIZE) + 1);
	char* buf_ptr = *hash;
	int index;
	for(index = 0; index < SHA256_DIGEST_SIZE; index++)
	{
		buf_ptr += sprintf(buf_ptr, "%02x", md[index]);
	}
}

/**
 * 
 * 
 * @param 
 * @return 
 */
unsigned long long OCPUtils_getCurrentTime()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	unsigned long long millisecondsSinceEpoch =
    	(unsigned long long)(tv.tv_sec) * 1000 +
    	(unsigned long long)(tv.tv_usec) / 1000;

	return millisecondsSinceEpoch;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
char* OCPUtils_generateUUID()
{
	char times[20] =  {0, };
	snprintf(times, 20, "%llu", OCPUtils_getCurrentTime());
	char* hashed = NULL;
	OCPUtils_getHash(times, &hashed);

	if(hashed == NULL)
	{
		return "1a2b3c4d-5e6f-7g8h-9i0j-1k2l3m4l5o6p";
	}

	char* hashPointer = hashed;

	char* uuid = malloc(37); 
	int uuidIndex = 0;
	int index = 0;
	for(index = 0; index < 8; index++)
	{
		*(uuid + uuidIndex++) = *(hashed++);	
	}
	*(uuid + uuidIndex++) = '-';

	for(index = 0; index < 4; index++)
	{
		*(uuid + uuidIndex++) = *(hashed++);	
	}
	*(uuid + uuidIndex++) = '-';

	for(index = 0; index < 4; index++)
	{
		*(uuid + uuidIndex++) = *(hashed++);	
	}
	*(uuid + uuidIndex++) = '-';

	for(index = 0; index < 4; index++)
	{
		*(uuid + uuidIndex++) = *(hashed++);	
	}
	*(uuid + uuidIndex++) = '-';

	for(index = 0; index < 12; index++)
	{
		*(uuid + uuidIndex++) = *(times + index + 1);	
	}
	*(uuid + uuidIndex) = '\0';

	free(hashPointer);
	return uuid;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
sem_t* OCPThread_create_sem(void)
{
	sem_t* sem = NULL;
	int rc = 0;
	sem = malloc(sizeof(sem_t));
	rc = sem_init(sem, 0, 0);

	if(rc != 0){
		if(sem != NULL)
		{
			free(sem);
			sem = NULL;
		}
		return NULL;
	}

	return sem;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
int OCPThread_wait_sem(sem_t* sem, int timeout)
{
	int rc = -1;

	struct timespec ts;

	if (clock_gettime(CLOCK_REALTIME, &ts) != -1)
	{
		ts.tv_sec += timeout;
		rc = sem_timedwait(sem, &ts);
	}

 	return rc;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
int OCPThread_check_sem(sem_t* sem)
{
	int semval = -1;
	sem_getvalue(sem, &semval);
	return semval > 0;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
int OCPThread_post_sem(sem_t* sem)
{
	int rc = 0;

	if (sem_post(sem) == -1){
		rc = errno;
	}

  return rc;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
int OCPThread_destroy_sem(sem_t* sem)
{
	int rc = 0;

	rc = sem_destroy(sem);
	free(sem);

	return rc;
}
