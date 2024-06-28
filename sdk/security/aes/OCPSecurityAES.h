/**
 * 
 * 
 * @author mw50.jeong
 * @since 2018. 2. 1.
 */

#ifndef _OCPSecurityAES_
#define _OCPSecurityAES_

#include "OCPSecurity.h"

// security aes pointer
typedef void* OCPSecurityAES;
// security aes properties struct
typedef struct 
{
	int aes_type;
	char auth_code[32];	
}OCPSecurityAES_properties;
// security aes properties initializer
#define OCPSECURITYAES_PROPERTIES_INITIALIZER { 128, {0,} }
// create security 
int OCPSecurityAES_create(OCPSecurityAES* handle, OCPSecurityAES_properties* properties);
// destroy security 
int OCPSecurityAES_destory(OCPSecurityAES* handle);
// decrypt 
unsigned char *OCPSecurityAES_decrypt(void* handle, char* header_format, unsigned char *encrypted, int encrypted_len, int *decrypted_len);
// encrypt 
unsigned char *OCPSecurityAES_encrypt_to_string(void* handle, char* header_format, char *plain_text, int plain_text_len, int *encrypted_len);

#endif