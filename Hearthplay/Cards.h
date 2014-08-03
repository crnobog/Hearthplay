#pragma once

#include <cstdint>

enum class Card
{
	Unknown,
	Coin,
	MurlocRaider,
	BloodfenRaptor,
	RiverCrocolisk,
	MagmaRager,
	ChillwindYeti,
	OasisSnapjaw,
	BoulderfistOgre,
	CoreHound,
	WarGolem,

	MAX
};

enum class CardType : uint8_t
{
	Minion,
	Spell,
};

enum class SpellEffect
{
	AddMana, // Coin and Innervate
};

struct CardData
{
	CardType Type;
	uint8_t ManaCost;
	const char* Name;

	uint8_t Attack;
	uint8_t Health;

	SpellEffect Effect;
	uint8_t EffectParam;
};

extern const CardData AllCards[];

const CardData* GetCardData(Card c);
