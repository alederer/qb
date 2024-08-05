#pragma once

#include "Common.h"

struct Board;
struct Event;
struct ResolveQueue;
struct Space;
enum EventType : u8;

struct Card
{
	cardIdx_t cardIdx = 0;
	i8 score = 0;
	i8 rank = 1;
	char* name = "_NAME_"; // no const for WebIDL-Binder compat
	char* desc = "_DESC_"; // no const for WebIDL-Binder compat

	i8 rankAdd = 0;
	u32 rankAddMask = 0;

	// Change passiveChange = kChange_Count;
	// i8 passiveValue = 0;
	// FactionFilter passiveTargetFilter = kFactionFilter_Any;
	// offsetMask_t passiveTargetMask = 0;

	// TallyType tallyEffectType = kTallyType_Count;
	// FactionFilter tallyEffectFilter = kFactionFilter_Any;
	// i8 tallyValue = 0; // stat is always score

	EventType reactType = kEventType_None;
	TallyType reactTallyType = kTallyType_Count;
	FactionFilter reactSourceFilter = kFactionFilter_Any;
	FactionFilter reactTargetFilter = kFactionFilter_Any;
	offsetMask_t reactTargetMask = 0;
	bool reactToSelfOnly = false;
	Change reactChange = kChange_Count;
	i8 reactValue = 0;
	bool reactOnce = false;
	bool reactIsPassive = false;

	bool CheckFactionFilter( Space* owner, FactionFilter filter, u8 faction );
	void OnPlay( Space* owner, Board* board, ResolveQueue* queue );
	void ReactToEvent( Space* owner, Board* board, ResolveQueue* queue, Event* ev );
	void CalcPotentialScore( const Board& board, u8 faction );
private:
	void OnTallyDelta( Space* owner, Board* board, ResolveQueue* queue, TallyType type, u8 faction, i8 delta );
};
