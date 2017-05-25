/* 
 * $Id: SearchResultsWnd.h 4483 2008-01-02 09:19:06Z soarchin $
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#pragma once
#include "ResizableLib\ResizableFormView.h"
#include "SearchListCtrl.h"
#include "ClosableTabCtrl.h"
#include "IconStatic.h"
#include "EditX.h"
#include "EditDelayed.h"
#include "ComboBoxEx2.h"
#include "ListCtrlEditable.h"
#include "TabItemAffector.h"
#include "afxwin.h"

class CCustomAutoComplete;
class Packet;
class CSafeMemFile;
class CSearchParamsWnd;
struct SSearchParams;
class CDropDownButton;


///////////////////////////////////////////////////////////////////////////////
// CSearchResultsSelector

class CSearchResultsSelector : public CClosableTabCtrl
{
public:
	CSearchResultsSelector(){}

protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
};

///////////////////////////////////////////////////////////////////////////////
// CSearchResultsWnd dialog

class CSearchResultsWnd : public CResizableFormView/*, public CTabItemAffector*/
{
	DECLARE_DYNCREATE(CSearchResultsWnd)

public:
	CSearchResultsWnd(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSearchResultsWnd();
	
	enum { IDD = IDD_SEARCH };

	CSearchListCtrl searchlistctrl;
	CSearchResultsSelector searchselect;
	CStringArray m_astrFilter;

	void	Localize();

	//MODIFIED by VC-fengwen on 2007/09/11 <begin> : 返回值标识pParams是否被删除了。
		//void	StartSearch(SSearchParams* pParams);
	bool	StartSearch(SSearchParams* pParams);
	//MODIFIED by VC-fengwen on 2007/09/11 <end> : 返回值标识pParams是否被删除了。
	bool	SearchMore();
	void	CancelSearch(UINT uSearchID = 0);

	bool	DoNewEd2kSearch(SSearchParams* pParams);
	void	CancelEd2kSearch();
	bool	IsLocalEd2kSearchRunning() const { return (m_uTimerLocalServer != 0); }
	bool	IsGlobalEd2kSearchRunning() const { return (global_search_timer != 0); }
	void	LocalEd2kSearchEnd(UINT count, bool bMoreResultsAvailable);
	void	AddGlobalEd2kSearchResults(UINT count);

	bool	DoNewKadSearch(SSearchParams* pParams);
	void	CancelKadSearch(UINT uSearchID);

	bool	CanSearchRelatedFiles() const;
	void	SearchRelatedFiles(const CAbstractFile* file);

	void	DownloadSelected();
	void	DownloadSelected(bool bPaused);

	bool	CanDeleteSearch(uint32 nSearchID) const;
	bool	CanDeleteAllSearches() const;
	void	DeleteSearch(uint32 nSearchID);
	void	DeleteAllSearches();

	bool	CreateNewTab(SSearchParams* pParams);
	void	ShowSearchSelector(bool visible);
	int		GetSelectedCat() { return m_cattabs.GetCurSel(); }
	void	UpdateCatTabs();
    uint32	GetFilterColumn() const				{ return m_nFilterColumn; }

	void	UpdateParamDisplay(SSearchParams* pParams);
protected:
	Packet*		searchpacket;
	bool		m_b64BitSearchPacket;
	UINT_PTR	global_search_timer;
	UINT		m_uTimerLocalServer;
	CProgressCtrl searchprogress;
	CEditDelayed m_ctlFilter;
	bool		canceld;
	uint16		servercount;
	bool		globsearch;
	uint32		m_nEd2kSearchID;
	CImageList	m_imlSearchResults;
	CTabCtrl	m_cattabs;
	CDropDownButton* m_btnSearchListMenu;
	HICON		icon_search;
	int			m_iSentMoreReq;
	uint32		m_nFilterColumn;

	bool StartNewSearch(SSearchParams* pParams);
	void SearchStarted();
	void SearchCanceled(UINT uSearchID);
	CString	CreateWebQuery(SSearchParams* pParams);
	void ShowResults(const SSearchParams* pParams);
	void SetAllIcons();
	void SetSearchResultsIcon(UINT uSearchID, int iImage);
	void SetActiveSearchResultsIcon(UINT uSearchID);
	void SetInactiveSearchResultsIcon(UINT uSearchID);
	SSearchParams* GetSearchResultsParams(UINT uSearchID) const;

	virtual void OnInitialUpdate();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	afx_msg void OnDblClkSearchList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSelChangeTab(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg LRESULT OnCloseTab(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDblClickTab(WPARAM wParam, LPARAM lParam);
	afx_msg void OnDestroy();
	afx_msg void OnSysColorChange();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnBnClickedDownloadSelected();
	afx_msg void OnBnClickedClearAll();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedOpenParamsWnd();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg LRESULT OnChangeFilter(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSearchListMenuBtnDropDown(NMHDR* pNMHDR, LRESULT* pResult);
	
public:
	CEdit m_SearchCtrl;
	CButton m_BtnSearch;
	afx_msg void OnBnClickedButtonSearch();
protected:
	SSearchParams* GetParameters(CString expression);
public:
	enum ESearchArea
	{
		//NOTE: The numbers are *equal* to the entries in the comboxbox -> TODO: use item data
		SearchAreaEd2kGlobal = 0,
		SearchAreaKademlia,
	};

	ESearchType SearchAreaToType(ESearchArea eArea);
	ESearchArea SearchTypeToArea(ESearchType eType);

	CComboBoxEx2 m_ctlMethod;
	CImageList m_imlSearchMethods;

	int m_iCurSearchIndexInRes;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void SetPos(int pos);
};
