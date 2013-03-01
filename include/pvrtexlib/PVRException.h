/******************************************************************************

 @File         PVRException.h

 @Title        

 @Version      

 @Copyright    Copyright (c) Imagination Technologies Limited. All Rights Reserved. Strictly Confidential.

 @Platform     ANSI

 @Description  Exception class and macros.

******************************************************************************/
#ifndef _PVREXCEPTION_H_
#define _PVREXCEPTION_H_

#ifndef PVR_DLL
#ifdef _WINDLL_EXPORT
#define PVR_DLL __declspec(dllexport)
#elif _WINDLL_IMPORT
#define PVR_DLL __declspec(dllimport)
#else
#define PVR_DLL
#endif
#endif

/*****************************************************************************
* Exception class and macros
* Use char* literals only for m_what.
*****************************************************************************/
class PVR_DLL PVRException
{
public:
	PVRException(const char* const what)throw();
	const char * const what() const;
	~PVRException() throw();
private:
	const char* const m_what;
	const PVRException& operator =(const PVRException&);	// don't copy exceptions
};

#define PVRTRY			try
#define PVRTHROW(A)		{PVRException myException(A); throw(myException);}
#define PVRCATCH(A)		catch(PVRException& A)
#define PVRCATCHALL		catch(...)

#ifdef PVR_ENABLE_LOG
#define PVRLOG			{ConsoleLog::log}			
#define PVRLOGTHROW(A)		{ConsoleLog::log(A); PVRException myException(A); throw(myException);}
#else
#define PVRLOG						
#define PVRLOGTHROW(A)		{PVRException myException(A); throw(myException);}
#endif

#endif // _PVREXCEPTION_H_

/*****************************************************************************
*  end of file
******************************************************************************/

