#pragma once

#include "Cards.h"

#include <cstdint>
#include <utility>

template< typename T, unsigned _Capacity, typename SizeType >
class FixedVector
{
	static const SizeType Capacity = _Capacity;

	T Data[Capacity];
	SizeType Size;

public:
	FixedVector()
		: Size(0)
	{
	}

	inline void Add(const T& t)
	{
		if (Size != Capacity)
		{
			Data[Size] =t;
			++Size;
		}
	}

	inline SizeType Num() const
	{
		return Size;
	}

	inline const T& operator[](SizeType index) const
	{
		return Data[index];
	}

	inline T& operator[](SizeType index)
	{
		return Data[index];
	}

	inline void Clear()
	{
		Size = 0;
	}

	inline T PopBack()
	{
		--Size;
		return Data[Size];
	}

	inline void RemoveAt(SizeType Index)
	{
		--Size;
		if (Index != Size)
		{
			memmove(&Data[Index], &Data[Index + 1], (Size - Index) * sizeof(T));
		}
	}

	inline void Set(const T* source, SizeType num)
	{
		memcpy(&Data[0], source, num * sizeof(T));
		Size = num;
	}

	inline void Shuffle() // TODO: Use ,random>
	{
		for (SizeType i = 0; i < Size; ++i)
		{
			SizeType j = (rand() % (Size - i)) + i;
			std::swap(Data[i], Data[j]);
		}
	}
};

enum class EWinner : int8_t
{
	Undetermined = -1,
	PlayerOne = 0,
	PlayerTwo = 1,
	Draw = 2
};


enum class MoveType
{
	EndTurn,
	AttackMinion,
	AttackHero,
	PlayCard,
};

struct Move
{
	MoveType Type;
	uint8_t SourceIndex;
	uint8_t TargetIndex;
};

struct Minion
{
	uint8_t Attack;
	int8_t Health;
	const CardData* SourceCard;
	bool AttackedThisTurn;
	bool SummonedThisTurn;

	inline void BeginTurn()
	{
		AttackedThisTurn = false;
		SummonedThisTurn = false;
	}

	const char* GetName() const;
};

struct Player
{
	int8_t Health;
	uint8_t MaxMana;
	uint8_t Mana;

	FixedVector<Minion, 7, uint8_t> Minions;
	FixedVector<Card, 10, uint8_t> Hand;
	FixedVector<Card, 30, uint8_t> Deck;

	inline void DrawOne()
	{
		if (Deck.Num())
		{
			Hand.Add(Deck.PopBack());
		}
	}
};

struct GameState
{
	static const uint8_t MaxPossibleMoves = 1 + 8 * 8 + 10 * 16;

	Player Players[2];
	EWinner Winner;
	int8_t ActivePlayerIndex;
	FixedVector<Move, MaxPossibleMoves, uint8_t> PossibleMoves;

	GameState();
	GameState(const GameState& other);

	void ProcessMove(const Move& m);
	void PlayOutRandomly();
	void UpdatePossibleMoves();

	void PrintMove(const Move& m) const;
	void PrintState() const;

protected:
	void EndTurn();
	void PlayCard(uint8_t SourceIndex, uint8_t TargetIndex);
	void AttackHero(uint8_t SourceIndex);
	void AttackMinion(uint8_t SourceIndex, uint8_t TargetIndex);
};