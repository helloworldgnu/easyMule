/*
 * $Id: MuleToolBarCtrl.h 4483 2008-01-02 09:19:06Z soarchin $
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

#define IDC_TOOLBAR			16127
#define IDC_TOOLBARBUTTON	16129

#define	TBBTN_CONNECT	(IDC_TOOLBARBUTTON + 0)
#define	TBBTN_KAD		(IDC_TOOLBARBUTTON + 1)
#define	TBBTN_SERVER	(IDC_TOOLBARBUTTON + 2)
#define	TBBTN_TRANSFERS	(IDC_TOOLBARBUTTON + 3)
#define	TBBTN_SEARCH	(IDC_TOOLBARBUTTON + 4)
#define	TBBTN_SHARED	(IDC_TOOLBARBUTTON + 5)
#define	TBBTN_MESSAGES	(IDC_TOOLBARBUTTON + 6)
#define	TBBTN_IRC		(IDC_TOOLBARBUTTON + 7)
#define	TBBTN_STATS		(IDC_TOOLBARBUTTON + 8)
#define	TBBTN_OPTIONS	(IDC_TOOLBARBUTTON + 9)
#define	TBBTN_TOOLS		(IDC_TOOLBARBUTTON + 10)
#define	TBBTN_HELP		(IDC_TOOLBARBUTTON + 11)
#define	TBBTN_WEBBROWSER	(IDC_TOOLBARBUTTON + 12) //Added by thilon on 2006.08.01, WebBrowser

#define	MULE_TOOLBAR_BAND_NR	0

#ifdef _DISABLE_WEBBROWSER
#define MULE_TOOLBAR_BUTTON_NUM      12
#else
#define MULE_TOOLBAR_BUTTON_NUM      13 //Added by thilon on 2006.08.01
#endif

enum EToolbarLabelType {
	NoLabels	= 0,
	LabelsBelow = 1,
	LabelsRight = 2
};

class CMuleToolbarCtrl : public CToolBarCtrl
{
	DECLARE_DYNAMIC(CMuleToolbarCtrl)

public:
	CMuleToolbarCtrl();
	virtual ~CMuleToolbarCtrl();

	void Init();
	void Localize();
	void Refresh();
	void SaveCurHeight();
	void UpdateBackground();
	void PressMuleButton(int nID);

	static int GetDefaultLabelType() { return (int)LabelsRight; }	//Changed by thilon on 2006.08.17, 将工具栏的文本默认位置修改为显示在右边

protected:
	CSize		m_sizBtnBmp;
	int			m_iPreviousHeight;
	int			m_iLastPressedButton;
	int			m_buttoncount;
	TBBUTTON	TBButtons[MULE_TOOLBAR_BUTTON_NUM];
	TCHAR		TBStrings[MULE_TOOLBAR_BUTTON_NUM][200];
	CStringArray m_astrToolbarPaths;
	EToolbarLabelType m_eLabelType;
	CStringArray m_astrSkinPaths;
	CBitmap		m_bmpBack;

	void ChangeToolbarBitmap(const CString& rstrPath, bool bRefresh);
	void ChangeTextLabelStyle(EToolbarLabelType eLabelType, bool bRefresh, bool bForceUpdateButtons = false);
	void UpdateIdealSize();
	void SetAllButtonsStrings();
	void SetAllButtonsWidth();
	void ForceRecalcLayout();

#ifdef _DEBUG
	void Dump();
#endif

	void AutoSize();
	virtual	BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSysColorChange();
	afx_msg void OnNMRclick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTbnQueryDelete(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTbnQueryInsert(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTbnGetButtonInfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTbnToolbarChange(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTbnReset(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTbnInitCustomize(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTbnEndAdjust(NMHDR* pNMHDR, LRESULT* pResult);
};
