/*
 * $Id: IPFilterDlg.h 4483 2008-01-02 09:19:06Z soarchin $
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
#include "ResizableLib/ResizableDialog.h"
#include "ListCtrlX.h"

struct SIPFilter;

class CIPFilterDlg : public CResizableDialog
{
	DECLARE_DYNAMIC(CIPFilterDlg)

public:
	CIPFilterDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CIPFilterDlg();

// Dialog Data
	enum { IDD = IDD_IPFILTER };

protected:
	static int sm_iSortColumn;
	CMenu* m_pMenuIPFilter;
	CListCtrlX m_ipfilter;
	HICON m_icoDlg;
	UINT m_uIPFilterItems;
	const SIPFilter** m_ppIPFilterItems;
	ULONG m_ulFilteredIPs;

	void SortIPFilterItems();
	void InitIPFilters();
	static bool FindItem(const CListCtrlX& lv, int iItem, DWORD_PTR lParam);

	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnLvnColumnClickIPFilter(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnKeyDownIPFilter(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedAppend();
	afx_msg void OnBnClickedCopy();
	afx_msg void OnBnClickedDelete();
	afx_msg void OnBnClickedSave();
	afx_msg void OnCopyIPFilter();
	afx_msg void OnDeleteIPFilter();
	afx_msg void OnSelectAllIPFilter();
	afx_msg void OnFind();
	afx_msg void OnLvnGetDispInfoIPFilter(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnDeleteItemIPFilter(NMHDR *pNMHDR, LRESULT *pResult);
};
