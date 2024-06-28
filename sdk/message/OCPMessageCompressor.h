#include "OCPMessage.h"

int OCPMessageCompressor_needCompress(char* msgCode, int datalen, int minCompressionSize);
int OCPMessageCompressor_needDecompress(char* encType);
void OCPMessageCompressor_convert_encType(char encType[], int enclen, int compressionType);
void OCPMessageCompressor_compress(void** data, int* datalen);
void OCPMessageCompressor_decompress(void** data, int* datalen);