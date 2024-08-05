#include "CardDb.h"

#include <cstring>
#include "Common.h"

CardDb* CardDb::instance = nullptr;
namespace
{
void ParseMask( const char* mask, offsetMask_t* rankMask, offsetMask_t* effectMask )
{
	int len = strlen(mask);
	int x = 0, y = 0;
	int centerX = 0, centerY = 0;

	int rankX[ 255 ], rankY[ 255 ];
	int rankCount = 0;

	int effectX[ 255 ], effectY[ 255 ];
	int effectCount = 0;

	for( int ii = 0; ii < len; ++ii, ++x )
	{
		switch( mask[ii] )
		{
			case '\n':
				y++;
				x = -1;
				break;
			case '\t':
			case ' ':
				x--;
				break;
			case '@':
				centerX = x;
				centerY = y;
				break;
			case '1':
				rankX[rankCount] = x;
				rankY[rankCount] = y;
				rankCount++;
				break;
			case '2':
				effectX[effectCount] = x;
				effectY[effectCount] = y;
				effectCount++;
				break;
			case '3':
				rankX[rankCount] = x;
				rankY[rankCount] = y;
				rankCount++;
				effectX[effectCount] = x;
				effectY[effectCount] = y;
				effectCount++;
				break;
		}
	}

	*rankMask = 0;
	for( int ii = 0; ii < rankCount; ++ii )
	{
		int ix = 0;
		ix += (rankY[ ii ] - centerY - OFFSET_MASK_BIAS_H) * OFFSET_MASK_W;
		ix += rankX[ii] - centerX - OFFSET_MASK_BIAS_W;
		*rankMask |= 1u << ix;
	}

	*effectMask = 0;
	for( int ii = 0; ii < effectCount; ++ii )
	{
		int ix = 0;
		ix += (effectY[ ii ] - centerY - OFFSET_MASK_BIAS_H) * OFFSET_MASK_W;
		ix += effectX[ii] - centerX - OFFSET_MASK_BIAS_W;
		*effectMask |= 1u << ix;
	}

	if( *effectMask == 0 )
	{
		// default to self if not specified
		*effectMask = OFFSET_MASK_SELF;
	}
}

struct CardBuilder
{
	Card* card;
	bool isPassive;

	CardBuilder( int idx, Card* card, char* name, char* desc, int score, int rank )
		: card( card )
	{
		card->name = name;
		card->cardIdx = idx;
		card->desc = desc;
		card->score = score;
		card->rank = rank;
		card->rankAdd = 1;
	}

	CardBuilder& RankAdd( int rank )
	{
		card->rankAdd = rank;
		return *this;
	}

	CardBuilder& Passive( FactionFilter faction, int score )
	{
		card->reactType = kEventType_Play;
		card->reactToSelfOnly = true;
		card->reactChange =
			faction == kFactionFilter_Ally ? kChange_AllyScore :
			faction == kFactionFilter_Enemy ? kChange_EnemyScore :
			kChange_AnyFactionScore;
		card->reactTargetFilter = faction;
		card->reactValue = score;
		card->reactIsPassive = true;
		return *this;
	}

	CardBuilder& TallyEffect( TallyType type, FactionFilter faction, int score )
	{
		card->reactType = kEventType_TallyDelta;
		card->reactTallyType = type;
		card->reactChange = kChange_CardScore;
		card->reactValue = score;
		card->reactSourceFilter = faction;
		card->reactTargetMask = OFFSET_MASK_SELF;
		// card->tallyEffectType = type;
		// card->tallyEffectFilter = faction;
		// card->tallyValue = score;
		return *this;
	}

	CardBuilder& ReactToOthersApplyOnSelf( EventType event, FactionFilter sourceFaction, int score )
	{
		card->reactType = event;
		card->reactSourceFilter = sourceFaction;
		card->reactTargetFilter = kFactionFilter_Any;
		card->reactTargetMask = OFFSET_MASK_SELF;
		card->reactToSelfOnly = false;
		card->reactChange = kChange_CardScore;
		card->reactValue = score;
		return *this;
	}

	CardBuilder& ReactToSelfApplyOnOthersOnce( EventType event, FactionFilter targetFaction, int score )
	{
		card->reactType = event;
		card->reactSourceFilter = kFactionFilter_Ally;
		card->reactTargetFilter = targetFaction;
		card->reactToSelfOnly = true;
		card->reactChange = kChange_CardScore;
		card->reactValue = score;
		return *this;
	}

	CardBuilder& Pattern( char* pattern )
	{
		offsetMask_t effectMask;
		ParseMask( pattern, &card->rankAddMask, &effectMask );
		// card->passiveTargetMask = effectMask;
		card->reactTargetMask = effectMask;
		return *this;
	}
};

struct CardDbBuilder
{
	int count = 1; // skip 0, 0 is invalid
	Card* cards = nullptr;
	CardDbBuilder( Card* cards ) : cards( cards ) {}

	CardBuilder Add( char* name, char* desc, int score, int rank )
	{
		Card* card = &cards[count++];
		return CardBuilder( count - 1, card, name, desc, score, rank );
	}
};
}

CardDb::CardDb()
{
CardDbBuilder builder( cards );

builder.Add( "Soldier", "", 1, 1 ).Pattern(R"(
.1.
1@1
.1.
)");

builder.Add( "Grenadier", "", 1, 1 )
.ReactToSelfApplyOnOthersOnce(kEventType_Play, kFactionFilter_Enemy, -3 )
.Pattern("@.2");

builder.Add( "Chocobo & Moogle", "", 1, 1 )
.TallyEffect( kTallyType_Enhanced, kFactionFilter_Ally, 1 )
.Pattern(R"(
1.
@1
1.
)");

builder.Add( "Riot Trooper", "", 3, 2 )
.Pattern(R"(
@1
1.
1.
)");

builder.Add( "Cactuar", "", 1, 1 )
.Passive( kFactionFilter_Ally, 3 )
.Pattern(R"(
@1
1.
.2
)");

builder.Add( "Crystal Crab", "", 1, 1 )
.Passive( kFactionFilter_Ally, 2 )
.Pattern(R"(
.3.
1@1
)");

builder.Add( "Spearhawk", "", 1, 1 )
.Passive( kFactionFilter_Ally, 2 )
.Pattern(R"(
1@1
.3.
)");

}

Card* CardDb::GetByName( char *name)
{
	return instance->GetByNameJS( name );
}
Card* CardDb::GetByIdx( cardIdx_t idx )
{
	return instance->GetByIdxJS( idx );
}
cardIdx_t CardDb::GetIdx( char *name)
{
	return instance->GetIdxJS( name );
}

Card* CardDb::GetByNameJS( char* name )
{
	cardIdx_t idx = GetIdx( name );
	if( idx == CARD_IDX_INVALID ) { return nullptr; }
	return &cards[ idx ];
}
Card* CardDb::GetByIdxJS( cardIdx_t idx )
{
	return &cards[ idx ];
}
cardIdx_t CardDb::GetIdxJS( char* name )
{
		for( int ii = 1; ii < MAX_CARDS; ++ii )
	{
		if( strcmp( name, cards[ ii ].name ) == 0 )
		{
			return ii;
		}
		if( cards[ ii ].cardIdx == 0 )
		{
			break;
		}
	}
	return 0;
}
