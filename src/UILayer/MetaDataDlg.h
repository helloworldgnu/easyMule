/*
 * $Id: MetaDataDlg.h 4483 2008-01-02 09:19:06Z soarchin $
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
#include "ResizableLib/ResizablePage.h"
#include "ListCtrlX.h"
#include <list>

class CAbstractFile;
namespace Kademlia 
{
	class CKadTag;
	typedef std::list<CKadTag*> TagList;
};

class CMetaDataDlg : public CResizablePage
{
	DECLARE_DYNAMIC(CMetaDataDlg)

public:
	CMetaDataDlg();
	virtual ~CMetaDataDlg();

	void SetFiles(const CSimpleArray<CObject*>* paFiles) { m_paFiles = paFiles; m_bDataChanged = true; }
	void SetTagList(Kademlia::TagList* taglist);
	CString GetTagNameByID(UINT id);

// Dialog Data
	enum { IDD = IDD_META_DATA };

protected:
	const CSimpleArray<CObject*>* m_paFiles;
	bool m_bDataChanged;
	Kademlia::TagList* m_taglist;
	CString m_strCaption;
	CMenu* m_pMenuTags;
	CListCtrlX m_tags;

	void RefreshData();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnSetActive();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnLvnKeydownTags(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnCopyTags();
	afx_msg void OnSelectAllTags();
	afx_msg void OnDestroy();
	afx_msg LRESULT OnDataChanged(WPARAM, LPARAM);
};
