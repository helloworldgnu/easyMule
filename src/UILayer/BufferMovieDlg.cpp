/*
 * $Id: BufferMovieDlg.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
// BufferMovieDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "emule.h"
#include "BufferMovieDlg.h"
#include "UIMessage.h"



// CBufferMovieDlg 对话框

IMPLEMENT_DYNAMIC(CBufferMovieDlg, CDialog)
CBufferMovieDlg::CBufferMovieDlg(CWnd* pParent)
	: CDialog(CBufferMovieDlg::IDD, pParent)
{
	m_Parent = pParent;
}

CBufferMovieDlg::~CBufferMovieDlg()
{

}

void CBufferMovieDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CBufferMovieDlg, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_MESSAGE(WM_UPDATE_GUI_FILEPROGRESS, OnUpdateGUIFileProgress)
END_MESSAGE_MAP()


// CBufferMovieDlg 消息处理程序

void CBufferMovieDlg::Localize(void)
{
	//if (m_hWnd)
	//{
	//	GetDlgItem(IDC_STATIC_BUFFERMOVIE)->SetWindowText(GetResString(IDS_STATIC_BUFFERMOVIE));
	//}
}

BOOL CBufferMovieDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_Parent->SendMessage(WM_UPDATE_GUI_START, 0, (LPARAM)this);

	return TRUE;
}


void CBufferMovieDlg::OnBnClickedOk()
{
	m_Parent->SendMessage(WM_UPDATE_GUI_STOP, 0, 0);
	OnOK();
}

void CBufferMovieDlg::OnBnClickedCancel()
{
	m_Parent->SendMessage(WM_UPDATE_GUI_STOP, 0, 0);
	OnCancel();
}

LRESULT CBufferMovieDlg::OnUpdateGUIFileProgress(WPARAM /*wParam*/, LPARAM lParam)
{
	static time_t tLast = 0;
	if(time(NULL) - tLast < 1)
		return S_OK;
	tLast = time(NULL);
	CPartFile* partfile = (CPartFile*)lParam;

	if (partfile)
	{
		CString strInfo;
		float percent = 100.0f - 100.0f * (float)partfile->GetTotalGapSizeInRange(0, 1024 * 1024 - 1) / (float)(1024 * 1024);
		strInfo.Format(GetResString(IDS_PLAY), percent);
		CWnd * wnd = GetDlgItem(IDC_STATIC_BUFFERMOVIE);
		wnd->SetWindowText(strInfo);
		if(percent >= 100.0f)
		{
			m_Parent->SendMessage(WM_UPDATE_GUI_STOP, 0, 0);
			OnOK();
		}
	}

	return S_OK;
}
