#include <cstdlib>
#include <cstring>
#include <ctime>
#include "Common.h"
#include "AI.h"
#include "Board.h"
#include "CardDb.h"
#include "MCTS.h"
#include "ResolveQueue.h"
#include "ResolveStep.h"
#include "BoardLog.h"
#include "Serialize.h"

#ifdef WASM
// #include "Space.h"
int main()
{
	Base64Test();
	printf( "WASM LOADED!!\n" );
	srand( time( nullptr ) );
	CardDb::instance = new CardDb();
	return 0;
}
#include "../build/glue.cpp"
#else
int main()
{
	Base64Test();

	// srand( 0 );
	srand( time( nullptr ) );

	QB_LOGD( "hi\n" );
	CardDb::instance = new CardDb();

	// int deck[ DECK_SIZE ];
	// int deckSize = 0;
	// auto Add = [&]( char* name, int count )
	// {
	// 	for( int ii = 0; ii < count && deckSize < DECK_SIZE; ++ii )
	// 	{
	// 		cardIdx_t idx = CardDb::GetIdx(name);
	// 		QB_ASSERT( idx != CARD_IDX_INVALID );
	// 		deck[deckSize++] = idx;
	// 	}
	// };

	JSDeck deck;
	deck.PushName( "Soldier", 2 );
	deck.PushName( "Cactuar", 2 );
	deck.PushName( "Spearhawk", 2 );
	deck.PushName( "Crystal Crab", 2 );
	deck.PushName( "Grenadier", 2 );
	deck.PushName( "Chocobo & Moogle", 2 );
	deck.PushName( "Riot Trooper", 3 );
// #if 0
// 	Add( "Soldier", 15 );
// #else
// 	Add( "Soldier", 2 );
// 	Add( "Cactuar", 2 );
// 	Add( "Spearhawk", 2 );
// 	Add( "Crystal Crab", 2 );
// 	Add( "Grenadier", 2 );
// 	Add( "Riot Trooper", 5 );
// #endif

	// for( int ii = 0; ii < DECK_SIZE; ++ii )
	// {
	// 	deck[ii] = CardDb::GetIdx( ii < 10 ? "Soldier" : "Riot Trooper" );
	// 	QB_ASSERT( deck[ii] > 0 );
	// }

	Board board;
	board.Init( &deck, &deck );

#if 1
	ResolveQueue queue;
	board.TestPlay( 0, CardDb::GetIdx( "Chocobo & Moogle" ), IX( 1, 2 ) );
	board.DebugPrint();
	board.TestPlay( 0, CardDb::GetIdx( "Spearhawk" ), IX( 0, 0 ) );
	board.DebugPrint();
	board.TestPlay( 0, CardDb::GetIdx( "Crystal Crab" ), IX( 0, 1 ) );
	board.DebugPrint();
	board.TestPlay( 0, CardDb::GetIdx( "Chocobo & Moogle" ), IX( 0, 2 ) );
	board.DebugPrint();
	// board.TestPlay( 0, CardDb::GetIdx( "Chocobo & Moogle" ), IX( 0, 2 ) );
	// board.DebugPrint();

	SerializedBoard ser;
	ser.ReadFrom(&board);
	printf( "base64: %s\n", ser.base64 );
	Board board2;
	ser.WriteTo(&board2);
	SerializedBoard ser2;
	ser2.ReadFrom(&board);
	QB_ASSERT(strcmp(ser.base64, ser2.base64) == 0);
	QB_ASSERT(board.spaces[0].card == board2.spaces[0].card);
	QB_ASSERT(board.spaces[1].card == board2.spaces[1].card);
	// QB_ASSERT(board == board2);
#elif 0
	ResolveQueue queue;
	board.TestPlay( 0, CardDb::GetIdx( "Soldier" ), IX( 0, 0 ) );
	board.TestPlay( 0, CardDb::GetIdx( "Chocobo & Moogle" ), IX( 0, 2 ) );
	board.TestPlay( 0, CardDb::GetIdx( "Crystal Crab" ), IX( 0, 1 ) );
	board.DebugPrint();
	board.QueueKill( IX(0, 1), &queue );
	board.Apply( &queue, true );
	board.DebugPrint();
	board.TestPlay( 0, CardDb::GetIdx( "Crystal Crab" ), IX( 0, 1 ) );
	board.DebugPrint();
	board.TestPlay( 0, CardDb::GetIdx( "Chocobo & Moogle" ), IX( 1, 1 ) );
	board.DebugPrint();

#elif 0
	ResolveQueue queue;

	board.TestPlay( 0, CardDb::GetIdx( "Soldier" ), IX( 0, 0 ) );
	board.DebugPrint();
	board.TestPlay( 0, CardDb::GetIdx( "Crystal Crab" ), IX( 0, 1 ) );
	board.DebugPrint();
	board.QueueKill( IX( 0, 1 ), &queue );
	board.Apply( &queue, true );
	board.DebugPrint();
	board.QueueKill( IX( 0, 0 ), &queue );
	board.Apply( &queue, true );
	board.DebugPrint();
	board.TestPlay( 0, CardDb::GetIdx( "Crystal Crab" ), IX( 0, 1 ) );
	board.DebugPrint();
	board.TestPlay( 0, CardDb::GetIdx( "Soldier" ), IX( 0, 0 ) );
	board.DebugPrint();

	// board.TestPlay( 0, CardDb::GetIdx( "Soldier" ), IX( 0, 1 ) );
	// board.spaces[IX(0, 1)].cardAddPoints += 3;
	// board.TestPlay( 1, CardDb::GetIdx( "Grenadier" ), IX( 2, 1 ) );
	// board.DebugPrint();
	// board.QueuePlay( 1, 0, IX(4, 1), &queue );
#elif 1
	MCTS ai;
	ResolveQueue queue;
	for( int ii = 0; ii < 50; ++ii )
	{
		MCTSSearchResult result = ai.SearchBestMove( &board, 2.0f );
		if( result.isMulligan )
		{
			QB_LOGV( "mulligan!" );
			board.Mulligan( board.turn, result.handIdx );
		}
		else
		{
			board.QueuePlay( board.turn, result.handIdx, result.playIx, &queue );
			board.Apply( &queue, true );
			board.Deal( board.turn, 1, true );
		}
		board.turn = 1 - board.turn;

		board.DebugPrint();

		if( board.passCount >= FACTION_COUNT )
		{
			break;
		}
	}
#else
	AI ai;
	ResolveQueue queue;
	for( int ii = 0; ii < 50; ++ii )
	{
		AIResult result = ai.SearchBestMove( board, 4 );
		board.QueuePlay( board.turn, result.handIdx, result.playIx, &queue );
		board.Apply( &queue, true );
		board.Deal( board.turn, 1, true );
		board.turn = 1 - board.turn;

		board.DebugPrint();

		if( board.passCount >= FACTION_COUNT )
		{
			break;
		}
	}
#endif

	// board.QueuePlay( 1, 0, IX(4, 1), &queue );
	// board.Apply( &queue, true );
	// board.QueuePlay( 0, 0, IX(1, 1), &queue );
	// board.Apply( &queue, true );

	return 0;
}
#endif