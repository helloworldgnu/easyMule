/*
 * $Id: TbcDownload.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
// TbcDownload.cpp : 实现文件
//

#include "stdafx.h"
#include "TbcDownload.h"
#include ".\tbcdownload.h"
#include "emule.h"
#include "MenuCmds.h"
#include "CIF.h"


// CTbcDownload

IMPLEMENT_DYNAMIC(CTbcDownload, CToolBarCtrlZ)
CTbcDownload::CTbcDownload()
{
}

CTbcDownload::~CTbcDownload()
{
}


BEGIN_MESSAGE_MAP(CTbcDownload, CToolBarCtrlZ)
	ON_WM_CREATE()
END_MESSAGE_MAP()

void CTbcDownload::Localize()
{
	ToolBarCtrl_SetText(this, 0, GetResString(IDS_NEW));
	ToolBarCtrl_SetText(this, 1, GetResString(IDS_OPENFOLDER));
	ToolBarCtrl_SetText(this, 2, GetResString(IDS_RESUME));
	ToolBarCtrl_SetText(this, 3, GetResString(IDS_PAUSE));
	ToolBarCtrl_SetText(this, 4, GetResString(IDS_STOP));
	ToolBarCtrl_SetText(this, 5, GetResString(IDS_DELETE_FILE));

	if (NULL != GetParent())
		GetParent()->SendMessage(WM_SIZE);
}

void CTbcDownload::InitImageList()
{
	AddImageIcon(_T("PNG_TBNEWTASK"));
	AddImageIcon(_T("PNG_TBOPENFOLDER"));
	AddImageIcon(_T("PNG_TBSTART"));
	AddImageIcon(_T("PNG_TBPAUSE"));
	AddImageIcon(_T("PNG_TBSTOP"));
	AddImageIcon(_T("PNG_TBREMOVE"));

	AddDisableImageIcon(_T("PNG_TBNEWTASK_D"));
	AddDisableImageIcon(_T("PNG_TBOPENFOLDER_D"));
	AddDisableImageIcon(_T("PNG_TBSTART_D"));
	AddDisableImageIcon(_T("PNG_TBPAUSE_D"));
	AddDisableImageIcon(_T("PNG_TBSTOP_D"));
	AddDisableImageIcon(_T("PNG_TBREMOVE_D"));
}

// CTbcDownload 消息处理程序


int CTbcDownload::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CToolBarCtrlZ::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码
	InitImageList();

	AddSingleString(GetResString(IDS_NEW));
	AddSingleString(GetResString(IDS_OPENFOLDER));
	AddSingleString(GetResString(IDS_RESUME));
	AddSingleString(GetResString(IDS_PAUSE));
	AddSingleString(GetResString(IDS_STOP));
	AddSingleString(GetResString(IDS_DELETE_FILE));

	
	TBBUTTON	tbb[BUTTON_COUNT];
	CString		str;

	tbb[0].idCommand = MP_NEW;
	tbb[1].idCommand = MP_OPENFOLDER;
	tbb[2].idCommand = MP_RESUME;
	tbb[3].idCommand = MP_PAUSE;
	tbb[4].idCommand = MP_STOP;
	tbb[5].idCommand = MP_CANCEL;

	int	i = 0;
	for (i = 0; i < BUTTON_COUNT; i++)
	{
		tbb[i].iString = i;
		tbb[i].iBitmap = i;
		tbb[i].fsState = 0;
		tbb[i].fsStyle = TBSTYLE_BUTTON | BTNS_AUTOSIZE;
	}
	AddButtons(BUTTON_COUNT, tbb);

	EnableButton(MP_NEW);
	//EnableButton(MP_OPENFOLDER);

	Localize();

	return 0;
}
