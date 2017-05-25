/*
 * $Id: UpdaterThread.cpp 9780 2009-01-07 07:58:37Z dgkang $
 * 
 * this file is part of easyMule
 * Copyright (C)2002-2008 VeryCD Dev Team ( strEmail.Format("%s@%s", "emuledev", "verycd.com") / http: * www.easymule.org )
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
// UpdaterThread.cpp : 实现文件
//

#include "stdafx.h"
#include "emule.h"
#include "EmuleDlg.h"
#include "UpdaterThread.h"
#include "Preferences.h"
#include "UserMsgs.h"
#include "Log.h"
//#include "otherfunctions.h"

// CUpdaterThread

IMPLEMENT_DYNCREATE(CUpdaterThread, CWinThread)

CUpdaterThread::CUpdaterThread()
{
	m_pParent = NULL;
}

CUpdaterThread::~CUpdaterThread()
{
}

BOOL CUpdaterThread::InitInstance()
{
	ULONG lResult = 0;

	PROCESS_INFORMATION piProcInfo; 
	STARTUPINFO siStartInfo = {sizeof(STARTUPINFO)};

	CString strFilename = thePrefs.GetMuleDirectory(EMULE_EXECUTEABLEDIR);

	CFileFind ff;

	strFilename.Append(_T("updater.exe"));

	if(!ff.FindFile(strFilename))
	{
		PostThreadMessage(WM_QUIT, 0, 0);
	}

	TCHAR sz[] = _T("updater.exe -checkforupdates");
	if(!CreateProcess(strFilename, sz, NULL, NULL, FALSE, NULL, NULL, NULL, &siStartInfo, &piProcInfo))
	{
		PostThreadMessage(WM_QUIT, 0, 0);
	}
	
	WaitForSingleObject(piProcInfo.hProcess, INFINITE);

	if(GetExitCodeProcess(piProcInfo.hProcess, &lResult))
	{
		switch(lResult)
		{
		case RESULT_NEWVERSION:
			AddLogLine(TRUE, _T("Start Emule VeryCD P2P Update."));
			PostMessageToParent(UM_STARTED2KUPDATE, 0, 0);
			break;
		case ERROR_NONEWVERSION:
			AddLogLine(TRUE,GetResString(IDS_LATEST_VERSION));
			PostMessageToParent(UM_EASYMULECHECKUPDATEFINISHED, 1, ERROR_NONEWVERSION);
			break;
		case ERROR_NOCONNECTION:
			AddLogLine(false, GetResString(IDS_NOCONNECT));
			break;
		case ERROR_SERVER:
			AddLogLine(false, GetResString(IDS_NOCONNECT_SERVER));
			break;
		case ERROR_CHECKFAIL:
			AddLogLine(false, GetResString(IDS_CHECKOUT_FAILED));
			break;
		case ERROR_MEMNOTCREATE:
			AddLogLine(false, GetResString(IDS_CREATEMEMORY_FAILED));
			break;
		case ERROR_MEMNOTOPEN:
			AddLogLine(false, GetResString(IDS_OPENMEMORY_FAILED));
			break;
		case ERROR_MEMNOTMAP:
			AddLogLine(false, GetResString(IDS_MAP_FAILED));
			break;
		case ERROR_WRITEMEM:
			AddLogLine(false, GetResString(IDS_READIN_FAILED));
			break;
		case ERROR_LOADFAIL:
			AddLogLine(false,GetResString(IDS_LOAD_FAILED));
			break;
		default:
			AddLogLine(false, GetResString(IDS_UNKNOWN_ERROR));
		}
	}

	return FALSE;
}

int CUpdaterThread::ExitInstance()
{
	if(theApp.emuledlg)
		theApp.emuledlg->SetUpdaterCreated( false );
	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(CUpdaterThread, CWinThread)
END_MESSAGE_MAP()


// CUpdaterThread 消息处理程序

void CUpdaterThread::SetParent(CDialog* pParentThread)
{
	m_pParent = pParentThread;
}

void CUpdaterThread::PostMessageToParent(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (!IsBadReadPtr(m_pParent, sizeof(CDialog*)))
	{
		m_pParent->PostMessage(message, wParam, lParam);
	}
}
