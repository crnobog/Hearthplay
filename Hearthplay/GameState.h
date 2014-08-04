#pragma once

#include "Cards.h"

#include <cstdint>
#include <random>
#include <utility>

extern std::random_device GlobalRandomDevice;

template< typename T, unsigned _Capacity, typename _SizeType >
class FixedVector
{
public:
	typedef _SizeType SizeType;
	static const SizeType Capacity = _Capacity;

private:
	T Data[_Capacity];
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

	inline void RemoveSwap(SizeType Index)
	{
		std::swap(Data[Index], Data[Size-1]);
		--Size;
	}

	inline void RemoveOne(const T& t)
	{
		for (SizeType i = 0; i < Size; ++i)
		{
			if (Data[i] == t)
			{
				RemoveAt(i);
				return;
			}
		}
	}

	inline void Set(const T* source, SizeType num)
	{
		memcpy(&Data[0], source, num * sizeof(T));
		Size = num;
	}

	inline void Shuffle( std::mt19937& r ) // TODO: Use ,random>
	{
		for (SizeType i = 0; i < Size; ++i)
		{
			std::uniform_int_distribution<uint32_t> dist(i, Size - 1); // Ideally use a template to generate shorts when SizeType is char
			SizeType j = (SizeType)dist(r);
			std::swap(Data[i], Data[j]);
		}
	}

	inline bool Contains(const T& t) const
	{
		for (SizeType i = 0; i < Size; ++i)
		{
			if (Data[i] == t)
			{
				return true;
			}
		}
		return false;
	}

	inline bool Find(const T& t, SizeType& idx) const
	{
		for (SizeType i = 0; i < Size; ++i)
		{
			if (Data[i] == t)
			{
				idx = i;
				return true;
			}
		}
		return false;
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

	inline bool operator==(const Move& m) const
	{
		return Type == m.Type && SourceIndex == m.SourceIndex && TargetIndex == m.TargetIndex;
	}

};

inline bool operator<(const Move& l, const Move& r)
{
	if (l.Type == r.Type)
	{
		if (l.SourceIndex == r.SourceIndex)
		{
			return l.TargetIndex < r.TargetIndex;
		}
		return l.SourceIndex < r.SourceIndex;
	}

	return l.Type < r.Type;
}

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
	FixedVector<Move, MaxPossibleMoves, uint16_t> PossibleMoves;

	GameState();
	GameState(const GameState& other);

	void ProcessMove(const Move& m);
	void PlayOutRandomly( std::mt19937& r );
	void UpdatePossibleMoves();

	void PrintMove(const Move& m) const;
	void PrintState() const;

protected:
	void EndTurn();
	void PlayCard(uint8_t SourceIndex, uint8_t TargetIndex);
	void AttackHero(uint8_t SourceIndex);
	void AttackMinion(uint8_t SourceIndex, uint8_t TargetIndex);
};