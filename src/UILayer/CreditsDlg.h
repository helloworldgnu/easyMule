/*
 * $Id: CreditsDlg.h 4483 2008-01-02 09:19:06Z soarchin $
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
#include "GDIThread.h"
#include "resource.h"
#include "enbitmap.h"

#include "../CxImage/xImage.h" // Added by thilon on 2006.08.01,”√”⁄º”‘ÿPNGÕº∆¨
#include "SplashScreen.h" // Added by thilon on 2006.08.01

/////////////////////////////////////////////////////////////////////////////
// CCreditsDlg dialog

class CCreditsDlg : public CDialog
{
// Construction
public:
	void KillThread();
	void StartThread();
	CCreditsDlg(CWnd* pParent = NULL);   // standard constructor
	CCreditsDlg::~CCreditsDlg();

	CClientDC*	m_pDC;
	CRect		m_rectScreen;

	CGDIThread* m_pThread;

// Dialog Data
	//{{AFX_DATA(CCreditsDlg)
	enum { IDD = IDD_ABOUTBOX };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCreditsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCreditsDlg)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	virtual BOOL OnInitDialog();
	virtual void OnPaint();
	afx_msg void OnDestroy();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	HBITMAP		m_hbm; 
	 BITMAP		m_bitmap;
	 CRect		rect;

	 CxImage *   m_image; // Added by thilon on 2006.08.01
	 CSplashScreen* m_pSplash;
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};
