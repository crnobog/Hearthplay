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

#define CHECK_MOVE_POSSIBLE( move ) \
	CHECK( MovePossible(g, move) )

#define CHECK_MOVE_IMPOSSIBLE( move ) \
	CHECK( !MovePossible(g, move) )

#define CHECK_DO_MOVE( move ) \
	CHECK( ProcessMove(g, move) )

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

void AddCardToDeck(GameState& g, uint8_t player, Card c)
{
	g.m_players[player].m_deck.Add(c);
}

bool ProcessMove(GameState& g, Move m)
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

Minion& GetMinion(GameState& g, uint8_t player_idx, uint8_t minion_idx)
{
	return g.m_players[player_idx].m_minions[minion_idx];
}

uint8_t GetNumMinions(const GameState& g, uint8_t player_idx)
{
	return g.m_players[player_idx].m_minions.Num( );
}

int8_t GetPlayerHealth(const GameState& g, uint8_t player_idx)
{
	return g.m_players[player_idx].m_health;
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

			CHECK_DO_MOVE(Move::AttackHero(0));
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

			CHECK_DO_MOVE(Move::AttackHero(0));
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

			CHECK_DO_MOVE(Move::AttackHero(0));
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

			CHECK_MOVE_IMPOSSIBLE(Move::AttackHero(0));
			CHECK_DO_MOVE(Move::EndTurn( ));
			CHECK_DO_MOVE(Move::EndTurn( ));
			CHECK_DO_MOVE(Move::AttackHero(0));

			return true;
		}
	},
	{
		"Minion with divine shield attacked", []( )
		{
			GameState g;
			AddMinion(g, 0, Card::ArgentSquire);
			CHECK(GetMinion(g,0,0).HasDivineShield( ) == true);

			AddMinion(g, 1, Card::BloodfenRaptor);
			g.UpdatePossibleMoves( );

			ProcessMove(g, Move::EndTurn( ));
			ProcessMove(g, Move::EndTurn( ));

			CHECK_DO_MOVE(Move::AttackMinion(0, 0));
			CHECK(GetNumMinions(g, 0) == 1);
			CHECK(GetMinion(g, 0, 0).HasDivineShield( ) == false);

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

			CHECK_DO_MOVE(Move::EndTurn( ));
			CHECK_DO_MOVE(Move::EndTurn( ));

			CHECK_MOVE_IMPOSSIBLE(Move::AttackHero(0));
			CHECK_MOVE_IMPOSSIBLE(Move::AttackHero(1));

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

			CHECK_DO_MOVE(Move::EndTurn( ));
			CHECK_DO_MOVE(Move::EndTurn( ));

			CHECK_MOVE_POSSIBLE(Move::AttackMinion(0, 0));
			CHECK_MOVE_IMPOSSIBLE(Move::AttackHero(0));

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

			CHECK(GetMinion(g, 1, 0).HasTaunt( ));
			CHECK(!GetMinion(g, 1, 1).HasTaunt( ));

			CHECK_DO_MOVE(Move::EndTurn( ));
			CHECK_DO_MOVE(Move::EndTurn( ));

			CHECK_MOVE_POSSIBLE(Move::AttackMinion(0, 0));
			CHECK_MOVE_IMPOSSIBLE(Move::AttackMinion(0, 1));

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

			CHECK(GetMinion(g, 1, 0).HasTaunt( ));

			CHECK_DO_MOVE(Move::EndTurn( ));
			CHECK_DO_MOVE(Move::EndTurn( ));

			CHECK_DO_MOVE(Move::AttackMinion(0, 0));
			CHECK_DO_MOVE(Move::AttackHero(1));

			return true;
		}
	},
	{
		"Minion cannot attack twice in one turn", []( )
		{
			GameState g;
			AddMinion(g, 0, Card::BloodfenRaptor);
			g.UpdatePossibleMoves( );

			CHECK_DO_MOVE(Move::EndTurn( ));
			CHECK_DO_MOVE(Move::EndTurn( ));

			CHECK_DO_MOVE(Move::AttackHero(0));
			CHECK_MOVE_IMPOSSIBLE(Move::AttackHero(0));

			return true;
		}
	},
	{
		"Windfury minion can attack twice in one turn", []( )
		{
			GameState g;
			AddMinion(g, 0, Card::ThrallmarFarseer);
			g.UpdatePossibleMoves( );

			CHECK_DO_MOVE(Move::EndTurn( ));
			CHECK_DO_MOVE(Move::EndTurn( ));

			CHECK_DO_MOVE(Move::AttackHero(0));
			CHECK_DO_MOVE(Move::AttackHero(0));
			CHECK_MOVE_IMPOSSIBLE(Move::AttackHero(0));

			return true;
		}
	},
	{
		"Leper Gnome deals 2 damage when it dies while attacking", []( )
		{
			GameState g;
			AddMinion(g, 0, Card::LeperGnome);
			AddMinion(g, 1, Card::SenjinShieldMasta);
			g.UpdatePossibleMoves( );

			CHECK_DO_MOVE(Move::EndTurn( ));
			CHECK_DO_MOVE(Move::EndTurn( ));

			CHECK(g.m_active_player_index == 0);
			CHECK_DO_MOVE(Move::AttackMinion(0, 0));
			CHECK(GetNumMinions(g, 0) == 0);
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

			CHECK_DO_MOVE(Move::EndTurn( ));

			CHECK(g.m_active_player_index == 1);
			CHECK_DO_MOVE(Move::AttackMinion(0, 0));
			CHECK(GetNumMinions(g, 0) == 0);
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

			CHECK_DO_MOVE(Move::EndTurn( ));
			CHECK_DO_MOVE(Move::EndTurn( ));

			CHECK(g.m_active_player_index == 0);
			CHECK_DO_MOVE(Move::AttackMinion(0, 0));
			CHECK(GetNumMinions(g, 0) == 0);
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

			CHECK_DO_MOVE(Move::EndTurn( ));

			CHECK(g.m_active_player_index == 1);
			CHECK_DO_MOVE(Move::AttackMinion(0, 0));
			CHECK(GetNumMinions(g, 0) == 0);
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

			CHECK_DO_MOVE(Move::EndTurn( ));

			CHECK(g.m_active_player_index == 1);
			CHECK_DO_MOVE(Move::AttackMinion(0, 0));
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
			CHECK_DO_MOVE(Move::PlayCard(Card::Coin, Move::TargetPlayer(g.m_active_player_index)));
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
			CHECK_DO_MOVE(Move::PlayCard(Card::BloodfenRaptor));
			CHECK(GetNumMinions(g, 0) == 1);
			CHECK(GetMinion(g,0,0).m_source_card == GetCardData(Card::BloodfenRaptor));
			CHECK(GetMinion(g,0,0).m_attack == 3);
			CHECK(GetMinion(g,0,0).m_health == 2);
			CHECK(GetMinion(g,0,0).CanAttack( ) == false);

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
			CHECK_DO_MOVE(Move::PlayCard(Card::ElvenArcher, Move::TargetPlayer(1)));
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
			CHECK_DO_MOVE(Move::PlayCard(Card::ElvenArcher, Move::TargetPlayer(1)));
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
			CHECK_DO_MOVE(Move::PlayCard(Card::ElvenArcher, Move::TargetPlayer(0)));
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
			CHECK_DO_MOVE(Move::PlayCard(Card::ElvenArcher, Move::TargetPlayer(0)));
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
			CHECK_DO_MOVE(Move::PlayCard(Card::ElvenArcher, Move::TargetMinion(1, 0)));
			CHECK(GetNumMinions(g, 1) == 1);
			CHECK(GetMinion(g, 1, 0).m_health == 1);

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
			CHECK_DO_MOVE(Move::PlayCard(Card::ElvenArcher, Move::TargetMinion(1, 0)));
			CHECK(GetNumMinions(g, 1) == 0);

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
			CHECK_DO_MOVE(Move::PlayCard(Card::ElvenArcher, Move::TargetMinion(0, 0)));
			CHECK(GetNumMinions(g, 0) == 2);
			CHECK(GetMinion(g,0,0).m_source_card == GetCardData(Card::BloodfenRaptor));
			CHECK(GetMinion(g,0,0).m_health == 1);

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
			CHECK_DO_MOVE(Move::PlayCard(Card::ElvenArcher, Move::TargetMinion(0, 0)));
			CHECK(GetNumMinions(g, 0) == 1);
			CHECK(GetMinion(g,0,0).m_source_card == GetCardData(Card::ElvenArcher));

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

			CHECK(g.m_active_player_index == 0);
			CHECK_DO_MOVE(Move::PlayCard(Card::ElvenArcher, Move::TargetMinion(0, 0)));
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
			CHECK_DO_MOVE(Move::PlayCard(Card::ElvenArcher, Move::TargetMinion(0, 0)));
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
			CHECK_DO_MOVE(Move::PlayCard(Card::ElvenArcher, Move::TargetMinion(1, 0)));
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
			CHECK_DO_MOVE(Move::PlayCard(Card::Nightblade, Move::TargetPlayer(1)));
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
			CHECK_MOVE_POSSIBLE(Move::PlayCard(Card::VoodooDoctor, Move::TargetMinion(0, 0)));
			CHECK_MOVE_POSSIBLE(Move::PlayCard(Card::VoodooDoctor, Move::TargetMinion(0, 1)));
			CHECK_MOVE_POSSIBLE(Move::PlayCard(Card::VoodooDoctor, Move::TargetMinion(1, 0)));
			CHECK_MOVE_POSSIBLE(Move::PlayCard(Card::VoodooDoctor, Move::TargetMinion(1, 1)));
			CHECK_MOVE_POSSIBLE(Move::PlayCard(Card::VoodooDoctor, Move::TargetPlayer(0)));
			CHECK_MOVE_POSSIBLE(Move::PlayCard(Card::VoodooDoctor, Move::TargetPlayer(1)));

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
			CHECK_MOVE_IMPOSSIBLE(Move::PlayCard(Card::VoodooDoctor));

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
			CHECK_DO_MOVE(Move::PlayCard(Card::VoodooDoctor, Move::TargetPlayer(0)));
			CHECK(g.m_players[0].m_health == 22);
			CHECK_DO_MOVE(Move::PlayCard(Card::VoodooDoctor, Move::TargetPlayer(1)));
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
			GetMinion(g,0,0).m_health = 2;
			g.m_players[0].m_minions[1].m_health = 4;
			g.UpdatePossibleMoves( );

			CHECK(g.m_active_player_index == 0);
			CHECK_DO_MOVE(Move::PlayCard(Card::VoodooDoctor, Move::TargetMinion(0, 0)));
			CHECK(GetMinion(g,0,0).m_health == 4);
			CHECK_DO_MOVE(Move::PlayCard(Card::VoodooDoctor, Move::TargetMinion(0, 1)));
			CHECK(GetMinion(g, 0, 1).m_health == 5);

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

			CHECK_MOVE_IMPOSSIBLE(Move::PlayCard(Card::AbusiveSergeant));
			CHECK_MOVE_POSSIBLE(Move::PlayCard(Card::AbusiveSergeant, Move::TargetMinion(0, 0)));
			CHECK_MOVE_POSSIBLE(Move::PlayCard(Card::AbusiveSergeant, Move::TargetMinion(1, 0)));

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

			CHECK_DO_MOVE(Move::PlayCard(Card::AbusiveSergeant));

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

			CHECK_DO_MOVE(Move::PlayCard(Card::AbusiveSergeant, Move::TargetMinion(0, 0)));
			CHECK_DO_MOVE(Move::AttackMinion(0, 0));
			CHECK(GetNumMinions(g, 1) == 0);

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

			CHECK_DO_MOVE(Move::PlayCard(Card::AbusiveSergeant, Move::TargetMinion(0, 0)));
			CHECK_DO_MOVE(Move::EndTurn( ));
			CHECK_DO_MOVE(Move::AttackMinion(0, 0));
			CHECK(GetNumMinions(g, 0) == 1);
			CHECK(GetNumMinions(g, 1) == 1);
			CHECK(GetMinion(g, 1, 0).m_health == 2);

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

			CHECK_DO_MOVE(Move::PlayCard(Card::AbusiveSergeant, Move::TargetMinion(0, 0)));
			CHECK_DO_MOVE(Move::EndTurn( ));
			CHECK_DO_MOVE(Move::EndTurn( ));
			CHECK_DO_MOVE(Move::AttackMinion(0, 0));
			CHECK(GetNumMinions(g, 0) == 1);
			CHECK(GetNumMinions(g, 1) == 1);
			CHECK(GetMinion(g, 1, 0).m_health == 2);

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

			CHECK_DO_MOVE(Move::EndTurn( ));
			CHECK_DO_MOVE(Move::AttackMinion(0, 0));
			CHECK(GetNumMinions(g, 0) == 0);
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

			CHECK(GetMinion(g,0,0).HasStealth( ));
			CHECK_DO_MOVE(Move::AttackHero(0));
			CHECK(!GetMinion(g,0,0).HasStealth( ));

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

			CHECK_DO_MOVE(Move::EndTurn( ));
			CHECK_MOVE_IMPOSSIBLE(Move::AttackMinion(0, 0));

			return true;
		}
	},
	{
		"Stealth minion with taunt does not prevent attacking hero or other minions", []( )
		{
			GameState g;
			AddMinion(g, 0, Card::WorgenInfiltrator);
			AddMinion(g, 0, Card::BloodfenRaptor);
			GetMinion(g,0,0).AddAbility(MinionAbilityFlags::Taunt);
			AddMinion(g, 1, Card::SenjinShieldMasta);
			g.UpdatePossibleMoves( );

			CHECK(GetMinion(g,0,0).HasStealth( ));
			CHECK(GetMinion(g,0,0).HasTaunt( ));
			CHECK_DO_MOVE(Move::EndTurn( ));
			CHECK_MOVE_POSSIBLE(Move::AttackHero(0));
			CHECK_MOVE_POSSIBLE(Move::AttackMinion(0, 1));
			CHECK_MOVE_IMPOSSIBLE(Move::AttackMinion(0, 0));

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

			CHECK_DO_MOVE(Move::EndTurn( ));
			CHECK_MOVE_IMPOSSIBLE(Move::PlayCard(Card::ElvenArcher, Move::TargetMinion(0, 0)));

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

			CHECK_DO_MOVE(Move::PlayCard(Card::ElvenArcher, Move::TargetMinion(0, 0)));
			CHECK(g.m_players[0].m_minions.Num( ) == 1);

			return true;
		}
	},
	{
		"Unstable Ghoul deathrattle", []( )
		{
			GameState g;
			AddMinion(g, 0, Card::UnstableGhoul);
			AddMinion(g, 0, Card::BloodfenRaptor);
			AddMinion(g, 1, Card::BloodfenRaptor);
			AddMinion(g, 1, Card::BloodfenRaptor);
			g.UpdatePossibleMoves( );

			CHECK_DO_MOVE(Move::EndTurn( ));
			CHECK_DO_MOVE(Move::AttackMinion(0, 0));
			CHECK(GetPlayerHealth(g, 0) == GameState::StartingHealth);
			CHECK(GetPlayerHealth(g, 1) == GameState::StartingHealth);
			CHECK(GetNumMinions(g, 0) == 1);
			CHECK(GetNumMinions(g, 1) == 1);
			CHECK(GetMinion(g, 0, 0).m_health == 1);
			CHECK(GetMinion(g, 1, 0).m_health == 1);

			return true;
		}
	},
	{
		"Abomination deathrattle", []( )
		{
			GameState g;
			AddMinion(g, 0, Card::Abomination);
			AddMinion(g, 0, Card::BloodfenRaptor);
			AddMinion(g, 1, Card::SpitefulSmith);
			AddMinion(g, 1, Card::BloodfenRaptor);
			g.UpdatePossibleMoves( );

			CHECK_DO_MOVE(Move::EndTurn( ));
			CHECK_DO_MOVE(Move::AttackMinion(0, 0));
			CHECK(GetNumMinions(g, 0) == 0);
			CHECK(GetNumMinions(g, 1) == 0);
			CHECK(GetPlayerHealth(g, 0) == GameState::StartingHealth - 2);
			CHECK(GetPlayerHealth(g, 1) == GameState::StartingHealth - 2);

			return true;
		}
	},
	{
		"Abomination deathrattle kills leper gnomes", []( )
		{
			GameState g;
			AddMinion(g, 0, Card::Abomination);
			AddMinion(g, 0, Card::LeperGnome);
			AddMinion(g, 0, Card::LeperGnome);
			AddMinion(g, 1, Card::SpitefulSmith);
			AddMinion(g, 1, Card::BloodfenRaptor);
			g.UpdatePossibleMoves( );

			CHECK_DO_MOVE(Move::EndTurn( ));
			CHECK_DO_MOVE(Move::AttackMinion(0, 0));
			CHECK(GetNumMinions(g, 0) == 0);
			CHECK(GetNumMinions(g, 1) == 0);
			CHECK(GetPlayerHealth(g, 0) == GameState::StartingHealth - 2);
			CHECK(GetPlayerHealth(g, 1) == GameState::StartingHealth - 2 - 2 - 2);

			return true;
		}
	},
	{
		"Abominations kill each other", []( )
		{
			GameState g;
			AddMinion(g, 0, Card::Abomination);
			AddMinion(g, 0, Card::DarkIronDwarf);
			AddMinion(g, 0, Card::DarkIronDwarf);
			AddMinion(g, 1, Card::Abomination);
			AddMinion(g, 1, Card::RiverCrocolisk);
			g.UpdatePossibleMoves( );

			CHECK_DO_MOVE(Move::EndTurn( ));
			CHECK_DO_MOVE(Move::AttackMinion(0, 0));
			CHECK(GetNumMinions(g, 0) == 0);
			CHECK(GetNumMinions(g, 1) == 0);
			CHECK(GetPlayerHealth(g, 0) == GameState::StartingHealth - 2 - 2);
			CHECK(GetPlayerHealth(g, 1) == GameState::StartingHealth - 2 - 2);

			return true;
		}
	},
	{
		"Abomination can win game", []( )
		{
			GameState g;
			AddMinionReadyToAttack(g, 0, Card::Abomination);
			SetHealth(g, 1, 2);
			AddMinion(g, 1, Card::ChillwindYeti);
			g.UpdatePossibleMoves( );

			CHECK_DO_MOVE(Move::AttackMinion(0, 0));
			CHECK(g.m_winner == Winner::PlayerOne);

			return true;
		}
	},
	{
		"Novice Engineer", []( )
		{
			GameState g;
			AddCard(g, 0, Card::NoviceEngineer);
			SetManaAndMax(g, 0, 2);
			AddCardToDeck(g, 0, Card::Abomination);
			g.UpdatePossibleMoves( );

			CHECK_DO_MOVE(Move::PlayCard(Card::NoviceEngineer, Move::TargetPlayer(0)));
			CHECK(g.m_players[0].m_hand.Num( ) == 1);
			CHECK(g.m_players[0].m_hand[0] == Card::Abomination);

			return true;
		}
	},
	{
		"Battlecries are unaffected by spell damage", []( )
		{
			GameState g;
			AddCard(g, 0, Card::ElvenArcher);
			AddMinion(g, 0, Card::AzureDrake);
			SetManaAndMax(g, 0, 1);
			g.UpdatePossibleMoves( );

			CHECK_DO_MOVE(Move::PlayCard(Card::ElvenArcher, Move::TargetPlayer(1)));
			CHECK(GetPlayerHealth(g, 1) == 29);
			
			return true;
		}
	},
	{
		"Spells are affected by spell damage", []( )
		{
			GameState g;
			AddCard(g, 0, Card::HolySmite);
			AddMinion(g, 0, Card::AzureDrake);
			SetManaAndMax(g, 0, 1);
			g.UpdatePossibleMoves( );

			CHECK_DO_MOVE(Move::PlayCard(Card::HolySmite, Move::TargetPlayer(1)));
			CHECK(GetPlayerHealth(g, 1) == 27);

			return true;
		}
	},
	{
		"Priestess of Elune", []( )
		{
			GameState g;
			AddCard(g, 0, Card::PriestessOfElune);
			SetManaAndMax(g, 0, 6);
			SetHealth(g, 0, 20);
			g.UpdatePossibleMoves( );

			CHECK_DO_MOVE(Move::PlayCard(Card::PriestessOfElune, Move::TargetPlayer(0)));
			CHECK(GetPlayerHealth(g, 0) == 24);

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
