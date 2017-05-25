/*
 * $Id: CommentDialogLst.h 4483 2008-01-02 09:19:06Z soarchin $
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
#include "CommentListCtrl.h"

class CPartFile;


///////////////////////////////////////////////////////////////////////////////
// CCommentDialogLst

class CCommentDialogLst : public CResizablePage
{ 
	DECLARE_DYNAMIC(CCommentDialogLst) 

public: 
	CCommentDialogLst(); 
	virtual ~CCommentDialogLst(); 

	void SetFiles(const CSimpleArray<CObject*>* paFiles) { m_paFiles = paFiles; m_bDataChanged = true; }

// Dialog Data 
	enum { IDD = IDD_COMMENTLST }; 

protected: 
	CString m_strCaption;
	CCommentListCtrl m_lstComments;
	const CSimpleArray<CObject*>* m_paFiles;
	bool m_bDataChanged;
	uint32 m_timer;

	void Localize();
	void RefreshData(bool deleteOld = true);

	virtual BOOL OnInitDialog(); 
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support 
	virtual BOOL OnSetActive();

	DECLARE_MESSAGE_MAP() 
	afx_msg void OnBnClickedApply(); 
	afx_msg void OnBnClickedSearchKad(); 
	afx_msg LRESULT OnDataChanged(WPARAM, LPARAM);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT nIDEvent);
};
