/*
 * $Id: SearchDlg.h 4483 2008-01-02 09:19:06Z soarchin $
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

struct SSearchParams;
class CSearchResultsWnd;
class CSearchParamsWnd;
class CSearchFile;
class CClosableTabCtrl;


///////////////////////////////////////////////////////////////////////////////
// CSearchDlg frame

class CSearchDlg : public CFrameWnd
{
	DECLARE_DYNCREATE(CSearchDlg)

public:
	CSearchDlg();           // protected constructor used by dynamic creation
	virtual ~CSearchDlg();
	CSearchResultsWnd* m_pwndResults;

	BOOL Create(CWnd* pParent);

	void Localize();
	void CreateMenus();

	void RemoveResult(const CSearchFile* toremove);

	bool DoNewEd2kSearch(SSearchParams* pParams);
	bool DoNewKadSearch(SSearchParams* pParams);
	void CancelEd2kSearch();
	void CancelKadSearch(UINT uSearchID);

	bool CanSearchRelatedFiles() const;
	void SearchRelatedFiles(const CAbstractFile* file);

	void DownloadSelected();
	void DownloadSelected(bool paused);

	bool CanDeleteSearch(uint32 nSearchID) const;
	bool CanDeleteAllSearches() const;
	void DeleteSearch(uint32 nSearchID);
	void DeleteAllSearches();

	void LocalEd2kSearchEnd(UINT count, bool bMoreResultsAvailable);
	void AddGlobalEd2kSearchResults(UINT count);

	bool CreateNewTab(SSearchParams* pParams);
	void ShowSearchSelector(bool visible);
	CClosableTabCtrl& GetSearchSelector();

	int GetSelectedCat();
	void UpdateCatTabs();
	void SaveAllSettings();
	BOOL SaveSearchStrings();
	void ResetHistory();

	void SetToolTipsDelay(UINT uDelay);
	void DeleteAllSearchListCtrlItems();

	BOOL IsSearchParamsWndVisible() const;
	void OpenParametersWnd();
	void DockParametersWnd();

	void UpdateSearch(CSearchFile* pSearchFile);

protected:
	//CSearchParamsWnd* m_pwndParams;

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnClose();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
};
