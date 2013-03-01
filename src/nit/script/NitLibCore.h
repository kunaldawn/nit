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

#include "nit/event/EventAutomata.h"

#include "nit/script/ScriptRuntime.h"

#include "nit/data/DataValue.h"

NS_NIT_BEGIN;

//////////////////////////////////////////////////////////////////////////

class NIT_API ScriptDataValue
{
public:
	static SQRESULT						toValue(HSQUIRRELVM v, int idx, DataValue& outValue);
	static SQRESULT						toArray(HSQUIRRELVM v, int idx, DataArray* array);
	static SQRESULT						toRecord(HSQUIRRELVM v, int idx, DataRecord* record);

	static SQInteger					push(HSQUIRRELVM v, const DataValue& value, bool fullTransform = false);
	static SQInteger					push(HSQUIRRELVM v, DataArray* array, bool fullTransform = false);
	static SQInteger					push(HSQUIRRELVM v, DataRecord* record, bool fullTransform = false);
};

//////////////////////////////////////////////////////////////////////////

NS_NIT_END;
