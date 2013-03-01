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

#ifndef _SQXAPI_H_
#define _SQXAPI_H_

// WARNING: if an extern "C" function throws, 
// the destructor call of above C stack executes not properly, 
// do not declare 'extern "C"' if a function needs or happens to throw!

/* extended error handling */
SQUIRREL_API SQInteger 		sqx_throw(HSQUIRRELVM v, const SQChar* errstr);
SQUIRREL_API SQInteger 		sqx_throwfmt(HSQUIRRELVM v, const SQChar* fmt, ...);

/* extended object creation handling */
SQUIRREL_API SQBool			sqx_getbool(HSQUIRRELVM v, SQInteger idx);
SQUIRREL_API SQInteger		sqx_getinteger(HSQUIRRELVM v, SQInteger idx);
SQUIRREL_API SQFloat		sqx_getfloat(HSQUIRRELVM v, SQInteger idx);
SQUIRREL_API const SQChar*	sqx_getstring(HSQUIRRELVM v, SQInteger idx);
SQUIRREL_API HSQUIRRELVM	sqx_getthread(HSQUIRRELVM v, SQInteger idx);
SQUIRREL_API SQUserPointer 	sqx_getuserpointer(HSQUIRRELVM v, SQInteger idx);
SQUIRREL_API SQUserPointer 	sqx_getinstanceup(HSQUIRRELVM v, SQInteger idx, SQUserPointer typetag);

SQUIRREL_API SQInteger		sqx_optinteger(HSQUIRRELVM v, SQInteger idx, SQInteger def);
SQUIRREL_API SQFloat		sqx_optfloat(HSQUIRRELVM v, SQInteger idx, SQFloat def);
SQUIRREL_API const SQChar*	sqx_optstring(HSQUIRRELVM v, SQInteger idx, const SQChar* def);

SQUIRREL_API SQInteger		sqx_getonlyinteger(HSQUIRRELVM v, SQInteger idx);
SQUIRREL_API SQFloat		sqx_getonlyfloat(HSQUIRRELVM v, SQInteger idx);

SQUIRREL_API SQBool			sqx_instanceof(HSQUIRRELVM v, SQInteger idx, SQUserPointer typetag);

SQUIRREL_API void			sqx_pushstringf(HSQUIRRELVM v, const SQChar* fmt, ...);

#ifdef __cplusplus
extern "C" {
#endif

/* extended debug support */
#define sqx_printf(v, ...)	sq_getprintfunc(v)(v, __VA_ARGS__)
#define sqx_prterrf(v, ...) sq_geterrorfunc(v)(v, __VA_ARGS__)
SQUIRREL_API SQRESULT		sqx_dumpfunc(HSQUIRRELVM v, SQInteger idx);
SQUIRREL_API void			sqx_dump(HSQUIRRELVM v);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _SQXAPI_H_ */