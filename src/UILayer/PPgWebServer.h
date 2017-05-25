/*
 * $Id: PPgWebServer.h 4483 2008-01-02 09:19:06Z soarchin $
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
#include "HypertextCtrl.h"
#include "ToolTipCtrlZ.h"

class CPPgWebServer : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgWebServer)

public:
	CPPgWebServer();
	virtual ~CPPgWebServer();

	enum { IDD = IDD_PPG_WEBSRV };

	void Localize(void);

protected:
	BOOL m_bModified;
	bool bCreated;
	CHyperTextCtrl m_wndMobileLink;
	CToolTipCtrlZ	m_ttc;

	void LoadSettings(void);

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	void SetModified(BOOL bChanged = TRUE){
		m_bModified = bChanged;
		CPropertyPage::SetModified(bChanged);
	}

	DECLARE_MESSAGE_MAP()
	afx_msg void OnEnChangeWSEnabled();
	afx_msg void OnEnChangeMMEnabled();
	afx_msg void OnReloadTemplates();
	afx_msg void OnBnClickedTmplbrowse();
	afx_msg void OnHelp();
	afx_msg void SetTmplButtonState();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnDataChange()				{SetModified(); SetTmplButtonState(); }
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
