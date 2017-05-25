/*
 * $Id: DlgMainTabShare.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
// DlgMainTabShare.cpp : 实现文件
//

#include "stdafx.h"
#include "DlgMainTabShare.h"
#include ".\dlgmaintabshare.h"
#include "TabItem_Normal.h"
#include "TabItem_Wnd.h"
#include "PageTabBkDraw.h"
#include "eMule.h"
#include "emuleDlg.h"
#include "SharedFilesWnd.h"


// CDlgMainTabShare 对话框

IMPLEMENT_DYNAMIC(CDlgMainTabShare, CDialog)
CDlgMainTabShare::CDlgMainTabShare(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgMainTabShare::IDD, pParent)
{
	m_posShare = NULL;
}

CDlgMainTabShare::~CDlgMainTabShare()
{
}

void CDlgMainTabShare::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgMainTabShare, CDialog)
	ON_WM_SIZE()
END_MESSAGE_MAP()

void CDlgMainTabShare::SetShareText(LPCTSTR lpszText)
{
	if (NULL != m_posShare)
		m_tabWnd.SetTabText(m_posShare, lpszText);
}

// CDlgMainTabShare 消息处理程序

BOOL CDlgMainTabShare::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (pMsg->message == WM_KEYDOWN)
	{
		// Don't handle Ctrl+Tab in this window. It will be handled by main window.
		if (pMsg->wParam == VK_TAB && GetAsyncKeyState(VK_CONTROL) < 0)
			return FALSE;

		if (VK_RETURN == pMsg->wParam || VK_ESCAPE == pMsg->wParam)
			return FALSE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}

BOOL CDlgMainTabShare::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	ModifyStyle(0, WS_CLIPCHILDREN, 0);

	m_tabWnd.Create(WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN, CRect(0, 0, 0, 0), this, 0);
	m_tabWnd.SetBarBkDraw(new CPageTabBkDraw);


	//	add Toolbar	<begin>
	m_toolbar.Create(WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, AFX_IDW_TOOLBAR);
	m_toolbar.SetOwner(&theApp.emuledlg->sharedfileswnd->sharedfilesctrl);
	m_toolbar.SetIndent(8);
	m_toolbar.SetBkDraw(new CPageTabBkDraw);

	CTabItem_Wnd	*pTabItemWnd = new CTabItem_Wnd;
	pTabItemWnd->SetItemWnd(&m_toolbar, FALSE);
	pTabItemWnd->SetDynDesireLength(TRUE);
	m_tabWnd.AddTab(pTabItemWnd);
	pTabItemWnd = NULL;
	//	add Toolbar	<end>


	CTabItem_Normal	*pTI_Normal = new CTabItem_Normal;
	pTI_Normal->SetCaption(GetResString(IDS_SF_FILES));
	pTI_Normal->SetRelativeWnd(theApp.emuledlg->sharedfileswnd->GetSafeHwnd());
	//pTI_Normal->SetDesireLength(300);
	pTI_Normal->SetDynDesireLength(TRUE);
	m_posShare = m_tabWnd.AddTab(pTI_Normal, TRUE);
	pTI_Normal = NULL;

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CDlgMainTabShare::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
	if (NULL != m_tabWnd.GetSafeHwnd())
	{
		m_tabWnd.MoveWindow(0, 0, cx, cy, FALSE);
	}
}
