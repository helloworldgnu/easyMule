
#include "stdafx.h"
#include "Ed2kLoader.h"
#include ".\noncollector.h"
#include "EmuleMgr.h"
#include "CommFunc.h"

extern CEmuleMgr	theEmuleMgr;

CNonCollector::CNonCollector(void)
{
}

CNonCollector::~CNonCollector(void)
{
}

BOOL CNonCollector::Proc(LPCTSTR lpszEd2kLink)
{
	if (SendEd2kToCollector(lpszEd2kLink))
		return TRUE;

	if (theEmuleMgr.IsMainDlgInited(5000))
		return theEmuleMgr.SendEd2kToEmule(lpszEd2kLink);
	else
		theEmuleMgr.OpenEmuleWithParam(lpszEd2kLink);
	return TRUE;
}

BOOL CNonCollector::SendEd2kToCollector(LPCTSTR lpszEd2kLink)
{
	HWND	hLoaderWnd = NULL;
	if (! theLoaderSingleInst.WaitForInitCompleted(&hLoaderWnd, sizeof(HWND)))
		return FALSE;

	COPYDATASTRUCT		cds;
	ZeroMemory(&cds, sizeof(cds));
	cds.dwData = UM_COPYDATACODE_ED2KLINK; 
	cds.cbData = (DWORD) ((_tcslen(lpszEd2kLink) + 1) * sizeof(TCHAR));
	cds.lpData = (PVOID) lpszEd2kLink;
	if(!(BOOL) ::SendMessage(hLoaderWnd, WM_COPYDATA, (WPARAM)0, (LPARAM)&cds))
		return FALSE;
#define OP_QUERYSTATUS		12003
	cds.dwData = OP_QUERYSTATUS;
	cds.cbData = 0;
	cds.lpData = NULL;
	ULONG res;
	if(_tcsnicmp(lpszEd2kLink, _T("ed2k://"), 7) == 0)
	{
		::PostMessage(hLoaderWnd, WM_COPYDATA, (WPARAM)0, (LPARAM)&cds);
	}
	else
	{
		while(((res = (int)::SendMessage(hLoaderWnd, WM_COPYDATA, (WPARAM)0, (LPARAM)&cds)) & 2) > 0)
			Sleep(1000);
		return (res & 1) > 0;
	}
	return FALSE;
}
