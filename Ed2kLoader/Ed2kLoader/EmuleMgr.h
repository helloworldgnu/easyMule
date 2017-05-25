#pragma once

class CEmuleMgr
{
public:
	CEmuleMgr(void);
	~CEmuleMgr(void);

	BOOL	IsMainDlgInited(DWORD dwWaitTime = 0);
	BOOL	IsEmuleOpenFlagSet();
	BOOL	WaitUntilMainDlgInited(DWORD dwMillisecond = INFINITE);

	HWND	GetEmuleMainDlg();
	
	BOOL	SendEd2kToEmule(LPCTSTR lpszEd2kLink);
	static BOOL CALLBACK SearchEmuleWindow(HWND hWnd, LPARAM lParam);

	void	OpenEmuleWithParam(LPCTSTR lpszEd2kLink);
	void	OpenEmule();

	void	GetAppDir(LPTSTR lpszPath, UINT uCch);
	void	GetPrefFilePath(LPTSTR lpszFilePath, UINT uCch);
	UINT	GetTcpPort();
	LPCTSTR	GetInstMutexName();
	LPCTSTR	GetMapFileMutexName();

protected:
	UINT	m_uTcpPort;
	TCHAR	m_szInstMutexName[MAX_PATH];
	TCHAR	m_szMapFileMutexName[MAX_PATH];
};
