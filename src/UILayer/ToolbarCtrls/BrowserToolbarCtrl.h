/*
 * $Id: BrowserToolbarCtrl.h 4483 2008-01-02 09:19:06Z soarchin $
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
//#include "toolbarctrlx.h"

#define NUM_BROWSER_BUTTON 5

#define IDC_BROWSERBAR  63000
#define IDC_BROWSERBUTTON	63001

#define	TB_BACK			(IDC_BROWSERBUTTON + 0)
#define	TB_FORWARD		(IDC_BROWSERBUTTON + 1)
#define	TB_STOP			(IDC_BROWSERBUTTON + 2)
#define TB_REFRESH      (IDC_BROWSERBUTTON + 3)  
#define	TB_HOME			(IDC_BROWSERBUTTON + 4)

#define	MULE_BROWSERBAR_BAND_NR	1

//  CBrowserToolbarCtrl
#include "Localizee.h"
#include "ToolBarCtrlZ.h"

class CBrowserToolbarCtrl : public CToolBarCtrlZ, public CLocalizee
{
	DECLARE_DYNAMIC(CBrowserToolbarCtrl)
	LOCALIZEE_WND_CANLOCALIZE()
public:
	CBrowserToolbarCtrl();
	virtual ~CBrowserToolbarCtrl();

	void Init();
	void Localize();
	void Refresh();

	void EnableAllButtons(BOOL bEnable = TRUE);

protected:
	TBBUTTON	TBButtons[NUM_BROWSER_BUTTON];    // Added by thilon on 2006.08.02
	TCHAR		TBStrings[NUM_BROWSER_BUTTON][200];

	void	InitImageList();
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize(UINT nType, int cx, int cy);
};


