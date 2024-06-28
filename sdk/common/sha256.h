/* Sha256.h -- SHA-256 Hash
2016-11-04 : Marc Bevand : A few changes to make it more self-contained
2010-06-11 : Igor Pavlov : Public domain */

#ifndef __CRYPTO_SHA256_H
#define __CRYPTO_SHA256_H

#include "7zTypes.h"

// sha256 작업후의 size
#define SHA256_DIGEST_SIZE 32

typedef struct
{
  UInt32 state[8];
  UInt64 count;
  Byte buffer[64];
} CSha256;

void Sha256_Init(CSha256 *p);
void Sha256_Update(CSha256 *p, const Byte *data, size_t size);
void Sha256_Final(CSha256 *p, Byte *digest);
void Sha256_Onestep(const Byte *data, size_t size, Byte *digest);

void Sha256_Transform(UInt32 *state, const UInt32 *data);
void Sha256_WriteByteBlock(CSha256 *p);

#endif