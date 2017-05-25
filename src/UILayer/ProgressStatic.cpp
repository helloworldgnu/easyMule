/*
 * $Id: ProgressStatic.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
// ProgressStatic.cpp : 实现文件
//

#include "stdafx.h"
#include "emule.h"
#include "ProgressStatic.h"
#include ".\progressstatic.h"
#include "Util.h"

// CProgressStatic

IMPLEMENT_DYNAMIC(CProgressStatic, CStatic)
CProgressStatic::CProgressStatic()
{
	m_lProgressPercent = 100;
}

CProgressStatic::~CProgressStatic()
{
}


BEGIN_MESSAGE_MAP(CProgressStatic, CStatic)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

BOOL CProgressStatic::CreatePS(CWnd *pParentWnd)
{
	return CStatic::Create(NULL, WS_CHILD | SS_NOTIFY, CRect(0, 0, 0, 0), pParentWnd);
}

void CProgressStatic::UpdatePos()
{
	CWnd	*pParent = GetParent();
	if (NULL == pParent)
		return;

	CRect	rect;
	pParent->GetClientRect(&rect);
	rect.right -= ::GetSystemMetrics(SM_CXVSCROLL);
	rect.left = rect.right - 170;
	rect.bottom = rect.top + 20;
	MoveWindow(&rect, FALSE);
}

void CProgressStatic::UpdateProgress(long lProgressPercent)
{
	long lOld = m_lProgressPercent;
	m_lProgressPercent = lProgressPercent;

	if (m_lProgressPercent >= 100)
		ShowWindow(SW_HIDE);
	else
	{
		if (m_lProgressPercent < lOld)
			ShowWindow(SW_SHOW);


		if (m_lProgressPercent != lOld)
			Invalidate();
	}
}

void CProgressStatic::UpdateProgress(long lProgress, long lProgressMax)
{
	long lProgressPercent = 0;
	if (0 == lProgressMax || lProgress >= lProgressMax || -1 == lProgress)
		lProgressPercent = 100;
	else
		lProgressPercent = lProgress * 100 / lProgressMax;
	UpdateProgress(lProgressPercent);
}

// CProgressStatic 消息处理程序


void CProgressStatic::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 在此处添加消息处理程序代码
	// 不为绘图消息调用 CStatic::OnPaint()

	CClientRect	rtClient(this);
	CBufferDC	bufDC(dc.GetSafeHdc(), rtClient);

	bufDC.FillSolidRect(&rtClient, RGB(200, 0, 0));

	CFontDC	fontDC(bufDC.GetSafeHdc(), _T("宋体"));
	fontDC = 12;

	CString	strText;
	strText.Format(_T("%s [%d%%]"), GetResString(IDS_PAGE_LOADING), m_lProgressPercent);
	bufDC.SetTextColor(RGB(255, 255, 255));
	bufDC.DrawText(strText, rtClient, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
}

void CProgressStatic::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	ShowWindow(SW_HIDE);

	CStatic::OnLButtonDown(nFlags, point);
}
