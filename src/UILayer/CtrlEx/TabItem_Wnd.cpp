/*
 * $Id: TabItem_Wnd.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
#include ".\tabitem_wnd.h"
#include "Util.h"
#include "UserMsgs.h"
CTabItem_Wnd::CTabItem_Wnd(void)
{
	m_pWnd			= NULL;
	m_bAutoDelete	= FALSE;
	m_iWndLength	= 100;
}

CTabItem_Wnd::~CTabItem_Wnd(void)
{
	if (m_bAutoDelete)
		SAFE_DELETE(m_pWnd);
}

IMPLEMENT_DYNCREATE(CTabItem_Wnd, CTabItem)

void CTabItem_Wnd::SetItemWnd(CWnd *pWnd, BOOL bAutoDelete)
{
	SetAttribute(ATTR_FIXLEN);

	if (m_bAutoDelete && NULL != m_pWnd)
	{
		SAFE_DELETE(m_pWnd);
		m_bAutoDelete = FALSE;
	}

	m_pWnd = pWnd;
	m_bAutoDelete = bAutoDelete;
}

int	CTabItem_Wnd::GetDesireLength(void)
{
	if (IsDynDesireLength())
	{
		if (NULL == m_pWnd)
			return 0;
		else
			return m_pWnd->SendMessage(UM_GET_DESIRE_LENGTH);
	}
	else
		return m_iWndLength;

}

void CTabItem_Wnd::Create(CTabBar *pParentBar, POSITION myPos)
{
	CTabItem::Create(pParentBar, myPos);
	m_pWnd->SetParent((CWnd*)pParentBar);
}

void CTabItem_Wnd::Paint(CDC* /*pDC*/)
{
	if (NULL != m_pWnd)
		m_pWnd->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
}

void CTabItem_Wnd::OnSize()
{
	CRect	rect = GetRect();
	if (NULL != m_pWnd)
		m_pWnd->MoveWindow(&rect, FALSE);
}
