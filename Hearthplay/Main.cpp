#include "GameState.h"
#include "Cards.h"
#include "MCTS.h"
#include "Clock.h"
#include "Tests.h"
#include "Tournament.h"

#include <cstdio>
#include <random>

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
	bool		m_value;
};

Setting Setting_RunTournament = { "-tournament", false };
Setting Setting_Wait= { "-wait", false };
Setting Setting_RunTests= { "-runtests", false };
Setting Setting_PrintDeckPossibleCards = { "-printimplementedcards", false };

Setting* Settings[] = {
	&Setting_RunTournament,
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
				Settings[j]->m_value = true;
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

	if (Setting_PrintDeckPossibleCards.m_value)
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

	if (Setting_RunTests.m_value)
	{
		RunTests();
	}

	if (Setting_RunTournament.m_value)
	{
		const uint32_t num_rounds = 1;
		auto tourn_start = std::chrono::system_clock::now( );
		printf("Playing %d rounds\n\n", num_rounds);

		PlayResults results;
		AITournament(num_rounds, results);

		auto tourn_end = std::chrono::system_clock::now( );
		auto duration_min = std::chrono::duration_cast<std::chrono::seconds>(tourn_end - tourn_start).count( ) / 60.0f;
		printf("\nPlayed %u rounds in %.2f minutes\n\n", num_rounds, duration_min);

		results.Print( );
	}

	if (Setting_Wait.m_value)
	{
		getc(stdin);
	}

	return 0;
}