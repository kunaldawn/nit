/*	see copyright notice in squirrel.h */
#ifndef _SQARRAY_H_
#define _SQARRAY_H_

struct SQArray : public CHAINABLE_OBJ
{
private:
	SQArray(SQSharedState *ss,SQInteger nsize){_values.resize(nsize); INIT_CHAIN();ADD_TO_CHAIN(&_ss(this)->_gc_chain,this);}
	~SQArray()
	{
		REMOVE_FROM_CHAIN(&_ss(this)->_gc_chain,this);
	}
public:
	static SQArray* Create(SQSharedState *ss,SQInteger nInitialSize){
		SQArray *newarray=(SQArray*)SQ_MALLOC(sizeof(SQArray));
		new (newarray) SQArray(ss,nInitialSize);
		return newarray;
	}
#ifndef NO_GARBAGE_COLLECTOR
	void Mark(SQGC* chain);
#endif
	void Finalize(){
		_values.resize(0);
	}
	bool Get(const SQInteger nidx,SQObjectPtr &val)
	{
		if(nidx>=0 && nidx<(SQInteger)_values.size()){
			SQObjectPtr &o = _values[nidx];
			_getrealval(o, NULL, val);
			return true;
		}
		else return false;
	}
	bool Set(const SQInteger nidx,const SQObjectPtr &val)
	{
		if(nidx>=0 && nidx<(SQInteger)_values.size()){
			_values[nidx]=val;
			GC_MUTATED(this);
			return true;
		}
		else return false;
	}
	SQInteger Next(const SQObjectPtr &refpos,SQObjectPtr &outkey,SQObjectPtr &outval)
	{
		SQUnsignedInteger idx=TranslateIndex(refpos);
		while(idx<_values.size()){
			//first found
			outkey=(SQInteger)idx;
			SQObjectPtr &o = _values[idx];
			_getrealval(o, NULL, outval);
			//return idx for the next iteration
			return ++idx;
		}
		//nothing to iterate anymore
		return -1;
	}
	SQArray *Clone(){SQArray *anew=Create(_opt_ss(this),Size()); anew->_values.copy(_values); return anew; }
	SQInteger Size() const {return _values.size();}
	void Resize(SQInteger size,SQObjectPtr &fill = _null_) { _values.resize(size,fill); ShrinkIfNeeded(); }
	void Reserve(SQInteger size) { _values.reserve(size); }
	void Append(const SQObject &o){_values.push_back(o); GC_MUTATED(this);}
	void Extend(const SQArray *a);
	SQObjectPtr &Top(){return _values.top();}
	void Pop(){_values.pop_back(); ShrinkIfNeeded(); }
	bool Insert(SQInteger idx,const SQObject &val){
		if(idx < 0 || idx > (SQInteger)_values.size())
			return false;
		_values.insert(idx,val);
		GC_MUTATED(this);
		return true;
	}
	void ShrinkIfNeeded() {
		if(_values.size() <= _values.capacity()>>2) //shrink the array
			_values.shrinktofit();
	}
	bool Remove(SQInteger idx){
		if(idx < 0 || idx >= (SQInteger)_values.size())
			return false;
		_values.remove(idx);
		ShrinkIfNeeded();
		return true;
	}
	void Release()
	{
		sq_delete(this,SQArray);
	}
	SQObjectPtrVec _values;
};
#endif //_SQARRAY_H_
