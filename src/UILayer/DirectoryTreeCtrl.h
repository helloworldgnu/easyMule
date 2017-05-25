/*
 * $Id: DirectoryTreeCtrl.h 4483 2008-01-02 09:19:06Z soarchin $
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
/////////////////////////////////////////////
// written by robert rostek - tecxx@rrs.at //
/////////////////////////////////////////////

#define MP_SHAREDFOLDERS_FIRST	46901

class CDirectoryTreeCtrl : public CTreeCtrl
{
	DECLARE_DYNAMIC(CDirectoryTreeCtrl)

public:
	// initialize control
	void Init(void);
	// get all shared directories
	void GetSharedDirectories(CStringList* list);
	// set shared directories
	void SetSharedDirectories(CStringList* list);

private:
	CImageList m_image; 
	// add a new item
	HTREEITEM AddChildItem(HTREEITEM hRoot, CString strText);
	// add subdirectory items
	void AddSubdirectories(HTREEITEM hRoot, CString strDir);
	// return the full path of an item (like C:\abc\somewhere\inheaven\)
	CString GetFullPath(HTREEITEM hItem);
	// returns true if strDir has at least one subdirectory
	bool HasSubdirectories(CString strDir);
	// check status of an item has changed
	void CheckChanged(HTREEITEM hItem, bool bChecked);
	// returns true if a subdirectory of strDir is shared
	bool HasSharedSubdirectory(CString strDir);
	// when sharing a directory, make all parent directories bold
	void UpdateParentItems(HTREEITEM hChild);
	void ShareSubDirTree(HTREEITEM hItem, BOOL bShare);

	// share list access
	bool IsShared(CString strDir);
	void AddShare(CString strDir);
	void DelShare(CString strDir);
	void MarkChilds(HTREEITEM hChild,bool mark);

	CStringList m_lstShared;
	CString m_strLastRightClicked;
	bool m_bSelectSubDirs;

public:
	// construction / destruction
	CDirectoryTreeCtrl();
	virtual ~CDirectoryTreeCtrl();
	virtual BOOL OnCommand(WPARAM wParam,LPARAM lParam );

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnTvnItemexpanding(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnTvnDeleteItem(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
};
