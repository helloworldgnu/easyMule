/* 
 * $Id: CollectionViewDialog.h 4483 2008-01-02 09:19:06Z soarchin $
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

// CCollectionViewDialog dialog

class CCollection;

class CCollectionViewDialog : public CResizableDialog
{
	DECLARE_DYNAMIC(CCollectionViewDialog)

public:
	CCollectionViewDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCollectionViewDialog();

	void SetCollection(CCollection* pCollection);
	virtual BOOL OnInitDialog(void);

	// Dialog Data
	enum { IDD = IDD_COLLECTIONVIEWDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	void DownloadSelected(void);

	CCollectionListCtrl m_CollectionViewList;
	CCollection* m_pCollection;
	HICON	m_icoWnd;
	HICON	m_icoColl;

	afx_msg void OnNMDblclkCollectionvewlist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedViewcollectiondl();
	afx_msg void OnBnClickedOk();
	CButton m_AddNewCatagory;
	CStatic m_CollectionViewListLabel;
	CStatic m_CollectionViewListIcon;
	CButton m_CollectionDownload;
	CButton m_CollectionExit;
	CEdit m_CollectionViewAuthor;
	CEdit m_CollectionViewAuthorKey;
};
