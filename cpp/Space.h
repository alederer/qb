#pragma once

#include "Common.h"
#include "CardDb.h"

struct Space
{
	cardIdx_t card = CARD_IDX_INVALID;
	points_t cardAddPoints = 0;
	points_t factionAddPoints[FACTION_COUNT] = FACTION_ARRAY_INIT_0;
	u8 ix = 0;

	union
	{
		u8 bits = 0;

		struct
		{
			u8 faction : FACTION_BITS;
			u8 rank : RANK_BITS;
			u8 reactTriggered : 1;
		};
	};

	bool IsEnhanced() const { return card != CARD_IDX_INVALID && GetAddScore() > 0; }
	bool IsEnfeebled() const { return card != CARD_IDX_INVALID && GetAddScore() < 0; }

	points_t GetFactionAddPoints( int faction )
	{
		return factionAddPoints[ faction ];
	}

	points_t GetAddScore() const
	{
		return cardAddPoints + factionAddPoints[ faction ];
	}

	points_t GetTotalScore() const
	{
		return CardDb::instance->cards[ card ].score + GetAddScore();
	}

	bool CheckFactionFilter( FactionFilter filter, u8 faction_ )
	{
		switch( filter )
		{
			case kFactionFilter_Any:
				return true;
			case kFactionFilter_Ally:
				return faction == faction_;
			case kFactionFilter_Enemy:
				return faction != faction_;
		}

		return false;
	}

	bool CheckFactionFilter( FactionFilter filter, Event* ev )
	{
		return CheckFactionFilter( filter, ev->faction );
	}

	void ResetCardData()
	{
		card = CARD_IDX_INVALID;
		cardAddPoints = 0;
		reactTriggered = 0;
	}
};