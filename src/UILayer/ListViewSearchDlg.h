/*
 * $Id: ListViewSearchDlg.h 4483 2008-01-02 09:19:06Z soarchin $
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

//////////////////////////////////////////////////////////////////////////////
// CListViewSearchDlg

class CListViewSearchDlg : public CDialog
{
	DECLARE_DYNAMIC(CListViewSearchDlg)

public:
	CListViewSearchDlg(CWnd* pParent = NULL);	  // standard constructor
	virtual ~CListViewSearchDlg();

// Dialog Data
	enum { IDD = IDD_LISTVIEW_SEARCH };

	CListCtrl* m_pListView;
	CString m_strFindText;
	bool m_bCanSearchInAllColumns;
	int m_iSearchColumn;

protected:
	HICON m_icnWnd;
	CComboBox m_ctlSearchCol;

	void UpdateControls();

	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	DECLARE_MESSAGE_MAP()
	afx_msg void OnEnChangeSearchText();
};
