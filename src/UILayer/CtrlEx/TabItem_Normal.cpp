/*
 * $Id: TabItem_Normal.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
// TabItem_Normal.cpp : 实现文件
//

#include "stdafx.h"
#include "TabItem_Normal.h"
#include ".\tabitem_normal.h"
#include "Util.h"
#include "TabWnd.h"
#include "FaceManager.h"

// CTabItem_Normal

IMPLEMENT_DYNCREATE(CTabItem_Normal, CTabItem)
CTabItem_Normal::CTabItem_Normal()
{
	m_bHasIcon = false;
}

CTabItem_Normal::~CTabItem_Normal()
{
}

void CTabItem_Normal::SetRelativeWnd(HWND hRelativeWnd, HWND hRelaWndOldParent,
								   BOOL bAutoDelRelaWndObject, CWnd* pRelaWndObjectToDel)
{
	if (NULL != m_hRelaWndOldParent
		&& IsWindow(m_hRelativeWnd))
		::SetParent(m_hRelativeWnd, m_hRelaWndOldParent);

	m_hRelaWndOldParent = hRelaWndOldParent;
	m_hRelativeWnd = hRelativeWnd;
	m_bAutoDelRelaWndObject = bAutoDelRelaWndObject;
	m_pDelRelaWnd = pRelaWndObjectToDel;

	SetRelaWndParent();
}

void CTabItem_Normal::OnCreate(void)
{
	SetRelaWndParent();
	if (IsDynDesireLength()) 
		RequestResize();
}

void CTabItem_Normal::SetRelaWndParent()
{
	if ( NULL != GetParentBar()
		&& NULL != GetParentBar()->GetParentTabWnd()
		&& NULL != GetParentBar()->GetParentTabWnd()->GetSafeHwnd()
		&& IsWindow(m_hRelativeWnd))
	{
		::SetParent(m_hRelativeWnd, GetParentBar()->GetParentTabWnd()->GetSafeHwnd());
	}
}

BOOL CTabItem_Normal::GetIconRect(CRect &rect)
{
	if (!m_bHasIcon)
		return FALSE;

	rect = GetRect();
	rect.left += MARGIN_H;
	rect.top += 6;
	rect.right = rect.left + 16;
	rect.bottom = rect.top + 16;

	return TRUE;
}

BOOL CTabItem_Normal::GetTextRect(CRect &rect)
{
	rect = GetRect();
	rect.right -= m_iItemGap;
	rect.right -= MARGIN_H;

	CRect	rtIcon;
	if (GetIconRect(rtIcon))
		rect.left = rtIcon.right + 2;
	else
		rect.left += MARGIN_H;

	if (m_bActive)
		rect.top += 4;
	else
		rect.top += 8;

	return TRUE;
}

void CTabItem_Normal::DrawActiveBk(CDC* pDC, const CRect &rect)
{
	COLORREF	aclrLayers[4] =
	{
		RGB(239, 243, 249), RGB(220, 229, 241), RGB(187, 206, 229), RGB(255, 255, 255)
	};


	DrawItem(pDC, rect, RGB(110, 150, 199), RGB(255, 255, 255), aclrLayers);


	//CRect	rtFrm;
	//CRect	rtFace;

	//rtFrm = rect;
	//rtFace = rect;

	//{
	//	CPenDC	penOutSide(pDC->GetSafeHdc(), RGB(110, 150, 199));

	//	//rtFace.top += 3;
	//	Draw2GradLayerRect(pDC->GetSafeHdc(), rtFace, RGB(239, 243, 249), RGB(220, 229, 241), RGB(187, 206, 229), RGB(255, 255, 255));
	//	//DrawRound(pDC->GetSafeHdc(), rtFrm, 3);

	//	switch (GetBarPos())
	//	{
	//	case TBP_TOP:
	//	default:
	//		Draw3Borders(pDC->GetSafeHdc(), rtFrm, 3);
	//		break;
	//	case TBP_BOTTOM:
	//		Draw3Borders(pDC->GetSafeHdc(), rtFrm, 1);
	//		break;
	//	}
	//}

	//{
	//	CPenDC	penInSide(pDC->GetSafeHdc(), RGB(255, 255, 255), 1);

	//	rtFrm.left += 2;
	//	rtFrm.top += 2;
	//	rtFrm.right -= 1;
	//	rtFrm.bottom -= 2;

	//	//DrawRound(pDC->GetSafeHdc(), rtFrm, 3);

	//	switch (GetBarPos())
	//	{
	//	case TBP_TOP:
	//	default:
	//		Draw3Borders(pDC->GetSafeHdc(), rtFrm, 3);
	//		break;
	//	case TBP_BOTTOM:
	//		Draw3Borders(pDC->GetSafeHdc(), rtFrm, 1);
	//		break;
	//	}
	//}
}

void CTabItem_Normal::DrawInactiveBk(CDC* pDC, const CRect &rect)
{
	COLORREF	aclrLayers[4] =
	{
		RGB(255, 255, 255),
		RGB(239, 238, 234),
		RGB(226, 223, 215),
		RGB(242, 241, 235)
	};


	DrawItem(pDC, rect, RGB(171, 164, 150), RGB(255, 255, 255), aclrLayers);


	//CRect	rtFrm;
	//CRect	rtFace;

	//rtFrm = rect;
	//rtFace = rect;

	//{
	//	CPenDC	penOutSide(pDC->GetSafeHdc(), RGB(171, 164, 150));

	//	//rtFace.top += 3;
	//	Draw2GradLayerRect(pDC->GetSafeHdc(), rtFace, RGB(255, 255, 255), RGB(239, 238, 234), RGB(226, 223, 215), RGB(242, 241, 235));
	//	//DrawRound(pDC->GetSafeHdc(), rtFrm, 3);

	//	switch (GetBarPos())
	//	{
	//	case TBP_TOP:
	//	default:
	//		Draw3Borders(pDC->GetSafeHdc(), rtFrm, 3);
	//		break;
	//	case TBP_BOTTOM:
	//		Draw3Borders(pDC->GetSafeHdc(), rtFrm, 1);
	//		break;
	//	}
	//}

	//{
	//	CPenDC	penInSide(pDC->GetSafeHdc(), RGB(255, 255, 255), 2);

	//	rtFrm.left += 2;
	//	rtFrm.top += 2;
	//	rtFrm.right -= 1;
	//	rtFrm.bottom -= 2;

	//	//DrawRound(pDC->GetSafeHdc(), rtFrm, 3);

	//	switch (GetBarPos())
	//	{
	//	case TBP_TOP:
	//	default:
	//		Draw3Borders(pDC->GetSafeHdc(), rtFrm, 3);
	//		break;
	//	case TBP_BOTTOM:
	//		Draw3Borders(pDC->GetSafeHdc(), rtFrm, 1);
	//		break;
	//	}
	//}
}

void CTabItem_Normal::DrawHover(CDC* pDC, const CRect &rect)
{
	COLORREF	aclrLayers[4] ={RGB(255, 255, 255),
								RGB(228, 236, 254),
								RGB(205, 221, 253),
								RGB(240, 246, 255)};
	

	DrawItem(pDC, rect, RGB(126, 165, 250), RGB(255, 255, 255), aclrLayers);

	//CRect	rtFrm;
	//CRect	rtFace;

	//rtFrm = rect;
	//rtFace = rect;

	//{
	//	CPenDC	penOutSide(pDC->GetSafeHdc(), RGB(126, 165, 250));

	//	//rtFace.top += 3;
	//	Draw2GradLayerRect(pDC->GetSafeHdc(), rtFace, RGB(255, 255, 255), RGB(228, 236, 254), RGB(205, 221, 253), RGB(240, 246, 255));
	//	//DrawRound(pDC->GetSafeHdc(), rtFrm, 3);
	//	switch (GetBarPos())
	//	{
	//	case TBP_TOP:
	//	default:
	//		Draw3Borders(pDC->GetSafeHdc(), rtFrm, 3);
	//		break;
	//	case TBP_BOTTOM:
	//		Draw3Borders(pDC->GetSafeHdc(), rtFrm, 1);
	//		break;
	//	}
	//}

	//{
	//	CPenDC	penInSide(pDC->GetSafeHdc(), RGB(255, 255, 255), 1);

	//	rtFrm.left += 2;
	//	rtFrm.top += 2;
	//	rtFrm.right -= 1;
	//	rtFrm.bottom -= 2;

	//	//DrawRound(pDC->GetSafeHdc(), rtFrm, 3);
	//	switch (GetBarPos())
	//	{
	//	case TBP_TOP:
	//	default:
	//		Draw3Borders(pDC->GetSafeHdc(), rtFrm, 3);
	//		break;
	//	case TBP_BOTTOM:
	//		Draw3Borders(pDC->GetSafeHdc(), rtFrm, 1);
	//		break;
	//	}
	//}
}

void CTabItem_Normal::DrawItem(CDC* pDC, const CRect &rect, COLORREF clrFrmOutside, COLORREF clrFrmInside, COLORREF clrLayers[4])
{
	CRect	rtFrm;
	CRect	rtFace;

	rtFrm = rect;
	rtFace = rect;

	switch (GetBarPos())
	{
	case TBP_TOP:
	default:
		rtFace.top += 2;
		Draw2GradLayerRect(pDC->GetSafeHdc(), rtFace, clrLayers[0], clrLayers[1], clrLayers[2], clrLayers[3], 62);
		break;
	case TBP_BOTTOM:
		rtFace.bottom -= 2;
		Draw2GradLayerRect(pDC->GetSafeHdc(), rtFace, clrLayers[0], clrLayers[1], clrLayers[2], clrLayers[3], 62);
		break;
	}



	{
		CPenDC	penInSide(pDC->GetSafeHdc(), clrFrmInside, 1);

		switch (GetBarPos())
		{
		case TBP_TOP:
		default:
			rtFrm.left += 1;
			rtFrm.right -= 1;
			rtFrm.top += 1;

			DrawRound(pDC->GetSafeHdc(), rtFrm, 3, 2);
			//Draw3Borders(pDC->GetSafeHdc(), rtFrm, 3);
			break;
		case TBP_BOTTOM:
			rtFrm.left += 1;
			rtFrm.right -= 1;
			rtFrm.bottom -= 1;
			DrawRound(pDC->GetSafeHdc(), rtFrm, 1, 2);
			//Draw3Borders(pDC->GetSafeHdc(), rtFrm, 1);
			break;
		}
	}

	{
		CPenDC	penOutSide(pDC->GetSafeHdc(), clrFrmOutside);

		switch (GetBarPos())
		{
		case TBP_TOP:
		default:
			DrawRound(pDC->GetSafeHdc(), rect, 3, 3);
			//Draw3Borders(pDC->GetSafeHdc(), rtFrm, 3);
			break;
		case TBP_BOTTOM:
			DrawRound(pDC->GetSafeHdc(), rect, 1, 3);
			//Draw3Borders(pDC->GetSafeHdc(), rect, 1);
			break;
		}
	}

}

void CTabItem_Normal::Paint(CDC* pDC)
{
	CRect	rect;
	
	rect = GetRect();
	rect.right -= m_iItemGap;
	if (m_bActive)
		CFaceManager::GetInstance()->DrawImageBar(IBI_PAGETAB_A, pDC->m_hDC, rect);
	else
	{
		if (IsHover())
			CFaceManager::GetInstance()->DrawImageBar(IBI_PAGETAB_H, pDC->m_hDC, rect);
		else
			CFaceManager::GetInstance()->DrawImageBar(IBI_PAGETAB_N, pDC->m_hDC, rect);
	}
	if(m_bHasIcon)
	{
		if (GetIconRect(rect))
		{
			m_imgIcon.Draw(pDC->GetSafeHdc(), rect.left, rect.top);
		}
	}

	//	DrawText	<begin>
	if (GetTextRect(rect))
	{
		CFontDC font(pDC->GetSafeHdc(), _T("宋体"));
		font = 12;

		int iOldMode = pDC->SetBkMode(TRANSPARENT);
		COLORREF	clrOldText;
		if (m_bActive)
		{
			clrOldText = pDC->SetTextColor(RGB(154, 2, 1));
		}
		else
		{
			clrOldText = pDC->SetTextColor(RGB(255, 255, 255));
		}

		pDC->DrawText(m_strCaption, rect, DT_VCENTER | DT_SINGLELINE | DT_CENTER | DT_WORD_ELLIPSIS);

		pDC->SetTextColor(clrOldText);
		pDC->SetBkMode(iOldMode);
	}
	//	DrawText	<end>

}

void CTabItem_Normal::OnLButtonDown(UINT /*nFlags*/, CPoint /*point*/)
{
	m_pParentBar->SetActiveTab(m_myPos);
}

void CTabItem_Normal::OnMouseHover(void)
{
	CTabItem::OnMouseHover();
	Invalidate();
}
void CTabItem_Normal::OnMouseLeave(void)
{
	CTabItem::OnMouseLeave();
	Invalidate();
}

int	CTabItem_Normal::GetDesireLength(void)
{
	if (IsDynDesireLength())
	{
		CRect	rtCalc;
		CalcTextRect(rtCalc, m_strCaption, _T("宋体"), 14);
		int		iIconWidth;
		if (m_bHasIcon)
			iIconWidth = 0;
		else 
			iIconWidth = 16 + 4;

		return MARGIN_H + iIconWidth + rtCalc.Width() + MARGIN_H + m_iItemGap;
	}
	else
		return CTabItem::GetDesireLength();
}

void CTabItem_Normal::SetCaption(LPCTSTR lpszCaption)
{
	m_strCaption = lpszCaption; 
	CTabItem::SetCaption(lpszCaption);
	if (IsDynDesireLength()) 
		RequestResize();
}

void CTabItem_Normal::SetIcon(LPCTSTR lpszPngResource)
{
	if (NULL == lpszPngResource)
		return;

	m_bHasIcon = m_imgIcon.LoadResource(FindResource(NULL, lpszPngResource, _T("PNG")), CXIMAGE_FORMAT_PNG);
}
