#include "GameState.h"

#include <memory>
#include <random>
#include <algorithm>

GameState::GameState()
{
	memset(this, 0, sizeof(GameState));
	Players[0].Health = StartingHealth;
	Players[1].Health = StartingHealth;

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
	for (uint8_t i = 0; i < Opponent.Minions.Num( ); ++i)
	{
		if (Opponent.Minions[i].HasTaunt( ))
		{
			opponent_has_taunt = true;
			break;
		}
	}

	// Attack each target with each minion
	for (uint8_t i = 0; i < ActivePlayer.Minions.Num(); ++i)
	{
		if (!ActivePlayer.Minions[i].CanAttack())
			continue;

		for (uint8_t j = 0; j < Opponent.Minions.Num( ); ++j)
		{
			if( opponent_has_taunt && !Opponent.Minions[j].HasTaunt() )
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
	for (uint8_t i = 0; i < ActivePlayer.Hand.Num(); ++i)
	{
		if (GetCardData(ActivePlayer.Hand[i])->m_mana_cost <= ActivePlayer.Mana)
		{
			PossibleMoves.Add(Move::PlayCard(ActivePlayer.Hand[i]));
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
	ActivePlayer.MaxMana = (uint8_t)std::min(10, ActivePlayer.MaxMana + 1);
	ActivePlayer.Mana = ActivePlayer.MaxMana;
	for (uint8_t i = 0; i < ActivePlayer.Minions.Num(); ++i)
	{
		Minion& m = ActivePlayer.Minions[i];
		m.BeginTurn();
	}

	// HACK before fatigue goes in
	if (Players[0].Deck.Num() == 0 && Players[1].Deck.Num() == 0)
	{
		Winner = EWinner::Draw;
	}
}

void GameState::PlayCard(Card c)
{
	Player& ToAct = Players[ActivePlayerIndex];
	const CardData* ToPlay = GetCardData(c);
	decltype(ToAct.Hand.Num( )) idx;
	if (!ToAct.Hand.Find(c, idx))
		return;

	ToAct.Hand.RemoveSwap(idx);

	ToAct.Mana -= ToPlay->m_mana_cost;

	if (ToPlay->m_type == CardType::Minion)
	{
		ToAct.Minions.Add({ ToPlay });
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

	Minion& Attacker = Active.Minions[SourceIndex];
	Attacker.Attacked( );

	Opponent.Health -= Attacker.Attack;

	if (Opponent.Health <= 0)
	{
		Winner = static_cast<EWinner>(ActivePlayerIndex);
	}
}

void GameState::CheckDeadMinion(uint8_t player_index, uint8_t minion_index)
{
	Player& owner = Players[player_index];
	Minion dead_minion = owner.Minions[minion_index];
	if (dead_minion.Health <= 0)
	{
		if (dead_minion.SourceCard->m_minion_deathrattle.m_effect != SpellEffect::None)
		{
			HandleDeathrattle(dead_minion.SourceCard->m_minion_deathrattle, player_index);
		}
		owner.Minions.RemoveAt(minion_index);
	}
}

void GameState::AttackMinion(uint8_t SourceIndex, uint8_t TargetIndex)
{
	Player& Active = Players[ActivePlayerIndex];
	Player& Opponent = Players[OppositePlayer(ActivePlayerIndex)];

	Minion& Attacker = Active.Minions[SourceIndex];
	Minion& Victim = Opponent.Minions[TargetIndex];

	Attacker.TakeDamage(Victim.Attack);
	Victim.TakeDamage(Attacker.Attack);

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
		p.Mana += spell_param;
	}
	case SpellEffect::DamageOpponent:
	{
		Player& opponent = Players[abs(owner_index - 1)];
		opponent.Health -= spell_param;
		if (opponent.Health <= 0)
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
			Active.Minions[m.m_source_index].GetName()
			);
		break;
	case MoveType::AttackMinion:
		printf("Player %d: Attack %s with %s\n", 
			ActivePlayerIndex, 
			Opponent.Minions[m.m_target_index].GetName(), 
			Active.Minions[m.m_source_index].GetName() 
			);
		break;
	case MoveType::EndTurn:
		printf("Player %d: End turn\n", ActivePlayerIndex);
		break;
	case MoveType::PlayCard:
		CardToPlay = GetCardData(Players[ActivePlayerIndex].Hand[m.m_source_index]);
		printf("Player %d: Play %s\n", ActivePlayerIndex, CardToPlay->m_name);
		break;
	}
}

void GameState::PrintState() const
{
	for (unsigned i = 0; i < 2; ++i)
	{
		printf("Player %d - %d - %d/%d\n", i, Players[i].Health, Players[i].Mana, Players[i].MaxMana );
		printf("Hand: ");
		for (uint8_t card = 0; card < Players[i].Hand.Num(); ++card)
		{
			const char* format = card == 0 ? "%s" : ", %s";
			printf(format, GetCardData(Players[i].Hand[card])->m_name);
		}
		printf("\n");
		printf("Minions: \n");
		for (uint8_t m = 0; m < Players[i].Minions.Num(); ++m)
		{
			const Minion& minion = Players[i].Minions[m];
			printf("%s %d/%d\n", minion.GetName(), minion.Attack, minion.Health);
		}
	}
}

const char* Minion::GetName() const
{
	return SourceCard->m_name;
}