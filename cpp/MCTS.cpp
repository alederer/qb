#include "MCTS.h"

#include <cstdlib>
#include <chrono>

MCTS::MCTS()
{
	for( NodePage*& page : m_pages )
	{
		page = nullptr;
	}

	// pre-alloc some pages
	for( u32 ii = 0; ii < 32; ++ii )
	{
		m_pages[ii] = new NodePage();
	}

	printf( "alloced\n" );
}

MCTS::~MCTS()
{
	for( u32 ii = 0; ii < m_pageCount; ++ii )
	{
		free( m_pages[ ii ] );
	}
}

MCTSSearchResult MCTS::SearchBestMove( const Board* board, f32 time )
{
	m_startingFaction = board->turn;
	m_totalNodeCount = 0;
	m_pageCount = 0;

	Node* root = Append( true, -1, 0 );
	Initialize( *board, root, true );

	auto timeBegin = std::chrono::system_clock::now();
	u64 microThresh = 1e6f * time;
	u64 descents = 0;

	while( true )
	{
		Descend( *board, root );
		descents++;

		auto now = std::chrono::system_clock::now();
		u64 micro = std::chrono::duration_cast<std::chrono::microseconds>( now - timeBegin ).count();
		if( micro >= microThresh )
		{
			break;
		}
	}

	QB_LOGD( "rollouts: %llu, nodes %u, pages %u\n", descents, m_totalNodeCount, m_pageCount );

	f32 bestWinRatio = -1.0f;
	MCTSSearchResult bestResult;
	for( int ii = 0; ii < root->childrenCount; ++ii )
	{
		f32 ratio = root->children[ii].wins / (f32)root->children[ii].descents;
		if( ratio > bestWinRatio )
		{
			bestWinRatio = ratio;
			bestResult.isMulligan = root->children[ii].isMulligan;
			bestResult.handIdx = root->children[ii].handIdx;
			bestResult.playIx = root->children[ii].ix;
		}
	}

	// don't dealloc any allocated pages

	return bestResult;
}

MCTS::DescendResult MCTS::Descend( const Board& parentBoard, Node* node )
{
	if( !node->initialized )
	{
		Initialize( parentBoard, node, false );
	}

	node->descents++;

	DescendResult result;
	if( node->terminal )
	{
		result = node->terminalWin ? kDescendResult_Win : kDescendResult_Lose;
	}
	else
	{
		result = Descend( node->board, SelectDescendChild( node ) );
	}

	if( result == kDescendResult_Win )
	{
		node->wins++;
	}

	return result;
}

MCTS::Node* MCTS::SelectDescendChild( Node* node )
{
	// @TODO
	QB_ASSERT( node->childrenCount > 0 );
	return &node->children[ rand() % node->childrenCount ];
}

MCTS::Node* MCTS::Append( bool firstChild, i8 handIdx, u8 ix )
{
	NodePage** page = nullptr;
	if( firstChild && (m_pageCount == 0 || m_pages[ m_pageCount - 1 ]->nodeCount + kMaxChildren >= kNodesPerPage) )
	{
		// add page
		QB_ASSERT( m_pageCount < kMaxPages );

		page = &m_pages[ m_pageCount++ ];
		if( !*page )
		{
			*page = new NodePage();
		}
		else
		{
			(*page)->nodeCount = 0;
		}
	}
	else
	{
		page = &m_pages[ m_pageCount - 1 ];
	}


	// QB_ASSERT( m_nodeCount < kMaxNodes );
	// QB_LOGD( "%u\n", m_nodeCount );
	++m_totalNodeCount;

	Node* node = (Node*)&(*page)->nodes[ sizeof(Node) * (*page)->nodeCount++ ]; //&m_nodes[ m_nodeCount++ ];
	node->initialized = false;
	node->handIdx = handIdx;
	node->ix = ix;
	node->isMulligan = false;
	return node;
}

void MCTS::Initialize( const Board& board, Node* node, bool isRoot )
{
	node->initialized = true;
	node->wins = 0;
	node->descents = 0;
	node->terminal = 0;
	node->childrenCount = 0;

	node->board = board;
	if( !isRoot )
	{
		if( node->isMulligan )
		{
			node->board.Mulligan( board.turn, node->mulliganMask );
		}
		else
		{
			node->board.QueuePlay( board.turn, node->handIdx, node->ix, &m_queue );
			node->board.Apply( &m_queue, true );
			node->board.Deal( board.turn, 1, true );
			node->board.turn = 1 - node->board.turn;
		}
	}
	else
	{
		// opponent gets full deck, play against worst case
		node->board.Deal( 1 - node->board.turn, DECK_SIZE, true );
		node->board.silent = true;
		node->isMulligan = false;
	}

	if( node->board.passCount >= FACTION_COUNT )
	{
		node->terminal = 1;
		node->terminalWin = node->board.GetWinner() == m_startingFaction;
	}
	else if( board.turn == m_startingFaction && !board.mulliganed[m_startingFaction] )
	{
		// mulligan options -- don't bother for opponent, they get their full hand
		int maskMax = (1 << HAND_SIZE) - 1;
		for( int ii = 0; ii < maskMax; ++ii )
		{
			Node* child = Append( ii == 0, 0, 0 );
			child->isMulligan = true;
			child->mulliganMask = ii;
			if( ii == 0 ) { node->children = child; } 
			node->childrenCount++;
		}
	}
	else
	{
		node->children = Append( true, -1, 0 );
		node->childrenCount++;

		int handSize = node->board.decks[node->board.turn].handSize;
		for( int ii = handSize * BOARD_SIZE - 1; ii >= 0; --ii )
		{
			int handIdx = ii / BOARD_SIZE;
			int ix = ii % BOARD_SIZE;
			if( node->board.CanPlay(node->board.turn, handIdx, ix ) )
			{
				Node* child = Append( false, handIdx, ix );
				node->childrenCount++;
			}
		}
	}
}