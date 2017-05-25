/*
 * $Id: TabItem.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
// TabItem.cpp : 实现文件
//

#include "stdafx.h"
#include "TabItem.h"
#include ".\tabitem.h"
#include "Util.h"
#include "TabBar.h"
#include "TabWnd.h"
#include "TabItemAffector.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CTabItem

IMPLEMENT_DYNCREATE(CTabItem, CObject)
CTabItem::CTabItem()
{
	m_bActive				= FALSE;
	m_hRelativeWnd			= NULL;
	m_hRelaWndOldParent		= NULL;
	m_pParentBar			= NULL;
	m_myPos					= NULL;
	m_bAutoDelRelaWndObject	= FALSE;
	m_pDelRelaWnd			= NULL;

	m_iItemGap				= 4;

	m_bHover				= FALSE;

	m_dwCustomData			= 0;
	m_pAffector				= NULL;
}

CTabItem::~CTabItem()
{
}

void CTabItem::Create(CTabBar *pParentBar, POSITION myPos)
{
	m_pParentBar = pParentBar;
	m_myPos = myPos;
	OnCreate();
	m_pParentBar->OnItemCreate(this);
}

void CTabItem::Destroy()
{
	m_pParentBar->OnItemDestroy(this);

	if (m_bAutoDelRelaWndObject)
	{
		if (!IsBadWritePtr(m_pDelRelaWnd, sizeof(CWnd)))
		{
			m_pDelRelaWnd->DestroyWindow();
			SAFE_DELETE(m_pDelRelaWnd);
		}
	}
	else
	{
		if (NULL != m_hRelaWndOldParent
			&& IsWindow(m_hRelativeWnd))
			::SetParent(m_hRelativeWnd, m_hRelaWndOldParent);
	}

	if (NULL != m_pAffector)
	{
		m_pAffector->m_pAssocItem = NULL;
		m_pAffector = NULL;
	}
}

void CTabItem::SetAffector(CTabItemAffector *pAffector)
{
	m_pAffector = pAffector;
}

void CTabItem::SetRelativeWnd(HWND hWnd, HWND hOldParent)
{
	if (NULL != m_hRelaWndOldParent
		&& IsWindow(m_hRelativeWnd))
		::SetParent(m_hRelativeWnd, m_hRelaWndOldParent);

	m_hRelaWndOldParent = hOldParent;
	m_hRelativeWnd = hWnd;

	if ( IsWindow(m_hRelativeWnd) )
	{
		::SetParent(m_hRelativeWnd, m_pParentBar->m_pParentTabWnd->GetSafeHwnd());
	}
}

CRect CTabItem::GetRectInScreen(void)
{
	CRect rect(GetRect()); 
	m_pParentBar->ClientToScreen(&rect); 
	return rect;
}

void CTabItem::SetActive(BOOL bActive)
{
	m_bActive = bActive;

	return;
}

int CTabItem::GetBarPos(void)
{
	return m_pParentBar->GetBarPos();
}

COLORREF CTabItem::GetBkColor()
{
	return m_pParentBar->GetBkColor();
}

void CTabItem::Invalidate()
{
	m_pParentBar->InvalidateItem(m_myPos);
}
void CTabItem::ShowRelativeWnd(BOOL bShow)
{
	if (IsWindow(m_hRelativeWnd))
	{
		if (bShow)
			ShowWindow(m_hRelativeWnd, SW_SHOW);
		else
			ShowWindow(m_hRelativeWnd, SW_HIDE);
	}
}

void CTabItem::RequestResize()
{
	if (NULL != m_pParentBar) 
		m_pParentBar->SendMessage(WM_SIZE);
}
