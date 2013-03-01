/// nit - Noriter Framework
/// A Cross-platform Open Source Integration for Game-oriented Apps
///
/// http://www.github.com/ellongrey/nit
///
/// Copyright (c) 2013 by Jun-hyeok Jang
/// 
/// (see each file to see the different copyright owners)
/// 
/// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
/// 
/// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
/// 
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
/// THE SOFTWARE.
///
/// Author: ellongrey

#pragma once

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

// Short BitSet class

template <class T>
class TBitSet
{
public:
										TBitSet(): _value(0)					{ }
										TBitSet(T value): _value(value) 		{ }
										TBitSet(const TBitSet& src): _value(src._value) 	{ }

	operator 							T() const								{ return _value; }

	TBitSet& 							operator = (const TBitSet& src)			{ _value = src._value; return *this; }
	TBitSet& 							operator = (T value)					{ _value = value; return *this; }

	bool 								operator == (const TBitSet& src) const	{ return _value == src._value; }
	bool 								operator != (const TBitSet& src) const	{ return _value != src._value; }

	void 								clear()									{ _value = 0; }
	void 								set(T mask, bool flag)					{ if (flag) _value |= mask; else _value &= ~mask; }
	void 								add(T mask)								{ _value |= mask; }
	void 								remove(T mask)							{ _value &= ~mask; }

	bool 								empty() const							{ return !_value; }
	bool 								any(T mask) const						{ return (_value & mask) != 0; }
	bool 								all(T mask) const						{ return (_value & mask) == mask; }

private:
	T									_value;
};

typedef TBitSet<int> 					BitSet;
typedef TBitSet<u32>					u32BitSet;
typedef TBitSet<u16>					u16BitSet;
typedef TBitSet<u8>						u8BitSet;

////////////////////////////////////////////////////////////////////////////////

// Long BitSet class
// combines several 32-bit DWORDs together as one Long BitSet

template <uint NUM_DWORDS>
class TLongBitSet
{
public:
										TLongBitSet()							{ clear(); }
										TLongBitSet(const TLongBitSet& src);

	void								clear()									{ for (int i=0; i<NUM_DWORDS; ++i) _bits[i] = 0; }

	bool								contains(u32 bitNo) const;

	void								set(u32 bitNo, bool flag);
	void								add(u32 bitNo)							{ set(bitNo, true); }
	void								remove(u32 bitNo)						{ set(bitNo, false); }

	static int							dword_size()							{ return NUM_DWORDS; }
	static uint							size()									{ return NUM_DWORDS << 5; /* ndw * 32 */ }
	
	bool								empty();			

public:
	TLongBitSet 						operator ^ (const TLongBitSet& src) const;
	TLongBitSet 						operator & (const TLongBitSet& src) const;
	bool		 						operator == (const TLongBitSet& src) const;
	bool		 						operator != (const TLongBitSet& src) const;

private:
	u32									_bits[NUM_DWORDS];
};

////////////////////////////////////////////////////////////////////////////////

// Fixed Length Array

template <class T, uint SIZE>
class TFixedArray
{
public:
	T& 									operator [] (uint index) 				{ ASSERT(index < SIZE); return _array[index]; }
	const T& 							operator [] (uint index) const 			{ ASSERT(index < SIZE); return _array[index]; }

	void 								fill(const T& value) 					{ for (uint i=0; i<SIZE; ++i) { _array[i] = value; } }

	static uint 						size() 		 							{ return SIZE; }

	T* 									begin() 								{ return &_array[0]; }
	T* 									end() 									{ return &_array[SIZE]; }

	const T* 							begin() const 							{ return &_array[0]; }
	const T* 							end() const 							{ return &_array[SIZE]; }

public:
	typedef T 							value_type;

	const static uint 					NUM_ITEMS = SIZE;
	const static uint 					NUM_BYTES = SIZE * sizeof(T);

private:
	T 									_array[SIZE];
};

////////////////////////////////////////////////////////////////////////////////

template <class T>
class TPriorityQueue
{
public:
	typedef typename vector<T>::type container;
	typedef typename container::iterator iterator;

public:	
	TPriorityQueue() { }

public:
	bool 								empty()									{ return _array.empty(); }

	uint 								size()									{ return _array.size(); }

	uint 								push(const T& item);
	void 								pop();
	const T& 							top()									{ return _array.front(); }

	void								swap(TPriorityQueue<T>& other)			{ _array.swap(other._array); }
	void 								clear() 								{ _array.resize(0); }

	iterator 							begin()									{ return _array.begin(); }
	iterator 							end()									{ return _array.end(); }

	const T&							operator[] (u32 index)					{ return _array[index]; }

private:
	container 							_array;

	bool 								less(uint a, uint b)					{ return _array[a] < _array[b]; }
};

////////////////////////////////////////////////////////////////////////////////

// Fixed-capacity Queue

template <class T>
class TTightQueue
{
public:
	TTightQueue(u16 maxSize);
	~TTightQueue();

public:
	bool 								empty();
	bool 								full();

	void								clear();

public:
	u16 								size();
	u16 								max_size();

public:
	T& 									front();
	void 								push_front(const T& value);
	void 								pop_front();

public:
	T& 									back();
	void 								push_back(const T& value);
	void 								pop_back();

public:
	class iterator;
	friend class iterator;

	iterator							begin();
	iterator							end();

	T&									at(u16 index);
	T&									operator[] (u16 index);

private:
	u16									_maxSize;
	u16									_size;
	u16									_head;
	u16									_tail;
	T*									_array;
};

////////////////////////////////////////////////////////////////////////////////

template <uint NUM_DWORDS>
TLongBitSet<NUM_DWORDS>::TLongBitSet( const TLongBitSet& src )
{
	for (int i = 0; i < NUM_DWORDS; ++i)
	{
		_bits[i] = src._bits[i];
	}
}

template <uint NUM_DWORDS>
inline bool TLongBitSet<NUM_DWORDS>::contains(u32 bitNo) const
{
	ASSERT(bitNo < size());
	u32 ord = bitNo >> 5;
	int f = 1 << (bitNo - (ord << 5));
	return (_bits[ord] & f) != 0;
}

template <uint NUM_DWORDS>
inline void TLongBitSet<NUM_DWORDS>::set(u32 bitNo, bool flag)
{ 
	ASSERT(bitNo < size());
	u32 ord = bitNo >> 5; // bitNo / 32
	int f = 1 << (bitNo - (ord << 5)); // 1 << (bitNo % 32)
	_bits[ord] = flag ? (_bits[ord] | f) : (_bits[ord] & ~f);
}

template <uint NUM_DWORDS>
bool TLongBitSet<NUM_DWORDS>::empty()
{
	for (int i = 0; i < NUM_DWORDS; ++i)
	{
		if (_bits[i] != 0)
			return false;
	}

	return true;
}

template <uint NUM_DWORDS>
TLongBitSet<NUM_DWORDS> TLongBitSet<NUM_DWORDS>::operator^(const TLongBitSet& src) const
{
	TLongBitSet<NUM_DWORDS> out;
	for (uint i = 0; i < NUM_DWORDS; ++i)
		out._bits[i] = _bits[i] ^ src._bits[i];

	return out;
}

template <uint NUM_DWORDS>
TLongBitSet<NUM_DWORDS> TLongBitSet<NUM_DWORDS>::operator&(const TLongBitSet& src) const
{
	TLongBitSet<NUM_DWORDS> out;
	for (uint i = 0; i < NUM_DWORDS; ++i)
		out._bits[i] = _bits[i] & src._bits[i];

	return out;
}

template <uint NUM_DWORDS>
bool TLongBitSet<NUM_DWORDS>::operator!=(const TLongBitSet& src) const
{
	for (uint i = 0; i < NUM_DWORDS; ++i)
	{
		if (src._bits[i] != _bits[i])
			return true;
	}

	return false;
}
template <uint NUM_DWORDS>
bool TLongBitSet<NUM_DWORDS>::operator==(const TLongBitSet& src) const
{
	for (uint i = 0; i < NUM_DWORDS; ++i)
	{
		if (src._bits[i] == _bits[i])
			return true;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
uint TPriorityQueue<T>::push(const T& item)
{
	uint p = _array.size();
	uint p2;

	_array.push_back(item);

	do
	{
		if (p == 0)
			break;

		p2 = (p - 1) / 2;

		if (!less(p, p2))
			break;

		std::swap(_array[p], _array[p2]);
		p = p2;

	} while (true);

	return p;
}

// gets the *smallest* item and remove it (by less operator)
template <class T>
void TPriorityQueue<T>::pop()
{
	T result = _array[0];

	uint p = 0;
	uint p1, p2, pn;

	_array[0] = _array.back();
	_array.pop_back();

	do
	{
		pn = p;
		p1 = 2 * p + 1;
		p2 = 2 * p + 2;

		if (_array.size() > p1 && less(p1, p)) // left is smaller
			p = p1;
		if (_array.size() > p2 && less(p2, p)) // right is smaller
			p = p2;

		if (p == pn)
			break;

		std::swap(_array[p], _array[pn]);

	} while (true);

	return;
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
TTightQueue<T>::TTightQueue(u16 maxSize)
{
	ASSERT(maxSize > 1);
	ASSERT(maxSize < u16(-2));

	_maxSize = maxSize;
	_array = new T[maxSize];
	clear();
}

template <class T>
TTightQueue<T>::~TTightQueue()
{
	delete[] _array;
}

template <class T>
bool TTightQueue<T>::empty()
{
	return _size == 0;
}

template <class T>
bool TTightQueue<T>::full()
{
	return size() == _maxSize;
}

template <class T>
void TTightQueue<T>::clear()
{
	_head = 0;
	_tail = 0;
	_size = 0;
}

template <class T>
u16 TTightQueue<T>::size()
{
	return _size;
}

template <class T>
u16 TTightQueue<T>::max_size()
{
	return _maxSize;
}

template <class T>
T& TTightQueue<T>::front()
{
	ASSERT(!empty());

	u16 idx = _head;
	++idx;
	return _array[idx == _maxSize ? 0 : idx];
}

template <class T>
void TTightQueue<T>::push_front(const T& value)
{
	ASSERT(!full());

	if (empty())
	{
		++_tail;
		if (_tail == _maxSize)
			_tail = 0;
	}

	++_size;

	_array[_head] = value;

	if (_head == 0)
		_head = _maxSize;
	--_head;
}

template <class T>
void TTightQueue<T>::pop_front()
{
	ASSERT(!empty());

	--_size;

	++_head;
	if (_head == _maxSize)
		_head = 0;

	if (empty())
		_tail = _head;
}

template <class T>
T& TTightQueue<T>::back()
{
	ASSERT(!empty());

	u16 idx = _tail == 0 ? _maxSize : _tail;
	--idx;
	return _array[idx];
}

template <class T>
void TTightQueue<T>::push_back(const T& value)
{
	ASSERT(!full());

	if (empty())
	{
		if (_head == 0)
			_head = _maxSize;
		--_head;
	}

	++_size;

	_array[_tail] = value;

	++_tail;
	if (_tail == _maxSize)
		_tail = 0;
}

template <class T>
void TTightQueue<T>::pop_back()
{
	ASSERT(!empty());

	--_size;

	if (_tail == 0)
		_tail = _maxSize;
	--_tail;

	if (empty())
		_head = _tail;
}

template <class T>
class TTightQueue<T>::iterator : public std::iterator<std::bidirectional_iterator_tag, T>
{
public:
	iterator() { _queue = NULL; _idx = 0; }
	iterator(TTightQueue<T>* queue, u16 idx) : _queue(queue), _idx(idx) { }

	iterator& 	operator++ () 			{ ++_idx; return *this; }
	iterator 	operator++ (int) 		{ iterator tmp(*this); ++(*this); return tmp; }
	iterator& 	operator-- ()			{ --_idx; return *this; }
	iterator	operator-- (int)		{ iterator tmp(*this); --(*this); return tmp; }
	T& 			operator* ()			{ return _queue->at(_idx); }
	T*			operator-> ()			{ return &_queue->at(_idx); }

	friend bool	operator == (const iterator& a, const iterator& b)	{ return a._idx == b._idx; }
	friend bool operator != (const iterator& a, const iterator& b)	{ return a._idx != b._idx; }

private:
	TTightQueue<T>* 	_queue;
	u16 				_idx;
};

template <class T>
typename TTightQueue<T>::iterator TTightQueue<T>::begin()
{
	return iterator(this, 0);
}

template <class T>
typename TTightQueue<T>::iterator TTightQueue<T>::end()
{
	return iterator(this, _size);
}

template <class T>
T& TTightQueue<T>::at(u16 index)
{
	ASSERT(index < _size);

	u16 idx = u16(_head + 1 + index);
	if (idx >= _maxSize) idx -= _maxSize;

	return _array[idx];
}

template <class T>
T& TTightQueue<T>::operator[] (u16 index)
{
	return at(index);
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
