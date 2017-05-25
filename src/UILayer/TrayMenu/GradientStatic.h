/*
 * $Id: GradientStatic.h 4483 2008-01-02 09:19:06Z soarchin $
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
// CGradientStatic window

class CGradientStatic : public CStatic
{
// Construction
public:
	CGradientStatic();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGradientStatic)
	//}}AFX_VIRTUAL

// Implementation
public:
	void SetFont(CFont *pFont);
	virtual ~CGradientStatic();

	void SetInit(bool bInit)				{ m_bInit = bInit;		 }
	void SetHorizontal(bool bHorz = true)	{ m_bHorizontal = bHorz; }
	void SetInvert(bool bInvert = false)	{ m_bInvert = bInvert;	 }
	void SetColors(COLORREF crText, COLORREF crLB, COLORREF crRT)
	{
		m_crTextColor = crText;
		m_crColorLB = crLB;
		m_crColorRT = crRT;
	}

	// Generated message map functions
protected:
	bool m_bInit;
	bool m_bHorizontal;
	bool m_bInvert;

	COLORREF m_crColorRT;
	COLORREF m_crColorLB;
	COLORREF m_crTextColor;

	CFont m_cfFont;

	struct _TAG_GRADIENTSTATIC_MEM_
	{
		CDC		dc;
		CBitmap bmp;
		CBitmap *pold;
		int		cx;
		int		cy;
	
	}m_Mem;		

	//{{AFX_MSG(CGradientStatic)
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
private:
	void DrawVerticalText(CRect *pRect);
	void DrawHorizontalText(CRect *pRect);
	void DrawVerticalGradient();
	void DrawHorizontalGradient();
	void CreateGradient(CDC *pDC, CRect *pRect);
};
