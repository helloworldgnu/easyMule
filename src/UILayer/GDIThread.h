/*
 * $Id: GDIThread.h 4483 2008-01-02 09:19:06Z soarchin $
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

// GDIThread.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGDIThread thread

class CGDIThread : public CWinThread
{
public:
	DECLARE_DYNAMIC(CGDIThread)
	CGDIThread(CWnd* pWnd, HDC hDC);

// Attributes
public:
	HDC m_hDC;
	CDC m_dc;
	HANDLE m_hEventKill;
	HANDLE m_hEventDead;
	//static HANDLE m_hAnotherDead;

	// options
	int m_nDelay;
	int m_nScrollInc;
	BOOL m_bWaitVRT;

	enum {SCROLL_UP = 1, SCROLL_PAUSE = 0, SCROLL_DOWN = -1};

	static CCriticalSection m_csGDILock;

// Operations
public:
	void KillThread();
	virtual void SingleStep() = 0;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGDIThread)
	//}}AFX_VIRTUAL

// Implementation
public:
	BOOL SetWaitVRT(BOOL bWait = TRUE);
	int SetScrollDirection(int nDirection);
	int SetDelay(int nDelay);
	virtual ~CGDIThread();
	virtual void Delete();

protected:
	virtual BOOL InitInstance();

	// Generated message map functions
	//{{AFX_MSG(CGDIThread)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};
