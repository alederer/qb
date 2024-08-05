#pragma once

#include "Common.h"

void Base64Test();
int EncodeBase64( const u8* input, int inputSize, char* output, int outputSize );
int DecodeBase64( u8* output, int outputSize, const char* input, int inputSize );

int Serialize( bool& x, u8** bytes, bool write );
int Serialize( u8& x, u8** bytes, bool write );
int Serialize( u16& x, u8** bytes, bool write );
int Serialize( u32& x, u8** bytes, bool write );
int Serialize( u64& x, u8** bytes, bool write );
int Serialize( i8& x, u8** bytes, bool write );
int Serialize( i16& x, u8** bytes, bool write );
int Serialize( i32& x, u8** bytes, bool write );
int Serialize( i64& x, u8** bytes, bool write );
int Serialize( f32& x, u8** bytes, bool write );