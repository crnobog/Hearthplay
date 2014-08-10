#include "GameState.h"

#include <memory>
#include <random>
#include <algorithm>

GameState::GameState()
{
	memset(this, 0, sizeof(GameState));
	Players[0].m_health = StartingHealth;
	Players[1].m_health = StartingHealth;

	Winner = EWinner::Undetermined;
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
	case MoveType::PlayCard: PlayCard(m.m_card); break;
	case MoveType::AttackHero: AttackHero(m.m_source_index); break;
	case MoveType::AttackMinion: AttackMinion(m.m_source_index, m.m_target_index); break;
	}

	UpdatePossibleMoves();
}

void GameState::PlayOutRandomly( std::mt19937& r )
{
	while (Winner == EWinner::Undetermined)
	{
		std::uniform_int_distribution<decltype(PossibleMoves.Num())> move_dist(0, PossibleMoves.Num() - 1);
		auto idx = move_dist(r);
		ProcessMove(PossibleMoves[idx]);
	}
}

void GameState::UpdatePossibleMoves()
{
	PossibleMoves.Clear();
	
	Player& ActivePlayer = Players[ActivePlayerIndex];
	Player& Opponent = Players[abs(ActivePlayerIndex - 1)];

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
			PossibleMoves.Add(Move::AttackMinion(i, j));
		}

		if (!opponent_has_taunt)
		{
			// Attack opponent
			PossibleMoves.Add(Move::AttackHero(i));
		}
	}

	// Play each card
	for (uint8_t i = 0; i < ActivePlayer.m_hand.Num(); ++i)
	{
		if (GetCardData(ActivePlayer.m_hand[i])->m_mana_cost <= ActivePlayer.m_mana)
		{
			PossibleMoves.Add(Move::PlayCard(ActivePlayer.m_hand[i]));
		}
	}

	// End turn
	PossibleMoves.Add(Move::EndTurn());
}

void GameState::EndTurn()
{
	ActivePlayerIndex = (uint8_t)abs(ActivePlayerIndex - 1);
	Player& ActivePlayer = Players[ActivePlayerIndex];
	ActivePlayer.DrawOne();
	ActivePlayer.m_max_mana = (uint8_t)std::min(10, ActivePlayer.m_max_mana + 1);
	ActivePlayer.m_mana = ActivePlayer.m_max_mana;
	for (uint8_t i = 0; i < ActivePlayer.m_minions.Num(); ++i)
	{
		Minion& m = ActivePlayer.m_minions[i];
		m.BeginTurn();
	}

	// HACK before fatigue goes in
	if (Players[0].m_deck.Num() == 0 && Players[1].m_deck.Num() == 0)
	{
		Winner = EWinner::Draw;
	}
}

void GameState::PlayCard(Card c)
{
	Player& ToAct = Players[ActivePlayerIndex];
	const CardData* ToPlay = GetCardData(c);
	decltype(ToAct.m_hand.Num( )) idx;
	if (!ToAct.m_hand.Find(c, idx))
		return;

	ToAct.m_hand.RemoveSwap(idx);

	ToAct.m_mana -= ToPlay->m_mana_cost;

	if (ToPlay->m_type == CardType::Minion)
	{
		ToAct.m_minions.Add({ ToPlay });
	}
	else
	{
		HandleSpellNoTarget(ToPlay->m_effect, ToPlay->m_effect_param, ActivePlayerIndex);
	}
}

void GameState::AttackHero(uint8_t SourceIndex)
{
	Player& Active = Players[ActivePlayerIndex];
	Player& Opponent = Players[abs(ActivePlayerIndex - 1)];

	Minion& Attacker = Active.m_minions[SourceIndex];
	Attacker.Attacked( );

	Opponent.m_health -= Attacker.m_attack;

	if (Opponent.m_health <= 0)
	{
		Winner = static_cast<EWinner>(ActivePlayerIndex);
	}
}

void GameState::CheckDeadMinion(uint8_t player_index, uint8_t minion_index)
{
	Player& owner = Players[player_index];
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
	Player& Active = Players[ActivePlayerIndex];
	Player& Opponent = Players[OppositePlayer(ActivePlayerIndex)];

	Minion& Attacker = Active.m_minions[SourceIndex];
	Minion& Victim = Opponent.m_minions[TargetIndex];

	Attacker.TakeDamage(Victim.m_attack);
	Victim.TakeDamage(Attacker.m_attack);

	Attacker.Attacked( );

	// Handle minion death
	// TODO: Refactor when handling simultaneous minion death
	CheckDeadMinion(ActivePlayerIndex, SourceIndex);
	CheckDeadMinion(OppositePlayer(ActivePlayerIndex), TargetIndex);
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
		Player& p = Players[owner_index];
		p.m_mana += spell_param;
	}
	case SpellEffect::DamageOpponent:
	{
		Player& opponent = Players[abs(owner_index - 1)];
		opponent.m_health -= spell_param;
		if (opponent.m_health <= 0)
		{
			Winner = static_cast<EWinner>(owner_index);
		}
	}
	}
}

void GameState::PrintMove(const Move& m) const
{
	const Player& Active = Players[ActivePlayerIndex];
	const Player& Opponent = Players[abs(ActivePlayerIndex - 1)];

	const CardData* CardToPlay = nullptr;
	switch (m.m_type)
	{
	case MoveType::AttackHero:
		printf("Player %d: Attack hero with %s\n",
			ActivePlayerIndex,
			Active.m_minions[m.m_source_index].GetName()
			);
		break;
	case MoveType::AttackMinion:
		printf("Player %d: Attack %s with %s\n", 
			ActivePlayerIndex, 
			Opponent.m_minions[m.m_target_index].GetName(), 
			Active.m_minions[m.m_source_index].GetName() 
			);
		break;
	case MoveType::EndTurn:
		printf("Player %d: End turn\n", ActivePlayerIndex);
		break;
	case MoveType::PlayCard:
		CardToPlay = GetCardData(Players[ActivePlayerIndex].m_hand[m.m_source_index]);
		printf("Player %d: Play %s\n", ActivePlayerIndex, CardToPlay->m_name);
		break;
	}
}

void GameState::PrintState() const
{
	for (unsigned i = 0; i < 2; ++i)
	{
		printf("Player %d - %d - %d/%d\n", i, Players[i].m_health, Players[i].m_mana, Players[i].m_max_mana );
		printf("Hand: ");
		for (uint8_t card = 0; card < Players[i].m_hand.Num(); ++card)
		{
			const char* format = card == 0 ? "%s" : ", %s";
			printf(format, GetCardData(Players[i].m_hand[card])->m_name);
		}
		printf("\n");
		printf("Minions: \n");
		for (uint8_t m = 0; m < Players[i].m_minions.Num(); ++m)
		{
			const Minion& minion = Players[i].m_minions[m];
			printf("%s %d/%d\n", minion.GetName(), minion.m_attack, minion.m_health);
		}
	}
}

const char* Minion::GetName() const
{
	return m_source_card->m_name;
}