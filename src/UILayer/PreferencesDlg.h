/*
 * $Id: PreferencesDlg.h 4483 2008-01-02 09:19:06Z soarchin $
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
#include "PPgGeneral.h"
#include "PPgConnection.h"
#include "PPgServer.h"
#include "PPgDirectories.h"
#include "PPgFiles.h"
#include "PPgDisplay.h"
#include "PPgWebServer.h"
#include "PPgScheduler.h"
#include "PPgProxy.h"
#include "PPgAntiLeech.h"
#include "PPGTweaks.h"
#include "PPgDownloadSecurity.h"

#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
#include "PPgDebug.h"
#endif
#include "otherfunctions.h"
#include "TreePropSheet.h"
#include "ToolTipCtrlZ.h"

class CPreferencesDlg : public CTreePropSheet
{
	DECLARE_DYNAMIC(CPreferencesDlg)

public:
	CPreferencesDlg();
	virtual ~CPreferencesDlg();
	
	CPPgGeneral		m_wndGeneral;
	CPPgConnection	m_wndConnection;
	CPPgDisplay		m_wndDisplay;
	CPPgDirectories	m_wndDirectories;
	CPPgFiles		m_wndFiles;
	// VC-kernel[2007-02-05]:to be deleted
	CPPgServer		m_wndServer;
	//CPPgStats		m_wndStats;
	//CPPgNotify		m_wndNotify;
	//CPPgIRC			m_wndIRC;
	CPPgTweaks		m_wndTweaks;
	//CPPgSecurity	m_wndSecurity;
	CPPgWebServer	m_wndWebServer;
	CPPgScheduler	m_wndScheduler;
	CPPgProxy		m_wndProxy;
	//CPPgMessages	m_wndMessages;
	// VC-kernel[2007-02-05]:to be deleted
	CPPgAntiLeech   m_wndAntiLeech;
	CPPgDownloadSecurity m_wndDownloadSecurity;

#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
	CPPgDebug		m_wndDebug;
#endif


	void Localize();
	void SetStartPage(UINT uStartPageID);

protected:
	LPCTSTR m_pPshStartPage;
	bool m_bSaveIniFile;
	
	CToolTipCtrlZ	m_ttc;
	BOOL			m_bTipAdded;
	void			AddTreeItemTip(int iPage, LPCTSTR lpszTipText, UINT uIdTool);


	virtual BOOL OnInitDialog();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg void OnHelp();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnPageAdvanceChanged();// VC-kernel[2007-02-09]:
	afx_msg LRESULT OnSetCurSel(WPARAM wParam, LPARAM lParam);
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
