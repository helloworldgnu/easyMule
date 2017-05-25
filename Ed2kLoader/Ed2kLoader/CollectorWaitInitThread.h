#pragma once

class CCollectorWaitInitThread
{
public:
	CCollectorWaitInitThread(void);
	~CCollectorWaitInitThread(void);

	static DWORD WINAPI ThreadProc(LPVOID lpParameter);
};
