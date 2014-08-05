#pragma once

#include <memory>
#include <random>

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
	FixedVector( )
		: Size(0)
	{
	}

	inline SizeType Add(const T& t)
	{
		if (Size != Capacity)
		{
			Data[Size] = t;
			++Size;
			return Size - 1;
		}
		return Size;
	}

	inline SizeType Num( ) const
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

	inline void Clear( )
	{
		Size = 0;
	}

	inline T PopBack( )
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
		std::swap(Data[Index], Data[Size - 1]);
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

	inline void Shuffle(std::mt19937& r) // TODO: Use ,random>
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