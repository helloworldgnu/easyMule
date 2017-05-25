/* 
 * $Id: CollectionCreateDialog.h 4483 2008-01-02 09:19:06Z soarchin $
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
#include "afxcmn.h"
#include "CollectionListCtrl.h"
#include "ResizableLib\ResizableDialog.h"

// CCollectionCreateDialog dialog

class CCollection;
class CCollectionFile;

class CCollectionCreateDialog : public CResizableDialog
{
	DECLARE_DYNAMIC(CCollectionCreateDialog)

public:
	CCollectionCreateDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCollectionCreateDialog();
	void SetCollection(CCollection* pCollection, bool create);

// Dialog Data
	enum { IDD = IDD_COLLECTIONCREATEDIALOG };

	CButton m_AddCollectionButton;
	CButton m_RemoveCollectionButton;
	CStatic m_CollectionListLabel;
	CButton m_SaveButton;
	CButton m_CancelButton;
	CStatic m_CollectionListIcon;
	CStatic m_CollectionSourceListIcon;
	CButton m_CollectionCreateSignNameKeyCheck;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog(void);

	void AddSelectedFiles(void);
	void RemoveSelectedFiles(void);
	void UpdateAvailFiles(void);

	DECLARE_MESSAGE_MAP()

	afx_msg void OnBnClickedCollectionremove();
	afx_msg void OnBnClickedCollectionadd();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCollectionviewsharebutton();
	afx_msg void OnNMDblclkCollectionavaillist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkCollectionlistctrl(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEnKillfocusCollectionnameedit();
	afx_msg void OnBnClickedCollectioncreateformat();

private:
	CCollection* m_pCollection;
	CEdit m_CollectionNameEdit;
	CCollectionListCtrl m_CollectionListCtrl;
	CCollectionListCtrl m_CollectionAvailListCtrl;
	bool m_bSharedFiles;
	CButton m_CollectionViewShareButton;
	CButton m_CollectionCreateFormatCheck;
	HICON	m_icoWnd;
	HICON	m_icoForward;
	HICON	m_icoBack;
	HICON	m_icoColl;
	HICON	m_icoFiles;
	bool	m_bCreatemode;
};
