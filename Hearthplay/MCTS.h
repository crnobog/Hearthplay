#pragma once

#include "GameState.h"

Move CheatingMCTS(const GameState& game, unsigned iterations);
Move DeterminizedMCTS(const GameState& game, unsigned determinizations, unsigned iterations);