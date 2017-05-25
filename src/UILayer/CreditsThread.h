/*
 * $Id: CreditsThread.h 4483 2008-01-02 09:19:06Z soarchin $
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

class CCreditsThread : public CGDIThread
{
public:
	DECLARE_DYNAMIC(CCreditsThread)
	CCreditsThread(CWnd* pWnd, HDC hDC, CRect rectScreen);

// Attributes
public:
	CRect		m_rectScreen;
	CRgn		m_rgnScreen;

	int			m_nScrollPos;

	// background bitmap
	CDC			m_dcBk;
	CBitmap		m_bmpBk;
	CBitmap*	m_pbmpOldBk;

	// credits bitmap
	CDC			m_dcCredits;
	CBitmap		m_bmpCredits;
	CBitmap*	m_pbmpOldCredits;

	// screen bitmap
	CDC			m_dcScreen;
	CBitmap		m_bmpScreen;
	CBitmap*	m_pbmpOldScreen;

	// mask bitmap
	CDC			m_dcMask;
	CBitmap		m_bmpMask;
	CBitmap*	m_pbmpOldMask;

	int			m_nCreditsBmpWidth;
	int			m_nCreditsBmpHeight;

	CArray<CString>				m_arCredits;
	CArray<COLORREF, COLORREF>	m_arColors;
	CArray<CFont*, CFont*>		m_arFonts;
	CArray<int, int>			m_arFontHeights;

// Operations
public:
	int CalcCreditsHeight();
	void InitText();
	void InitColors();
	void InitFonts();
	void CreateCredits();
	virtual BOOL InitInstance();
	virtual void SingleStep();
	void PaintBk(CDC* pDC);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCreditsThread)
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CCreditsThread();

	// Generated message map functions
	//{{AFX_MSG(CCreditsThread)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};
