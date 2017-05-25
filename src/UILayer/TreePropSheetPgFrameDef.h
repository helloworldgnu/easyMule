/*
 * $Id: TreePropSheetPgFrameDef.h 4483 2008-01-02 09:19:06Z soarchin $
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
/********************************************************************
*
* Copyright (c) 2002 Sven Wiegand <mail@sven-wiegand.de>
*
* You can use this and modify this in any way you want,
* BUT LEAVE THIS HEADER INTACT.
*
* Redistribution is appreciated.
*
* $Workfile:$
* $Revision:$
* $Modtime:$
* $Author:$
*
* Revision History:
*	$History:$
*
*********************************************************************/
#pragma once
#include "TreePropSheetPgFrame.h"

/**
An implementation of CPropPageFrame, that works well for Windows XP
styled systems and older windows versions (without themes).

@author Sven Wiegand
*/
class /*AFX_EXT_CLASS*/ CPropPageFrameDefault : public CWnd,
                                            public CPropPageFrame
{
// construction/destruction
public:
	CPropPageFrameDefault();
	virtual ~CPropPageFrameDefault();

// operations
public:

// overridings
public:
	virtual BOOL Create(DWORD dwWindowStyle, const RECT &rect, CWnd *pwndParent, UINT nID);
	virtual CWnd* GetWnd();
	virtual void SetCaption(LPCTSTR lpszCaption, HICON hIcon = NULL);
	
protected:
	virtual CRect CalcMsgArea();
	virtual CRect CalcCaptionArea();
	virtual void DrawCaption(CDC *pDc, CRect rect, LPCTSTR lpszCaption, HICON hIcon);

// Implementation helpers
protected:
	/**
	Fills a rectangular area with a gradient color starting at the left
	side with the color clrLeft and ending at the right sight with the
	color clrRight.

	@param pDc
		Device context to draw the rectangle in.
	@param rect
		Rectangular area to fill.
	@param clrLeft
		Color on the left side.
	@param clrRight
		Color on the right side.
	*/
	void FillGradientRectH(CDC *pDc, const RECT &rect, COLORREF clrLeft, COLORREF clrRight);

	/**
	Returns TRUE if Windows XP theme support is available, FALSE 
	otherwise.
	*/
	BOOL ThemeSupport() const;

protected:
	//{{AFX_VIRTUAL(CPropPageFrameDefault)
	//}}AFX_VIRTUAL

// message handlers
protected:
	//{{AFX_MSG(CPropPageFrameDefault)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// attributes
protected:
	/** 
	Image list that contains only the current icon or nothing if there
	is no icon.
	*/
	CImageList m_Images;
};
