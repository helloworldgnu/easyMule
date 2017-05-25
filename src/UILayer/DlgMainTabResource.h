/*
 * $Id: DlgMainTabResource.h 9073 2008-12-18 04:38:51Z dgkang $
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


// CDlgMainTabResource 对话框
#include "resource.h"
//#include "ResourceTabWnd.h"
#include "TabWnd.h"
#include "ResizableLib\ResizableDialog.h"
#include "SearchParams.h"
#include "BrowserToolbarCtrl.h"

class CWebBrowserWnd;
class CDlgMainTabResource : public CResizableDialog
{
	DECLARE_DYNAMIC(CDlgMainTabResource)

public:
	CDlgMainTabResource(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgMainTabResource();

// 对话框数据
	enum { IDD = IDD_MAINTAB_RESOURCE };

	CWebBrowserWnd*	OpenNewUrl(LPCTSTR lpszUrl, LPCTSTR lpszCaption, BOOL bSetActive = TRUE, BOOL bClosable = TRUE);
	void    OpenNewSearchResult(LPCTSTR lpszCaption,ESearchType * pSearchType = NULL);
	CTabWnd			m_tabWnd;

	void	UpdateSearchParam(int iIndex, SSearchParams *pSearchParams);
protected:
	enum _TabWndCustomData {TCD_WEB_BROWSER = 1};
	//CResourceTabWnd		m_tabWnd;

	CBrowserToolbarCtrl		m_browserToolbar;

	DWORD           m_dwCounter;
	CMap <int, int, SSearchParams*, SSearchParams*> m_SearchMap;

	void	ShowVerycdPage();
	DWORD		  m_dwTotalCount;

	//{begin} VC-dgkang 2008年7月8日
public:
	void	ShowEmuleSearch();
	void    UpdateEMsClosableStatus();
	BOOL	CreateNewWbTabSearch(WPARAM wParam,LPCTSTR lpszText);
	BOOL	CreateNewShareFileTab(SSearchParams *pSS);
	void	Localize(void);
	CStringList	m_strSearchUrl;
	//{end}


	void	UpdateWbsClosableStatus();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnNMActiveTabChanged(NMHDR* pNMHDR, LRESULT *pResult);
	afx_msg void OnNMTabDestroy(NMHDR* pNMHDR, LRESULT *pResult);
	afx_msg void OnNMTabCreate(NMHDR* pNMHDR, LRESULT *pResult);
	LRESULT OnWbTabDestroy(WPARAM wParam, LPARAM lParam);

	LRESULT OnEMTabDestroy(WPARAM wParam, LPARAM lParam);
	
};
