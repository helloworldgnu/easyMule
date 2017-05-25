/*
 * $Id: ThreadsMgr.h 4483 2008-01-02 09:19:06Z soarchin $
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
#pragma once

#include <AfxTempl.h>
#include <AfxMt.h>

class CThreadsMgr
{
public:
	CThreadsMgr(void);
	~CThreadsMgr(void);

	typedef void (*CLEAN_PROC) (HANDLE hThread);
	static void	CleanProc_WaitAndDelWinThd(HANDLE hThread);
	static void	CleanProc_DelWinThd(HANDLE hThread);

	static CWinThread* BegingThreadAndRecDown(CLEAN_PROC cleanProc, AFX_THREADPROC pfnThreadProc, LPVOID pParam,
												int nPriority = THREAD_PRIORITY_NORMAL, UINT nStackSize = 0,
												DWORD dwCreateFlags = 0, LPSECURITY_ATTRIBUTES lpSecurityAttrs = NULL);
	static CWinThread* BegingThreadAndRecDown(CLEAN_PROC cleanProc, CRuntimeClass* pThreadClass,
												int nPriority = THREAD_PRIORITY_NORMAL, UINT nStackSize = 0,
												DWORD dwCreateFlags = 0, LPSECURITY_ATTRIBUTES lpSecurityAttrs = NULL);


	void	RegThread(DWORD dwThreadId, CLEAN_PROC pfnCleanProc);
	void	UnregThread(DWORD dwThreadId);

	void	CleanAllThreads();
protected:
	CMap<DWORD, DWORD, CLEAN_PROC, const CLEAN_PROC&>	m_mapRecs;
	CCriticalSection m_cs;
};

extern CThreadsMgr	theThreadsMgr;

class CUnregThreadAssist
{
public:
	CUnregThreadAssist(DWORD dwThreadId){m_dwThreadId = dwThreadId;}
	~CUnregThreadAssist(){theThreadsMgr.UnregThread(m_dwThreadId);}
protected:
	DWORD	m_dwThreadId;
};
