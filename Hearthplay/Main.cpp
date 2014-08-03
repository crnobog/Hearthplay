#include "GameState.h"
#include "Cards.h"
#include "MCTS.h"

#include <cstdio>

#if 0
#define DEBUG_GAME(...) __VA_ARGS__
#else
#define DEBUG_GAME(...)
#endif

GameState SetupGame(const Card (&deck)[30] )
{
	GameState game;
	game.Players[0].Deck.Set(deck, sizeof(deck) / sizeof(Card));
	game.Players[0].Deck.Shuffle();
	game.Players[1].Deck.Set(deck, sizeof(deck) / sizeof(Card));
	game.Players[1].Deck.Shuffle();

	game.Players[0].DrawOne();
	game.Players[0].DrawOne();
	game.Players[0].DrawOne();
	game.Players[0].DrawOne();
	game.Players[0].Mana = 1;
	game.Players[0].MaxMana = 1;

	game.Players[1].DrawOne();
	game.Players[1].DrawOne();
	game.Players[1].DrawOne();
	game.Players[1].DrawOne();
	game.Players[1].Hand.Add(Card::Coin);

	game.UpdatePossibleMoves();
	return game;
}

int main(int , char** )
{
	Card deck[] =
	{
		Card::MurlocRaider, Card::MurlocRaider, Card::MurlocRaider,
		Card::RiverCrocolisk, Card::RiverCrocolisk, Card::RiverCrocolisk,
		Card::BloodfenRaptor, Card::BloodfenRaptor, Card::BloodfenRaptor,
		Card::BloodfenRaptor, Card::BloodfenRaptor, Card::BloodfenRaptor,
		Card::MagmaRager, Card::MagmaRager, Card::MagmaRager,
		Card::ChillwindYeti, Card::ChillwindYeti, Card::ChillwindYeti,
		Card::OasisSnapjaw, Card::OasisSnapjaw, Card::OasisSnapjaw,
		Card::BoulderfistOgre, Card::BoulderfistOgre, Card::BoulderfistOgre,
		Card::CoreHound, Card::CoreHound, Card::CoreHound,
		Card::WarGolem, Card::WarGolem, Card::WarGolem,
	};

	const int games = 1000;
	int wins[2] = { 0, 0 };
	for (int i = 0; i < games; ++i)
	{
		GameState game = SetupGame(deck);
		while (game.Winner == EWinner::Undetermined)
		{
			DEBUG_GAME( 
				printf("\n");
				game.PrintState();
				printf("\n");
			)
			Move m;
			if (game.ActivePlayerIndex == 0)
			{
				m = game.PossibleMoves[rand() % game.PossibleMoves.Num()];
			}
			else
			{
				m = SO_IS_MCTS::ChooseMove(game, 1000);
			}
			DEBUG_GAME(game.PrintMove(m));
			game.ProcessMove(m);
		}

		if (game.Winner != EWinner::Draw)
		{
			wins[(uint8_t)game.Winner]++;
		}
		printf("Game %d result %d\n", i, game.Winner);
		printf("Player one (random) wins: %.1f%%\n", 100.0f * wins[0] / (float)(i+1));
		printf("Player two (SO-IS-MCTS) wins: %.1f%%\n", 100.0f * wins[1] / (float)(i+1));
	}

	printf("Player one (random) wins: %.1f%%\n", 100.0f * wins[0] / (float)games);
	printf("Player two (SO-IS-MCTS) wins: %.1f%%\n", 100.0f * wins[1] / (float)games);

	getc(stdin);

	return 0;
}