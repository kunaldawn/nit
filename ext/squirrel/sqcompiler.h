/*	see copyright notice in squirrel.h */
#ifndef _SQCOMPILER_H_
#define _SQCOMPILER_H_

struct SQVM;

#define	TK_IDENTIFIER		258

#define	TK_STRING_LITERAL	259
#define	TK_INTEGER			260
#define	TK_FLOAT			261

#define	TK_EQ				262
#define	TK_NE				263
#define	TK_LE				264
#define	TK_GE				265
#define	TK_ARROW			266
#define	TK_AND				270
#define	TK_OR				271
#define	TK_NEWSLOT			272
#define	TK_UMINUS			273
#define	TK_PLUSEQ			274
#define	TK_MINUSEQ			275
#define TK_SHIFTL			276
#define TK_SHIFTR			277
#define TK_DOUBLE_COLON		278
#define TK_PLUSPLUS			279
#define TK_MINUSMINUS		280
#define TK_3WAYSCMP			281
#define TK_USHIFTR			282
#define TK_VARPARAMS		283
#define TK_MULEQ			284
#define TK_DIVEQ			285
#define TK_MODEQ			286
#define TK_LAMBDA			287

#define TK_KEYWORD_START	300

#define	TK_BASE				300
#define	TK_SWITCH			301
#define	TK_IF				302
#define	TK_ELSE				303
#define	TK_WHILE			304
#define	TK_BREAK			305
#define	TK_FOR				306
#define	TK_DO				307
#define	TK_NULL				308
#define	TK_FOREACH			309
#define	TK_IN				310
#define	TK_CLONE			311
#define	TK_FUNCTION			312
#define	TK_RETURN			313
#define	TK_TYPEOF			314
#define	TK_CONTINUE			315
#define TK_YIELD			316
#define TK_TRY				317
#define TK_CATCH			318
#define TK_THROW			319
#define TK_RESUME			320
#define TK_CASE				321
#define TK_DEFAULT			322
#define TK_THIS				323
#define TK_CLASS			324
#define TK_CONSTRUCTOR		325
#define TK_IS				326
#define TK_TRUE				327
#define TK_FALSE			328
#define TK_STATIC			329
#define TK_ENUM				330
#define TK_CONST			331
#define TK_PROPERTY			332
#define TK_REQUIRE			333
#define TK_INTDIV			334
#define TK_INTMOD			335
#define TK_DESTRUCTOR		336
#define TK_VAR				337
#define TK_WITH				338
#define TK_CHAR				339

typedef void(*CompilerErrorFunc)(void *ud, const SQChar *s);
bool Compile(SQVM *vm, SQLEXREADFUNC rg, SQUserPointer up, const SQChar *sourcename, SQObjectPtr &out, bool raiseerror, bool lineinfo);
#endif //_SQCOMPILER_H_
