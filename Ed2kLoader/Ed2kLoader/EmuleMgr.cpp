
#include "stdafx.h"
#include <shellapi.h>
#include ".\emulemgr.h"
#include "CommFunc.h"
#include <strsafe.h>

#include "..\..\src\WorkLayer\opcodes.h"
#define	DEFAULT_TCP_PORT	4662
#define OP_ED2KLINK			12000
#define OP_QUERYSTATUS		12003
const static UINT UWM_ARE_YOU_EMULE = RegisterWindowMessage(EMULE_GUID);

CEmuleMgr::CEmuleMgr(void)
{
	m_uTcpPort = 0;
	m_szInstMutexName[0] = _T('\0');
	m_szMapFileMutexName[0] = _T('\0');
}

CEmuleMgr::~CEmuleMgr(void)
{
}

BOOL CEmuleMgr::IsMainDlgInited(DWORD dwWaitTime)
{
	HANDLE	hMutex;
	BOOL	bInited;
	DWORD	dwResult;

	bInited = FALSE;
	hMutex = ::OpenMutex(SYNCHRONIZE, FALSE, GetInstMutexName());
	
	if (NULL != hMutex)
	{
		dwResult = WaitForSingleObject(hMutex, dwWaitTime);

		if (WAIT_OBJECT_0 == dwResult || WAIT_ABANDONED == dwResult)
		{
			ReleaseMutex(hMutex);
			bInited = TRUE;
		}

		::CloseHandle(hMutex);
	}
	
	return bInited;
}

BOOL CEmuleMgr::IsEmuleOpenFlagSet()
{
	return CommFunc::IsMutexExist(GetInstMutexName());
}

BOOL CEmuleMgr::WaitUntilMainDlgInited(DWORD dwMillisecond)
{
	return CommFunc::WaitUntilMutexRelease(GetInstMutexName(), dwMillisecond);
}

HWND CEmuleMgr::GetEmuleMainDlg()
{
	HWND	hEmuleMainDlg;
	HANDLE	hMapFile;
	LPVOID	pBuf;

	hMapFile = OpenFileMapping(FILE_MAP_READ, FALSE, GetMapFileMutexName());
	if (NULL == hMapFile || INVALID_HANDLE_VALUE == hMapFile)
		return NULL;

	pBuf = MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, sizeof(HWND));
	CopyMemory(&hEmuleMainDlg, pBuf, sizeof(HWND));
	UnmapViewOfFile(pBuf);

	CloseHandle(hMapFile);

	return hEmuleMainDlg;
}

BOOL CEmuleMgr::SendEd2kToEmule(LPCTSTR lpszEd2kLink)
{
	if (NULL == lpszEd2kLink)
		return FALSE;

	HWND	hEmuleWnd = NULL;
	hEmuleWnd = GetEmuleMainDlg();

	COPYDATASTRUCT		cds;
	if (NULL != hEmuleWnd)
	{
		ZeroMemory(&cds, sizeof(cds));
		cds.dwData = OP_ED2KLINK; 
		cds.cbData = (DWORD) ((_tcslen(lpszEd2kLink) + 1) * sizeof(TCHAR));
		cds.lpData = (PVOID) lpszEd2kLink;
		if(!(BOOL) ::SendMessage(hEmuleWnd, WM_COPYDATA, (WPARAM)0, (LPARAM)&cds))
			return FALSE;
		cds.dwData = OP_QUERYSTATUS;
		cds.cbData = 0;
		cds.lpData = NULL;
		ULONG res;
		if(_tcsnicmp(lpszEd2kLink, _T("ed2k://"), 7) == 0)
		{
			::PostMessage(hEmuleWnd, WM_COPYDATA, (WPARAM)0, (LPARAM)&cds);
		}
		else
		{
			while(((res = (int)::SendMessage(hEmuleWnd, WM_COPYDATA, (WPARAM)0, (LPARAM)&cds)) & 2) > 0)
				Sleep(1000);
			return (res & 1) > 0;
		}
		return TRUE;
	}
	return FALSE;
}

void CEmuleMgr::OpenEmuleWithParam(LPCTSTR lpszEd2kLink)
{
	if (NULL == lpszEd2kLink)
		return;

	TCHAR	szExeFilePath[MAX_PATH];
	GetAppDir(szExeFilePath, MAX_PATH);
	StringCchCat(szExeFilePath, MAX_PATH, _T("\\eMule.exe"));

	ShellExecute(NULL, _T("open"), szExeFilePath, lpszEd2kLink, NULL, SW_SHOW);
}

void CEmuleMgr::OpenEmule()
{
	TCHAR	szExeFilePath[MAX_PATH];
	GetAppDir(szExeFilePath, MAX_PATH);
	StringCchCat(szExeFilePath, MAX_PATH, _T("\\eMule.exe"));

	ShellExecute(NULL, _T("open"), szExeFilePath, _T("-AutoStart"), NULL, SW_SHOW);
}

void CEmuleMgr::GetAppDir(LPTSTR lpszPath, UINT uCch)
{
	GetModuleFileName(NULL, lpszPath, uCch * sizeof(TCHAR) );
	TCHAR *pc = _tcsrchr(lpszPath, _T('\\'));
	if (NULL != pc)
		*pc = _T('\0');
}

void CEmuleMgr::GetPrefFilePath(LPTSTR lpszFilePath, UINT uCch)
{
	GetAppDir(lpszFilePath, uCch);
	StringCchCat(lpszFilePath, uCch, _T("\\"));
	StringCchCat(lpszFilePath, uCch, CONFIGFOLDER);
	StringCchCat(lpszFilePath, uCch, _T("preferences.ini"));
}


UINT CEmuleMgr::GetTcpPort()
{
	if (0 != m_uTcpPort)
		return m_uTcpPort;

	TCHAR szPrefFilePath[MAX_PATH];
	GetPrefFilePath(szPrefFilePath, MAX_PATH);
	m_uTcpPort = GetPrivateProfileInt(_T("eMule"), _T("Port"), DEFAULT_TCP_PORT, szPrefFilePath);

	return m_uTcpPort;
}

LPCTSTR	CEmuleMgr::GetInstMutexName()
{
	if (_T('\0') != m_szInstMutexName[0])
		return m_szInstMutexName;

	//MODIFIED by VC-fengwen on 2007/10/11 <begin> : 使用路径为字符串标识。以解决vista找配置文件麻烦。
		//UINT	uTcpPort = GetTcpPort();
		//StringCchPrintf(m_szInstMutexName, MAX_PATH, _T("%s:%u"), EMULE_GUID, uTcpPort);
	GetAppDir(m_szInstMutexName, MAX_PATH);
	_tcslwr(m_szInstMutexName);
	for (int i = 0; i < MAX_PATH; i++)
	{
		if (0 == m_szInstMutexName[i])
			break;
		else if (_T('\\') == m_szInstMutexName[i])
			m_szInstMutexName[i] = _T('-');
	}
	//MODIFIED by VC-fengwen on 2007/10/11 <end> : 使用路径为字符串标识。以解决vista找配置文件麻烦。
	return m_szInstMutexName;
}
LPCTSTR	CEmuleMgr::GetMapFileMutexName()
{
	if (_T('\0') != m_szMapFileMutexName[0])
		return m_szMapFileMutexName;

	//MODIFIED by VC-fengwen on 2007/10/11 <begin> : 使用路径为字符串标识。以解决vista找配置文件麻烦。
		//UINT	uTcpPort = GetTcpPort();
		//StringCchPrintf(m_szMapFileMutexName, MAX_PATH, _T("%s:%u_MapFile"), EMULE_GUID, uTcpPort);
	StringCchCopy(m_szMapFileMutexName, MAX_PATH, GetInstMutexName());
	StringCchCat(m_szMapFileMutexName, MAX_PATH, _T("_MapFile"));
	//MODIFIED by VC-fengwen on 2007/10/11 <end> : 使用路径为字符串标识。以解决vista找配置文件麻烦。

	return m_szMapFileMutexName;
}
