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

#include "nit_pch.h"

#include "MemManager.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

#ifndef NO_MEM_MANAGER

static uint g_MemAllocCounter = 1;

// NOTE: Only use with Dump()!
#if !defined(NIT_NO_LOG)
#define MEM_DUMP_PRT(...) { ::nit::MemManager::getInstance()->dumpLog(__VA_ARGS__); }
#else
#define MEM_DUMP_PRT(...) { }
#endif

////////////////////////////////////////////////////////////////////////////////

MemPool::MemPool()
{
	_entrySize			= 0;
	_byteAlignment		= 0;
	_numEntries		= 0;
	_numFree			= 0;
	_headOfFreeList	= NULL;
	_entryStart		= 0;
	_entryEnd			= 0;
	_rawMemory			= NULL;
	_debugInfoArray	= NULL;
}

MemPool::MemPool(u16 entrySize, u16 byteAlignment, void* rawMemory, size_t size, MemDebugInfo* debugInfoArray)
{
	setup(entrySize, byteAlignment, rawMemory, size, debugInfoArray);
}

MemPool::~MemPool()
{
}

void MemPool::setup(u16 entrySize, u16 byteAlignment, void* rawMemory, size_t size, MemDebugInfo* debugInfoArray)
{
	_entrySize = entrySize;

	ASSERT(_entrySize >= sizeof(Entry));

	size_t memStart = (size_t)rawMemory;

	_byteAlignment = byteAlignment;
	_rawMemory = rawMemory;

	// Adjust actual allocation size according to byteAlignment
	size_t tailPadding = byteAlignment - (_entrySize % byteAlignment);
	if (tailPadding == byteAlignment) tailPadding = 0;
	_entrySize += tailPadding;

	// Align startOffset to byteAlignment
	size_t startOffset = byteAlignment - (memStart % byteAlignment);
	if (startOffset == byteAlignment) startOffset = 0;

	// Calculate entry start position
	_entryStart = memStart + startOffset;
	_headOfFreeList = (Entry*)_entryStart;

	// Count total number of entries
	_numEntries = ((memStart + size) - _entryStart) / _entrySize;
	_numFree = _numEntries;
	_minFree = _numFree;
	_entryEnd = _entryStart + _numEntries * _entrySize;

	// Initialize individual entries
	Entry* entry = NULL;
	byte* ptr = (byte*)_headOfFreeList;
	for (uint i=0; i<_numEntries; ++i)
	{
		entry = (Entry*) ptr;
		ASSERT(size_t(entry) % byteAlignment == 0);

		ptr += _entrySize;
		entry->next = (Entry*)ptr;
	}

	// Mark last entry's next as sentinel
	entry->next = getEndSentinel();

	ASSERT(size_t(entry) + _entrySize <= _entryEnd);

#if !defined(NIT_SHIPPING)
	// Initialize debug info array
	_debugInfoArray = debugInfoArray;
	if (_debugInfoArray)
		memset(_debugInfoArray, 0, sizeof(MemDebugInfo) * _numEntries);
#endif // #if !defined(NIT_SHIPPING)
}

void* MemPool::Allocate()
{
	Entry* entry = _headOfFreeList;

#if !defined(NIT_SHIPPING)
	ASSERT_MSG(_numFree > 0, "MemPool: Attempt to allocate from empty block");

	bool valid = 
		contains(entry) && 
		(contains(_headOfFreeList->next) || _headOfFreeList->next == getEndSentinel());

	ASSERT_MSG(valid, "MemPool: Pool corrupted at %d byte pool: %08x", _entrySize, entry);
#endif // #if !defined(NIT_SHIPPING)

	_headOfFreeList = _headOfFreeList->next;
	--_numFree;

	if (_minFree > _numFree) _minFree = _numFree;

#if !defined(NIT_SHIPPING)
	ASSERT_MSG(_numFree == 0 || _headOfFreeList != getEndSentinel(),
		"MemPool: Pool empty but NumFree still non-zero");

// 	if (_numFree == 0)
// 		MEM_DUMP_PRT("*** Pool %5d full\n", _entrySize);

	entry->next = (Entry*)0xDEADBEEF;
#endif // #if !defined(NIT_SHIPPING)

	return entry;
}

inline int MemPool::indexOf(void* ptr)
{
	if (!contains(ptr)) return -1;

	int offset = size_t(ptr) - _entryStart;

	int index = offset / _entrySize;

	return index * _entrySize + _entryStart == size_t(ptr) ? index : -1;
}

bool MemPool::deallocate(void* ptr)
{
#if !defined(NIT_SHIPPING)
	int index = indexOf(ptr);

	bool valid =
		index >= 0 &&
		_numFree < _numEntries;

	ASSERT_MSG(valid, "MemPool: Invalid deallocate at %d byte pool: %08x", _entrySize, ptr);
#endif // #if !defined(NIT_SHIPPING)
	
	Entry* entry = (Entry*) ptr;

	entry->next = _headOfFreeList;
	_headOfFreeList = entry;
	++_numFree;

#if !defined(NIT_SHIPPING)
	ASSERT(_numFree <= _numEntries);

	if (_debugInfoArray)
	{
		MemDebugInfo& info = _debugInfoArray[index];
		info.counter = 0;
	}
#endif // #if !defined(NIT_SHIPPING)

	return true;
}

void MemPool::dump(bool print, uint& varMaxSize, uint& varTotalAllocated, uint& varTotalActual)
{
#if !defined(NIT_SHIPPING)
	uint actual = 0;

	if (_debugInfoArray)
	{
		uint numAlloc = 0;
		for (uint i=0; i<_numEntries; ++i)
		{
			MemDebugInfo& info = _debugInfoArray[i];
			if (info.counter == 0) continue;
			++numAlloc;
			actual += info.size;
		}

		if (numAlloc > 0)
		{
			ASSERT(numAlloc == _numEntries - _numFree);
		}
	}

	uint inUse = (_numEntries - _numFree) * _entrySize;

	varMaxSize += _numEntries * _entrySize;
	varTotalAllocated += inUse;
	varTotalActual += actual;

	if (print)
	{
		MEM_DUMP_PRT("  Pool (%5d/%3d) %4.1fmb: %6d/%6d (high %6d) eff: %6dkb/%6dkb (%3d%%) %08x-%08x\n", 
			_entrySize, 
			_byteAlignment,
			(_numEntries * _entrySize) / float(1024 * 1024),
			_numEntries - _numFree, 
			_numEntries, 
			_numEntries - _minFree,
			actual / 1024,
			inUse / 1024,
			inUse ? int(double(actual) / inUse * 100) : 0,
			_entryStart,
			_entryStart + _numEntries * _entrySize
			);
	}
#endif // #if !defined(NIT_SHIPPING)
}

////////////////////////////////////////////////////////////////////////////////

PooledAllocator::PooledAllocator(const char* name) : MemAllocator(name)
{
	_numPools = 0;
	for (uint i=0; i<NUM_MAX_POOLS; ++i)
	{
		_rangeList[i] = &_pools[i];
		_sizeList[i] = &_pools[i];
	}
}

PooledAllocator::~PooledAllocator()
{
}

void PooledAllocator::addPool(u16 entrySize, u16 byteAlignment, void* rawMemory, size_t size, MemDebugInfo* debugInfoArray)
{
	ASSERT(_numPools < NUM_MAX_POOLS);
	
	MemPool& block = _pools[_numPools++];
	block.setup(entrySize, byteAlignment, rawMemory, size, debugInfoArray);

	sortRange();
}

void PooledAllocator::sortRange()
{
	std::sort(rangeBegin(), rangeEnd(), MemPool::RangeLess());
	std::sort(sizeBegin(), sizeEnd(), MemPool::SizeLess());
}

MemPool* PooledAllocator::findPool(void* ptr)
{
	Iterator itr = std::lower_bound(rangeBegin(), rangeEnd(), ptr, MemPool::RangeLess());

	if (itr == rangeEnd())
		return NULL;

	MemPool* block = *itr;
	if (!block->contains(ptr))
		return NULL;

	return block;
}

MemPool* PooledAllocator::findFree(size_t size, size_t alignment)
{
	Iterator itr = std::lower_bound(sizeBegin(), sizeEnd(), size, MemPool::SizeLess());

	MemPool* pool = *itr;

 	if (alignment <= pool->getByteAlignment())
 		return pool->getNumFree() > 0 ? pool : NULL;
 
 	pool = NULL;

 	for (; itr != sizeEnd(); ++itr)
 	{
 		if ((*itr)->getNumFree() == 0) continue;
 		if ((*itr)->getByteAlignment() < alignment) continue;
 
 		pool = *itr;
 		break;
 	}
 
 	return pool;
}

void* PooledAllocator::Allocate(size_t size, size_t alignment)
{
	MemPool* pool = findFree(size, alignment);
	if (pool == NULL) return NULL;

	ASSERT(pool->getEntrySize() >= size);

	void* mem = pool->Allocate();

#if !defined(NIT_SHIPPING)
	if (pool->_debugInfoArray)
	{
		int index = pool->indexOf(mem);
		ASSERT(index != -1);
		MemDebugInfo& info = pool->_debugInfoArray[index];
		info.counter = g_MemAllocCounter++;
		info.size = size;
		info.alignment = alignment;
	}
#endif // #if !defined(NIT_SHIPPING)

	return mem;
}

bool PooledAllocator::deallocate(void* memory, size_t size)
{
	if (memory == NULL) return false;

	MemPool* pool = findPool(memory);
	if (pool == NULL) return false;

	return pool->deallocate(memory);
}

void* PooledAllocator::reallocate(void* memory, size_t newSize, size_t oldSize, size_t alignment)
{
	// TODO: CURRENTLY NOT USED - IMPL THIS
	if (newSize == 0)
	{
		deallocate(memory, oldSize);
		return NULL;
	}

	if (memory == NULL)
		return Allocate(newSize, alignment);

	MemPool* oldPool = findPool(memory);
	if (oldPool == NULL) return NULL;

	u16 oldEntrySize = oldPool->getEntrySize();

	if (newSize <= oldEntrySize && (newSize + newSize >= oldEntrySize))
		return memory;

	MemPool* newPool = findFree(newSize, alignment);
	if (oldPool == newPool)
		return memory;

	if (newPool == NULL)
		return NULL;

	void* newMem = newPool->Allocate();
	if (newMem == NULL) return NULL;

	memcpy(newMem, memory, newSize);
	oldPool->deallocate(memory);

	return newMem;
}

void PooledAllocator::dump()
{
#if !defined(NIT_SHIPPING)
	MEM_DUMP_PRT("%s:\n", _name);

	uint maxSize = 0;
	uint totalAllocated = 0;
	uint totalActual = 0;

	for (Iterator itr = sizeBegin(), end = sizeEnd(); itr != end; ++itr)
	{
		(*itr)->dump(true, maxSize, totalAllocated, totalActual);
	}
	MEM_DUMP_PRT("  Capacity : %6dkb, Allocated : %6dkb (%3d%%), Actual : %6dkb, Waste : %6dkb (%3d%%)\n",
		maxSize / 1024,										// capacity
		totalAllocated / 1024,								// allocated
		maxSize ? int(double(totalAllocated) / maxSize * 100) : 0,		// allocated %
		totalActual / 1024,									// actual
		(totalAllocated - totalActual) / 1024,				// waste
		totalAllocated ? int(double(totalAllocated - totalActual) / totalAllocated * 100) : 0// waste %
	);
#endif // #if !defined(NIT_SHIPPING)
}

////////////////////////////////////////////////////////////////////////////////

void* HeapAllocator::Allocate(size_t size, size_t alignment)
{
	return AlignedMalloc(size, alignment);
}

bool HeapAllocator::deallocate(void* memory, size_t size)
{
	AlignedFree(memory);
	return true;
}

void* HeapAllocator::reallocate(void* memory, size_t newSize, size_t oldSize, size_t alignment)
{
	return AlignedRealloc(memory, newSize, alignment);
}

void HeapAllocator::dump()
{
#if !defined(NIT_SHIPPING) && defined(NIT_WIN32)
	MEM_DUMP_PRT("%s:\n", _name);

	HANDLE heap = GetProcessHeap();
	PROCESS_HEAP_ENTRY heapEntry = { 0 };

	int totalCount = 0;
	int totalMemory = 0;

	int countBelow32 = 0;	int memBelow32 = 0;
	int countBelow256 = 0;	int memBelow256 = 0;
	int countBelow1024 = 0;	int memBelow1024 = 0;
	int countBelow32k = 0;	int memBelow32k = 0;
	int countBelow1MB = 0;	int memBelow1MB = 0;
	int countAbove1MB = 0;	int memAbove1MB = 0;
	
	while (HeapWalk(heap, &heapEntry))
	{
		++totalCount;
		totalMemory += heapEntry.cbData;

		if (heapEntry.cbData <= 32)
		{
			++countBelow32;
			memBelow32 += heapEntry.cbData;
		}
		else if (heapEntry.cbData <= 256)
		{
			++countBelow256;
			memBelow256 += heapEntry.cbData;
		}
		else if (heapEntry.cbData <= 1024)
		{
			++countBelow1024;
			memBelow1024 += heapEntry.cbData;
		}
		else if (heapEntry.cbData <= 32768)
		{
			++countBelow32k;
			memBelow32k += heapEntry.cbData;
		}
		else if (heapEntry.cbData <= 1024 * 1024)
		{
			++countBelow1MB;
			memBelow1MB += heapEntry.cbData;
		}
		else
		{
			++countAbove1MB;
			memAbove1MB += heapEntry.cbData;
		}
	}

	bool showBig = false;

	if (showBig)
	{
		// HACK: workaround for MEM_DUMP_PRT would also use heap, so heap walking could be broken.
		heapEntry.lpData = NULL;
		while (HeapWalk(heap, &heapEntry))
		{
			if (heapEntry.cbData > 1024 * 1024)
			{
				MEM_DUMP_PRT("  big entry: %08x-%08x (%d bytes)\n",
					heapEntry.lpData, uint(heapEntry.lpData) + heapEntry.cbData, heapEntry.cbData);
			}
		}
	}

	MEM_DUMP_PRT("  ~   32 : %6d entries, total %6dkb\n", countBelow32, memBelow32 / 1024);
	MEM_DUMP_PRT("  ~  256 : %6d entries, total %6dkb\n", countBelow256, memBelow256 / 1024);
	MEM_DUMP_PRT("  ~  1KB : %6d entries, total %6dkb\n", countBelow1024, memBelow1024 / 1024);
	MEM_DUMP_PRT("  ~ 32KB : %6d entreis, total %6dkb\n", countBelow32k, memBelow32k / 1024);
	MEM_DUMP_PRT("  ~  1MB : %6d entries, total %6dkb\n", countBelow1MB, memBelow1MB / 1024);
	MEM_DUMP_PRT("  >  1MB : %6d entries, total %6dkb\n", countAbove1MB, memAbove1MB / 1024);
	MEM_DUMP_PRT("  total %d entries, %dkb\n", totalCount, totalMemory / 1024);
#endif // #if !defined(NIT_SHIPPING)
}

////////////////////////////////////////////////////////////////////////////////

MemManager::MemManager()
: _pool("pool")
, _heap("heap")
{
	_initialized = true;
}

MemManager::~MemManager()
{
	// TODO: assert fails when CTRL+C to console
//	assert(_initialized == false);
}

MemManager* MemManager::getInstance()
{
	static MemManager g_instance;
	return &g_instance;
}

bool MemManager::initPools(const RawArenas& arenas)
{
	Mutex::ScopedLock lock(_lock);

	if (!_arenas.empty()) return false;

	PooledAllocator* pool = getPool();

	_arenas = arenas;

	for (size_t i = 0; i < _arenas.size(); ++i)
	{
		RawArena& a = _arenas[i];

		a._rawBase = AlignedMalloc(a.size, a.alignment);
		a._numEntries = a.size / a.entrySize;

#ifdef _SHIPPING
		a._infoBase = 0;
#else
		a._infoBase = new MemDebugInfo[a._numEntries];
#endif

		pool->addPool(a.entrySize, a.alignment, a._rawBase, a.size, a._infoBase);
	}

	return true;
}

void MemManager::shutdown()
{
	_initialized = false;

	bool ignorePoolArenas = true;

	// Memory that remains after disposing GameAppMemory (mainly statics)
	// Forget about them when we are allowed
	if (!ignorePoolArenas)
	{
		for (uint i = 0; i < _arenas.size(); ++i)
		{
			RawArena& a = _arenas[i];
			AlignedFree(a._rawBase);
			delete[] a._infoBase;
		}
	}
}

void* MemManager::Allocate(size_t size, size_t alignment, MemHint hint)
{
	if (!_initialized) 
		return preInitAlloc(size, alignment);

	bool poolOnly = false;

	// TODO: use hint
	void* mem = tryAlloc(size, alignment, poolOnly);
	if (mem) return mem;

	NIT_THROW_FMT(EX_MEMORY, "MemManager: Allocation Fail!!");
	return NULL;
}

void* MemManager::failSafeAlloc(size_t size, size_t alignment)
{
	// TODO: Allocate emergency memory in advance and use here (for HINT_SYSTEM)
	return AlignedMalloc(size, alignment);
}

void* MemManager::preInitAlloc(size_t size, size_t alignment)
{
	return AlignedMalloc(size, alignment);
}

void* MemManager::tryAlloc(size_t size, size_t alignment, bool poolOnly)
{
	void* mem = NULL;

	Mutex::ScopedLock lock(_lock);

	mem = _pool.Allocate(size, alignment);
	if (mem == NULL && !poolOnly)
		mem = _heap.Allocate(size, alignment);

	return mem;
}

bool MemManager::deallocate(void* memory, size_t size)
{
	if (memory == NULL) return true;

	if (!_initialized) return false;
	
	Mutex::ScopedLock lock(_lock);

	if (_pool.deallocate(memory, size))
		return true;

	if (_heap.deallocate(memory, size))
		return true;

	ASSERT(false);
	return false;
}

void* MemManager::reallocate(void* memory, size_t newSize, size_t oldSize, size_t alignment, MemHint hint)
{
	if (memory == NULL && newSize == 0) return NULL;
	if (newSize == 0 && oldSize == 0) return NULL;

	if (
		(memory == NULL && oldSize != 0) ||
		(newSize == 0 && oldSize == 0) ||
		(alignment == 0) )
	{
		// NOTE: not locked yet so safe here.
		LOG(0, "*** MemManager::Reallocate detected: mem:%08x, new:%d, old:%d, align:%d\n", memory, newSize, oldSize, alignment);
		return NULL;
	}

	if (memory == NULL)
	{
		return Allocate(newSize, alignment, hint);
	}

	if (newSize == 0)
	{
		deallocate(memory, oldSize);
		return NULL;
	}
	
	Mutex::ScopedLock lock(_lock);

	// TODO: Reimpl Reallocate so that utilize pool's reallocation facility

	void* newMem = Allocate(newSize, alignment, hint);
	if (newMem == NULL)
		return NULL;

	if (newMem)
	{
		if (oldSize == MEM_DEALLOC_SIZE_UNKNOWN)
			memcpy(newMem, memory, newSize);
		else
			memcpy(newMem, memory, std::min(oldSize, newSize));

		deallocate(memory, oldSize);
	}
	
	return newMem;
}

void MemManager::dump()
{
#if !defined(NIT_SHIPPING)
	ASSERT_MSG(Thread::current() == NULL, "Dump from non-main thread");

	_lock.lock();

	MEM_DUMP_PRT("MemManager Dump (age %d):\n", g_MemAllocCounter);
	
#if defined(NIT_WIN32)
	MEMORYSTATUS memstat;
	GlobalMemoryStatus(&memstat);

	MEM_DUMP_PRT("  virtual:  %6dmb/%6dmb (%3d%%)\n",
		(memstat.dwTotalVirtual - memstat.dwAvailVirtual) / (1024 * 1024),
		memstat.dwTotalVirtual / (1024 * 1024),
		int(double(memstat.dwTotalVirtual - memstat.dwAvailVirtual) / memstat.dwTotalVirtual * 100)
		);

	MEM_DUMP_PRT("  physical: %6dmb/%6dmb (%3d%%)\n",
		(memstat.dwTotalPhys - memstat.dwAvailPhys) / (1024 * 1024),
		memstat.dwTotalPhys / (1024 * 1024),
		int(double(memstat.dwTotalPhys - memstat.dwAvailPhys) / memstat.dwTotalPhys * 100)
		);
#endif // #if defined(NIT_WIN32)

	_pool.dump();
	_heap.dump();

	_lock.unlock();

	if (_dumpLines.empty()) return;

	LogManager& log = LogManager::getSingleton();

	for (uint i=0; i<_dumpLines.size(); ++i)
	{
		std::string& line = _dumpLines[i];
		log.doLog(0, "PM", 0, 0, 0, "..", line.c_str(), line.length(), false);
	}
	_dumpLines.clear();
#endif // #if !defined(NIT_SHIPPING)
}

void MemManager::dumpLog(const char* fmt, ...)
{
#if !defined(NIT_SHIPPING)
	_dumpLines.push_back(std::string());
	std::string& line = _dumpLines.back();
	va_list args;
	va_start(args, fmt);
	LogManager::getSingleton().vformat(line, fmt, args);
	va_end(args);
#endif // #if !defined(NIT_SHIPPING)
}

#endif // #ifndef NO_MEM_MANAGER

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
