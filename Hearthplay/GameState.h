#pragma once

#include "Cards.h"
#include "FixedVector.h"

#include <cstdint>
#include <random>
#include <utility>

extern std::random_device GlobalRandomDevice;

enum class EWinner : int8_t
{
	Undetermined = -1,
	PlayerOne = 0,
	PlayerTwo = 1,
	Draw = 2
};

enum class MoveType
{
	EndTurn,
	AttackMinion,
	AttackHero,
	PlayCard,
};

struct Move
{
	MoveType Type;
	uint8_t SourceIndex;
	uint8_t TargetIndex;

	Move( )
		: Type( MoveType::EndTurn )
		, SourceIndex( 0 )
		, TargetIndex( 0 )
	{
	}

	inline bool operator==(const Move& m) const
	{
		return Type == m.Type && SourceIndex == m.SourceIndex && TargetIndex == m.TargetIndex;
	}

	static Move AttackHero(uint8_t with_minion)
	{
		return Move{ MoveType::AttackHero, with_minion, 0 };
	}

	static Move AttackMinion(uint8_t attacker, uint8_t defender)
	{
		return Move{ MoveType::AttackMinion, attacker, defender };
	}

	static Move EndTurn( )
	{
		return Move{ MoveType::EndTurn, 0, 0 };
	}

	static Move PlayCard(uint8_t source_index)
	{
		return Move{ MoveType::PlayCard, source_index, 0 };
	}

protected:
	Move( MoveType type, uint8_t source, uint8_t target )
		: Type(type), SourceIndex( source ), TargetIndex(target)
	{
	}
};

inline bool operator<(const Move& l, const Move& r)
{
	if (l.Type == r.Type)
	{
		if (l.SourceIndex == r.SourceIndex)
		{
			return l.TargetIndex < r.TargetIndex;
		}
		return l.SourceIndex < r.SourceIndex;
	}

	return l.Type < r.Type;
}

enum class MinionFlags : uint8_t
{
	None = 0x0,
	AttackedThisTurn = 0x1,
	WindfuryAttackedThisTurn = 0x2,
	SummonedThisTurn = 0x4,
	Charge = 0x8,
	DivineShield = 0x10,
	Windfury = 0x20,
};

inline MinionFlags operator|(MinionFlags l, MinionFlags r)
{
	return (MinionFlags)((uint8_t)l | (uint8_t)r);
}

inline MinionFlags operator&(MinionFlags l, MinionFlags r)
{
	return (MinionFlags)((uint8_t)l & (uint8_t)r);
}

inline MinionFlags operator~(MinionFlags f)
{
	return (MinionFlags)(~(uint8_t)f);
}

inline MinionFlags& operator|=(MinionFlags& l, MinionFlags r)
{
	l = l | r;
	return l;
}

inline MinionFlags& operator&=(MinionFlags& l, MinionFlags r)
{
	l = l & r;
	return l;
}

struct Minion
{
	uint8_t Attack;
	int8_t Health;
	const CardData* SourceCard;
	MinionFlags Flags;

	Minion( )
		: Attack(0)
		, Health(0)
		, SourceCard(nullptr)
		, Flags(MinionFlags::None)
	{
		
	}

	Minion(const CardData* source_card)
		: Attack(source_card->Attack)
		, Health(source_card->Health)
		, SourceCard(source_card)
		, Flags(MinionFlags::SummonedThisTurn)
	{
	}

	Minion(uint8_t attack, int8_t health, const CardData* source_card)
		: Attack(attack)
		, Health(health)
		, SourceCard(source_card)
		, Flags( MinionFlags::SummonedThisTurn )
	{
	}

	inline void BeginTurn()
	{
		Flags &= ~( MinionFlags::AttackedThisTurn 
				|	MinionFlags::WindfuryAttackedThisTurn
				|	MinionFlags::SummonedThisTurn
			);
	}

	inline void Attacked( )
	{
		if (AttackedThisTurn( ))
		{
			Flags |= MinionFlags::WindfuryAttackedThisTurn;
		}

		Flags |= MinionFlags::AttackedThisTurn;
	}

	const char* GetName() const;

	inline bool CanAttack( )
	{
		return (!SummonedThisTurn() || HasCharge())
			&& ((!WindfuryAttackedThisTurn() && HasWindfury())
			|| !AttackedThisTurn()
			);
	}

	inline bool AttackedThisTurn( ) const
	{
		return (Flags & MinionFlags::AttackedThisTurn) != MinionFlags::None;
	}

	inline bool WindfuryAttackedThisTurn( ) const
	{
		return (Flags & MinionFlags::WindfuryAttackedThisTurn) != MinionFlags::None;
	}

	inline bool SummonedThisTurn( ) const
	{
		return (Flags & MinionFlags::SummonedThisTurn) != MinionFlags::None;
	}

	inline bool HasCharge( ) const
	{
		return (Flags & MinionFlags::Charge) != MinionFlags::None;
	}

	inline bool HasDivineShield( ) const
	{
		return (Flags & MinionFlags::DivineShield) != MinionFlags::None;
	}

	inline bool HasWindfury( ) const
	{
		return (Flags & MinionFlags::Windfury) != MinionFlags::None;
	}
};

struct Player
{
	int8_t Health;
	uint8_t MaxMana;
	uint8_t Mana;

	FixedVector<Minion, 7, uint8_t> Minions;
	FixedVector<Card, 10, uint8_t> Hand;
	FixedVector<Card, 30, uint8_t> Deck;

	inline void DrawOne()
	{
		if (Deck.Num())
		{
			Hand.Add(Deck.PopBack());
		}
	}
};

struct GameState
{
	static const uint8_t MaxPossibleMoves = 1 + 8 * 8 + 10 * 16;

	Player Players[2];
	EWinner Winner;
	int8_t ActivePlayerIndex;
	FixedVector<Move, MaxPossibleMoves, uint16_t> PossibleMoves;

	GameState();
	GameState(const GameState& other);

	void ProcessMove(const Move& m);
	void PlayOutRandomly( std::mt19937& r );
	void UpdatePossibleMoves();

	void PrintMove(const Move& m) const;
	void PrintState() const;

protected:
	void EndTurn();
	void PlayCard(uint8_t SourceIndex, uint8_t TargetIndex);
	void AttackHero(uint8_t SourceIndex);
	void AttackMinion(uint8_t SourceIndex, uint8_t TargetIndex);
};