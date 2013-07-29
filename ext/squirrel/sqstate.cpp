/*
	see copyright notice in squirrel.h
*/
#include "sqpcheader.h"
#include "sqopcodes.h"
#include "sqvm.h"
#include "sqfuncproto.h"
#include "sqclosure.h"
#include "sqstring.h"
#include "sqtable.h"
#include "sqarray.h"
#include "squserdata.h"
#include "sqclass.h"

SQObjectPtr _null_;
//SQObjectPtr _true_(true);
//SQObjectPtr _false_(false);
//SQObjectPtr _one_((SQInteger)1);
//SQObjectPtr _minusone_((SQInteger)-1);

SQSharedState::SQSharedState()
{
	_compilererrorhandler = NULL;
	_printfunc = NULL;
	_errorfunc = NULL;
	_debuginfo = false;
	_notifyallexceptions = false;
	_enableasserts = true;
	_enablehelp = true;
	_oplimit = 0;
	_curr_thread = NULL;
}

#define newsysstring(s) {	\
	_systemstrings->push_back(SQString::Create(this,s));	\
	}

#define newmetamethod(s) {	\
	_metamethods->push_back(SQString::Create(this,s));	\
	sqi_table(_metamethodsmap)->NewSlot(_metamethods->back(),(SQInteger)(_metamethods->size()-1)); \
	}

bool CompileTypemask(SQIntVec &res,const SQChar *typemask)
{
	SQInteger i = 0;
	
	SQInteger mask = 0;
	while(typemask[i] != 0) {
		
		switch(typemask[i]){
				case 'o': mask |= _RT_NULL; break;
				case 'i': mask |= _RT_INTEGER; break;
				case 'f': mask |= _RT_FLOAT; break;
				case 'n': mask |= (_RT_FLOAT | _RT_INTEGER); break;
				case 's': mask |= _RT_STRING; break;
				case 't': mask |= _RT_TABLE; break;
				case 'a': mask |= _RT_ARRAY; break;
				case 'u': mask |= _RT_USERDATA; break;
				case 'c': mask |= (_RT_CLOSURE | _RT_NATIVECLOSURE); break;
				case 'b': mask |= _RT_BOOL; break;
				case 'g': mask |= _RT_GENERATOR; break;
				case 'p': mask |= _RT_USERPOINTER; break;
				case 'v': mask |= _RT_THREAD; break;
				case 'x': mask |= _RT_INSTANCE; break;
				case 'y': mask |= _RT_CLASS; break;
				case 'r': mask |= _RT_WEAKREF; break;
				case 'R': mask |= _RT_NATIVEWEAKREF; break;
				case '.': mask = -1; res.push_back(mask); i++; mask = 0; continue;
				case ' ': i++; continue; //ignores spaces
				default:
					return false;
		}
		i++;
		if(typemask[i] == '|') { 
			i++; 
			if(typemask[i] == 0)
				return false;
			continue; 
		}
		res.push_back(mask);
		mask = 0;
		
	}
	return true;
}

SQTable *CreateDefaultDelegate(SQSharedState *ss,SQRegFunction *funcz)
{
	SQInteger i=0;
	SQTable *t=SQTable::Create(ss,0);
	while(funcz[i].name!=0){
		SQNativeClosure *nc = SQNativeClosure::Create(ss,funcz[i].f);
		nc->_nparamscheck = funcz[i].nparamscheck;
		nc->_name = funcz[i].name;
		if (funcz[i].help)
			nc->_help = SQString::Create(ss,funcz[i].help);
		if(funcz[i].typemask && !CompileTypemask(nc->_typecheck,funcz[i].typemask))
			return NULL;
		t->NewSlot(SQString::Create(ss,funcz[i].name),nc);
		i++;
	}
	return t;
}

void SQSharedState::Init()
{	
	_scratchpadindex = -1;
	PushScratchPad();
#ifndef NO_GARBAGE_COLLECTOR
	_gc._ss = this;
	_gc_chain = &_gc;
	_refs_table.SetGC(&_gc);
#endif
	_stringtable = (SQStringTable*)SQ_MALLOC(sizeof(SQStringTable));
	new (_stringtable) SQStringTable(this);
	sq_new(_metamethods,SQObjectPtrVec);
	sq_new(_systemstrings,SQObjectPtrVec);
	sq_new(_types,SQObjectPtrVec);
	sq_new(_classes,SQObjectPtrVec);
	_metamethodsmap = SQTable::Create(this,MT_LAST-1);
	//adding type strings to avoid memory trashing
	//types names
	newsysstring(_SC("null"));
	newsysstring(_SC("table"));
	newsysstring(_SC("array"));
	newsysstring(_SC("closure"));
	newsysstring(_SC("string"));
	newsysstring(_SC("userdata"));
	newsysstring(_SC("integer"));
	newsysstring(_SC("float"));
	newsysstring(_SC("userpointer"));
	newsysstring(_SC("function"));
	newsysstring(_SC("generator"));
	newsysstring(_SC("thread"));
	newsysstring(_SC("class"));
	newsysstring(_SC("instance"));
	newsysstring(_SC("bool"));
	//meta methods
	newmetamethod(MM_ADD);
	newmetamethod(MM_SUB);
	newmetamethod(MM_MUL);
	newmetamethod(MM_DIV);
	newmetamethod(MM_UNM);
	newmetamethod(MM_MODULO);
	newmetamethod(MM_SET);
	newmetamethod(MM_GET);
	newmetamethod(MM_TYPEOF);
	newmetamethod(MM_NEXTI);
	newmetamethod(MM_CMP);
	newmetamethod(MM_CALL);
	newmetamethod(MM_CLONED);
	newmetamethod(MM_NEWSLOT);
	newmetamethod(MM_DELSLOT);
	newmetamethod(MM_TOSTRING);
	newmetamethod(MM_NEWMEMBER);
	newmetamethod(MM_INHERITED);
	newmetamethod(MM_ASSIGN);
	newmetamethod(MM_WEAKREF);

	_root_table = SQTable::Create(this, 0);
	_thisidx = SQString::Create(this,_SC("this"));
	_classnameidx = SQString::Create(this,_SC("_classname"));
	_namespaceidx = SQString::Create(this,_SC("_namespace"));
	_initializeridx = SQString::Create(this,_SC("_initializer"));
	_constructoridx = SQString::Create(this,_SC("constructor"));
	_destructoridx = SQString::Create(this,_SC("_destructor"));
	_unnamed = SQString::Create(this,_SC("unnamed"));
	_registry = SQTable::Create(this,0);
	_consts = SQTable::Create(this,0);
	_null_default_delegate = CreateDefaultDelegate(this,_null_default_delegate_funcz);
	_table_default_delegate = CreateDefaultDelegate(this,_table_default_delegate_funcz);
	_array_default_delegate = CreateDefaultDelegate(this,_array_default_delegate_funcz);
	_string_default_delegate = CreateDefaultDelegate(this,_string_default_delegate_funcz);
	_number_default_delegate = CreateDefaultDelegate(this,_number_default_delegate_funcz);
	_closure_default_delegate = CreateDefaultDelegate(this,_closure_default_delegate_funcz);
	_generator_default_delegate = CreateDefaultDelegate(this,_generator_default_delegate_funcz);
	_thread_default_delegate = CreateDefaultDelegate(this,_thread_default_delegate_funcz);
	_class_default_delegate = CreateDefaultDelegate(this,_class_default_delegate_funcz);
	_instance_default_delegate = CreateDefaultDelegate(this,_instance_default_delegate_funcz);
	_weakref_default_delegate = CreateDefaultDelegate(this,_weakref_default_delegate_funcz);
	_nativeweakref_default_delegate = CreateDefaultDelegate(this,_nativeweakref_default_delegate_funcz);

}

extern void SSFinale(SQSharedState* ss);

SQSharedState::~SQSharedState()
{
//	SSFinale(this);
	
	// HACK: to avoid circular reference between class and instances
	for (SQUnsignedInteger i = 0; i < _classes->size(); ++i)
	{
		SQWeakRef* w = sqi_weakref((*_classes)[i]);
		if (w->_obj._type == OT_NULL) continue;
		SQClass* c = sqi_class(w->_obj);
		// ignore classname (_methods[0]), namespace (_methods[1])
		for (SQUnsignedInteger j = 2; j < c->_methods.size(); ++j)
		{
			c->_methods[j].val.Null();
		}
	}

	_thisidx = _null_;
	_classnameidx = _null_;
	_namespaceidx = _null_;
	_initializeridx = _null_;
	_constructoridx = _null_;
	_destructoridx = _null_;
	_unnamed = _null_;
	sqi_table(_registry)->Finalize();
	sqi_table(_consts)->Finalize();
	sqi_table(_metamethodsmap)->Finalize();
	_registry = _null_;
	_consts = _null_;
	_metamethodsmap = _null_;
	while(!_systemstrings->empty()) {
		_systemstrings->back()=_null_;
		_systemstrings->pop_back();
	}
	sqi_thread(_root_vm)->Finalize();
	_root_table = _null_;
	_root_vm = _null_;
	_null_default_delegate = _null_;
	_table_default_delegate = _null_;
	_array_default_delegate = _null_;
	_string_default_delegate = _null_;
	_number_default_delegate = _null_;
	_closure_default_delegate = _null_;
	_generator_default_delegate = _null_;
	_thread_default_delegate = _null_;
	_class_default_delegate = _null_;
	_instance_default_delegate = _null_;
	_weakref_default_delegate = _null_;
	_nativeweakref_default_delegate = _null_;
	_refs_table.Finalize();

#ifndef NO_GARBAGE_COLLECTOR
	// sweepCount: count of isolated objects unlinked during above _null_ifying (especially roottable)
	SQInteger sweepCount = 0;
	sweepCount = _gc.fullSweep();

	// stickyCount: count of isolated objects which stay alive very above FullSweep() (perhaps native related)
	SQInteger stickyCount = 0;			
	stickyCount = _gc.finalize();

	if (_gc_finalize_report)
	{
		_gc_finalize_report->sweepCount = sweepCount;
		_gc_finalize_report->stickyCount = stickyCount;
		_gc_finalize_report->leakCount = _gc._objtotal;
	}
#endif

	sq_delete(_types,SQObjectPtrVec);
	sq_delete(_systemstrings,SQObjectPtrVec);
	sq_delete(_metamethods,SQObjectPtrVec);
	sq_delete(_classes,SQObjectPtrVec);
	sq_delete(_stringtable,SQStringTable);

	for (size_t i=0; i<_scratchpads.size(); ++i)
	{
		if (_scratchpads[i].buf)
			SQ_FREE(_scratchpads[i].buf, _scratchpads[i].size);
	}
}


SQInteger SQSharedState::GetMetaMethodIdxByName(const SQObjectPtr &name)
{
	if(sqi_type(name) != OT_STRING)
		return -1;
	SQObjectPtr ret;
	if(sqi_table(_metamethodsmap)->Get(name,ret)) {
		return sqi_integer(ret);
	}
	return -1;
}

#ifndef NO_GARBAGE_COLLECTOR

SQInteger SQSharedState::CollectGarbage(SQVM *vm)
{
	return _gc.fullSweep();
}
#endif

SQChar* SQSharedState::GetScratchPad(SQInteger size)
{
	ScratchPad& pad = _scratchpads[_scratchpadindex];

	SQInteger newsize;
	if(size>0) {
		if(pad.size < size) {
			newsize = size + (size>>1);
			pad.buf = (SQChar *)SQ_REALLOC(pad.buf, pad.size, newsize);
			pad.size = newsize;

		}else if(pad.size >= (size<<5)) {
			newsize = pad.size >> 1;
			pad.buf = (SQChar *)SQ_REALLOC(pad.buf, pad.size, newsize);
			pad.size = newsize;
		}
	}

	return pad.buf;
}

void SQSharedState::PushScratchPad()
{
	if (++_scratchpadindex == _scratchpads.size())
	{
		ScratchPad pad;
		pad.buf = NULL;
		pad.size = 0;
		_scratchpads.push_back(pad);
	}
}

void SQSharedState::PopScratchPad()
{
	--_scratchpadindex;
}

void SQSharedState::CallReleaseHook(SQRELEASEHOOK _hook, SQUserPointer up, SQInteger size)
{
	SQVM* v = GetWorkThread();

	// Avoid the stack slot which potentially Null()-ifying
	SQInteger oldTop = v->_top++;

	_hook(v, up, size);
	
	v->_top = oldTop;
}

RefTable::RefTable()
{
	AllocNodes(4);
}

void RefTable::Finalize()
{
	RefNode *nodes = _nodes;
	for(SQUnsignedInteger n = 0; n < _numofslots; n++) {
		nodes->obj.Null();
		nodes++;
	}
}

RefTable::~RefTable()
{
	SQ_FREE(_buckets,(_numofslots * sizeof(RefNode *)) + (_numofslots * sizeof(RefNode)));
}

void RefTable::AddRef(SQObject &obj)
{
	SQHash mainpos;
	RefNode *prev;
	RefNode *ref = Get(obj,mainpos,&prev,true);
	ref->refs++;
	_gc->mark(ref->obj);
}

SQBool RefTable::Release(SQObject &obj)
{
	SQHash mainpos;
	RefNode *prev;
	RefNode *ref = Get(obj,mainpos,&prev,false);
	if(ref) {
		if(--ref->refs == 0) {
			SQObjectPtr o = ref->obj;
			if(prev) {
				prev->next = ref->next;
			}
			else {
				_buckets[mainpos] = ref->next;
			}
			ref->next = _freelist;
			_freelist = ref;
			_slotused--;
			ref->obj = _null_;
			//<<FIXME>>test for shrink?
			return SQTrue;
		}
	}
	else {
		assert(0);
	}
	return SQFalse;
}

void RefTable::Resize(SQUnsignedInteger size)
{
	RefNode **oldbucks = _buckets;
	RefNode *t = _nodes;
	SQUnsignedInteger oldnumofslots = _numofslots;
	AllocNodes(size);
	//rehash
	SQUnsignedInteger nfound = 0;
	for(SQUnsignedInteger n = 0; n < oldnumofslots; n++) {
		if(sqi_type(t->obj) != OT_NULL) {
			//add back;
			assert(t->refs != 0);
			RefNode *nn = Add(::HashObj(t->obj)&(_numofslots-1),t->obj);
			nn->refs = t->refs; 
			t->obj = _null_;
			nfound++;
		}
		t++;
	}
	assert(nfound == oldnumofslots);
	SQ_FREE(oldbucks,(oldnumofslots * sizeof(RefNode *)) + (oldnumofslots * sizeof(RefNode)));
}

RefTable::RefNode *RefTable::Add(SQHash mainpos,SQObject &obj)
{
	RefNode *t = _buckets[mainpos];
	RefNode *newnode = _freelist;
	newnode->obj = obj;
	_buckets[mainpos] = newnode;
	_freelist = _freelist->next;
	newnode->next = t;
	assert(newnode->refs == 0);
	_slotused++;
	return newnode;
}

RefTable::RefNode *RefTable::Get(SQObject &obj,SQHash &mainpos,RefNode **prev,bool add)
{
	RefNode *ref;
	mainpos = ::HashObj(obj)&(_numofslots-1);
	*prev = NULL;
	for (ref = _buckets[mainpos]; ref; ) {
		if(sqi_rawval(ref->obj) == sqi_rawval(obj) && sqi_type(ref->obj) == sqi_type(obj))
			break;
		*prev = ref;
		ref = ref->next;
	}
	if(ref == NULL && add) {
		if(_numofslots == _slotused) {
			assert(_freelist == 0);
			Resize(_numofslots*2);
			mainpos = ::HashObj(obj)&(_numofslots-1);
		}
		ref = Add(mainpos,obj);
	}
	return ref;
}

void RefTable::AllocNodes(SQUnsignedInteger size)
{
	RefNode **bucks;
	RefNode *nodes;
	bucks = (RefNode **)SQ_MALLOC((size * sizeof(RefNode *)) + (size * sizeof(RefNode)));
	nodes = (RefNode *)&bucks[size];
	RefNode *temp = nodes;
	SQUnsignedInteger n;
	for(n = 0; n < size - 1; n++) {
		bucks[n] = NULL;
		temp->refs = 0;
		new (&temp->obj) SQObjectPtr;
		temp->next = temp+1;
		temp++;
	}
	bucks[n] = NULL;
	temp->refs = 0;
	new (&temp->obj) SQObjectPtr;
	temp->next = NULL;
	_freelist = nodes;
	_nodes = nodes;
	_buckets = bucks;
	_slotused = 0;
	_numofslots = size;
}
//////////////////////////////////////////////////////////////////////////
//SQStringTable
/*
* The following code is based on Lua 4.0 (Copyright 1994-2002 Tecgraf, PUC-Rio.)
* http://www.lua.org/copyright.html#4
* http://www.lua.org/source/4.0.1/src_lstring.c.html
*/

SQStringTable::SQStringTable(SQSharedState *ss)
{
	_sharedstate = ss;
	AllocNodes(4);
	_slotused = 0;
}

SQStringTable::~SQStringTable()
{
	SQ_FREE(_strings,sizeof(SQString*)*_numofslots);
	_strings = NULL;
}

void SQStringTable::AllocNodes(SQInteger size)
{
	_numofslots = size;
	_strings = (SQString**)SQ_MALLOC(sizeof(SQString*)*_numofslots);
	memset(_strings,0,sizeof(SQString*)*_numofslots);
}

SQString *SQStringTable::Add(const SQChar *news,SQInteger len)
{
	if(len<0)
		len = (SQInteger)scstrlen(news);
	SQHash h = ::_hashstr(news,len)&(_numofslots-1);
	SQString *s;
	for (s = _strings[h]; s; s = s->_next){
		if(s->_len == len && (!memcmp(news,s->_val,rsl(len))))
			return s; //found
	}

	SQString *t=(SQString *)SQ_MALLOC(rsl(len)+sizeof(SQString));
	new (t) SQString;
	t->_sharedstate = _sharedstate;
	memcpy(t->_val,news,rsl(len));
	t->_val[len] = _SC('\0');
	t->_len = len;
	t->_utf8_len = -1; // will calculate later
	t->_hash = ::_hashstr(news,len);
	t->_next = _strings[h];
	_strings[h] = t;
	_slotused++;
	if (_slotused > _numofslots)  /* too crowded? */
		Resize(_numofslots*2);
	return t;
}

void SQStringTable::Resize(SQInteger size)
{
	SQInteger oldsize=_numofslots;
	SQString **oldtable=_strings;
	AllocNodes(size);
	for (SQInteger i=0; i<oldsize; i++){
		SQString *p = oldtable[i];
		while(p){
			SQString *next = p->_next;
			SQHash h = p->_hash&(_numofslots-1);
			p->_next = _strings[h];
			_strings[h] = p;
			p = next;
		}
	}
	SQ_FREE(oldtable,oldsize*sizeof(SQString*));
}

void SQStringTable::Remove(SQString *bs)
{
	SQString *s;
	SQString *prev=NULL;
	SQHash h = bs->_hash&(_numofslots - 1);
	
	for (s = _strings[h]; s; ){
		if(s == bs){
			if(prev)
				prev->_next = s->_next;
			else
				_strings[h] = s->_next;
			_slotused--;
			SQInteger slen = s->_len;
			s->~SQString();
			SQ_FREE(s,sizeof(SQString) + rsl(slen));
			return;
		}
		prev = s;
		s = s->_next;
	}
	assert(0);//if this fail something is wrong
}
