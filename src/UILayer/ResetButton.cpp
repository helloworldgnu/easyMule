/*
 * $Id: ResetButton.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
// ResetButton.cpp : 实现文件
//

#include "stdafx.h"
#include "emule.h"
#include "ResetButton.h"
#include ".\resetbutton.h"


// CResetButton

IMPLEMENT_DYNAMIC(CResetButton, CStatic)
CResetButton::CResetButton()
{
	m_pImageList = NULL;
	m_nCurrentIcon = 0;
}

CResetButton::~CResetButton()
{
	if (m_pImageList != NULL)
	{
		m_pImageList->DeleteImageList();
	}
	delete m_pImageList;
}


BEGIN_MESSAGE_MAP(CResetButton, CStatic)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
END_MESSAGE_MAP()



// CResetButton 消息处理程序


BOOL CResetButton::OnEraseBkgnd(CDC* /*pDC*/)
{
	//return CStatic::OnEraseBkgnd(pDC);
	return TRUE;
}

void CResetButton::OnPaint()
{
	CPaintDC dc( this );
	CRect rect;
	GetClientRect( &rect );
	dc.FillSolidRect( rect, GetSysColor(COLOR_WINDOW) );
	m_pImageList->Draw(&dc, m_nCurrentIcon, CPoint(2, (rect.bottom - 16) / 2), ILD_NORMAL);
	
}

void CResetButton::ShowIcon(int nIconNumber)
{
	if (nIconNumber == m_nCurrentIcon)
	{
		return;
	}

	m_nCurrentIcon = nIconNumber;
	Invalidate();
	UpdateWindow();
}
