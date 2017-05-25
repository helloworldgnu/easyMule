/*
 * $Id: SearchButton.h 4483 2008-01-02 09:19:06Z soarchin $
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


// CSearchButton

class CSearchButton : public CButton
{

public:
	CSearchButton();
	virtual ~CSearchButton();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);

	//{{AFX_VIRTUAL(CButtonST)
	//virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_VIRTUAL
protected:
	virtual void PreSubclassWindow();
	void DrawInactive(CDC* pDC, const CRect& rect);
	void DrawHover(CDC* pDC,const CRect& rect);
	void DrawPressed(CDC* pDC, const CRect& rect);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
protected:
	HICON	m_hIcon;
	BOOL	m_bHover;
	BOOL	m_bIsPressed;
public:
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);

public:
	BOOL Create(CWnd* pParentWnd, CRect rect, LPCTSTR lpszCaption = NULL, DWORD dwStyle = WS_CHILD | WS_VISIBLE, UINT nID = 123);

	void GetDesireSize(CSize &size);
private:
	LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnBnClicked();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
//	afx_msg void OnPaint();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
