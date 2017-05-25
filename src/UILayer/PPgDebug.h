/*
 * $Id: PPgDebug.h 4483 2008-01-02 09:19:06Z soarchin $
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
#include "TreeOptionsCtrlEx.h"

#define	MAX_DETAIL_ITEMS	7
#define	MAX_INTEGER_ITEMS	1

class CPPgDebug : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgDebug)

public:
	CPPgDebug();
	virtual ~CPPgDebug();

// Dialog Data
	enum { IDD = IDD_PPG_DEBUG };

protected:
	HTREEITEM m_htiServer;
	HTREEITEM m_htiClient;

	// detail level items
	HTREEITEM m_cb[MAX_DETAIL_ITEMS];
	HTREEITEM m_lv[MAX_DETAIL_ITEMS];
	BOOL m_checks[MAX_DETAIL_ITEMS];
	int m_levels[MAX_DETAIL_ITEMS];

	// integer items
	HTREEITEM m_htiInteger[MAX_INTEGER_ITEMS];
	int m_iValInteger[MAX_INTEGER_ITEMS];

	CTreeOptionsCtrlEx m_ctrlTreeOptions;
	bool m_bInitializedTreeOpts;

	void ClearAllMembers();

	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	virtual BOOL OnKillActive();
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg LRESULT OnTreeOptsCtrlNotify(WPARAM wParam, LPARAM lParam);
	afx_msg void OnHelp();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
};
