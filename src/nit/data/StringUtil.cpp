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

#include "nit_pch.h"

#include "StringUtil.h"

#include "nit/runtime/MemManager.h"

NS_NIT_BEGIN;

////////////////////////////////////////////////////////////////////////////////

template <typename TChar>
static TChar* allocBuf(size_t len)
{
	return (TChar*)NIT_ALLOC(len * sizeof(TChar));
}

template <typename TChar>
static void freeBuf(TChar* buf)
{
	NIT_DEALLOC(buf, 0);
}

UniString Unicode::toUtf16(const char* str, int len, CodePage codePage)
{
	len = len ? len : strlen(str);
	if (codePage == CODE_DEFAULT) codePage = CODE_UTF8;
	
	if (codePage == CODE_UTF8)
	{
		UniString ret;
		ret.reserve(len);

		const char* utf8 = str;
		const char* end = str + len;
		while (utf8 < end)
		{
			int ch = utf8Advance(utf8);
			ret.push_back(UniChar(ch));
		}

		return ret;
	}

#if defined(NIT_WIN32)
	// NOTE: Generally, 2-MBCS chars converted to 1-UTF16 char, 2~3 UTF8 chars converted to 1-UTF16 char,
	// the length of converted string (in char) are always less than the length of original str (in byte).
	UniChar staticBuf[MAX_PATH];
	UniChar* dynamicBuf = len >= MAX_PATH ? allocBuf<UniChar>(len + 1) : NULL;

	UniChar* buf = dynamicBuf ? dynamicBuf : staticBuf;
	
	// Convert to UTF16
	int bufSize = MultiByteToWideChar(codePage, 0, str, len, buf, len);
	buf[bufSize] = 0;
	
	UniString ret(buf, bufSize);
	
	if (dynamicBuf)
		freeBuf<UniChar>(dynamicBuf);
	
	return ret;
#endif

	NIT_THROW(EX_NOT_SUPPORTED);
}

static const int	halfShift  = 10; /* used for shifting by 10 bits */ 
static const uint32 halfBase = 0x0010000UL; 
static const uint32 halfMask = 0x3FFUL; 
static const uint8	firstByteMark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC }; 

size_t Unicode::toUtf8Char(uint32 ch, char utf8Seq[4])
{
	const uint32 byteMask = 0xBF;
	const uint32 byteMark = 0x80;

	size_t bytesToWrite = 0;

	// figure out how many bytes the result will require
	if (ch < (uint32)0x80)
		bytesToWrite = 1;
	else if (ch < (uint32)0x800)
		bytesToWrite = 2;
	else if (ch < (uint32)0x10000)
		bytesToWrite = 3;
	else if (ch <= UNI_MAX_LEGAL_UTF32)
		bytesToWrite = 4;
	else
	{
		ch = UNI_REPLACEMENT_CHAR;
		bytesToWrite = 3;
	}

	char* seq = utf8Seq + bytesToWrite;
	switch (bytesToWrite)
	{
		// NOTE: case falls through...
	case 4: *--seq = (uint8)((ch | byteMark) & byteMask); ch >>= 6;
	case 3: *--seq = (uint8)((ch | byteMark) & byteMask); ch >>= 6;
	case 2: *--seq = (uint8)((ch | byteMark) & byteMask); ch >>= 6;
	case 1: *--seq = (uint8)((ch | firstByteMark[bytesToWrite]));
	}

	return bytesToWrite;
}

String Unicode::toUtf8(const UniChar* utf16str, int len)
{
	const UniChar* src = utf16str;

	if (len == 0)
	{
		for (; *src; ++src) ;
		len = src - utf16str;
		src = utf16str;
	}

	uint32 ch;

	String ret;

	if (len == 0)
		return ret;

	ret.reserve(len * 2);

	char utf8seq[4];

	const UniChar* end = utf16str + len;

	while (src < end)
	{
		ch = *src++;

		if (ch < 0x80)
		{
			ret.push_back(ch);
			continue;
		}

		// if we have a surrogate pair, convert to utf32 first
		if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_HIGH_END)
		{
			ch = UNI_REPLACEMENT_CHAR; // pessimistic approach

			// if the 16 bits following the high surrogate are in the string ...
			if (src < end)
			{
				uint32 ch2 = *src;
				// if it's a low surrogate, convert to utf32
				if (ch2 >= UNI_SUR_LOW_START && ch2 <= UNI_SUR_LOW_END)
				{
					ch = ((ch - UNI_SUR_HIGH_START) << halfShift) + (ch2 - UNI_SUR_LOW_START) + halfBase;
					++src;
				}
			}
		}
		else if (ch >= UNI_SUR_LOW_START && ch <= UNI_SUR_LOW_END) 
		{ 
			ch = UNI_REPLACEMENT_CHAR; 
		}

		size_t bytesToWrite = toUtf8Char(ch, utf8seq);
		ret.append(utf8seq, utf8seq + bytesToWrite);
	}

	return ret;
}

String Unicode::reencode(const char* srcStr, CodePage srcCodePage, CodePage destCodePage, int len)
{
#if defined(NIT_WIN32)
	len = len ? len : strlen(srcStr);
	if (srcCodePage == CODE_DEFAULT) srcCodePage = CODE_SYSTEM;
	if (destCodePage == CODE_DEFAULT) destCodePage = CODE_UTF8;
	
	UniChar staticUtf16[MAX_PATH];
	UniChar* dynamicUtf16 = len >= MAX_PATH ? allocBuf<UniChar>(len + 1) : NULL;

	UniChar* utf16 = dynamicUtf16 ? dynamicUtf16 : staticUtf16;
	
	// Convert to utf16
	int utf16Size = MultiByteToWideChar(srcCodePage, 0, srcStr, len, utf16, len);
	utf16[utf16Size] = 0;
	
	// determine length of buffer
	int destSize = WideCharToMultiByte(destCodePage, 0, utf16, utf16Size, 0, 0, 0, 0);

	char staticDest[MAX_PATH];
	char* dynamicDest = destSize >= MAX_PATH ? allocBuf<char>(destSize + 1) : NULL;

	char* dest = dynamicDest ? dynamicDest : staticDest;
	
	// Convert result to destCodePage
	destSize = WideCharToMultiByte(destCodePage, 0, utf16, utf16Size, dest, destSize, 0, 0);
	dest[destSize] = 0;
	
	String ret(dest, destSize);

	if (dynamicUtf16)
		freeBuf<UniChar>(dynamicUtf16);

	if (dynamicDest)
		freeBuf<char>(dynamicDest);
	
	return ret;
#endif

	NIT_THROW(EX_NOT_SUPPORTED);
}

size_t Unicode::utf8Length(const char* utf8)
{
	size_t len = 0;
	while (*utf8)
	{
		if ((*utf8 & 0xc0) != 0x80)
			++len;
		++utf8;
	}
	return len;
}

static const int utf8_codelen[16] =
{
	1, 1, 1, 1, 1, 1, 1, 1,				// 0000xxxx ~ 0111xxxx	: 1 byte(plain ascii)
	1, 1, 1, 1,							// 1000xxxx ~ 1011xxxx	: invalid (treat as 1 byte)
	2, 2,								// 1100xxxx ~ 1101xxxx	: 2 bytes
	3,									// 1110xxxx				: 3 bytes
	4									// 1111xxxx				: 4 bytes
};

static const int utf8_bytemasks[5] =
{
	0xff,								
	0xff,								// 1 byte  : xxxxxxxx
	0x1f,								// 2 bytes : 110xxxxx 10xxxxxx
	0x0f,								// 3 bytes : 1110xxxx 10xxxxxx
	0x07								// 4 bytes : 11110xxx 10xxxxxx
};

int Unicode::utf8ByteCount(const char* utf8, int charCount)
{
	const char* p = utf8;

	for (int i=0; *p && i<charCount; ++i)
		p = utf8Next(p);
	return p - utf8;
}

int Unicode::toUniChar(const char* utf8)
{
	return utf8Advance(utf8);
}

int Unicode::utf8Advance(const char*& utf8)
{
	int c = *(uint8*)utf8++;
	if (c & 0x80)
	{
		int codelen = utf8_codelen[c >> 4];
		c = c & utf8_bytemasks[codelen];
		for (int i = 1; i < codelen; ++i)
		{
			c <<= 6;
			c |= *(uint8*)utf8++ & 0x3f;
		}
	}
	return c;
}

const char* Unicode::utf8Next(const char* utf8)
{
	int codelen = utf8_codelen[*(uint8*)utf8 >> 4];
	return utf8 + codelen;
}

const char* Unicode::utf8Prev(const char* utf8)
{
	const uint8* p = (const uint8*)utf8;
	while ((*(--p) & 0xC0) == 0x80);

	return (const char*)p;
}

////////////////////////////////////////////////////////////////////////////////

bool Wildcard::has(const char* str)
{
	for (const char* ch = str; *ch; ++ch)
		if (*ch == '*' || *ch == '?' || *ch == '+') return true;

	return false;
}

static inline bool wcm_match(char a, char b, bool ignoreCase)
{
	return ignoreCase ? toupper(a) == toupper(b) : a == b;
}

static bool wcm_asterisk(const char* & wildcard, const char* & test, bool ignoreCase)
{
	bool match = true;

	//erase the leading asterisks
	++wildcard;
	while ((*test && (*wildcard == '?')) || *wildcard == '*')
	{
		if (*wildcard == '?') ++test;
		++wildcard;
	}

	while (*wildcard == '+')
		++wildcard;

	if (*test == 0 && *wildcard)
		return false;

	if (*test == 0 && *wildcard == 0)
		return true;

	// Neither test nor wildcard are empty!
	if (Wildcard::match(wildcard, test, ignoreCase) == false)
	{
		do
		{
			++test;

			// skip as much characters as possible in the test string
			// stop if a character match occurs
			while (!wcm_match(*wildcard, *test, ignoreCase) && *test)
				++test;
		}while (*test ? Wildcard::match(wildcard, test, ignoreCase) == false : (match = false));
	}

	if (*test == 0 && *wildcard == 0) match = true;

	return match;
}

bool Wildcard::match(const char* wildcard, const char* test, bool ignoreCase)
{
	bool match = true;

	if (strcmp("*", wildcard) == 0)
		return true;

	if (test == NULL)
		test = "";

	for (; match && *wildcard && *test; ++wildcard)
	{
		switch(*wildcard)
		{
		case '?':
			++test;
			break;
		case '*':
			match = wcm_asterisk(wildcard, test, ignoreCase);
			--wildcard;
			break;
		default:
			match = wcm_match(*wildcard, *test, ignoreCase);
			++test;
		}
	}

	while (*wildcard == '*' && match)
		++wildcard;

	return match && *test == 0 && *wildcard == 0;
}

////////////////////////////////////////////////////////////////////////////////

NS_NIT_END;
