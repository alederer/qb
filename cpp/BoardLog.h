#pragma once

#include "Common.h"

struct BoardLogEvent
{
	u32 resolveStepIdx;
	Change change;
	u8 ix;
	u8 faction;
	i32 value;
};

struct BoardLog
{
	BoardLogEvent* Append() { QB_ASSERT( eventCount < kMaxEvents ); return &events[eventCount++]; }
	BoardLogEvent& GetEvent( u32 i ) { return events[i]; }

	static const u32 kMaxEvents = 512;
	BoardLogEvent events[ kMaxEvents ];
	u32 eventCount = 0;
	u32 resolveStepIdx = 0;
};