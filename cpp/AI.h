#pragma once

#include "Board.h"

#define AI_MAX_DEPTH 64

struct AIResult
{
	int8_t handIdx = -1; // -1 to pass
	int8_t playIx = 0;
	f32 aiScores[FACTION_COUNT];
};

struct AI
{
	AIResult SearchBestMove( const Board& board0, int maxDepth );
	u32 originalTurn = 0;

private:
	AIResult SearchBestMoveInternal( const Board& board0, int maxDepth );

	Board boards[ AI_MAX_DEPTH ];
	u32 boardCount = 0;
};