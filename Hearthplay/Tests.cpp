#include "Tests.h"
#include "GameState.h"
#include "Cards.h"

#include <string>

bool AnyFailed = false;

#define CHECK( foo ) \
if( !(foo) ) { \
	printf( "Check failed: " ## #foo "\n" ); \
	return false;\
}

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

bool MovePossible(const GameState& g, Move m)
{
	return g.PossibleMoves.Contains(m);
}

typedef bool(*TestFunc)();

struct TestCase
{
	const char* Name;
	TestFunc Func;
};

TestCase Tests[] = {
		{ "PlayerOne_Win_MinionAttackHero", []( ) 
			{
				GameState g;
				g.ActivePlayerIndex = 0;
				g.Players[1].Health = 2;

				AddMinionReadyToAttack(g, 0, Card::MurlocRaider);

				g.UpdatePossibleMoves( );

				CHECK(CheckAndProcessMove(g, Move::AttackHero(0)));
				CHECK(g.Winner == EWinner::PlayerOne);

				return true;
			}
		},
		{
			"PlayerTwo_Win_MinionAttackHero", []( ) 
			{
				GameState g;
				g.ActivePlayerIndex = 1;
				g.Players[0].Health = 2;

				AddMinionReadyToAttack(g, 1, Card::MurlocRaider);

				g.UpdatePossibleMoves( );

				CHECK(CheckAndProcessMove(g, Move::AttackHero(0)));
				CHECK(g.Winner == EWinner::PlayerTwo);
				return true;

			}
		},
		{
			"Charge_MinionAttack_AttackHero", []( )
			{
				GameState g;

				AddMinion(g, 0, Card::BluegillWarrior);

				g.UpdatePossibleMoves( );

				CHECK(CheckAndProcessMove(g, Move::AttackHero(0)));
				CHECK(g.Players[1].Health == GameState::StartingHealth - GetCardData(Card::BluegillWarrior)->Attack);
				return true;
			}
		},
		{
			"SummoningSickness_AttackHero", []( )
			{
				GameState g;
				AddMinion(g, 0, Card::BloodfenRaptor);
				g.UpdatePossibleMoves( );

				CHECK(!MovePossible(g, Move::AttackHero(0)));
				CHECK(CheckAndProcessMove(g, Move::EndTurn( )));
				CHECK(CheckAndProcessMove(g, Move::EndTurn( )));
				CHECK(CheckAndProcessMove(g, Move::AttackHero(0)));

				return true;
			}
		}
};

void RunTests( )
{
	for (auto test : Tests)
	{
		if (!test.Func( ))
		{
			printf("Test %s failed\n", test.Name);
		}
	}

	if (!AnyFailed)
	{
		printf("All tests passed!\n");
	}
}
