/*
 * $Id: TabItem_MainButton.cpp 4848 2008-02-27 01:51:40Z fengwen $
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
#include "StdAfx.h"
#include ".\tabitem_mainbutton.h"
#include "Util.h"
#include "TabBar.h"
#include "FaceManager.h"


IMPLEMENT_DYNCREATE(CTabItem_MainButton, CTabItem)
CTabItem_MainButton::CTabItem_MainButton(void)
{
	m_TextType = TextsRight;
	m_bHasActiveIco = false;
}

CTabItem_MainButton::~CTabItem_MainButton(void)
{
}

//void CTabItem_MainButton::SetActivedIcon(HICON hIcon)
//{
//	if (m_hIconActived != hIcon)
//	{
//		if (NULL != m_hIconActived)
//			::DestroyIcon(m_hIconActived);
//		m_hIconActived = hIcon;
//	}
//}

void CTabItem_MainButton::SetActivedIcon(LPCTSTR lpszPngResource)
{
	m_bHasActiveIco = m_imgActiveIco.LoadResource(FindResource(NULL, lpszPngResource, _T("PNG")), CXIMAGE_FORMAT_PNG);
}
int	CTabItem_MainButton::GetDesireLength(void)
{
	CRect	rtCalc;
	rtCalc.SetRect(0, 0, 0, 0);
	CalcTextRect(rtCalc, m_strCaption, _T("ËÎÌו"), 14);
	return (MARGIN_HEAD + 32 + ICON_TXT_GAP + rtCalc.Width() + MARGIN_TAIL + m_iItemGap);
}

void CTabItem_MainButton::Paint(CDC* pDC)
{
	//CFontDC font(pDC->GetSafeHdc(), (int) 14);
	CFontDC font(pDC->GetSafeHdc(), _T("ËÎÌו"));
	font = 14;

	font.SetWeight(FW_BOLD);

	CRect	rect;
	COLORREF	clrShadow1, clrShadow2;

	rect = GetRect();
	rect.right -= m_iItemGap;

	COLORREF	clrOldText;
	if (m_bActive)
	{
		CFaceManager::GetInstance()->DrawImageBar(IBI_MAINBTN_A, pDC->m_hDC, rect);
		clrOldText = pDC->SetTextColor(RGB(255, 255, 255));
		clrShadow1 = RGB(80, 0, 0);
		clrShadow2 = RGB(116, 0, 0);
	}
	else
	{   
		if (IsHover())
			CFaceManager::GetInstance()->DrawImageBar(IBI_MAINBTN_H, pDC->m_hDC, rect);
		else
			CFaceManager::GetInstance()->DrawImageBar(IBI_MAINBTN_N, pDC->m_hDC, rect);

		clrOldText = pDC->SetTextColor(RGB(169, 47, 47));
		clrShadow1 = RGB(131, 131, 131);
		clrShadow2 = RGB(172, 172, 172);
	}

	int iOldMode = pDC->SetBkMode(TRANSPARENT);
	
	CRect	rtCalc(0, 0, 0, 0);
	UINT	uDTFlagAppend = 0;
	pDC->DrawText(m_strCaption, rtCalc, DT_CALCRECT | DT_VCENTER | DT_SINGLELINE | DT_CENTER);
	if (rtCalc.Width() < rect.Width())
		uDTFlagAppend = DT_CENTER;
	else
		uDTFlagAppend = 0;

	CxImage		*pImgDraw;
	pImgDraw = GetActive() ? &m_imgActiveIco : &m_imgIcon;


	switch(m_TextType)
	{
	case NoTexts:
		{
			if(NULL != pImgDraw)
			{
				pImgDraw->Draw(pDC->GetSafeHdc(),
					rect.left+(rect.Width() - pImgDraw->GetWidth() + 1)/2,
					rect.top+(rect.Height() - pImgDraw->GetHeight() + 1)/2);
			}
			break;
		}
	case TextsBelow:
		{
			if(NULL != pImgDraw)
			{
				pImgDraw->Draw(pDC->GetSafeHdc(),
								rect.left+(rect.Width() - pImgDraw->GetWidth() + 1)/2,
								rect.top+(rect.Height() - pImgDraw->GetHeight() + 1)/2);
			}
			//pDC->DrawText(m_strCaption, rect, DT_BOTTOM /*|DT_VCENTER*/ | DT_SINGLELINE  | uDTFlagAppend);
			
			if (m_bActive)
				DrawShdText(pDC->GetSafeHdc(), m_strCaption, m_strCaption.GetLength(), &rect, DT_BOTTOM | DT_SINGLELINE | uDTFlagAppend, clrShadow1, clrShadow2);
			else
				pDC->DrawText(m_strCaption, m_strCaption.GetLength(), &rect, DT_BOTTOM | DT_SINGLELINE | uDTFlagAppend);
			break;
		}
	case TextsRight:
		{
			int		iHead, iTail, iGap;
			int		iMargin;
			float	fShrinkRate;
			CRect	rtItem;
			rtItem = GetRect();
			iMargin = rtItem.Width() - 32 - rtCalc.Width();

			iHead = MARGIN_HEAD;
			iGap = ICON_TXT_GAP;
			iTail = MARGIN_TAIL;
			if (iMargin < TOTAL_GAP)
			{
				fShrinkRate = (float)iMargin / TOTAL_GAP;
				iHead	= (int) (iHead * fShrinkRate);
				iGap	= (int) (iGap * fShrinkRate);
				iTail	= (int) (iTail * fShrinkRate);
			}

			if(NULL != pImgDraw)
			{
				pImgDraw->Draw(pDC->GetSafeHdc(),
							rect.left + iHead,
							rect.top+(rect.Height()- pImgDraw->GetHeight() + 1)/2);
			}

			rect.left = rect.left + iHead + 32 + iGap;
			rect.top = rect.top + rect.Height()/2 - 4;
			uDTFlagAppend = DT_LEFT;
			//if (rtCalc.Width() < rect.Width())
			//{
			//	rect.left = rect.left + (rect.Width()-GetSystemMetrics(SM_CXICON) + 1)/2 - 2;
			//	rect.top = rect.top + rect.Height()/2 - 4;
			//	//pDC->DrawText(m_strCaption, rect, DT_CENTER /*|DT_VCENTER*/ | DT_SINGLELINE | DT_CENTER);
			//}
			//else
			//{
			//	//pDC->DrawText(m_strCaption, rect, DT_BOTTOM /*| DT_VCENTER*/ | DT_SINGLELINE);
			//}
			if (m_bActive)
				DrawShdText(pDC->GetSafeHdc(), m_strCaption, m_strCaption.GetLength(), &rect, DT_SINGLELINE | uDTFlagAppend, clrShadow1, clrShadow2);
			else
				pDC->DrawText(m_strCaption, m_strCaption.GetLength(), &rect, DT_SINGLELINE | uDTFlagAppend);
			break;
		}
	}
	
	pDC->SetTextColor(clrOldText);
	pDC->SetBkMode(iOldMode);
}

void CTabItem_MainButton::OnLButtonDown(UINT /*nFlags*/, CPoint /*point*/)
{
	m_pParentBar->SetActiveTab(m_myPos);
}

void CTabItem_MainButton::OnMouseHover(void)
{
	CTabItem::OnMouseHover();
	Invalidate();
}

void CTabItem_MainButton::OnMouseLeave(void)
{
	CTabItem::OnMouseLeave();
	Invalidate();
}
