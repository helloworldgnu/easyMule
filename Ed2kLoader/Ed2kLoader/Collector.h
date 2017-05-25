#pragma once

#include <list>
using namespace std;

class CCollector
{
public:
	static CCollector*	GetInstance();
	static void			FreeInstance();
protected:
	static CCollector*	ms_instance;



protected:
	CCollector(void);
	~CCollector(void);
public:
	BOOL	Proc(LPCTSTR lpszEd2kLink);

protected:
	bool	CreateHelperWnd(void);
	ATOM MyRegisterClass(void);
	LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK HelperWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	HWND	m_hHelperWnd;


	list<LPTSTR>		m_listLinks;
	void	AddEd2kToList(LPCTSTR lpszEd2kLink);
	bool	RemoveEd2kFromList(LPTSTR lpszEd2kLink, UINT uCch);
	void	CleanupList();
};
