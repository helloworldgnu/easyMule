/*
 * $Id: SplitterControlEx.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
#include "StdAfx.h"
#include "SplitterControlex.h"
#include "emule.h"
#include "emuledlg.h"
#include "UserMsgs.h"
#include ".\splittercontrolex.h"

CSplitterControlEx::CSplitterControlEx(void)
{
	m_bHover = false;
	m_bPress = false;
}

CSplitterControlEx::~CSplitterControlEx(void)
{
}
BEGIN_MESSAGE_MAP(CSplitterControlEx, CSplitterControl)
	ON_WM_PAINT()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
//	ON_WM_SETCURSOR()
//	ON_WM_NCHITTEST()
END_MESSAGE_MAP()

void CSplitterControlEx::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect rtClient;
	GetClientRect(rtClient);

	CRect rtButton;

	if (m_nType == SPS_VERTICAL)
	{
		rtButton = rtClient;
		rtButton.top = rtClient.Height()/2 - 25;
		rtButton.bottom = rtClient.Height()/2 + 25;
	}

	if(m_nType == SPS_HORIZONTAL)
	{
		rtButton = rtClient;
		rtButton.left = rtClient.Width()/2 - 25;
		rtButton.right = rtClient.Width()/2 + 25;
	}

	m_rtBtn = rtButton;

	Draw(&dc, rtClient);

	if(m_bHover)
	{
		if(m_bPress)
		{
			dc.FillSolidRect(rtButton, RGB(123, 30, 30));
			DrawTriangle(&dc, rtButton, RGB(155, 85, 85), !m_nflag);
		}
		else
		{
			dc.FillSolidRect(rtButton, RGB(249, 177, 25));	//RGB(90, 13, 13)
			DrawTriangle(&dc, rtButton, RGB(153, 0, 0), !m_nflag);	//RGB(242, 233, 233)
		}
	}
	else
	{
		dc.FillSolidRect(rtButton, RGB(162, 40, 40));
		DrawTriangle(&dc, rtButton, RGB(242, 233, 233), !m_nflag);
	}
}
void CSplitterControlEx::DrawTriangle(CDC* pDC, CRect& rect, COLORREF clr, bool bup)
{
	CPen Pen(PS_SOLID, 1, clr);
	
	CPen* oldPen = (CPen*)pDC->SelectObject(&Pen);

	rect.left += rect.Width()/2 - 3;
	rect.top += 1;

	if(m_nType == SPS_HORIZONTAL)
	{
		if(bup)//箭头向上
		{
			pDC->MoveTo(rect.left + 2, rect.top);
			pDC->LineTo(rect.left + 3, rect.top);

			pDC->MoveTo(rect.left + 1, rect.top + 1);
			pDC->LineTo(rect.left + 4, rect.top + 1);

			pDC->MoveTo(rect.left, rect.top + 2);
			pDC->LineTo(rect.left + 5, rect.top + 2);
		}
		else//箭头向下
		{
			pDC->MoveTo(rect.left, rect.top);
			pDC->LineTo(rect.left + 5, rect.top);

			pDC->MoveTo(rect.left + 1, rect.top + 1);
			pDC->LineTo(rect.left + 4, rect.top + 1);

			pDC->MoveTo(rect.left + 2, rect.top + 2);
			pDC->LineTo(rect.left + 3, rect.top + 2);
		}
	}

	if(m_nType == SPS_VERTICAL)
	{
		if(bup) //箭头向右
		{
			pDC->MoveTo(rect.left + 1, rect.top + rect.Height()/2);
			pDC->LineTo(rect.left + 1, rect.top + rect.Height()/2 - 5);

			pDC->MoveTo(rect.left + 2, rect.top + rect.Height()/2 - 1);
			pDC->LineTo(rect.left + 2, rect.top + rect.Height()/2 - 4);

			pDC->MoveTo(rect.left + 3, rect.top + rect.Height()/2 - 2);
			pDC->LineTo(rect.left + 3, rect.top + rect.Height()/2 - 3);
		}
		else
		{
			pDC->MoveTo(rect.left + 1, rect.top + rect.Height()/2 - 2);
			pDC->LineTo(rect.left + 1, rect.top + rect.Height()/2 - 3);

			pDC->MoveTo(rect.left + 2, rect.top + rect.Height()/2 - 1);
			pDC->LineTo(rect.left + 2, rect.top + rect.Height()/2 - 4);

			pDC->MoveTo(rect.left + 3, rect.top + rect.Height()/2);
			pDC->LineTo(rect.left + 3, rect.top + rect.Height()/2 - 5);

		}
	}
	
	pDC->SelectObject(oldPen);
}

void CSplitterControlEx::OnLButtonUp(UINT nFlags, CPoint point)
{
	if(HitTest(point))
	{
		CWnd *pOwner = GetOwner();
		if (pOwner && IsWindow(pOwner->m_hWnd))
		{
			SPCEX_NMHDR nmsp;
			nmsp.hdr.hwndFrom = m_hWnd;
			nmsp.hdr.idFrom = GetDlgCtrlID();
			nmsp.hdr.code = UM_SPLITTER_CLICKED;
			nmsp.flag = m_nflag;
			pOwner->SendMessage(WM_NOTIFY, nmsp.hdr.idFrom, (LPARAM)&nmsp);

			m_bPress = FALSE;
			Invalidate();
		}
		else
		{
			m_bPress = FALSE;
			Invalidate();
		}
	}
	else
	{
		m_bPress = FALSE;
		Invalidate();
	}

	CSplitterControl::OnLButtonUp(nFlags, point);
}

BOOL CSplitterControlEx::HitTest(const CPoint& point)
{
	if (m_rtBtn.PtInRect(point))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

void CSplitterControlEx::OnMouseMove(UINT nFlags, CPoint point)
{

	TRACKMOUSEEVENT		csTME;

	CSplitterControl::OnMouseMove(nFlags, point);

	if(HitTest(point))
	{
		::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));

		m_bHover = true;
		Invalidate();

		ZeroMemory(&csTME, sizeof(csTME));
		csTME.cbSize = sizeof(csTME);
		csTME.dwFlags = TME_LEAVE;
		csTME.hwndTrack = m_hWnd;
		::_TrackMouseEvent(&csTME);
	}
}

LRESULT CSplitterControlEx::OnMouseLeave(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if (m_bHover)
	{
		m_bHover = FALSE;
		Invalidate();
	}

	return 0;
}

void CSplitterControlEx::OnLButtonDown(UINT nFlags, CPoint point)
{
	if(!m_bPress)
	{
		m_bPress = TRUE;
		Invalidate();
	}
	CSplitterControl::OnLButtonDown(nFlags, point);
}
