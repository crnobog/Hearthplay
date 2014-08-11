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
		MCTSNode*					m_parent;
		std::unique_ptr<MCTSNode>	m_child;
		std::unique_ptr<MCTSNode>	m_sibling;

		Move m_move; // The move that got us here from Parent
		decltype(GameState::m_possible_moves) m_num_untried_moves;

		uint32_t m_visits;
		uint32_t m_wins;

		MCTSNode(const GameState& state)
			: m_move(Move::EndTurn())
		{
			memset(this, 0, sizeof(MCTSNode));
			m_num_untried_moves = state.m_possible_moves;
		}

		MCTSNode(MCTSNode* parent, Move m, const GameState& state)
			: m_move(Move::EndTurn( ))
		{
			memset(this, 0, sizeof(MCTSNode));
			m_num_untried_moves = state.m_possible_moves;
			m_move = m;
			m_parent = parent;
		}

		MCTSNode(const MCTSNode& other) = delete;

		inline bool HasUntriedMoves()
		{
			return m_num_untried_moves.Num() != 0;
		}

		inline bool HasChildren()
		{
			return m_child != nullptr;
		}

		inline MCTSNode* UCTSelectChild()
		{
			MCTSNode* best_child = nullptr;
			float best_score = -1.0f;

			for (MCTSNode* node = m_child.get(); node; node = node->m_sibling.get())
			{
				float score = (node->m_wins / (float)node->m_visits)
					+ (float)sqrtf(log((float)m_visits) / node->m_visits); // TODO tiebreak?
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
			std::uniform_int_distribution<uint16_t> move_dist(0, m_num_untried_moves.Num( ) - 1);

			uint16_t index = move_dist(r);
			Move m = m_num_untried_moves[index];
			m_num_untried_moves.RemoveAt(index); // TODO: RemoveSwap

			return m;
		}

		inline MCTSNode* AddChild(Move m, const GameState& state)
		{
			std::unique_ptr<MCTSNode> new_node = std::make_unique<MCTSNode>(this, m, state);

			for (MCTSNode* node = m_child.get(); node; node = node->m_sibling.get())
			{
				if (!node->m_sibling.get())
				{
					node->m_sibling = std::move(new_node);
					return node->m_sibling.get();
				}
			}

			m_child = std::move(new_node);
			return m_child.get();
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
				MCTS_DEBUG(sim_state.PrintMove(node->m_move));
				sim_state.ProcessMove(node->m_move);
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
			bool won = sim_state.m_winner == (Winner)game.m_active_player_index;
			MCTS_DEBUG(printf( "Simulation result: %d\n", sim_state.m_winner));

			while (node)
			{
				node->m_visits++;
				if (won) node->m_wins++;
			
				node = node->m_parent;
			}
		}

		uint32_t max_visits = 0;
		MCTSNode* best_child = nullptr;
		for( MCTSNode* node = root.m_child.get(); node; node = node->m_sibling.get() )
		{
			if (node->m_visits > max_visits)
			{
				best_child = node;
				max_visits = node->m_visits;
			}
		}
		return best_child->m_move;
	}
}