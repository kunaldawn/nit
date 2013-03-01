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

//#define NO_MEM_MANAGER

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

enum MemHint
{
	MEM_HINT_NONE = 0,
};

const static uint MEM_DEFAULT_ALIGNMENT = 16;
const static uint MEM_DEALLOC_SIZE_UNKNOWN = -1;

#define NIT_ALLOC(size)					(g_MemManager->Allocate(size, nit::MEM_DEFAULT_ALIGNMENT, nit::MEM_HINT_NONE))
#define NIT_REALLOC(ptr, osize, nsize)	(g_MemManager->reallocate(ptr, nsize, osize, nit::MEM_DEFAULT_ALIGNMENT, nit::MEM_HINT_NONE))
#define NIT_DEALLOC(ptr, size)			(g_MemManager->deallocate(ptr, size))

#ifndef NO_MEM_MANAGER

class MemAllocator;
class PooledAllocator;
class MemPool;
struct MemDebugInfo;

////////////////////////////////////////////////////////////////////////////////

struct NIT_API MemDebugInfo
{
	uint								counter;
	uint								size;
	uint								alignment;
	uint								reserved;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API MemPool
{
public:
	MemPool();
	MemPool(u16 entrySize, u16 byteAlignment, void* rawMemory, size_t size, MemDebugInfo* debugInfoArray);
	~MemPool();

public:
	void*								Allocate();
	bool								deallocate(void* ptr);

	bool								contains(void* ptr)						{ return _entryStart <= size_t(ptr) && size_t(ptr) < _entryEnd; }
	int									indexOf(void* ptr);

	u16									getEntrySize()							{ return _entrySize; }
	u16									getByteAlignment()						{ return _byteAlignment; }

	uint								getNumFree()							{ return _numFree; }
	uint								getNumEntries()							{ return _numEntries; }
	uint								getNumAllocated()						{ return _numEntries - _numFree; }

	size_t								getEntryStart()							{ return _entryStart; }
	size_t								getEntryEnd()							{ return _entryEnd; }

	MemDebugInfo*						getDebugInfoArray()						{ return _debugInfoArray; }
	void								setDebugInfoArray(MemDebugInfo* array);

	void								dump(bool print, uint& varMaxSize, uint& varTotalAllocated, uint& varTotalActual);

public:
	struct RangeLess
	{
		bool							operator () (MemPool* a, MemPool* b)	{ return a->_entryStart < b->_entryStart; }
		bool							operator () (MemPool* a, void* b)		{ return a->_entryEnd <= size_t(b); }
		bool							operator () (void* a, MemPool* b)		{ return size_t(a) < b->_entryStart; }
	};

	struct SizeLess
	{
		bool							operator () (MemPool* a, MemPool* b)	{ return a->_entrySize < b->_entrySize; }
		bool							operator () (MemPool* a, size_t b)		{ return a->_entrySize < b; }
		bool							operator () (size_t a, MemPool* b)		{ return a < b->_entrySize; }
	};

private:
	struct Entry
	{
		Entry*							next;	// 4 bytes
	};

	u16									_entrySize;
	u16									_byteAlignment;

	uint								_numEntries;
	uint								_numFree;
	uint								_minFree;

	Entry*								_headOfFreeList;

	void*								_rawMemory;

	size_t								_entryStart;
	size_t								_entryEnd;

	MemDebugInfo*						_debugInfoArray;

private:
	friend class						PooledAllocator;
	void								setup(u16 entrySize, u16 byteAlignment, void* allocated, size_t size, MemDebugInfo* debugInfoArray);

	inline Entry*						getEndSentinel()						{ return (Entry*) this; }
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API MemAllocator
{
public:
	MemAllocator(const char* name) : _name(name) { }
	virtual ~MemAllocator() { }

public:
	virtual void*						Allocate(size_t size, size_t alignment) = 0;
	virtual bool						deallocate(void* memory, size_t size) = 0;
	virtual void*						reallocate(void* memory, size_t newSize, size_t oldSize, size_t alignment) = 0;

	virtual void						dump() = 0;

protected:
	const char*							_name;
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API PooledAllocator : public MemAllocator
{
public:
										PooledAllocator(const char* name);
	virtual								~PooledAllocator();

public:
	virtual void*						Allocate(size_t size, size_t alignment);
	virtual bool						deallocate(void* memory, size_t size);
	virtual void*						reallocate(void* memory, size_t newSize, size_t oldSize, size_t alignment);

	virtual void						dump();

public:
	void								addPool(u16 entrySize, u16 byteAlignment, void* rawMemory, size_t size, MemDebugInfo* debugInfoArray = NULL);

private:
	const static int					NUM_MAX_POOLS = 64;

	MemPool								_pools[NUM_MAX_POOLS];
	MemPool*							_rangeList[NUM_MAX_POOLS];
	MemPool*							_sizeList[NUM_MAX_POOLS];
	uint								_numPools;

	void								sortRange();

	MemPool*							findPool(void* ptr);
	MemPool*							findFree(size_t size, size_t alignment);

	typedef MemPool**					Iterator;

	Iterator							rangeBegin()							{ return &_rangeList[0]; }
	Iterator							rangeEnd()								{ return &_rangeList[_numPools]; }

	Iterator							sizeBegin()								{ return &_sizeList[0]; }
	Iterator							sizeEnd()								{ return &_sizeList[_numPools]; }
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API HeapAllocator : public MemAllocator
{
public:
	HeapAllocator(const char* name) : MemAllocator(name)						{ }

public:
	virtual void*						Allocate(size_t size, size_t alignment);
	virtual bool						deallocate(void* memory, size_t size);
	virtual void*						reallocate(void* memory, size_t newSize, size_t oldSize, size_t alignment);

	virtual void						dump();
};

////////////////////////////////////////////////////////////////////////////////

class NIT_API MemManager
{
public:
	MemManager();
	~MemManager();

public:
	static MemManager*					getInstance()							{ return g_Instance; }
	
	static void							initialize(MemManager* inst);
	void								shutdown();

public:
	void*								Allocate(size_t size, size_t alignment, MemHint hint);
	bool								deallocate(void* memory, size_t size);
	void*								reallocate(void* memory, size_t newSize, size_t oldSize, size_t alignment, MemHint hint);

	void								dump(); // WARNING: Use only on main thread!
	void								dumpLog(const char* fmt, ...);

public:
	PooledAllocator*					getPool()								{ return &_pool; }
	HeapAllocator*						getHeap()								{ return &_heap; }

private:
	static MemManager*					g_Instance;	// you should Initialize it somewhere

	bool								_initialized;
	Mutex								_lock;

	void*								tryAlloc(size_t size, size_t alignment, bool poolOnly);
	void*								failSafeAlloc(size_t size, size_t alignment);
	static void*						preInitAlloc(size_t size, size_t alignment);

	PooledAllocator						_pool;
	HeapAllocator						_heap;

	std::vector<std::string>			_dumpLines;
};

#define g_MemManager (::nit::MemManager::getInstance())

#endif // #ifndef NO_MEM_MANAGER

////////////////////////////////////////////////////////////////////////////////

#ifdef NO_MEM_MANAGER

class NIT_API MemManager
{
public:
	static MemManager* getInstance()
	{ 
		static MemManager g; 
		return &g; 
	}

	void* Allocate(size_t size, size_t alignment, MemHint hint)
	{
		return AlignedMalloc(size, MEM_DEFAULT_ALIGNMENT);
	}

	bool deallocate(void* memory, size_t size)
	{
		AlignedFree(memory);
		
		return true;
	}

	void* reallocate(void* memory, size_t newSize, size_t oldSize, size_t alignment, MemHint hint)
	{
		return AlignedRealloc(memory, newSize, MEM_DEFAULT_ALIGNMENT);
	}

	void dump()
	{
		LOG(0, "*** MemManager not installed\n");
	}
};

#define g_MemManager (::MemManager::getInstance())

#endif // #ifdef NO_MEM_MANAGER

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;

