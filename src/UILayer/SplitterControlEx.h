/*
 * $Id: SplitterControlEx.h 4483 2008-01-02 09:19:06Z soarchin $
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
#include "SplitterControl.h"

typedef struct SPCEX_NMHDR
{
	NMHDR hdr;
	int flag;
} SPCEX_NMHDR;

class CSplitterControlEx : public CSplitterControl
{
public:
	CSplitterControlEx(void);
	virtual ~CSplitterControlEx(void);
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();

protected:
	CRect m_rtBtn;
protected:
	void DrawTriangle(CDC* pDC, CRect& rect, COLORREF clr, bool bup = 1);
	BOOL HitTest(const CPoint& point);
public:
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

private:
	LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);

protected:
	BOOL m_bHover;
	BOOL m_bPress;
public:
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};
