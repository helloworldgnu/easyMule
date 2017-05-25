#include "StdAfx.h"
#include "printwriter.h"
#include <io.h>
#include <share.h>

#define ARRSIZE(x)	(sizeof(x)/sizeof(x[0]))

CPrintWriter::CPrintWriter(LPCTSTR pszFilePath)
{
	if (pszFilePath != NULL)
	{
		m_pLogFile = _tfsopen(pszFilePath, _T("a+b"), _SH_DENYWR);
	}
	else
	{
		ASSERT(0);
	}
}

CPrintWriter::~CPrintWriter(void)
{
	Close();
}

BOOL CPrintWriter::Log(LPCTSTR pszMsg, int iLen)
{
	if (m_pLogFile == NULL)
	{
		return FALSE;
	}

	size_t uWritten;
	size_t uToWrite = ((iLen == -1) ? _tcslen(pszMsg) : (size_t)iLen)*sizeof(TCHAR);
	uWritten = fwrite(pszMsg, 1, uToWrite, m_pLogFile);

	BOOL bResult = !ferror(m_pLogFile);
	fflush(m_pLogFile);

	return bResult;
}

void CPrintWriter::AddLogText(LPCTSTR pszLine, va_list argptr)
{
	ASSERT(pszLine != NULL);
	TCHAR szLogLine[1000];

	if (_vsntprintf(szLogLine, ARRSIZE(szLogLine), pszLine, argptr) == -1)
	{
		szLogLine[ARRSIZE(szLogLine) - 1] = _T('\0');
	}
	else
	{
		TRACE(_T("App Log: %s\n"), szLogLine);

		TCHAR szFullLogLine[1060];
		int iLen = _sntprintf(szFullLogLine, ARRSIZE(szFullLogLine), _T("%s: %s\r\n"), CTime::GetCurrentTime().Format(L"%c"), szLogLine);

		if (iLen >= 0)
		{
			Log(szFullLogLine, iLen);
		}
	}
}

void CPrintWriter::println(LPCTSTR pszLine, ...)
{
	ASSERT(pszLine != NULL);

	va_list argptr;
	va_start(argptr, pszLine);
	AddLogText(pszLine, argptr);
	va_end(argptr);
}

void CPrintWriter::Close(void)
{
	if (m_pLogFile != NULL)
	{
		fclose(m_pLogFile);
		m_pLogFile = NULL;
	}
}