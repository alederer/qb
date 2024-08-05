#include "Board.h"

#include <cstring>
#include <cstdlib>
#include "BoardLog.h"
#include "ResolveQueue.h"
#include "ResolveStep.h"
#include "Serialize.h"

#define RESOLVE_LOG( ... ) if( !silent ) { QB_LOGV( __VA_ARGS__ ); }

void JSDeck::PushIdx( int idx, int count )
{
	for( int ii = 0; ii < count && cardCount < DECK_SIZE; ++ii )
	{
		cards[cardCount++] = idx;
	}
}

void JSDeck::PushName( char* name, int count )
{
	PushIdx( CardDb::GetIdx( name ), count );
}

Board::Board()
{
	for( int ii = 0; ii < BOARD_SIZE; ++ii )
	{
		spaces[ ii ].ix = ii;
	}

	for( int ii = 0; ii < kTallyType_Count; ++ii )
	{
		for( int jj = 0; jj < FACTION_COUNT; ++jj )
		{
			tallies[ii][jj] = 0;
		}
	}
}

void Board::Init( JSDeck* deck0, JSDeck* deck1  )
{
	// QB_ASSERT( deck0Size <= DECK_SIZE );
	// QB_ASSERT( deck1Size <= DECK_SIZE );
	// memcpy( decks[0].cards, deck0, sizeof(cardIdx_t) * deck0Size );
	// memcpy( decks[1].cards, deck1, sizeof(cardIdx_t) * deck1Size );
	for( int ii = 0; ii < deck0->cardCount; ++ii )
	{
		decks[0].cards[ii] = deck0->cards[ii];
	}
	for( int ii = 0; ii < deck1->cardCount; ++ii )
	{
		decks[1].cards[ii] = deck1->cards[ii];
	}
	decks[0].deckSize = deck0->cardCount;
	decks[1].deckSize = deck1->cardCount;

	Deal( 0, 5, true );
	Deal( 1, 5, true );

	for( int jj = 0; jj < BOARD_H; ++jj )
	{
		spaces[IX(0, jj)].rank = 1;
		spaces[IX(0, jj)].faction = 0;

		spaces[IX(BOARD_W - 1, jj)].rank = 1;
		spaces[IX(BOARD_W - 1, jj)].faction = 1;
	}
}

void Board::Deal( u8 faction, u8 count, bool randomize )
{
	DeckAndHand& deck = decks[faction];
	for( int ii = 0; ii < count && deck.deckSize > 0; ++ii )
	{
		if( randomize )
		{
			cardIdx_t* deckStart = &deck.cards[deck.handSize];
			Swap( deckStart, deckStart + (rand() % deck.deckSize) );
		}

		deck.handSize++;
		deck.deckSize--;
	}
}

void Board::Mulligan( u8 faction, u8 mask )
{
	RESOLVE_LOG( "Mulligan %u: %i %i %i %i %i\n", faction, mask & 0x1, mask & 0x2, mask & 0x4, mask & 0x8, mask & 0x10 );
	// i8 mulliganIdx[5] = { a, b, c, d, e };

	cardIdx_t* cards = decks[faction].cards;
	i8 handSize = decks[faction].handSize;
	i8 candidatesStart = handSize;
	i8 candidatesCount = decks[faction].deckSize;

	for( u32 ii = 0; ii < 5 && candidatesCount > 0; ++ii )
	{
		if( (mask & (1u << ii)) == 0 ) { continue; }
		i8 oldCardIdx = ii;
		if( oldCardIdx >= decks[faction].handSize ) { continue; }
		i8 newCardIdx = candidatesStart + rand() % candidatesCount;
		i8 endIdx = candidatesStart + candidatesCount - 1;

		// swap old card to end of deck
		cardIdx_t oldCard = cards[oldCardIdx];
		cardIdx_t newCard = cards[newCardIdx];
		cardIdx_t cardInOldCardsNewSlot = cards[endIdx];
		cards[oldCardIdx] = newCard; // new card goes in hand!
		cards[newCardIdx] = cardInOldCardsNewSlot; // fill new card's slot with another candidate
		cards[endIdx] = oldCard; // old card goes to the end and gets excluded

		candidatesCount--;
	}

	mulliganed[faction] = true;
}


void Board::CloneInto( Board* b )
{
	*b = *this;
}

i32 Board::GetRowScore(u8 faction, i8 row) const
{
	int score = 0;
	for( int ii = 0; ii < BOARD_W; ++ii )
	{
		const Space& space = spaces[IX(ii, row)];
		if( space.card && space.faction == faction )
		{
			score += space.GetTotalScore();
		}
	}
	return score;
}

i8 Board::GetWinner() const
{
	int totals[FACTION_COUNT] = FACTION_ARRAY_INIT_0;
	for( int jj = BOARD_H - 1; jj >= 0; --jj )
	{
		int maxScore = -1000;
		int maxScoreFaction = -1;
		for( int ii = FACTION_COUNT - 1; ii >= 0; --ii )
		{
			int score = GetRowScore( ii, jj );
			if( score == maxScore ) { maxScoreFaction = -1; }
			else if( score > maxScore ) { maxScore = score; maxScoreFaction = ii; }
		}
		if( maxScoreFaction >= 0 )
		{
			totals[maxScoreFaction] += maxScore;
		}
	}

	if( totals[0] > totals[1] ) { return 0; }
	if( totals[1] > totals[0] ) { return 1; }
	return -1;
}

DeckAndHand& Board::GetDeckAndHand(int faction)
{
	return decks[faction];
}

Space& Board::GetSpace(int ix)
{
	return spaces[ix];
}

int Board::GetSerializeIntCount()
{
	return sizeof(Board)/sizeof(int);
}

int Board::Serialize(int* ints, bool writeToBoard)
{
	int* boardPtr = (int*)this;
	int* intsPtr = ints;

	Board boardCopy;
	if( !writeToBoard )
	{
		boardCopy = *this;
		boardCopy.log = nullptr;
		boardCopy.silent = false;
		boardPtr = (int*)&boardCopy;
	}

	int*& writePtr = writeToBoard ? boardPtr : intsPtr;
	int*& readPtr = writeToBoard ? intsPtr : boardPtr;
	for( int ii = 0; ii < sizeof(Board)/sizeof(int); ++ii )
	{
		writePtr[ii] = readPtr[ii];
	}

	return sizeof(Board);
}

int Board::GetByte(int n)
{
	u8* buffer = (u8*)this;
	u8* ptr = buffer + n;
	if( ptr >= (u8*)&silent && ptr < (u8*)&silent + sizeof(silent) ) { return 0; }
	if( ptr >= (u8*)&log && ptr < (u8*)&log + sizeof(log) ) { return 0; }
	return *ptr;
}

void Board::SetByte(int n, int byte)
{
	u8* buffer = (u8*)this;
	buffer[n] = byte;
}

bool Board::CanPlay( u8 faction, u8 handIdx, u8 ix ) const
{
	const Space& space = spaces[ix];
	if( space.faction != faction ) { return false; }
	if( space.card ) { return false; }
	
	Card* card = CardDb::GetByIdx(decks[faction].cards[handIdx]);
	if( space.rank < card->rank ) { return false; }

	return true;
}

void Board::QueueKill(u8 ix, ResolveQueue *queue)
{
	QB_ASSERT( spaces[ix].card != CARD_IDX_INVALID );

	// @NOTE: old card's death fully resolves before new card gets played
	Space* space = &spaces[ix];
	if( space->card != CARD_IDX_INVALID )
	{
		ResolveStep* step = queue->Append();
		step->boardMask = 1u << ix;
		step->change = kChange_Kill;
		step->value = 0;
		step->factionSource = space->faction;
		step->factionFilter = kFactionFilter_Any;
	}
}

void Board::QueuePlay( u8 faction, i8 handIdx, u8 ix, ResolveQueue* queue )
{
	if( handIdx < 0 )
	{
		Pass( faction );
		return;
	}

	QB_ASSERT( handIdx < decks[faction].handSize );
	QB_ASSERT( spaces[ix].card == CARD_IDX_INVALID );

	ResolveStep* step = queue->Append();
	step->boardMask = 1u << ix;
	step->change = kChange_PlayFromHand;
	step->value = handIdx;
	step->factionSource = faction;
	step->factionFilter = kFactionFilter_Any;
}

void Board::Pass( u8 faction )
{
	passCount++;
}

void Board::TestPlay(u8 faction, cardIdx_t cardIdx, u8 ix)
{
	QB_ASSERT( spaces[ix].card == CARD_IDX_INVALID );
	spaces[ix].faction = faction;

	ResolveQueue queue;

	ResolveStep* step = queue.Append();
	step->boardMask = 1u << ix;
	step->change = kChange_Play;
	step->value = cardIdx;
	step->factionSource = faction;
	step->factionFilter = kFactionFilter_Any;

	Apply( &queue, true );
}

void Board::Broadcast( Event* ev, ResolveQueue* queue )
{
	if( ev->type == kEventType_TallyDelta )
	{
		RESOLVE_LOG( "  Broadcast %s (%s %i)\n", EventTypeToString( ev->type ), TallyTypeToString(ev->tallyType), ev->tallyDelta );
	}
	else
	{
		RESOLVE_LOG( "  Broadcast %s\n", EventTypeToString( ev->type ) );
	}

	for( int ii = BOARD_SIZE - 1; ii >= 0; --ii )
	{
		Space* space = &spaces[ii];
		if( space->card == CARD_IDX_INVALID) { continue; }
		CardDb::GetByIdx(space->card)->ReactToEvent( space, this, queue, ev );
	}

	if( ev->type == kEventType_TallyDelta )
	{
		tallies[ev->tallyType][ev->faction] += ev->tallyDelta;
	}
}

void Board::Apply( ResolveQueue* queue, bool repeatedly )
{
	while( !queue->IsNextEmpty() )
	{
		queue->Flip();
		RESOLVE_LOG( "--------- RESOLVE ---------\n" );
		for( int ii = 0; ii < *queue->curCount; ++ii )
		{
			ApplyStep( &queue->cur[ii], queue );
		}
		(*queue->curCount) = 0;

		if( !repeatedly ) { break; }
	}

	RESOLVE_LOG( "------- END RESOLVE -------\n" );
	QB_ASSERT( !repeatedly || (*queue->curCount == 0 && queue->IsNextEmpty()) );
}

void Board::ApplyStep( ResolveStep* step, ResolveQueue* queue )
{
	RESOLVE_LOG( "Apply %s %i\n", ChangeToString( step->change ), step->value );

	if( log ) { log->resolveStepIdx++; }

	ForEachBoardSpot( step->boardMask, [&]( int x, int y )
	{
		Space* space = &spaces[IX( x, y )];
		if( !space->CheckFactionFilter( step->factionFilter, step->factionSource ) )
		{
			return;
		}

		int baseScore = CardDb::GetByIdx( space->card )->score;
		int prevAddScore = space->GetAddScore();
		bool wasEnhanced = space->IsEnhanced();
		bool wasEnfeebled = space->IsEnfeebled();
		bool removed = false;
		bool killed = false;
		bool played = false;

		auto AddLog = [&]() {
			if( log )
			{
				BoardLogEvent* ble = log->Append();
				ble->resolveStepIdx = log->resolveStepIdx;
				ble->ix = IX(x, y);
				ble->change = step->change;
				ble->faction = space->faction;
				ble->value = step->value;
				return ble;
			}

			return (BoardLogEvent*)nullptr;
		};

		switch( step->change )
		{
		case kChange_Rank:
			{
				if( space->card ) { return; }

				int rankPrev = space->rank;
				if( space->faction == step->factionSource && space->rank < MAX_RANK )
				{
					space->rank += step->value;
					space->rank = qbMin( space->rank, (u8)MAX_RANK );

					if( BoardLogEvent* ble = AddLog() )
					{
						ble->value = space->rank - rankPrev;
					}
				}
				else if( space->faction != step->factionSource )
				{
					space->faction = step->factionSource;
					if( space->rank == 0 ) { space->rank++; }

					if( BoardLogEvent* ble = AddLog() )
					{
						ble->value = space->rank - rankPrev;
					}
				}
			}
			break;
		case kChange_CardScore:
			{
				if( space->card )
				{
					space->cardAddPoints += step->value;
					AddLog();
				}
			}
			break;
		case kChange_AllyScore:
			{
				space->factionAddPoints[step->factionSource] += step->value;
				AddLog();
			}
			break;
		case kChange_EnemyScore:
			{
				space->factionAddPoints[1 - step->factionSource] += step->value;
				AddLog();
			}
			break;
		case kChange_AnyFactionScore:
			{
				space->factionAddPoints[step->factionSource] += step->value;
				space->factionAddPoints[1 - step->factionSource] += step->value;
				AddLog();
			}
			break;
		case kChange_Kill:
			{
				killed = true;
				AddLog();
			}
			break;
		case kChange_RemoveFromBoard:
			{
				removed = space->card != CARD_IDX_INVALID;
				space->ResetCardData();
				AddLog();
			}
			break;
		case kChange_Play:
			{
				space->card = step->value;
				Card* card = CardDb::GetByIdx(space->card);
				card->OnPlay( space, this, queue );
				AddLog();

				played = true;
			}
			break;
		case kChange_PlayFromHand:
			{
				DeckAndHand* deck = &decks[step->factionSource];
				cardIdx_t* cards = deck->cards;
				cardIdx_t cardIdx = cards[step->value];
				memmove( cards + step->value, cards + step->value + 1, deck->handSize + deck->deckSize - step->value - 1 );
				deck->handSize--;
				
				space->card = cardIdx;
				Card* card = CardDb::GetByIdx(space->card);
				card->OnPlay( space, this, queue );
				AddLog();

				played = true;
			}
			break;
		case kChange_AddCardToHand:
			{
				AddLog();
			};
		default:
			QB_ASSERT( false );
		}

		Event ev;
		ev.faction = space->faction;
		ev.ix = space->ix;

		// enhanced tally/event
		if( space->IsEnhanced() != wasEnhanced )
		{
			ev.type = kEventType_TallyDelta;
			ev.tallyType = kTallyType_Enhanced;
			ev.tallyDelta = space->IsEnhanced() ? 1 : -1;
			Broadcast( &ev, queue );

			if( space->IsEnhanced() )
			{
				ev.type = kEventType_Enhanced;
				Broadcast( &ev, queue );
			}
		}

		// enfeebled tally/event
		if( space->IsEnfeebled() && !wasEnfeebled )
		{
			ev.type = kEventType_TallyDelta;
			ev.tallyType = kTallyType_Enfeebled;
			ev.tallyDelta = space->IsEnfeebled() ? 1 : -1;
			Broadcast( &ev, queue );

			if( space->IsEnfeebled() )
			{
				ev.type = kEventType_Enfeebled;
				Broadcast( &ev, queue );
			}
		}
	
		// death event
		if( space->card && space->GetTotalScore() <= 0 )
		{
			ev.type = kEventType_Death;
			Broadcast( &ev, queue );

			// after death effects are processed, remove from board
			RESOLVE_LOG( "  Append %s\n", ChangeToString(kChange_RemoveFromBoard) );
			ResolveStep* step = queue->Append();
			step->boardMask = 1u << space->ix;
			step->change = kChange_RemoveFromBoard;
			step->value = 0;
			step->factionSource = space->faction;
			step->factionFilter = kFactionFilter_Any;
		}
	});
}

#define PRINT_COLOR
void Board::DebugPrint()
{
	static const u32 kBufSize = 1024;
	char rows[BOARD_H][kBufSize];
	u32 lens[BOARD_H];
	u32 visualLens[BOARD_H];

#ifdef PRINT_COLOR
	// const char* bg[3] = { "\e[44m", "\e[41m", "\e[40m" };
	// const char* bg[3] = {"\e[0;34m", "\e[0;31m", "\e[0;37m" };
	const char* bg[3] = {"\e[0;34m", "\e[0;31m", "\e[0m" };
#else
	const char* bg[3] = { "", "", "" };
#endif

	auto Color = [&]( int jj, int faction )
	{
		lens[jj] += snprintf( rows[jj] + lens[jj], kBufSize - lens[jj],
					"%s", bg[faction] );
	};

	auto EvenUp = [&]( int extra )
	{
		u32 maxVisualLen = 0;
		for( int jj = 0; jj < BOARD_H; ++jj )
		{
			maxVisualLen = qbMax( maxVisualLen, visualLens[jj] );
		}
		for( int jj = 0; jj < BOARD_H; ++jj )
		{
			int len = snprintf( rows[jj] + lens[jj], kBufSize - lens[jj],
				"%*s", extra + maxVisualLen - visualLens[jj], "" );
			lens[jj] += len;
			visualLens[jj] += len;
		}
	};

	for( int ii = 0; ii < BOARD_H; ++ii )
	{
		memset(rows[ii], 0, sizeof(rows[ii]) );
		lens[ii] = 0;
		visualLens[ii] = 0;
	}

	for( int jj = 0; jj < BOARD_H; ++jj )
	{
		int n = GetRowScore( 0, jj ) > GetRowScore( 1, jj ) ? 0 : 2;
		Color( jj, n );
		lens[jj] += snprintf( rows[jj] + lens[jj], kBufSize - lens[jj],
			"%u", GetRowScore( 0, jj ) );
	}

	EvenUp( 2 );

	for( int ii = 0; ii < BOARD_W; ++ii )
	{
		for( int jj = 0; jj < BOARD_H; ++jj )
		{
			Space& space = spaces[IX( ii, jj )];
			if( space.card != CARD_IDX_INVALID)
			{
				Color( jj, space.faction );

				const char* name = CardDb::GetByIdx(space.card)->name;
				int len = snprintf( rows[jj] + lens[jj], kBufSize - lens[jj],
					"%.*s(%d)", qbMin<u32>( strlen( name ), 7 ), name, space.GetTotalScore() );

				lens[jj] += len;
				visualLens[jj] += len;
			}
			else
			{	
				Color( jj, space.rank ? space.faction : 2 );

				int len = snprintf( rows[jj] + lens[jj], kBufSize - lens[jj],
					"[%u]", space.rank );

				lens[jj] += len;
				visualLens[jj] += len;
			}
		}

		EvenUp( 2 );
	}

	for( int jj = 0; jj < BOARD_H; ++jj )
	{
		int n = GetRowScore( 1, jj ) > GetRowScore( 0, jj ) ? 1 : 2;
		Color( jj, n );
		lens[jj] += snprintf( rows[jj] + lens[jj], kBufSize - lens[jj],
			"%u", GetRowScore( 1, jj ) );
	}

	Color( 2, 2 );

	for( int jj = 0; jj < BOARD_H; ++jj )
	{
		QB_LOGD( "%s\n", rows[jj] );
	}
	QB_LOGD( "\n" );
}

void SerializedBoard::Set(char *str)
{
	base64Len = strlen(str);
	strncpy( base64, str, base64Len );
}

void SerializedBoard::ReadFrom(Board *board)
{
	base64Len = EncodeBase64( (u8*)board, sizeof(Board), base64, sizeof(base64) );
}

void SerializedBoard::WriteTo(Board *board)
{
	DecodeBase64( (u8*)board, sizeof(Board), base64, base64Len );
	board->silent = false;
	board->log = nullptr;
}
