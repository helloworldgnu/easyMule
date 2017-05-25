/*
 * $Id: UpLoading.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
// UpLoading.cpp : 实现文件
//

#include "stdafx.h"
#include "emule.h"
#include "UpLoading.h"


// CUpLoading 对话框

IMPLEMENT_DYNAMIC(CUpLoading, CDialog)
CUpLoading::CUpLoading(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CUpLoading::IDD, pParent)
{
}

CUpLoading::~CUpLoading()
{
}

void CUpLoading::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_UPLOADING, uploadlistctrl);
}


BEGIN_MESSAGE_MAP(CUpLoading, CResizableDialog)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CUpLoading 消息处理程序
BOOL CUpLoading::OnInitDialog()
{
	CResizableDialog::OnInitDialog();
	uploadlistctrl.Init();

	AddAnchor(uploadlistctrl.m_hWnd,TOP_LEFT, BOTTOM_RIGHT);

	return TRUE;
}
void CUpLoading::OnSize(UINT nType, int cx, int cy)
{
	CResizableDialog::OnSize(nType, cx, cy);
	Invalidate(FALSE);
}
