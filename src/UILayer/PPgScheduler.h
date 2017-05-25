/*
 * $Id: PPgScheduler.h 4483 2008-01-02 09:19:06Z soarchin $
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

class CPPgScheduler : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgScheduler)

public:
	CPPgScheduler();
	virtual ~CPPgScheduler();

	// Dialog Data
	enum { IDD = IDD_PPG_SCHEDULER };

	void Localize(void);

// Dialog Data
protected:
	CComboBox m_timesel;
	CDateTimeCtrl m_time;
	CDateTimeCtrl m_timeTo;
	CListCtrl m_list;
	CListCtrl m_actions;
	CToolTipCtrlZ	m_ttc;


	CString GetActionLabel(int index);
	CString GetDayLabel(int index);
	void LoadSchedule(int index);
	void RecheckSchedules();
	void FillScheduleList();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnNMDblclkList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedAdd();
	afx_msg void OnBnClickedApply();
	afx_msg void OnBnClickedRemove();
	afx_msg void OnSettingsChange() {SetModified();}
	afx_msg void OnEnableChange();
	afx_msg void OnDisableTime2();
	afx_msg void OnNMDblclkActionlist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRclickActionlist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnHelp();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	BOOL PreTranslateMessage(MSG* pMsg);

};
