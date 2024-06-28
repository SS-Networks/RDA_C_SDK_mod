/**
 * 
 * 
 * @author mw50.jeong
 * @since 2018. 2. 1.
 */

#ifndef _OCPSecurity_
#define _OCPSecurity_

#include "OCPTypes.h"
#include <stdlib.h>

// log trace for security
#define OCPSECURITY_TRACE          "OCPSECURITY"
// iv len 
#define IV_LEN 16
// base64 encode
char* OCPSecurity_encode(unsigned char* data, int len);
// base64 decode
unsigned char* OCPSecurity_decode(char* data, int len, size_t* decodelen);
// hash with sha256
char* OCPSecurity_sha256(char* plain);
// generate iv
char* OCPSecurity_generate_iv();

#endif