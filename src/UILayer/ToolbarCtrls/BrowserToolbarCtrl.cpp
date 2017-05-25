/*
 * $Id: BrowserToolbarCtrl.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
// BrowserToolbarCtrl.cpp : 实现文件
//

#include "stdafx.h"
#include "emule.h"
#include "otherfunctions.h"
#include "BrowserToolbarCtrl.h"
#include "Util.h"
#include ".\browsertoolbarctrl.h"


// CBrowserToolbarCtrl

IMPLEMENT_DYNAMIC(CBrowserToolbarCtrl, CToolBarCtrlZ)
CBrowserToolbarCtrl::CBrowserToolbarCtrl()
{
}

CBrowserToolbarCtrl::~CBrowserToolbarCtrl()
{
}


BEGIN_MESSAGE_MAP(CBrowserToolbarCtrl, CToolBarCtrlZ)
	ON_WM_SIZE()
END_MESSAGE_MAP()

//  CBrowserToolbarCtrl 消息处理程序

void CBrowserToolbarCtrl::Init()
{
	InitImageList();


    TBButtons[0].idCommand = TB_BACK;
	TBButtons[1].idCommand = TB_FORWARD;
	TBButtons[2].idCommand = TB_STOP;
	TBButtons[3].idCommand = TB_REFRESH;
	TBButtons[4].idCommand = TB_HOME;

	// add button-text:
	TCHAR cButtonStrings[500];
	int lLen, lLen2;
	
	_tcscpy(cButtonStrings, GetResString(IDS_BROWSER_BACK));
	lLen = _tcslen(GetResString(IDS_BROWSER_BACK)) + 1;

	lLen2 = _tcslen(GetResString(IDS_BROWSER_FORWARD)) + 1;
	memcpy(cButtonStrings+lLen, GetResString(IDS_BROWSER_FORWARD), lLen2*sizeof(TCHAR));
	lLen += lLen2;

	lLen2 = _tcslen(GetResString(IDS_BROWSER_STOP)) + 1;
	memcpy(cButtonStrings+lLen, GetResString(IDS_BROWSER_STOP), lLen2*sizeof(TCHAR));
	lLen += lLen2;

	lLen2 = _tcslen(GetResString(IDS_BROWSER_REFRESH)) + 1;
	memcpy(cButtonStrings+lLen, GetResString(IDS_BROWSER_REFRESH), lLen2*sizeof(TCHAR));
	lLen += lLen2;

	lLen2 = _tcslen(GetResString(IDS_BROWSER_VERYCD)) + 1;
	memcpy(cButtonStrings+lLen, GetResString(IDS_BROWSER_VERYCD), lLen2*sizeof(TCHAR));
	lLen += lLen2;

	// terminate
	memcpy(cButtonStrings+lLen, _T("\0"), sizeof(TCHAR));
	
	/*int iRet =*/AddStrings(cButtonStrings);

    
	for( int i = 0; i < NUM_BROWSER_BUTTON; i++ )
	{
		TBButtons[i].iBitmap = i;
		TBButtons[i].fsState = TBSTATE_ENABLED;
		TBButtons[i].fsStyle = TBSTYLE_BUTTON | BTNS_AUTOSIZE;
		TBButtons[i].iString = i;
	}

	AddButtons(NUM_BROWSER_BUTTON, TBButtons);

	Localize();
}

void CBrowserToolbarCtrl::OnSize(UINT nType, int cx, int cy)
{
	CToolBarCtrlZ::OnSize(nType, cx, cy);
	CToolBarCtrlZ::AutoSize();
}

void CBrowserToolbarCtrl::Localize()
{
	static const int TBStringIDs[] =
	{
		IDS_BROWSER_BACK, 
		IDS_BROWSER_FORWARD,
		IDS_BROWSER_STOP,
		IDS_BROWSER_REFRESH,
		IDS_BROWSER_VERYCD //Added by GGSoSo for webbrowser
	};
	TBBUTTONINFO tbi;
	tbi.dwMask = TBIF_TEXT;
	tbi.cbSize = sizeof(TBBUTTONINFO);

	for (int i = 0; i < NUM_BROWSER_BUTTON; i++)
	{
		_sntprintf(TBStrings[i], ARRSIZE(TBStrings[0]), _T("%s"), GetResString(TBStringIDs[i]));
		tbi.pszText = TBStrings[i];
		SetButtonInfo(IDC_BROWSERBUTTON+i, &tbi);
	}

	CWnd *pParent = GetParent();
	if (NULL != pParent)
		pParent->SendMessage(WM_SIZE);
}

void CBrowserToolbarCtrl::EnableAllButtons(BOOL bEnable)
{
	int iCount = GetButtonCount();

	int	i;
	TBBUTTONINFO	tbbi;

	ZeroMemory(&tbbi, sizeof(tbbi));
	tbbi.cbSize = sizeof(tbbi);
	tbbi.dwMask = TBIF_BYINDEX | TBIF_STATE;

	for (i = 0; i < iCount; i++)
	{
		GetButtonInfo(i, &tbbi);

		if (bEnable)
			tbbi.fsState |= TBSTATE_ENABLED;
		else
			tbbi.fsState &= ~TBSTATE_ENABLED;

		SetButtonInfo(i, &tbbi);
	}
}

void CBrowserToolbarCtrl::InitImageList()
{
	AddImageIcon(_T("PNG_BTBACK"));
	AddImageIcon(_T("PNG_BTFORWARD"));
	AddImageIcon(_T("PNG_BTSTOP"));
	AddImageIcon(_T("PNG_BTREFRESH"));
	AddImageIcon(_T("PNG_BTVERYCD"));

	AddDisableImageIcon(_T("PNG_BTBACK_D"));
	AddDisableImageIcon(_T("PNG_BTFORWARD_D"));
	AddDisableImageIcon(_T("PNG_BTSTOP_D"));
	AddDisableImageIcon(_T("PNG_BTREFRESH_D"));
	AddDisableImageIcon(_T("PNG_BTVERYCD_D"));

}
