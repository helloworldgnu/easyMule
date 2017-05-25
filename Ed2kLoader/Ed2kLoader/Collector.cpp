#include "stdafx.h"
#include "Ed2kLoader.h"
#include ".\collector.h"
#include "EmuleMgr.h"
#include "CollectorWaitInitThread.h"
#include <strsafe.h>

extern CEmuleMgr	theEmuleMgr;


CCollector*	CCollector::ms_instance = NULL;
CCollector*	CCollector::GetInstance()
{
	if (NULL == ms_instance)
		ms_instance = new CCollector;

	return ms_instance;
}
void CCollector::FreeInstance()
{
	if (NULL != ms_instance)
		delete ms_instance;
}


CCollector::CCollector(void)
{
	m_hHelperWnd = NULL;
}

CCollector::~CCollector(void)
{
	CleanupList();
}

BOOL CCollector::Proc(LPCTSTR lpszEd2kLink)
{
	if (!CreateHelperWnd())
	{
		theEmuleMgr.OpenEmuleWithParam(lpszEd2kLink);
		return FALSE;
	}

	CreateThread(NULL, 0, CCollectorWaitInitThread::ThreadProc, (LPVOID)m_hHelperWnd, 0, NULL);

	AddEd2kToList(lpszEd2kLink);

	theLoaderSingleInst.InitCompleted(&m_hHelperWnd, sizeof(HWND));

	bool bCanSendEd2k;
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (UM_STOP_MESSAGE == msg.message)
		{
			bCanSendEd2k = (0 == msg.wParam) ? false : true;
			break;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	

	theLoaderSingleInst.AppEnd();
	::DestroyWindow(m_hHelperWnd);
	m_hHelperWnd = NULL;
	BOOL result = TRUE;

	if (bCanSendEd2k)
	{
		TCHAR	szEd2kLink[1024];
		while (RemoveEd2kFromList(szEd2kLink, 1024))
		{
			result = result & theEmuleMgr.SendEd2kToEmule(szEd2kLink);
		}
	}
	return result;
}

bool CCollector::CreateHelperWnd(void)
{
	MyRegisterClass();

	m_hHelperWnd = CreateWindow(LOADERHELPERWND_CLASSNAME, NULL, WS_OVERLAPPEDWINDOW,
								CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, GetModuleHandle(NULL), NULL);

	if (!m_hHelperWnd)
	{
		return false;
	}
	return true;
}

ATOM CCollector::MyRegisterClass(void)
{
	WNDCLASSEX wcex;

	ZeroMemory(&wcex, sizeof(wcex));
	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.lpfnWndProc	= (WNDPROC)CCollector::HelperWndProc;
	wcex.hInstance		= GetModuleHandle(NULL);
	wcex.lpszClassName	= LOADERHELPERWND_CLASSNAME;

	return RegisterClassEx(&wcex);
}

LRESULT CALLBACK CCollector::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (UWM_ARE_YOU_LOADER == message)
		return (LRESULT) UWM_ARE_YOU_LOADER;

	switch (message) 
	{
	case WM_COPYDATA:
		{
			COPYDATASTRUCT		*pcds = (COPYDATASTRUCT*) lParam;
			if (UM_COPYDATACODE_ED2KLINK == pcds->dwData)
			{
				LPCTSTR				pLink = (LPCTSTR) pcds->lpData;
				AddEd2kToList(pLink);
			}
			return 1;
		}
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
LRESULT CALLBACK CCollector::HelperWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return CCollector::GetInstance()->WndProc(hWnd, message, wParam, lParam);
}


void CCollector::AddEd2kToList(LPCTSTR lpszEd2kLink)
{
	if (NULL == lpszEd2kLink)
		return;

	UINT	uLen = (UINT) _tcslen(lpszEd2kLink);
	LPTSTR	pLink = new TCHAR[uLen + 1];
	StringCchCopy(pLink, uLen + 1, lpszEd2kLink);

	m_listLinks.push_back(pLink);
}

bool CCollector::RemoveEd2kFromList(LPTSTR lpszEd2kLink, UINT uCch)
{
	if (m_listLinks.size() <= 0)
		return false;

	LPTSTR pLink;
	pLink = m_listLinks.front();
	m_listLinks.pop_front();

	if (NULL != lpszEd2kLink)
		StringCchCopy(lpszEd2kLink, uCch, pLink);

	return true;
}

void CCollector::CleanupList()
{
	LPTSTR pLink;

	while (m_listLinks.size() > 0)
	{
		pLink = m_listLinks.front();
		delete pLink;
		pLink = NULL;
		m_listLinks.pop_front();
	}
}
