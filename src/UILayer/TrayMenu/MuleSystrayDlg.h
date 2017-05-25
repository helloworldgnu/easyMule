/*
 * $Id: MuleSystrayDlg.h 4483 2008-01-02 09:19:06Z soarchin $
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

class CInputBox : public CEdit
{
protected:
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);

    DECLARE_MESSAGE_MAP()
};

#include "TrayMenuBtn.h"		// Added by ClassView
#include "GradientStatic.h"	// Added by ClassView
#include "resource.h"


/////////////////////////////////////////////////////////////////////////////
// CMuleSystrayDlg dialog

class CMuleSystrayDlg : public CDialog
{
// Construction
public:
	CMuleSystrayDlg(CWnd* pParent, CPoint pt, int iMaxUp, int iMaxDown, int iCurUp, int iCurDown);
	~CMuleSystrayDlg();
    
// Dialog Data
	//{{AFX_DATA(CMuleSystrayDlg)
	enum { IDD = IDD_MULETRAYDLG };
	CStatic	m_ctrlUpArrow;
	CStatic	m_ctrlDownArrow;
	CGradientStatic	m_ctrlSidebar;
	CSliderCtrl	m_ctrlUpSpeedSld;
	CSliderCtrl	m_ctrlDownSpeedSld;
	CInputBox	m_DownSpeedInput;
	CInputBox	m_UpSpeedInput;
	int		m_nDownSpeedTxt;
	int		m_nUpSpeedTxt;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMuleSystrayDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	CTrayMenuBtn m_ctrlSpeed;
	CTrayMenuBtn m_ctrlAllToMax;
	CTrayMenuBtn m_ctrlAllToMin;
	CTrayMenuBtn m_ctrlRestore;
	CTrayMenuBtn m_ctrlDisconnect;
	CTrayMenuBtn m_ctrlConnect;
	CTrayMenuBtn m_ctrlExit;
	CTrayMenuBtn m_ctrlPreferences;
	CTrayMenuBtn m_ctrlUpdate;

	bool m_bClosingDown;
	
	int m_iMaxUp;
	int m_iMaxDown;
	CPoint m_ptInitialPosition;

	HICON m_hUpArrow;
	HICON m_hDownArrow;

	UINT m_nExitCode;

	// Generated message map functions
	//{{AFX_MSG(CMuleSystrayDlg)
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeDowntxt();
	afx_msg void OnChangeUptxt();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnCaptureChanged(CWnd *pWnd);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
