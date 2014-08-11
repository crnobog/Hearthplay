#pragma once

#include <cstdint>

#include "GameState.h"

enum class AIType
{
	Random,
	CheatingMCTS,
	DeterminizedMCTS,
	SO_IS_MCTS,

	MAX
};

extern const char* AINames[(unsigned)AIType::MAX]; 

struct PairingResults
{
	uint32_t m_player_one_wins;
	uint32_t m_player_two_wins;
	uint32_t m_draws;
};

struct PlayResults
{
	PairingResults m_results[(uint32_t)AIType::MAX * (uint32_t)AIType::MAX];

	PlayResults( );
	void AddResult(AIType player_one, AIType player_two, Winner Winner);
	void AddResults(const PlayResults& other);
	void Print( ) const;
};

void AITournament( uint32_t rounds, PlayResults& results );
void AITournamentMT( uint32_t rounds, PlayResults& results );


typedef Move(*PlayFunction)(const GameState&);

