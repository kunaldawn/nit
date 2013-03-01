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

#include "nit/nit.h"

////////////////////////////////////////////////////////////////////////////////

#define COUNT_OF(C_ARRAY) (sizeof(C_ARRAY) / sizeof( (C_ARRAY)[0] ))

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

template <typename TClass>
void callDestructor(TClass& obj)
{
	obj.~TClass();
}

template <typename PTR>
void safeDelete(PTR*& ptr)
{
	if (ptr) 
	{ 
		delete ptr; 
		ptr = NULL; 
	}
}

template <typename TPointer>
void safeDeleteArray(TPointer**& ptr)
{
	if (ptr)
	{
		delete[] ptr;
		ptr = NULL;
	}
}

template <typename TBaseClass>
void incRefCount(TBaseClass* obj)
{
	obj->incRefCount();
}

template <typename TBaseClass>
void decRefCount(TBaseClass* obj)
{
	obj->decRefCount();
}

////////////////////////////////////////////////////////////////////////////////

struct DeleteFunc
{
	template <class _TypeClass>
	void operator() (_TypeClass* ptr)
	{
		if (ptr) delete ptr;
	}
};

struct IncRefFunc
{
	template <class _TypeClass>
	void operator() (_TypeClass* ptr)
	{
		SAFE_INC_REF(ptr);
	}
};

struct DecRefFunc
{
	template <class _TypeClass>
	void operator() (_TypeClass* ptr)
	{
		SAFE_DEC_REF(ptr);
	}
};


////////////////////////////////////////////////////////////////////////////////

template <class Operation>
struct Pair1stFunc
{
	Pair1stFunc(const Operation& op) : _op(op) { }
	template <class Pair> void operator () (Pair& p) { _op(p.first); }
	Operation _op;
};

template <class Operation>
inline Pair1stFunc<Operation> pair1st(const Operation& op)
{
	return Pair1stFunc<Operation>(op);
}

////////////////////////////////////////////////////////////////////////////////

template <class Operation>
struct Pair2ndFunc
{
	Pair2ndFunc(const Operation& op) : _op(op) { }
	template <class Pair> void operator () (Pair& p) { _op(p.second); }
	Operation _op;
};

template <class Operation>
inline Pair2ndFunc<Operation> pair2nd(const Operation& op)
{
	return Pair2ndFunc<Operation>(op);
}

////////////////////////////////////////////////////////////////////////////////

template <class Operation>
struct PairBothFunc
{
	PairBothFunc(const Operation& op) : _op(op) { }
	template <class Pair> void operator () (Pair& p) { _op(p.first); _op(p.second); }
	Operation _op;
};

template <class Operation>
inline PairBothFunc<Operation> pair_both(const Operation& op)
{
	return PairBothFunc<Operation>(op);
}

////////////////////////////////////////////////////////////////////////////////

template <typename TContainer> class _SafeItrImpl;

// WARNING: NOT COMPATIBLE WITH vector<> deque<> String which can invalidate all iterators on insertion

// Tested with std::set, std::map, std::list

template <typename TContainer>
class SafeItr
{
public:
	typedef typename TContainer::iterator TIterator;
	typedef typename TContainer::value_type TValueType;
	typedef _SafeItrImpl<TContainer> TImpl;

	explicit SafeItr()															{ _innerMost = NULL; }
	~SafeItr()																	{ stopAll(); }

public:
	bool 								begin(TContainer& container);
	bool 								begin(TContainer& container, TIterator pos);
	bool 								end();

	bool								hasBegun()								{ return _innerMost != NULL; }
	bool								hasOuter();
	bool								stopAll();

public:
	bool								hasNext()								{ return _innerMost && _innerMost->hasNext(); }
	TValueType&							next()									{ return _innerMost->next(); }

public:
	void								onClear()								{ if (_innerMost) _innerMost->doClear(); }
	void								onErase(TIterator& itr)					{ if (_innerMost) _innerMost->doErase(itr); }
	void								onInsert(TIterator& itr)				{ if (_innerMost) _innerMost->doExclude(itr); }
	void								onInsert(std::pair<TIterator, bool> r)	{ if (_innerMost && r.second) _innerMost->doExclude(r.first); }

private:
	TImpl*								_innerMost;

	// not allowed
	SafeItr(SafeItr& other)														{ }
	SafeItr&							operator = (SafeItr& other)				{ return *this; }
};

////////////////////////////////////////////////////////////////////////////////

template <typename TContainer>
inline bool SafeItr<TContainer>::begin(TContainer& container) 
{ 
	_innerMost = new TImpl(container, container.begin(), _innerMost); 
	return _innerMost->_outer == NULL; 
}

template <typename TContainer>
inline bool SafeItr<TContainer>::begin(TContainer& container, TIterator pos) 
{ 
	_innerMost = new TImpl(container, pos, _innerMost); 
	return _innerMost->_outer == NULL; 
}

template <typename TContainer>
inline bool SafeItr<TContainer>::end()
{ 
	if (_innerMost == NULL) return true; 
	TImpl* outer = _innerMost->_outer; 
	delete _innerMost; 
	_innerMost = outer; 
	return _innerMost == NULL; 
}

template <typename TContainer>
inline bool SafeItr<TContainer>::hasOuter()
{ 
	return _innerMost && _innerMost->_outer; 
}

template <typename TContainer>
inline bool SafeItr<TContainer>::stopAll()
{ 
	if (_innerMost == NULL) return false; 
	_innerMost->doDeleteAll(); 
	_innerMost = NULL; 
	return true; 
}

////////////////////////////////////////////////////////////////////////////////

template <typename TContainer>
class _SafeItrImpl
{
	typedef typename TContainer::iterator TIterator;
	typedef typename TContainer::value_type TValueType;

	friend class SafeItr<TContainer>;

	struct ExcludeNode
	{
		TIterator target;
		ExcludeNode* next;
	};

	TContainer&							_container;
	TIterator							_itr;
	_SafeItrImpl*						_outer;
	ExcludeNode*						_excludeList;

	_SafeItrImpl(TContainer& container, TIterator pos, _SafeItrImpl* outer)
		: _container(container), _itr(pos), _outer(outer), _excludeList(NULL)
	{
	}

	~_SafeItrImpl()
	{
		ExcludeNode* ex = _excludeList;
		while (ex)
		{
			ExcludeNode* n = ex->next;
			delete ex;
			ex = n;
		}
	}

	bool hasNext() 
	{ 
		return _itr != _container.end(); 
	}

	TValueType& next()
	{
		TValueType& v = (TValueType&)(*_itr++); 
		if (_excludeList) skipExcluded();
		return v; 
	}

	void doDeleteAll()
	{
		if (_outer) _outer->doDeleteAll();
		delete this;
	}

	void doClear()
	{
		if (_outer) _outer->doClear();
		_itr = _container.end();
	}

	void doErase(TIterator& itr)
	{
		if (_outer) _outer->doErase(itr);
		if (_itr == itr) ++_itr;
	}

	void doExclude(TIterator& itr)
	{
		if (_outer) _outer->doExclude(itr);

		ExcludeNode* n = new ExcludeNode();
		n->target = itr;
		n->next = _excludeList;
		_excludeList = n;
	}

	void skipExcluded()
	{
		bool retry;

		do
		{
			retry = false;
			for (ExcludeNode* ex = _excludeList; ex; ex = ex->next)
			{
				if (_itr == ex->target)
				{
					++_itr;
					retry = true;
					break;
				}
			}
		} while (_itr != _container.end() && retry);
	}
};

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
