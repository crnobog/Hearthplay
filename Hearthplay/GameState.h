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
	MoveType m_type;
	uint8_t m_source_index;
	uint8_t m_target_index;
	Card m_card;

	Move( )
		: m_type( MoveType::EndTurn )
		, m_source_index( 0 )
		, m_target_index( 0 )
		, m_card( Card::MAX )
	{
	}

	inline bool operator==(const Move& m) const
	{
		return m_type == m.m_type 
			&& m_source_index == m.m_source_index 
			&& m_target_index == m.m_target_index
			&& m_card == m.m_card;
	}

	static Move AttackHero(uint8_t with_minion)
	{
		return Move{ MoveType::AttackHero, with_minion, 0, Card::MAX };
	}

	static Move AttackMinion(uint8_t attacker, uint8_t defender)
	{
		return Move{ MoveType::AttackMinion, attacker, defender, Card::MAX };
	}

	static Move EndTurn( )
	{
		return Move{ MoveType::EndTurn, 0, 0, Card::MAX };
	}

	static Move PlayCard(Card c)
	{
		return Move{ MoveType::PlayCard, 0, 0, c };
	}

protected:
	Move( MoveType type, uint8_t source, uint8_t target, Card c)
		: m_type(type), m_source_index( source ), m_target_index(target), m_card(c)
	{
	}
};

inline bool operator<(const Move& l, const Move& r)
{
	if (l.m_type == r.m_type)
	{
		if (l.m_source_index == r.m_source_index)
		{
			return l.m_target_index < r.m_target_index;
		}
		return l.m_source_index < r.m_source_index;
	}

	return l.m_type < r.m_type;
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
	CannotAttack = 0x40,
	Taunt = 0x80,
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
		: Attack(source_card->m_attack)
		, Health(source_card->m_health)
		, SourceCard(source_card)
		, Flags(MinionFlags::SummonedThisTurn)
	{
		if ((source_card->m_minion_flags & MinionCardFlags::Charge) != MinionCardFlags::None)
		{
			Flags |= MinionFlags::Charge;
		}
		if ((source_card->m_minion_flags & MinionCardFlags::DivineShield) != MinionCardFlags::None)
		{
			Flags |= MinionFlags::DivineShield;
		}
		if ((source_card->m_minion_flags & MinionCardFlags::CannotAttack) != MinionCardFlags::None)
		{
			Flags |= MinionFlags::CannotAttack;
		}
		if ((source_card->m_minion_flags & MinionCardFlags::Taunt) != MinionCardFlags::None)
		{
			Flags |= MinionFlags::Taunt;
		}
		if ((source_card->m_minion_flags & MinionCardFlags::Windfury) != MinionCardFlags::None)
		{
			Flags |= MinionFlags::Windfury;
		}
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

	inline void TakeDamage(uint8_t damage)
	{
		if (HasDivineShield( ))
		{
			RemoveDivineShield( );
		}
		else
		{
			Health -= damage;
		}
	}

	const char* GetName() const;

	inline bool CanAttack( )
	{
		return (Flags & MinionFlags::CannotAttack) == MinionFlags::None
			&& (!SummonedThisTurn() || HasCharge())
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

	inline void RemoveDivineShield( )
	{
		Flags &= ~MinionFlags::DivineShield;
	}

	inline bool HasWindfury( ) const
	{
		return (Flags & MinionFlags::Windfury) != MinionFlags::None;
	}

	inline bool HasTaunt( ) const
	{
		return (Flags & MinionFlags::Taunt) != MinionFlags::None;
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
	static const int StartingHealth = 30;
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

	inline int8_t OppositePlayer(int8_t p_index)
	{
		return p_index == 0 ? 1 : 0;
	}

protected:
	void EndTurn();
	void PlayCard(Card c);
	void AttackHero(uint8_t SourceIndex);
	void AttackMinion(uint8_t SourceIndex, uint8_t TargetIndex);

	void CheckDeadMinion(uint8_t player_index, uint8_t minion_index);
	void HandleDeathrattle(Deathrattle deathrattle, uint8_t owner_index);
	void HandleSpellNoTarget(SpellEffect effect, uint8_t spell_param, uint8_t owner_index);
};