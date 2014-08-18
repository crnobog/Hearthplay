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
	auto idx = g.m_players[player].m_minions.Add(m);
	return g.m_players[player].m_minions[idx];
}

Minion& AddMinionReadyToAttack(GameState& g, uint8_t player, Card c)
{
	Minion& m = AddMinion(g, player, c);
	m.m_flags &= ~(MinionFlags::AttackedThisTurn | MinionFlags::SummonedThisTurn);
	return m;
}

void SetManaAndMax(GameState& g, uint8_t player, uint8_t mana)
{
	g.m_players[player].m_mana = mana;
	g.m_players[player].m_max_mana = mana;
}

void SetHealth(GameState& g, uint8_t player, int8_t health)
{
	g.m_players[player].m_health = health;
}

void AddCard(GameState& g, uint8_t player, Card c)
{
	g.m_players[player].m_hand.Add(c);
}

bool CheckAndProcessMove(GameState& g, Move m)
{
	if (!g.m_possible_moves.Contains(m))
	{
		return false;
	}
	g.ProcessMove(m);
	return true;
}

bool MovePossible(const GameState& g, Move m)
{
	return g.m_possible_moves.Contains(m);
}

typedef bool(*TestFunc)();

struct TestCase
{
	const char* m_name;
	TestFunc	m_func;
};

TestCase Tests[] =
{
	{
		"Player one wins by attacking hero with minion", []( )
		{
			GameState g;
			g.m_active_player_index = 0;
			g.m_players[1].m_health = 2;

			AddMinionReadyToAttack(g, 0, Card::MurlocRaider);

			g.UpdatePossibleMoves( );

			CHECK(CheckAndProcessMove(g, Move::AttackHero(0)));
			CHECK(g.m_winner == Winner::PlayerOne);

			return true;
		}
	},
	{
		"Player two wins by attacking hero with minion", []( )
		{
			GameState g;
			g.m_active_player_index = 1;
			g.m_players[0].m_health = 2;

			AddMinionReadyToAttack(g, 1, Card::MurlocRaider);

			g.UpdatePossibleMoves( );

			CHECK(CheckAndProcessMove(g, Move::AttackHero(0)));
			CHECK(g.m_winner == Winner::PlayerTwo);
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
			CHECK(g.m_players[1].m_health == GameState::StartingHealth - GetCardData(Card::BluegillWarrior)->m_attack);
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
			CHECK(g.m_players[0].m_minions[0].HasDivineShield( ) == true);

			AddMinion(g, 1, Card::BloodfenRaptor);
			g.UpdatePossibleMoves( );

			CheckAndProcessMove(g, Move::EndTurn( ));
			CheckAndProcessMove(g, Move::EndTurn( ));

			CHECK(CheckAndProcessMove(g, Move::AttackMinion(0, 0)));
			CHECK(g.m_players[0].m_minions.Num( ) == 1);
			CHECK(g.m_players[0].m_minions[0].HasDivineShield( ) == false);

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

			CHECK(g.m_players[1].m_minions[0].HasTaunt( ));

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

			CHECK(g.m_players[1].m_minions[0].HasTaunt( ));
			CHECK(!g.m_players[1].m_minions[1].HasTaunt( ));

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

			CHECK(g.m_players[1].m_minions[0].HasTaunt( ));

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
	},
	{
		"Windfury minion can attack twice in one turn", []( )
		{
			GameState g;
			AddMinion(g, 0, Card::ThrallmarFarseer);
			g.UpdatePossibleMoves( );

			CHECK(CheckAndProcessMove(g, Move::EndTurn( )));
			CHECK(CheckAndProcessMove(g, Move::EndTurn( )));

			CHECK(CheckAndProcessMove(g, Move::AttackHero(0)));
			CHECK(CheckAndProcessMove(g, Move::AttackHero(0)));
			CHECK(!MovePossible(g, Move::AttackHero(0)));

			return true;
		}
	},
	{
		"Leper Gnome deals 2 damage when it dies while attacking", []( )
		{
			GameState g;
			AddMinion(g, 0, Card::LeperGnome);
			AddMinion(g, 1, Card::SenjinShieldMasta);
			g.UpdatePossibleMoves();

			CHECK(CheckAndProcessMove(g, Move::EndTurn( )));
			CHECK(CheckAndProcessMove(g, Move::EndTurn( )));

			CHECK(g.m_active_player_index == 0);
			CHECK(CheckAndProcessMove(g, Move::AttackMinion(0, 0)));
			CHECK(g.m_players[0].m_minions.Num( ) == 0);
			CHECK(g.m_players[1].m_health == GameState::StartingHealth - 2);

			return true;
		}
	},
	{
		"Leper Gnome deals 2 damage when it dies after being attacked", []( )
		{
			GameState g;
			AddMinion(g, 0, Card::LeperGnome);
			AddMinion(g, 1, Card::SenjinShieldMasta);
			g.UpdatePossibleMoves( );

			CHECK(CheckAndProcessMove(g, Move::EndTurn( )));

			CHECK(g.m_active_player_index == 1);
			CHECK(CheckAndProcessMove(g, Move::AttackMinion(0, 0)));
			CHECK(g.m_players[0].m_minions.Num( ) == 0);
			CHECK(g.m_players[1].m_health == GameState::StartingHealth - 2);

			return true;
		}
	},
	{
		"Leper Gnome can win a game when it dies while attacking", []( )
		{
			GameState g;
			AddMinion(g, 0, Card::LeperGnome);
			AddMinion(g, 1, Card::SenjinShieldMasta);
			g.m_players[1].m_health = 2;
			g.UpdatePossibleMoves( );

			CHECK(CheckAndProcessMove(g, Move::EndTurn( )));
			CHECK(CheckAndProcessMove(g, Move::EndTurn( )));

			CHECK(g.m_active_player_index == 0);
			CHECK(CheckAndProcessMove(g, Move::AttackMinion(0, 0)));
			CHECK(g.m_players[0].m_minions.Num( ) == 0);
			CHECK(g.m_winner == Winner::PlayerOne);

			return true;
		}
	},
	{
		"Leper Gnome can win a game when it dies after being attacked", []( )
		{
			GameState g;
			AddMinion(g, 0, Card::LeperGnome);
			AddMinion(g, 1, Card::SenjinShieldMasta);
			g.m_players[1].m_health = 2;
			g.UpdatePossibleMoves( );

			CHECK(CheckAndProcessMove(g, Move::EndTurn( )));

			CHECK(g.m_active_player_index == 1);
			CHECK(CheckAndProcessMove(g, Move::AttackMinion(0, 0)));
			CHECK(g.m_players[0].m_minions.Num( ) == 0);
			CHECK(g.m_winner == Winner::PlayerOne);

			return true;
		}
	},
	{
		"Two Leper Gnomes dying can cause a draw", []( )
		{
			GameState g;
			AddMinion(g, 0, Card::LeperGnome);
			AddMinion(g, 1, Card::LeperGnome);
			SetHealth(g, 0, 2);
			SetHealth(g, 1, 2);
			g.UpdatePossibleMoves( );

			CHECK(CheckAndProcessMove(g, Move::EndTurn( )));

			CHECK(g.m_active_player_index == 1);
			CHECK(CheckAndProcessMove(g, Move::AttackMinion(0, 0)));
			CHECK(g.m_winner == Winner::Draw);

			return true;
		}
	},
	{
		"Coin adds one mana", []( )
		{
			GameState g;
			g.m_players[0].m_hand.Add(Card::Coin);
			g.UpdatePossibleMoves( );

			auto mana = g.m_players[0].m_mana;
			CHECK(CheckAndProcessMove(g, Move::PlayCard(Card::Coin, Move::TargetPlayer(g.m_active_player_index))));
			CHECK(g.m_players[0].m_mana == mana + 1);

			return true;
		}
	},
	{
		"Play minion", []( )
		{
			GameState g;
			g.m_players[0].m_hand.Add(Card::BloodfenRaptor);
			SetManaAndMax(g, 0, 2);
			g.UpdatePossibleMoves( );

			CHECK(g.m_active_player_index == 0);
			CHECK(CheckAndProcessMove(g, Move::PlayCard(Card::BloodfenRaptor)));
			CHECK(g.m_players[0].m_minions.Num( ) == 1);
			CHECK(g.m_players[0].m_minions[0].m_source_card == GetCardData(Card::BloodfenRaptor));
			CHECK(g.m_players[0].m_minions[0].m_attack == 3);
			CHECK(g.m_players[0].m_minions[0].m_health == 2);
			CHECK(g.m_players[0].m_minions[0].CanAttack( ) == false);

			return true;
		}
	},
	{
		"Elven Archer can damage opponent", []( )
		{
			GameState g;
			g.m_players[0].m_hand.Add(Card::ElvenArcher);
			SetManaAndMax(g, 0, 1);
			g.UpdatePossibleMoves( );

			CHECK(g.m_active_player_index == 0);
			CHECK(CheckAndProcessMove(g, Move::PlayCard(Card::ElvenArcher, Move::TargetPlayer(1))));
			CHECK(g.m_players[1].m_health == GameState::StartingHealth - 1);

			return true;
		}
	},
	{
		"Elven Archer can damage opponent and win", []( )
		{
			GameState g;
			g.m_players[0].m_hand.Add(Card::ElvenArcher);
			SetManaAndMax(g, 0, 1);
			SetHealth(g, 1, 1);
			g.UpdatePossibleMoves( );

			CHECK(g.m_active_player_index == 0);
			CHECK(CheckAndProcessMove(g, Move::PlayCard(Card::ElvenArcher, Move::TargetPlayer(1))));
			CHECK(g.m_winner == Winner::PlayerOne);

			return true;
		}
	},
	{
		"Elven Archer can damage owner", []( )
		{
			GameState g;
			g.m_players[0].m_hand.Add(Card::ElvenArcher);
			SetManaAndMax(g, 0, 1);
			g.UpdatePossibleMoves( );

			CHECK(g.m_active_player_index == 0);
			CHECK(CheckAndProcessMove(g, Move::PlayCard(Card::ElvenArcher, Move::TargetPlayer(0))));
			CHECK(g.m_players[0].m_health == GameState::StartingHealth - 1);

			return true;
		}
	},
	{
		"Elven Archer can damage owner and lose", []( )
		{
			GameState g;
			g.m_players[0].m_hand.Add(Card::ElvenArcher);
			SetManaAndMax(g, 0, 1);
			SetHealth(g, 0, 1);
			g.UpdatePossibleMoves( );

			CHECK(g.m_active_player_index == 0);
			CHECK(CheckAndProcessMove(g, Move::PlayCard(Card::ElvenArcher, Move::TargetPlayer(0))));
			CHECK(g.m_winner == Winner::PlayerTwo);

			return true;
		}
	},
	{
		"Elven Archer can damage enemy minion", []( )
		{
			GameState g;
			g.m_players[0].m_hand.Add(Card::ElvenArcher);
			AddMinion(g, 1, Card::BloodfenRaptor);
			SetManaAndMax(g, 0, 1);
			g.UpdatePossibleMoves( );

			CHECK(g.m_active_player_index == 0);
			CHECK(CheckAndProcessMove(g, Move::PlayCard(Card::ElvenArcher, Move::TargetMinion(1, 0))));
			CHECK(g.m_players[1].m_minions.Num() == 1);
			CHECK(g.m_players[1].m_minions[0].m_health == 1);

			return true;
		}
	},
	{
		"Elven Archer can damage enemy minion and kill it", []( )
		{
			GameState g;
			g.m_players[0].m_hand.Add(Card::ElvenArcher);
			AddMinion(g, 1, Card::BluegillWarrior);
			SetManaAndMax(g, 0, 1);
			g.UpdatePossibleMoves( );

			CHECK(g.m_active_player_index == 0);
			CHECK(CheckAndProcessMove(g, Move::PlayCard(Card::ElvenArcher, Move::TargetMinion(1, 0))));
			CHECK(g.m_players[1].m_minions.Num( ) == 0);

			return true;
		}
	},
	{
		"Elven Archer can damage friendly minion", []( )
		{
			GameState g;
			g.m_players[0].m_hand.Add(Card::ElvenArcher);
			AddMinion(g, 0, Card::BloodfenRaptor);
			SetManaAndMax(g, 0, 1);
			g.UpdatePossibleMoves( );

			CHECK(g.m_active_player_index == 0);
			CHECK(CheckAndProcessMove(g, Move::PlayCard(Card::ElvenArcher, Move::TargetMinion(0, 0))));
			CHECK(g.m_players[0].m_minions.Num( ) == 2);
			CHECK(g.m_players[0].m_minions[0].m_source_card == GetCardData(Card::BloodfenRaptor));
			CHECK(g.m_players[0].m_minions[0].m_health == 1);

			return true;
		}
	},
	{
		"Elven Archer can damage friendly minion and kill it", []( )
		{
			GameState g;
			g.m_players[0].m_hand.Add(Card::ElvenArcher);
			AddMinion(g, 0, Card::BluegillWarrior);
			SetManaAndMax(g, 0, 1);
			g.UpdatePossibleMoves( );

			CHECK(g.m_active_player_index == 0);
			CHECK(CheckAndProcessMove(g, Move::PlayCard(Card::ElvenArcher, Move::TargetMinion(0, 0))));
			CHECK(g.m_players[0].m_minions.Num( ) == 1);
			CHECK(g.m_players[0].m_minions[0].m_source_card == GetCardData(Card::ElvenArcher));

			return true;
		}
	},
	{
		"Elven Archer killing Leper Gnome triggers deathrattle", []( )
		{
			GameState g;
			AddMinion(g, 0, Card::LeperGnome);
			AddCard(g, 0, Card::ElvenArcher);
			SetManaAndMax(g, 0, 1);
			g.UpdatePossibleMoves( );

			CHECK( g.m_active_player_index == 0 );
			CHECK(CheckAndProcessMove(g, Move::PlayCard(Card::ElvenArcher, Move::TargetMinion(0, 0))));
			CHECK(g.m_players[1].m_health == GameState::StartingHealth - 2);
			
			return true;
		}
	},
	{
		"Elven Archer killing Leper Gnome can win game", []( )
		{
			GameState g;
			AddMinion(g, 0, Card::LeperGnome);
			AddCard(g, 0, Card::ElvenArcher);
			SetManaAndMax(g, 0, 1);
			SetHealth(g, 1, 2);
			g.UpdatePossibleMoves( );

			CHECK(g.m_active_player_index == 0);
			CHECK(CheckAndProcessMove(g, Move::PlayCard(Card::ElvenArcher, Move::TargetMinion(0, 0))));
			CHECK(g.m_winner == Winner::PlayerOne);

			return true;
		}
	},
	{
		"Elven Archer killing Leper Gnome can lose game", []( )
		{
			GameState g;
			AddMinion(g, 1, Card::LeperGnome);
			AddCard(g, 0, Card::ElvenArcher);
			SetManaAndMax(g, 0, 1);
			SetHealth(g, 0, 2);
			g.UpdatePossibleMoves( );

			CHECK(g.m_active_player_index == 0);
			CHECK(CheckAndProcessMove(g, Move::PlayCard(Card::ElvenArcher, Move::TargetMinion(1, 0))));
			CHECK(g.m_winner == Winner::PlayerTwo);

			return true;
		}
	},
	{
		"Nightblade battlecry damages opponent and can win", []( )
		{
			GameState g;
			AddCard(g, 0, Card::Nightblade);
			SetManaAndMax(g, 0, 5);
			SetHealth(g, 1, 3);
			g.UpdatePossibleMoves( );

			CHECK(g.m_active_player_index == 0);
			CHECK(CheckAndProcessMove(g, Move::PlayCard(Card::Nightblade, Move::TargetPlayer(1))));
			CHECK(g.m_winner == Winner::PlayerOne);

			return true;
		}
	},
	{
		"Voodoo Doctor battlecry can target any character", []( )
		{
			GameState g;
			AddMinion(g, 0, Card::BloodfenRaptor);
			AddMinion(g, 0, Card::SenjinShieldMasta);
			AddMinion(g, 1, Card::SenjinShieldMasta);
			AddMinion(g, 1, Card::BloodfenRaptor);
			AddCard(g, 0, Card::VoodooDoctor);
			SetManaAndMax(g, 0, 1);
			g.UpdatePossibleMoves( );

			CHECK(g.m_active_player_index == 0);
			CHECK(MovePossible(g, Move::PlayCard(Card::VoodooDoctor, Move::TargetMinion(0, 0))));
			CHECK(MovePossible(g, Move::PlayCard(Card::VoodooDoctor, Move::TargetMinion(0, 1))));
			CHECK(MovePossible(g, Move::PlayCard(Card::VoodooDoctor, Move::TargetMinion(1, 0))));
			CHECK(MovePossible(g, Move::PlayCard(Card::VoodooDoctor, Move::TargetMinion(1, 1))));
			CHECK(MovePossible(g, Move::PlayCard(Card::VoodooDoctor, Move::TargetPlayer(0))));
			CHECK(MovePossible(g, Move::PlayCard(Card::VoodooDoctor, Move::TargetPlayer(1))));

			return true;
		}
	},
	{
		"Voodoo Doctor battlecry must have a target", []( )
		{
			GameState g;
			AddMinion(g, 0, Card::BloodfenRaptor);
			AddMinion(g, 0, Card::SenjinShieldMasta);
			AddMinion(g, 1, Card::SenjinShieldMasta);
			AddMinion(g, 1, Card::BloodfenRaptor);
			AddCard(g, 0, Card::VoodooDoctor);
			SetManaAndMax(g, 0, 1);
			g.UpdatePossibleMoves( );

			CHECK(g.m_active_player_index == 0);
			CHECK(!MovePossible(g, Move::PlayCard(Card::VoodooDoctor)));

			return true;
		}
	},
	{
		"Voodoo Doctor heals either player", []( )
		{
			GameState g;
			AddCard(g, 0, Card::VoodooDoctor);
			AddCard(g, 0, Card::VoodooDoctor);
			SetManaAndMax(g, 0, 2);
			SetHealth(g, 0, 20);
			SetHealth(g, 1, 20);
			g.UpdatePossibleMoves( );

			CHECK(g.m_active_player_index == 0);
			CHECK(CheckAndProcessMove(g, Move::PlayCard(Card::VoodooDoctor, Move::TargetPlayer(0))));
			CHECK(g.m_players[0].m_health == 22);
			CHECK(CheckAndProcessMove(g, Move::PlayCard(Card::VoodooDoctor, Move::TargetPlayer(1))));
			CHECK(g.m_players[1].m_health == 22);

			return true;
		}
	},
	{
		"Voodoo Doctor heals a minion by 2 up to max health", []( )
		{
			GameState g;
			AddCard(g, 0, Card::VoodooDoctor);
			AddCard(g, 0, Card::VoodooDoctor);
			SetManaAndMax(g, 0, 2);
			AddMinion(g, 0, Card::SenjinShieldMasta);
			AddMinion(g, 0, Card::SenjinShieldMasta);
			g.m_players[0].m_minions[0].m_health = 2;
			g.m_players[0].m_minions[1].m_health = 4;
			g.UpdatePossibleMoves( );

			CHECK(g.m_active_player_index == 0);
			CHECK(CheckAndProcessMove(g, Move::PlayCard(Card::VoodooDoctor, Move::TargetMinion(0, 0))));
			CHECK(g.m_players[0].m_minions[0].m_health == 4);
			CHECK(CheckAndProcessMove(g, Move::PlayCard(Card::VoodooDoctor, Move::TargetMinion(0, 1))));
			CHECK(g.m_players[0].m_minions[1].m_health == 5);

			return true;
		}
	},
	{
		"Abusive Sergeant can target any minion", []( )
		{
			GameState g;
			AddCard(g, 0, Card::AbusiveSergeant);
			SetManaAndMax(g, 0, 1);
			AddMinion(g, 0, Card::SenjinShieldMasta);
			AddMinion(g, 1, Card::SenjinShieldMasta);
			g.UpdatePossibleMoves( );

			CHECK(!MovePossible(g, Move::PlayCard(Card::AbusiveSergeant)));
			CHECK(MovePossible(g, Move::PlayCard(Card::AbusiveSergeant, Move::TargetMinion(0, 0))));
			CHECK(MovePossible(g, Move::PlayCard(Card::AbusiveSergeant, Move::TargetMinion(1, 0))));

			return true;
		}
	},
	{
		"Abusive Sergeant has no target on empty board", []( )
		{
			GameState g;
			AddCard(g, 0, Card::AbusiveSergeant);
			SetManaAndMax(g, 0, 1);
			g.UpdatePossibleMoves( );

			CHECK(CheckAndProcessMove(g, Move::PlayCard(Card::AbusiveSergeant)));

			return true;
		}
	},
	{
		"Minion affected by Abusive Sergeant does more damage", []( )
		{
			GameState g;
			AddCard(g, 0, Card::AbusiveSergeant);
			SetManaAndMax(g, 0, 1);
			AddMinionReadyToAttack(g, 0, Card::BloodfenRaptor);
			AddMinion(g, 1, Card::SenjinShieldMasta);
			g.UpdatePossibleMoves( );

			CHECK(CheckAndProcessMove(g, Move::PlayCard(Card::AbusiveSergeant, Move::TargetMinion(0, 0))));
			CHECK(CheckAndProcessMove(g, Move::AttackMinion(0, 0)));
			CHECK(g.m_players[1].m_minions.Num( ) == 0);

			return true;
		}
	},
	{
		"Minion affected by Abusive Sergeant normal damage on opponent's next turn", []( )
		{
			GameState g;
			AddCard(g, 0, Card::AbusiveSergeant);
			SetManaAndMax(g, 0, 1);
			AddMinion(g, 0, Card::BloodfenRaptor);
			AddMinion(g, 1, Card::SenjinShieldMasta);
			g.UpdatePossibleMoves( );

			CHECK(CheckAndProcessMove(g, Move::PlayCard(Card::AbusiveSergeant, Move::TargetMinion(0, 0))));
			CHECK(CheckAndProcessMove(g, Move::EndTurn( )));
			CHECK(CheckAndProcessMove(g, Move::AttackMinion(0, 0)));
			CHECK(g.m_players[0].m_minions.Num( ) == 1);
			CHECK(g.m_players[1].m_minions.Num( ) == 1);
			CHECK(g.m_players[1].m_minions[0].m_health == 2);

			return true;
		}
	},
	{
		"Minion affected by Abusive Sergeant normal damage on owner's next turn", []( )
		{
			GameState g;
			AddCard(g, 0, Card::AbusiveSergeant);
			SetManaAndMax(g, 0, 1);
			AddMinion(g, 0, Card::BloodfenRaptor);
			AddMinion(g, 1, Card::SenjinShieldMasta);
			g.UpdatePossibleMoves( );

			CHECK(CheckAndProcessMove(g, Move::PlayCard(Card::AbusiveSergeant, Move::TargetMinion(0, 0))));
			CHECK(CheckAndProcessMove(g, Move::EndTurn( )));
			CHECK(CheckAndProcessMove(g, Move::EndTurn( )));
			CHECK(CheckAndProcessMove(g, Move::AttackMinion(0, 0)));
			CHECK(g.m_players[0].m_minions.Num( ) == 1);
			CHECK(g.m_players[1].m_minions.Num( ) == 1);
			CHECK(g.m_players[1].m_minions[0].m_health == 2);

			return true;
		}
	},
	{
		"Zombie chow heals opponent", []( )
		{
			GameState g;
			AddMinion(g, 0, Card::ZombieChow);
			AddMinion(g, 1, Card::SenjinShieldMasta);
			SetHealth(g, 1, 25);
			g.UpdatePossibleMoves( );

			CHECK(CheckAndProcessMove(g, Move::EndTurn( )));
			CHECK(CheckAndProcessMove(g, Move::AttackMinion(0, 0)));
			CHECK(g.m_players[0].m_minions.Num( ) == 0);
			CHECK(g.m_players[1].m_health == 30);

			return true;
		}
	},
	{
		"Stealth minion loses stealth when attacking", []( )
		{
			GameState g;
			AddMinionReadyToAttack(g, 0, Card::WorgenInfiltrator);
			g.UpdatePossibleMoves( );

			CHECK(g.m_players[0].m_minions[0].HasStealth( ));
			CHECK(CheckAndProcessMove(g, Move::AttackHero(0)));
			CHECK(!g.m_players[0].m_minions[0].HasStealth( ));

			return true;
		}
	},
	{
		"Stealth minion cannot be attacked", []( )
		{
			GameState g;
			AddMinion(g, 0, Card::WorgenInfiltrator);
			AddMinion(g, 1, Card::SenjinShieldMasta);
			g.UpdatePossibleMoves( );

			CHECK(CheckAndProcessMove(g, Move::EndTurn( )));
			CHECK(!MovePossible(g, Move::AttackMinion(0, 0)));

			return true;
		}
	},
	{
		"Stealth minion with taunt does not prevent attacking hero or other minions", []( )
		{
			GameState g;
			AddMinion(g, 0, Card::WorgenInfiltrator);
			AddMinion(g, 0, Card::BloodfenRaptor);
			g.m_players[0].m_minions[0].AddAbility(MinionAbilityFlags::Taunt);
			AddMinion(g, 1, Card::SenjinShieldMasta);
			g.UpdatePossibleMoves( );

			CHECK(g.m_players[0].m_minions[0].HasStealth( ));
			CHECK(g.m_players[0].m_minions[0].HasTaunt( ));
			CHECK(CheckAndProcessMove(g, Move::EndTurn( )));
			CHECK(MovePossible(g, Move::AttackHero(0)));
			CHECK(MovePossible(g, Move::AttackMinion(0, 1)));
			CHECK(!MovePossible(g, Move::AttackMinion(0, 0)));

			return true;
		}
	},
	{
		"Stealth minion cannot be targeted by enemy battlecry", []( )
		{
			GameState g;
			AddMinion(g, 0, Card::WorgenInfiltrator);
			AddCard(g, 1, Card::ElvenArcher);
			SetManaAndMax(g, 1, 1);
			g.UpdatePossibleMoves( );

			CHECK(CheckAndProcessMove(g, Move::EndTurn( )));
			CHECK(!MovePossible(g, Move::PlayCard(Card::ElvenArcher, Move::TargetMinion(0, 0))));

			return true;
		}
	},
	{
		"Stealth minion can be targeted by friendly battlecry", []( )
		{
			GameState g;
			AddMinion(g, 0, Card::WorgenInfiltrator);
			AddCard(g, 0, Card::ElvenArcher);
			SetManaAndMax(g, 0, 1);
			g.UpdatePossibleMoves( );

			CHECK(CheckAndProcessMove(g, Move::PlayCard(Card::ElvenArcher, Move::TargetMinion(0, 0))));
			CHECK(g.m_players[0].m_minions.Num( ) == 1);

			return true;
		}
	}
};

void RunTests( )
{
	uint32_t num_tests = sizeof(Tests) / sizeof(TestCase);
	bool any_failed = false;
	for (auto test : Tests)
	{
		if (!test.m_func( ))
		{
			printf("Test %s failed\n", test.m_name);
			any_failed = true;
		}
	}

	if (!any_failed)
	{
		printf("All %u tests passed!\n", num_tests);
	}
}
