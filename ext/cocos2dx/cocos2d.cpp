#include "nit2d_pch.h"

/****************************************************************************
Copyright (c) 2010-2011 cocos2d-x.org
Copyright (c) 2008-2010 Ricardo Quesada
Copyright (c) 2011      Zynga Inc.

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

#include "cocos2d.h"

NS_CC_BEGIN;

const char* cocos2dVersion()
{
	return "cocos2d v1.0.1";
}

void CCLog(const char* tag, const char* src, int line, const char* func, const char * pszFormat, ...)
{
	char szBuf[MAX_PATH];

	va_list ap;
	va_start(ap, pszFormat);
	size_t len = vsnprintf(szBuf, MAX_PATH, pszFormat, ap);
	szBuf[len++] = '\n';
	szBuf[len] = 0;
	va_end(ap);

	LogManager& lmgr = LogManager::getSingleton();
	static Ref<LogChannel> ccChannel = new LogChannel("cocos");

	LogManager::getSingleton().doLog(
		ccChannel, // channel
		NULL, // act
		src, // srcname
		line, // line
		func, // fnname
		tag, // tag
		szBuf, // msg
		len, // msgLen
		true // forceLineEnd
		);

	//     WCHAR wszBuf[MAX_LEN] = {0};
	//     MultiByteToWideChar(CP_UTF8, 0, szBuf, -1, wszBuf, sizeof(wszBuf));
	//     OutputDebugStringW(wszBuf);
	//     OutputDebugStringA("\n");
}

NS_CC_END;
