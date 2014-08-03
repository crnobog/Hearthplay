#pragma once

#include "GameState.h"

namespace CheatingMCTS
{
	Move ChooseMove(const GameState& game, unsigned iterations);
}

namespace DeterminizedMCTS
{
	Move ChooseMove(const GameState& game, unsigned determinizations, unsigned iterations);
}

namespace SO_IS_MCTS
{
	Move ChooseMove(const GameState& game, unsigned iterations);
}