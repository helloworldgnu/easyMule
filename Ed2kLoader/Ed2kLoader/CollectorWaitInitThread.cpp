
#include "stdafx.h"
#include "Ed2kLoader.h"
#include ".\collectorwaitinitthread.h"
#include "EmuleMgr.h"

CCollectorWaitInitThread::CCollectorWaitInitThread(void)
{
}

CCollectorWaitInitThread::~CCollectorWaitInitThread(void)
{
}

DWORD WINAPI CCollectorWaitInitThread::ThreadProc(LPVOID lpParameter)
{
	if (0 == lpParameter)
		return 0;

	HWND hHelperWnd = (HWND) lpParameter;
	BOOL	bWaitSucceeded = FALSE;

	BOOL			bTimeout;
	CEmuleMgr		emuleMgr;

	if (! emuleMgr.IsEmuleOpenFlagSet())
	{
		emuleMgr.OpenEmule();

		int	i;
		int iWaitCount = 10;
		bTimeout = TRUE;
		for (i = 0; i < iWaitCount; i++)
		{
			Sleep(1000);

			if (emuleMgr.IsEmuleOpenFlagSet())
			{
				bTimeout = FALSE;
				break;
			}
		}
		if (bTimeout)
		{
			bWaitSucceeded = FALSE;
			goto end;
		}
	}

	if (emuleMgr.WaitUntilMainDlgInited(INFINITE))
		bWaitSucceeded = TRUE;
	else
		bWaitSucceeded = FALSE;


end:
	::PostMessage(hHelperWnd, UM_STOP_MESSAGE, bWaitSucceeded, 0);

	return 0;
}
