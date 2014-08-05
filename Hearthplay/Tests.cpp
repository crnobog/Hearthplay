#include "Tests.h"
#include "GameState.h"
#include "Cards.h"

bool AnyFailed = false;


Minion& AddMinion(GameState& g, uint8_t player, Card c)
{
	const CardData* data = GetCardData(c);
	Minion m{ data };
	auto idx = g.Players[player].Minions.Add(m);
	return g.Players[player].Minions[idx];
}

Minion& AddMinionReadyToAttack(GameState& g, uint8_t player, Card c)
{
	Minion& m = AddMinion(g, player, c);
	m.Flags &= ~(MinionFlags::AttackedThisTurn | MinionFlags::SummonedThisTurn);
	return m;
}

bool CheckAndProcessMove(GameState& g, Move m)
{
	if (!g.PossibleMoves.Contains(m))
	{
		return false;
	}
	g.ProcessMove(m);
	return true;
}

void Test_PlayerOne_Win_MinionAttack()
{
	GameState g;
	g.ActivePlayerIndex = 0;
	g.Players[1].Health = 2;

	AddMinionReadyToAttack(g, 0, Card::MurlocRaider);

	g.UpdatePossibleMoves();

	if (!CheckAndProcessMove(g, Move::AttackHero(0)))
	{
		printf("%s failed\n", __FUNCTION__);
		AnyFailed = true;
		return;
	}

	if (g.Winner != EWinner::PlayerOne)
	{
		printf("%s failed\n", __FUNCTION__ );
		AnyFailed = true;
		return;
	}
}

void Test_PlayerTwo_Win_MinionAttack()
{
	GameState g;
	g.ActivePlayerIndex = 1;
	g.Players[0].Health = 2;

	AddMinionReadyToAttack(g, 1, Card::MurlocRaider);

	g.UpdatePossibleMoves( );

	if (!CheckAndProcessMove(g, Move::AttackHero(0)))
	{
		printf("%s failed\n", __FUNCTION__);
		AnyFailed = true;
		return;
	}

	if (g.Winner != EWinner::PlayerOne)
	{
		printf("%s failed\n", __FUNCTION__);
		AnyFailed = true;
	}
}

void RunTests()
{
	Test_PlayerOne_Win_MinionAttack();

	if (!AnyFailed)
	{
		printf("All tests passed!\n");
	}
}
