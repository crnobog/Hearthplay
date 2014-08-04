#include "MCTS.h"

#include <memory>
#include <map>
#include <random>

#if 0
#define MCTS_DEBUG(...) __VA_ARGS__
#else
#define MCTS_DEBUG(...) 
#endif

namespace DeterminizedMCTS
{
	struct MCTSNode
	{
		MCTSNode* Parent;
		std::unique_ptr<MCTSNode> Child;
		std::unique_ptr<MCTSNode> Sibling;

		Move ChosenMove; // The move that got us here from Parent
		decltype(GameState::PossibleMoves) UntriedMoves;

		uint32_t Visits;
		uint32_t Wins;

		MCTSNode(const GameState& state)
		{
			memset(this, 0, sizeof(MCTSNode));
			UntriedMoves = state.PossibleMoves;
		}

		MCTSNode(MCTSNode* parent, Move m, const GameState& state)
		{
			memset(this, 0, sizeof(MCTSNode));
			UntriedMoves = state.PossibleMoves;
			ChosenMove = m;
			Parent = parent;
		}

		MCTSNode(const MCTSNode& other)
			: Parent(other.Parent)
			, ChosenMove(other.ChosenMove)
			, UntriedMoves(other.UntriedMoves)
			, Visits(other.Visits)
			, Wins(other.Wins)
		{
			if (other.Child.get())
			{
				Child = std::make_unique<MCTSNode>(*other.Child);
			}
			if (other.Sibling.get())
			{
				Sibling = std::make_unique<MCTSNode>(*other.Sibling);
			}
		}

		inline bool HasUntriedMoves()
		{
			return UntriedMoves.Num() != 0;
		}

		inline bool HasChildren()
		{
			return Child != nullptr;
		}

		inline MCTSNode* UCTSelectChild()
		{
			MCTSNode* best_child = nullptr;
			float best_score = -1.0f;

			for (MCTSNode* node = Child.get(); node; node = node->Sibling.get())
			{
				float score = (node->Wins / (float)node->Visits)
					+ (float)sqrtf(log((float)Visits) / node->Visits); // TODO tiebreak?
				if (score > best_score)
				{
					best_child = node;
					best_score = score;
				}
			}

			return best_child;
		}

		inline Move RemoveRandomUntriedMove()
		{
			uint16_t index = rand() % UntriedMoves.Num();
			Move m = UntriedMoves[index];
			UntriedMoves.RemoveAt(index); // TODO: RemoveSwap

			return m;
		}

		inline MCTSNode* AddChild(Move m, const GameState& state)
		{
			std::unique_ptr<MCTSNode> new_node = std::make_unique<MCTSNode>(this, m, state);

			for (MCTSNode* node = Child.get(); node; node = node->Sibling.get())
			{
				if (!node->Sibling.get())
				{
					node->Sibling = std::move(new_node);
					return node->Sibling.get();
				}
			}

			Child = std::move(new_node);
			return Child.get();
		}
	};

	std::uniform_int_distribution<uint16_t> hand_distribution((unsigned)Card::Coin, (unsigned)Card::MAX - 1);
	std::uniform_int_distribution<uint16_t> deck_distribution((unsigned)Card::Coin + 1, (unsigned)Card::MAX - 1);

	static GameState Determinize(const GameState& game, std::mt19937& r)
	{
		GameState new_state(game);

		int8_t opponent_idx = (int8_t)abs(new_state.ActivePlayerIndex - 1);
		Player& active = new_state.Players[new_state.ActivePlayerIndex];
		Player& opponent = new_state.Players[opponent_idx];

		// Randomize cards in opponent's hand
		for (uint8_t i = 0; i < opponent.Hand.Num(); ++i)
		{
			opponent.Hand[i] = (Card)hand_distribution(r);
		}

		// Shuffle my deck
		active.Deck.Shuffle(r);

		// Randomize opponent's deck
		for (uint8_t i = 0; i < opponent.Deck.Num(); ++i)
		{
			opponent.Deck[i] = (Card)deck_distribution(r);
		}

		// This should not have actually changed the possible moves, but just to be safe against future changes
		new_state.UpdatePossibleMoves();

		return new_state;
	}

	Move ChooseMove(const GameState& game, unsigned num_determinizations, unsigned num_iterations)
	{
		std::mt19937 r(GlobalRandomDevice());
		std::map<Move, uint32_t> move_visits;

		for (unsigned det = 0; det < num_determinizations; ++det)
		{
			GameState det_game = Determinize(game, r); // TODO: Determinize
			MCTSNode root(det_game);

			for (unsigned iter = 0; iter < num_iterations; ++iter)
			{
				MCTS_DEBUG(printf("Iteration %d\n", iter));
				GameState sim_state(det_game);
				MCTSNode* node = &root;

				// Fully expand each node before expanding its children
				while (!node->HasUntriedMoves() && node->HasChildren())
				{
					node = node->UCTSelectChild();
					MCTS_DEBUG(printf("Selection: "));
					MCTS_DEBUG(sim_state.PrintMove(node->ChosenMove));
					sim_state.ProcessMove(node->ChosenMove);
				}

				if (node->HasUntriedMoves())
				{
					Move m = node->RemoveRandomUntriedMove();
					MCTS_DEBUG(printf("Expansion: "));
					MCTS_DEBUG(sim_state.PrintMove(m));
					sim_state.ProcessMove(m);
					node = node->AddChild(m, sim_state);
				}

				sim_state.PlayOutRandomly(r);
				bool won = sim_state.Winner == (EWinner)game.ActivePlayerIndex;
				MCTS_DEBUG(printf("Simulation result: %d\n", sim_state.Winner));

				while (node)
				{
					node->Visits++;
					if (won) node->Wins++;

					node = node->Parent;
				}
			}

			for (MCTSNode* node = root.Child.get(); node; node = node->Sibling.get())
			{
				move_visits[node->ChosenMove] += node->Visits;
			}
		}

		Move best_move{ MoveType::EndTurn };
		uint32_t best_visits = 0;
		for (auto p : move_visits)
		{
			if (p.second > best_visits)
			{
				best_move = p.first;
				best_visits = p.second;
			}
		}

		return best_move;
	}
}