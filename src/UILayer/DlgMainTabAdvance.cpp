/*
 * $Id: DlgMainTabAdvance.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
// DlgMainTabAdvance.cpp : 实现文件
//

#include "stdafx.h"
#include "DlgMainTabAdvance.h"
#include ".\dlgmaintabadvance.h"
// CDlgMainTabAdvance 对话框

IMPLEMENT_DYNAMIC(CDlgMainTabAdvance, CDialog)
CDlgMainTabAdvance::CDlgMainTabAdvance(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgMainTabAdvance::IDD, pParent)
{
}

CDlgMainTabAdvance::~CDlgMainTabAdvance()
{
}

void CDlgMainTabAdvance::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgMainTabAdvance, CDialog)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


// CDlgMainTabAdvance 消息处理程序

int CDlgMainTabAdvance::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	ModifyStyle(0, WS_CLIPCHILDREN);

	// TODO:  在此添加您专用的创建代码
	m_tbwAdvance.Create(WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this);

	return 0;
}

void CDlgMainTabAdvance::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
	m_tbwAdvance.MoveWindow(0, 0, cx, cy, FALSE);
}

BOOL CDlgMainTabAdvance::OnEraseBkgnd(CDC* /*pDC*/)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	return TRUE;
	//return CDialog::OnEraseBkgnd(pDC);
}

void CDlgMainTabAdvance::OnCancel()
{
	// TODO: 在此添加专用代码和/或调用基类

	//CDialog::OnCancel();
}

void CDlgMainTabAdvance::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类

	//CDialog::OnOK();
}
