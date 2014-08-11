#include "MCTS.h"

#include <memory>
#include <random>

namespace SO_IS_MCTS
{ 
	struct MCTSNode
	{
		MCTSNode* m_parent;
		Move m_move; // The move that got us here from parent

		MCTSNode* m_child;
		MCTSNode* m_siblings;

		uint32_t m_visits;
		uint32_t m_wins;
		uint32_t m_availability; // How often this node was available when its parent was visited

		MCTSNode()
			: m_parent(nullptr)
			, m_move(Move::EndTurn( ))
			, m_child(nullptr)
			, m_siblings(nullptr)
			, m_visits(0)
			, m_wins(0)
			, m_availability(0)
		{
		}

		MCTSNode(MCTSNode* parent, Move m)
			: m_parent(parent)
			, m_move(m)
			, m_child(nullptr)
			, m_siblings(nullptr)
			, m_visits(0)
			, m_wins(0)
			, m_availability(0)
		{
		}

		inline auto GetUntriedMoves(const GameState& game) -> decltype(GameState::m_possible_moves)
		{
			auto moves = game.m_possible_moves;
			for (MCTSNode* node = m_child; node; node = node->m_siblings)
			{
				decltype(moves.Num()) idx;
				if (moves.Find(node->m_move, idx))
				{
					moves.RemoveSwap(idx);
				}
			}
			return moves;
		}

		inline bool HasUntriedMoves(const GameState& game)
		{
			return GetUntriedMoves(game).Num() > 0;
		}

		inline bool HasChildren()
		{
			return m_child != nullptr;
		}

		inline MCTSNode* UCTSelectChild( const GameState& state )
		{
			MCTSNode* best_child = nullptr;
			float best_score = -1.0f;

			for (MCTSNode* node = m_child; node; node = node->m_siblings)
			{
				if (!state.m_possible_moves.Contains(node->m_move))
				{
					continue;
				}

				float score = (node->m_wins / (float)node->m_visits)
					+ (float)sqrtf(log((float)node->m_availability) / node->m_visits); // TODO tiebreak?
				if (score > best_score)
				{
					best_child = node;
					best_score = score;
				}
			}

			return best_child;
		}

		inline Move ChooseRandomUntriedMove(const GameState& game, std::mt19937& r)
		{
			auto moves = GetUntriedMoves(game);
			std::uniform_int_distribution< decltype(GameState::m_possible_moves)::SizeType > dist(0, moves.Num() - 1);
			return moves[dist(r)];
		}

		inline MCTSNode* AddChild(Move m, MCTSNode* store)
		{
			MCTSNode* new_node = new(store) MCTSNode(this, m); // TODO placement
			if (m_child == nullptr)
			{
				m_child = new_node;
			}
			else
			{
				for (MCTSNode* node = m_child; node; node = node->m_siblings)
				{
					if (node->m_siblings == nullptr)
					{
						node->m_siblings = new_node;
						break;
					}
				}
			}
			return new_node;
		}
	};

	static GameState Determinize(const GameState& game, std::mt19937& r)
	{
		GameState new_state(game);

		int8_t opponent_idx = (int8_t)abs(new_state.m_active_player_index - 1);
		Player& active = new_state.m_players[new_state.m_active_player_index];
		Player& opponent = new_state.m_players[opponent_idx];

		std::uniform_int_distribution<uint32_t> hand_distribution(0, DeckPossibleCards.size());
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

	Move ChooseMove(const GameState& game, unsigned iterations)
	{
		std::mt19937 r(GlobalRandomDevice());
		MCTSNode root;

		MCTSNode* store = (MCTSNode*)malloc(sizeof(MCTSNode) * iterations);
		MCTSNode* store_head = store;

		for (unsigned iter = 0; iter < iterations; ++iter)
		{
			GameState sim_state = Determinize(game, r);

			// Selection
			MCTSNode* node = &root;
			while (!node->HasUntriedMoves(sim_state) && node->HasChildren())
			{
				MCTSNode* next_node = node->UCTSelectChild(sim_state);

				// Update availability
				for (MCTSNode* avail_node = node->m_child; avail_node; avail_node = avail_node->m_siblings)
				{
					if (sim_state.m_possible_moves.Contains(avail_node->m_move))
					{
						avail_node->m_availability++;
					}
				}

				sim_state.ProcessMove(next_node->m_move);
				node = next_node;
			}
		
			// Expansion
			if (node->HasUntriedMoves(sim_state))
			{
				Move m = node->ChooseRandomUntriedMove(sim_state, r);
				for (MCTSNode* avail_node = node->m_child; avail_node; avail_node = avail_node->m_siblings)
				{
					if (sim_state.m_possible_moves.Contains(avail_node->m_move))
					{
						avail_node->m_availability++;
					}
				}

				sim_state.ProcessMove(m);
				node = node->AddChild(m, store_head);
				store_head += 1;
				node->m_availability++;
			}

			// Simulation
			sim_state.PlayOutRandomly(r);
		
			// Backpropagation
			bool won = sim_state.m_winner == (Winner)game.m_active_player_index;
			while (node)
			{
				node->m_visits++;
				if (won) node->m_wins++;

				node = node->m_parent;
			}
		}

		MCTSNode* best_node = nullptr;
		uint32_t best_visits = 0;

		for (MCTSNode* node = root.m_child; node; node = node->m_siblings)
		{
			if (node->m_visits > best_visits)
			{
				best_visits = node->m_visits;
				best_node = node;
			}
		}

		Move m = best_node->m_move;
		free(store);
		return m;
	}

}