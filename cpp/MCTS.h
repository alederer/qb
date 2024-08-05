#pragma once

#include "Common.h"
#include "Board.h"
#include "ResolveQueue.h"

struct MCTSSearchResult
{
	bool isMulligan = false;
	i8 handIdx = -1;
	u8 playIx = 0;
};

struct MCTS
{
	MCTS();
	~MCTS();

	MCTSSearchResult SearchBestMove( const Board* board, f32 time );

private:
	struct Node
	{
		Board board;
		Node* children;
		u32 childrenCount;
		u32 wins;
		u32 descents;
		union
		{
			i8 handIdx;
			i8 mulliganMask;
		};
		u8 ix : 4;
		u8 initialized : 1;
		u8 terminal : 1;
		u8 terminalWin : 1;
		u8 isMulligan : 1;
	};

	enum DescendResult { kDescendResult_Win, kDescendResult_Lose };
	DescendResult Descend( const Board& parentBoard, Node* child );
	Node* SelectDescendChild( Node* node );

	Node* Append( bool firstChild, i8 handIdx, u8 playIdx );
	void Initialize( const Board& board, Node* node, bool isRoot );

	static const u32 kMaxChildren = 16 * 15;
	static const u32 kNodesPerPage = 64 * 1024 + kMaxChildren;
	static const u32 kMaxPages = 64 * 1024;
	struct NodePage { u8 nodes[sizeof(Node) * kNodesPerPage]; u32 nodeCount = 0; };
	NodePage* m_pages[ kMaxPages ];
	u32 m_pageCount = 0;
	u32 m_totalNodeCount = 0;

	u32 m_startingFaction = 0;
	ResolveQueue m_queue;

	// Node m_nodes[ MAX_NODES ];
};