/*
 * $Id: TrayMenuBtn.h 4483 2008-01-02 09:19:06Z soarchin $
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

/////////////////////////////////////////////////////////////////////////////
// CTrayMenuBtn window

class CTrayMenuBtn : public CWnd
{
// Construction
public:
	CTrayMenuBtn();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTrayMenuBtn)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTrayMenuBtn();

	// Generated message map functions
public:
	bool	m_bBold;
	bool	m_bMouseOver;
	bool	m_bNoHover;
	bool	m_bUseIcon;
	bool	m_bParentCapture;
	UINT	m_nBtnID;
	CSize	m_sIcon;
	HICON	m_hIcon;
	CString m_strText;
	CFont	m_cfFont;
	
	//{{AFX_MSG(CTrayMenuBtn)
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};
