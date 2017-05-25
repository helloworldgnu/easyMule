/* 
 * $Id: SharedFilesWnd.h 7232 2008-09-11 10:39:39Z huby $
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

#pragma once
#include "SharedFilesCtrl.h"
#include "ProgressCtrlX.h"
#include "IconStatic.h"
#include "SharedDirsTreeCtrl.h"
#include "SplitterControl.h"
#include "SplitterControlEx.h"

#include "TabWnd_Cake.h"
#include "UILayer/DetailInfo.h"
#include "UILayer/StatisticsInfo.h"
#include "UILayer/UpLoading.h"
#include "UILayer/UserComment.h"
#include "TbcShare.h"
#include "sharefilescountstatic.h"
#include "ResizableLib\ResizableDialog.h"
#include "SplitterControlEx.h"

class CSharedFilesWnd : public CResizableDialog
{
	DECLARE_DYNAMIC(CSharedFilesWnd)

public:
	CSharedFilesWnd(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSharedFilesWnd();

	void Localize();
	void ShowSelectedFilesSummary();
	void Reload();

// Dialog Data
	enum { IDD = IDD_FILES };

	CSharedFilesCtrl sharedfilesctrl;
	CSharedDirsTreeCtrl m_ctlSharedDirTree;
	CUpLoading       m_UpLoading;

	BOOL	IsRemarkTabActived(){return m_tabIds[TI_REMARK] == m_tabWnd.GetActiveTab();}

private:
	CFont bold;
	
	HICON icon_files;
	CSplitterControlEx m_wndVSplitter;
	CSplitterControlEx m_wndHSplitter;

	CTabWnd_Cake m_tabWnd;
	enum ETabId{TI_DETAIL, TI_REMARK, TI_STAT, TI_UPLOADING, TI_MAX};
	POSITION m_tabIds[TI_MAX];

	CDetailInfo      m_DetailInfo;
	CStatisticsInfo  m_StatisticsInfo;
	CUserComment     m_UserComment;

	UINT m_WndHSpliterPos;
	UINT m_WndVSpliterPos;
	

protected:
	void CreateSplitter();
	void CreateTabWnd();
	void InitCtrlsSize();

	void SetAllIcons();
	void DoVResize(int delta);
	void DoHResize(int delta);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedReloadsharedfiles();
	afx_msg void OnLvnItemActivateSflist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickSflist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSysColorChange();
	afx_msg void OnStnDblclickFilesIco();
	afx_msg void OnTvnSelchangedShareddirstree(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSplitterMoved(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnHSplitterClicked(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnVSplitterClicked(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNM_TabInfo_ActiveTabChanged(NMHDR* pNMHDR, LRESULT *pResult);

public:
	void ShowAllUploadingUsers( );
	LRESULT OnListSelFileChanged(WPARAM wParam, LPARAM lParam);
protected:
	void UpdateSplitterRange(void);
	void ShowList(int nflag);
	void ShowTree(int nflag);
public:
	bool AddNewClient(CUpDownClient *pNewClient);
};
