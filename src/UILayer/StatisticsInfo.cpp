/*
 * $Id: StatisticsInfo.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
// StatisticsInfo.cpp : 实现文件
//

#include "stdafx.h"
#include "emule.h"
#include "StatisticsInfo.h"
#include ".\statisticsinfo.h"

// CStatisticsInfo 对话框

IMPLEMENT_DYNAMIC(CStatisticsInfo, CDialog)
CStatisticsInfo::CStatisticsInfo(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CStatisticsInfo::IDD, pParent)
{
}

CStatisticsInfo::~CStatisticsInfo()
{
}

void CStatisticsInfo::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_POPBAR, pop_bar);
	DDX_Control(pDX, IDC_POPBAR2, pop_baraccept);
	DDX_Control(pDX, IDC_POPBAR3, pop_bartrans);
	DDX_Control(pDX, IDC_STATISTICS, m_ctrlStatisticsFrm);
}


BEGIN_MESSAGE_MAP(CStatisticsInfo, CResizableDialog)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CStatisticsInfo 消息处理程序
BOOL CStatisticsInfo::OnInitDialog()
{
	CResizableDialog::OnInitDialog();

	pop_bar.SetGradientColors(RGB(255,255,240),RGB(255,255,0));
	pop_bar.SetTextColor(RGB(20,70,255));
	pop_baraccept.SetGradientColors(RGB(255,255,240),RGB(255,255,0));
	pop_baraccept.SetTextColor(RGB(20,70,255));
	pop_bartrans.SetGradientColors(RGB(255,255,240),RGB(255,255,0));
	pop_bartrans.SetTextColor(RGB(20,70,255));


	AddAnchor(GetDlgItem(IDC_STATISTICS)->m_hWnd,TOP_LEFT, BOTTOM_RIGHT);
	Localize();
	return TRUE;
}

void CStatisticsInfo::SetRequest(int range, int pos)
{
	pop_bar.SetRange32(0, range);
	pop_bar.SetPos(pos);
	pop_bar.SetShowPercent();
	SetDlgItemInt(IDC_SREQUESTED, pos, FALSE);	
}

void CStatisticsInfo::SetAcceptUpload(int range, int pos)
{
	pop_baraccept.SetRange32(0, range);
	pop_baraccept.SetPos(pos);
	pop_baraccept.SetShowPercent();
	SetDlgItemInt(IDC_SACCEPTED, pos, FALSE);
}

void CStatisticsInfo::SetTransfer(int range, uint64 pos)
{
	pop_bartrans.SetRange32(0, range/1024);
	pop_bartrans.SetPos((int)pos/1024);
	pop_bartrans.SetShowPercent();
	SetDlgItemText(IDC_STRANSFERRED, CastItoXBytes(pos, false, false));
}

void CStatisticsInfo::SetAll(int request,  int accept, uint64 transfer)
{
	SetDlgItemText(IDC_STRANSFERRED2, CastItoXBytes(transfer, false, false));
	SetDlgItemInt(IDC_SREQUESTED2, request, FALSE);
	SetDlgItemInt(IDC_SACCEPTED2, accept, FALSE);
}

void CStatisticsInfo::SetNoFile()
{
	pop_bartrans.SetRange32(0, 100);
	pop_bartrans.SetPos(0);
	pop_bartrans.SetTextFormat(_T(""));
	SetDlgItemText(IDC_STRANSFERRED, _T("-"));

	pop_bar.SetRange32(0, 100);
	pop_bar.SetPos(0);
	pop_bar.SetTextFormat(_T(""));
	SetDlgItemText(IDC_SREQUESTED, _T("-"));

	pop_baraccept.SetRange32(0, 100);
	pop_baraccept.SetPos(0);
	pop_baraccept.SetTextFormat(_T(""));
	SetDlgItemText(IDC_SACCEPTED, _T("-"));

	SetDlgItemText(IDC_STRANSFERRED2, _T("-"));
	SetDlgItemText(IDC_SREQUESTED2, _T("-"));
	SetDlgItemText(IDC_SACCEPTED2, _T("-"));
}

void CStatisticsInfo::SetStatisticsFrmText(CString str)
{
	m_ctrlStatisticsFrm.SetWindowText(str);
}
void CStatisticsInfo::OnSize(UINT nType, int cx, int cy)
{
	CResizableDialog::OnSize(nType, cx, cy);
	Invalidate(FALSE);
}
void CStatisticsInfo::Localize()
{
	GetDlgItem(IDC_CURSESSION_LBL)->SetWindowText(GetResString(IDS_SF_CURRENT));
	GetDlgItem(IDC_TOTAL_LBL)->SetWindowText(GetResString(IDS_SF_TOTAL));
	GetDlgItem(IDC_FSTATIC6)->SetWindowText(GetResString(IDS_SF_TRANS));
	GetDlgItem(IDC_FSTATIC5)->SetWindowText(GetResString(IDS_SF_ACCEPTED));
	GetDlgItem(IDC_FSTATIC4)->SetWindowText(GetResString(IDS_SF_REQUESTS)+_T(":"));
	GetDlgItem(IDC_FSTATIC9)->SetWindowText(GetResString(IDS_SF_TRANS));
	GetDlgItem(IDC_FSTATIC8)->SetWindowText(GetResString(IDS_SF_ACCEPTED));
	GetDlgItem(IDC_FSTATIC7)->SetWindowText(GetResString(IDS_SF_REQUESTS)+_T(":"));

	m_ctrlStatisticsFrm.SetWindowText(GetResString(IDS_SF_STATISTICS));
}
