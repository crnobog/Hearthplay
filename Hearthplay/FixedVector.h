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
	T			m_data[_Capacity];
	SizeType	m_size;
	
public:
	FixedVector( )
		: m_size(0)
	{
	}

	inline SizeType Add(const T& t)
	{
		if (m_size != Capacity)
		{
			m_data[m_size] = t;
			++m_size;
			return m_size - 1;
		}
		return m_size;
	}

	inline SizeType Num( ) const
	{
		return m_size;
	}

	inline const T& operator[](SizeType index) const
	{
		return m_data[index];
	}

	inline T& operator[](SizeType index)
	{
		return m_data[index];
	}

	inline void Clear( )
	{
		m_size = 0;
	}

	inline T PopBack( )
	{
		--m_size;
		return m_data[m_size];
	}

	inline void RemoveAt(SizeType Index)
	{
		--m_size;
		if (Index != m_size)
		{
			memmove(&m_data[Index], &m_data[Index + 1], (m_size - Index) * sizeof(T));
		}
	}

	inline void RemoveSwap(SizeType Index)
	{
		std::swap(m_data[Index], m_data[m_size - 1]);
		--m_size;
	}

	inline void RemoveOne(const T& t)
	{
		for (SizeType i = 0; i < m_size; ++i)
		{
			if (m_data[i] == t)
			{
				RemoveAt(i);
				return;
			}
		}
	}

	inline void Set(const T* source, SizeType num)
	{
		memcpy(&m_data[0], source, num * sizeof(T));
		m_size = num;
	}

	inline void Shuffle(std::mt19937& r) // TODO: Use ,random>
	{
		for (SizeType i = 0; i < m_size; ++i)
		{
			std::uniform_int_distribution<uint32_t> dist(i, m_size - 1); // Ideally use a template to generate shorts when SizeType is char
			SizeType j = (SizeType)dist(r);
			std::swap(m_data[i], m_data[j]);
		}
	}

	inline bool Contains(const T& t) const
	{
		for (SizeType i = 0; i < m_size; ++i)
		{
			if (m_data[i] == t)
			{
				return true;
			}
		}
		return false;
	}

	inline bool Find(const T& t, SizeType& idx) const
	{
		for (SizeType i = 0; i < m_size; ++i)
		{
			if (m_data[i] == t)
			{
				idx = i;
				return true;
			}
		}
		return false;
	}
};