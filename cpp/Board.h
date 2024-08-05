#pragma once

#include "Space.h"
#include "Common.h"

struct BoardLog;
struct ResolveQueue;
struct ResolveStep;
struct JSDeck
{
	int cardCount = 0;
	int cards[ DECK_SIZE ];
	void PushIdx( int idx, int count );
	void PushName( char* name, int count );
};

struct DeckAndHand
{
	cardIdx_t cards[ DECK_SIZE ];
	cardIdx_t GetCard( int idx ) { return cards[idx]; }
	u8 deckSize = 0;
	u8 handSize = 0;
};

struct Board
{
	// temp stuff
	bool silent = false;
	BoardLog* log = nullptr;

	// real stuff
	bool mulliganed[FACTION_COUNT] = FACTION_ARRAY_INIT_0;
	Space spaces[DECK_SIZE];
	DeckAndHand decks[FACTION_COUNT];
	u8 tallies[kTallyType_Count][FACTION_COUNT];
	u8 turn = 0;
	u8 passCount = 0;

	Board();

	void Init( JSDeck* deck0, JSDeck* deck1 );
	void Deal( u8 faction, u8 count, bool randomize );
	void Mulligan( u8 faction, u8 mask );

	bool CanPlay( u8 faction, u8 handIdx, u8 ix ) const;
	void QueueKill( u8 ix, ResolveQueue* queue );
	void QueuePlay( u8 faction, i8 handIdx, u8 ix, ResolveQueue* queue );
	void Pass( u8 faction );

	void TestPlay( u8 faction, cardIdx_t cardIdx, u8 ix );

	void Broadcast( Event* ev, ResolveQueue* queue );
	void Apply( ResolveQueue* queue, bool repeatedly );
	void ApplyStep( ResolveStep* step, ResolveQueue* queue );
	void CloneInto( Board* b );

	i32 GetRowScore( u8 faction, i8 row ) const;
	i8 GetWinner() const;

	// for JS
	DeckAndHand& GetDeckAndHand( int faction );
	Space& GetSpace( int ix );

	int GetSerializeIntCount();
	int Serialize( int* bytes, bool writeToBoard );

	int GetByte( int n );
	void SetByte( int n, int byte );

	bool HasMulliganed( int faction ) { return mulliganed[ faction ]; }

	void DebugPrint();
};

struct SerializedBoard
{
	void Set( char* str );
	void ReadFrom( Board* board );
	void WriteTo( Board* board );
	char base64[ sizeof(Board) * 2 ];
	int base64Len = 0;
};
