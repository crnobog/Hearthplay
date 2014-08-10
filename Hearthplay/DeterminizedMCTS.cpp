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
		MCTSNode* m_parent;
		std::unique_ptr<MCTSNode> m_child;
		std::unique_ptr<MCTSNode> m_sibling;

		Move m_move; // The move that got us here from Parent
		decltype(GameState::m_possible_moves) m_num_untried_moves;

		uint32_t m_visits;
		uint32_t m_wins;

		MCTSNode(const GameState& state)
			: m_move(Move::EndTurn( ))
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

		MCTSNode(const MCTSNode& other)
			: m_parent(other.m_parent)
			, m_move(other.m_move)
			, m_num_untried_moves(other.m_num_untried_moves)
			, m_visits(other.m_visits)
			, m_wins(other.m_wins)
		{
			if (other.m_child.get())
			{
				m_child = std::make_unique<MCTSNode>(*other.m_child);
			}
			if (other.m_sibling.get())
			{
				m_sibling = std::make_unique<MCTSNode>(*other.m_sibling);
			}
		}

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

	static GameState Determinize(const GameState& game, std::mt19937& r)
	{
		GameState new_state(game);

		int8_t opponent_idx = (int8_t)abs(new_state.m_active_player_index - 1);
		Player& active = new_state.m_players[new_state.m_active_player_index];
		Player& opponent = new_state.m_players[opponent_idx];

		std::uniform_int_distribution<uint32_t> hand_distribution(0, DeckPossibleCards.size() );
		std::uniform_int_distribution<uint32_t> deck_distribution(0, DeckPossibleCards.size() - 1);

		// Randomize cards in opponent's hand
		for (uint8_t i = 0; i < opponent.m_hand.Num(); ++i)
		{
			auto idx = hand_distribution(r);
			Card c = idx == DeckPossibleCards.size() ? Card::Coin : DeckPossibleCards[idx];
			opponent.m_hand[i] = c;
		}

		// Shuffle my deck
		active.m_deck.Shuffle(r);

		// Randomize opponent's deck
		for (uint8_t i = 0; i < opponent.m_deck.Num(); ++i)
		{
			Card c = DeckPossibleCards[deck_distribution(r)];
			opponent.m_deck[i] = c;
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
				bool won = sim_state.m_winner == (EWinner)game.m_active_player_index;
				MCTS_DEBUG(printf("Simulation result: %d\n", sim_state.m_winner));

				while (node)
				{
					node->m_visits++;
					if (won) node->m_wins++;

					node = node->m_parent;
				}
			}

			for (MCTSNode* node = root.m_child.get(); node; node = node->m_sibling.get())
			{
				move_visits[node->m_move] += node->m_visits;
			}
		}

		Move best_move = Move::EndTurn();
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