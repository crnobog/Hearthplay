#include "MCTS.h"

#include <memory>
#include <random>

namespace SO_IS_MCTS
{ 
	struct MCTSNode
	{
		MCTSNode* Parent;
		Move ChosenMove; // The move that got us here from parent

		MCTSNode* Child;
		MCTSNode* Sibling;

		uint32_t Visits;
		uint32_t Wins;
		uint32_t Availability; // How often this node was available when its parent was visited

		MCTSNode()
			: Parent(nullptr)
			, Child(nullptr)
			, Sibling(nullptr)
			, Visits(0)
			, Wins(0)
			, Availability(0)
		{
		}

		MCTSNode(MCTSNode* parent, Move m)
			: Parent(parent)
			, Child(nullptr)
			, Sibling(nullptr)
			, ChosenMove(m)
			, Visits(0)
			, Wins(0)
			, Availability(0)
		{
		}

		inline auto GetUntriedMoves(const GameState& game) -> decltype(GameState::PossibleMoves)
		{
			auto moves = game.PossibleMoves;
			for (MCTSNode* node = Child; node; node = node->Sibling)
			{
				decltype(moves.Num()) idx;
				if (moves.Find(node->ChosenMove, idx))
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
			return Child != nullptr;
		}

		inline MCTSNode* UCTSelectChild( const GameState& state )
		{
			MCTSNode* best_child = nullptr;
			float best_score = -1.0f;

			for (MCTSNode* node = Child; node; node = node->Sibling)
			{
				if (!state.PossibleMoves.Contains(node->ChosenMove))
				{
					continue;
				}

				float score = (node->Wins / (float)node->Visits)
					+ (float)sqrtf(log((float)node->Availability) / node->Visits); // TODO tiebreak?
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
			std::uniform_int_distribution< decltype(GameState::PossibleMoves)::SizeType > dist(0, moves.Num() - 1);
			return moves[dist(r)];
		}

		inline MCTSNode* AddChild(Move m, MCTSNode* store)
		{
			MCTSNode* new_node = new(store) MCTSNode(this, m); // TODO placement
			if (Child == nullptr)
			{
				Child = new_node;
			}
			else
			{
				for (MCTSNode* node = Child; node; node = node->Sibling)
				{
					if (node->Sibling == nullptr)
					{
						node->Sibling = new_node;
						break;
					}
				}
			}
			return new_node;
		}
	};

	std::uniform_int_distribution<uint32_t> hand_distribution((unsigned)Card::Coin, (unsigned)Card::MAX - 1);
	std::uniform_int_distribution<uint32_t> deck_distribution((unsigned)Card::Coin + 1, (unsigned)Card::MAX - 1);

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
				for (MCTSNode* avail_node = node->Child; avail_node; avail_node = avail_node->Sibling)
				{
					if (sim_state.PossibleMoves.Contains(avail_node->ChosenMove))
					{
						avail_node->Availability++;
					}
				}

				sim_state.ProcessMove(next_node->ChosenMove);
				node = next_node;
			}
		
			// Expansion
			if (node->HasUntriedMoves(sim_state))
			{
				Move m = node->ChooseRandomUntriedMove(sim_state, r);
				for (MCTSNode* avail_node = node->Child; avail_node; avail_node = avail_node->Sibling)
				{
					if (sim_state.PossibleMoves.Contains(avail_node->ChosenMove))
					{
						avail_node->Availability++;
					}
				}

				sim_state.ProcessMove(m);
				node = node->AddChild(m, store_head);
				store_head += 1;
				node->Availability++;
			}

			// Simulation
			sim_state.PlayOutRandomly(r);
		
			// Backpropagation
			bool won = sim_state.Winner == (EWinner)game.ActivePlayerIndex;
			while (node)
			{
				node->Visits++;
				if (won) node->Wins++;

				node = node->Parent;
			}
		}

		MCTSNode* best_node = nullptr;
		uint32_t best_visits = 0;

		for (MCTSNode* node = root.Child; node; node = node->Sibling)
		{
			if (node->Visits > best_visits)
			{
				best_visits = node->Visits;
				best_node = node;
			}
		}

		Move m = best_node->ChosenMove;
		free(store);
		return m;
	}

}