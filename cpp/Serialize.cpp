#include "Serialize.h"

#include <cstring>
#include <cmath>

namespace
{
	struct CharLookup
	{
		u8 values[ 256 ];
		u8 operator[] (int n) { return values[n]; }
		CharLookup( const char* str )
		{
			for( int ii = 0; str[ii] != '\0'; ++ii )
			{
				values[str[ii]] = ii;
			}
		}
	};

	const char* kValueToBase64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
	CharLookup kBase64ToValue( kValueToBase64 );

	int MaskBits( int bits )
	{
		return (1 << bits) - 1;
	}

	int MaskRange( int a, int b )
	{
		return MaskBits(b) & ~MaskBits(a);
	}

	struct BitRange6
	{
		int byteA;
		int bitStartA;
		int bitSizeA;
		int byteB;
		int bitSizeB;
		// int bitB0;
		// int bitB1;
		BitRange6( int bit )
			: byteA( bit / 8 )
			, bitStartA( bit % 8 )
			, bitSizeA( qbMin( 8, bitStartA + 6 ) - bitStartA )
			, byteB( (bit + 6) / 8 )
			, bitSizeB( 6 - bitSizeA )
			// , bitB0( 0 )
			// , bitB1( 6 - (bitA1 - bitA0) )
		{}

		void WriteValue( u8* bytes, int byteCount, int value )
		{
			u8 dummy = 0;
			u8& a = byteA < byteCount ? bytes[byteA] : dummy;
			u8& b = byteB < byteCount ? bytes[byteB] : dummy;
			a |= (value << bitStartA) & 0xFF;
			b |= (value >> bitSizeA) & MaskBits( bitSizeB );
		}

		u8 ReadValue( const u8* bytes, int byteCount )
		{
			int a = byteA < byteCount ? bytes[byteA] : 0;
			int b = byteB < byteCount ? bytes[byteB] : 0;
			int value = (a >> bitStartA) & MaskBits( bitSizeA );
			// printf( "%d %x\n", value, MaskBits(bitSizeA) );
			value |= (b & MaskBits( bitSizeB )) << bitSizeA;
			return value;
		}
	};
}

void Base64Test()
{
	const char* kStr = "AODIpws9d8fhasdufhPSD(HFUpj";
	char encoded[ 1024 ];
	char decoded[ 1024 ];
	int len = EncodeBase64( (u8*)kStr, strlen(kStr) + 1, encoded, sizeof(encoded) );
	printf( "\n%s\n", encoded );
	DecodeBase64( (u8*)decoded, sizeof(decoded), encoded, len );
	QB_ASSERT( strcmp( kStr, decoded ) == 0 );
}

int EncodeBase64(const u8 *input, int inputSize, char *output, int outputSize)
{
#if 1
	int cursor = 0;
	for( int bit = 0; bit < (inputSize + 1) * 8; bit += 6 )
	{
		BitRange6 range( bit );
		int value = range.ReadValue( input, inputSize );
		output[cursor++] = kValueToBase64[value];
	}
	return cursor;
#else
	// base 64 is 6 bits per char
	// 3 bytes is 4 chars
	u32* input4 = (u32*)input;
	int input4SizeCeil = size / 4 + 1;

	int ii = 0;
	for( ; ii < input4SizeCeil - 1; ++ii )
	{
		*(output++) = kValueToBase64[input4[ii] & (0x3F << 0)];
		*(output++) = kValueToBase64[input4[ii] & (0x3F << 6)];
		*(output++) = kValueToBase64[input4[ii] & (0x3F << 12)];
	}	

	// the final one may be incomplete
	*(output++) = 4 * ii + 0 < size ? kValueToBase64[input4[ii] & 0x3F] : '=';
	*(output++) = 4 * ii + 1 < size ? kValueToBase64[input4[ii] & 0xFC0] : '=';
	*(output++) = 4 * ii + 2 < size ? kValueToBase64[input4[ii] & 0x3F000] : '=';
#endif
}

int DecodeBase64(u8* output, int outputSize, const char* input, int inputSize)
{
#if 1
	for( int ii = 0; ii < outputSize; ++ii )
	{
		output[ii] = 0;
	}

	for( int bit = 0; bit < inputSize * 6; bit += 6 )
	{
		BitRange6 range( bit );
		range.WriteValue( output, outputSize, kBase64ToValue[input[bit / 6]] );
	}

	return (inputSize * 8) / 6;
#else
	// base 64 is 6 bits per char
	// int input4SizeCeil = size / 4 + 1;
	int input3SizeCeil = size / 3 + 1;
	u32* output4 = (u32*)output;

	int ii = 0;
	for( ; ii < input3SizeCeil - 1; ++ii )
	{
		*output4 |= kBase64ToValue.values[input[3 * ii + 0]] << 0;
		*output4 |= kBase64ToValue.values[input[3 * ii + 1]] << 6;
		*output4 |= kBase64ToValue.values[input[3 * ii + 2]] << 12;
		output4++;
	}	

	// the final one may be incomplete
	u32 final4 = 0;
	for( int jj = 0; jj < 3; ++jj )
	{
		if( 3 * ii + jj >= size ) { break; }
		char c = input[3 * ii + jj];
		if( c == '=' ) { break; }
		final4 |= kBase64ToValue.values[c] << 0;
	}

	for( int jj = 0; jj < 4 && final4; ++jj )
	{
		output[ ii * 4 + jj ] = final4 & 0x3F;
		final4 >>= 6;
	}
#endif
}

int Serialize(bool &x, u8 **bytes, bool write)
{
	u8* xPtr = (u8*)&x;
	for( int ii = 0; ii < sizeof(x); ++ii )
	{
		if( write )
		{
			**bytes = xPtr[ii];
			(*bytes)++;
		}
		else
		{
			xPtr[ii] = **bytes;
			(*bytes)++;
		}
	}
	return sizeof(x);
}
int Serialize( u8& x, u8** bytes, bool write )
{
	u8* xPtr = (u8*)&x;
	for( int ii = 0; ii < sizeof(x); ++ii )
	{
		if( write )
		{
			**bytes = xPtr[ii];
			(*bytes)++;
		}
		else
		{
			xPtr[ii] = **bytes;
			(*bytes)++;
		}
	}
	return sizeof(x);
}
int Serialize( u16& x, u8** bytes, bool write )
{
	u8* xPtr = (u8*)&x;
	for( int ii = 0; ii < sizeof(x); ++ii )
	{
		if( write )
		{
			**bytes = xPtr[ii];
			(*bytes)++;
		}
		else
		{
			xPtr[ii] = **bytes;
			(*bytes)++;
		}
	}
	return sizeof(x);
}
int Serialize( u32& x, u8** bytes, bool write )
{
	u8* xPtr = (u8*)&x;
	for( int ii = 0; ii < sizeof(x); ++ii )
	{
		if( write )
		{
			**bytes = xPtr[ii];
			(*bytes)++;
		}
		else
		{
			xPtr[ii] = **bytes;
			(*bytes)++;
		}
	}
	return sizeof(x);
}
int Serialize( u64& x, u8** bytes, bool write )
{
	u8* xPtr = (u8*)&x;
	for( int ii = 0; ii < sizeof(x); ++ii )
	{
		if( write )
		{
			**bytes = xPtr[ii];
			(*bytes)++;
		}
		else
		{
			xPtr[ii] = **bytes;
			(*bytes)++;
		}
	}
	return sizeof(x);
}
int Serialize( i8& x, u8** bytes, bool write )
{
	u8* xPtr = (u8*)&x;
	for( int ii = 0; ii < sizeof(x); ++ii )
	{
		if( write )
		{
			**bytes = xPtr[ii];
			(*bytes)++;
		}
		else
		{
			xPtr[ii] = **bytes;
			(*bytes)++;
		}
	}
	return sizeof(x);
}
int Serialize( i16& x, u8** bytes, bool write )
{
	u8* xPtr = (u8*)&x;
	for( int ii = 0; ii < sizeof(x); ++ii )
	{
		if( write )
		{
			**bytes = xPtr[ii];
			(*bytes)++;
		}
		else
		{
			xPtr[ii] = **bytes;
			(*bytes)++;
		}
	}
	return sizeof(x);
}
int Serialize( i32& x, u8** bytes, bool write )
{
	u8* xPtr = (u8*)&x;
	for( int ii = 0; ii < sizeof(x); ++ii )
	{
		if( write )
		{
			**bytes = xPtr[ii];
			(*bytes)++;
		}
		else
		{
			xPtr[ii] = **bytes;
			(*bytes)++;
		}
	}
	return sizeof(x);
}
int Serialize( i64& x, u8** bytes, bool write )
{
	u8* xPtr = (u8*)&x;
	for( int ii = 0; ii < sizeof(x); ++ii )
	{
		if( write )
		{
			**bytes = xPtr[ii];
			(*bytes)++;
		}
		else
		{
			xPtr[ii] = **bytes;
			(*bytes)++;
		}
	}
	return sizeof(x);
}
int Serialize( f32& x, u8** bytes, bool write )
{
	u8* xPtr = (u8*)&x;
	for( int ii = 0; ii < sizeof(x); ++ii )
	{
		if( write )
		{
			**bytes = xPtr[ii];
			(*bytes)++;
		}
		else
		{
			xPtr[ii] = **bytes;
			(*bytes)++;
		}
	}
	return sizeof(x);
}