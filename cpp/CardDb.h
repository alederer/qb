#pragma once

#include "Card.h"
struct CardDb
{
	CardDb();

	static Card* GetByName( char* name );
	static Card* GetByIdx( cardIdx_t idx );
	static cardIdx_t GetIdx( char* name );
	static CardDb* instance;

	Card* GetByNameJS( char* name );
	Card* GetByIdxJS( cardIdx_t idx );
	cardIdx_t GetIdxJS( char* name );

	Card cards[ MAX_CARDS ];
};