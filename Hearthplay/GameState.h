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
	uint8_t m_target_packed;
	Card m_card;

	Move( )
		: m_type( MoveType::EndTurn )
		, m_source_index( 0 )
		, m_target_packed( 0 )
		, m_card( Card::MAX )
	{
	}

	inline bool operator==(const Move& m) const
	{
		return m_type == m.m_type 
			&& m_source_index == m.m_source_index 
			&& m_target_packed == m.m_target_packed
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
		return Move{ MoveType::EndTurn, 0xFF, 0xFF, Card::MAX };
	}

	static Move PlayCard(Card c)
	{
		return Move{ MoveType::PlayCard, 0xFF, 0xFF, c };
	}
	
	static Move PlayCard(Card c, uint8_t target_index)
	{
		return Move{ MoveType::PlayCard, 0xFF, target_index, c };
	}

	// Static methods to make targets for spells
	static uint8_t TargetPlayer(uint8_t player_index)
	{
		return (player_index << 4) | 0xF;
	}

	static uint8_t TargetMinion(uint8_t player_index, uint8_t minion_index)
	{
		return (player_index << 4) | (minion_index & 0xF);
	}

	static void UnpackTarget(uint8_t packed, uint8_t& out_player, uint8_t& out_minion)
	{
		out_player = (packed & 0xF0) >> 4;
		out_minion = packed & 0xF;
	}

protected:
	Move( MoveType type, uint8_t source, uint8_t target, Card c)
		: m_type(type), m_source_index( source ), m_target_packed(target), m_card(c)
	{
	}
};

inline bool operator<(const Move& l, const Move& r)
{
	if (l.m_type == r.m_type)
	{
		if (l.m_source_index == r.m_source_index)
		{
			return l.m_target_packed < r.m_target_packed;
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
	SummonedThisTurn = 0x4
};

IMPLEMENT_FLAGS(MinionFlags, uint8_t);

struct Minion
{
	uint8_t				m_attack;
	int8_t				m_health;
	const CardData*		m_source_card;
	MinionAbilityFlags	m_abilities;
	MinionFlags			m_flags;

	Minion( )
		: m_attack(0)
		, m_health(0)
		, m_source_card(nullptr)
		, m_flags(MinionFlags::None)
	{
		
	}

	Minion(const CardData* source_card)
		: m_attack(source_card->m_attack)
		, m_health(source_card->m_health)
		, m_source_card(source_card)
		, m_flags(MinionFlags::SummonedThisTurn)
		, m_abilities(source_card->m_minion_abilities)
	{
	}

	inline void BeginTurn()
	{
		m_flags &= ~( MinionFlags::AttackedThisTurn 
				|	MinionFlags::WindfuryAttackedThisTurn
				|	MinionFlags::SummonedThisTurn
			);
	}

	inline void Attacked( )
	{
		if (AttackedThisTurn( ))
		{
			m_flags |= MinionFlags::WindfuryAttackedThisTurn;
		}

		m_flags |= MinionFlags::AttackedThisTurn;
	}

	inline void TakeDamage(uint8_t damage)
	{
		if (HasDivineShield( ))
		{
			RemoveDivineShield( );
		}
		else
		{
			m_health -= damage;
		}
	}

	const char* GetName() const;

	inline bool CanAttack( )
	{
		return !HasFlag(m_abilities, MinionAbilityFlags::CannotAttack)
			&& (!SummonedThisTurn() || HasCharge())
			&& ((!WindfuryAttackedThisTurn() && HasWindfury())
			|| !AttackedThisTurn()
			);
	}

	inline bool AttackedThisTurn( ) const
	{
		return HasFlag(m_flags, MinionFlags::AttackedThisTurn);
	}

	inline bool WindfuryAttackedThisTurn( ) const
	{
		return HasFlag(m_flags, MinionFlags::WindfuryAttackedThisTurn);
	}

	inline bool SummonedThisTurn( ) const
	{
		return HasFlag(m_flags, MinionFlags::SummonedThisTurn);
	}

	inline bool HasCharge( ) const
	{
		return HasFlag(m_abilities, MinionAbilityFlags::Charge);
	}

	inline bool HasDivineShield( ) const
	{
		return HasFlag(m_abilities, MinionAbilityFlags::DivineShield);
	}

	inline void RemoveDivineShield( )
	{
		m_abilities &= ~MinionAbilityFlags::DivineShield;
	}

	inline bool HasWindfury( ) const
	{
		return HasFlag(m_abilities, MinionAbilityFlags::Windfury);
	}

	inline bool HasTaunt( ) const
	{
		return HasFlag(m_abilities, MinionAbilityFlags::Taunt);
	}
};

struct Player
{
	int8_t m_health;
	uint8_t m_max_mana;
	uint8_t m_mana;

	FixedVector<Minion, 7, uint8_t> m_minions;
	FixedVector<Card, 10, uint8_t> m_hand;
	FixedVector<Card, 30, uint8_t> m_deck;

	inline void DrawOne()
	{
		if (m_deck.Num())
		{
			m_hand.Add(m_deck.PopBack());
		}
	}
};

struct GameState
{
	static const int StartingHealth = 30;
	static const uint8_t MaxPossibleMoves = 1 + 8 * 8 + 10 * 16;

	Player m_players[2];
	EWinner m_winner;
	int8_t m_active_player_index;
	FixedVector<Move, MaxPossibleMoves, uint16_t> m_possible_moves;

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
	void PlayCard(Card c, uint8_t target_index);
	void AttackHero(uint8_t SourceIndex);
	void AttackMinion(uint8_t SourceIndex, uint8_t TargetIndex);

	void CheckDeadMinion(uint8_t player_index, uint8_t minion_index);
	void HandleDeathrattle(Deathrattle deathrattle, uint8_t owner_index);
	void HandleSpell(SpellEffect effect, uint8_t spell_param, uint8_t target_packed);
	void HandleSpellNoTarget(SpellEffect effect, uint8_t spell_param, uint8_t owner_index);
	void PlayMinion(Card c, uint8_t target_index);

	void CheckVictory( );
};