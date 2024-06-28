/**
 * 
 * 
 * @author mw50.jeong
 * @since 2018. 2. 1.
 */

#ifndef _OCP_UTILS_H_
#define _OCP_UTILS_H_

#include <stdio.h>
#include <semaphore.h>

// trace log 
void OCPUilts_log(char* target, char* fmt, ...);
// sha256 hash
void OCPUtils_getHash(char* plain, char** hash);
// get system current time 
unsigned long long OCPUtils_getCurrentTime();
// get time based uuid 
char* OCPUtils_generateUUID();
// create semaphore
sem_t* OCPThread_create_sem(void);
// waite semaphore
int OCPThread_wait_sem(sem_t* sem, int timeout);
// check sempaphore
int OCPThread_check_sem(sem_t* sem);
// post sempaphore
int OCPThread_post_sem(sem_t* sem);
// destroy sempaphore
int OCPThread_destroy_sem(sem_t* sem);

#endif