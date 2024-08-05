#include "Card.h"

#include "Board.h"
#include "ResolveQueue.h"
#include "ResolveStep.h"
#include "Space.h"

#define RESOLVE_LOG( ... ) if( !board->silent ) { QB_LOGV( __VA_ARGS__ ); }

bool Card::CheckFactionFilter( Space* owner, FactionFilter filter, u8 faction )
{
	switch( filter )
	{
		case kFactionFilter_Any:
			return true;
		case kFactionFilter_Ally:
			return faction == owner->faction;
		case kFactionFilter_Enemy:
			return faction != owner->faction;
	}

	return false;
}

void Card::OnPlay( Space* owner, Board* board, ResolveQueue* queue )
{
	RESOLVE_LOG( "  Card::OnPlay\n" );
	if( rankAddMask )
	{
		RESOLVE_LOG( "    \"%s\" append %s %u\n", name, ChangeToString(kChange_Rank), rankAdd );
		ResolveStep* step = queue->Append();
		step->boardMask = OffsetToBoardMask( rankAddMask, owner->faction, IX_TO_XY( owner->ix ) );
		step->change = kChange_Rank;
		step->value = rankAdd;
		step->factionSource = owner->faction;
		step->factionFilter = kFactionFilter_Any;

		// DebugPrintOffsetMask( rankAddMask );
		// DebugPrintBoardMask( step->boardMask );
	}

	// retroactively apply the existing tally
	if( reactType == kEventType_TallyDelta )
	{
		points_t tally = 0;
		u8* tallies = board->tallies[reactTallyType];
		i8 validFaction = -1;
		for( u32 ii = 0; ii < FACTION_COUNT; ++ii )
		{
			if( CheckFactionFilter( owner, reactSourceFilter, ii ) )
			{
				tally += tallies[ii];
				validFaction = ii;
			}
		}

		if( tally )
		{
			OnTallyDelta( owner, board, queue, reactTallyType, validFaction, tally );
		}
	}

	Event ev;
	ev.type = kEventType_Play;
	ev.faction = owner->faction;
	ev.ix = owner->ix;
	board->Broadcast( &ev, queue );
}

void Card::OnTallyDelta( Space* owner, Board* board, ResolveQueue* queue, TallyType type, u8 faction, i8 delta  )
{
	RESOLVE_LOG( "    \"%s\" %s delta %d, append %s\n", name, TallyTypeToString( type ), delta, ChangeToString(kChange_CardScore) );
	ResolveStep* step = queue->Append();
	step->boardMask = 1u << owner->ix;
	step->change = kChange_CardScore;
	step->value = delta * reactValue;
	step->factionSource = owner->faction;
	step->factionFilter = kFactionFilter_Any;
}

void Card::ReactToEvent( Space* owner, Board* board, ResolveQueue* queue, Event* ev )
{
	if( reactType != ev->type && !(reactIsPassive && ev->type == kEventType_Death) ) { return; }
	if( reactToSelfOnly && owner->ix != ev->ix ) { return; }
	if( !CheckFactionFilter( owner, reactSourceFilter, ev->faction ) ) { return; }

	if( ev->type == kEventType_TallyDelta )
	{
		// never include self in tallies
		if( owner->ix == ev->ix ) { return; }
		if( ev->tallyType != reactTallyType ) { return; }
		OnTallyDelta( owner, board, queue, ev->tallyType, ev->faction, ev->tallyDelta );
	}
	else
	{
		RESOLVE_LOG( "    \"%s\" React %s, append %s\n", name, EventTypeToString( ev->type ), ChangeToString(reactChange) );
		ResolveStep* step = queue->Append();
		step->boardMask = OffsetToBoardMask( reactTargetMask, owner->faction, IX_TO_XY( owner->ix ) );
		step->change = reactChange;
		step->value = reactValue;
		step->factionSource = owner->faction;
		step->factionFilter = reactTargetFilter;

		if( reactIsPassive && ev->type == kEventType_Death )
		{
			step->value *= -1;
		}
	}
}
