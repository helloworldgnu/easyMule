/*
 * $Id: TrayDialog.h 4483 2008-01-02 09:19:06Z soarchin $
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
#include "DialogMinTrayBtn.h"
#include "ResizableLib\ResizableDialog.h"

#define	IDT_SINGLE_CLICK	100

class CTrayDialog : public CDialogMinTrayBtn<CResizableDialog>
{
protected:
	typedef CDialogMinTrayBtn<CResizableDialog> CTrayDialogBase;

public:
	CTrayDialog(UINT uIDD, CWnd* pParent = NULL);   // standard constructor

	void TraySetMinimizeToTray(bool* pbMinimizeToTray);
	BOOL TraySetMenu(UINT nResourceID);
	BOOL TraySetMenu(HMENU hMenu);
	BOOL TraySetMenu(LPCTSTR lpszMenuName);
	BOOL TrayUpdate();
	BOOL TrayShow();
	BOOL TrayHide();
	void TraySetToolTip(LPCTSTR lpszToolTip);
	void TraySetIcon(HICON hIcon, bool bDelete = false);
	void TraySetIcon(UINT nResourceID);
	void TraySetIcon(LPCTSTR lpszResourceName);
	BOOL TrayIsVisible();

	virtual void TrayMinimizeToTrayChange();
	virtual void RestoreWindow();
	virtual void OnTrayLButtonDown(CPoint pt);
	virtual void OnTrayLButtonUp(CPoint pt);
	virtual void OnTrayLButtonDblClk(CPoint pt);
	virtual void OnTrayRButtonUp(CPoint pt);
	virtual void OnTrayRButtonDblClk(CPoint pt);
	virtual void OnTrayMouseMove(CPoint pt);

protected:
	bool* m_pbMinimizeToTray;
    bool m_bCurIconDelete;
    HICON m_hPrevIconDelete;
	bool m_bLButtonDblClk;
	bool m_bLButtonDown;
	bool m_bLButtonUp;
	BOOL m_bTrayIconVisible;
	NOTIFYICONDATA m_nidIconData;
	CMenu m_mnuTrayMenu;
	UINT m_nDefaultMenuItem;
	UINT m_uSingleClickTimer;

	void KillSingleClickTimer();

	DECLARE_MESSAGE_MAP()	
	afx_msg LRESULT OnTrayNotify(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnTaskBarCreated(WPARAM wParam, LPARAM lParam);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);	
	afx_msg void OnTimer(UINT nIDEvent);
};
