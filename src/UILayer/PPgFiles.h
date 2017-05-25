/*
 * $Id: PPgFiles.h 4483 2008-01-02 09:19:06Z soarchin $
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
#include "ToolTipCtrlZ.h"

class CPPgFiles : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgFiles)

public:
	CPPgFiles();
	virtual ~CPPgFiles();

// Dialog Data
	enum { IDD = IDD_PPG_FILES };

	void Localize(void);

protected:
	UINT m_iFileBufferSize;
	CListBox m_uncfolders;
	CToolTipCtrlZ	m_ttc;

	void LoadSettings(void);
	void OnSettingsChangeCat(uint8 index);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnSetCleanupFilter();
	afx_msg void BrowseVideoplayer();
	afx_msg void OnSettingsChange();
	afx_msg void OnMonitorChange();
	afx_msg void OnSettingsChangeCat1() {OnSettingsChangeCat(1);}
	afx_msg void OnSettingsChangeCat2()	{OnSettingsChangeCat(2);}
	afx_msg void OnHelp();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
public:
	afx_msg void OnBnClickedClipboardwatch();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
