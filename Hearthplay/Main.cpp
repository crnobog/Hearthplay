#include "GameState.h"
#include "Cards.h"
#include "MCTS.h"
#include "Clock.h"
#include "Tests.h"
#include "Tournament.h"

#include <cstdio>
#include <random>
#include <thread>

std::random_device GlobalRandomDevice;

GameState SetupGame(const Card (&deck)[30], std::mt19937& r)
{
	GameState game;
	game.m_players[0].m_deck.Set(deck, sizeof(deck) / sizeof(Card));
	game.m_players[0].m_deck.Shuffle(r);
	game.m_players[1].m_deck.Set(deck, sizeof(deck) / sizeof(Card));
	game.m_players[1].m_deck.Shuffle(r);

	game.m_players[0].DrawOne();
	game.m_players[0].DrawOne();
	game.m_players[0].DrawOne();
	game.m_players[0].DrawOne();
	game.m_players[0].m_mana = 1;
	game.m_players[0].m_max_mana = 1;

	game.m_players[1].DrawOne();
	game.m_players[1].DrawOne();
	game.m_players[1].DrawOne();
	game.m_players[1].DrawOne();
	game.m_players[1].m_hand.Add(Card::Coin);

	game.UpdatePossibleMoves();
	return game;
}

void BenchmarkRandomPlay(const Card(&deck)[30])
{
	std::mt19937 r(GlobalRandomDevice());
	for (int i = 0; i < 1000; ++i)
	{
		GameState game = SetupGame(deck, r);
		game.PlayOutRandomly(r);
	}
}

struct Setting
{
	std::string m_name;
	bool		m_enabled;
	bool		m_takes_uint;
	uint32_t	m_uint_value;

	Setting(const char* name, bool takes_uint)
		: m_name( name )
		, m_enabled( false )
		, m_takes_uint( takes_uint )
		, m_uint_value( 0 )
	{
	}
};

Setting Setting_RunTournament = { "-tournament", true };
Setting Setting_RunTournamentMT = { "-tournamentmt", true };
Setting Setting_Wait= { "-wait", false };
Setting Setting_RunTests= { "-runtests", false };
Setting Setting_PrintDeckPossibleCards = { "-printimplementedcards", false };

Setting* Settings[] = {
	&Setting_RunTournament,
	&Setting_RunTournamentMT,
	&Setting_RunTests,
	&Setting_Wait,
	&Setting_PrintDeckPossibleCards
};

int main(int argc, char** argv )
{
	FilterDeckPossibleCards( );

	for (int i = 1; i < argc; ++i)
	{
		bool found = false;
		for (int j = 0; j < sizeof(Settings) / sizeof(Setting*); ++j)
		{
			if (argv[i] == Settings[j]->m_name)
			{
				Settings[j]->m_enabled = true;

				if (Settings[j]->m_takes_uint)
				{
					int val = atoi(argv[i + 1]);
					Settings[j]->m_uint_value = (uint32_t)val;
					++i;
				}

				found = true;
				break;
			}
		}
		if (!found)
		{
			printf("Unrecognised argument %s\n", argv[i]);
		}
	}

	std::mt19937 r(GlobalRandomDevice( ));

	if (Setting_PrintDeckPossibleCards.m_enabled)
	{
		printf("Deck possible cards:\n");
		uint32_t num = 0;
		for (Card c : DeckPossibleCards)
		{
			const CardData* data = GetCardData(c);
			printf("%s\n", data->m_name);
			++num;
		}
		printf("%u cards\n", num);
	}

	if (Setting_RunTests.m_enabled)
	{
		RunTests();
	}

	if (Setting_RunTournamentMT.m_enabled)
	{
		printf("std::thread::hardware_concurrency: %u\n", std::thread::hardware_concurrency( ));

		const uint32_t num_rounds = Setting_RunTournamentMT.m_uint_value;
		auto tourn_start = std::chrono::system_clock::now( );
		printf("Playing %d rounds\n\n", num_rounds);

		PlayResults results;
		AITournamentMT(num_rounds, results);

		auto tourn_end = std::chrono::system_clock::now( );
		auto duration_min = std::chrono::duration_cast<std::chrono::seconds>(tourn_end - tourn_start).count( ) / 60.0f;
		printf("\nPlayed %u rounds in %.2f minutes\n\n", num_rounds, duration_min);

		results.Print( );
	}

	if (Setting_RunTournament.m_enabled)
	{
		const uint32_t num_rounds = Setting_RunTournament.m_uint_value;
		auto tourn_start = std::chrono::system_clock::now( );
		printf("Playing %d rounds\n\n", num_rounds);

		PlayResults results;
		AITournament(num_rounds, results);

		auto tourn_end = std::chrono::system_clock::now( );
		auto duration_min = std::chrono::duration_cast<std::chrono::seconds>(tourn_end - tourn_start).count( ) / 60.0f;
		printf("\nPlayed %u rounds in %.2f minutes\n\n", num_rounds, duration_min);

		results.Print( );
	}

	if (Setting_Wait.m_enabled)
	{
		getc(stdin);
	}

	return 0;
}