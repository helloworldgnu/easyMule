/* 
 * $Id: ClosableTabCtrl.cpp 4483 2008-01-02 09:19:06Z soarchin $
 * 
 * this file is part of eMule
 * Copyright (C)2002-2006 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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

#include "stdafx.h"
#include "emule.h"
#include "ClosableTabCtrl.h"
#include "OtherFunctions.h"
#include "MenuCmds.h"
#include "UserMsgs.h"
#include "VisualStylesXP.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// _WIN32_WINNT >= 0x0501 (XP only)
#define _WM_THEMECHANGED                0x031A	
#define _ON_WM_THEMECHANGED()														\
	{	_WM_THEMECHANGED, 0, 0, 0, AfxSig_l,										\
		(AFX_PMSG)(AFX_PMSGW)														\
		(static_cast< LRESULT (AFX_MSG_CALL CWnd::*)(void) > (_OnThemeChanged))		\
	},

///////////////////////////////////////////////////////////////////////////////
// CClosableTabCtrl

IMPLEMENT_DYNAMIC(CClosableTabCtrl, CTabCtrl)

BEGIN_MESSAGE_MAP(CClosableTabCtrl, CTabCtrl)
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MBUTTONUP()
	ON_WM_CREATE()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_CONTEXTMENU()
	_ON_WM_THEMECHANGED()
END_MESSAGE_MAP()

CClosableTabCtrl::CClosableTabCtrl()
{
	m_bCloseable = true;
	memset(&m_iiCloseButton, 0, sizeof m_iiCloseButton);
	m_ptCtxMenu.SetPoint(-1, -1);
}

CClosableTabCtrl::~CClosableTabCtrl()
{
}

void CClosableTabCtrl::GetCloseButtonRect(const CRect& rcItem, CRect& rcCloseButton)
{
	rcCloseButton.top = rcItem.top + 2;
	rcCloseButton.bottom = rcCloseButton.top + (m_iiCloseButton.rcImage.bottom - m_iiCloseButton.rcImage.top);
	rcCloseButton.right = rcItem.right - 2;
	rcCloseButton.left = rcCloseButton.right - (m_iiCloseButton.rcImage.right - m_iiCloseButton.rcImage.left);
}

int CClosableTabCtrl::GetTabUnderPoint(CPoint point) const
{
	int iTabs = GetItemCount();
	for (int i = 0; i < iTabs; i++)
	{
		CRect rcItem;
		GetItemRect(i, rcItem);
		if (rcItem.PtInRect(point))
			return i;
	}
	return -1;
}

int CClosableTabCtrl::GetTabUnderContextMenu() const
{
	if (m_ptCtxMenu.x == -1 || m_ptCtxMenu.y == -1)
		return -1;
	return GetTabUnderPoint(m_ptCtxMenu);
}

void CClosableTabCtrl::OnMButtonUp(UINT nFlags, CPoint point)
{
	if (m_bCloseable)
	{
		int iTab = GetTabUnderPoint(point);
		if (iTab != -1) {
			GetParent()->SendMessage(UM_CLOSETAB, (WPARAM)iTab);
			return;
		}
	}

	CTabCtrl::OnMButtonUp(nFlags, point);
}

void CClosableTabCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_bCloseable)
	{
		int iTab = GetTabUnderPoint(point);
		if (iTab != -1)
		{
			CRect rcItem;
			GetItemRect(iTab, rcItem);
			CRect rcCloseButton;
			GetCloseButtonRect(rcItem, rcCloseButton);
			rcCloseButton.top -= 2;
			rcCloseButton.left -= 4;
			rcCloseButton.right += 2;
			rcCloseButton.bottom += 4;
			if (rcCloseButton.PtInRect(point)) {
				GetParent()->SendMessage(UM_CLOSETAB, (WPARAM)iTab);
				return;
			}
		}
	}
	
	CTabCtrl::OnLButtonUp(nFlags, point);
}

void CClosableTabCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	int iTab = GetTabUnderPoint(point);
	if (iTab != -1) {
		GetParent()->SendMessage(UM_DBLCLICKTAB, (WPARAM)iTab);
		return;
	}
	CTabCtrl::OnLButtonDblClk(nFlags, point);
}

void CClosableTabCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CRect rect = lpDrawItemStruct->rcItem;
	int nTabIndex = lpDrawItemStruct->itemID;
	if (nTabIndex < 0)
		return;
	BOOL bSelected = (nTabIndex == GetCurSel());

	TCHAR szLabel[256];
	TC_ITEM tci;
	tci.mask = TCIF_TEXT | TCIF_IMAGE | TCIF_STATE;
	tci.pszText = szLabel;
	tci.cchTextMax = ARRSIZE(szLabel);
	tci.dwStateMask = TCIS_HIGHLIGHTED;
	if (!GetItem(nTabIndex, &tci))
		return;

	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	if (!pDC)
		return;

	if (!g_xpStyle.IsThemeActive() || !g_xpStyle.IsAppThemed())
		pDC->FillSolidRect(&lpDrawItemStruct->rcItem, GetSysColor(COLOR_BTNFACE));

	int iOldBkMode = pDC->SetBkMode(TRANSPARENT);

	// Draw image on left side
	CImageList* piml = GetImageList();
	if (tci.iImage >= 0 && piml && piml->m_hImageList)
	{
		IMAGEINFO ii;
		piml->GetImageInfo(0, &ii);
		rect.left += bSelected ? 8 : 4;
		piml->Draw(pDC, tci.iImage, CPoint(rect.left, rect.top + 2), ILD_TRANSPARENT);
		rect.left += (ii.rcImage.right - ii.rcImage.left);
		if (!bSelected)
			rect.left += 4;
	}

	bool bCloseable = m_bCloseable;
	if (bCloseable && GetParent()->SendMessage(UM_QUERYTAB, nTabIndex))
		bCloseable = false;

	// Draw 'Close button' at right side
	if (bCloseable && m_ImgLstCloseButton.m_hImageList)
	{
		CRect rcCloseButton;
		GetCloseButtonRect(rect, rcCloseButton);
		m_ImgLstCloseButton.Draw(pDC, 0, rcCloseButton.TopLeft(), ILD_TRANSPARENT);
		rect.right = rcCloseButton.left - 2;
	}

	COLORREF crOldColor = RGB(0, 0, 0);
	if (tci.dwState & TCIS_HIGHLIGHTED)
		crOldColor = pDC->SetTextColor(RGB(192, 0, 0));

	rect.top += 4;
	pDC->DrawText(szLabel, rect, DT_SINGLELINE | DT_TOP | DT_CENTER | DT_NOPREFIX);

	if (tci.dwState & TCIS_HIGHLIGHTED)
		pDC->SetTextColor(crOldColor);
	pDC->SetBkMode(iOldBkMode);
}

void CClosableTabCtrl::PreSubclassWindow()
{
	CTabCtrl::PreSubclassWindow();
	InternalInit();
}

int CClosableTabCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CTabCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;
	InternalInit();
	return 0;
}

void CClosableTabCtrl::InternalInit()
{
	ModifyStyle(0, TCS_OWNERDRAWFIXED);
	SetAllIcons();
}

void CClosableTabCtrl::OnSysColorChange()
{
	CTabCtrl::OnSysColorChange();
	SetAllIcons();
}

void CClosableTabCtrl::SetAllIcons()
{
	if (m_bCloseable)
	{
		m_ImgLstCloseButton.DeleteImageList();
		m_ImgLstCloseButton.Create(8, 8, theApp.m_iDfltImageListColorFlags | ILC_MASK, 0, 1);
		m_ImgLstCloseButton.SetBkColor(CLR_NONE);
		m_ImgLstCloseButton.Add(CTempIconLoader(_T("CloseTab"), 8, 8));
		m_ImgLstCloseButton.GetImageInfo(0, &m_iiCloseButton);
		Invalidate();
	}
}

void CClosableTabCtrl::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	if (m_bCloseable)
	{
		CMenu menu;
		menu.CreatePopupMenu();
		menu.AppendMenu(MF_STRING, MP_REMOVE, GetResString(IDS_FD_CLOSE));
		m_ptCtxMenu = point;
		ScreenToClient(&m_ptCtxMenu);
		menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
	}
}

BOOL CClosableTabCtrl::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == MP_REMOVE)
	{
		if (m_ptCtxMenu.x != -1 && m_ptCtxMenu.y != -1)
		{
			int iTab = GetTabUnderPoint(m_ptCtxMenu);
			if (iTab != -1) {
				GetParent()->SendMessage(UM_CLOSETAB, (WPARAM)iTab);
				return TRUE;
			}
		}
	}
	return CTabCtrl::OnCommand(wParam, lParam);
}

LRESULT CClosableTabCtrl::_OnThemeChanged()
{
	// Owner drawn tab control seems to have troubles with updating itself due to an XP theme change..
	ModifyStyle(TCS_OWNERDRAWFIXED, 0);	// Reset control style to not-owner drawn
    Default();							// Process original WM_THEMECHANGED message
	ModifyStyle(0, TCS_OWNERDRAWFIXED);	// Apply owner drawn style again
	return 0;
}
