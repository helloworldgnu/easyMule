#include "stdafx.h"
#include ".\commfunc.h"

BEGIN_NAMESPACE(CommFunc)

BOOL WaitUntilMutexRelease(LPCTSTR lpszMutexName, DWORD dwMillisecond)
{
	BOOL	bRet;
	HANDLE	hMutex;
	DWORD	dwResult;
	
	bRet = TRUE;
	hMutex = ::OpenMutex(SYNCHRONIZE, FALSE, lpszMutexName);
	if (NULL != hMutex)
	{
		dwResult = WaitForSingleObject(hMutex, dwMillisecond);

		if (WAIT_OBJECT_0 == dwResult || WAIT_ABANDONED == dwResult)
			ReleaseMutex(hMutex);
		else
			bRet = FALSE;
	}
	::CloseHandle(hMutex);

	return bRet;
}

BOOL IsMutexExist(LPCTSTR lpszMutexName)
{
	BOOL bExist;
	HANDLE hMutex;

	hMutex = ::OpenMutex(SYNCHRONIZE, FALSE, lpszMutexName);
	bExist = (NULL == hMutex) ? FALSE : TRUE;
	::CloseHandle(hMutex);

	return bExist;
}

END_NAMESPACE()