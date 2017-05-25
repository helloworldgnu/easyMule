/*
 * $Id: PPgDisplay.h 6625 2008-08-13 09:17:19Z dgkang $
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

#include "3dpreviewcontrol.h"
#include "HotKey/HotKeyEdit.h"// VC-kernel[2007-05-14]:hotkey
#include "afxcmn.h"

class CPPgDisplay : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgDisplay)

public:
	CPPgDisplay();
	virtual ~CPPgDisplay();

// Dialog Data
	enum { IDD = IDD_PPG_DISPLAY };

	void Localize(void);

protected:
	enum ESelectFont
	{
		sfServer,
		sfLog
	} m_eSelectFont;
	void LoadSettings(void);

	void DrawPreview();		//Cax2 - aqua bar
	C3DPreviewControl	m_3DPreview;
	//CHotKeyEdit * m_HotKey;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	static UINT CALLBACK ChooseFontHook(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSettingsChange()					{ SetModified(); }
	afx_msg void OnBnClickedSelectHypertextFont();
	afx_msg void OnBtnClickedResetHist();
	afx_msg void OnHelp();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);

	afx_msg LRESULT HotKeyChange(WPARAM,LPARAM)		{ SetModified();return NULL;}
public:
	CHotKeyEdit m_Hotkey;
	afx_msg void OnBnClickedShowtasknotify();
	afx_msg void OnBnClickedShownotifyonnew();
	afx_msg void OnBnClickedShowhomepage();
};
