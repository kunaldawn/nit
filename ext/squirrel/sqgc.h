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

#ifndef _SQGC_H_
#define _SQGC_H_

#ifdef NO_GARBAGE_COLLECTOR
#define CHAINABLE_OBJ					SQRefCounted
#define ADD_TO_CHAIN(chain,obj)			((void)0)
#define INIT_CHAIN()					((void)0)
#define REMOVE_FROM_CHAIN(chain,obj)	((void)0)
#define GC_MUTATED(obj)					((void)0)
#endif

#ifndef NO_GARBAGE_COLLECTOR

//////////////////////////////////////////////////////////

class SQGC;

extern SQUnsignedInteger g_GCObjCount;

#define CHAINABLE_OBJ					SQCollectable
#define ADD_TO_CHAIN(chain,obj)			{(*chain)->enter(obj);}
#define INIT_CHAIN()					{_gcnext=NULL;_gcprev=NULL;_sharedstate=ss;}
#define REMOVE_FROM_CHAIN(chain,obj)	{(*chain)->leave(obj);}
#define GC_MUTATED(obj)					{_ss(obj)->_gc.mutated(obj);}

struct SQCollectable : public SQRefCounted
{
public:
	SQUnsignedInteger _gcmark;
	SQCollectable*	_gcprev;
	SQCollectable*	_gcnext;
	SQSharedState*	_sharedstate;

	virtual void						Finalize() = 0;
	virtual void						Mark(SQGC* gc) = 0;

	bool __gcDetach()
	{
		if (_gcprev == NULL && _gcnext == NULL)
			return false;

		if (_gcprev) _gcprev->_gcnext = _gcnext;
		if (_gcnext) _gcnext->_gcprev = _gcprev;
		_gcprev = NULL;
		_gcnext = NULL;

		return true;
	}
};

//////////////////////////////////////////////////////////

typedef SQBool (*SQGCVISITOR) (SQCollectable* obj, SQUserPointer up);
typedef void (*SQGCINFOCALLBACK) (SQGC* gc, SQUserPointer up);

class SQGCList
{
public:
	class Sentinel : public SQCollectable
	{
	public:
		virtual void					Finalize() { }
		virtual void					Mark(SQGC* gc) { }
		virtual void					Release() { }
	};

	Sentinel 							_head;
	Sentinel 							_tail;

	SQGCList();
	~SQGCList();

	inline bool isEmpty()
	{
		assert(_head._gcnext != &_tail || _tail._gcprev == &_head);
		return _head._gcnext == &_tail;
	}

	inline void push(SQCollectable* obj)
	{
		// Detach
		if (obj->_gcprev) obj->_gcprev->_gcnext = obj->_gcnext;
		if (obj->_gcnext) obj->_gcnext->_gcprev = obj->_gcprev;

		// Attach
		obj->_gcprev = _tail._gcprev;
		obj->_gcnext = &_tail;

		_tail._gcprev->_gcnext = obj;
		_tail._gcprev = obj;
	}

	inline SQCollectable* pop()
	{
		assert(!isEmpty());
		SQCollectable* obj = _tail._gcprev;
		obj->__gcDetach();
		return obj;
	}

	void								swap(SQGCList& other);

	SQBool								debugVisit(SQGCVISITOR visitor, SQUserPointer up);
	int									debugCountAll();
};

//////////////////////////////////////////////////////////

#if !defined(_SHIPPING)
#	define SQGC_DEBUG_STMT(stmt)			stmt
#else
#	define SQGC_DEBUG_STMT(stmt)
#endif

class SQGC
{
public:
	SQGC();
	~SQGC();

	inline void							enter(SQCollectable* obj)		{ obj->_gcmark = _curblack; _blacklist.push(obj); ++_entercount; SQGC_DEBUG_STMT(++_objtotal; ++_totalEnterCount); }
	inline void							leave(SQCollectable* obj)		{ if (obj->__gcDetach()) { ++_leavecount; SQGC_DEBUG_STMT(--_objtotal; ++_totalLeaveCount); } }
	inline void							mutated(SQCollectable* obj)		{ if (obj->_gcmark == _curblack) { obj->_gcmark = GRAY; _graylist.push(obj); ++_mutatecount; } }

	inline void							mark(SQCollectable* obj)		{ if (obj && obj->_gcmark == _curwhite) { obj->_gcmark = GRAY; _graylist.push(obj); ++_markcount; } }
	void								mark(SQObjectPtr &o);

	void								step(SQGCINFOCALLBACK infoCallback = NULL, SQUserPointer infoUP = NULL);
	SQInteger							fullSweep();
	SQInteger							finalize();

	void								startCycle();
	void								markLoop();
	void								sweepLoop();

	enum GCColor						{ GRAY = 0x00, COLOR0 = 0x01, COLOR1 = 0x02 };

	SQSharedState*						_ss;

	SQGCList							_whitelist;
	SQGCList							_graylist;
	SQGCList							_blacklist;

	SQInteger							_objtotal;
	SQInteger							_entercount;
	SQInteger							_leavecount;
	SQInteger							_mutatecount;

	SQInteger							_totalEnterCount;
	SQInteger							_totalLeaveCount;

	SQInteger							_markratio;
	SQInteger							_markcount;
	SQInteger							_marktotal;

	SQInteger							_sweepratio;
	SQInteger							_sweepcount;
	SQInteger							_sweeptotal;
	
	SQInteger							_curwhite;
	SQInteger							_curblack;

	SQBool								_fullcycle;
	SQBool								_finalizing;

	inline SQChar						debugColorChar(int gccolor)		{ return (gccolor == GRAY) ? _SC('g') : (gccolor == _curwhite) ? _SC('w') : (gccolor == _curblack) ? _SC('b') : _SC('?'); }
	inline void							debugPrintChange(SQCollectable* obj, SQInteger newcolor) { printf("%08x %c->%c\n", (int)obj, debugColorChar(obj->_gcmark), debugColorChar(newcolor)); }
	SQBool								debugVisitAll(SQGCVISITOR visitor, SQUserPointer up);
};

#endif // NO_GARBAGE_COLLECTOR

#endif // _SQGC_H_