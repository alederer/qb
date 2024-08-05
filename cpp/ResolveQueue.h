#pragma once

#include "Common.h"
#include "ResolveStep.h"

struct ResolveQueue
{
private:
	ResolveStep steps[2][128];
	u8 stepCount[2] = { 0, 0 };

	ResolveStep* next = steps[1];
	u8* nextCount = &stepCount[1];

public:
	ResolveStep* cur = steps[0];
	u8* curCount = &stepCount[0];

	ResolveStep* Append()
	{
		return &next[(*nextCount)++];
	}

	void Flip()
	{
		Swap( &cur, &next );
		Swap( &curCount, &nextCount );
	}

	bool IsNextEmpty() const
	{
		return *nextCount == 0;
	}
};