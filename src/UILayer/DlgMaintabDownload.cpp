/*
 * $Id: DlgMaintabDownload.cpp 9780 2009-01-07 07:58:37Z dgkang $
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
// DlgMaintabDownload.cpp : 实现文件
//

#include "stdafx.h"
#include "DlgMaintabDownload.h"
#include "TabItem_Normal.h"
#include "eMule.h"
#include "eMuleDlg.h"
#include "TransferWnd.h"
#include "UserMsgs.h"
#include "WndMgr.h"
#include "DropDownButton.h"
#include "CmdFuncs.h"
#include "TabItem_Cake.h"
#include "PageTabBkDraw.h"

#include "HttpClient.h"

#include "FileMgr.h"
#include "DownloadedListCtrl.h"

#include "Version.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	SPLITTER_MARGIN		1
// CDlgMaintabDownload 对话框

IMPLEMENT_DYNAMIC(CDlgMaintabDownload, CDialog)
CDlgMaintabDownload::CDlgMaintabDownload(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CDlgMaintabDownload::IDD, pParent)
{
	m_pwebUserComment = 0;
	int	i;
	for (i = 0; i < TI_MAX; i++)
		m_aposTabs[i] = NULL;

	m_plcDownloading = NULL;
	m_dwShowListIDC = 0;
	m_strLastCommentWeb = _T("");
}

CDlgMaintabDownload::~CDlgMaintabDownload()
{
	
}

void CDlgMaintabDownload::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgMaintabDownload, CResizableDialog)
	ON_NOTIFY(UM_SPN_SIZED, IDC_SPLITTER_DOWNLOAD, OnSplitterMoved)
	ON_NOTIFY(UM_SPLITTER_CLICKED, IDC_SPLITTER_DOWNLOAD, OnSplitterClicked)
	ON_WM_DESTROY()
	ON_MESSAGE(UM_MTDD_CUR_SEL_FILE, OnCurSelFile)
	ON_MESSAGE(UM_MTDD_CUR_SEL_FILE_TASK,OnCurSelFileTask)
	ON_MESSAGE(UM_MTDD_CUR_SEL_PEER, OnCurSelPeer)
	ON_NOTIFY(NMC_TW_ACTIVE_TAB_CHANDED, IDC_TAB_INFO, OnNM_TabInfo_ActiveTabChanged)
	ON_NOTIFY(NMC_TW_ACTIVE_TAB_CHANDED, IDC_TAB_LIST, OnNM_TabList_ActiveTabChanged)
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////
// Added by thilon on 2007.02.03, for Resize
//BEGIN_ANCHOR_MAP(CDlgMaintabDownload)
//	ANCHOR_MAP_ENTRY(m_tabwndDlList.GetSafeHwnd(), ANF_TOPLEFT | ANF_BOTTOMRIGHT)
//	ANCHOR_MAP_ENTRY(m_wndSplitter.GetSafeHwnd(), ANF_AUTOMATIC)
//	ANCHOR_MAP_ENTRY(m_tabwndInfo.GetSafeHwnd(),ANF_AUTOMATIC)
//END_ANCHOR_MAP()

CKnownFile*	CDlgMaintabDownload::GetCurrentSelectedFile( CFileTaskItem* &pFileTask )
{
	POSITION	pos;
	int	iSelectedItem = -1;
	CListCtrl	*pListCtrl = NULL;
	DWORD_PTR	dwItemData;
	
	if (m_DownloadTabWnd.GetActiveTab() == m_aposTabs[TI_DOWNLOADING])
		pListCtrl = m_plcDownloading;
	else if (m_DownloadTabWnd.GetActiveTab() == m_aposTabs[TI_DOWNLOADED])
		pListCtrl = &m_lcDownloaded;
	else
		return NULL;

	if (NULL == pListCtrl)
		return NULL;


	pos = pListCtrl->GetFirstSelectedItemPosition();
	if (pos == NULL)
		return NULL;

	iSelectedItem = pListCtrl->GetNextSelectedItem(pos);
	dwItemData = pListCtrl->GetItemData(iSelectedItem);

	if (m_DownloadTabWnd.GetActiveTab() == m_aposTabs[TI_DOWNLOADING])
	{
		CtrlItem_Struct	*pcis = (CtrlItem_Struct*) dwItemData;
		if (FILE_TYPE == pcis->type)
			return (CKnownFile*)pcis->value;
		else
			return NULL;
	}
	else if (m_DownloadTabWnd.GetActiveTab() == m_aposTabs[TI_DOWNLOADED])
	{  
		ItemData *pItemData = (ItemData*)dwItemData;
		if( FILE_TYPE==pItemData->type )
		{
			CKnownFile* pmyfile =(CKnownFile *)pItemData->pItem;
			return pmyfile;
		}
		else if( FILE_TASK==pItemData->type ) 
		{ 
           pFileTask = (CFileTaskItem*)pItemData->pItem;
		}
		return NULL;
	}
	else
		return NULL;
}


void CDlgMaintabDownload::InitTabs(void)
{
	CClientRect	rtClient(this);
	CRect	rtDownload;

	GetDlgItem(IDC_TAB_LIST)->GetWindowRect(&rtDownload);
	ScreenToClient(&rtDownload);

	m_DownloadTabWnd.Create(WS_CHILD | WS_VISIBLE, rtDownload, this, IDC_TAB_LIST);
	//m_tabwndDlList.SetBkColor(GetSysColor(COLOR_3DFACE), FALSE);
	CPageTabBkDraw	*pBarBkDraw = new CPageTabBkDraw;
	m_DownloadTabWnd.SetBarBkDraw(pBarBkDraw);

	CRect	rcSpl;
	GetDlgItem(IDC_STATIC_SPLITER)->GetWindowRect(&rcSpl);
	ScreenToClient(&rcSpl);
	rcSpl.top = rtDownload.bottom;
	rcSpl.bottom = rcSpl.top;
	m_wndSplitter.Create(WS_CHILD | WS_VISIBLE, rcSpl, this, IDC_SPLITTER_DOWNLOAD);
	m_wndSplitter.SetNormalDraw(FALSE);


	CRect	rtTabInfo;
	GetDlgItem(IDC_TAB_INFO)->GetWindowRect(&rtTabInfo);
	ScreenToClient(&rtTabInfo);

	rtTabInfo.top = rcSpl.top + m_wndSplitter.GetHBreadth();
	m_tabwndInfo.Create(WS_CHILD | WS_VISIBLE, rtTabInfo, this, IDC_TAB_INFO);

	m_lcDownloaded.Create(WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_OWNERDRAWFIXED, CRect(0, 0, 0, 0), this, IDC_DOWNLOADED_LISTCTRL);


	m_plcDownloading = (CListCtrl*) theApp.emuledlg->transferwnd->GetDlgItem(IDC_DOWNLOADLIST);
	m_aposTabs[TI_DOWNLOADING] = CmdFuncs::TabWnd_AddNormalTab(&m_DownloadTabWnd, GetResString(IDS_DLTAB_DOWNLOADING), m_plcDownloading->GetSafeHwnd());
	
	m_aposTabs[TI_DOWNLOADED] = CmdFuncs::TabWnd_AddNormalTab(&m_DownloadTabWnd, GetResString(IDS_DLTAB_COMPLETED), m_lcDownloaded.GetSafeHwnd());


	CTabItem_Cake	*pTiCake = NULL;

	if(! m_dlgDetailInfo.GetSafeHwnd())
	{
		m_dlgDetailInfo.Create(m_dlgDetailInfo.IDD, this);
	}

	//添加日志页
	if(!m_dlgPeerLog.GetSafeHwnd())
	{
		m_dlgPeerLog.Create(m_dlgPeerLog.IDD, this);
	}

	//m_aposTabs[TI_DETAIL] = CmdFuncs::TabWnd_AddNormalTab(&m_tabwndInfo, GetResString(IDS_DETAIL_INFO), m_dlgDetailInfo.GetSafeHwnd());
	pTiCake = new CTabItem_Cake;
	pTiCake->SetCaption(GetResString(IDS_DETAIL_INFO));
	pTiCake->SetRelativeWnd(m_dlgDetailInfo.GetSafeHwnd());
	pTiCake->SetIcon(_T("PNG_DETAILINFO"));
	m_posInfo = m_aposTabs[TI_DETAIL] = m_tabwndInfo.AddTab(pTiCake);
	pTiCake = NULL;

	//VC-dgkang
#ifndef _FOREIGN_VERSION 
	if(! m_pwebUserComment)
	{
		//  don't delete it, auto-deleted
		m_pwebUserComment = new CHtmlCtrl;
		CRect rect(0,0,1,1);  //  tab will resize it
		m_pwebUserComment->Create(NULL, NULL ,WS_CHILD | WS_VISIBLE &~WS_BORDER, rect,this, 34345,NULL);
		m_pwebUserComment->SetSilent(true);
		//m_webUserComment.Create(IDD_WEBBROWSER);
	}

	pTiCake = new CTabItem_Cake;
	pTiCake->SetCaption(GetResString(IDS_USER_COMMENT));
	pTiCake->SetRelativeWnd(m_pwebUserComment->GetSafeHwnd());
	pTiCake->SetIcon(_T("PNG_DETAILCOMMENT"));
	m_aposTabs[TI_REMARK] = m_tabwndInfo.AddTab(pTiCake);
	pTiCake = NULL;
#endif
	//添加日志标签
	pTiCake = new CTabItem_Cake;
	pTiCake->SetCaption(GetResString(IDS_TASK_LOG));
	pTiCake->SetRelativeWnd(m_dlgPeerLog.GetSafeHwnd());
	pTiCake->SetIcon(_T("PNG_PEERLOG"));
	m_posPeerLog = m_aposTabs[TI_PEERLOG] = m_tabwndInfo.AddTab(pTiCake);
	pTiCake = NULL;

	//添加上传标签
	pTiCake = new CTabItem_Cake;
	pTiCake->SetCaption(GetResString(IDS_UPLOADING));
	pTiCake->SetRelativeWnd(((CListCtrl*) theApp.emuledlg->transferwnd->GetDlgItem(IDC_UPLOADLIST))->GetSafeHwnd());
	pTiCake->SetIcon(_T("PNG_DETAILUPLOAD"));
	m_posUploading = m_aposTabs[TI_UPLOADING] = m_tabwndInfo.AddTab(pTiCake);
	pTiCake = NULL;

}

void CDlgMaintabDownload::ShowUpingOrQueue(UINT iDlgItem)
{
	CTabItem_Cake	*pTiCake = NULL;
	pTiCake = (CTabItem_Cake*)m_tabwndInfo.GetTabItem(m_posUploading);

	pTiCake->SetRelativeWnd(((CListCtrl*)theApp.emuledlg->transferwnd->GetDlgItem(iDlgItem))->GetSafeHwnd());
}

void CDlgMaintabDownload::RefreshLowerPannel(CKnownFile * file)
{
	//[10/17/2007 VC-huby] 和void CUserComment::Refresh(CKnownFile * file) 重复代码，由于UI层没有规划好,暂复制一份..
	if( !m_pwebUserComment || !file )
		return;

	CString strFileEd2k = CreateED2kLink(file, false);
	if( strFileEd2k.IsEmpty() )
	{
	//	m_pwebUserComment->Navigate2(_T("about:blank"), 0, NULL);
		theApp.emuledlg->transferwnd->downloadlistctrl.OnNoComment(m_pwebUserComment);
		return;
	}

	bool bFileisFinished = true;
	if( file->IsKindOf(RUNTIME_CLASS(CPartFile)) )
	{
		if( ((CPartFile*)file)->GetStatus()!=PS_COMPLETE )
			bFileisFinished = false;
	}

	CString strCommentUrl = bFileisFinished ? thePrefs.m_strFinishedFileCommentUrl : thePrefs.m_strPartFileCommentUrl;
	strCommentUrl.Replace(_T("[ed2k]"),strFileEd2k);
	strCommentUrl.Replace(_T("|"), _T("%7C"));

	CString sVersion;
	sVersion.Format(_T("&v=%u"),VC_VERSION_BUILD);
	strCommentUrl += sVersion;

	if (m_strLastCommentWeb != strCommentUrl)
	{
		m_pwebUserComment->Navigate2(strCommentUrl, 0, NULL);
		m_strLastCommentWeb = strCommentUrl;
	}
}

void CDlgMaintabDownload::RefreshLowerPannel(CFileTaskItem * pFileTask)
{
	if( !m_pwebUserComment || !pFileTask )
		return;
	if( pFileTask->m_strEd2kLink.IsEmpty() || pFileTask->m_strEd2kLink.Left(7)!=_T("ed2k://") )
	{
		theApp.emuledlg->transferwnd->downloadlistctrl.OnNoComment(m_pwebUserComment);	
	}
	else
	{
		bool bFileisFinished = (pFileTask->m_nFileState>=2 && pFileTask->m_nFileState<=6);
		CString strCommentUrl = bFileisFinished ? thePrefs.m_strFinishedFileCommentUrl : thePrefs.m_strPartFileCommentUrl;
		strCommentUrl.Replace(_T("[ed2k]"),pFileTask->m_strEd2kLink);
		strCommentUrl.Replace(_T("|"), _T("%7C"));

		CString sVersion;
		sVersion.Format(_T("&v=%u"),VC_VERSION_BUILD);
		strCommentUrl += sVersion;

		if (m_strLastCommentWeb != strCommentUrl)
		{
			m_pwebUserComment->Navigate2(strCommentUrl, 0, NULL);
			m_strLastCommentWeb = strCommentUrl;
		}		
	}
}

// CDlgMaintabDownload 消息处理程序

BOOL CDlgMaintabDownload::OnInitDialog()
{
	CResizableDialog::OnInitDialog();

	ModifyStyle(0, WS_CLIPCHILDREN);

	theWndMgr.SetWndHandle(CWndMgr::WI_MAINTAB_DOWNLOAD_DLG, m_hWnd);

	InitTabs();

	m_wndSplitter.m_nflag = 1;

	CRect rcWnd;
	GetWindowRect(rcWnd);
	ScreenToClient(rcWnd);

	InitControlContainer();

	LONG splitpos = thePrefs.GetSplitterbarPositionDownload() * rcWnd.Height() / 100;

	CRect rcDlgItem;
	m_DownloadTabWnd.GetWindowRect(rcDlgItem);
	ScreenToClient(rcDlgItem);
	rcDlgItem.bottom = splitpos;
	m_DownloadTabWnd.MoveWindow(rcDlgItem);

	m_tabwndInfo.GetWindowRect(rcDlgItem);
	ScreenToClient(rcDlgItem);
	rcDlgItem.top = splitpos + m_wndSplitter.GetHBreadth();
	m_tabwndInfo.MoveWindow(rcDlgItem);

	AddAnchor(m_DownloadTabWnd, TOP_LEFT, CSize(100, thePrefs.GetSplitterbarPositionDownload()));
	AddAnchor(m_wndSplitter, CSize(0, thePrefs.GetSplitterbarPositionDownload()), BOTTOM_RIGHT);
	AddAnchor(m_tabwndInfo, CSize(0, thePrefs.GetSplitterbarPositionDownload()), BOTTOM_RIGHT);

	return TRUE; 
}

BOOL CDlgMaintabDownload::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		// Don't handle Ctrl+Tab in this window. It will be handled by main window.
		if (pMsg->wParam == VK_TAB && GetAsyncKeyState(VK_CONTROL) < 0)
			return FALSE;

		if (VK_RETURN == pMsg->wParam || VK_ESCAPE == pMsg->wParam)
			return FALSE;
	}

	if (pMsg->message == WM_MBUTTONUP)
	{
		if (m_plcDownloading)
		{
			((CDownloadListCtrl*)m_plcDownloading)->ShowSelectedFileDetails();
			return TRUE;
		}
	}

	return CResizableDialog::PreTranslateMessage(pMsg);
}

void CDlgMaintabDownload::DoResize(int delta)
{
	CSplitterControl::ChangeHeight((CWnd*)&m_DownloadTabWnd, delta);
	CSplitterControl::ChangeHeight((CWnd*)&m_tabwndInfo, -delta, CW_BOTTOMALIGN);

	UpdateSplitterRange();

	m_plcDownloading->Invalidate();
	m_plcDownloading->UpdateWindow();
}

LRESULT CDlgMaintabDownload::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_PAINT:
			if (m_wndSplitter)
			{
				CRect rcWnd;
				GetWindowRect(rcWnd);

				if (rcWnd.Height() > 0)
				{
					CRect rcDown;
					m_DownloadTabWnd.GetWindowRect(rcDown);
					ScreenToClient(rcDown);

					// splitter paint update
					CRect rcSpl;
					rcSpl.left = rcDown.left;
					rcSpl.right = rcDown.right;
					rcSpl.top = rcDown.bottom;
					rcSpl.bottom = rcSpl.top + m_wndSplitter.GetHBreadth();
					m_wndSplitter.MoveWindow(rcSpl, TRUE);

					UpdateSplitterRange();
				}
			}

			//// Workaround to solve a glitch with WM_SETTINGCHANGE message
			//if (m_btnWnd1 && m_btnWnd1->m_hWnd && m_btnWnd1->GetBtnWidth(IDC_DOWNLOAD_ICO) != WND1_BUTTON_WIDTH)
			//	m_btnWnd1->SetBtnWidth(IDC_DOWNLOAD_ICO, WND1_BUTTON_WIDTH);

			break;
	}

	return CResizableDialog::DefWindowProc(message, wParam, lParam);
}

void CDlgMaintabDownload::OnDestroy()
{
	CResizableDialog::OnDestroy();

	// TODO: 在此处添加消息处理程序代码
	theWndMgr.SetWndHandle(CWndMgr::WI_MAINTAB_DOWNLOAD_DLG, NULL);
}

LRESULT	CDlgMaintabDownload::OnCurSelFile(WPARAM wParam, LPARAM lParam)
{	
	if (!lParam)
	{
		theApp.emuledlg->m_mainTabWnd.m_dlgDownload.m_dlgPeerLog.m_LogListCtrl.DeleteAllItems();
		theApp.emuledlg->m_mainTabWnd.m_dlgDownload.m_dlgDetailInfo.m_ListDetail.DeleteAllItems();
		return 0;
	}

	if (wParam == 1)
	{
		CKnownFile* pKnownFile= (CKnownFile*)lParam;
		if( pKnownFile->HasNullHash() )
		{
			CFileTaskItem* pFileTaskItem = CGlobalVariable::filemgr.GetFileTaskItem(pKnownFile->GetPartFileURL());
			ASSERT(pFileTaskItem);
			if(pFileTaskItem) 
				m_dlgDetailInfo.FileInfo(pFileTaskItem);
		}
		else
		{
			m_dlgDetailInfo.UpdateInfo((CPartFile*)lParam, CDetailInfo::IM_COMBINE_DOWNLOADED);
		}

		return 0;
	}

	CPartFile *pFile = (CPartFile*) lParam;
	if (IsRemarkTabActived())
	{
		RefreshLowerPannel(pFile);
	}
	else /*if( IsLogTabActived() )*/
	{
		m_dlgPeerLog.m_LogListCtrl.ShowSelectedFileLogs(pFile);
	}

	//MODIFIED by VC-fengwen on 2008/03/17 <begin> : 无论是否处于当前页面都更新DetailInfo（为了方便处理，而且此处性能消耗不大）
	if ( IsDownloadingActived() && pFile->GetStatus() != PS_COMPLETE && pFile->GetFileSize() != (uint64)0)
	{
		m_dlgDetailInfo.UpdateInfo(pFile, CDetailInfo::IM_COMBINE_DOWNLOAD);
	}
	else if (IsDownloadingActived() && pFile->GetFileSize() == (uint64)0)
	{   
		CFileTaskItem* pFileTaskItem = CGlobalVariable::filemgr.GetFileTaskItem(pFile->GetPartFileURL());
		ASSERT(pFileTaskItem);
		if(pFileTaskItem) 
			m_dlgDetailInfo.FileInfo(pFileTaskItem);
	}
	else
	{
		m_dlgDetailInfo.UpdateInfo(pFile, CDetailInfo::IM_COMBINE_DOWNLOADED);
	}
	//MODIFIED by VC-fengwen on 2008/03/17 <end> : 无论是否处于当前页面都更新DetailInfo（为了方便处理，而且此处性能消耗不大）
		
	return 0;
}


LRESULT	CDlgMaintabDownload::OnCurSelFileTask(WPARAM /*wParam*/, LPARAM lParam)
{
	//CKnownFile	*pFile = (CKnownFile*) lParam;
	 CFileTaskItem *pFile =(CFileTaskItem *)lParam;
	 m_dlgDetailInfo.FileInfo(pFile);
	 return 0;
}

LRESULT	CDlgMaintabDownload::OnCurSelPeer(WPARAM /*wParam*/, LPARAM lParam)
{
	m_tabwndInfo.SetActiveTab(m_posPeerLog);

	CtrlItem_Struct* content = (CtrlItem_Struct*) lParam;

	if(content == NULL)
	{
		return 0;
	}

	CUpDownClient*  pUpDownClient = (CUpDownClient*)content->value;

	if( pUpDownClient == NULL)
	{
		return 0;
	}

	m_dlgPeerLog.m_LogListCtrl.ShowSelectedPeerLogs(pUpDownClient);

	return 0;
}

void CDlgMaintabDownload::OnNM_TabInfo_ActiveTabChanged(NMHDR* pNMHDR, LRESULT *pResult)
{
	__try
	{	
		NMTW_TABOP *pTabOp = reinterpret_cast<NMTW_TABOP*>(pNMHDR);

		CKnownFile*	pFile = NULL;
		if (pTabOp->posTab == m_aposTabs[TI_REMARK])
		{
			CFileTaskItem *pFileTask=NULL;
			pFile = GetCurrentSelectedFile(pFileTask);
			if ( NULL!= pFile )
				RefreshLowerPannel(pFile);
			else if( NULL!=pFileTask )
				RefreshLowerPannel(pFileTask);				
		}

		*pResult = 0;
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
	}
}

void CDlgMaintabDownload::OnNM_TabList_ActiveTabChanged(NMHDR* /*pNMHDR*/, LRESULT *pResult)
{	
	if (m_DownloadTabWnd.GetActiveTab() == m_aposTabs[TI_DOWNLOADING])
	{
		m_DownloadTabWnd.m_Toolbar.SetOwner(m_plcDownloading);
		m_DownloadTabWnd.m_Toolbar.EnableButton(MP_OPENFOLDER, ( FALSE ));

		m_tabwndInfo.SetActiveTab(m_posInfo);
		m_dlgDetailInfo.m_ListDetail.DeleteAllItems();

		theApp.emuledlg->transferwnd->downloadlistctrl.UpdateToolBarItem();
	}
		
	if (m_DownloadTabWnd.GetActiveTab() == m_aposTabs[TI_DOWNLOADED])
	{
		theApp.emuledlg->m_mainTabWnd.m_dlgDownload.m_dlgPeerLog.m_LogListCtrl.DeleteAllItems();
		m_DownloadTabWnd.m_Toolbar.SetOwner( &m_lcDownloaded);

		
		m_tabwndInfo.SetActiveTab(m_posInfo);
		m_dlgDetailInfo.m_ListDetail.DeleteAllItems();

		m_DownloadTabWnd.m_Toolbar.EnableButton(MP_PAUSE, FALSE);
		m_DownloadTabWnd.m_Toolbar.EnableButton(MP_RESUME, FALSE);
		m_DownloadTabWnd.m_Toolbar.EnableButton(MP_STOP, FALSE);

		//Toolbar
		int iSelectedItems = m_lcDownloaded.GetSelectedCount();

		if(iSelectedItems)
		{
			m_DownloadTabWnd.m_Toolbar.EnableButton(MP_CANCEL, TRUE);
			if (iSelectedItems == 1)
			{
				m_DownloadTabWnd.m_Toolbar.EnableButton(MP_OPENFOLDER, TRUE);
			}
			else
			{
				m_DownloadTabWnd.m_Toolbar.EnableButton(MP_OPENFOLDER, FALSE);
			}
		}
		else
		{	
			m_DownloadTabWnd.m_Toolbar.EnableButton(MP_OPENFOLDER, FALSE);
			m_DownloadTabWnd.m_Toolbar.EnableButton(MP_CANCEL, FALSE);

			m_dlgDetailInfo.m_ListDetail.DeleteAllItems();
		}
	}

	*pResult = 0;
}

void CDlgMaintabDownload::Localize()
{
	m_DownloadTabWnd.SetTabText(m_aposTabs[TI_DOWNLOADING], GetResString(IDS_DLTAB_DOWNLOADING));
	m_DownloadTabWnd.SetTabText(m_aposTabs[TI_DOWNLOADED], GetResString(IDS_DLTAB_COMPLETED));

	m_tabwndInfo.SetTabText(m_aposTabs[TI_DETAIL], GetResString(IDS_DETAIL_INFO));
	m_tabwndInfo.SetTabText(m_aposTabs[TI_REMARK], GetResString(IDS_USER_COMMENT));
}

void CDlgMaintabDownload::ShowList(int nflag)
{
	
	if(nflag)
	{
		CRect rcWnd;
		GetWindowRect(rcWnd);
		ScreenToClient(rcWnd);

		CRect rcTabList;
		m_DownloadTabWnd.GetWindowRect(rcTabList);
		ScreenToClient(rcTabList);
		pos = (rcTabList.bottom * 100) / rcWnd.Height();
		rcTabList.bottom = rcWnd.bottom - 5;// - 30;
		//rcTabList.top = 28;
		
		RemoveAnchor(m_DownloadTabWnd);

		CRect rcTabInfo;
		m_tabwndInfo.GetWindowRect(rcTabInfo);
		ScreenToClient(rcTabInfo);
		rcTabInfo.bottom = rcWnd.bottom;// - 20;
		rcTabInfo.top = rcWnd.bottom;// - 23;
		RemoveAnchor(m_tabwndInfo);

		m_DownloadTabWnd.MoveWindow(rcTabList);
		m_tabwndInfo.MoveWindow(rcTabInfo);

		AddAnchor(m_DownloadTabWnd, TOP_LEFT, BOTTOM_RIGHT);
		AddAnchor(m_tabwndInfo, TOP_LEFT, BOTTOM_RIGHT);

		m_wndSplitter.m_nflag = 0;
	}
	else
	{
		CRect rcWnd;
		GetWindowRect(rcWnd);
		ScreenToClient(rcWnd);

		LONG splitpos = (pos * rcWnd.Height()) / 100;

		CRect rcTabList;
		m_DownloadTabWnd.GetWindowRect(rcTabList);
		ScreenToClient(rcTabList);
		rcTabList.bottom = splitpos - 5;
		RemoveAnchor(m_DownloadTabWnd);

		CRect rcTabInfo;
		m_tabwndInfo.GetWindowRect(rcTabInfo);
		ScreenToClient(rcTabInfo);
		rcTabInfo.top = splitpos;
		RemoveAnchor(m_tabwndInfo);

		m_DownloadTabWnd.MoveWindow(rcTabList);
		m_tabwndInfo.MoveWindow(rcTabInfo);

		AddAnchor(m_DownloadTabWnd, TOP_LEFT, CSize(100, thePrefs.GetSplitterbarPositionDownload()));
		AddAnchor(m_tabwndInfo, CSize(0, thePrefs.GetSplitterbarPositionDownload()), BOTTOM_RIGHT);

		m_wndSplitter.m_nflag = 1;
	}
	
}

void CDlgMaintabDownload::UpdateSplitterRange(void)
{
	CRect rcWnd;
	GetWindowRect(rcWnd);

	CRect rcDown;
	m_DownloadTabWnd.GetWindowRect(rcDown);
	ScreenToClient(rcDown);

	CRect rcUp;
	m_tabwndInfo.GetWindowRect(rcUp);
	ScreenToClient(rcUp);

	
	thePrefs.SetSplitterbarPositionDownload((rcDown.bottom * 100) / rcWnd.Height());

	RemoveAnchor(m_DownloadTabWnd);
	RemoveAnchor(m_tabwndInfo);


	AddAnchor(m_DownloadTabWnd, TOP_LEFT, CSize(100, thePrefs.GetSplitterbarPositionDownload()));
	AddAnchor(m_tabwndInfo, CSize(0, thePrefs.GetSplitterbarPositionDownload()), BOTTOM_RIGHT);

	m_wndSplitter.SetRange(rcDown.top + 50, rcUp.bottom - 40);
}

void CDlgMaintabDownload::OnSplitterMoved(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	SPC_NMHDR* pHdr = (SPC_NMHDR*)pNMHDR;
	DoResize(pHdr->delta);
}

void CDlgMaintabDownload::OnSplitterClicked(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	SPCEX_NMHDR* pHdr = (SPCEX_NMHDR*)pNMHDR;
	theApp.emuledlg->m_mainTabWnd.m_dlgDownload.ShowList(pHdr->flag);
}
