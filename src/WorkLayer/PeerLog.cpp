/*
 * $Id: PeerLog.cpp 5051 2008-03-20 02:43:47Z huby $
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
// PeerLog.cpp : 实现文件
//

#include "stdafx.h"
#include "emule.h"
#include "PeerLog.h"
#include "MenuCmds.h"

#define IDC_LOGLIST 12345

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
// CPeerLog 对话框

IMPLEMENT_DYNAMIC(CPeerLog, CDialog)
CPeerLog::CPeerLog(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CPeerLog::IDD, pParent)
{
	m_pMenuXP = NULL;
	//m_pClient = NULL;
	//m_pPartFile = NULL;
}

CPeerLog::~CPeerLog()
{
	if(m_pMenuXP)
	{
		delete m_pMenuXP;
	}

	//m_pClient =NULL;
	//m_pPartFile = NULL;
}

void CPeerLog::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPeerLog, CResizableDialog)
	ON_COMMAND(MP_COPYSELECTED, OnCopySelected)
	ON_COMMAND(MP_CUTSELECTED, OnCutSelected)
	ON_COMMAND(MP_SELECTALL, OnSelectAll)
	ON_COMMAND(MP_SAVEAS, OnSaveAs)
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_WM_DRAWITEM()
	ON_WM_MEASUREITEM()
//	ON_WM_TIMER()
END_MESSAGE_MAP()


// CPeerLog 消息处理程序

BOOL CPeerLog::OnInitDialog()
{
	CResizableDialog::OnInitDialog();

	CRect rect;
	GetClientRect(rect);
	m_LogListCtrl.Create(/*LVS_OWNERDRAWFIXED |*/ WS_VISIBLE | LVS_REPORT, rect, this, IDC_LOGLIST);
	m_LogListCtrl.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	m_LogListCtrl.InsertColumn(0, GetResString(IDS_PEER_TIME), LVCFMT_LEFT, 200);
	m_LogListCtrl.InsertColumn(1, GetResString(IDS_PEER_INFO), LVCFMT_LEFT, 700);
	m_LogListCtrl.Init();

	AddAnchor(m_LogListCtrl,TOP_LEFT,BOTTOM_RIGHT);

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CPeerLog::OnSize(UINT nType, int cx, int cy)
{
	CResizableDialog::OnSize(nType, cx, cy);

	if (m_LogListCtrl.GetSafeHwnd())
	{
		CRect rect;
		GetClientRect(rect);
		m_LogListCtrl.SetColumnWidth(1, rect.right - 200);
	}
}

void CPeerLog::OnCopySelected()
{
	POSITION pos = m_LogListCtrl.GetFirstSelectedItemPosition();

	CString strText = _T("");

	if (pos == NULL)
	{
		return;
	}
	else
	{	
		while (pos)
		{
			int nItem = m_LogListCtrl.GetNextSelectedItem(pos);

			strText += m_LogListCtrl.GetItemText(nItem, 0);
			strText += m_LogListCtrl.GetItemText(nItem, 1);

			strText += '\n';
		}
	}

	if (strText.GetAt(strText.GetLength() - 1) == '\n')
	{
		strText.Delete(strText.GetLength() - 1);
	}

	theApp.CopyTextToClipboard(strText);
}

void CPeerLog::OnSelectAll()
{
	for (int i = 0; i < m_LogListCtrl.GetItemCount(); i++)
	{
		m_LogListCtrl.SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
	}
}

void CPeerLog::OnSaveAs()
{
	CFileDialog SaveDlg(FALSE, _T("log"), NULL, OFN_FILEMUSTEXIST| OFN_HIDEREADONLY, _T("Log File (*.log)"), this);

	if (SaveDlg.DoModal() == IDOK)
	{
		try
		{
			CStdioFile file(SaveDlg.GetFileName(), CFile::modeCreate | CFile::modeWrite);

			CString strText = _T("");

			for (int nItem = 0; nItem < m_LogListCtrl.GetItemCount(); nItem++)
			{
				strText += m_LogListCtrl.GetItemText(nItem, 0);
				strText += m_LogListCtrl.GetItemText(nItem, 1);

				strText += _T("\n");

				file.WriteString(strText);

				strText = _T("");
			}

			file.Close();
		}
		catch (CFileException* e)
		{
			e->Delete();
		}
	}
}

void CPeerLog::OnCutSelected()
{
	POSITION posListCtrl = m_LogListCtrl.GetFirstSelectedItemPosition();	//列表中第一个元素位置

	CString strText = _T("");

	if (posListCtrl == NULL)
	{
		TRACE0("No items were selected!\n");
		return;
	}

	while (posListCtrl)
	{
		int nItem = m_LogListCtrl.GetNextSelectedItem(posListCtrl);	

		strText += m_LogListCtrl.GetItemText(nItem, 0);
		strText += m_LogListCtrl.GetItemText(nItem, 1);

		strText += '\n';
	}

	if (strText.GetAt(strText.GetLength() - 1) == '\n')
	{
		strText.Delete(strText.GetLength() - 1);
	}

	theApp.CopyTextToClipboard(strText);

//	m_LogListCtrl.DeleteAllItems(LVIS_SELECTED);
}

void CPeerLog::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	CMenu menu;

	menu.CreatePopupMenu();

	//menu.AppendMenu(MF_STRING, MP_CUTSELECTED, GetResString(IDS_CUT));
	menu.AppendMenu(MF_STRING, MP_COPYSELECTED, GetResString(IDS_COPY));
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, MP_SELECTALL, GetResString(IDS_SELECTALL));
	menu.AppendMenu(MF_STRING, MP_SAVEAS, GetResString(IDS_SAVE));

	m_pMenuXP = new CMenuXP();
	m_pMenuXP->AddMenu(&menu, TRUE);
	menu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);
	delete m_pMenuXP;
	m_pMenuXP = NULL;
}

void CPeerLog::OnDrawItem(int /*nIDCtl*/, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	m_pMenuXP->DrawItem(lpDrawItemStruct);
}

void CPeerLog::OnMeasureItem(int /*nIDCtl*/, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	m_pMenuXP->MeasureItem(lpMeasureItemStruct);
}
