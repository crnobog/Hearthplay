#pragma once

#include "Cards.h"
#include "FixedVector.h"

#include <cstdint>
#include <random>
#include <utility>

extern std::random_device GlobalRandomDevice;

enum class Winner : int8_t
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

struct PackedTarget
{
	uint8_t m_player : 4;
	uint8_t m_minion : 4;

	inline bool operator==(const PackedTarget& other) const
	{
		return m_player == other.m_player && m_minion == other.m_minion;
	}

	inline bool operator<(const PackedTarget& other) const
	{
		return m_player < other.m_player || m_minion < other.m_minion;
	}
};

struct Move
{
	MoveType		m_type;
	uint8_t			m_source_index;
	PackedTarget	m_target_packed;
	Card			m_card;

	Move( )
		: m_type( MoveType::EndTurn )
		, m_source_index( 0 )
		, m_target_packed({ 0xF, 0xF })
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
		return Move{ MoveType::AttackHero, with_minion, TargetNone(), Card::MAX };
	}

	static Move AttackMinion(uint8_t attacker, uint8_t defender)
	{
		// Technically target player index is wrong here
		return Move{ MoveType::AttackMinion, attacker, TargetMinion(0, defender), Card::MAX };
	}

	static Move EndTurn( )
	{
		return Move{ MoveType::EndTurn, 0xFF, TargetNone(), Card::MAX };
	}

	static Move PlayCard(Card c)
	{
		return Move{ MoveType::PlayCard, 0xFF, TargetNone(), c };
	}
	
	static Move PlayCard(Card c, PackedTarget packed_target)
	{
		return Move{ MoveType::PlayCard, 0xFF, packed_target, c };
	}

	// Static methods to make targets for spells
	static PackedTarget TargetPlayer(uint8_t player_index)
	{
		return{ player_index & 0xF, 0xF };
	}

	static PackedTarget TargetMinion(uint8_t player_index, uint8_t minion_index)
	{
		return{ player_index & 0xF, minion_index & 0xF };
	}

	static PackedTarget TargetNone( )
	{
		return{ 0xF, 0xF };
	}

	static void UnpackTarget(PackedTarget packed, uint8_t& out_player, uint8_t& out_minion)
	{
		out_player = packed.m_player;
		out_minion = packed.m_minion;
	}

protected:
	Move( MoveType type, uint8_t source, PackedTarget target, Card c)
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
	static const uint8_t MaxAuras = 10; // TODO: What is a reasonable max?

	uint8_t				m_attack;
	int8_t				m_health, m_max_health;
	uint8_t				m_spelldamage;
	const CardData*		m_source_card;
	MinionAbilityFlags	m_abilities;
	MinionFlags			m_flags;
	FixedVector<MinionAura, MaxAuras, uint8_t> m_auras;

	Minion( )
		: m_attack(0)
		, m_health(0)
		, m_max_health(0)
		, m_spelldamage(0)
		, m_source_card(nullptr)
		, m_abilities(MinionAbilityFlags::None)
		, m_flags(MinionFlags::None)
	{
		
	}

	Minion(const CardData* source_card)
		: m_attack(source_card->m_attack)
		, m_health(source_card->m_health)
		, m_max_health(source_card->m_health)
		, m_spelldamage(source_card->m_minion_spelldamage)
		, m_source_card(source_card)
		, m_flags(MinionFlags::SummonedThisTurn)
		, m_abilities(source_card->m_minion_abilities)
	{
	}

	inline void ClearAttackFlags()
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
		RemoveStealth( );
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

	inline bool HasStealth( ) const
	{
		return HasFlag(m_abilities, MinionAbilityFlags::Stealth);
	}

	inline void RemoveStealth( )
	{
		m_abilities &= ~MinionAbilityFlags::Stealth;
	}

	inline void AddAbility( MinionAbilityFlags ability )
	{
		m_abilities |= ability;
	}

	inline bool HasDeathrattle( ) const
	{
		return m_source_card->m_minion_deathrattle.m_effect != SpellEffect::None;
	}

	inline void Heal( uint8_t amt );
	void AddAuraEffects(const MinionAura& aura);
	void RemoveAuraEffects(const MinionAura& aura);
	void RemoveEndOfTurnAuras( );
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

	inline void Heal( uint8_t amt );

	uint8_t CalculateSpelldamage( ) const
	{
		uint8_t spelldamage = 0;
		for (uint8_t i = 0; i < m_minions.Num( ); ++i)
		{
			spelldamage += m_minions[i].m_spelldamage;
		}
		return spelldamage;
	}
};

struct PendingSpellEffect
{
	SpellData	m_spell_data;
	uint8_t		m_owner_index;
};

struct GameState
{
	static const uint8_t NoMinion = 0xF;
	static const int8_t StartingHealth = 30;
	static const uint8_t MaxPossibleMoves = 1 + 8 * 8 + 10 * 16;
	static const uint8_t MaxTargets = 7 + 7 + 1 + 1; // Each character

	Player m_players[2];
	Winner m_winner;
	int8_t m_active_player_index;
	FixedVector<Move, MaxPossibleMoves, uint16_t> m_possible_moves;
	FixedVector<PendingSpellEffect, MaxTargets, uint8_t> m_pending_spell_effects; // TODO: Count

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
	void PlayCard(Card c, PackedTarget packed_target);
	void AttackHero(uint8_t SourceIndex);
	void AttackMinion(uint8_t SourceIndex, uint8_t TargetIndex);

	void CheckDeadMinion(uint8_t player_index, uint8_t minion_index);
	void HandlePendingSpellEffect(const SpellData& data, uint8_t owner_index);
	void HandleSpell(const SpellData& spell_data, PackedTarget target_packed, uint8_t owner_index, bool affected_by_spelldamage=false);
	void PlayMinion(Card c, PackedTarget packed_target);

	void CheckVictory( );

	void CheckDeadMinions( );
	void PushDeathrattle(uint8_t owner_idx, const Minion& m);

	template<typename FuncType>
	void ForEachPlayer(FuncType func)
	{
		for (uint8_t player_idx = 0; player_idx < 2; ++player_idx)
		{
			func(m_players[player_idx]);
		}
	}

	template<typename FuncType>
	void ForEachMinion(FuncType func)
	{
		for (uint8_t player_idx = 0; player_idx < 2; ++player_idx)
		{
			for (uint8_t minion_idx = 0; minion_idx < m_players[player_idx].m_minions.Num( ); ++minion_idx)
			{
				func(m_players[player_idx].m_minions[minion_idx]);
			}
		}
	}

	template<typename FuncType>
	void ForEachMinion(uint8_t owner_idx, FuncType func)
	{
		for (uint8_t minion_idx = 0; minion_idx < m_players[owner_idx].m_minions.Num( ); ++minion_idx)
		{
			func(m_players[owner_idx].m_minions[minion_idx]);
		}
	}
};