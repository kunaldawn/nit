/*	see copyright notice in squirrel.h */
#ifndef _SQPCHEADER_H_
#define _SQPCHEADER_H_

#ifndef SQUIRREL_API
#	if defined(_WIN32)
#		if defined(NIT_BUILD_DLL)
#			define SQUIRREL_API __declspec(dllexport)
#		else
#			define SQUIRREL_API __declspec(dllimport)
#		endif
#	else
#		define SQUIRREL_API
#	endif
#endif

#if defined(_MSC_VER) && defined(_DEBUG)
#include <crtdbg.h>
#endif 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <new>
//squirrel stuff
#include "squirrel.h"
#include "sqobject.h"
#include "sqstate.h"

#endif //_SQPCHEADER_H_
