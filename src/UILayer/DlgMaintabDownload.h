/*
 * $Id: DlgMaintabDownload.h 8435 2008-11-24 08:52:24Z huby $
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
#pragma once

// CDlgMaintabDownload 对话框
#include "resource.h"
#include "DownloadTabCtrl.h"
#include "SplitterControlEx.h"
#include "DownloadedListCtrl.h"
#include "WebBrowserWnd.h"
#include "DetailInfo.h"
#include "Localizee.h"
#include "ResizableLib\ResizableDialog.h"
#include "TabWnd_Cake.h"
#include "PeerLog.h"
#include "UILayer/UpLoading.h"

class CDlgMaintabDownload : public CResizableDialog, public CLocalizee
{
	DECLARE_DYNAMIC(CDlgMaintabDownload)
	LOCALIZEE_WND_CANLOCALIZE()
public:
	CDlgMaintabDownload(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgMaintabDownload();

// 对话框数据
	enum { IDD = IDD_MAINTAB_DOWNLOAD };

public:
	enum ETabId
	{
		TI_DOWNLOADING,
		TI_DOWNLOADED,
		
		TI_DETAIL,
		TI_REMARK,
		TI_PEERLOG,
		TI_UPLOADING,
		TI_MAX,
	};

	void	SetDownloadlistActiveTab(ETabId eTabId){m_DownloadTabWnd.SetActiveTab(m_aposTabs[eTabId]);}
	BOOL	IsLogTabActived(){return m_aposTabs[TI_PEERLOG] == m_tabwndInfo.GetActiveTab();}
	BOOL	IsRemarkTabActived(){return m_aposTabs[TI_REMARK] == m_tabwndInfo.GetActiveTab();}
	BOOL	IsDownloadingActived(){return m_aposTabs[TI_DOWNLOADING] == m_DownloadTabWnd.GetActiveTab();}
	CKnownFile*	GetCurrentSelectedFile( CFileTaskItem* &pFileTask );
public:
		CDownloadTabWnd		m_DownloadTabWnd;
		CPeerLog				m_dlgPeerLog;
		CDownloadedListCtrl		m_lcDownloaded;
        CDetailInfo				m_dlgDetailInfo;
protected:
	void	InitTabs(void);


	CTabWnd_Cake			m_tabwndInfo;
	CHtmlCtrl				* m_pwebUserComment;

	// VC-dgkang 2008年9月5日
	CString					m_strLastCommentWeb;

	POSITION	m_aposTabs[TI_MAX];
	UINT pos;


	CListCtrl*				m_plcDownloading;
	
	CSplitterControlEx		m_wndSplitter;
	uint32		m_dwShowListIDC;	

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
    void RefreshLowerPannel(CKnownFile * file);
	void RefreshLowerPannel(CFileTaskItem * pFileTask);
protected:
	void DoResize(int delta);
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);

    afx_msg void OnSplitterMoved(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSplitterClicked(NMHDR* pNMHDR, LRESULT* pResult);
public:
	afx_msg void OnDestroy();
	afx_msg LRESULT	OnCurSelFile(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnCurSelFileTask(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnCurSelPeer(WPARAM wParam, LPARAM lParam);
	afx_msg void OnNM_TabInfo_ActiveTabChanged(NMHDR* pNMHDR, LRESULT *pResult);
	afx_msg void OnNM_TabList_ActiveTabChanged(NMHDR* pNMHDR, LRESULT *pResult);
	void Localize(void);
	void ShowList(int nflag);

	void ShowUpingOrQueue(UINT iDlgItem);

protected:
	POSITION m_posPeerLog;
	POSITION m_posInfo;
	POSITION m_posUploading;

public:
	void UpdateSplitterRange(void);
};
