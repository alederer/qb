#pragma once

#include "Common.h"

struct ResolveStep
{
	boardMask_t boardMask = 0;
	Change change = kChange_Count;
	i32 value = 1;
	u8 factionSource = 0;
	FactionFilter factionFilter = kFactionFilter_Any;
};
