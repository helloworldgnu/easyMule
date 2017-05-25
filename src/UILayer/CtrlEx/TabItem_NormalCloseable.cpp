/*
 * $Id: TabItem_NormalCloseable.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
// TabItem_NormalCloseable.cpp : 实现文件
//

#include "stdafx.h"
#include "TabItem_NormalCloseable.h"
#include "TabBar.h"


// CTabItem_NormalCloseable
IMPLEMENT_DYNCREATE(CTabItem_NormalCloseable, CTabItem_Normal)

CTabItem_NormalCloseable::CTabItem_NormalCloseable()
{
	m_bEnableClose = TRUE;
	m_bHover = FALSE;
	m_isCloseBtnOver = FALSE;
	m_rtCloseBn.SetRect(0, 0, 0, 0);
	//SetAttribute(ATTR_FIXLEN | ATTR_TAIL);
}

CTabItem_NormalCloseable::~CTabItem_NormalCloseable()
{
}


// CTabItem_NormalCloseable 成员函数
void CTabItem_NormalCloseable::OnSize(void)
{
	m_rtCloseBn = GetRect();
	m_rtCloseBn.right -= m_iItemGap;

	//左上角坐标
	m_rtCloseBn.left = m_rtCloseBn.right - 10 - CLOSEBN_SIZE;
	m_rtCloseBn.top += 10 + 4;
	
	//右下角坐标
	m_rtCloseBn.right -= 10;
	m_rtCloseBn.bottom = m_rtCloseBn.top + CLOSEBN_SIZE;
	
}

void CTabItem_NormalCloseable::Paint(CDC* pDC)
{
	CTabItem_Normal::Paint(pDC);

	if (m_bEnableClose)
	{
		if(m_bActive)
		{
			if (m_bHover && m_isCloseBtnOver)
			{
				CPenDC	pen(pDC->GetSafeHdc(), RGB(204, 0, 0), 2);

				pDC->MoveTo(m_rtCloseBn.TopLeft());
				pDC->LineTo(m_rtCloseBn.BottomRight());

				pDC->MoveTo(m_rtCloseBn.right, m_rtCloseBn.top);
				pDC->LineTo(m_rtCloseBn.left, m_rtCloseBn.bottom);
			}
			else
			{
				CPenDC	pen(pDC->GetSafeHdc(), RGB(0, 0, 0), 2);

				pDC->MoveTo(m_rtCloseBn.TopLeft());
				pDC->LineTo(m_rtCloseBn.BottomRight());

				pDC->MoveTo(m_rtCloseBn.right, m_rtCloseBn.top);
				pDC->LineTo(m_rtCloseBn.left, m_rtCloseBn.bottom);
			}
		}
	}
}

void CTabItem_NormalCloseable::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_bEnableClose)
	{
		if (m_rtCloseBn.PtInRect(point) && m_bActive)
		{		
			m_pParentBar->RemoveTab(m_myPos);
		}
		else
		{
			CTabItem_Normal::OnLButtonDown(nFlags, point);
		}
	}
	else
	{
		CTabItem_Normal::OnLButtonDown(nFlags, point);
	}
}

void CTabItem_NormalCloseable::OnLButtonDblClk(UINT /*nFlags*/, CPoint /*point*/)
{
	if (m_bEnableClose)
		m_pParentBar->RemoveTab(m_myPos);
}

void CTabItem_NormalCloseable::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bEnableClose)
	{
		int iOldOver = m_isCloseBtnOver;
		CRect	rtCloseBnHover = m_rtCloseBn;
		rtCloseBnHover.InflateRect(2, 2);
		if(rtCloseBnHover.PtInRect(point))
		{
			m_isCloseBtnOver = TRUE;
		}
		else
		{
			m_isCloseBtnOver = FALSE;
		}

		if (iOldOver != m_isCloseBtnOver)
		{
			Invalidate();
		}
	}
}

BOOL CTabItem_NormalCloseable::GetTextRect(CRect &rect)
{
	if (!CTabItem_Normal::GetTextRect(rect))
		return FALSE;

	if (m_bEnableClose)
		rect.right = m_rtCloseBn.left - 2;

	return TRUE;
}

int CTabItem_NormalCloseable::GetDesireLength()
{
	return CTabItem_Normal::GetDesireLength() + CLOSEBN_SIZE + 8;
}
