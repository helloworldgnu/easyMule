/* 
 * $Id: SharedDirsTreeCtrl.h 5160 2008-03-27 09:41:32Z thilon $
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
#include "TitleMenu.h"

enum ESpecialDirectoryItems{
	SDI_NO = 0,
	SDI_ALL,
	SDI_INCOMING,
	SDI_TEMP,
	SDI_DIRECTORY,
	SDI_CATINCOMING,
	SDI_UNSHAREDDIRECTORY,
#ifdef _ENABLE_SHARE_FILES
	SDI_SHAREEDFILES, // VC-SearchDream[2007-02-12]: Add for shared files
#endif
	SDI_FILESYSTEMPARENT
};

class CSharedFilesCtrl;
class CKnownFile;

//**********************************************************************************
// CDirectoryItem

class CDirectoryItem{
public:
	CDirectoryItem(CString strFullPath, HTREEITEM htItem = TVI_ROOT, ESpecialDirectoryItems eItemType = SDI_NO, int m_nCatFilter = -1);
	~CDirectoryItem();
	CDirectoryItem*		CloneContent() { return new CDirectoryItem(m_strFullPath, 0, m_eItemType, m_nCatFilter); }
	HTREEITEM			FindItem(CDirectoryItem* pContentToFind) const;

	CString		m_strFullPath;
	HTREEITEM	m_htItem;
	int			m_nCatFilter;
	CList<CDirectoryItem*, CDirectoryItem*> liSubDirectories;
	ESpecialDirectoryItems m_eItemType;
};

//**********************************************************************************
// CSharedDirsTreeCtrl

class CSharedDirsTreeCtrl : public CTreeCtrl
{
	DECLARE_DYNAMIC(CSharedDirsTreeCtrl)

public:
	CSharedDirsTreeCtrl();
	virtual ~CSharedDirsTreeCtrl();

public:
	enum CheckedState { UNUSED1 = 0, 
		UNCHECKED, CHECKED, TRISTATE, 
		UNUSED2,
		UNCHECKED_DISABLED, CHECKED_DISABLED, TRISTATE_DISABLED };

public:	
	void			Initalize(CSharedFilesCtrl* pSharedFilesCtrl);
	void			SetAllIcons();

	CDirectoryItem* GetSelectedFilter() const;
	bool			IsCreatingTree() const		{return m_bCreatingTree;};
	void			Localize();
	void			EditSharedDirectories(CDirectoryItem* pDir, bool bAdd, bool bSubDirectories);
	void			Reload(bool bFore = false);
	bool			GetDirectoryState(CString strDir);
	void			UpdateTreeItem(CKnownFile* file);

protected:
	virtual BOOL	OnCommand(WPARAM wParam, LPARAM lParam);
	void			CreateMenues();
	void			ShowFileDialog(CTypedPtrList<CPtrList, CKnownFile*>& aFiles, UINT uPshInvokePage = 0);
	void			DeleteChildItems(CDirectoryItem* pParent);
	void			AddSharedDirectory(CString strDir, bool bSubDirectories);
	void			RemoveSharedDirectory(CString strDir, bool bSubDirectories);
	int				AddSystemIcon(HICON hIcon, int nSystemListPos);
	void			FetchSharedDirsList();

	DECLARE_MESSAGE_MAP()
	afx_msg void	OnSysColorChange();
	afx_msg void	OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg	void	OnRButtonDown(UINT nFlags, CPoint point );
	afx_msg	void	OnLButtonUp(UINT nFlags, CPoint point );
	afx_msg void	OnTvnItemexpanding(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void	OnTvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void	OnLvnBegindrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void	OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void	OnCancelMode();

	CTitleMenu			m_SharedFilesMenu;
	CTitleMenu			m_ShareDirsMenu;
	CMenu				m_PrioMenu;
	CDirectoryItem*		m_pRootDirectoryItem;
	CDirectoryItem*		m_pRootUnsharedDirectries;
	CDirectoryItem*		m_pDraggingItem;
	CSharedFilesCtrl*	m_pSharedFilesCtrl;
	CStringList			m_strliSharedDirs;
	CStringList			m_strliCatIncomingDirs;
	CImageList			m_imlTree;

	CMenuXP*			m_pMenuXP;

private:
	void	InitalizeStandardItems();
	
	void	FileSystemTreeCreateTree();
	void	FileSystemTreeAddChildItem(CDirectoryItem* pRoot, CString strText, bool bTopLevel);
	bool	FileSystemTreeHasSubdirectories(CString strDir);
	bool	FileSystemTreeHasSharedSubdirectory(CString strDir);
	void	FileSystemTreeAddSubdirectories(CDirectoryItem* pRoot);
	bool	FileSystemTreeIsShared(CString strDir);
	void	FileSystemTreeUpdateBoldState(CDirectoryItem* pDir = NULL);
	void	FileSystemTreeSetShareState(CDirectoryItem* pDir, bool bShared, bool bSubDirectories);

	void	FilterTreeAddSharedDirectory(CDirectoryItem* pDir, bool bRefresh);
	void	FilterTreeAddSubDirectories(CDirectoryItem* pDirectory, CStringList& liDirs, int nLevel = 0);
	bool	FilterTreeIsSubDirectory(CString strDir, CString strRoot, CStringList& liDirs);
	void	FilterTreeReloadTree();


	bool			m_bCreatingTree;
	bool			m_bUseIcons;
	CMap<int, int, int, int> m_mapSystemIcons;
public:
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);

	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);

protected:
	CImageList		m_StateImage;

public:
	BOOL		CreateCheckboxImages();
	int			GetStateImage(HTREEITEM hItem);
	void			ShareSubDirectories(HTREEITEM hItem, BOOL bShare);
	void			ExpandList(void);

protected:
	void		MarkChilds(HTREEITEM hChild,BOOL mark);
	HTREEITEM	FindItem(CString strDir, HTREEITEM hItem = NULL);
	HTREEITEM	GetNextItem(HTREEITEM hItem);
public:
	afx_msg void OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);
};


