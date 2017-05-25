/* 
 * $Id: SharedFilesWnd.cpp 9780 2009-01-07 07:58:37Z dgkang $
 * 
 * this file is part of eMule
 * Copyright (C)2002-2005 Merkur ( devs@emule-project.net / http://www.emule-project.net )
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

#include "stdafx.h"
#include "emule.h"
#include "emuleDlg.h"
#include "SharedFilesWnd.h"
#include "OtherFunctions.h"
#include "SharedFileList.h"
#include "KnownFileList.h"
#include "KnownFile.h"
#include "SharedFilesWnd.h"

#include "TabItem_Cake.h"

#include "MenuCmds.h"
#include "CmdFuncs.h"
#include "PageTabBkDraw.h"
#include "UserMsgs.h"
#include "MainTabWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	SPLITTER_RANGE_MIN		100
#define	SPLITTER_RANGE_MAX		350

#define	WND_SPLITTER_YOFF	8
#define	WND_SPLITTER_HEIGHT	5

#define	SPLITTER_MARGIN			1
#define	SPLITTER_WIDTH			5
// CSharedFilesWnd dialog

IMPLEMENT_DYNAMIC(CSharedFilesWnd, CDialog)

BEGIN_MESSAGE_MAP(CSharedFilesWnd, CResizableDialog)
	ON_BN_CLICKED(IDC_RELOADSHAREDFILES, OnBnClickedReloadsharedfiles)
	ON_NOTIFY(LVN_ITEMACTIVATE, IDC_SFLIST, OnLvnItemActivateSflist)
	ON_NOTIFY(NM_CLICK, IDC_SFLIST, OnNMClickSflist)
	ON_NOTIFY(UM_SPLITTER_CLICKED, IDC_STATIC_SPLITTER, OnHSplitterClicked)
	ON_NOTIFY(UM_SPLITTER_CLICKED, IDC_SPLITTER_SHAREDFILES, OnVSplitterClicked)
	ON_WM_SYSCOLORCHANGE()
	ON_STN_DBLCLK(IDC_FILES_ICO, OnStnDblclickFilesIco)
	ON_NOTIFY(TVN_SELCHANGED, IDC_SHAREDDIRSTREE, OnTvnSelchangedShareddirstree)
	ON_MESSAGE(UM_MTSD_CUR_SEL_FILE, OnListSelFileChanged)
	ON_NOTIFY(UM_SPN_SIZED, IDC_STATIC_SPLITTER, OnSplitterMoved)
	ON_NOTIFY(NMC_TW_ACTIVE_TAB_CHANDED, IDC_SHAREFILETABWND, OnNM_TabInfo_ActiveTabChanged)
END_MESSAGE_MAP()

CSharedFilesWnd::CSharedFilesWnd(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CSharedFilesWnd::IDD, pParent)
{
	icon_files = NULL;

	m_WndHSpliterPos = (UINT)-1;
	m_WndVSpliterPos = (UINT)-1;
	int i = 0;
	for (i = 0; i < TI_MAX; i++)
		m_tabIds[i] = NULL;
}

CSharedFilesWnd::~CSharedFilesWnd()
{
	if (icon_files)
		VERIFY( DestroyIcon(icon_files) );
}

void CSharedFilesWnd::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SFLIST, sharedfilesctrl);
	DDX_Control(pDX, IDC_SHAREDDIRSTREE, m_ctlSharedDirTree);
}

BOOL CSharedFilesWnd::OnInitDialog()
{
	CResizableDialog::OnInitDialog();	

	InitControlContainer();
	InitWindowStyles(this);
	SetAllIcons();

	CreateSplitter();
	CreateTabWnd();
	InitCtrlsSize();
	
	//////////////////////////////////////////////////////////////////
	
	sharedfilesctrl.Init();
	m_ctlSharedDirTree.Initalize(&sharedfilesctrl);

	LOGFONT lf;
	GetFont()->GetLogFont(&lf);
	lf.lfWeight = FW_BOLD;
	bold.CreateFontIndirect(&lf);

	Localize();



	CRect rcWnd;
	GetWindowRect(rcWnd);
	ScreenToClient(rcWnd);

	LONG HSplitPos  = thePrefs.GetSplitterbarPosition() * rcWnd.Height() / 100;
	LONG VSplitPost = thePrefs.GetSplitterbarPositionShared();// * rcWnd.Width() / 100;

	CRect rcDlgItem;

	GetDlgItem(IDC_SHAREDDIRSTREE)->GetWindowRect(rcDlgItem);
	ScreenToClient(rcDlgItem);
	rcDlgItem.right = VSplitPost;
	GetDlgItem(IDC_SHAREDDIRSTREE)->MoveWindow(&rcDlgItem,FALSE);

	GetDlgItem(IDC_SFLIST)->GetWindowRect(rcDlgItem);
	ScreenToClient(rcDlgItem);
	rcDlgItem.left = VSplitPost + m_wndVSplitter.GetHBreadth();
	rcDlgItem.bottom = HSplitPos;
	GetDlgItem(IDC_SFLIST)->MoveWindow(&rcDlgItem, FALSE);

	m_tabWnd.GetWindowRect(rcDlgItem);
	ScreenToClient(rcDlgItem);
	rcDlgItem.top = HSplitPos + m_wndHSplitter.GetHBreadth() ;
	rcDlgItem.left = VSplitPost + m_wndVSplitter.GetHBreadth();
	m_tabWnd.MoveWindow(rcDlgItem);


	AddAnchor(IDC_FILES_ICO, TOP_LEFT);
	AddAnchor(IDC_RELOADSHAREDFILES, TOP_RIGHT);
	AddAnchor(IDC_SFLIST,TOP_LEFT, CSize(100, thePrefs.GetSplitterbarPosition()));
	AddAnchor(IDC_SHAREDDIRSTREE,TOP_LEFT, BOTTOM_LEFT);
	AddAnchor(IDC_TRAFFIC_TEXT,TOP_LEFT);
	AddAnchor(m_tabWnd.m_hWnd,CSize(0, thePrefs.GetSplitterbarPosition()), BOTTOM_RIGHT);

	DoHResize(0);

	return TRUE;
}

void CSharedFilesWnd::DoVResize(int delta)
{
	CSplitterControl::ChangeWidth((CWnd*)&m_ctlSharedDirTree, delta);
	CSplitterControl::ChangeWidth((CWnd*)&sharedfilesctrl, -delta, CW_RIGHTALIGN);
	CSplitterControl::ChangeWidth((CWnd*)&m_tabWnd, -delta, CW_RIGHTALIGN);
	CSplitterControl::ChangeWidth((CWnd*)&m_wndHSplitter, -delta, CW_RIGHTALIGN);


	CRect rcW;
	GetWindowRect(rcW);
	ScreenToClient(rcW);

	CRect rcspl;
	GetDlgItem(IDC_SHAREDDIRSTREE)->GetClientRect(rcspl);

	thePrefs.SetSplitterbarPositionShared(rcspl.right + 1);
	//thePrefs.SetSplitterbarPositionShared(rcspl.right  * 100 / rcW.Width());

	RemoveAnchor(m_wndVSplitter);
	AddAnchor(m_wndVSplitter, TOP_LEFT);

	RemoveAnchor(IDC_FILES_ICO);
	RemoveAnchor(IDC_RELOADSHAREDFILES);
	RemoveAnchor(IDC_SFLIST);
	RemoveAnchor(IDC_SHAREDDIRSTREE);
	RemoveAnchor(IDC_TRAFFIC_TEXT);
	RemoveAnchor(m_tabWnd.m_hWnd);

	AddAnchor(IDC_FILES_ICO, TOP_LEFT);
	AddAnchor(IDC_RELOADSHAREDFILES, TOP_RIGHT);
	AddAnchor(IDC_SFLIST,TOP_LEFT,CSize(100, thePrefs.GetSplitterbarPosition()));
	AddAnchor(IDC_SHAREDDIRSTREE,TOP_LEFT, BOTTOM_LEFT);
	AddAnchor(IDC_TRAFFIC_TEXT,TOP_LEFT);
	AddAnchor(m_tabWnd.m_hWnd,CSize(0, thePrefs.GetSplitterbarPosition()), BOTTOM_RIGHT);

	m_wndVSplitter.SetRange(rcW.left + 3, rcW.left + SPLITTER_RANGE_MAX);

	if(rcspl.Width()<5)
	{
		m_wndVSplitter.m_nflag = 0;
	}
	else
	{
		m_wndVSplitter.m_nflag = 1;
	}
	Invalidate();
	UpdateWindow();
}

void CSharedFilesWnd::DoHResize(int delta)
{
	CSplitterControl::ChangeHeight((CWnd*)&sharedfilesctrl, delta);
	CSplitterControl::ChangeHeight((CWnd*)&m_tabWnd, -delta, CW_BOTTOMALIGN);

	UpdateSplitterRange();
}


void CSharedFilesWnd::Reload()
{	
	sharedfilesctrl.SetDirectoryFilter(NULL, false);
	m_ctlSharedDirTree.Reload();
	sharedfilesctrl.SetDirectoryFilter(m_ctlSharedDirTree.GetSelectedFilter(), false);
	CGlobalVariable::sharedfiles->Reload();

	ShowSelectedFilesSummary();
}

void CSharedFilesWnd::OnStnDblclickFilesIco()
{
	theApp.emuledlg->ShowPreferences(IDD_PPG_DIRECTORIES);
}

void CSharedFilesWnd::OnBnClickedReloadsharedfiles()
{
	CWaitCursor curWait;
	Reload();
}

void CSharedFilesWnd::OnLvnItemActivateSflist(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	ShowSelectedFilesSummary();
}
bool CSharedFilesWnd::AddNewClient(CUpDownClient *pNewClient)
{
  int iSel = sharedfilesctrl.GetSelectedCount();
  if (iSel == 0)
  {
	  return true;
  }
  POSITION pos = sharedfilesctrl.GetFirstSelectedItemPosition();
  while (pos)
  {
	  int iItem = sharedfilesctrl.GetNextSelectedItem(pos);
	  const CKnownFile* pFile = (CKnownFile*)sharedfilesctrl.GetItemData(iItem);
      POSITION position = pFile->m_ClientUploadList.GetHeadPosition();
	  while (position)
	  {
		  CUpDownClient *pClient = pFile->m_ClientUploadList.GetNext(position);
		  if (pClient == pNewClient)
		  {
			  return true;
		  }
	  } 
  }
  return false;
}
void CSharedFilesWnd::ShowSelectedFilesSummary()
{
	if(theApp.emuledlg)
		if(theApp.emuledlg->m_mainTabWnd.GetActiveTab() != theApp.emuledlg->m_mainTabWnd.m_aposTabs[CMainTabWnd::TI_SHARE])
			return;

	const CKnownFile* pTheFile = NULL;
	int iFiles = 0;
	uint64 uTransferred = 0;
	UINT uRequests = 0;
	UINT uAccepted = 0;
	uint64 uAllTimeTransferred = 0;
	UINT uAllTimeRequests = 0;
	UINT uAllTimeAccepted = 0;

	CList<CKnownFile *, CKnownFile *> filelist;

	POSITION pos = sharedfilesctrl.GetFirstSelectedItemPosition();

	while (pos)
	{
		int iItem = sharedfilesctrl.GetNextSelectedItem(pos);
		const CKnownFile* pFile = (CKnownFile*)sharedfilesctrl.GetItemData(iItem);
		iFiles++;
		if (iFiles == 1)
			pTheFile = pFile;

		filelist.AddTail((CKnownFile*)pFile);

		uTransferred += pFile->statistic.GetTransferred();
		uRequests += pFile->statistic.GetRequests();
		uAccepted += pFile->statistic.GetAccepts();

		uAllTimeTransferred += pFile->statistic.GetAllTimeTransferred();
		uAllTimeRequests += pFile->statistic.GetAllTimeRequests();
		uAllTimeAccepted += pFile->statistic.GetAllTimeAccepts();
	}

	if (iFiles != 0)
	{
		m_StatisticsInfo.SetTransfer((int)CGlobalVariable::knownfiles->transferred, uTransferred);
		m_StatisticsInfo.SetRequest(CGlobalVariable::knownfiles->requested, uRequests);
		m_StatisticsInfo.SetAcceptUpload(CGlobalVariable::knownfiles->accepted, uAccepted);
		m_StatisticsInfo.SetAll(uAllTimeRequests, uAllTimeAccepted, uAllTimeTransferred);

		CString str(GetResString(IDS_SF_STATISTICS));
		if (iFiles == 1 && pTheFile != NULL)
			str += _T(" (") + MakeStringEscaped(pTheFile->GetFileName()) +_T(")");
		m_StatisticsInfo.SetStatisticsFrmText(str);

		if(m_tabWnd.GetActiveTab() == m_tabIds[TI_UPLOADING])
		{
			m_UpLoading.uploadlistctrl.ShowUpLoadingUsers(filelist);
		}

		//VC-dgkang 2008年7月15日
#ifndef _FOREIGN_VERSION
		if( IsRemarkTabActived() )
			m_UserComment.Refresh(filelist.GetHead());
#endif
	}
	else
	{
		m_StatisticsInfo.SetNoFile();
		m_StatisticsInfo.SetStatisticsFrmText(GetResString(IDS_SF_STATISTICS));

		if(m_tabWnd.GetActiveTab() == m_tabIds[TI_UPLOADING])
		{
			CGlobalVariable::sharedfiles->GetAllSharedFile(filelist);
			m_UpLoading.uploadlistctrl.ShowUpLoadingUsers(filelist);
		}
	}
}

void CSharedFilesWnd::ShowAllUploadingUsers()
{
	m_UpLoading.uploadlistctrl.DeleteAllItems();

	CKnownFile* cur_file;	
	CUpDownClient * pClient;
	
	CList<CKnownFile *, CKnownFile *> filelist;
	CGlobalVariable::sharedfiles->GetAllSharedFile(filelist);	
	for( POSITION pos1 = filelist.GetHeadPosition(); pos1 != 0; )
	{
		cur_file =filelist.GetNext(pos1);
		for (POSITION pos2 = cur_file->m_ClientUploadList.GetHeadPosition(); pos2 != 0; )
		{
			pClient = cur_file->m_ClientUploadList.GetNext(pos2);
			m_UpLoading.uploadlistctrl.AddClient(pClient);
		}	
	}
		
/*	the follow code only show the fullfile 
	for (POSITION pos1 =  CGlobalVariable::knownfiles->GetKnownFiles().GetStartPosition();pos1 != 0;)
	{
		CGlobalVariable::knownfiles->GetKnownFiles().GetNextAssoc(pos1,bufKey,cur_file);

		for (POSITION pos2 = cur_file->m_ClientUploadList.GetHeadPosition(); pos2 != 0; )
		{
			pClient = cur_file->m_ClientUploadList.GetNext(pos2);
			m_UpLoading.uploadlistctrl.AddClient(pClient);
		}
	}
*/		
}
void CSharedFilesWnd::OnNMClickSflist(NMHDR *pNMHDR, LRESULT *pResult)
{
	OnLvnItemActivateSflist(pNMHDR,pResult);
	*pResult = 0;
}

void CSharedFilesWnd::OnNM_TabInfo_ActiveTabChanged(NMHDR* pNMHDR, LRESULT *pResult)
{
	__try
	{	
		NMTW_TABOP *pTabOp = reinterpret_cast<NMTW_TABOP*>(pNMHDR);

		if (pTabOp->posTab == m_tabIds[TI_REMARK])
		{
			POSITION pos = sharedfilesctrl.GetFirstSelectedItemPosition();
			int iItem = sharedfilesctrl.GetNextSelectedItem(pos);
			const CKnownFile* pFile = (CKnownFile*)sharedfilesctrl.GetItemData(iItem);
			if ( NULL!= pFile )
				m_UserComment.Refresh(pFile);			
		}

		*pResult = 0;
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
	}
}

BOOL CSharedFilesWnd::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		// Don't handle Ctrl+Tab in this window. It will be handled by main window.
		if (pMsg->wParam == VK_TAB && GetAsyncKeyState(VK_CONTROL) < 0)
			return FALSE;

		if (VK_RETURN == pMsg->wParam || VK_ESCAPE == pMsg->wParam)
			return FALSE;
	}
	else if (pMsg->message == WM_KEYUP)
	{
		if (pMsg->hwnd == GetDlgItem(IDC_SFLIST)->m_hWnd)
			OnLvnItemActivateSflist(0, 0);
	}
	else if (pMsg->message == WM_MBUTTONUP)
	{
		POINT point;
		::GetCursorPos(&point);
		CPoint p = point; 
		sharedfilesctrl.ScreenToClient(&p); 
		int it = sharedfilesctrl.HitTest(p); 
		if (it == -1)
			return FALSE;

		sharedfilesctrl.SetItemState(-1, 0, LVIS_SELECTED);
		sharedfilesctrl.SetItemState(it, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		sharedfilesctrl.SetSelectionMark(it);   // display selection mark correctly!
		sharedfilesctrl.ShowComments((CKnownFile*)sharedfilesctrl.GetItemData(it));
		return TRUE;
	}

	return CResizableDialog::PreTranslateMessage(pMsg);
}

void CSharedFilesWnd::OnSysColorChange()
{
	CResizableDialog::OnSysColorChange();
	SetAllIcons();
}

void CSharedFilesWnd::CreateSplitter()
{
	CRect	rcLeft;
	GetDlgItem(IDC_SHAREDDIRSTREE)->GetWindowRect(rcLeft);
	ScreenToClient(rcLeft);

	//创建垂直分割条
	CRect rcSpl;
	GetDlgItem(IDC_SPLITTER_SHAREDFILES)->GetWindowRect(rcSpl);
	ScreenToClient(rcSpl);
	rcSpl.left = rcLeft.right;
	rcSpl.right = rcSpl.left;
	m_wndVSplitter.Create(WS_CHILD | WS_VISIBLE, rcSpl, this, IDC_SPLITTER_SHAREDFILES);
	m_wndVSplitter.SetNormalDraw(FALSE);

	CRect	rcTop;
	GetDlgItem(IDC_SFLIST)->GetWindowRect(rcTop);
	ScreenToClient(rcTop);


	//创建水平分割条
	rcSpl.left = rcTop.left;
	rcSpl.top = rcTop.bottom + 5;
	rcSpl.bottom = rcTop.top + 8;
	rcSpl.right = rcTop.right;

	m_wndHSplitter.Create(WS_CHILD | WS_VISIBLE, rcSpl, this, IDC_STATIC_SPLITTER);
	m_wndHSplitter.SetNormalDraw(FALSE);
}

void CSharedFilesWnd::CreateTabWnd()
{
	CRect	rcVSplitter;
	m_wndVSplitter.GetWindowRect(&rcVSplitter);
	ScreenToClient(&rcVSplitter);
	CRect	rcHSplitter;
	m_wndHSplitter.GetWindowRect(&rcHSplitter);
	ScreenToClient(&rcHSplitter);

	CRect	rect;
	GetDlgItem(IDC_TAB_FILE_INFO)->GetWindowRect(&rect);
	ScreenToClient(&rect);
	rect.left = rcVSplitter.right;
	rect.top = rcHSplitter.bottom;
	m_tabWnd.Create(WS_VISIBLE | WS_CHILD, rect, this, IDC_SHAREFILETABWND);

	m_DetailInfo.Create(m_DetailInfo.IDD, this);
	m_StatisticsInfo.Create(m_StatisticsInfo.IDD, this);
	m_UpLoading.Create(m_UpLoading.IDD, this);

	//VC-dgkang 2008年7月15日
#ifndef _FOREIGN_VERSION
	m_UserComment.Create(m_UserComment.IDD, this);
#endif

	CTabItem_Cake	*pTiCake = NULL;

	pTiCake = new CTabItem_Cake;
	pTiCake->SetCaption(GetResString(IDS_DETAIL_INFO));
	pTiCake->SetRelativeWnd(m_DetailInfo.GetSafeHwnd());
	pTiCake->SetIcon(_T("PNG_DETAILINFO"));
	m_tabIds[TI_DETAIL] = m_tabWnd.AddTab(pTiCake);
	pTiCake = NULL;

	//VC-dgkang 2008年7月15日
#ifndef _FOREIGN_VERSION
	pTiCake = new CTabItem_Cake;
	pTiCake->SetCaption(GetResString(IDS_USER_COMMENT));
	pTiCake->SetRelativeWnd(m_UserComment.GetSafeHwnd());
	pTiCake->SetIcon(_T("PNG_DETAILCOMMENT"));
	m_tabIds[TI_REMARK] = m_tabWnd.AddTab(pTiCake);
	pTiCake = NULL;
#endif

	pTiCake = new CTabItem_Cake;
	pTiCake->SetCaption(GetResString(IDS_SL_STAT));
	pTiCake->SetRelativeWnd(m_StatisticsInfo.GetSafeHwnd());
	pTiCake->SetIcon(_T("PNG_DETAILSTAT"));
	m_tabIds[TI_STAT] = m_tabWnd.AddTab(pTiCake);
	pTiCake = NULL;

	pTiCake = new CTabItem_Cake;
	pTiCake->SetCaption(GetResString(IDS_SL_UPLOADING));
	pTiCake->SetRelativeWnd(m_UpLoading.GetSafeHwnd());
	pTiCake->SetIcon(_T("PNG_DETAILUPLOAD"));
	m_tabIds[TI_UPLOADING] = m_tabWnd.AddTab(pTiCake);
	pTiCake = NULL;
}

void CSharedFilesWnd::InitCtrlsSize()
{

	CRect	rcTree;
	GetDlgItem(IDC_SHAREDDIRSTREE)->GetWindowRect(rcTree);
	ScreenToClient(rcTree);

	CRect	rcVSplitter;
	m_wndVSplitter.GetWindowRect(&rcVSplitter);
	ScreenToClient(rcVSplitter);
	rcVSplitter.left = rcTree.right;
	rcVSplitter.right = rcVSplitter.left + 6;
	m_wndVSplitter.MoveWindow(&rcVSplitter, FALSE);

	CRect	rcList;
	GetDlgItem(IDC_SFLIST)->GetWindowRect(rcList);
	ScreenToClient(rcList);
	rcList.left = rcVSplitter.right;
	GetDlgItem(IDC_SFLIST)->MoveWindow(&rcList, FALSE);

	CRect	rcHSplitter;
	m_wndHSplitter.GetWindowRect(&rcHSplitter);
	ScreenToClient(rcHSplitter);
	rcHSplitter.left = rcVSplitter.right;
	rcHSplitter.top = rcList.bottom;
	rcHSplitter.bottom = rcHSplitter.top + 5;
	m_wndHSplitter.MoveWindow(&rcHSplitter, FALSE);

	CRect	rcTabWnd;
	m_tabWnd.GetWindowRect(rcTabWnd);
	ScreenToClient(rcTabWnd);
	rcTabWnd.left = rcVSplitter.right;
	rcTabWnd.top = rcHSplitter.bottom;
	m_tabWnd.MoveWindow(&rcTabWnd, FALSE);
}

void CSharedFilesWnd::SetAllIcons()
{
	if (icon_files)
		VERIFY( DestroyIcon(icon_files) );
	icon_files = theApp.LoadIcon(_T("SharedFilesList"), 16, 16);
	((CStatic*)GetDlgItem(IDC_FILES_ICO))->SetIcon(icon_files);
}

void CSharedFilesWnd::Localize()
{
	sharedfilesctrl.Localize();
	m_ctlSharedDirTree.Localize();
	sharedfilesctrl.SetDirectoryFilter(NULL,true);

	GetDlgItem(IDC_TRAFFIC_TEXT)->SetWindowText(GetResString(IDS_SF_FILES));
	GetDlgItem(IDC_RELOADSHAREDFILES)->SetWindowText(GetResString(IDS_SF_RELOAD));

	m_tabWnd.SetTabText(m_tabIds[TI_DETAIL], GetResString(IDS_DETAIL_INFO));
	m_tabWnd.SetTabText(m_tabIds[TI_REMARK], GetResString(IDS_USER_COMMENT));
	m_tabWnd.SetTabText(m_tabIds[TI_STAT], GetResString(IDS_SL_STAT));
	m_tabWnd.SetTabText(m_tabIds[TI_UPLOADING], GetResString(IDS_SL_UPLOADING));
}

void CSharedFilesWnd::OnTvnSelchangedShareddirstree(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	bool bShared = m_ctlSharedDirTree.GetDirectoryState(m_ctlSharedDirTree.GetSelectedFilter()->m_strFullPath);

	sharedfilesctrl.SetDirectoryState(bShared);
	sharedfilesctrl.SetDirectoryFilter(m_ctlSharedDirTree.GetSelectedFilter(), !m_ctlSharedDirTree.IsCreatingTree());
	
	*pResult = 0;
}

LRESULT CSharedFilesWnd::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch (message)
	{
	case WM_PAINT:
		if (m_wndVSplitter)
		{
			CRect rcW;
			GetWindowRect(rcW);
			ScreenToClient(rcW);
			if (rcW.Width() > 0)
			{
				CRect rctree;
				GetDlgItem(IDC_SHAREDDIRSTREE)->GetWindowRect(rctree);
				ScreenToClient(rctree);
				CRect rcSpl;
				rcSpl.left = rctree.right + SPLITTER_MARGIN;
				rcSpl.right = rcSpl.left + SPLITTER_WIDTH;
				rcSpl.top = rctree.top;
				rcSpl.bottom = rctree.bottom;
				m_wndVSplitter.MoveWindow(rcSpl, TRUE);
			}
		}

		if(m_wndHSplitter)
		{
			CRect rcWnd;
			GetWindowRect(rcWnd);
			if (rcWnd.Height() > 0)
			{
				CRect rcSFList;
				GetDlgItem(IDC_SFLIST)->GetWindowRect(rcSFList);
				ScreenToClient(rcSFList);
				CRect rcSpl;
				rcSpl.left = rcSFList.left;
				rcSpl.right = rcSFList.right;
				rcSpl.top = rcSFList.bottom;
				rcSpl.bottom = rcSpl.top + WND_SPLITTER_HEIGHT;
				m_wndHSplitter.MoveWindow(rcSpl, TRUE);
				UpdateSplitterRange();
			}
		}
		break;

		case WM_NOTIFY:
			if (wParam == IDC_SPLITTER_SHAREDFILES)
			{ 
				SPC_NMHDR* pHdr = (SPC_NMHDR*)lParam;
				DoVResize(pHdr->delta);
			}
			break;

		case WM_SIZE:
			if (m_wndVSplitter)
			{
				CRect rc;
				GetWindowRect(rc);
				ScreenToClient(rc);
				m_wndVSplitter.SetRange(rc.left + 3, rc.left + SPLITTER_RANGE_MAX);
			}
			break;
	}

	return CResizableDialog::DefWindowProc(message, wParam, lParam);
}

LRESULT CSharedFilesWnd::OnListSelFileChanged(WPARAM /*wParam*/, LPARAM lParam)
{
	m_DetailInfo.UpdateInfo((CPartFile*) lParam, CDetailInfo::IM_COMBINE_SHARE);
	return 0;
}

void CSharedFilesWnd::UpdateSplitterRange(void)
{
	CRect rcWnd;
	GetWindowRect(rcWnd);

	CRect rcTop;
	GetDlgItem(IDC_SFLIST)->GetWindowRect(rcTop);
	ScreenToClient(rcTop);

	CRect rcBottom;
	m_tabWnd.GetWindowRect(rcBottom);
	ScreenToClient(rcBottom);

	rcBottom.bottom = rcWnd.bottom - 3;

	thePrefs.SetSplitterbarPosition((rcTop.bottom * 100) / rcWnd.Height());

	RemoveAnchor(IDC_SFLIST);
	RemoveAnchor(m_tabWnd.m_hWnd);

	AddAnchor(IDC_SFLIST,TOP_LEFT,CSize(100, thePrefs.GetSplitterbarPosition()));
	AddAnchor(m_tabWnd.m_hWnd,CSize(0, thePrefs.GetSplitterbarPosition()), BOTTOM_RIGHT);

	if(rcBottom.Height()< 364)
	{
		m_wndHSplitter.m_nflag = 0;
	}
	else
	{
		m_wndHSplitter.m_nflag = 1;
	}

	m_wndHSplitter.SetRange(rcTop.top + 50, rcBottom.bottom - 364);
}

void CSharedFilesWnd::OnSplitterMoved(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	SPC_NMHDR* pHdr = (SPC_NMHDR*)pNMHDR;
	DoHResize(pHdr->delta);
}

void CSharedFilesWnd::OnHSplitterClicked(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	SPCEX_NMHDR* pHdr = (SPCEX_NMHDR*)pNMHDR;

	ShowList(pHdr->flag);
}

void CSharedFilesWnd::OnVSplitterClicked(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	SPCEX_NMHDR* pHdr = (SPCEX_NMHDR*)pNMHDR;
	ShowTree(pHdr->flag);
}
void CSharedFilesWnd::ShowList(int nflag)
{
	if(nflag)
	{
		CRect rcWnd;
		GetWindowRect(rcWnd);
		ScreenToClient(rcWnd);


		CRect rcSharedFileList;
		sharedfilesctrl.GetWindowRect(rcSharedFileList);
		ScreenToClient(rcSharedFileList);
		m_WndHSpliterPos = (rcSharedFileList.bottom * 100) / rcWnd.Height();
		rcSharedFileList.bottom = rcWnd.bottom - 8;

		RemoveAnchor(sharedfilesctrl);

		CRect rcTabInfo;
		m_tabWnd.GetWindowRect(rcTabInfo);
		ScreenToClient(rcTabInfo);
		rcTabInfo.bottom = rcWnd.bottom;// - 20;
		rcTabInfo.top = rcWnd.bottom;// - 23;
		RemoveAnchor(m_tabWnd);

		sharedfilesctrl.MoveWindow(rcSharedFileList);
		m_tabWnd.MoveWindow(rcTabInfo);

		AddAnchor(sharedfilesctrl.m_hWnd,TOP_LEFT, CSize(100, thePrefs.GetSplitterbarPosition()));
		AddAnchor(m_tabWnd.m_hWnd,CSize(0, thePrefs.GetSplitterbarPosition()), BOTTOM_RIGHT);
		
		m_wndHSplitter.m_nflag = 0;
	}
	else
	{
		CRect rcWnd;
		GetWindowRect(rcWnd);
		ScreenToClient(rcWnd);

		LONG splitpos = 0;

		if(m_WndHSpliterPos == -1)
		{
			splitpos = 364;
		}
		else
		{
			splitpos = (m_WndHSpliterPos * rcWnd.Height()) / 100;
		}

		CRect rcSharedFileList;
		sharedfilesctrl.GetWindowRect(rcSharedFileList);
		ScreenToClient(rcSharedFileList);
		rcSharedFileList.bottom = splitpos - 5;
		RemoveAnchor(sharedfilesctrl);

		CRect rcTabInfo;
		m_tabWnd.GetWindowRect(rcTabInfo);
		ScreenToClient(rcTabInfo);
		rcTabInfo.top = splitpos;
		rcTabInfo.bottom = rcWnd.bottom - 3;
		RemoveAnchor(m_tabWnd);

		sharedfilesctrl.MoveWindow(rcSharedFileList);
		m_tabWnd.MoveWindow(rcTabInfo);

		m_wndHSplitter.m_nflag = 1;
	}
}

void CSharedFilesWnd::ShowTree(int nflag)
{
	if(nflag)
	{   //隐藏sharedfilesctrl
		CRect rcWnd;
		GetWindowRect(rcWnd);
		ScreenToClient(rcWnd);

		CRect rtSharedFileTree;
		m_ctlSharedDirTree.GetWindowRect(rtSharedFileTree);
		ScreenToClient(rtSharedFileTree);
		m_WndVSpliterPos = (rtSharedFileTree.right * 100) / rcWnd.Width();
		rtSharedFileTree.left = rcWnd.left;
		rtSharedFileTree.right = rcWnd.left;
		RemoveAnchor(m_ctlSharedDirTree);

		CRect rcSharedFileList;
		sharedfilesctrl.GetWindowRect(rcSharedFileList);
		ScreenToClient(rcSharedFileList);
		rcSharedFileList.left = rcWnd.left + 5;
		RemoveAnchor(sharedfilesctrl);

		CRect rcTabInfo;
		m_tabWnd.GetWindowRect(rcTabInfo);
		ScreenToClient(rcTabInfo);
		rcTabInfo.left = rcWnd.left + 5;
		RemoveAnchor(m_tabWnd);

		m_ctlSharedDirTree.MoveWindow(rtSharedFileTree);
		sharedfilesctrl.MoveWindow(rcSharedFileList);
		m_tabWnd.MoveWindow(rcTabInfo);

		AddAnchor(IDC_SHAREDDIRSTREE,TOP_LEFT, BOTTOM_LEFT);
		AddAnchor(sharedfilesctrl.m_hWnd,TOP_LEFT, CSize(100, thePrefs.GetSplitterbarPosition()));
		AddAnchor(m_tabWnd.m_hWnd,CSize(0, thePrefs.GetSplitterbarPosition()), BOTTOM_RIGHT);

		m_wndVSplitter.m_nflag = 0;
	}
	else
	{
		CRect rcWnd;
		GetWindowRect(rcWnd);
		ScreenToClient(rcWnd);

		LONG splitpos = 0;

		if(m_WndVSpliterPos == -1)
		{
			splitpos = SPLITTER_RANGE_MAX - 50;
		}
		else
		{
			splitpos = (m_WndVSpliterPos * rcWnd.Width()) / 100;
		}

		CRect rtSharedFileTree;
		m_ctlSharedDirTree.GetWindowRect(rtSharedFileTree);
		ScreenToClient(rtSharedFileTree);
		rtSharedFileTree.right = splitpos - 5;
		RemoveAnchor(m_ctlSharedDirTree);

		CRect rcSharedFileList;
		sharedfilesctrl.GetWindowRect(rcSharedFileList);
		ScreenToClient(rcSharedFileList);

		if(splitpos < 10)
		{
			rcSharedFileList.left = splitpos + 5;
		}
		else
		{
			rcSharedFileList.left = splitpos;
		}
		
		RemoveAnchor(sharedfilesctrl);

		CRect rcTabInfo;
		m_tabWnd.GetWindowRect(rcTabInfo);
		ScreenToClient(rcTabInfo);

		if(splitpos < 10)
		{
			rcTabInfo.left = splitpos + 5;
		}
		else
		{
			rcTabInfo.left = splitpos;
		}
		
		RemoveAnchor(m_tabWnd);

		m_ctlSharedDirTree.MoveWindow(rtSharedFileTree);
		sharedfilesctrl.MoveWindow(rcSharedFileList);
		m_tabWnd.MoveWindow(rcTabInfo);

		AddAnchor(IDC_SHAREDDIRSTREE,TOP_LEFT, BOTTOM_LEFT);
		AddAnchor(sharedfilesctrl.m_hWnd,TOP_LEFT, CSize(100, thePrefs.GetSplitterbarPosition()));
		AddAnchor(m_tabWnd.m_hWnd,CSize(0, thePrefs.GetSplitterbarPosition()), BOTTOM_RIGHT);

		m_wndVSplitter.m_nflag = 1;
	}
}
