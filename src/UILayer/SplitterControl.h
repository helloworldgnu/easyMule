/*
 * $Id: SplitterControl.h 4483 2008-01-02 09:19:06Z soarchin $
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
/**************CSplitterControl interface***********
*	Class name :CSplitterControl
*	Purpose: Implement splitter control for dialog
*			or any other windows.
*	Author: Nguyen Huy Hung, Vietnamese student.
*	Date:	May 29th 2002.
*	Note: You can use it for any purposes. Feel free
*			to change, modify, but please do not 
*			remove this.
*	No warranty of any risks.
*	(:-)
*/
#pragma once

/////////////////////////////////////////////////////////////////////////////
// CSplitterControl window

#define CW_LEFTALIGN	1
#define CW_RIGHTALIGN	2
#define CW_TOPALIGN		3
#define CW_BOTTOMALIGN	4

#define SPS_VERTICAL	1
#define SPS_HORIZONTAL	2

typedef struct SPC_NMHDR
{
	NMHDR hdr;
	int delta;
} SPC_NMHDR;

class CSplitterControl : public CStatic
{
public:
	CSplitterControl();
	virtual	~CSplitterControl();

	void Create(DWORD dwStyle, const CRect& rect, CWnd* pParent, UINT nID);
	int GetStyle();
	int	SetStyle(int nStyle = SPS_VERTICAL);
	void SetRange(int nMin, int nMax);
	void SetRange(int nSubtraction, int nAddition, int nRoot);

	static void ChangePos(CWnd* pWnd, int dx, int dy);
	static void ChangeWidth(CWnd* pWnd, int dx, DWORD dwFlag = CW_LEFTALIGN);
	static void ChangeHeight(CWnd* pWnd, int dy, DWORD dwFlag = CW_TOPALIGN);

	void	SetNormalDraw(BOOL bNormalDraw = TRUE){m_bNormalDraw = bNormalDraw;}
	UINT	GetHBreadth();
	UINT	GetVBreadth();
public:
	int m_nflag;
protected:
	static HCURSOR m_hcurMoveVert;
	static HCURSOR m_hcurMoveHorz;
	BOOL	m_bIsPressed;
	int		m_nType;
	int		m_nX, m_nY;
	int		m_nMin, m_nMax;
	int		m_nSavePos;		// Save point on the lbutton down message

	BOOL	m_bNormalDraw;

	void MoveWindowTo(CPoint pt);
	virtual void DrawLine(CDC* pDC, int x, int y);
	void Draw(CDC *pDC, const CRect &rect);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
};
