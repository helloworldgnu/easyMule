/*
 * $Id: PPgAntiLeech.h 6876 2008-08-27 09:36:20Z dgkang $
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

//#include "preferences.h"
//#include "afxwin.h"
// CPPgAntiLeech dialog

class CPPgAntiLeech : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgAntiLeech)

public:
	CPPgAntiLeech();
	virtual ~CPPgAntiLeech();

	// Dialog Data
	enum { IDD = IDD_PPG_ANTILEECH };

	virtual BOOL OnApply();
	virtual BOOL OnInitDialog();

	void Localize();


protected:
	void LoadSettings();

	// Generated message map functions
	afx_msg void OnSettingsChange() {SetModified();}
	DECLARE_MESSAGE_MAP()

	// for dialog data exchange and validation
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	
public:
	afx_msg void OnBnClickedAntiLeecher(); //Xman Anti-Leecher
	afx_msg void OnBnClickedDlpreload(); //Xman DLP
	afx_msg void OnBnClickedHplink(); //Xman Xtreme Links
	afx_msg void OnBnClickedForumlink(); //Xman Xtreme Links
	afx_msg void OnBnClickedVotelink(); //Xman Xtreme Links
	afx_msg void OnBnClickedAntileecherUpdateCheck();
};
