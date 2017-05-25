/*
 * $Id: DownloadTabCtrl.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
#include ".\downloadtabctrl.h"
#include "MenuCmds.h"
#include "resource.h"

#include "emule.h"
#include "emuleDlg.h"
#include "TransferWnd.h"
#include "TabItem_Wnd.h"

CDownloadTabWnd::CDownloadTabWnd(void)
{
}

CDownloadTabWnd::~CDownloadTabWnd(void)
{
}
BEGIN_MESSAGE_MAP(CDownloadTabWnd, CTabWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

int CDownloadTabWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CTabWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	ModifyStyle(0, WS_CLIPCHILDREN, 0);

	InitToolBar();

	return 0;
}

void CDownloadTabWnd::InitToolBar(void)
{
	m_Toolbar.Create(WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, AFX_IDW_TOOLBAR);
	m_Toolbar.SetOwner(&(theApp.emuledlg->transferwnd->downloadlistctrl));
	m_Toolbar.SetIndent(8);

	CTabItem_Wnd	*pTabItemWnd = new CTabItem_Wnd;
	pTabItemWnd->SetItemWnd(&m_Toolbar, FALSE);

	pTabItemWnd->SetDynDesireLength(TRUE);
	AddTab(pTabItemWnd);
	pTabItemWnd = NULL;
}


void CDownloadTabWnd::OnSize(UINT nType, int cx, int cy)
{
	CTabWnd::OnSize(nType, cx, cy);

	CRect rcWnd;
	GetWindowRect(&rcWnd);
	Invalidate(FALSE);
}
