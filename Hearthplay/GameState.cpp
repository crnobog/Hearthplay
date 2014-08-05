#include "GameState.h"

#include <memory>
#include <random>
#include <algorithm>

GameState::GameState()
{
	memset(this, 0, sizeof(GameState));
	Players[0].Health = 30;
	Players[1].Health = 30;

	Winner = EWinner::Undetermined;
}

GameState::GameState(const GameState& other)
{
	memcpy(this, &other, sizeof(GameState));
}

void GameState::ProcessMove(const Move& m)
{
	switch (m.Type)
	{
	case MoveType::EndTurn: EndTurn(); break;
	case MoveType::PlayCard: PlayCard(m.SourceIndex, m.TargetIndex); break;
	case MoveType::AttackHero: AttackHero(m.SourceIndex); break;
	case MoveType::AttackMinion: AttackMinion(m.SourceIndex, m.TargetIndex); break;
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

	// Attack each target with each minion
	for (uint8_t i = 0; i < ActivePlayer.Minions.Num(); ++i)
	{
		if (!ActivePlayer.Minions[i].CanAttack())
			continue;

		for (uint8_t j = 0; j < Opponent.Minions.Num( ); ++j)
		{
			// Attack minion
			PossibleMoves.Add(Move::AttackMinion(i, j));
		}

		// Attack opponent
		PossibleMoves.Add( Move::AttackHero(i) );
	}

	// Play each card
	for (uint8_t i = 0; i < ActivePlayer.Hand.Num(); ++i)
	{
		if (GetCardData(ActivePlayer.Hand[i])->ManaCost <= ActivePlayer.Mana)
		{
			PossibleMoves.Add(Move::PlayCard(i));
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

void GameState::PlayCard(uint8_t SourceIndex, uint8_t /*TargetIndex*/)
{
	Player& ToAct = Players[ActivePlayerIndex];
	const CardData* ToPlay = GetCardData(ToAct.Hand[SourceIndex]);
	ToAct.Hand.RemoveSwap(SourceIndex);

	ToAct.Mana -= ToPlay->ManaCost;

	if (ToPlay->Type == CardType::Minion)
	{
		ToAct.Minions.Add({ ToPlay });
	}
	else
	{
		switch (ToPlay->Effect)
		{
		case SpellEffect::AddMana:
			ToAct.Mana += ToPlay->EffectParam;
			break;
		}
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

void GameState::AttackMinion(uint8_t SourceIndex, uint8_t TargetIndex)
{
	Player& Active = Players[ActivePlayerIndex];
	Player& Opponent = Players[abs(ActivePlayerIndex- 1)];

	Minion& Attacker = Active.Minions[SourceIndex];
	Minion& Victim = Opponent.Minions[TargetIndex];

	Attacker.Health -= Victim.Attack;
	Victim.Health -= Attacker.Attack;

	Attacker.Attacked( );

	// Handle minion death
	// TODO: Refactor when handling simultaneous minion death
	if (Active.Minions[SourceIndex].Health <= 0)
	{
		Active.Minions.RemoveAt(SourceIndex);
	}

	if (Opponent.Minions[TargetIndex].Health <= 0)
	{
		Opponent.Minions.RemoveAt(TargetIndex);
	}
}

void GameState::PrintMove(const Move& m) const
{
	const Player& Active = Players[ActivePlayerIndex];
	const Player& Opponent = Players[abs(ActivePlayerIndex - 1)];

	const CardData* CardToPlay = nullptr;
	switch (m.Type)
	{
	case MoveType::AttackHero:
		printf("Player %d: Attack hero with %s\n",
			ActivePlayerIndex,
			Active.Minions[m.SourceIndex].GetName()
			);
		break;
	case MoveType::AttackMinion:
		printf("Player %d: Attack %s with %s\n", 
			ActivePlayerIndex, 
			Opponent.Minions[m.TargetIndex].GetName(), 
			Active.Minions[m.SourceIndex].GetName() 
			);
		break;
	case MoveType::EndTurn:
		printf("Player %d: End turn\n", ActivePlayerIndex);
		break;
	case MoveType::PlayCard:
		CardToPlay = GetCardData(Players[ActivePlayerIndex].Hand[m.SourceIndex]);
		printf("Player %d: Play %s\n", ActivePlayerIndex, CardToPlay->Name);
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
			printf(format, GetCardData(Players[i].Hand[card])->Name);
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
	return SourceCard->Name;
}