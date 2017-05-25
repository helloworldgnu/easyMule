#include "stdafx.h"
#include "Ed2kLoader.h"
#include "EmuleMgr.h"
#include "Collector.h"
#include "NonCollector.h"
#include <strsafe.h>

const UINT UWM_ARE_YOU_LOADER = RegisterWindowMessage(LOADER_GUID);

CEmuleMgr	theEmuleMgr;
CSingleInst	theLoaderSingleInst(LOADER_GUID);

TCHAR* ParseCmdLine(LPTSTR lpCmdLine);

int APIENTRY _tWinMain(HINSTANCE hInstance,
					   HINSTANCE hPrevInstance,
					   LPTSTR    lpCmdLine,
					   int       nCmdShow)
{

	//::MessageBox(NULL, _T("a"), NULL, MB_OK);
	//return 0;
	TCHAR*	pEd2kLink = ParseCmdLine(lpCmdLine);
	if (NULL == pEd2kLink || _T('\0') == pEd2kLink[0])
		return 0;
	

	if (theEmuleMgr.IsMainDlgInited(100))
	{
		return theEmuleMgr.SendEd2kToEmule(pEd2kLink);
	}
	else
	{
		if (theLoaderSingleInst.AppStart())
		{
			BOOL result = CCollector::GetInstance()->Proc(pEd2kLink);
			CCollector::FreeInstance();
			return result;
		}
		else
		{
			CNonCollector	procer;
			return procer.Proc(pEd2kLink);
		}
	}
	return 0;
}

TCHAR* ParseCmdLine(LPTSTR lpCmdLine)
{
	int	iLen = (int) _tcslen(lpCmdLine);

	if (0 == iLen)
		return NULL;


	lpCmdLine[iLen - 1] = _T('\0');		//去掉两边的引号。
	return lpCmdLine + 1;
}

