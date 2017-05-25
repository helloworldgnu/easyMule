/*
 * $Id: ColourPopup.h 4483 2008-01-02 09:19:06Z soarchin $
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

// ColourPopup.h : header file
//
// Written by Chris Maunder (chrismaunder@codeguru.com)
// Extended by Alexander Bischofberger (bischofb@informatik.tu-muenchen.de)
// Copyright (c) 1998.
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed unmodified by any means PROVIDING it is
// not sold for profit without the authors written consent, and
// providing that this notice and the authors name is included. If
// the source code in  this file is used in any commercial application
// then a simple email would be nice.
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability if it causes any damage whatsoever.
// It's free - so you get what you pay for.


// forward declaration
class CColourPicker;

// To hold the colours and their names
typedef struct
{
	COLORREF crColour;
	TCHAR	 *szName;
}
ColourTableEntry;


/////////////////////////////////////////////////////////////////////////////
// CColourPopup window

class CColourPopup : public CWnd
{
// Construction
public:
	CColourPopup();
	CColourPopup(CPoint p, COLORREF crColour, CWnd* pParentWnd,
		         LPCTSTR szDefaultText = NULL, LPCTSTR szCustomText = NULL,
		         COLORREF* colourArray = NULL,int NumberOfColours = 0);
	void Initialise();

// Attributes
public:

// Operations
public:
	BOOL Create(CPoint p, COLORREF crColour, CWnd* pParentWnd, LPCTSTR szDefaultText = NULL,
	            LPCTSTR szCustomText = NULL);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CColourPopup)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CColourPopup();

protected:
		COLORREF* colourArrayPassed;
	BOOL GetCellRect(int nIndex, const LPRECT& rect);
	void FindCellFromColour(COLORREF crColour);
	void SetWindowSize();
	void CreateToolTips();
	void ChangeSelection(int nIndex);
	void EndSelection(int nMessage);
	void DrawCell(CDC* pDC, int nIndex);

	COLORREF GetColour(int nIndex)
	{
		if(colourArrayPassed==NULL)
			return m_crColours[nIndex].crColour;
		else
			return colourArrayPassed[nIndex];
	}
	LPCTSTR GetColourName(int nIndex) { return m_crColours[nIndex].szName; }
	int	 GetIndex(int row, int col) const;
	int	 GetRow(int nIndex) const;
	int	 GetColumn(int nIndex) const;
	
// protected attributes
protected:
	static ColourTableEntry m_crColours[];
	int			   m_nNumColours;
	int			   m_nNumColumns, m_nNumRows;
	int			   m_nBoxSize, m_nMargin;
	int			   m_nCurrentSel;
	int			   m_nChosenColourSel;
	CString		   m_strDefaultText;
	CString		   m_strCustomText;
	CRect		   m_CustomTextRect, m_DefaultTextRect, m_WindowRect;
	CFont		   m_Font;
	CPalette	   m_Palette;
	COLORREF	   m_crInitialColour, m_crColour;
	CToolTipCtrl   m_ToolTip;
	CWnd*		   m_pParent;
	BOOL		   m_bChildWindowVisible;

	// Generated message map functions
protected:
	//{{AFX_MSG(CColourPopup)
	afx_msg void OnNcDestroy();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnQueryNewPalette();
	afx_msg void OnPaletteChanged(CWnd* pFocusWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnActivateApp(BOOL bActive, DWORD hTask);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

