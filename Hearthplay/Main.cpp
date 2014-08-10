#include "GameState.h"
#include "Cards.h"
#include "MCTS.h"
#include "Clock.h"
#include "Tests.h"

#include <cstdio>
#include <random>

#if 0
#define DEBUG_GAME(...) __VA_ARGS__
#else
#define DEBUG_GAME(...)
#endif

std::random_device GlobalRandomDevice;

typedef Move (*PlayFunction)(const GameState&);


enum class AIType
{
	Random,
	CheatingMCTS,
	DeterminizedMCTS,
	SO_IS_MCTS,

	MAX
};

const char* AINames[] = {
	"Random",
	"CheatingMCTS",
	"DetMCTS",
	"SO-IS-MCTS",
};

struct PairingResults
{
	uint32_t PlayerOneWins;
	uint32_t PlayerTwoWins;
	uint32_t Draws;
};

struct PlayResults
{
	PairingResults Results[(uint32_t)AIType::MAX * (uint32_t)AIType::MAX];

	PlayResults()
	{
		memset(&Results, 0, sizeof(Results));
	}

	void AddResult(AIType player_one, AIType player_two, EWinner Winner)
	{
		PairingResults& res = Results[(uint32_t)player_two * (uint32_t)AIType::MAX + (uint32_t)player_one];
		switch (Winner)
		{
		case EWinner::PlayerOne:
			res.PlayerOneWins++;
			break;
		case EWinner::PlayerTwo:
			res.PlayerTwoWins++;
			break;
		case EWinner::Draw:
			res.Draws++;
			break;
		}
	}

	void Print()
	{
		printf("| Matchup | Player One Wins | Player Two Wins | Draws |\n");
		printf("| ------------- | ------------- | ------------- | ------------- |\n");

		for (AIType player_one = AIType::Random; player_one != AIType::MAX; player_one = (AIType)(1 + (int)player_one))
		{
			for (AIType player_two = AIType::Random; player_two != AIType::MAX; player_two = (AIType)(1 + (int)player_two))
			{
				PairingResults& res = Results[(uint32_t)player_two * (uint32_t)AIType::MAX + (uint32_t)player_one];
				if (res.Draws + res.PlayerOneWins + res.PlayerTwoWins != 0)
				{
					printf("| %s vs %s | %d | %d | %d |\n",
						AINames[(int)player_one],
						AINames[(int)player_two],
						res.PlayerOneWins, res.PlayerTwoWins, res.Draws
						);
				}
			}
		}
	}
};

Move PlayRandomMove(const GameState& state)
{
	uint16_t idx = rand() % state.PossibleMoves.Num();
	return state.PossibleMoves[idx];
}

PlayFunction PlayFunctions[] = 
{
	PlayRandomMove,
	[](const GameState& state) { return CheatingMCTS::ChooseMove(state, 1000); },
	[](const GameState& state) { return DeterminizedMCTS::ChooseMove(state, 10, 100); },
	[](const GameState& state) { return SO_IS_MCTS::ChooseMove(state, 1000); },
};

GameState SetupGame(const Card (&deck)[30], std::mt19937& r)
{
	GameState game;
	game.Players[0].m_deck.Set(deck, sizeof(deck) / sizeof(Card));
	game.Players[0].m_deck.Shuffle(r);
	game.Players[1].m_deck.Set(deck, sizeof(deck) / sizeof(Card));
	game.Players[1].m_deck.Shuffle(r);

	game.Players[0].DrawOne();
	game.Players[0].DrawOne();
	game.Players[0].DrawOne();
	game.Players[0].DrawOne();
	game.Players[0].m_mana = 1;
	game.Players[0].m_max_mana = 1;

	game.Players[1].DrawOne();
	game.Players[1].DrawOne();
	game.Players[1].DrawOne();
	game.Players[1].DrawOne();
	game.Players[1].m_hand.Add(Card::Coin);

	game.UpdatePossibleMoves();
	return game;
}

EWinner PlayGame(std::mt19937& r, const Card(&deck)[30], AIType player_one, AIType player_two)
{
	GameState game = SetupGame(deck, r);
	while (game.Winner == EWinner::Undetermined)
	{
		DEBUG_GAME(
			printf("\n");
		game.PrintState();
		printf("\n");
		);
		
		Move m = Move::EndTurn( );
		if (game.ActivePlayerIndex == 0)
		{
			m = PlayFunctions[(int)player_one](game);
		}
		else
		{
			m = PlayFunctions[(int)player_two](game);
		}
		DEBUG_GAME(game.PrintMove(m));
		game.ProcessMove(m);
	}
	return game.Winner;
}

void AITournament()
{
	std::mt19937 r(GlobalRandomDevice());
	PlayResults results;

	auto tourn_start = std::chrono::system_clock::now( );
	const int rounds = 1;
	printf("Playing %d rounds\n\n", rounds);

	Card deck[30];
	for (int i = 0; i < rounds; ++i)
	{
		if ((i % 10) == 0)
		{
			printf("New tournament deck:\n");
			std::uniform_int_distribution<uint32_t> deck_dist(0, DeckPossibleCards.size( ) - 1);
			for (Card& c : deck)
			{
				c = DeckPossibleCards[deck_dist(r)];
				const CardData* data = GetCardData(c);
				printf("%s\n", data->m_name);
			}

			if (i != 0)
			{
				printf("Results so far:\n");
				results.Print( );
			}
		}

		for (AIType player_one = AIType::Random; player_one != AIType::MAX; player_one = (AIType)(1 + (int)player_one))
		{
			for (AIType player_two = AIType::Random; player_two != AIType::MAX; player_two = (AIType)(1 + (int)player_two))
			{
				//auto t1 = HighResClock::now();
				EWinner winner = PlayGame(r, deck, player_one, player_two);
				//auto t2 = HighResClock::now();
				//auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( t2 - t1 );
				//printf("%s vs %s - Game finished in %.2f seconds - winner %d\n",
				//	AINames[(int)player_one],
				//	AINames[(int)player_two],
				//	duration.count() / 1000.0f,
				//	(int)winner );
				results.AddResult(player_one, player_two, winner);
			}
		}
	}

	auto tourn_end = std::chrono::system_clock::now( );
	auto duration_min = std::chrono::duration_cast<std::chrono::seconds>(tourn_end - tourn_start).count( ) / 60.0f;
	printf("\nPlayed %d rounds in %.2f minutes\n\n", rounds, duration_min);

	results.Print();
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
	std::string name;
	bool value;
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

	for (int i = 0; i < argc; ++i)
	{
		for (int j = 0; j < sizeof(Settings) / sizeof(Setting*); ++j)
		{
			if (argv[i] == Settings[j]->name)
			{
				Settings[j]->value = true;
			}
		}
	}

	std::mt19937 r(GlobalRandomDevice( ));

	if (Setting_PrintDeckPossibleCards.value)
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

	if (Setting_RunTests.value)
	{
		RunTests();
	}

	if (Setting_RunTournament.value)
	{
		AITournament();
	}

	if (Setting_Wait.value)
	{
		getc(stdin);
	}

	return 0;
}