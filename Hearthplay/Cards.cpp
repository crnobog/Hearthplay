#include "Cards.h"

#include <random>

const CardData AllCards[] = {
	{}, // Unknown,
	{ CardType::Spell, 0, "The Coin", 0, 0 }, // Coin
	{ CardType::Minion, 1, "Murloc Raider", 2, 1 },
	{ CardType::Minion, 2, "Bloodfen Raptor", 3, 2 },
	{ CardType::Minion, 2, "River Crocolisk", 2, 3 },
	{ CardType::Minion, 3, "Magma Rager", 5, 1 },
	{ CardType::Minion, 4, "Chillwind Yeti", 4, 5 },
	{ CardType::Minion, 4, "Oasis Snapjaw", 2, 7 },
	{ CardType::Minion, 6, "Boulderfist Ogre", 6, 7 },
	{ CardType::Minion, 7, "Core Hound", 9, 5 },
	{ CardType::Minion, 7, "War Golem", 7, 7 },
};

const CardData* GetCardData(Card c)
{
	return &AllCards[(unsigned)c];
}