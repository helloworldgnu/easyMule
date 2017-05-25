/*
 * $Id: CommentDialog.h 4483 2008-01-02 09:19:06Z soarchin $
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
#include "ComboBoxEx2.h"
#include "CommentListCtrl.h"

class CKnownFile;
namespace Kademlia {
	class CEntry;
};

class CCommentDialog : public CResizablePage
{
	DECLARE_DYNAMIC(CCommentDialog)

public:
	CCommentDialog();	// standard constructor
	virtual ~CCommentDialog();

	void SetFiles(const CSimpleArray<CObject*>* paFiles) { m_paFiles = paFiles; m_bDataChanged = true; }

	// Dialog Data
	enum { IDD = IDD_COMMENT };

	void Localize();

protected:
	const CSimpleArray<CObject*>* m_paFiles;
	bool m_bDataChanged;
	CComboBoxEx2 m_ratebox;
	CImageList m_imlRating;
	CCommentListCtrl m_lstComments;
	bool m_bMergedComment;
	bool m_bSelf;
	uint32 m_timer;

	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnSetActive();
	virtual BOOL OnApply();
	void RefreshData(bool deleteOld = true);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedSearchKad(); 
	afx_msg void OnBnClickedReset();
	afx_msg LRESULT OnDataChanged(WPARAM, LPARAM);
	afx_msg void OnEnChangeCmtText();
	afx_msg void OnCbnSelendokRatelist();
	afx_msg void OnCbnSelchangeRatelist();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT nIDEvent);
};
