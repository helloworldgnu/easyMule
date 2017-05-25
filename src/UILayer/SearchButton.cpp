/*
 * $Id: SearchButton.cpp 7475 2008-09-26 04:42:08Z huby $
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
// SearchButton.cpp : 实现文件
//

#include "stdafx.h"
#include "emule.h"
#include "emuleDlg.h"
#include "SearchButton.h"
#include "Util.h"
#include "SearchBarCtrl.h"
#include "CmdFuncs.h"
#include "FaceManager.h"
#include "DlgMainTabSidePanel.h"
#include "StringConversion.h"
#include "CmdFuncs.h"
#include ".\searchbutton.h"
// CSearchButton

CSearchButton::CSearchButton()
{
	m_hIcon = theApp.LoadIcon(_T("FIND"), 16, 16, LR_DEFAULTSIZE);
	m_bHover		= FALSE;
	m_bIsPressed	= FALSE;
}

CSearchButton::~CSearchButton()
{
}


BEGIN_MESSAGE_MAP(CSearchButton, CButton)
	ON_WM_MEASUREITEM()
	ON_WM_MOUSEMOVE()
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_CONTROL_REFLECT(BN_CLICKED, OnBnClicked)
	ON_WM_ERASEBKGND()
	//ON_WM_PAINT()
END_MESSAGE_MAP()

// CSearchButton 消息处理程序
BOOL CSearchButton::Create(CWnd* pParentWnd, CRect rect, LPCTSTR lpszCaption, DWORD dwStyle, UINT nID)
{
	return CButton::Create(lpszCaption, dwStyle | BS_OWNERDRAW, rect, pParentWnd, nID);
}

void CSearchButton::GetDesireSize(CSize &size)
{
	CFaceManager::GetInstance()->GetImageSize(II_SEARCHBTN_N, size);
}

void CSearchButton::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	CButton::OnMeasureItem(nIDCtl, lpMeasureItemStruct);
}

void CSearchButton::PreSubclassWindow()
{
	//ModifyStyle(BS_TYPEMASK, BS_OWNERDRAW, SWP_FRAMECHANGED);
//	ModifyStyle(BS_TYPEMASK, 0, SWP_FRAMECHANGED);

	CButton::PreSubclassWindow();
}

void CSearchButton::DrawInactive(CDC* pDC, const CRect& rect)
{
	CRect	rtFrm;
	CRect	rtFace;

	rtFrm = rect;
	rtFace = rect;

	{
		//CPenDC	penOutSide(pDC->GetSafeHdc(), RGB(171, 164, 150));
		Draw2GradLayerRect(pDC->GetSafeHdc(), rtFace, RGB(224, 240, 248), RGB(224, 240, 248), RGB(200, 216, 232), RGB(200, 216, 232), 62);
	}

	{
		CPenDC	penInSide(pDC->GetSafeHdc(), RGB(255, 255, 255), 1);

		rtFrm.left += 2;
		rtFrm.top += 2;
		rtFrm.right -= 1;
		rtFrm.bottom -= 2;

		//DrawRound(pDC->GetSafeHdc(), rtFrm, 4);
	}
}

void CSearchButton::DrawHover(CDC* pDC, const CRect& rect)
{
	CRect	rtFrm;
	CRect	rtFace;

	rtFrm = rect;
	rtFace = rect;

	{
		//CPenDC	penOutSide(pDC->GetSafeHdc(), RGB(110, 150, 199));

		//rtFace.top += 3;
		Draw2GradLayerRect(pDC->GetSafeHdc(), rtFace, RGB(208, 248, 200), RGB(208, 248, 200), RGB(160, 232, 144), RGB(160, 232, 144), 62);
		//DrawRound(pDC->GetSafeHdc(), rtFrm, 3);
	}

	{
		CPenDC	penInSide(pDC->GetSafeHdc(), RGB(255, 255, 255), 2);

		rtFrm.left += 2;
		rtFrm.top += 2;
		rtFrm.right -= 1;
		rtFrm.bottom -= 2;

		//DrawRound(pDC->GetSafeHdc(), rtFrm, 3);
	}
}

void CSearchButton::DrawPressed(CDC* pDC, const CRect& rect)
{
	CRect	rtFrm;
	CRect	rtFace;

	rtFrm = rect;
	rtFace = rect;

	{
		Draw2GradLayerRect(pDC->GetSafeHdc(), rtFace, RGB(170, 206, 227), RGB(170, 206, 227), RGB(107, 171, 207), RGB(113, 174, 209), 62);
	}

	{
		CPenDC	penInSide(pDC->GetSafeHdc(), RGB(255, 255, 255), 2);

		rtFrm.left += 2;
		rtFrm.top += 2;
		rtFrm.right -= 1;
		rtFrm.bottom -= 2;
	}
}

void CSearchButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CClientRect	rtClient(this);
	CBufferDC	bufDC(lpDrawItemStruct->hDC, rtClient);


	DrawParentBk(GetSafeHwnd(), bufDC.GetSafeHdc());
	m_bIsPressed = lpDrawItemStruct->itemState & ODS_SELECTED;

	if (m_bHover)
	{
		if (m_bIsPressed)
			CFaceManager::GetInstance()->DrawImage(II_SEARCHBTN_P, bufDC.GetSafeHdc(), rtClient);
		else
			CFaceManager::GetInstance()->DrawImage(II_SEARCHBTN_H, bufDC.GetSafeHdc(), rtClient);
	}
	else
		CFaceManager::GetInstance()->DrawImage(II_SEARCHBTN_N, bufDC.GetSafeHdc(), rtClient);

}
void CSearchButton::OnMouseMove(UINT nFlags, CPoint point)
{
	TRACKMOUSEEVENT		csTME;

	CButton::OnMouseMove(nFlags, point);

	if (!m_bHover)
	{
		m_bHover = TRUE;
		Invalidate();

		ZeroMemory(&csTME, sizeof(csTME));
		csTME.cbSize = sizeof(csTME);
		csTME.dwFlags = TME_LEAVE;
		csTME.hwndTrack = m_hWnd;
		::_TrackMouseEvent(&csTME);
	}
}

LRESULT CSearchButton::OnMouseLeave(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if (m_bHover)
	{
		m_bHover = FALSE;
		Invalidate();
	}
	return 0;
}

void CSearchButton::OnBnClicked()
{
	CString str;
	CString query;
	
	CSearchBarCtrl* SearchBarCtrl = (CSearchBarCtrl*)(&((CDlgMainTabSidePanel*)GetParent())->m_SearchBarCtrl);
	CSearchEdit* SearchEditor = SearchBarCtrl->GetEditor();
	SearchEditor->GetWindowText(str);

	
	if(!str.Compare(GetResString(IDS_SEARCH_INPUT)))
	{
		return;
	}

	switch(SearchBarCtrl->GetSearchType())
	{
		case SearchTypeEd2kGlobal:

			if(SearchEditor->m_bTipinfo || str.IsEmpty())
			{
				return;
			}
			else
			{
				if (str.Left(13).CompareNoCase(_T("ed2k://|file|")) == 0)
				{
					CmdFuncs::AddEd2kLinksToDownload(str, 0);
				}
				// VC-SearchDream[2007-04-06]: For HTTP and FTP Direct DownLoad
				else if ((str.Left(7).CompareNoCase(_T("http://")) == 0) || (str.Left(6).CompareNoCase(_T("ftp://")) == 0)) // Direct HTTP and FTP DownLoad
				{
					CmdFuncs::AddUrlToDownload(str);
				}
				else
				{
					theApp.emuledlg->m_mainTabWnd.m_dlgResource.OpenNewSearchResult(str);
				}
			}

			break;
		case SearchTypeVeryCD:

			if (str.Left(13).CompareNoCase(_T("ed2k://|file|")) == 0)
			{
				CmdFuncs::AddEd2kLinksToDownload(str, 0);
			}
			else
			{
				if ((str.Left(7).CompareNoCase(_T("http://")) == 0) || (str.Left(6).CompareNoCase(_T("ftp://")) == 0)) // Direct HTTP and FTP DownLoad
				{
					CmdFuncs::AddUrlToDownload(str);
				}
				else
				{
					if(thePrefs.m_bShowBroswer /*&& IsWindow(theApp.emuledlg->webbrowser->m_hWnd)*/ )
					{
//VC-dgkang 2008年7月8日
#ifdef _FOREIGN_VERSION
						query = thePrefs.m_strSearchPage;
#else
						query = "http://www.verycd.com/search/folders/"; 
#endif
						query += EncodeUrlUtf8(str);
						//VC-dgkang 2008年7月10日
						//VC 搜索也可以不加的，所以去掉 / 
						//query += "/";

						theApp.emuledlg->m_mainTabWnd.m_dlgResource.OpenNewUrl(query, str);
					}
					else
					{
						ShellOpenFile(query);
					}
				}
			}
			
			break;
		default:
			ASSERT(0);
	}
}

BOOL CSearchButton::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

BOOL CSearchButton::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (WM_LBUTTONDBLCLK == pMsg->message)
		pMsg->message = WM_LBUTTONDOWN;

	return CButton::PreTranslateMessage(pMsg);
}
