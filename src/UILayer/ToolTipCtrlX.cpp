/* 
 * $Id: ToolTipCtrlX.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
#include "ToolTipCtrlX.h"
#include "OtherFunctions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define DFLT_DRAWTEXT_FLAGS	(DT_NOPREFIX | DT_END_ELLIPSIS)


IMPLEMENT_DYNAMIC(CToolTipCtrlX, CToolTipCtrl)

BEGIN_MESSAGE_MAP(CToolTipCtrlX, CToolTipCtrl)
	ON_WM_SYSCOLORCHANGE()
	ON_WM_SETTINGCHANGE()
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnNMCustomDraw)
	ON_NOTIFY_REFLECT(NM_THEMECHANGED, OnNMThemeChanged)
END_MESSAGE_MAP()

CToolTipCtrlX::CToolTipCtrlX()
{
	m_bCol1Bold = true;
	ResetSystemMetrics();
	m_dwCol1DrawTextFlags = DFLT_DRAWTEXT_FLAGS | DT_LEFT;
	m_dwCol2DrawTextFlags = DFLT_DRAWTEXT_FLAGS | DT_LEFT;
}

CToolTipCtrlX::~CToolTipCtrlX()
{
}

void CToolTipCtrlX::SetCol1DrawTextFlags(DWORD dwFlags)
{
	m_dwCol1DrawTextFlags = DFLT_DRAWTEXT_FLAGS | dwFlags;
}

void CToolTipCtrlX::SetCol2DrawTextFlags(DWORD dwFlags)
{
	m_dwCol2DrawTextFlags = DFLT_DRAWTEXT_FLAGS | dwFlags;
}

void CToolTipCtrlX::ResetSystemMetrics()
{
	m_fontBold.DeleteObject();
	m_crTooltipBkColor = GetSysColor(COLOR_INFOBK);
	m_rcScreen.left = 0;
	m_rcScreen.top = 0;
	m_rcScreen.right = GetSystemMetrics(SM_CXSCREEN);
	m_rcScreen.bottom = GetSystemMetrics(SM_CYSCREEN);
	m_iScreenWidth4 = m_rcScreen.Width() / 4;
}

void CToolTipCtrlX::OnNMThemeChanged(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	ResetSystemMetrics();
	*pResult = 0;
}

void CToolTipCtrlX::OnSysColorChange()
{
	ResetSystemMetrics();
	CToolTipCtrl::OnSysColorChange();
}

void CToolTipCtrlX::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	ResetSystemMetrics();
	CToolTipCtrl::OnSettingChange(uFlags, lpszSection);
}

void CToolTipCtrlX::OnNMCustomDraw(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTTCUSTOMDRAW pNMCD = reinterpret_cast<LPNMTTCUSTOMDRAW>(pNMHDR);
	if (pNMCD->nmcd.dwDrawStage == CDDS_PREPAINT)
	{
		CWnd* pwnd = CWnd::FromHandle(pNMCD->nmcd.hdr.hwndFrom);
		CDC* pdc = CDC::FromHandle(pNMCD->nmcd.hdc);

		CString strText;
		pwnd->GetWindowText(strText);

		CRect rcWnd;
		pwnd->GetWindowRect(&rcWnd);
		CRect rcBorder;
		rcBorder.left = pNMCD->nmcd.rc.left - rcWnd.left;
		rcBorder.top = pNMCD->nmcd.rc.top - rcWnd.top;
		rcBorder.right = rcWnd.right - pNMCD->nmcd.rc.right;
		rcBorder.bottom = rcWnd.bottom - pNMCD->nmcd.rc.bottom;

		if (m_bCol1Bold && m_fontBold.m_hObject == NULL) 
		{
			CFont* pFont = pwnd->GetFont();
			if (pFont) {
				LOGFONT lf;
				pFont->GetLogFont(&lf);
				lf.lfWeight = FW_BOLD;
				VERIFY( m_fontBold.CreateFontIndirect(&lf) );
			}
		}

		int iTextHeight = 0;
		int iMaxCol1Width = 0;
		int iMaxCol2Width = 0;
		int iMaxSingleLineWidth = 0;
		CSize sizText(0);
		int iPos = 0;
		while (iPos != -1)
		{
			CString strLine = GetNextString(strText, _T('\n'), iPos);
			int iColon = strLine.Find(_T(':'));
			if (iColon != -1) {
				CFont* pOldFont = m_bCol1Bold ? pdc->SelectObject(&m_fontBold) : NULL;
				CSize siz = pdc->GetTextExtent(strLine, iColon + 1);
				if (pOldFont)
					pdc->SelectObject(pOldFont);
				iMaxCol1Width = max(iMaxCol1Width, siz.cx);
				iTextHeight = siz.cy; // update height with 'col1' string, because 'col2' string might be empty and therefore has no height
				sizText.cy += siz.cy;

				LPCTSTR pszCol2 = (LPCTSTR)strLine + iColon + 1;
				while (_istspace(*pszCol2))
					pszCol2++;
				if (*pszCol2 != _T('\0')) {
					siz = pdc->GetTextExtent(pszCol2, ((LPCTSTR)strLine + strLine.GetLength()) - pszCol2);
					iMaxCol2Width = max(iMaxCol2Width, siz.cx);
				}
			}
			else if (!strLine.IsEmpty()) {
				CSize siz = pdc->GetTextExtent(strLine);
				iMaxSingleLineWidth = max(iMaxSingleLineWidth, siz.cx);
				sizText.cy += siz.cy;
			}
			else {
				CSize siz = pdc->GetTextExtent(_T(" "), 1);
				sizText.cy += siz.cy;
			}
		}
		iMaxCol1Width = min(m_iScreenWidth4, iMaxCol1Width);
		iMaxCol2Width = min(m_iScreenWidth4*2, iMaxCol2Width);

		const int iMiddleMargin = 6;
		iMaxSingleLineWidth = max(iMaxSingleLineWidth, iMaxCol1Width + iMiddleMargin + iMaxCol2Width);
		sizText.cx = iMaxSingleLineWidth;

		rcWnd.right = rcWnd.left + rcBorder.left + sizText.cx + rcBorder.right;
		rcWnd.bottom = rcWnd.top + rcBorder.top + sizText.cy + rcBorder.bottom;

		if (rcWnd.left >= m_rcScreen.left) {
			if (rcWnd.right > m_rcScreen.right && rcWnd.Width() <= m_rcScreen.Width())
				rcWnd.OffsetRect(-(rcWnd.right - m_rcScreen.right), 0);
		}
		if (rcWnd.top >= m_rcScreen.top) {
			if (rcWnd.bottom > m_rcScreen.bottom && rcWnd.Height() <= m_rcScreen.Height())
				rcWnd.OffsetRect(0, -(rcWnd.bottom - m_rcScreen.bottom));
		}

		pwnd->MoveWindow(&rcWnd);

		pwnd->ScreenToClient(&rcWnd);
		pdc->FillSolidRect(&rcWnd, m_crTooltipBkColor);

		CPoint ptText(pNMCD->nmcd.rc.left, pNMCD->nmcd.rc.top);
		iPos = 0;
		while (iPos != -1)
		{
			CString strLine = GetNextString(strText, _T('\n'), iPos);
			int iColon = strLine.Find(_T(':'));
			if (iColon != -1) {
				CRect rcDT(ptText.x, ptText.y, ptText.x + iMaxCol1Width, ptText.y + iTextHeight);
				// don't draw empty <col1> strings (they are still handy to use for skipping the <col1> space)
				if (iColon > 0) {
					CFont* pOldFont = m_bCol1Bold ? pdc->SelectObject(&m_fontBold) : NULL;
					pdc->DrawText(strLine, iColon + 1, &rcDT, m_dwCol1DrawTextFlags);
					if (pOldFont)
						pdc->SelectObject(pOldFont);
				}

				LPCTSTR pszCol2 = (LPCTSTR)strLine + iColon + 1;
				while (_istspace(*pszCol2))
					pszCol2++;
				if (*pszCol2 != _T('\0')) {
					rcDT.left = ptText.x + iMaxCol1Width + iMiddleMargin;
					rcDT.right = rcDT.left + iMaxCol2Width;
					pdc->DrawText(pszCol2, ((LPCTSTR)strLine + strLine.GetLength()) - pszCol2, &rcDT, m_dwCol2DrawTextFlags);
				}

				ptText.y += iTextHeight;
			}
			else {
				CSize siz = pdc->TabbedTextOut(ptText.x, ptText.y, strLine, 0, NULL, 0);
				ptText.y += siz.cy;
			}
		}

		*pResult = CDRF_SKIPDEFAULT;
		return;
	}

	*pResult = CDRF_DODEFAULT;
}
