#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "OCPMessageCompressor.h"
#define ZSTD_STATIC_LINKING_ONLY
#include "zstd.h"

int startsWith(const char *pre, const char *str);
int skipMessage(char* msgCode);
void printHex(void* src, int len);

int startsWith(const char *pre, const char *str)
{
    int lenpre = strlen(pre);
    int lenstr = strlen(str);
    return lenstr < lenpre ? !0 : strncmp(pre, str, lenpre);
}

void OCPMessageCompressor_convert_encType(char encType[], int enclen, int compressionType)
{
	if(!encType) {
		return;
	}

	int i;
	char tmp[enclen + 1];

	for(i = 0; i < enclen; i++)
	{
		tmp[i] = encType[i];
	}

	if(0 == compressionType) //decompress
	{	
		for(i = 0; i < enclen; i++)
		{
			encType[i] = 0;
		}
		for(i = 0; i < enclen - 1; i++)
		{
			encType[i] = tmp[i + 1];
		}
	}
	else if(1 == compressionType)
	{
		for(i = 0; i < enclen; i++)
		{
			encType[i] = 0;
		}
		encType[0] = 'Z';
		for(i = 0; i < enclen; i++)
		{
			encType[i + 1] = tmp[i];
		}
		
	}
	else
	{
		//log 만 찍어 
	}
}

int skipMessage(char* msgCode)
{
	int rc = 0;

	if(!msgCode) {
		return 1;
	}

	if(0 == strlen(msgCode) || !strcmp("", msgCode)) {
		rc = 2;
	}

	if(!startsWith(OCPMESSAGE_MSGCODE_AUTH, msgCode) 
		|| !strcmp(OCPMESSAGE_MSGCODE_SEND_HEARTBEAT_REQ, msgCode) 
		|| !strcmp(OCPMESSAGE_MSGCODE_SEND_HEARTBEAT_RES, msgCode)) {
		rc = 3;
	}

	return rc;
}

int OCPMessageCompressor_needCompress(char* msgCode, int datalen, int minCompressionSize)
{
	int rc = 0; 

	if (minCompressionSize <= 0 || datalen <= 0) {
		// 			throw new IllegalArgumentException(
// 					"Wrong compression bytes. Please check your server setting. only 0, 1, 1048576(=1Mb) server setting is "
// 							+ minCompressionSize);
		rc = 1;
	}

	if(skipMessage(msgCode)) {
		rc = 2;
	}

	if(datalen < minCompressionSize) {
		rc = 3;
	}

	return rc;
}

int OCPMessageCompressor_needDecompress(char* encType)
{
	int rc = 0;

	if(startsWith("Z", encType)) {
		rc = !0;
	}

	return rc;
}

void printHex(void* src, int len)
{
	int i;
	char* tBuff = (char *)src;
	for(i = 0; i < len; i++)
	{
		fprintf(stderr, "%02x", tBuff[i] & 0xff);
	}
	fprintf(stderr, "\n");
}

void OCPMessageCompressor_compress(void** data, int* datalen)
{
	if(*data == NULL || datalen <= 0) {
		return;
	}

	// fprintf(stderr, "STEP #2 COMPRESS\n");
	int fSize = *datalen;
	void* const fBuff = malloc(fSize + 1);
	memset(fBuff, 0, fSize + 1);
	memcpy(fBuff, *data, fSize);

	// fprintf(stderr, "fSize : %d\n", fSize);
	// fprintf(stderr, "fBuff : %s\n", (char*)fBuff);
	// fprintf(stderr, "fBuff HEX : ");
	// printHex(fBuff, fSize);

	size_t const cBuffSize = ZSTD_compressBound(fSize);
	void* const cBuff = malloc(cBuffSize);
	size_t const cSize = ZSTD_compress(cBuff, cBuffSize, fBuff, fSize, 1);
    if (ZSTD_isError(cSize)) {
        fprintf(stderr, "error compressing : %s \n", ZSTD_getErrorName(cSize));
        // exit(8);
		free(fBuff);
		free(cBuff);
        return;
    }

 //    fprintf(stderr, "ZSTD_compress\ncBuffSize: %ld\ncSize:%ld\n", cBuffSize, cSize);
 //    fprintf(stderr, "cBuff HEX : ");
	// printHex(cBuff, cSize);
	// fprintf(stderr, "\n");

	// return cBuff;
	char *temp = *data;
	*data = realloc(*data, cSize + 1);
	if(*data == NULL)
	{

		free(cBuff);
		free(fBuff);

		*data = temp;
		return;
	}

	memset(*data, 0, cSize + 1);
	memcpy(*data, cBuff, cSize);
	*datalen = cSize;

	free(cBuff);
	free(fBuff);
}

void OCPMessageCompressor_decompress(void** data, int* datalen)
{
	int cSize = *datalen;
	void* const cBuff = malloc(cSize + !cSize);
	memcpy(cBuff, *data, cSize);

	unsigned long long const rSize = ZSTD_findDecompressedSize(cBuff, (size_t)cSize);
    if (rSize==ZSTD_CONTENTSIZE_ERROR) {
        fprintf(stderr, "it was not compressed by zstd.\n");
        // exit(5);
		free(cBuff);
        return;
    } else if (rSize==ZSTD_CONTENTSIZE_UNKNOWN) {
        fprintf(stderr,
                "original size unknown. Use streaming decompression instead.\n");
        // exit(6);
        return;
    }
    
    void* const rBuff = malloc((size_t)rSize + (size_t)!rSize + 1);
    size_t const dSize = ZSTD_decompress(rBuff, rSize, cBuff, cSize);

    if (dSize != rSize) {
        fprintf(stderr, "error decoding %s \n", ZSTD_getErrorName(dSize));
        // exit(7);
		free(cBuff);
		free(rBuff);
        return;
    }

 //    fprintf(stderr, "ZSTD_decompress\nrSize: %lld\ndSize:%ld\n", rSize, dSize);
	// fprintf(stderr, "rBuff : %s\n", (char*)rBuff);
	// fprintf(stderr, "rBuff HEX : ");
	// printHex(rBuff, dSize);

	char *temp = *data;
	*data = realloc(*data, dSize + 1);
	if(*data == NULL)
	{
		free(rBuff);
		free(cBuff);
		*data = temp;
		return;
	}

	memset(*data, 0, dSize + 1);
	memcpy(*data, rBuff, dSize);
	*datalen = dSize;

	free(rBuff);
	free(cBuff);
}
