#pragma once

#define BEGIN_NAMESPACE(n) namespace n{
#define END_NAMESPACE() }


BEGIN_NAMESPACE(CommFunc)

BOOL WaitUntilMutexRelease(LPCTSTR lpszMutexName, DWORD dwMillisecond = INFINITE);
BOOL IsMutexExist(LPCTSTR lpszMutexName);

END_NAMESPACE()