/*========================================================
 * 시스템  명
 * 프로그램ID
 * 프로그램명
 * 개　　　요
 * 변 경 일자
=========================================================*/

#include "OCPSecurityAES.h"
#include "aes_ocp.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct 
{
	int aes_type;
	int aes_len;
	char AES128[16];
	char AES256[32];
}IOCPSecurityAES;

/**
 * 
 * 
 * @param 
 * @return 
 */
int OCPSecurityAES_create(OCPSecurityAES* handle, OCPSecurityAES_properties* properties)
{
 // fprintf(stderr, "\n\n\nOCPSecurityAES_create \n\n\n");	
	if(*handle != NULL || properties == NULL)
 	{
 		// OCPUilts_log(OCPSECURITY_TRACE, "[OCPSecurityWbc_create] [*handle] is not null or [properties] is null \n");
 		fprintf(stderr, "NULL!!\n");
 		return 0;
 	}

 	IOCPSecurityAES *security = NULL;
 	security = malloc(sizeof(IOCPSecurityAES));
 	memset(security, '\0', sizeof(IOCPSecurityAES));
 	*handle = security; 

 	security->aes_type = properties->aes_type;

// fprintf(stderr, "security->aes_type : %d\n", security->aes_type);
 	if(security->aes_type == 128)
 	{
		security->aes_len = 16;
 	}
 	else if(security->aes_type == 256)
 	{
		security->aes_len = 32;
 	}
 	else
 	{
 		OCPSecurityAES_destory(handle);
 		// OCPEncryptorAES128_destroy(security);
		// OCPUilts_log(OCPEncryptor_Trace, "@@ OCPEncryptorAes128_create FAIL, because key generate failed STEP#1 @@\n");
 		return 0;
 	}
// fprintf(stderr, "---- 0\n"); 	 	
// fprintf(stderr, "security->aes_len : %d\n", security->aes_len); 	

// fprintf(stderr, "---- 1\n"); 	

	char* md = OCPSecurity_sha256(properties->auth_code);
	if(md == NULL)
	{
// fprintf(stderr, "md is null properties->auth_code : %s\n", properties->auth_code); 			
		OCPSecurityAES_destory(handle);
		// OCPEncryptorAES128_destroy(security);
		// OCPUilts_log(OCPEncryptor_Trace, "@@ OCPEncryptorAes128_create FAIL, because key generate failed STEP#1 @@\n");
		return 0;
	}
	char* encode = OCPSecurity_encode(md, security->aes_len);
// fprintf(stderr, "char* encode = OCPSecurity_encode(md, security->%d);\n", security->aes_len);
	if(encode == NULL)
	{
// fprintf(stderr, "encode is null md : %s\n", md); 					
		if(!md)
		{
			free(md);
		}
		OCPSecurityAES_destory(handle);
		// OCPEncryptorAES128_destroy(security);
		// OCPUilts_log(OCPEncryptor_Trace, "@@ OCPEncryptorAes128_create FAIL, because key generate failed STEP#2 @@\n");
		return 0;
	}

	char key[security->aes_len];
	memset(key, 0, sizeof(char) * security->aes_len);
  	memcpy(key, encode, security->aes_len);
// fprintf(stderr, "---- 2\n"); 	
	if(security->aes_type == 128)
 	{
	// security->AES128 = malloc(KEYLEN + 1);
		memset(security->AES128, 0, sizeof(char) * security->aes_len);
		memcpy(security->AES128, key, security->aes_len);
	}
	else if(security->aes_type == 256)
	{
		memset(security->AES256, 0, sizeof(char) * security->aes_len);
		memcpy(security->AES256, key, security->aes_len);	
	}
// fprintf(stderr, "---- 3\n"); 		
	if(md != NULL) 
	{
		free(md);
		md = NULL;
	}
	
	if(encode != NULL) 
	{
		free(encode);
		encode = NULL;	
	}
// fprintf(stderr, "----------- success \n");	

	return 1;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
int OCPSecurityAES_destory(OCPSecurityAES* handle)
// void OCPThingManager_destroy(OCPManager* handle)
{
// 	OCPUilts_log(OCPMANAGER_TRACE, "@@ OCPThingManager_destroy START @@\n");
	IOCPSecurityAES *security = *handle;
	if(security == NULL)
	{
		// OCPUilts_log(OCPMANAGER_TRACE, "@@ OCPProtocolMqtt_destroy FAIL, because [ocpManager] is null > already destroy @@\n");
		return 0;
	}

	security->aes_type = 0;
	security->aes_len = 0;	
	memset(security->AES128, 0, sizeof(security->AES128));
 	memset(security->AES256, 0, sizeof(security->AES256));

	if(security != NULL)
	{
		free(security);
		security = NULL;
	}

	*handle = NULL;
// 	OCPUilts_log(OCPMANAGER_TRACE, "@@ OCPThingManager_destroy END @@\n");
	return 1;
}

// void* init_swbc(void* handle)
// {
// 	IOCPSecurityWbc *security = handle;
// 	if(security == NULL)
// 	{
// 		fprintf(stderr, "initSWBC - security null\n");
// 		// OCPUilts_log(OCPMANAGER_TRACE, "[initSWBC] [security] is null \n");			
// 		return 0;
// 	}

// 	void* result = NULL;
// 	switch(security->swbcType)
// 	{
// 		case 1:
// 		{
// 			aes128_swbc_layer1_ctr_ctx_t *ctx = NULL;
// 			ctx = (aes128_swbc_layer1_ctr_ctx_t *)malloc(sizeof(aes128_swbc_layer1_ctr_ctx_t));
// 			if(!ctx)
// 			{
// 				fprintf(stderr, "ctx allocation failed.\n");
// 				// printf("ctx allocation failed.\n");
// 			    // error_code = -1;
// 			    // goto enc_dec_text_test_free;
// 				if(ctx)
// 				{
// 					free(ctx);
// 					ctx = NULL;
// 				}			    
// 			}
// 			if(SWBC_SUCCESS != aes128_swbc_layer1_ctr_load(ctx, (const unsigned char **)security->encTable, (const unsigned char *)security->encTableTag)) 
// 			{
// 				fprintf(stderr, "swbc table load error\n");
// 				// printf("swbc table load error\n");
// 				// error_code = -1;
// 				// goto enc_dec_text_test_free; 
// 				if(ctx)
// 				{
// 					free(ctx);
// 					ctx = NULL;
// 				}
// 			}

// 			if(SWBC_SUCCESS != aes128_swbc_layer1_ctr_init(ctx, security->mc)) 
// 			{ /* swbc encryption mode */
// 				// printf("swbc init error\n");
// 				// error_code = -1;
// 				// goto enc_dec_text_test_free;
// 				if(ctx)
// 				{
// 					free(ctx);
// 					ctx = NULL;
// 				}
// 			}
// 			result = ctx;
// 		}
// 		break;
// 		case 5:
// 		{

// 		}
// 		break;
// 		default:
// 		{
// 			result = NULL;
// 		}
// 		break;
// 	}
// 	return result;
// }

/**
 * 
 * 
 * @param 
 * @return 
 */
unsigned char *OCPSecurityAES_decrypt(void* handle, char* header_format, unsigned char *encrypted, int encrypted_len, int *decrypted_len)
{
	IOCPSecurityAES *security = handle;
	if(security == NULL)
	{
		// OCPUilts_log(OCPMANAGER_TRACE, "[initSWBC] [security] is null \n");	
		// fprintf(stderr, "[initSWBC] [security] is null \n");		
		return 0;
	}

	size_t enc_in_len = 0;
	unsigned char* enc_in = NULL;

	if(!strcmp(header_format, OCPMESSAGECONVERTER_JSON))
	{
		enc_in_len = 0;
		enc_in = OCPSecurity_decode(encrypted, encrypted_len, &enc_in_len);

		// fprintf(stderr, "-------- decode_len %ld\n", decode_len);
		if(!enc_in)
		{
			// fprintf(stderr, "!enc_in \n");		
			return NULL;
		}	

	}
	else
	{
		enc_in_len = encrypted_len;
		enc_in = malloc(encrypted_len + 1);
		memset(enc_in, 0, (enc_in_len * sizeof(char)) + 1 );
		memcpy(enc_in, encrypted, enc_in_len);
	}

	//IV 추출 
	char iv[IV_LEN] = {0, };
	int i;
	for(i = 0; i < IV_LEN; i++)
	{
		iv[i] = enc_in[i];
	}

	int cipher_len = enc_in_len - IV_LEN;
	// fprintf(stderr, "-------- cipher_len %d\n", cipher_len);
	if(cipher_len <= 0) {
		free(enc_in);
		return NULL;
	}

	unsigned char cipher[cipher_len + 1];
	//memset(cipher, 0, cipher_len + 1);
	// code inspection : memset 할 때 sizeof 사용해라
	memset(cipher, 0, sizeof(cipher));
	for(i = 0; i < cipher_len; i++)
	{
		cipher[i] = enc_in[i + IV_LEN];
	}
	// cipher[cipher_len] = '\0';
	if(enc_in != NULL) { free(enc_in); }

	// int buf_size = cipher_len;

	
	// OCPUilts_log(OCPEncryptor_Trace, "@@ OCPEncryptorAes128_decrypt START @@\n");
	// IOCPAES128 *encryptor = handle;
	// if(encryptor == NULL)
	// {
	// 	OCPUilts_log(OCPEncryptor_Trace, "@@ OCPEncryptorAes128_decrypt FAIL, because [encryptor] is null @@\n");
	// 	return OCPEncryptor_FAILURE;
	// }

	// if(data == NULL || strlen(data) < 1)
	// {
	// 	OCPUilts_log(OCPEncryptor_Trace, "@@ OCPEncryptorAes128_decrypt FAIL, because [data] is null | datalen is [0] @@\n");
	// 	return OCPEncryptor_FAILURE;
	// }


	// int buf_size = datalen;

	// uint8_t outputBytes[buf_size];

	// uint8_t dataBytes[datalen + 1];
 //  	memset(dataBytes, 0x00, sizeof(char) * datalen + 1); 	
 //  	memcpy(dataBytes, data, datalen);

 //  	uint8_t keyBytes[KEYLEN + 1];
	// memset(keyBytes, 0, sizeof(char) * KEYLEN + 1); 	
 //  	memcpy(keyBytes, encryptor->key, KEYLEN);

 //  	uint8_t ivBytes[IVLEN + 1];
	// memset(ivBytes, 0, sizeof(char) * IVLEN + 1); 	
 //  	memcpy(ivBytes, iv, IVLEN);



  	struct AES_ctx ctx;
  	if(security->aes_type == 128)
  	{
// fprintf(stderr, "%s", "b");
    	AES_init_ctx_iv_mode(&ctx, security->AES128, iv, 128);
	}
	else 
	{
// fprintf(stderr, "%s", "d");
		AES_init_ctx_iv_mode(&ctx, security->AES256, iv, 256);
	}
    AES_CBC_decrypt_buffer_mode(&ctx, cipher, cipher_len);
  	// AES128_CBC_decrypt_buffer(outputBytes, dataBytes, datalen, keyBytes, ivBytes);

	//여기서 cipher 랑 cipher len 이 모두 바뀐다 
  	int padding = cipher[cipher_len - 1];
  	int dec_len = cipher_len - padding;
  	if(padding < 0x01 || padding > 0x10 || dec_len < 1)
  	{
  		// OCPUilts_log(OCPEncryptor_Trace, "@@ OCPEncryptorAes128_decrypt FAIL, because [padding | buf_size] invalid @@\n");
  		// return OCPEncryptor_FAILURE;
  		// fprintf(stderr, "@@@@@@@@@@@@ padding fail \n");
  		return NULL;
  	}

	// int i;
// unsigned char dest[data_size];
// 	int destlen = 0;
  	//data 생성 및 전달 인거잖아  ? 
	// for(i = 0; i < data_size; i++)
	// {
	// 	dest[i] = cipher[i];
	// }
	// dest[data_size] = '\0';
// memset(dest, 0, data_size);
// memcpy(dest, cipher, data_size);

  	// *destlen = data_size;

  	unsigned char* dec_out = (unsigned char *)malloc(dec_len + 1);
  	// if(!dec_out)
			// {
			// 	// printf("decryption buffer allocation failed\n");
			// 	// error_code = -1;
			// 	// goto enc_dec_text_test_free;
			// 	if(ctx)
			// 	{
			// 		free(ctx);
			// 		ctx = NULL;
			// 	}
			// 	return NULL;				
			// }
  	//실
	memset(dec_out, 0, dec_len * sizeof(char) + 1);
	memcpy(dec_out, cipher, dec_len);

	*decrypted_len = dec_len;
	// fprintf(stderr, " %d / %s\n", dec_len, dec_out);
	return dec_out;
}

/**
 * 
 * 
 * @param 
 * @return 
 */
unsigned char *OCPSecurityAES_encrypt_to_string(void* handle, char* header_format, char *plain_text, int plain_text_len, int *encrypted_len)
{
	// fprintf(stderr, "--------------------- OCPSecurityAES_encrypt_to_string\n");		

	IOCPSecurityAES *security = handle;
	if(security == NULL)
	{
		return 0;
	}

	char* iv = OCPSecurity_generate_iv();

	// int enc_len = security->aes_len * (plain_text_len / security->aes_len + 1);
	int enc_len = 16 * (plain_text_len / 16 + 1);
	uint8_t enc_out[enc_len];
  	memset(enc_out, 0x00, sizeof(char) * enc_len);
  	memcpy(enc_out, plain_text, plain_text_len);

 //   	uint8_t ivBytes[IV_LEN];
	// memset(ivBytes, 0, sizeof(char) * IV_LEN); 	
 //  	memcpy(ivBytes, iv, IV_LEN);

	struct AES_ctx ctx;
  	if(security->aes_type == 128)
  	{
		AES_init_ctx_iv_mode(&ctx, security->AES128, iv, 128);
  	}
  	else
  	{
  		AES_init_ctx_iv_mode(&ctx, security->AES256, iv, 256);	
  	}
	
    
    AES_CBC_encrypt_buffer_pkcs5(&ctx, enc_out, plain_text_len);

    int data_len = IV_LEN + enc_len;
	unsigned char* data = (unsigned char *)malloc(data_len + 1);
	memset(data, 0, data_len * sizeof(char) + 1);
	memcpy(data, iv, IV_LEN);
	memcpy(data + IV_LEN, enc_out, enc_len);

	if(!strcmp(header_format, OCPMESSAGECONVERTER_JSON))
	{
		char* encode = NULL;
		if(data != NULL)
		{
			encode = OCPSecurity_encode(data, data_len);
		}
		if(data != NULL)
		{
			free(data);
		}
	
		if(encode != NULL)		
		{
			*encrypted_len = strlen(encode);
		}
		// fprintf(stderr, "--------------------- encode : %s\n", encode);		
		return encode;

	}
	else
	{
		*encrypted_len = data_len;
		// fprintf(stderr, "--------------------- data : %s\n", data);		
		return data;
	}

	// char* encode = NULL;
	// if(data)
	// {
	// 	encode = OCPSecurity_encode(data, data_len);
	// 	free(data);
	// }
}