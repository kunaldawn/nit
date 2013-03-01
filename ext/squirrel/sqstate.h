/*	see copyright notice in squirrel.h */
#ifndef _SQSTATE_H_
#define _SQSTATE_H_

#include "squtils.h"
#include "sqobject.h"
struct SQString;
struct SQTable;
//max number of character for a printed number
#define NUMBER_MAX_CHAR 50

struct SQStringTable
{
	SQStringTable(SQSharedState*ss);
	~SQStringTable();
	SQString *Add(const SQChar *,SQInteger len);
	void Remove(SQString *);
private:
	void Resize(SQInteger size);
	void AllocNodes(SQInteger size);
	SQString **_strings;
	SQUnsignedInteger _numofslots;
	SQUnsignedInteger _slotused;
	SQSharedState *_sharedstate;
};

struct RefTable {
	struct RefNode {
		SQObjectPtr obj;
		SQUnsignedInteger refs;
		struct RefNode *next;
	};
	RefTable();
	~RefTable();
	void AddRef(SQObject &obj);
	SQBool Release(SQObject &obj);
#ifndef NO_GARBAGE_COLLECTOR
	void Mark(SQGC*);
	void SetGC(SQGC* gc) { _gc = gc; }
	SQGC* _gc;
#endif
	void Finalize();
private:
	RefNode *Get(SQObject &obj,SQHash &mainpos,RefNode **prev,bool add);
	RefNode *Add(SQHash mainpos,SQObject &obj);
	void Resize(SQUnsignedInteger size);
	void AllocNodes(SQUnsignedInteger size);
	SQUnsignedInteger _numofslots;
	SQUnsignedInteger _slotused;
	RefNode *_nodes;
	RefNode *_freelist;
	RefNode **_buckets;
};

#define ADD_STRING(ss,str,len) ss->_stringtable->Add(str,len)
#define REMOVE_STRING(ss,bstr) ss->_stringtable->Remove(bstr)

struct SQObjectPtr;

struct SQSharedState
{
	SQSharedState();
	~SQSharedState();
	void Init();
public:
	SQChar* GetScratchPad(SQInteger size);
	void PushScratchPad();
	void PopScratchPad();
	SQInteger GetMetaMethodIdxByName(const SQObjectPtr &name);
	void CallReleaseHook(SQRELEASEHOOK _hook, SQUserPointer up, SQInteger size);
	SQVM* GetWorkThread() { return _curr_thread ? _curr_thread : sqi_thread(_root_vm); }
#ifndef NO_GARBAGE_COLLECTOR
	SQInteger CollectGarbage(SQVM *vm); 
	void MarkRootSet(SQGC*);
#endif
	SQObjectPtrVec *_metamethods;
	SQObjectPtr _metamethodsmap;
	SQObjectPtrVec *_systemstrings;
	SQObjectPtrVec *_types;
	SQObjectPtrVec *_classes;
	SQStringTable *_stringtable;
	RefTable _refs_table;
	SQObjectPtr _registry;
	SQObjectPtr _consts;
	SQObjectPtr _thisidx;
	SQObjectPtr _classnameidx;
	SQObjectPtr _namespaceidx;
	SQObjectPtr _initializeridx;
	SQObjectPtr _constructoridx;
	SQObjectPtr _destructoridx;
	SQObjectPtr _unnamed;
#ifndef NO_GARBAGE_COLLECTOR
	SQGC _gc;
	SQGC* _gc_chain;
	SQGCFinalizeReport* _gc_finalize_report;
#endif
	SQObjectPtr _root_vm;
	SQObjectPtr _root_table;
	SQVM* _curr_thread;
	SQObjectPtr _null_default_delegate;
	static SQRegFunction _null_default_delegate_funcz[];
	SQObjectPtr _table_default_delegate;
	static SQRegFunction _table_default_delegate_funcz[];
	SQObjectPtr _array_default_delegate;
	static SQRegFunction _array_default_delegate_funcz[];
	SQObjectPtr _string_default_delegate;
	static SQRegFunction _string_default_delegate_funcz[];
	SQObjectPtr _number_default_delegate;
	static SQRegFunction _number_default_delegate_funcz[];
	SQObjectPtr _generator_default_delegate;
	static SQRegFunction _generator_default_delegate_funcz[];
	SQObjectPtr _closure_default_delegate;
	static SQRegFunction _closure_default_delegate_funcz[];
	SQObjectPtr _thread_default_delegate;
	static SQRegFunction _thread_default_delegate_funcz[];
	SQObjectPtr _class_default_delegate;
	static SQRegFunction _class_default_delegate_funcz[];
	SQObjectPtr _instance_default_delegate;
	static SQRegFunction _instance_default_delegate_funcz[];
	SQObjectPtr _weakref_default_delegate;
	static SQRegFunction _weakref_default_delegate_funcz[];
	SQObjectPtr _nativeweakref_default_delegate;
	static SQRegFunction _nativeweakref_default_delegate_funcz[];
	
	SQUnsignedInteger _oplimit;
	SQCOMPILERERROR _compilererrorhandler;
	SQPRINTFUNCTION _printfunc;
	SQPRINTFUNCTION _errorfunc;
	bool _debuginfo;
	bool _enablehelp;
	bool _notifyallexceptions;
	bool _enableasserts;
private:
	struct ScratchPad
	{
		SQChar* buf;
		SQInteger size;
	};
	sqvector<ScratchPad> _scratchpads;
	int _scratchpadindex;
};

#define _sp(s) (_sharedstate->GetScratchPad(s))
#define _spval (_sharedstate->GetScratchPad(-1))

#define _null_ddel		sqi_table(_sharedstate->_null_default_delegate)
#define _table_ddel		sqi_table(_sharedstate->_table_default_delegate) 
#define _array_ddel		sqi_table(_sharedstate->_array_default_delegate) 
#define _string_ddel	sqi_table(_sharedstate->_string_default_delegate) 
#define _number_ddel	sqi_table(_sharedstate->_number_default_delegate) 
#define _generator_ddel	sqi_table(_sharedstate->_generator_default_delegate) 
#define _closure_ddel	sqi_table(_sharedstate->_closure_default_delegate) 
#define _thread_ddel	sqi_table(_sharedstate->_thread_default_delegate) 
#define _class_ddel		sqi_table(_sharedstate->_class_default_delegate) 
#define _instance_ddel	sqi_table(_sharedstate->_instance_default_delegate) 
#define _weakref_ddel	sqi_table(_sharedstate->_weakref_default_delegate) 
#define _nativeweakref_ddel sqi_table(_sharedstate->_nativeweakref_default_delegate) 

#ifdef SQUNICODE //rsl REAL STRING LEN
#define rsl(l) ((l)<<1)
#else
#define rsl(l) (l)
#endif

extern SQObjectPtr _null_;
//extern SQObjectPtr _true_;
//extern SQObjectPtr _false_;
//extern SQObjectPtr _one_;
//extern SQObjectPtr _minusone_;

bool CompileTypemask(SQIntVec &res,const SQChar *typemask);

void *sq_vm_malloc(SQUnsignedInteger size);
void *sq_vm_realloc(void *p,SQUnsignedInteger oldsize,SQUnsignedInteger size);
void sq_vm_free(void *p,SQUnsignedInteger size);
#endif //_SQSTATE_H_
