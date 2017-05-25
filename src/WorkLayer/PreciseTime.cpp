#include "StdAfx.h"
#include ".\precisetime.h"
#include <MMSystem.h>

class CPreciseTimeBase
{
public:
	CPreciseTimeBase(void)
	{
		m_dwTickBase = timeGetTime();
		GetSystemTimeAsFileTime((LPFILETIME)&m_n64TimeBase);
	}
	~CPreciseTimeBase(void)
	{

	}
	
	DWORD		m_dwTickBase;
	LONGLONG	m_n64TimeBase;
} g_preciseTimeBase;

CPreciseTime::CPreciseTime(void)
{
}

CPreciseTime::~CPreciseTime(void)
{
}

CString CPreciseTime::NowStr()
{
	DWORD		dwTick = timeGetTime();
	LONGLONG	n64Time = g_preciseTimeBase.m_n64TimeBase + (dwTick - g_preciseTimeBase.m_dwTickBase) * 10000;
	SYSTEMTIME	systime;
	FileTimeToSystemTime((LPFILETIME)&n64Time, &systime);

	CString	str;
	str.Format(_T("%02d:%02d:%02d:%03d"), systime.wHour, systime.wMinute, systime.wSecond, systime.wMilliseconds);
	return str;
}
