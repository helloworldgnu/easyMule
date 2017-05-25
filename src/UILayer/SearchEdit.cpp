/*
 * $Id: SearchEdit.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
// SearchEditor.cpp : 实现文件
//

#include "stdafx.h"
#include "SearchEdit.h"
#include "resource.h"
#include "otherfunctions.h"
#include "SearchParams.h"
#include "CmdFuncs.h"
#include "emuledlg.h"
#include "StringConversion.h"
#include "SearchButton.h"
#include ".\searchedit.h"

// CSearchEditor

IMPLEMENT_DYNAMIC(CSearchEdit, CEdit)
CSearchEdit::CSearchEdit()
{
	m_bTipinfo = TRUE;
	m_Font.CreateFont(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,DEFAULT_PITCH|FF_DONTCARE, _T("宋体"));
	m_bFocusing = FALSE;
}

CSearchEdit::~CSearchEdit()
{
}

BEGIN_MESSAGE_MAP(CSearchEdit, CEdit)
	ON_CONTROL_REFLECT(EN_KILLFOCUS, OnEnKillfocus)
	ON_CONTROL_REFLECT(EN_SETFOCUS, OnEnSetfocus)
	ON_WM_CREATE()
	ON_WM_LBUTTONUP()
//	ON_WM_CHAR()
END_MESSAGE_MAP()


BOOL CSearchEdit::Create(CWnd* pParentWnd, CRect rect, DWORD dwStyle, UINT nID)
{
	dwStyle = dwStyle | ES_AUTOHSCROLL;
	return CEdit::Create(dwStyle, rect, pParentWnd, nID);
}

void CSearchEdit::OnEnKillfocus()
{
	CString str;
	GetWindowText(str);

	str.Remove(' ');

	if (str == _T(""))
	{
		SetWindowText(GetResString(IDS_SEARCH_INPUT));
		m_bTipinfo = TRUE;
	}
}

void CSearchEdit::OnEnSetfocus()
{
	if (m_bTipinfo)
	{
		SetWindowText(_T(""));
		m_bTipinfo = FALSE;
	}
	m_bFocusing = TRUE;
}

void CSearchEdit::Localize()
{
	if (m_bTipinfo)
	{
		SetWindowText(GetResString(IDS_SEARCH_INPUT));
	}

}
int CSearchEdit::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (__super::OnCreate(lpCreateStruct) == -1)
		return -1;

	SetFont(&m_Font);
	SetWindowText(GetResString(IDS_SEARCH_INPUT));

	return 0;
}

void CSearchEdit::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_bFocusing)
	{
		m_bFocusing = FALSE;
		SetSel(0, -1);
	}

	__super::OnLButtonUp(nFlags, point);
}
