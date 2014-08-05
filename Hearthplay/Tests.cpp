#include "Tests.h"
#include "GameState.h"
#include "Cards.h"

#include <string>

#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define LINE_STRING STRINGIZE(__LINE__) 

#define CHECK( foo ) \
if( !(foo) ) { \
	printf( "Check failed(" LINE_STRING "): " ## #foo "\n" ); \
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

TestCase Tests[] =
{
	{
		"Player one wins by attacking hero with minion", []( )
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
		"Player two wins by attacking hero with minion", []( )
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
		"Minion with charge attacks hero on turn it is summoned", []( )
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
		"Minion with summoning sickness attacking hero", []( )
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
	},
	{
		"Minion with divine shield attacked", []( )
		{
			GameState g;
			AddMinion(g, 0, Card::ArgentSquire);
			CHECK(g.Players[0].Minions[0].HasDivineShield( ) == true);

			AddMinion(g, 1, Card::BloodfenRaptor);
			g.UpdatePossibleMoves( );

			CheckAndProcessMove(g, Move::EndTurn( ));
			CheckAndProcessMove(g, Move::EndTurn( ));

			CHECK(CheckAndProcessMove(g, Move::AttackMinion(0, 0)));
			CHECK(g.Players[0].Minions.Num( ) == 1);
			CHECK(g.Players[0].Minions[0].HasDivineShield( ) == false);

			return true;
		}
	},
	{
		"Minions that cannot attack", []( )
		{
			GameState g;
			AddMinion(g, 0, Card::RagnarosTheFirelord);
			AddMinion(g, 0, Card::AncientWatcher);
			g.UpdatePossibleMoves( );

			CHECK(CheckAndProcessMove(g, Move::EndTurn( )));
			CHECK(CheckAndProcessMove(g, Move::EndTurn( )));

			CHECK(!MovePossible(g, Move::AttackHero(0)));
			CHECK(!MovePossible(g, Move::AttackHero(1)));

			return true;
		}
	},
	{
		"Hero cannot be attacked behind taunt minion", []( )
		{
			GameState g;
			AddMinion(g, 0, Card::BloodfenRaptor);
			AddMinion(g, 0, Card::BloodfenRaptor);
			AddMinion(g, 1, Card::SenjinShieldMasta);
			g.UpdatePossibleMoves( );

			CHECK(g.Players[1].Minions[0].HasTaunt( ));

			CHECK(CheckAndProcessMove(g, Move::EndTurn( )));
			CHECK(CheckAndProcessMove(g, Move::EndTurn( )));

			CHECK(MovePossible(g, Move::AttackMinion(0, 0)));
			CHECK(!MovePossible(g, Move::AttackHero(0)));

			return true;
		}
	},
	{
		"Other minions cannot be attacked behind taunt minion", []( )
		{
			GameState g;
			AddMinion(g, 0, Card::BloodfenRaptor);
			AddMinion(g, 0, Card::BloodfenRaptor);
			AddMinion(g, 1, Card::SenjinShieldMasta);
			AddMinion(g, 1, Card::MurlocRaider);
			g.UpdatePossibleMoves( );

			CHECK(g.Players[1].Minions[0].HasTaunt( ));
			CHECK(!g.Players[1].Minions[1].HasTaunt( ));

			CHECK(CheckAndProcessMove(g, Move::EndTurn( )));
			CHECK(CheckAndProcessMove(g, Move::EndTurn( )));

			CHECK(MovePossible(g, Move::AttackMinion(0, 0)));
			CHECK(!MovePossible(g, Move::AttackMinion(0, 1)));

			return true;
		}
	},
	{
		"Hero can be attacked when taunt minion is killed", []( )
		{
			GameState g;
			AddMinion(g, 0, Card::BloodfenRaptor);
			AddMinion(g, 0, Card::BloodfenRaptor);
			AddMinion(g, 1, Card::GoldshireFootman);
			g.UpdatePossibleMoves( );

			CHECK(g.Players[1].Minions[0].HasTaunt( ));

			CHECK(CheckAndProcessMove(g, Move::EndTurn( )));
			CHECK(CheckAndProcessMove(g, Move::EndTurn( )));

			CHECK(CheckAndProcessMove(g, Move::AttackMinion(0, 0)));
			CHECK(CheckAndProcessMove(g, Move::AttackHero(1)));

			return true;
		}
	},
	{
		"Minion cannot attack twice in one turn", []( )
		{
			GameState g;
			AddMinion(g, 0, Card::BloodfenRaptor);
			g.UpdatePossibleMoves( );

			CHECK(CheckAndProcessMove(g, Move::EndTurn( )));
			CHECK(CheckAndProcessMove(g, Move::EndTurn( )));

			CHECK(CheckAndProcessMove(g, Move::AttackHero(0)));
			CHECK(!MovePossible(g, Move::AttackHero(0)));

			return true;
		}
	}
};

void RunTests( )
{
	bool any_failed = false;
	for (auto test : Tests)
	{
		if (!test.Func( ))
		{
			printf("Test %s failed\n", test.Name);
			any_failed = true;
		}
	}

	if (!any_failed)
	{
		printf("All tests passed!\n");
	}
}
