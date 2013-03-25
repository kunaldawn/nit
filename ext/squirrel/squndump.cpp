/*	see copyright notice in squirrel.h */
#include "sqpcheader.h"
#include "sqcompiler.h"
#include "sqfuncproto.h"
#include "sqstring.h"
#include "sqtable.h"
#include "sqopcodes.h"
#include "sqfuncstate.h"
#include "sqvm.h"
#include "squserdata.h"
#include "sqclosure.h"
#include "sqxapi.h"

SQInstructionDesc g_InstrDesc[]={
	{_SC("LINE")},
	{_SC("LOAD")},
	{_SC("LOADINT")},
	{_SC("LOADFLOAT")},
	{_SC("DLOAD")},
	{_SC("TAILCALL")},
	{_SC("CALL")},
	{_SC("PREPCALL")},
	{_SC("PREPCALLK")},
	{_SC("GETK")},
	{_SC("MOVE")},
	{_SC("NEWSLOT")},
	{_SC("REQUIRE")},
	{_SC("SET")},
	{_SC("GET")},
	{_SC("EQ")},
	{_SC("NE")},
	{_SC("ADD")},
	{_SC("SUB")},
	{_SC("MUL")},
	{_SC("DIV")},
	{_SC("MOD")},
	{_SC("BITW")},
	{_SC("RETURN")},
	{_SC("LOADNULLS")},
	{_SC("LOADROOT")},
	{_SC("LOADBOOL")},
	{_SC("DMOVE")},
	{_SC("JMP")},
	{_SC("JNZ")},
	{_SC("JZ")},
	{_SC("SETOUTER")},
	{_SC("GETOUTER")},
	{_SC("NEWOBJ")},
	{_SC("APPENDARRAY")},
	{_SC("COMPARITH")},
	{_SC("INC")},
	{_SC("INCL")},
	{_SC("PINC")},
	{_SC("PINCL")},
	{_SC("CMP")},
	{_SC("EXISTS")},
	{_SC("INSTANCEOF")},
	{_SC("AND")},
	{_SC("OR")},
	{_SC("NEG")},
	{_SC("NOT")},
	{_SC("BWNOT")},
	{_SC("CLOSURE")},
	{_SC("YIELD")},
	{_SC("RESUME")},
	{_SC("FOREACH")},
	{_SC("POSTFOREACH")},
	{_SC("CLONE")},
	{_SC("TYPEOF")},
	{_SC("PUSHTRAP")},
	{_SC("POPTRAP")},
	{_SC("THROW")},
	{_SC("NEWSLOTA")},
	{_SC("GETBASE")},
	{_SC("CLOSE")},
	{_SC("NEWPROP")},
	{_SC("INTDIV")},
	{_SC("INTMOD")},
	{_SC("ASSIGN")},
	{_SC("SWAP")},
	{_SC("DSWAP")},
	{_SC("RETTRAP")},
	{_SC("IMPORT")},
};

static void squndump_dumpliteral(HSQUIRRELVM v, SQObjectPtr &o)
{
	switch(sqi_type(o)){
		case OT_STRING:	sqx_printf(v, _SC("\"%s\""),sqi_stringval(o));break;
		case OT_FLOAT: sqx_printf(v, _SC("%f"),sqi_float(o));break;
		case OT_INTEGER: sqx_printf(v, _SC("%d"),sqi_integer(o));break;
		case OT_BOOL: sqx_printf(v, _SC("%s"),sqi_integer(o)?_SC("true"):_SC("false"));break;
		default: sqx_printf(v, _SC("(%s %p)"),GetTypeName(o),(void*)sqi_rawval(o));break; break; //shut up compiler
	}
}

static void squndump_dumpfunc(HSQUIRRELVM v, SQClosure* c)
{
	SQFunctionProto* f = c->_function;
	SQInteger n=0,i;
	SQInteger si;
	sqx_printf(v, _SC("* function \"%s\"\n"),sqi_type(f->_name)==OT_STRING?sqi_stringval(f->_name):_SC("unknown"));

	sqx_printf(v, _SC("- source: \"%s\""), sqi_type(f->_sourcename)==OT_STRING?sqi_stringval(f->_sourcename):_SC("unknown"));
	if (f->_nlineinfos == 0) 
		sqx_printf(v, _SC("\n"));
	else
		sqx_printf(v, _SC(" line %d\n"), f->_lineinfos[0]._line);

	sqx_printf(v, _SC("- max stack size: %d\n"),f->_stacksize);

	if (sqi_type(c->_env) != OT_NULL)
	{
		sqx_printf(v, _SC("- env: "));
		squndump_dumpliteral(v, c->_env);
	}

	if (f->_nliterals)
	{
		sqx_printf(v, _SC("- literals:\n"));
		for (i=0; i < f->_nliterals; ++i)
		{
			sqx_printf(v, _SC("  [%d] "),n);
			squndump_dumpliteral(v, f->_literals[i]);
			sqx_printf(v, _SC("\n"));
			n++;
		}
	}
	sqx_printf(v, _SC("- parameters:\n"));
	if (!f->_varparams && f->_nparameters == 0) sqx_printf(v, _SC("  none\n"));
	if(f->_varparams)
		sqx_printf(v, _SC("  <<VARPARAMS>>\n"));
	n=0;
	for (i=0; i< f->_nparameters; ++i)
	{
		sqx_printf(v, _SC("  [%d] "),n);
		squndump_dumpliteral(v, f->_parameters[i]);
		sqx_printf(v, _SC("\n"));
		n++;
	}
	if (f->_noutervalues)
	{
		sqx_printf(v, _SC("- outers:\n"));
		for(si=0;si<f->_noutervalues;si++){
			SQOuterVar ov=f->_outervalues[si];
			sqx_printf(v, _SC("  [%d] %s (%s->%d) = "), si, sqi_stringval(ov._name), ov._type == otLOCAL ? _SC("local") : ov._type == otOUTER ? _SC("outer") : _SC("???"), sqi_integer(ov._src));
			squndump_dumpliteral(v, sqi_outer(c->_outervalues[si])->_value);
			sqx_printf(v, _SC("\n"));
		}
	}
	if (f->_nlocalvarinfos)
	{
		sqx_printf(v, _SC("- locals:\n"));
		for(si=0;si<f->_nlocalvarinfos;si++){
			SQLocalVarInfo lvi=f->_localvarinfos[si];
			sqx_printf(v, _SC("  [%d] %s op[%d-%d]\n"),lvi._pos,sqi_stringval(lvi._name),lvi._start_op,lvi._end_op);
			n++;
		}
	}
	if (f->_nlineinfos)
	{
		sqx_printf(v, _SC("- line info:\n"));
		for(i=0;i<f->_nlineinfos;i++){
			SQLineInfo li=f->_lineinfos[i];
			sqx_printf(v, _SC("  op [%d] line [%d] \n"),li._op,li._line);
			n++;
		}
	}
	if (f->_ninstructions)
	{
		sqx_printf(v, _SC("- opcode:\n"));
		n=0;
		for(i=0;i<f->_ninstructions;i++){
			SQInstruction &inst=f->_instructions[i];
			sqx_printf(v, _SC("  [%03d] %02X (%02X %08X %02X %02X): "), n, inst.op, inst._arg0, inst._arg1, inst._arg2, inst._arg3);
			if(inst.op==_OP_LOAD || inst.op==_OP_DLOAD || inst.op==_OP_PREPCALLK || inst.op==_OP_GETK ){

				SQInteger lidx = inst._arg1;
				sqx_printf(v, _SC("%15s %d "),g_InstrDesc[inst.op].name,inst._arg0);
				if(lidx >= 0xFFFFFFFF)
					sqx_printf(v, _SC("null"));
				else {
					squndump_dumpliteral(v, f->_literals[lidx]);
				}
				if(inst.op != _OP_DLOAD) {
					sqx_printf(v, _SC(" %d %d \n"),inst._arg2,inst._arg3);
				}
				else {
					sqx_printf(v, _SC(" %d "),inst._arg2);
					lidx = inst._arg3;
					if(lidx >= 0xFFFFFFFF)
						sqx_printf(v, _SC("null"));
					else {
						squndump_dumpliteral(v, f->_literals[lidx]);
						sqx_printf(v, _SC("\n"));
					}
				}
			}
			else if(inst.op==_OP_LOADFLOAT) {
				sqx_printf(v, _SC("%15s %d %f %d %d\n"),g_InstrDesc[inst.op].name,inst._arg0,*((SQFloat*)&inst._arg1),inst._arg2,inst._arg3);
			}
			else 
				sqx_printf(v, _SC("%15s %d %d %d %d\n"),g_InstrDesc[inst.op].name,inst._arg0,inst._arg1,inst._arg2,inst._arg3);
			n++;
		}
	}
}

static void squndump_dumpnative(HSQUIRRELVM v, SQNativeClosure* c)
{
	sqx_printf(v, _SC("* native closure \"%s\"\n"), c->_name ? c->_name : _SC("unknown"));
	sqx_printf(v, _SC("- source: \"%s\"\n"), c->_file ? c->_file : _SC("unknown"));
	sqx_printf(v, _SC("- function pointer: 0x%08x\n"), c->_function);

	if (sqi_type(c->_env) != OT_NULL)
	{
		sqx_printf(v, _SC("- env: "));
		squndump_dumpliteral(v, c->_env);
	}

	if (!c->_outervalues.empty())
	{
		sqx_printf(v, _SC("- outers:\n"));
		for (SQUnsignedInteger i=0; i<c->_outervalues.size(); ++i)
		{
			sqx_printf(v, _SC("  [%d] "), i);
			squndump_dumpliteral(v, c->_outervalues[i]);
			sqx_printf(v, _SC("\n"));
		}
	}
}

static void squndump_dumpstack(HSQUIRRELVM v)
{
	SQInteger stackbase = v->_stackbase;

	sqx_printf(v, _SC("* stack dump:\n"));
	for (SQInteger i=v->_top-1; i>=0; --i)
	{
		int idx = i - stackbase + 1;
		int minus = v->_top - i;

		if(stackbase==i)sqx_printf(v, _SC("  >"));else sqx_printf(v, _SC("   "));

		if (idx > 0)
		{
			sqx_printf(v, _SC("[%02d] [-%02d] [%03d]:"),idx, minus, i);
		}
		else
		{
			sqx_printf(v, _SC("[--] [---] [%03d]:"), i);
		}

		SQObjectPtr &obj=v->_stack[i];	
		switch(sqi_type(obj)){
		case OT_FLOAT:			sqx_printf(v, _SC("FLOAT %.3f"),sqi_float(obj));break;
		case OT_INTEGER:		sqx_printf(v, _SC("INTEGER %d"),sqi_integer(obj));break;
		case OT_BOOL:			sqx_printf(v, _SC("BOOL %s"),sqi_integer(obj)?"true":"false");break;
		case OT_STRING:			sqx_printf(v, _SC("STRING \"%s\""),sqi_stringval(obj));break;
		case OT_NULL:			sqx_printf(v, _SC("NULL"));	break;
		case OT_TABLE:			sqx_printf(v, _SC("TABLE %p[%p]"),sqi_table(obj),sqi_table(obj)->_delegate);break;
		case OT_ARRAY:			sqx_printf(v, _SC("ARRAY %p"),sqi_array(obj));break;
		case OT_CLOSURE:		sqx_printf(v, _SC("CLOSURE [%p]"),sqi_closure(obj));break;
		case OT_NATIVECLOSURE:	sqx_printf(v, _SC("NATIVECLOSURE"));break;
		case OT_USERDATA:		sqx_printf(v, _SC("USERDATA %p[%p]"),sqi_userdataval(obj),sqi_userdata(obj)->_delegate);break;
		case OT_GENERATOR:		sqx_printf(v, _SC("GENERATOR %p"),sqi_generator(obj));break;
		case OT_THREAD:			sqx_printf(v, _SC("THREAD [%p]"),sqi_thread(obj));break;
		case OT_USERPOINTER:	sqx_printf(v, _SC("USERPOINTER %p"),sqi_userpointer(obj));break;
		case OT_CLASS:			sqx_printf(v, _SC("CLASS %p"),sqi_class(obj));break;
		case OT_INSTANCE:		sqx_printf(v, _SC("INSTANCE %p"),sqi_instance(obj));break;
		case OT_WEAKREF:		sqx_printf(v, _SC("WEAKERF %p"),sqi_weakref(obj));break;
		case OT_NATIVEWEAKREF:	sqx_printf(v, _SC("NATIVEWEAKREF %p"),sqi_nativeweakref(obj));break;
		default:
			assert(0);
			break;
		};
		sqx_printf(v, _SC("\n"));

		if (idx == 1)
			sqx_printf(v, _SC("--------------------------------\n"));
	}
}

SQRESULT sqx_dumpfunc(HSQUIRRELVM v, SQInteger idx)
{
	SQObject o = stack_get(v, idx);
	if (sq_isclosure(o))
	{
		SQClosure* c = sqi_closure(o);
		squndump_dumpfunc(v, c);
		return SQ_OK;
	}
	if (sq_isnativeclosure(o))
	{
		SQNativeClosure* c = sqi_nativeclosure(o);
		squndump_dumpnative(v, c);
		return SQ_OK;
	}
	return sq_throwerror(v,_SC("the object is not a closure"));
}

void sqx_dumpstacktrace(HSQUIRRELVM v)
{
	SQPRINTFUNCTION pf = sq_getprintfunc(v);
	if (pf == NULL) return;

	pf(v, _SC("* call stack trace:\n"));

	SQStackInfos si;
	SQInteger i;
	SQFloat f;
	const SQChar *s;
	SQInteger level=0;
	const SQChar *name=0; 
	SQInteger seq=0;

	while(SQ_SUCCEEDED(sq_stackinfos(v,level,&si)))
	{
		const SQChar *fn=_SC("unknown");
		const SQChar *src=_SC("unknown");
		if(si.funcname)fn=si.funcname;
		if(si.source)src=si.source;

		if (si.line >= 0)
			pf(v,_SC("- function %s() from '%s' line %d\n"),fn,src,si.line);
		else
			pf(v,_SC("- function %s() from '%s'\n"),fn,src);

		seq=0;
		while((name = sq_getlocal(v,level,seq)))
		{
			seq++;
			switch(sq_gettype(v,-1))
			{
			case OT_NULL:
				pf(v,_SC("  [%s] NULL\n"),name);
				break;
			case OT_INTEGER:
				sq_getinteger(v,-1,&i);
				pf(v,_SC("  [%s] %d\n"),name,i);
				break;
			case OT_FLOAT:
				sq_getfloat(v,-1,&f);
				pf(v,_SC("  [%s] %.14g\n"),name,f);
				break;
			case OT_USERPOINTER:
				pf(v,_SC("  [%s] USERPOINTER\n"),name);
				break;
			case OT_STRING:
				sq_getstring(v,-1,&s);
				pf(v,_SC("  [%s] \"%s\"\n"),name,s);
				break;
			case OT_TABLE:
				pf(v,_SC("  [%s] TABLE\n"),name);
				break;
			case OT_ARRAY:
				pf(v,_SC("  [%s] ARRAY\n"),name);
				break;
			case OT_CLOSURE:
				pf(v,_SC("  [%s] CLOSURE\n"),name);
				break;
			case OT_NATIVECLOSURE:
				pf(v,_SC("  [%s] NATIVECLOSURE\n"),name);
				break;
			case OT_GENERATOR:
				pf(v,_SC("  [%s] GENERATOR\n"),name);
				break;
			case OT_USERDATA:
				pf(v,_SC("  [%s] USERDATA\n"),name);
				break;
			case OT_THREAD:
				pf(v,_SC("  [%s] THREAD\n"),name);
				break;
			case OT_CLASS:
				pf(v,_SC("  [%s] CLASS\n"),name);
				break;
			case OT_INSTANCE:
				pf(v,_SC("  [%s] INSTANCE\n"),name);
				break;
			case OT_WEAKREF:
				pf(v,_SC("  [%s] WEAKREF\n"),name);
				break;
			case OT_NATIVEWEAKREF:
				pf(v, _SC("  [%s] NATIVEWEAKREF\n"), name);
				break;
			case OT_BOOL:{
				sq_getinteger(v,-1,&i);
				pf(v,_SC("  [%s] %s\n"),name,i?_SC("true"):_SC("false"));
						 }
						 break;
			default: assert(0); break;
			}
			sq_pop(v,1);
		}

		level++;
	}
}

void sqx_dump(HSQUIRRELVM v)
{
	sqx_printf(v, _SC("* Internal of vm %08x\n"), v);
	squndump_dumpstack(v);
	sqx_dumpstacktrace(v);
}