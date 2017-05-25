/* 
 * $Id: FileDetailDialogName.cpp 4483 2008-01-02 09:19:06Z soarchin $
 * 
 * this file is part of eMule
 * Copyright (C)2002-2006 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "stdafx.h"
#include "emule.h"
#include "FileDetailDialogName.h"
#include "UserMsgs.h"
#include "OtherFunctions.h"
#include "PartFile.h"
#include "UpDownClient.h"
#include "TitleMenu.h"
#include "MenuCmds.h"
#include "StringConversion.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CFileDetailDialogName, CResizablePage)

BEGIN_MESSAGE_MAP(CFileDetailDialogName, CResizablePage)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTONSTRIP, OnBnClickedButtonStrip)
	ON_BN_CLICKED(IDC_TAKEOVER, TakeOver)	
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LISTCTRLFILENAMES, OnLvnColumnclick)
	ON_NOTIFY(NM_DBLCLK, IDC_LISTCTRLFILENAMES, OnNMDblclkList)
	ON_NOTIFY(NM_RCLICK, IDC_LISTCTRLFILENAMES, OnNMRclickList)
	ON_MESSAGE(UM_DATA_CHANGED, OnDataChanged)
	ON_EN_CHANGE(IDC_FILENAME, OnEnChangeFilename)
END_MESSAGE_MAP()

CFileDetailDialogName::CFileDetailDialogName()
	: CResizablePage(CFileDetailDialogName::IDD, 0)
{
	m_paFiles = NULL;
	m_bDataChanged = false;
	m_strCaption = GetResString(IDS_SW_NAME);
	m_psp.pszTitle = m_strCaption;
	m_psp.dwFlags |= PSP_USETITLE;
	m_timer = 0;
	memset(m_aiColWidths, 0, sizeof m_aiColWidths);
	m_bAppliedSystemImageList = false;
	m_sortorder = 0;
	m_sortindex = 1;
	m_bSelf = false;
}

CFileDetailDialogName::~CFileDetailDialogName()
{
}

void CFileDetailDialogName::OnTimer(UINT /*nIDEvent*/)
{
	RefreshData();
}

void CFileDetailDialogName::DoDataExchange(CDataExchange* pDX)
{
	CResizablePage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LISTCTRLFILENAMES, m_listFileNames);
}

BOOL CFileDetailDialogName::OnInitDialog()
{
	CResizablePage::OnInitDialog();
	InitWindowStyles(this);

	AddAnchor(IDC_FD_SN, TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(IDC_LISTCTRLFILENAMES, TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(IDC_TAKEOVER, BOTTOM_LEFT);
	AddAnchor(IDC_BUTTONSTRIP, BOTTOM_RIGHT);
	AddAnchor(IDC_FILENAME, BOTTOM_LEFT, BOTTOM_RIGHT);

	m_listFileNames.SetName(_T("FileDetailDlgName"));
	m_listFileNames.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
	m_listFileNames.InsertColumn(0, GetResString(IDS_DL_FILENAME), LVCFMT_LEFT, 380);
	m_listFileNames.InsertColumn(1, GetResString(IDS_DL_SOURCES), LVCFMT_LEFT, 80);
	ASSERT( (m_listFileNames.GetStyle() & LVS_SHAREIMAGELISTS) != 0 );
	m_listFileNames.LoadSettings();

	m_listFileNames.SetSortArrow();
	m_listFileNames.SortItems(&CompareListNameItems, m_listFileNames.GetSortItem() + ( (m_listFileNames.GetSortAscending()) ? 0:10) );

	Localize();

	// start time for calling 'RefreshData'
	VERIFY( (m_timer = SetTimer(301, 5000, 0)) != NULL );

	return TRUE;
}

BOOL CFileDetailDialogName::OnSetActive()
{
	if (!CResizablePage::OnSetActive())
		return FALSE;
	if (m_bDataChanged)
	{
		m_bSelf = true;
		GetDlgItem(IDC_FILENAME)->SetWindowText(STATIC_DOWNCAST(CPartFile, (*m_paFiles)[0])->GetFileName());
		m_bSelf = false;
		RefreshData();
		m_bDataChanged = false;
	}
	return TRUE;
}

LRESULT CFileDetailDialogName::OnDataChanged(WPARAM, LPARAM)
{
	m_bDataChanged = true;
	return 1;
}

void CFileDetailDialogName::RefreshData()
{
	bool bEnableRename = CanRenameFile();
	GetDlgItem(IDC_FILENAME)->EnableWindow(bEnableRename);
	GetDlgItem(IDC_BUTTONSTRIP)->EnableWindow(bEnableRename);
	GetDlgItem(IDC_TAKEOVER)->EnableWindow(bEnableRename);

	FillSourcenameList();
}

void CFileDetailDialogName::OnDestroy()
{
	m_listFileNames.SaveSettings();

	for (int i=0;i<m_listFileNames.GetItemCount();++i) {
		FCtrlItem_Struct* item= (FCtrlItem_Struct*)m_listFileNames.GetItemData(i);
		delete item;
	}

	if (m_timer){
		KillTimer(m_timer);
		m_timer = 0;
	}
}

void CFileDetailDialogName::Localize()
{
	if (thePrefs.GetLanguageID() != MAKELANGID(LANG_ENGLISH,SUBLANG_DEFAULT)){
	    GetDlgItem(IDC_TAKEOVER)->SetWindowText(GetResString(IDS_TAKEOVER));
	    GetDlgItem(IDC_BUTTONSTRIP)->SetWindowText(GetResString(IDS_CLEANUP));
	    GetDlgItem(IDC_FD_SN)->SetWindowText(GetResString(IDS_SOURCENAMES));
    }
}

void CFileDetailDialogName::FillSourcenameList()
{
	LVFINDINFO info; 
	info.flags = LVFI_STRING; 
	int itempos; 

	CString strText; 

	// reset
	for (int i=0;i<m_listFileNames.GetItemCount();i++){
		FCtrlItem_Struct* item= (FCtrlItem_Struct*)m_listFileNames.GetItemData(i);
		item->count=0;
	}

	// update
	const CPartFile* file = STATIC_DOWNCAST(CPartFile, (*m_paFiles)[0]);
	for (POSITION pos = file->srclist.GetHeadPosition(); pos != NULL; )
	{ 
		CUpDownClient* cur_src = file->srclist.GetNext(pos); 
		if (cur_src->GetRequestFile() != file || cur_src->GetClientFilename().GetLength()==0)
			continue;

		info.psz = cur_src->GetClientFilename(); 
		if ((itempos=m_listFileNames.FindItem(&info, -1)) == -1)
		{ 
			FCtrlItem_Struct* newitem= new FCtrlItem_Struct();
			newitem->count=1;
			newitem->filename=cur_src->GetClientFilename();

			int iSystemIconIdx = theApp.GetFileTypeSystemImageIdx(cur_src->GetClientFilename());
			if (theApp.GetSystemImageList() && !m_bAppliedSystemImageList)
			{
				m_listFileNames.ApplyImageList(theApp.GetSystemImageList());
				ASSERT( (m_listFileNames.GetStyle() & LVS_SHAREIMAGELISTS) != 0 );
				m_bAppliedSystemImageList = true;
			}

			int ix=m_listFileNames.InsertItem(LVIF_TEXT|LVIF_PARAM|LVIF_IMAGE, m_listFileNames.GetItemCount() ,cur_src->GetClientFilename(),0,0,iSystemIconIdx,(LPARAM)newitem);
			m_listFileNames.SetItemText(ix, 1, _T("1")); 
		}
		else
		{
			FCtrlItem_Struct* item= (FCtrlItem_Struct*)m_listFileNames.GetItemData(itempos);
			item->count+=1;
			strText.Format(_T("%i"),item->count);
			m_listFileNames.SetItemText(itempos, 1,strText ); 
		} 
	} 

	// remove 0'er
	for (int i=0;i<m_listFileNames.GetItemCount();i++)
	{
		FCtrlItem_Struct* item= (FCtrlItem_Struct*)m_listFileNames.GetItemData(i);
		if (item && item->count==0)
		{
			delete item;
			m_listFileNames.DeleteItem(i);
			i=0;
		}
	}

	m_listFileNames.SortItems(&CompareListNameItems, m_sortindex + ( (m_sortorder) ? 0:10) );
}

void CFileDetailDialogName::TakeOver()
{
	int iSel = m_listFileNames.GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1)
		GetDlgItem(IDC_FILENAME)->SetWindowText(m_listFileNames.GetItemText(iSel, 0));
}

void CFileDetailDialogName::Copy()
{
	int iSel = m_listFileNames.GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1)
		theApp.CopyTextToClipboard(m_listFileNames.GetItemText(iSel, 0));
}

void CFileDetailDialogName::OnBnClickedButtonStrip()
{
	CString filename;
	GetDlgItem(IDC_FILENAME)->GetWindowText(filename);
	GetDlgItem(IDC_FILENAME)->SetWindowText(CleanupFilename(filename));
}

void CFileDetailDialogName::OnLvnColumnclick(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	if (m_sortindex != pNMLV->iSubItem)
		m_sortorder = 1;
	else
		m_sortorder = !m_sortorder;
	m_sortindex = pNMLV->iSubItem;

	m_listFileNames.SetSortArrow(m_sortindex, m_sortorder);
	m_listFileNames.SortItems(&CompareListNameItems, m_sortindex + (m_sortorder ? 0 : 10));

	*pResult = 0;
}

int CALLBACK CFileDetailDialogName::CompareListNameItems(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	FCtrlItem_Struct* item1=(FCtrlItem_Struct*) lParam1;
	FCtrlItem_Struct* item2=(FCtrlItem_Struct*) lParam2;
	switch (lParamSort){
		case 0:
			return CompareLocaleStringNoCase(item1->filename, item2->filename);
		case 10:
			return CompareLocaleStringNoCase(item2->filename, item1->filename);
		case 1:
			return (item1->count - item2->count);
		case 11:
			return (item2->count - item1->count);
	}
	return 0;
} 

void CFileDetailDialogName::OnNMDblclkList(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	TakeOver();
	*pResult = 0;
}

void CFileDetailDialogName::OnNMRclickList(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	UINT flag = MF_STRING;
	if (m_listFileNames.GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED) == -1)
		flag = MF_GRAYED;

	POINT point;
	::GetCursorPos(&point);
	CTitleMenu popupMenu;
	popupMenu.CreatePopupMenu();
	popupMenu.AppendMenu(flag,MP_MESSAGE, GetResString(IDS_TAKEOVER));
	popupMenu.AppendMenu(flag,MP_COPYSELECTED, GetResString(IDS_COPY));
	popupMenu.AppendMenu(MF_STRING,MP_RESTORE, GetResString(IDS_SV_UPDATE));
	popupMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);
	VERIFY( popupMenu.DestroyMenu() );

	*pResult = 0;
}

BOOL CFileDetailDialogName::OnCommand(WPARAM wParam,LPARAM lParam )
{
	int iSel = m_listFileNames.GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1)
	{
		switch (wParam)
		{
		case MP_MESSAGE:
			TakeOver();
			return true;
		case MP_COPYSELECTED:
			Copy();
			return true;
		case MP_RESTORE:
			FillSourcenameList();
			return true;
		}
	}
	return CResizablePage::OnCommand(wParam, lParam);
}

void CFileDetailDialogName::RenameFile()
{
	if (CanRenameFile())
	{
		CString strNewFileName;
		GetDlgItem(IDC_FILENAME)->GetWindowText(strNewFileName);
		strNewFileName.Trim();
		if (strNewFileName.IsEmpty() || !IsValidEd2kString(strNewFileName))
			return;
		CPartFile* file = STATIC_DOWNCAST(CPartFile, (*m_paFiles)[0]);
		file->SetFileName(strNewFileName, true);
		file->UpdateDisplayedInfo();
		file->SavePartFile();
	}
}

bool CFileDetailDialogName::CanRenameFile() const
{
	const CPartFile* file = STATIC_DOWNCAST(CPartFile, (*m_paFiles)[0]);
	return (file->GetStatus() != PS_COMPLETE && file->GetStatus() != PS_COMPLETING);
}

void CFileDetailDialogName::OnEnChangeFilename()
{
	if (!m_bSelf)
		SetModified();
}

BOOL CFileDetailDialogName::OnApply()
{
	if (!m_bDataChanged)
		RenameFile();
	return CResizablePage::OnApply();
}
