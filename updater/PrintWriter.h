#pragma once

class CPrintWriter
{
public:
	CPrintWriter(LPCTSTR pszFilePath = NULL);
	~CPrintWriter(void);
public:
	void println(LPCTSTR pszLine, ...);
	void Close(void);
private:
	void AddLogText(LPCTSTR pszLine, va_list argptr);
protected:
	BOOL Log(LPCTSTR pszMsg, int iLen = -1);
protected:
	FILE*	m_pLogFile;
};
