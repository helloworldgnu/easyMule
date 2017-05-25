#pragma once

#include <xstring>
class CSingleInst
{
public:
	CSingleInst(LPCTSTR lpszGuid);
	~CSingleInst(void);

	BOOL	AppStart();	//	return IsFirstInstance
	void	AppEnd();

	void	InitCompleted(LPVOID pData = NULL, UINT uSize = 0);
	void	OnUninit();

	BOOL	WaitForInitCompleted(LPVOID pData = NULL, UINT uSize = 0, DWORD dwMillisecond = INFINITE);

protected:
	std::basic_string<TCHAR>	m_strGuid;
	std::basic_string<TCHAR>	m_strGuidMapFile;
	
	HANDLE	m_hInstMutex;
	HANDLE	m_hMapFile;
};
