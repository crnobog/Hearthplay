#include "MCTS.h"
#include "GameState.h"

#include <memory>
#include <math.h>

#if 0
#define MCTS_DEBUG(...) __VA_ARGS__
#else
#define MCTS_DEBUG(...) 
#endif

namespace CheatingMCTS
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
			: ChosenMove(Move::EndTurn())
		{
			memset(this, 0, sizeof(MCTSNode));
			UntriedMoves = state.PossibleMoves;
		}

		MCTSNode(MCTSNode* parent, Move m, const GameState& state)
			: ChosenMove(Move::EndTurn( ))
		{
			memset(this, 0, sizeof(MCTSNode));
			UntriedMoves = state.PossibleMoves;
			ChosenMove = m;
			Parent = parent;
		}

		MCTSNode(const MCTSNode& other) = delete;

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

		inline Move RemoveRandomUntriedMove(std::mt19937& r)
		{
			std::uniform_int_distribution<uint16_t> move_dist(0, UntriedMoves.Num( ) - 1);

			uint16_t index = move_dist(r);
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

	Move ChooseMove(const GameState& game, unsigned Iterations)
	{
		std::mt19937 r(GlobalRandomDevice());

		MCTSNode root(game);
		for (unsigned iter = 0; iter < Iterations; ++iter)
		{
			MCTS_DEBUG(printf("Iteration %d\n", iter));
			GameState sim_state(game);
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
				Move m = node->RemoveRandomUntriedMove(r);
				MCTS_DEBUG(printf("Expansion: "));
				MCTS_DEBUG(sim_state.PrintMove(m));
				sim_state.ProcessMove(m);
				node = node->AddChild(m, sim_state);
			}

			sim_state.PlayOutRandomly(r);
			bool won = sim_state.Winner == (EWinner)game.ActivePlayerIndex;
			MCTS_DEBUG(printf( "Simulation result: %d\n", sim_state.Winner));

			while (node)
			{
				node->Visits++;
				if (won) node->Wins++;
			
				node = node->Parent;
			}
		}

		uint32_t max_visits = 0;
		MCTSNode* best_child = nullptr;
		for( MCTSNode* node = root.Child.get(); node; node = node->Sibling.get() )
		{
			if (node->Visits > max_visits)
			{
				best_child = node;
				max_visits = node->Visits;
			}
		}
		return best_child->ChosenMove;
	}
}