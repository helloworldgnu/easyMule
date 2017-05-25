/*
 * $Id: CloseModeDlg.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
// CloseModeDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "emule.h"
#include "CloseModeDlg.h"
#include ".\closemodedlg.h"


// CCloseModeDlg 对话框

bool CCloseModeDlg::m_bAskingClose = false;

IMPLEMENT_DYNAMIC(CCloseModeDlg, CDialog)
CCloseModeDlg::CCloseModeDlg(CWnd* pParent /*=NULL*/) : CDialog(CCloseModeDlg::IDD, pParent)
{
	m_iCloseMode= -1;
}

CCloseModeDlg::~CCloseModeDlg()
{
	m_bAskingClose = false;
}

void CCloseModeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_CLOSE_0, m_iCloseMode);
}


BEGIN_MESSAGE_MAP(CCloseModeDlg, CDialog)
END_MESSAGE_MAP()


// CCloseModeDlg 消息处理程序

BOOL CCloseModeDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_bAskingClose = true;

	SetWindowText(GetResString(IDS_CLOSEMODE_CAPTION));
	GetDlgItem(IDC_STATIC)->SetWindowText(GetResString(IDS_CLOSEMODE_TEXT));
	GetDlgItem(IDC_CLOSE_0)->SetWindowText(GetResString(IDS_MINIMISE));
	GetDlgItem(IDC_CLOSE_1)->SetWindowText(GetResString(IDS_SHUTDOWN));
    GetDlgItem(IDOK)->SetWindowText(GetResString(IDS_OK));
	GetDlgItem(IDCANCEL)->SetWindowText(GetResString(IDS_CANCEL));
	switch(thePrefs.GetCloseMode())
	{
	case 0:
		m_iCloseMode = 0;
		break;
	case 1:
		m_iCloseMode = 1;
		break;
	case 2:
		m_iCloseMode = 0;
		break;
	default:
		assert(0);
	}

	UpdateData( FALSE );
	return TRUE;
}

void CCloseModeDlg::OnOK()
{
	UpdateData();

	switch ( m_iCloseMode )
	{
	case 0:
		thePrefs.SetCloseMode(2);
		break;
	case 1:
		thePrefs.SetCloseMode(1);
		break;
	default:
		assert(0);
	}

	CDialog::OnOK();
}
