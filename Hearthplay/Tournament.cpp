#include "Tournament.h"
#include "GameState.h"
#include "MCTS.h"
#include "Clock.h"

#include <thread>
#include <future>

#if 0
#define DEBUG_GAME(...) __VA_ARGS__
#else
#define DEBUG_GAME(...)
#endif

const char* AINames[] = {
	"Random",
	"CheatingMCTS",
	"DetMCTS",
	"SO-IS-MCTS",
};


static GameState SetupGame(const Card(&deck)[30], std::mt19937& r)
{
	GameState game;
	game.m_players[0].m_deck.Set(deck, sizeof(deck) / sizeof(Card));
	game.m_players[0].m_deck.Shuffle(r);
	game.m_players[1].m_deck.Set(deck, sizeof(deck) / sizeof(Card));
	game.m_players[1].m_deck.Shuffle(r);

	game.m_players[0].DrawOne( );
	game.m_players[0].DrawOne( );
	game.m_players[0].DrawOne( );
	game.m_players[0].DrawOne( );
	game.m_players[0].m_mana = 1;
	game.m_players[0].m_max_mana = 1;

	game.m_players[1].DrawOne( );
	game.m_players[1].DrawOne( );
	game.m_players[1].DrawOne( );
	game.m_players[1].DrawOne( );
	game.m_players[1].m_hand.Add(Card::Coin);

	game.UpdatePossibleMoves( );
	return game;
}

Move PlayRandomMove(const GameState& state)
{
	uint16_t idx = rand( ) % state.m_possible_moves.Num( );
	return state.m_possible_moves[idx];
}

static PlayFunction PlayFunctions[] =
{
	PlayRandomMove,
	[](const GameState& state) { return CheatingMCTS::ChooseMove(state, 1000); },
	[](const GameState& state) { return DeterminizedMCTS::ChooseMove(state, 10, 100); },
	[](const GameState& state) { return SO_IS_MCTS::ChooseMove(state, 1000); },
};

PlayResults::PlayResults( )
{
	memset(&m_results, 0, sizeof(m_results));
}

void PlayResults::AddResult(AIType player_one, AIType player_two, Winner Winner)
{
	PairingResults& res = m_results[(uint32_t)player_two * (uint32_t)AIType::MAX + (uint32_t)player_one];
	switch (Winner)
	{
	case Winner::PlayerOne:
		res.m_player_one_wins++;
		break;
	case Winner::PlayerTwo:
		res.m_player_two_wins++;
		break;
	case Winner::Draw:
		res.m_draws++;
		break;
	}
}

void PlayResults::AddResults(const PlayResults& other)
{
	for (uint32_t i = 0; i < sizeof(m_results) / sizeof(PairingResults); ++i)
	{
		m_results[i].m_player_one_wins += other.m_results[i].m_player_one_wins;
		m_results[i].m_player_two_wins += other.m_results[i].m_player_two_wins;
		m_results[i].m_draws += other.m_results[i].m_draws;
	}
}

void PlayResults::Print( ) const
{
	printf("| Matchup | Player One Wins | Player Two Wins | Draws |\n");
	printf("| ------------- | ------------- | ------------- | ------------- |\n");

	for (AIType player_one = AIType::Random; player_one != AIType::MAX; player_one = (AIType)(1 + (int)player_one))
	{
		for (AIType player_two = AIType::Random; player_two != AIType::MAX; player_two = (AIType)(1 + (int)player_two))
		{
			const PairingResults& res = m_results[(uint32_t)player_two * (uint32_t)AIType::MAX + (uint32_t)player_one];
			if (res.m_draws + res.m_player_one_wins + res.m_player_two_wins != 0)
			{
				printf("| %s vs %s | %d | %d | %d |\n",
					   AINames[(int)player_one],
					   AINames[(int)player_two],
					   res.m_player_one_wins, res.m_player_two_wins, res.m_draws
					   );
			}
		}
	}
}

static Winner PlayGame(std::mt19937& r, const Card(&deck)[30], AIType player_one, AIType player_two)
{
	GameState game = SetupGame(deck, r);
	while (game.m_winner == Winner::Undetermined)
	{
		DEBUG_GAME(
			printf("\n");
		game.PrintState( );
		printf("\n");
		);

		Move m = Move::EndTurn( );
		if (game.m_active_player_index == 0)
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
	return game.m_winner;
}

void AITournament( uint32_t num_rounds, PlayResults& results )
{
	std::mt19937 r(GlobalRandomDevice( ));
	Card deck[30];
	for (uint32_t i = 0; i < num_rounds; ++i)
	{
		if ((i % 10) == 0)
		{
			std::uniform_int_distribution<uint32_t> deck_dist(0, DeckPossibleCards.size( ) - 1);
			for (Card& c : deck)
			{
				c = DeckPossibleCards[deck_dist(r)];
			}
		}

		for (AIType player_one = AIType::Random; player_one != AIType::MAX; player_one = (AIType)(1 + (int)player_one))
		{
			for (AIType player_two = AIType::Random; player_two != AIType::MAX; player_two = (AIType)(1 + (int)player_two))
			{
				Winner winner = PlayGame(r, deck, player_one, player_two);
				results.AddResult(player_one, player_two, winner);
			}
		}
	}
}

void AITournamentMT( uint32_t total_rounds, PlayResults& out_results )
{
	auto job = []( uint32_t rounds ) {
		PlayResults res;
		AITournament(rounds, res);
		return res;
	};

	unsigned num_jobs = std::thread::hardware_concurrency( );
	std::vector< std::future<PlayResults> > job_results;
	for (unsigned i = 0; i < num_jobs; ++i)
	{
		uint32_t rounds = total_rounds / num_jobs;
		if (rounds > (total_rounds - i * rounds))
		{
			rounds = total_rounds - i * rounds;
		}

		job_results.emplace_back(std::async(job, rounds));
	}

	for (auto& f : job_results)
	{
		out_results.AddResults(f.get( ));
	}
}