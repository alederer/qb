#pragma once

#include <cassert>
#include <cinttypes>
#include <cstdio>

// #define QB_DEBUG

#ifdef QB_DEBUG
#define QB_ASSERT( x_ ) assert( x_ )
#else
#define QB_ASSERT( x_ )
#endif

#ifdef QB_DEBUG
#define QB_LOGD( ... ) printf( __VA_ARGS__ )
#else
#define QB_LOGD( ... )
#endif

#ifdef QB_VERBOSE
#define QB_LOGV( ... ) printf( __VA_ARGS__ )
#else
#define QB_LOGV( ... )
#endif

template <typename T>
T qbMin( T a, T b )
{
	return a < b ? a : b;
}

template <typename T>
T qbMax( T a, T b )
{
	return a > b ? a : b;
}

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef float f32;
typedef double f64;

#define MAX_CARDS 256
#define BOARD_W 5
#define BOARD_H 3
#define BOARD_SIZE 15
#define DECK_SIZE 15
#define HAND_SIZE 5
#define CARD_IDX_INVALID 0
#define IX(x_, y_) ((BOARD_W * ((y_)) + x_))
#define IX_TO_X(ix_) (((ix_)) % BOARD_W)
#define IX_TO_Y(ix_) (((ix_)) / BOARD_W)
#define IX_TO_XY(ix_) IX_TO_X((ix_)), IX_TO_Y((ix_))
#define FACTION_COUNT 2
#define FACTION_BITS 1
#define MAX_RANK 3
#define RANK_BITS 2
#define FACTION_ARRAY_INIT_0 { 0, 0 }
#define OFFSET_MASK_W 5
#define OFFSET_MASK_H 5
#define OFFSET_MASK_BIAS_W -2
#define OFFSET_MASK_BIAS_H -2
#define OFFSET_MASK_SIZE 25
#define OFFSET_MASK_SELF ((u32)(1u << (2 * OFFSET_MASK_W + 3)))
typedef u16 cardIdx_t;
typedef i16 points_t;
typedef u32 offsetMask_t;
typedef u16 boardMask_t; // 5x3 encoded -- unused?

enum TallyType : u8
{
	kTallyType_All = 0,
	kTallyType_Enhanced,
	kTallyType_Enfeebled,
	kTallyType_Count
};

enum Change : u8
{
	kChange_Rank,
	kChange_CardScore,
	kChange_AllyScore,
	kChange_EnemyScore,
	kChange_AnyFactionScore,
	kChange_Kill,
	kChange_RemoveFromBoard,
	kChange_Play,
	kChange_PlayFromHand,
	kChange_AddCardToHand,
	kChange_Count,
};

inline const char* TallyTypeToString( TallyType type )
{
	switch( type )
	{
		case kTallyType_All: return "kTallyType_All";
		case kTallyType_Enhanced: return "kTallyType_Enhanced";
		case kTallyType_Enfeebled: return "kTallyType_Enfeebled";
		case kTallyType_Count: return "kTallyType_Count";
	}

	return "???";
}

inline const char* ChangeToString( Change change )
{
	switch( change )
	{
		case kChange_Rank:
			return "kChange_Rank";
		case kChange_CardScore:
			return "kChange_CardScore";
		case kChange_AllyScore:
			return "kChange_AllyScore";
		case kChange_EnemyScore:
			return "kChange_EnemyScore";
		case kChange_AnyFactionScore:
			return "kChange_AnyFactionScore";
		case kChange_Kill:
			return "kChange_Kill";
		case kChange_RemoveFromBoard:
			return "kChange_RemoveFromBoard";
		case kChange_Play:
			return "kChange_Play";
		case kChange_PlayFromHand:
			return "kChange_PlayFromHand";
		case kChange_AddCardToHand:
			return "kChange_AddCardToHand";
		case kChange_Count:
			return "kChange_Count";
	}

	return "????";
}

enum FactionFilter : u8
{
	kFactionFilter_Any,
	kFactionFilter_Ally,
	kFactionFilter_Enemy
};

enum EventType : u8
{
	kEventType_None,
	kEventType_Play,
	kEventType_Death,
	kEventType_Enhanced,
	kEventType_Enfeebled,
	kEventType_TallyDelta,
};

inline const char* EventTypeToString( EventType type )
{
	switch( type )
	{
		case kEventType_None: return "kEventType_None";
		case kEventType_Play: return "kEventType_Play";
		case kEventType_Death: return "kEventType_Death";
		case kEventType_Enhanced: return "kEventType_Enhanced";
		case kEventType_Enfeebled: return "kEventType_Enfeebled";
		case kEventType_TallyDelta: return "kEventType_TallyDelta";
	}

	return "???";
}
struct Event
{
	EventType type = kEventType_Play;
	TallyType tallyType = kTallyType_Count;
	i8 tallyDelta = 0;
	u8 faction = 0;
	u8 ix = 0;
};

template <typename T>
void Swap( T* a, T* b )
{
	T temp = *a;
	*a = *b;
	*b = temp;
}

template <typename F>
void ForEachBoardSpot( boardMask_t mask, const F& f )
{
	for( int ii = 0; ii < BOARD_SIZE && mask; ++ii )
	{
		if( mask & 0x1 )
		{
			f( IX_TO_XY( ii ) );
		}

		mask >>= 1;
	}
}

template <typename F>
void ForEachOffsetSpot( offsetMask_t mask, u8 faction, int x, int y, const F& f )
{
	if( mask == OFFSET_MASK_SELF )
	{
		f( x, y );
		return;
	}

	int xSign = faction == 0 ? 1 : -1;
	for( int ii = 0; ii < OFFSET_MASK_SIZE && mask; ++ii )
	{
		if( mask & 0x1 )
		{
			int tx = x + xSign * (OFFSET_MASK_BIAS_W + (ii % OFFSET_MASK_W));
			int ty = y + OFFSET_MASK_BIAS_H + (ii / OFFSET_MASK_W);
			if( tx >= 0 && tx < BOARD_W && ty >= 0 && ty < BOARD_H)
			{
				f( tx, ty );
			}
		}

		mask >>= 1;
	}
}

inline boardMask_t OffsetToBoardMask( offsetMask_t mask, u8 faction, int x, int y )
{
	boardMask_t result = 0;
	ForEachOffsetSpot( mask, faction, x, y, [&]( int bx, int by )
	{
		result |= 1u << IX(bx, by);
	});
	return result;
}

inline void DebugPrintOffsetMask( offsetMask_t mask )
{
	for( int jj = 0; jj < OFFSET_MASK_H; ++jj )
	{
		for( int ii = 0; ii < OFFSET_MASK_W; ++ii )
		{
			printf( "%u", (mask & (1u << IX(ii, jj))) ? 1 : 0 );
		}
		printf( "\n" );
	}
	printf( "\n" );
}

inline void DebugPrintBoardMask( boardMask_t mask )
{
	for( int jj = 0; jj < BOARD_H; ++jj )
	{
		for( int ii = 0; ii < BOARD_W; ++ii )
		{
			printf( "%u", (mask & (1u << IX(ii, jj))) ? 1 : 0 );
		}
		printf( "\n" );
	}
	printf( "\n" );
}
