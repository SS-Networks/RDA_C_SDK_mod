/*========================================================
 * 시스템  명
 * 프로그램ID
 * 프로그램명
 * 개　　　요
 * 변 경 일자
=========================================================*/

#include "OCPSecurity.h"
#include "OCPUtils.h"

#include "b64.h"
#include "sha256.h"

#include <stdio.h>
#include <string.h>
#include <sys/time.h>

/**
 * 
 * 
 * @param 
 * @return 
 */
char* OCPSecurity_encode(unsigned char* data, int len)
{
	// OCPUilts_log(OCPEncryptor_Trace, "@@ OCPEncryptor_encode START @@\n");
	if(data == NULL || len < 1)
	{
		OCPUilts_log(OCPSECURITY_TRACE, "[OCPSecurity_encode] [data] is null | [len] < 1\n");
		return NULL;
	}

	// OCPUilts_log(OCPEncryptor_Trace, "@@ OCPEncryptor_encode END @@\n");
	return b64_encode(data, len);
}

/**
 * 
 * 
 * @param 
 * @return 
 */
unsigned char* OCPSecurity_decode(char* data, int len, size_t* decodelen)
{
	// OCPUilts_log(OCPEncryptor_Trace, "@@ OCPEncryptor_decode START @@\n");
	if(data == NULL || len < 1)
	{
		fprintf(stderr, "OCPEncryptor_decode FAIL, because [data] is null | len is [0]");
		// OCPUilts_log(OCPEncryptor_Trace, "@@ OCPEncryptor_decode FAIL, because [data] is null | len is [0] @@\n");
		return NULL;
	}

	// OCPUilts_log(OCPEncryptor_Trace, "@@ OCPEncryptor_decode END @@\n");
	//return b64_decode_len(data, len, decodelen);
	return b64_decode_ex(data, len, decodelen);
}

/**
 * 
 * 
 * @param 
 * @return 
 */
char* OCPSecurity_sha256(char* plain)
{
	if(plain == NULL || strlen(plain) < 1)
	{
		// OCPUilts_log(OCPEncryptor_Trace, "@@ OCPEncryptor_sha256 FAIL, because [plain] is null | plainlen is [0] @@\n");
		return NULL;
	}

	CSha256 csha256 = {0, };
	Byte md[SHA256_DIGEST_SIZE] = {0, };
	Sha256_Init(&csha256);
	Sha256_Update(&csha256, (Byte*)plain, strlen(plain));
	Sha256_Final(&csha256, md);

	// char* hash = (char*) malloc (SHA256_DIGEST_SIZE + 1);
	// memcpy(hash, md, SHA256_DIGEST_SIZE);	
	// hash[SHA256_DIGEST_SIZE] = 0;
	char* hash = (char*) malloc (SHA256_DIGEST_SIZE );
	memcpy(hash, md, SHA256_DIGEST_SIZE);	

	return hash;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
char* OCPSecurity_generate_iv()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	unsigned long long millisecondsSinceEpoch =
    	(unsigned long long)(tv.tv_sec) * 1000 +
    	(unsigned long long)(tv.tv_usec) / 1000;

	char times[IV_LEN] = {0, };
	snprintf(times, IV_LEN, "%llu", millisecondsSinceEpoch);

	char* res = NULL;
	char* encode = NULL;
	char* tmp_iv = NULL;
	char* iv = NULL;

	res = OCPSecurity_sha256(times);
	if(res == NULL)
	{
		tmp_iv = "74c2262a7550cf1b";
		// iv = malloc(IV_LEN + 1);
		// memset(iv, 0, sizeof(char) * IV_LEN + 1);
		iv = malloc(IV_LEN);
		memset(iv, 0, sizeof(char) * IV_LEN);
		memcpy(iv, tmp_iv, IV_LEN);	

		// OCPUilts_log(OCPEncryptor_Trace, "@@ generateIV FAIL, res is NULL [%s] @@\n", iv);
		return iv;
	}

	encode = OCPSecurity_encode(res, IV_LEN);
	if(encode == NULL)
	{
		if(res != NULL) 
		{
			free(res);
		}
		
		tmp_iv = "74c2262a7550cf1b";
		// iv = malloc(IV_LEN + 1);
		// memset(iv, 0, sizeof(char) * IV_LEN + 1);
		iv = malloc(IV_LEN);
		memset(iv, 0, sizeof(char) * IV_LEN);
		memcpy(iv, tmp_iv, IV_LEN);	

		// OCPUilts_log(OCPEncryptor_Trace, "@@ generateIV FAIL, encode is NULL [%s] @@\n", iv);
		return iv;
	}

	// tmp_iv = malloc(IV_LEN + 1);
	// memset(tmp_iv, 0, sizeof(char) * IV_LEN + 1);
	tmp_iv = malloc(IV_LEN);
	memset(tmp_iv, 0, sizeof(char) * IV_LEN);
	memcpy(tmp_iv, encode, IV_LEN);

	// iv = malloc(IV_LEN + 1);
	// memset(iv, 0, sizeof(char) * IV_LEN + 1);
	iv = malloc(IV_LEN);
	memset(iv, 0, sizeof(char) * IV_LEN);
	memcpy(iv, tmp_iv, IV_LEN);		

	if(res != NULL) 
	{
		free(res);
	}
	
	if(encode != NULL) 
	{
		free(encode);
	}

	if(tmp_iv != NULL) 
	{
		free(tmp_iv);
	}

	return iv;
}

