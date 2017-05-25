/*
 * $Id: AffirmDeleteDlg.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
// AffirmDeleteDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "emule.h"
#include "AffirmDeleteDlg.h"
#include ".\affirmdeletedlg.h"


// CAffirmDeleteDlg 对话框

IMPLEMENT_DYNAMIC(CAffirmDeleteDlg, CDialog)
CAffirmDeleteDlg::CAffirmDeleteDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAffirmDeleteDlg::IDD, pParent)
{
}

CAffirmDeleteDlg::~CAffirmDeleteDlg()
{
}

void CAffirmDeleteDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK1, IsChecked);
}


BEGIN_MESSAGE_MAP(CAffirmDeleteDlg, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CAffirmDeleteDlg 消息处理程序
BOOL CAffirmDeleteDlg::OnInitDialog()
{  
   CDialog::OnInitDialog();
   this->SetWindowText(GetResString(IDS_DELETE_FILE));
   GetDlgItem(IDC_STATIC)->SetWindowText(GetResString(IDS_DELETE_TASK));
   GetDlgItem(IDC_CHECK1)->SetWindowText(GetResString(IDS_DELETE_DISKFILE));
   GetDlgItem(IDOK)->SetWindowText(GetResString(IDS_OK));
   GetDlgItem(IDCANCEL)->SetWindowText(GetResString(IDS_CANCEL));
   HICON hicon = ::LoadIcon(NULL, IDI_QUESTION);
   if(!hicon)
	   return false;
   CStatic *pStatic = (CStatic *)GetDlgItem(IDC_IMAGE);
   if(pStatic)
       pStatic->SetIcon(hicon);
   return TRUE;
}
void CAffirmDeleteDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	if(IsChecked.GetCheck() == 1)
		bIsDeleteFile = true;
	else
		bIsDeleteFile = false;
	OnOK();
}
