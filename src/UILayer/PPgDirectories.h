/*
 * $Id: PPgDirectories.h 4483 2008-01-02 09:19:06Z soarchin $
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
#include "DirectoryTreeCtrl.h"
#include "ToolTipCtrlZ.h"

class CPPgDirectories : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgDirectories)

public:
	CPPgDirectories();									// standard constructor
	virtual ~CPPgDirectories();

// Dialog Data
	enum { IDD = IDD_PPG_DIRECTORIES };

	void Localize(void);

protected:
	CDirectoryTreeCtrl m_ShareSelector;
	CListCtrl m_ctlUncPaths;
	CToolTipCtrlZ	m_ttc;

	void LoadSettings(void);
	void FillUncList(void);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnSettingsChange()					{ SetModified(); }
	afx_msg void OnBnClickedSelincdir();
	afx_msg void OnBnClickedSeltempdir();
	afx_msg void OnBnClickedAddUNC();
	afx_msg void OnBnClickedRemUNC();
	afx_msg void OnHelp();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnBnClickedSeltempdiradd();
public:
	afx_msg void OnBnClickedIncomingFrm();
	afx_msg void OnTvnSelchangedShareselector(NMHDR *pNMHDR, LRESULT *pResult);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
