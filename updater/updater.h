// updater.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols


// CUpdaterApp:
// See updater.cpp for the implementation of this class
//

class CUpdaterApp : public CWinApp
{
public:
	CUpdaterApp();

protected:
	int m_iResult;

// Overrides
public:
	virtual BOOL InitInstance();

protected:
	void	StartApplication(CString sCommandLine);
	void	CheckForUpdates(void);

// Implementation

	DECLARE_MESSAGE_MAP()
public:
	virtual int ExitInstance();
};

extern CUpdaterApp theApp;