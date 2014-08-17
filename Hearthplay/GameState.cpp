#include "GameState.h"

#include <memory>
#include <random>
#include <algorithm>

void Player::Heal( uint8_t amt )
{
	m_health = std::min<int8_t>(m_health + amt, GameState::StartingHealth);
}

void Minion::Heal(uint8_t amt)
{
	m_health = std::min<int8_t>(m_health + amt, m_max_health);
}



GameState::GameState()
{
	memset(this, 0, sizeof(GameState));
	m_players[0].m_health = StartingHealth;
	m_players[1].m_health = StartingHealth;

	m_winner = Winner::Undetermined;
}

GameState::GameState(const GameState& other)
{
	memcpy(this, &other, sizeof(GameState));
}

void GameState::ProcessMove(const Move& m)
{
	switch (m.m_type)
	{
	case MoveType::EndTurn: EndTurn(); break;
	case MoveType::PlayCard: PlayCard(m.m_card, m.m_target_packed); break;
	case MoveType::AttackHero: AttackHero(m.m_source_index); break;
	case MoveType::AttackMinion: AttackMinion(m.m_source_index, m.m_target_packed); break;
	}

	UpdatePossibleMoves();
}

void GameState::PlayOutRandomly( std::mt19937& r )
{
	while (m_winner == Winner::Undetermined)
	{
		std::uniform_int_distribution<decltype(m_possible_moves.Num())> move_dist(0, m_possible_moves.Num() - 1);
		auto idx = move_dist(r);
		ProcessMove(m_possible_moves[idx]);
	}
}

void GameState::UpdatePossibleMoves()
{
	m_possible_moves.Clear();
	
	Player& ActivePlayer = m_players[m_active_player_index];
	Player& Opponent = m_players[abs(m_active_player_index - 1)];

	bool opponent_has_taunt = false;
	for (uint8_t i = 0; i < Opponent.m_minions.Num( ); ++i)
	{
		if (Opponent.m_minions[i].HasTaunt( ))
		{
			opponent_has_taunt = true;
			break;
		}
	}

	// Attack each target with each minion
	for (uint8_t i = 0; i < ActivePlayer.m_minions.Num(); ++i)
	{
		if (!ActivePlayer.m_minions[i].CanAttack())
			continue;

		for (uint8_t j = 0; j < Opponent.m_minions.Num( ); ++j)
		{
			if( opponent_has_taunt && !Opponent.m_minions[j].HasTaunt() )
				continue;

			// Attack minion
			m_possible_moves.Add(Move::AttackMinion(i, j));
		}

		if (!opponent_has_taunt)
		{
			// Attack opponent
			m_possible_moves.Add(Move::AttackHero(i));
		}
	}

	// Play each card
	for (uint8_t i = 0; i < ActivePlayer.m_hand.Num(); ++i)
	{
		Card c = ActivePlayer.m_hand[i];
		const CardData* const data = GetCardData(c);
		if (data->m_mana_cost > ActivePlayer.m_mana)
			continue;

		switch (data->m_type)
		{
		case CardType::Minion:
		{
			if (data->HasTargetedBattlecry( ))
			{
				// TODO: enumerate possible targets
				for (uint8_t player_index = 0; player_index < 2; ++player_index)
				{
					for (uint8_t minion_index = 0; minion_index < m_players[player_index].m_minions.Num( ); ++minion_index)
					{
						m_possible_moves.Add(Move::PlayCard(c, Move::TargetMinion(player_index, minion_index)));
					}

					m_possible_moves.Add(Move::PlayCard(c, Move::TargetPlayer(player_index)));
				}
			}
			else
			{
				m_possible_moves.Add(Move::PlayCard(c));
			}
		}
			break;
		case CardType::Spell:
		{
			if (HasFlag(data->m_spell_flags, SpellFlags::NoTarget))
			{
				m_possible_moves.Add(Move::PlayCard(c));
			}
			else
			{
				// TODO: enumerate possible targets
			}
		}
			break;
		}
	}

	// End turn
	m_possible_moves.Add(Move::EndTurn());
}

void GameState::EndTurn()
{
	m_active_player_index = (uint8_t)abs(m_active_player_index - 1);
	Player& ActivePlayer = m_players[m_active_player_index];
	ActivePlayer.DrawOne();
	ActivePlayer.m_max_mana = (uint8_t)std::min(10, ActivePlayer.m_max_mana + 1);
	ActivePlayer.m_mana = ActivePlayer.m_max_mana;
	for (uint8_t i = 0; i < ActivePlayer.m_minions.Num(); ++i)
	{
		Minion& m = ActivePlayer.m_minions[i];
		m.BeginTurn();
	}

	// HACK before fatigue goes in
	if (m_players[0].m_deck.Num() == 0 && m_players[1].m_deck.Num() == 0)
	{
		m_winner = Winner::Draw;
	}
}

void GameState::PlayCard(Card c, uint8_t target_index)
{
	Player& ToAct = m_players[m_active_player_index];
	const CardData* ToPlay = GetCardData(c);
	decltype(ToAct.m_hand.Num( )) idx;
	if (!ToAct.m_hand.Find(c, idx))
		return;

	ToAct.m_hand.RemoveSwap(idx);

	ToAct.m_mana -= ToPlay->m_mana_cost;

	if (ToPlay->m_type == CardType::Minion)
	{
		PlayMinion(c, target_index);
	}
	else
	{
		HandleSpellNoTarget(ToPlay->m_effect, ToPlay->m_effect_param, m_active_player_index);
	}
}

void GameState::AttackHero(uint8_t SourceIndex)
{
	Player& Active = m_players[m_active_player_index];
	Player& Opponent = m_players[abs(m_active_player_index - 1)];

	Minion& Attacker = Active.m_minions[SourceIndex];
	Attacker.Attacked( );

	Opponent.m_health -= Attacker.m_attack;

	if (Opponent.m_health <= 0)
	{
		m_winner = static_cast<Winner>(m_active_player_index);
	}
}

void GameState::CheckDeadMinion(uint8_t player_index, uint8_t minion_index)
{
	Player& owner = m_players[player_index];
	Minion dead_minion = owner.m_minions[minion_index];
	if (dead_minion.m_health <= 0)
	{
		if (dead_minion.m_source_card->m_minion_deathrattle.m_effect != SpellEffect::None)
		{
			HandleDeathrattle(dead_minion.m_source_card->m_minion_deathrattle, player_index);
		}
		owner.m_minions.RemoveAt(minion_index);
	}
}

void GameState::AttackMinion(uint8_t SourceIndex, uint8_t TargetIndex)
{
	Player& Active = m_players[m_active_player_index];
	Player& Opponent = m_players[OppositePlayer(m_active_player_index)];

	Minion& Attacker = Active.m_minions[SourceIndex];
	Minion& Victim = Opponent.m_minions[TargetIndex];

	Attacker.TakeDamage(Victim.m_attack);
	Victim.TakeDamage(Attacker.m_attack);

	Attacker.Attacked( );

	// Handle minion death
	// TODO: Refactor when handling simultaneous minion death
	CheckDeadMinion(m_active_player_index, SourceIndex);
	CheckDeadMinion(OppositePlayer(m_active_player_index), TargetIndex);

	CheckVictory( );
}

void GameState::HandleDeathrattle(Deathrattle deathrattle, uint8_t owner_index)
{
	HandleSpellNoTarget(deathrattle.m_effect, deathrattle.m_param, owner_index);
}

void GameState::HandleSpellNoTarget(SpellEffect effect, uint8_t spell_param, uint8_t owner_index)
{
	switch (effect)
	{
	case SpellEffect::AddMana:
	{
		Player& p = m_players[owner_index];
		p.m_mana += spell_param;
	}
	case SpellEffect::DamageOpponent:
	{
		Player& opponent = m_players[abs(owner_index - 1)];
		opponent.m_health -= spell_param;
		if (opponent.m_health <= 0)
		{
			m_winner = static_cast<Winner>(owner_index);
		}
	}
	}
}

void GameState::HandleSpell(SpellEffect effect, uint8_t spell_param, uint8_t target_packed)
{
	uint8_t target_player, target_minion;
	Move::UnpackTarget(target_packed, target_player, target_minion);
	switch (effect)
	{
	case SpellEffect::DamageCharacter:
	{ 
		if (target_minion == NoMinion)
		{
			// Target hero
			m_players[target_player].m_health -= spell_param;
		}
		else
		{
			m_players[target_player].m_minions[target_minion].m_health -= spell_param;
			CheckDeadMinion(target_player, target_minion);
		}

		CheckVictory( );
	}
		break;
	case SpellEffect::HealCharacter:
		if (target_minion == NoMinion)
		{
			m_players[target_player].Heal(spell_param);
		}
		else
		{
			m_players[target_player].m_minions[target_minion].Heal(spell_param);
		}
		break;
	}
}

void GameState::PlayMinion(Card c, uint8_t target_packed)
{
	const CardData* data = GetCardData(c);

	if (data->HasTargetedBattlecry( ))
	{
		HandleSpell(data->m_minion_battlecry.m_effect, data->m_minion_battlecry.m_param, target_packed);
	}
	else if (data->HasUntargetedBattlecry( ))
	{
		HandleSpellNoTarget(data->m_minion_battlecry.m_effect, data->m_minion_battlecry.m_param, m_active_player_index);
	}

	m_players[m_active_player_index].m_minions.Add({ data });
}

void GameState::CheckVictory( )
{
	if (m_players[0].m_health <= 0 && m_players[1].m_health <= 0)
	{
		m_winner = Winner::Draw;
	}
	else if (m_players[0].m_health <= 0)
	{
		m_winner = Winner::PlayerTwo;
	}
	else if (m_players[1].m_health <= 0)
	{
		m_winner = Winner::PlayerOne;
	}
}

void GameState::PrintMove(const Move& m) const
{
	const Player& Active = m_players[m_active_player_index];
	const Player& Opponent = m_players[abs(m_active_player_index - 1)];

	const CardData* CardToPlay = nullptr;
	switch (m.m_type)
	{
	case MoveType::AttackHero:
		printf("Player %d: Attack hero with %s\n",
			m_active_player_index,
			Active.m_minions[m.m_source_index].GetName()
			);
		break;
	case MoveType::AttackMinion:
		printf("Player %d: Attack %s with %s\n", 
			m_active_player_index, 
			Opponent.m_minions[m.m_target_packed].GetName(), 
			Active.m_minions[m.m_source_index].GetName() 
			);
		break;
	case MoveType::EndTurn:
		printf("Player %d: End turn\n", m_active_player_index);
		break;
	case MoveType::PlayCard:
		CardToPlay = GetCardData(m_players[m_active_player_index].m_hand[m.m_source_index]);
		printf("Player %d: Play %s\n", m_active_player_index, CardToPlay->m_name);
		break;
	}
}

void GameState::PrintState() const
{
	for (unsigned i = 0; i < 2; ++i)
	{
		printf("Player %d - %d - %d/%d\n", i, m_players[i].m_health, m_players[i].m_mana, m_players[i].m_max_mana );
		printf("Hand: ");
		for (uint8_t card = 0; card < m_players[i].m_hand.Num(); ++card)
		{
			const char* format = card == 0 ? "%s" : ", %s";
			printf(format, GetCardData(m_players[i].m_hand[card])->m_name);
		}
		printf("\n");
		printf("Minions: \n");
		for (uint8_t m = 0; m < m_players[i].m_minions.Num(); ++m)
		{
			const Minion& minion = m_players[i].m_minions[m];
			printf("%s %d/%d\n", minion.GetName(), minion.m_attack, minion.m_health);
		}
	}
}

const char* Minion::GetName() const
{
	return m_source_card->m_name;
}