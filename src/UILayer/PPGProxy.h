/*
 * $Id: PPGProxy.h 4483 2008-01-02 09:19:06Z soarchin $
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
#include "Preferences.h"

class CPPgProxy : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgProxy)

public:
	CPPgProxy();
	virtual ~CPPgProxy();

	// Dialog Data
	enum { IDD = IDD_PPG_PROXY };

	void Localize(void);

protected:
	ProxySettings proxy;

	void LoadSettings();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedEnableProxy();
	afx_msg void OnBnClickedEnableAuthentication();
	afx_msg void OnCbnSelChangeProxyType();
	afx_msg void OnEnChangeProxyName() { SetModified(TRUE); }
	afx_msg void OnEnChangeProxyPort() { SetModified(TRUE); }
	afx_msg void OnEnChangeUserName() { SetModified(TRUE); }
	afx_msg void OnEnChangePassword() { SetModified(TRUE); }
	afx_msg void OnHelp();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
};
