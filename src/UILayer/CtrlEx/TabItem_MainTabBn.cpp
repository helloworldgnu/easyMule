/*
 * $Id: TabItem_MainTabBn.cpp 6511 2008-08-04 07:30:17Z huby $
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
// TabItem_MainTabBn.cpp : 实现文件
//

#include "stdafx.h"
#include "TabItem_MainTabBn.h"
#include "Util.h"
#include "CmdFuncs.h"
#include "Resource.h"
#include "otherfunctions.h"
#include "CmdMainTabOp.h"
#include "FaceManager.h"
#include "emuleDlg.h"
#include "CmdFuncs.h"
#include "CreditsDlg.h"


// CTabItem_MainTabBn
IMPLEMENT_DYNCREATE(CTabItem_MainTabBn, CTabItem)

CTabItem_MainTabBn::CTabItem_MainTabBn()
{
	SetAttribute(ATTR_FIXLEN);
}

CTabItem_MainTabBn::~CTabItem_MainTabBn()
{
}


// CTabItem_MainTabBn 成员函数

void CTabItem_MainTabBn::Paint(CDC* pDC)
{
	//CRect		rtDraw(GetRectInbar());
	//CBufferDC	bufDC(pDC->GetSafeHdc(), rtDraw);

	//CBrush brush(RGB(200, 200, 200));
	//bufDC.FillRect(&rtDraw, &brush);

	//int iOldMode = pDC->GetBkMode();
	//bufDC.SetBkMode(TRANSPARENT);
	//bufDC.DrawText(m_strCaption, rtDraw, DT_VCENTER | DT_SINGLELINE | DT_CENTER);
	//bufDC.SetBkMode(iOldMode);
	//DrawRound(pDC->GetSafeHdc(), rtDraw, 3);

	/*CFontDC font(pDC->GetSafeHdc(), _T("Wingdings 3"));
	font = 12;*/

	CRect	rect;

	rect = GetRect();
	rect.right -= m_iItemGap;

	//if (m_bActive)
	//{
	//	DrawActiveBk(pDC, rect);
	//}
	//else
	//{   
	//	if(IsHover())
	//	{
	//		DrawHover(pDC, rect);
	//	}
	//	else
	//	{
	//		DrawInactiveBk(pDC, rect);
	//	}
	//}

	CSize	sizeIcon;
	CRect	rtIcon;
	CFaceManager::GetInstance()->GetImageSize(II_MAINTABMORE_N, sizeIcon);
	CenterRect(&rtIcon, rect, sizeIcon);
	rtIcon.OffsetRect(0, 3);

	if (IsHover())
	{
		CFaceManager::GetInstance()->DrawImageBar(IBI_MAINBTN_H, pDC->m_hDC, rect);
		CFaceManager::GetInstance()->DrawImage(II_MAINTABMORE_H, pDC->m_hDC, rtIcon);
	}
	else
	{
		CFaceManager::GetInstance()->DrawImageBar(IBI_MAINBTN_N, pDC->m_hDC, rect);
		CFaceManager::GetInstance()->DrawImage(II_MAINTABMORE_N, pDC->m_hDC, rtIcon);
	}

	
	//{
	//	CPenDC	penDC(pDC->GetSafeHdc(), RGB(255, 255, 255));
	//	DrawTriangle(pDC, rect);
	//}
}
void CTabItem_MainTabBn::OnLButtonDown(UINT /*nFlags*/, CPoint /*point*/)
{
	CRect	rtItem(GetRectInScreen());

	CCmdMainTabOp	cmdMainTabOp;

	/*CMenu	file;
	file.CreateMenu();
	file.AppendMenu(MF_STRING, 8, GetResString(IDS_NEWTASK));
	file.AppendMenu(MF_SEPARATOR);
	file.AppendMenu(MF_STRING, 3, GetResString(IDS_IMPORT_UNFINISHED));*/

	CMenu tool;
	tool.CreateMenu();
	tool.AppendMenu(MF_STRING, 8, GetResString(IDS_NEWTASK));
	tool.AppendMenu(MF_STRING, 3, GetResString(IDS_IMPORT_UNFINISHED));
	

	CMenu   help;
	help.CreateMenu();
	help.AppendMenu(MF_STRING, 2, GetResString(IDS_HELP_));
	//help.AppendMenu(MF_STRING, 5, GetResString(IDS_BUGREPORT));

#ifndef _FOREIGN_VERSION
	help.AppendMenu(MF_STRING, 6, GetResString(IDS_NEWVERSIONREPORT));
#endif

	help.AppendMenu(MF_SEPARATOR);
	help.AppendMenu(MF_STRING, 4, GetResString(IDS_VERSIONCHECK));
	help.AppendMenu(MF_SEPARATOR);
	help.AppendMenu(MF_STRING, 9, GetResString(IDS_ABOUTBOX));

	CMenu	menu;
	menu.CreatePopupMenu();
	/*menu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)file.m_hMenu, GetResString(IDS_FILE));*/
	menu.AppendMenu(MF_STRING, 1, GetResString(IDS_EM_PREFS));
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)tool.m_hMenu, GetResString(IDS_TOOLS));
	menu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)help.m_hMenu, GetResString(IDS_HELP));
	menu.AppendMenu(MF_SEPARATOR);

	const static int SHOW_TAB_CMD_START = 7;

	if (cmdMainTabOp.IsTabShowed(CMainTabWnd::TI_ADVANCE))
		menu.AppendMenu(MF_STRING | MF_CHECKED, SHOW_TAB_CMD_START, GetResString(IDS_ADVANCE_));
	else
		menu.AppendMenu(MF_STRING, SHOW_TAB_CMD_START, GetResString(IDS_ADVANCE_));


	int iRet = menu.TrackPopupMenu(TPM_RETURNCMD, rtItem.left, rtItem.bottom, ::AfxGetMainWnd());

	menu.DestroyMenu();

	switch(iRet)
	{
	case 1:
		CmdFuncs::OpenPreferencesWnd();
		break;
	case 2:
		CmdFuncs::GotoGuide();
		break;
	case 3:
		CmdFuncs::ImportUnfinishedTasks();
		break;
	case 4:
		theApp.emuledlg->DoVersioncheck(true);
		break;
	case 5:
		CmdFuncs::OpenNewUrl(_T("http://www.verycd.com/groups/eMuleBug"), GetResString(IDS_BUGREPORT));
		break;
	case 6:
		CmdFuncs::OpenNewUrl(_T("http://www.verycd.com/groups/eMuleBeta/"), GetResString(IDS_NEWVERSIONREPORT));
		break;
	case SHOW_TAB_CMD_START:
		if (cmdMainTabOp.IsTabShowed(CMainTabWnd::TI_ADVANCE))
		{
			cmdMainTabOp.RemoveTabById(CMainTabWnd::TI_ADVANCE);
		}
		else
		{
			cmdMainTabOp.AddTabById(CMainTabWnd::TI_ADVANCE);
		}
		break;
	case 8:
		CmdFuncs::PopupNewTaskDlg();
		break;
	case 9:
		{
			CCreditsDlg dlgAbout;
			dlgAbout.DoModal();
			break;
		}
	default:
		break;
	}

}

void CTabItem_MainTabBn::DrawActiveBk(CDC* pDC, const CRect &rect)
{
	CRect	rtFrm;
	CRect	rtFace;

	rtFrm = rect;
	rtFace = rect;

	{
		CPenDC	penOutSide(pDC->GetSafeHdc(), RGB(110, 150, 199));

		rtFace.top += 3;
		Draw2GradLayerRect(pDC->GetSafeHdc(), rtFace, RGB(239, 243, 249), RGB(220, 229, 241), RGB(187, 206, 229), RGB(255, 255, 255), 62);
		DrawRound(pDC->GetSafeHdc(), rtFrm, 3);
	}

	{
		CPenDC	penInSide(pDC->GetSafeHdc(), RGB(255, 255, 255), 2);

		rtFrm.left += 2;
		rtFrm.top += 2;
		rtFrm.right -= 1;
		rtFrm.bottom -= 2;

		DrawRound(pDC->GetSafeHdc(), rtFrm, 3);
	}
}

void CTabItem_MainTabBn::DrawInactiveBk(CDC* pDC, const CRect &rect)
{
	CRect	rtFrm;
	CRect	rtFace;

	rtFrm = rect;
	rtFace = rect;

	{
		CPenDC	penOutSide(pDC->GetSafeHdc(), RGB(171, 164, 150));

		rtFace.top += 3;
		Draw2GradLayerRect(pDC->GetSafeHdc(), rtFace, RGB(255, 255, 255), RGB(239, 238, 234), RGB(226, 223, 215), RGB(242, 241, 235), 62);
		DrawRound(pDC->GetSafeHdc(), rtFrm, 3);
	}

	{
		CPenDC	penInSide(pDC->GetSafeHdc(), RGB(255, 255, 255), 2);

		rtFrm.left += 2;
		rtFrm.top += 2;
		rtFrm.right -= 1;
		rtFrm.bottom -= 2;

		DrawRound(pDC->GetSafeHdc(), rtFrm, 3);
	}
}

void CTabItem_MainTabBn::OnMouseHover(void)
{
	CTabItem::OnMouseHover();
	Invalidate();
}

void CTabItem_MainTabBn::OnMouseLeave(void)
{
	CTabItem::OnMouseLeave();
	Invalidate();
}

void CTabItem_MainTabBn::DrawHover(CDC* pDC, const CRect &rect)
{
	CRect	rtFrm;
	CRect	rtFace;

	rtFrm = rect;
	rtFace = rect;

	{
		CPenDC	penOutSide(pDC->GetSafeHdc(), RGB(126, 165, 250));

		rtFace.top += 3;
		Draw2GradLayerRect(pDC->GetSafeHdc(), rtFace, RGB(255, 255, 255), RGB(228, 236, 254), RGB(205, 221, 253), RGB(240, 246, 255), 62);
		DrawRound(pDC->GetSafeHdc(), rtFrm, 3);
	}

	{
		CPenDC	penInSide(pDC->GetSafeHdc(), RGB(255, 255, 255), 2);

		rtFrm.left += 2;
		rtFrm.top += 2;
		rtFrm.right -= 1;
		rtFrm.bottom -= 2;

		DrawRound(pDC->GetSafeHdc(), rtFrm, 3);
	}
}

void CTabItem_MainTabBn::DrawTriangle(CDC* pDC, CRect &rect)
{
	rect.left += rect.Width()/2 - 2;
	rect.top += rect.Height()/2 + 3;

	pDC->MoveTo(rect.left, rect.top);
	pDC->LineTo(rect.left + 5, rect.top);

	pDC->MoveTo(rect.left + 1, rect.top + 1);
	pDC->LineTo(rect.left + 4, rect.top + 1);

	pDC->MoveTo(rect.left + 2, rect.top + 2);
	pDC->LineTo(rect.left + 3, rect.top + 2);
}

