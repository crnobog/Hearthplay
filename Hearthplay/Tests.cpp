#include "Tests.h"
#include "GameState.h"
#include "Cards.h"

bool AnyFailed = false;

void Test_PlayerOne_Win_MinionAttack()
{
	GameState g;
	g.ActivePlayerIndex = 0;
	g.Players[1].Health = 2;

	g.Players[0].Minions.Add({ 2, 1, GetCardData(Card::MurlocRaider), false, false });

	g.UpdatePossibleMoves();
	g.ProcessMove({ MoveType::AttackHero, 0, 0 });

	if (g.Winner != EWinner::PlayerOne)
	{
		printf("%s failed\n", __FUNCTION__ );
		AnyFailed = true;
	}
}

void Test_PlayerTwo_Win_MinionAttack()
{
	GameState g;
	g.ActivePlayerIndex = 1;
	g.Players[0].Health = 2;

	g.Players[1].Minions.Add({ 2, 1, GetCardData(Card::MurlocRaider), false, false });

	g.UpdatePossibleMoves();
	g.ProcessMove({ MoveType::AttackHero, 0, 0 });

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
