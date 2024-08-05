#include "AI.h"

#include "ResolveQueue.h"

#define BIG_SCORE 1e6f

namespace
{
void EvaluateBoardFinalScore( const Board& board, AIResult* result )
{
	result->aiScores[0] = 0;
	result->aiScores[1] = 0;

	for( int jj = BOARD_H - 1; jj >= 0; --jj )
	{
		int rowScores[FACTION_COUNT] = FACTION_ARRAY_INIT_0;
		int rowWinBonus[FACTION_COUNT] = FACTION_ARRAY_INIT_0;
		for( int ii = BOARD_W - 1; ii >= 0; --ii )
		{
			const Space& space = board.spaces[IX(ii, jj)];
			if( space.card )
			{
				rowScores[space.faction] += space.GetTotalScore();
				// @TODO: evaluate endgame effects
			}
		}

		if( rowScores[0] > rowScores[1] )
		{
			result->aiScores[0] += rowScores[0] + rowWinBonus[0];
		}
		else if( rowScores[1] > rowScores[0] )
		{
			result->aiScores[1] += rowScores[1] + rowWinBonus[1];
		}
	}
}

static const u32 kRankScores[] = { 0, 4, 6, 7 };
void EvaluateBoardAIScore( const Board& board, AIResult* result )
{
	if( board.passCount >= FACTION_COUNT )
	{
		EvaluateBoardFinalScore( board, result );
		if( result->aiScores[0] > result->aiScores[1] )
		{
			result->aiScores[0] = BIG_SCORE;
			result->aiScores[1] = -BIG_SCORE;
		}
		else if( result->aiScores[1] > result->aiScores[0] )
		{
			result->aiScores[0] = -BIG_SCORE;
			result->aiScores[1] = BIG_SCORE;
		}
		else
		{
			result->aiScores[0] = -BIG_SCORE;
			result->aiScores[1] = -BIG_SCORE;
		}
	}
	else
	{
		// score and rank heuristics
		f32 score[FACTION_COUNT] = FACTION_ARRAY_INIT_0;
		f32 rankScore[FACTION_COUNT] = FACTION_ARRAY_INIT_0;
		f32 openSpaces[FACTION_COUNT] = FACTION_ARRAY_INIT_0;
		// f32 potentialScore[FACTION_COUNT] = FACTION_ARRAY_INIT_0;
		for( int ii = 0; ii < BOARD_SIZE; ii++ )
		{
			const Space& space = board.spaces[ii];
			if( space.card )
			{
				score[space.faction] += space.GetTotalScore();
				
			}
			else if( space.rank > 0 )
			{
				openSpaces[space.faction]++;
				rankScore[space.faction] += kRankScores[space.rank];
			}
		}

		for( int ii = 0; ii < FACTION_COUNT; ++ii )
		{
			f32& s = result->aiScores[ii];
			s = 0.0f;
			s += score[ii];
			s += 2 * openSpaces[ii];
			s += 0.25f * rankScore[ii];
		}
	}
}

struct SearchHelper
{
	ResolveQueue queue;
	AI* ai;
	const Board* board0;
	Board* board;
	AIResult best;

	SearchHelper( AI* ai, const Board* board0, Board* board )
		: ai( ai )
		, board0( board0 )
		, board( board )
	{
		best.aiScores[board0->turn] = -1e8f;
		best.aiScores[1 - board0->turn] = 1e8f;
	}

	void Evaluate( int handIdx, int ix, int maxDepth )
	{
		if( handIdx < 0 )
		{
			// passing
			*board = *board0;
			board->silent = true;
			board->passCount++;
		}
		else if( board0->CanPlay( board0->turn, handIdx, ix ) )
		{
			// playing
			*board = *board0;
			board->silent = true;
			board->QueuePlay( board->turn, handIdx, ix, &queue );
			board->Apply( &queue, true );
		}
		else
		{
			// do nothing, don't evaluate
			return;
		}

		AIResult result;
		if( maxDepth > 0 && board->passCount < FACTION_COUNT )
		{
			board->turn = 1 - board->turn;

			// opponent has access to all their cards
			if( board->turn != ai->originalTurn && board->decks[board->turn].deckSize > 0 )
			{
				board->decks[board->turn].handSize += board->decks[board->turn].deckSize;
				board->decks[board->turn].deckSize = 0;
			}
			
			result = ai->SearchBestMove( *board, maxDepth - 1 );
		}
		else
		{
			// evaluate score directly
			EvaluateBoardAIScore( *board, &result );
		}

		f32 delta = result.aiScores[board0->turn] - result.aiScores[1 - board0->turn];
		f32 bestDelta = best.aiScores[board0->turn] - best.aiScores[1 - board0->turn];
		// if( board0->turn == 1 && maxDepth == 2 )
		// {
		// 	QB_LOGD( "%u[%u] %i %u %f %f\n", board0->turn, maxDepth, handIdx, ix, delta, bestDelta );
		// }

		if( delta >= bestDelta  )
		{
			// QB_LOGD( "best %u %i %u %f %f\n", board0->turn, handIdx, ix, result.aiScores[board0->turn] - result.aiScores[1 - board0->turn], best.aiScores[board0->turn] - best.aiScores[1 - board0->turn] );
			best = result;
			best.handIdx = handIdx;
			best.playIx = ix;
		}
	}
};
}

AIResult AI::SearchBestMove( const Board& board0, int maxDepth )
{
	originalTurn = board0.turn;
	return SearchBestMoveInternal( board0, maxDepth );
}

AIResult AI::SearchBestMoveInternal( const Board& board0, int maxDepth )
{
	Board* board = &boards[boardCount];
	boardCount++;

	SearchHelper helper( this, &board0, board );
	{
		int handSize = board0.decks[board0.turn].handSize;
		int iterMax = handSize * BOARD_SIZE;

		helper.Evaluate( -1, 0, maxDepth );
		for( int ii = iterMax - 1; ii >= 0; --ii )
		{
			helper.Evaluate( ii / BOARD_SIZE, ii % BOARD_SIZE, maxDepth );
		}
	}

	boardCount--;

	// QB_LOGD( "best: %u %i %i\n", board0.turn, helper.best.handIdx, helper.best.playIx );
	return helper.best;
}