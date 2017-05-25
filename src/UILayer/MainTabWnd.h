/*
 * $Id: MainTabWnd.h 11398 2009-03-17 11:00:27Z huby $
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

#include "TabWnd.h"

// CMainTabWnd
#include "DlgMainTabResource.h"
#include "DlgMaintabDownload.h"
#include "DlgMainTabShare.h"
#include "DlgMainTabSidePanel.h"
#include "DlgMainTabAdvance.h"
#include "SpeedMeterDlg.h"

#include "Localizee.h"
//#include "StatDlg.h"

class CMainTabWnd : public CTabWnd, public CLocalizee
{
	DECLARE_DYNAMIC(CMainTabWnd)
	LOCALIZEE_WND_CANLOCALIZE()
public:
	CMainTabWnd();
	virtual ~CMainTabWnd();

	BOOL	CreateEx(const RECT& rect, CWnd* pParentWnd, UINT nID);

	CDlgMainTabResource			m_dlgResource;
	CDlgMaintabDownload			m_dlgDownload;
	CDlgMainTabShare			m_dlgShare;
	CDlgMainTabAdvance			m_dlgAdvance;

	CDlgMainTabSidePanel		m_dlgSidePanel;
	/*CSpeedMeterDlg				m_SpeedMeterDlg;*/

	enum ETabId
	{
		TI_RESOURCE,
		TI_DOWNLOAD,
		TI_SHARE,
		TI_ADVANCE,
		TI_BN,
		
		TI_MAX
	};
	POSITION	m_aposTabs[TI_MAX];
	BOOL       IsTabActive(ETabId eTabId){return m_aposTabs[eTabId] == GetActiveTab();}
	BOOL		IsTabShowed(ETabId eTabId);
	void		AddTabById(ETabId eTabId);
	void		RemoveTabById(ETabId eTabId);
	void		SetActiveTabById(ETabId eTabId){SetActiveTab(m_aposTabs[eTabId]);}
protected:
	void		Localize();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	
//	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
//	afx_msg void OnPaint();
};


