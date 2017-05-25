/* 
 * $Id: DownloadListCtrl.h 5411 2008-04-29 08:21:19Z thilon $
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
#include "MuleListCtrl.h"
#include "TitleMenu.h"
#include <map>
#include "ListCtrlItemWalk.h"

#define COLLAPSE_ONLY	0
#define EXPAND_ONLY		1
#define EXPAND_COLLAPSE	2

// Foward declaration
class CPartFile;
class CUpDownClient;
class CDownloadListCtrl;
class CToolTipCtrlX;


///////////////////////////////////////////////////////////////////////////////
// CtrlItem_Struct

enum ItemType { FILE_TYPE = 1, AVAILABLE_SOURCE = 2, UNAVAILABLE_SOURCE = 3,FILE_TASK = 4};

class CtrlItem_Struct : public CObject
{
	DECLARE_DYNAMIC(CtrlItem_Struct)

public:
	~CtrlItem_Struct() { status.DeleteObject(); }

	ItemType         type;
	CPartFile*       owner;
	void*            value; // could be both CPartFile or CUpDownClient
	CtrlItem_Struct* parent;
	DWORD            dwUpdated;
	CBitmap          status;
};


///////////////////////////////////////////////////////////////////////////////
// CDownloadListListCtrlItemWalk

class CDownloadListListCtrlItemWalk : public CListCtrlItemWalk
{
public:
	CDownloadListListCtrlItemWalk(CDownloadListCtrl* pListCtrl);

	virtual CObject* GetNextSelectableItem();
	virtual CObject* GetPrevSelectableItem();

	void SetItemType(ItemType eItemType) { m_eItemType = eItemType; }

protected:
	CDownloadListCtrl* m_pDownloadListCtrl;
	ItemType m_eItemType;
};


///////////////////////////////////////////////////////////////////////////////
// CDownloadListCtrl

class CDownloadListCtrl : public CMuleListCtrl, public CDownloadListListCtrlItemWalk
{
	DECLARE_DYNAMIC(CDownloadListCtrl)
	friend class CDownloadListListCtrlItemWalk;

public:
	CDownloadListCtrl();
	virtual ~CDownloadListCtrl();

	UINT	curTab;

	void	UpdateItem(void* toupdate);
	void	Init();
	void	AddFile(CPartFile* toadd);
	void	AddSource(CPartFile* owner, CUpDownClient* source, bool notavailable);
	void	RemoveSource(CUpDownClient* source, CPartFile* owner);
	bool	RemoveFile(const CPartFile* toremove);
	void	ClearCompleted(int incat=-2);
	void	ClearCompleted(const CPartFile* pFile);
	void	SetStyle();
	void	CreateMenues();
	void	Localize();
	void	ShowFilesCount();
	void	ChangeCategory(int newsel);
	CString getTextList();
	void	ShowSelectedFileDetails();
	void	HideFile(CPartFile* tohide);
	void	ShowFile(CPartFile* tohide);
	void	ExpandCollapseItem(int iItem, int iAction, bool bCollapseSource = false);
	void	HideSources(CPartFile* toCollapse);
	void	GetDisplayedFiles(CArray<CPartFile*, CPartFile*>* list);
	void	MoveCompletedfilesCat(uint8 from, uint8 to);
	int		GetCompleteDownloads(int cat,int &total);
	void	UpdateCurrentCategoryView();
	void	UpdateCurrentCategoryView(CPartFile* thisfile);

protected:
	CImageList  m_ImageList;
	CTitleMenu	m_PrioMenu;
	CTitleMenu	m_FileMenu;
	CMenu		m_SourcesMenu;
	CMenuXP*	m_MenuXP;
	bool		m_bRemainSort;
	typedef std::pair<void*, CtrlItem_Struct*> ListItemsPair;
	typedef std::multimap<void*, CtrlItem_Struct*> ListItems;
    ListItems	m_ListItems;
	CFont		m_fontBold;
	CToolTipCtrlX* m_tooltip;

	void ShowFileDialog(UINT uInvokePage);
	void ShowClientDialog(CUpDownClient* pClient);
	void SetAllIcons();
	void DrawFileItem(CDC *dc, int nColumn, LPCRECT lpRect, CtrlItem_Struct *lpCtrlItem);
	void DrawSourceItem(CDC *dc, int nColumn, LPCRECT lpRect, CtrlItem_Struct *lpCtrlItem);
	int GetFilesCountInCurCat();

	static int CALLBACK SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
    static int Compare(const CPartFile* file1, const CPartFile* file2, LPARAM lParamSort);
    static int Compare(const CUpDownClient* client1, const CUpDownClient* client2, LPARAM lParamSort);

	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnSysColorChange();
	afx_msg void OnItemActivate(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg	void OnColumnClick( NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnListModified(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkDownloadlist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLvnGetInfoTip(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnCommentClick(NMHDR *pNMHDR, LRESULT *pResult);
public:
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);

	afx_msg LRESULT OnReCreatePartFile(WPARAM wParam, LPARAM lParam);

protected:
	typedef	struct _ToolBarUI
	{
		int iFilesToCancel;
		int iFilesToStop;
		int iFilesToPause;
		int iFilesToResume;
		int iFilesToOpenFolder;
	} ToolBarUI;

	ToolBarUI m_ToolBarUI;

public:
	CPartFile* m_pPartFile;
	CWnd* m_pDialog;

protected:
	void UpdateToolBarItem(ToolBarUI* pToolBarUI);
public:
	void UpdateToolBarItem(void);

private:
	afx_msg LRESULT OnUpdateGUIStart(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUpdateGUIStop(WPARAM wParam, LPARAM lParam);
public:
	void OnNoComment(CHtmlCtrl *pHtml);
	int FindFile(CPartFile* pPartFile);
	int FindFile(CUpDownClient* pClient);
};
