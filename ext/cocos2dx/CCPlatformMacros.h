/****************************************************************************
 Copyright (c) 2010 cocos2d-x.org
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/
#ifndef __CC_PLATFORM_MACROS_H__
#define __CC_PLATFORM_MACROS_H__

/**
 * define some platform specific macros
 */
#include "ccConfig.h"
#include "CCPlatformConfig.h"
#include "nit/nit.h"

#define MacGLView						void
#define NSWindow					    void

// generic macros

// namespace cocos2d {}
#define NS_CC_BEGIN                     namespace cocos2d {
#define NS_CC_END                       }
#define USING_NS_CC                     using namespace cocos2d

/** CC_PROPERTY_READONLY is used to declare a protected variable.
 We can use getter to read the variable.
 @param varType : the type of variable.
 @param varName : variable name.
 @param funName : "get + funName" is the name of the getter.
 @warning : The getter is a public virtual function, you should rewrite it first.
 The variables and methods declared after CC_PROPERTY_READONLY are all public.
 If you need protected or private, please declare.
 */
#define CC_PROPERTY_READONLY(varType, varName, funName)\
protected: varType varName;\
public: virtual varType get##funName(void);

#define CC_PROPERTY_READONLY_PASS_BY_REF(varType, varName, funName)\
protected: varType varName;\
public: virtual const varType& get##funName(void);

/** CC_PROPERTY is used to declare a protected variable.
 We can use getter to read the variable, and use the setter to change the variable.
 @param varType : the type of variable.
 @param varName : variable name.
 @param funName : "get + funName" is the name of the getter.
 "set + funName" is the name of the setter.
 @warning : The getter and setter are public virtual functions, you should rewrite them first.
 The variables and methods declared after CC_PROPERTY are all public.
 If you need protected or private, please declare.
 */
#define CC_PROPERTY(varType, varName, funName)\
protected: varType varName;\
public: virtual varType get##funName(void);\
public: virtual void set##funName(varType var);

#define CC_PROPERTY_PASS_BY_REF(varType, varName, funName)\
protected: varType varName;\
public: virtual const varType& get##funName(void);\
public: virtual void set##funName(const varType& var);

/** CC_SYNTHESIZE_READONLY is used to declare a protected variable.
 We can use getter to read the variable.
 @param varType : the type of variable.
 @param varName : variable name.
 @param funName : "get + funName" is the name of the getter.
 @warning : The getter is a public inline function.
 The variables and methods declared after CC_SYNTHESIZE_READONLY are all public.
 If you need protected or private, please declare.
 */
#define CC_SYNTHESIZE_READONLY(varType, varName, funName)\
protected: varType varName;\
public: inline varType get##funName(void) const { return varName; }

#define CC_SYNTHESIZE_READONLY_PASS_BY_REF(varType, varName, funName)\
protected: varType varName;\
public: inline const varType& get##funName(void) const { return varName; }

/** CC_SYNTHESIZE is used to declare a protected variable.
 We can use getter to read the variable, and use the setter to change the variable.
 @param varType : the type of variable.
 @param varName : variable name.
 @param funName : "get + funName" is the name of the getter.
 "set + funName" is the name of the setter.
 @warning : The getter and setter are public  inline functions.
 The variables and methods declared after CC_SYNTHESIZE are all public.
 If you need protected or private, please declare.
 */
#define CC_SYNTHESIZE(varType, varName, funName)\
protected: varType varName;\
public: inline varType get##funName(void) const { return varName; }\
public: inline void set##funName(varType var){ varName = var; }

#define CC_SYNTHESIZE_PASS_BY_REF(varType, varName, funName)\
protected: varType varName;\
public: inline const varType& get##funName(void) const { return varName; }\
public: inline void set##funName(const varType& var){ varName = var; }

#define CC_SAFE_DELETE(p)			if(p) { delete p; p = 0; }
#define CC_SAFE_DELETE_ARRAY(p)    if(p) { delete[] p; p = 0; }
#define CC_SAFE_FREE(p)			if(p) { free(p); p = 0; }
#define CC_SAFE_RELEASE(p)			if(p) { p->release(); }
#define CC_SAFE_RELEASE_NULL(p)	if(p) { p->release(); p = 0; }
#define CC_SAFE_RETAIN(p)			if(p) { p->retain(); }
#define CC_BREAK_IF(cond)			if(cond) break;

#ifdef NIT_DEBUG
#define COCOS2D_DEBUG 1
#endif

#if defined(NIT_FASTDEBUG) || defined(NIT_RELEASE)
#define COCOS2D_DEBUG 1
#endif

// cocos2d debug
#if !defined(COCOS2D_DEBUG) || COCOS2D_DEBUG == 0
#define CCLOG(...)              
#define CCLOGINFO(...)         
#define CCLOGERROR(...)         

#elif COCOS2D_DEBUG == 1
#define CCLOG(format, ...)      cocos2d::CCLog("--", __FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__)
#define CCLOGERROR(format,...)  cocos2d::CCLog("***", __FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__)
#define CCLOGINFO(format,...)   do {} while (0)

#elif COCOS2D_DEBUG > 1
#define CCLOG(format, ...)      cocos2d::CCLog("--", __FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__)
#define CCLOGERROR(format,...)  cocos2d::CCLog("***", __FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__)
#define CCLOGINFO(format,...)   cocos2d::CCLog("..", __FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__)
#endif // COCOS2D_DEBUG

// shared library declartor
#define CC_DLL                 

// assertion
#ifndef NIT_SHIPPING
#	define CC_ASSERT(cond)				ASSERT_THROW(cond, EX_RENDER)
#else
#	define CC_ASSERT(cond)				(void)0
#endif

#define CC_UNUSED_PARAM(unusedparam)   (void)unusedparam

#if (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
#undef CC_UNUSED_PARAM
#define CC_UNUSED_PARAM(unusedparam)   //unusedparam
#endif


// platform depended macros

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
#ifndef CC_STATIC
    #undef CC_DLL
    #if defined(_USRDLL)
        #define CC_DLL     __declspec(dllexport)
    #else 		/* use a DLL library */
        #define CC_DLL     __declspec(dllimport)
    #endif
#endif // CC_STATIC
#endif  // CC_PLATFORM_WIN32

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WOPHONE && defined(_TRANZDA_VM_))

    #undef CC_DLL
    #if defined(SS_MAKEDLL)
        #define CC_DLL     __declspec(dllexport)
    #else 		/* use a DLL library */
        #define CC_DLL     __declspec(dllimport)
    #endif

#endif  // wophone VM

#endif // __CC_PLATFORM_MACROS_H__
