/*
 * $Id: ToolBarCtrlZ.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
// ToolBarCtrlZ.cpp : 实现文件
//

#include "stdafx.h"
#include "eMule.h"
#include "ToolBarCtrlZ.h"
#include ".\toolbarctrlz.h"
#include "Util.h"
#include "FaceManager.h"
#include "UserMsgs.h"
#include "StrSafe.h"


// CToolBarCtrlZ

IMPLEMENT_DYNAMIC(CToolBarCtrlZ, CToolBarCtrl)
CToolBarCtrlZ::CToolBarCtrlZ()
{
}

CToolBarCtrlZ::~CToolBarCtrlZ()
{
	CleanupAllImages();
}


BEGIN_MESSAGE_MAP(CToolBarCtrlZ, CToolBarCtrl)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	//ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnNMCustomdraw)
	ON_WM_PAINT()
	ON_MESSAGE(UM_GET_DESIRE_LENGTH, OnGetDesireLength)
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

int CToolBarCtrlZ::AddSingleString(LPCTSTR lpszText)
{
	enum {STRBUF = 512};
	TCHAR szTmpStr[STRBUF];
	int	iLen;

	::StringCchCopy(szTmpStr, STRBUF - 1, lpszText);
	iLen = _tcslen(lpszText);
	iLen = min(iLen, STRBUF - 2);
	szTmpStr[iLen + 1] = _T('\0');
	return AddStrings(szTmpStr);
}

void CToolBarCtrlZ::DrawItem(CDC *pDC, int iIndex, const CRect &rtItem, BOOL bHover)
{
	UINT			uItemId;
	UINT			uItemState;
	TBBUTTON		tbb;


	TCHAR			szText[1024];
	TBBUTTONINFO	tbi;
	CArray<CxImage*, CxImage*>	*parrImgs = NULL;
	CxImage			*pIconImg = NULL;		
	int				iIconTop;
	CRect			rtDraw;
	CRect			rtText;
	COLORREF		clrText;
	
	CClientRect		rtClient(this);
	rtDraw = rtItem;
	rtDraw.top = rtClient.top;
	rtDraw.bottom = rtClient.bottom;

	if (!GetButton(iIndex, &tbb))
		return;

	uItemId = tbb.idCommand;
	uItemState = GetState(uItemId);

	parrImgs = &m_arrImgs;
	if ( !IsButtonEnabled(uItemId) )
	{
		clrText = RGB(204, 128, 128);
		if (0 != m_arrDisableImgs.GetCount())
			parrImgs = &m_arrDisableImgs;
	}
	else
	{
		clrText = RGB(255, 254, 253);

		if (TBSTATE_PRESSED & uItemState/*IsButtonPressed(uItemId)*/)
			rtDraw.OffsetRect(1, 1);
		else if (bHover/*iIndex == GetHotItem()*/)
			rtDraw.OffsetRect(-1, -2);
	}


	ZeroMemory(&tbi, sizeof(TBBUTTONINFO));
	tbi.cbSize = sizeof(TBBUTTONINFO);
	tbi.dwMask = TBIF_TEXT | TBIF_IMAGE;
	tbi.pszText = szText;
	tbi.cchText = 1024;
	//if (GetButtonInfo(p->nmcd.dwItemSpec, &tbi))
	GetButtonInfo(uItemId, &tbi);
	{

		rtText = rtDraw;

		if (tbi.iImage < parrImgs->GetCount())
		{
			pIconImg = parrImgs->GetAt(tbi.iImage);
			if (NULL != pIconImg)
			{
				iIconTop = rtDraw.Height() - pIconImg->GetHeight();
				iIconTop /= 2;
				iIconTop += rtDraw.top;
				pIconImg->Draw(pDC->GetSafeHdc(), rtDraw.left, iIconTop);
				rtText.left += pIconImg->GetWidth() + 4;
			}
		}


		{
			CWndFontDC	fontDC(pDC->GetSafeHdc(), GetSafeHwnd());
			CTextDC		textDC(pDC->GetSafeHdc(), clrText);
			
			pDC->DrawText(tbi.pszText, -1, &rtText, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		}
	}

}

void CToolBarCtrlZ::CleanupAllImages(void)
{
	int	i,iCount;
	iCount = m_arrImgs.GetCount();
	for (i = 0; i < iCount; i++)
		delete m_arrImgs[i];
	iCount = m_arrDisableImgs.GetCount();
	for (i = 0; i < iCount; i++)
		delete m_arrDisableImgs[i];

	m_arrImgs.RemoveAll();
	m_arrDisableImgs.RemoveAll();

}

void CToolBarCtrlZ::AddDisableImageIcon(LPCTSTR lpszResource)
{
	CxImage	*pImage = new CxImage;
	pImage->LoadResource(FindResource(NULL, lpszResource, _T("PNG")), CXIMAGE_FORMAT_PNG);
	m_arrDisableImgs.Add(pImage);
}

void CToolBarCtrlZ::AddImageIcon(LPCTSTR lpszResource)
{
	CxImage	*pImage = new CxImage;
	pImage->LoadResource(FindResource(NULL, lpszResource, _T("PNG")), CXIMAGE_FORMAT_PNG);
	m_arrImgs.Add(pImage);

	// 同时增加ImageList，否则在SetButtonInfo时，大于button数的Image会被忽略。	<begin>
	CImageList *pil = GetImageList();
	if (NULL == pil)
	{
		m_ilFake.Create(16, 16, ILC_COLOR, 5, 1);
		SetImageList(&m_ilFake);
	}
	if (NULL != pil)
		pil->Add(CTempIconLoader(_T("EMPTY")));
	// 同时增加ImageList，否则在SetButtonInfo时，大于button数的Image会被忽略。	<end>
}

// CToolBarCtrlZ 消息处理程序

int CToolBarCtrlZ::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CToolBarCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;
	m_mouseMgr.Init(GetSafeHwnd(), 0);

	// TODO:  在此添加您专用的创建代码
	ModifyStyle(0, TBSTYLE_LIST | CCS_NODIVIDER | CCS_NORESIZE/* | TBSTYLE_CUSTOMERASE*/);

	return 0;
}

BOOL CToolBarCtrlZ::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CToolBarCtrlZ::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	
	CRect		rtClip;
	dc.GetClipBox(&rtClip);

	CClientRect	rtClient(this);
	CBufferDC	bufDC(dc.GetSafeHdc(), rtClient);

	if (!DrawBk(&bufDC, rtClient))
		DrawParentBk(GetSafeHwnd(), bufDC.GetSafeHdc());

	int			i;
	CRect		rtItem;
	CRect		rtInsersect;
	CPoint		ptCursor;
	BOOL		bHover;

	GetCursorPos(&ptCursor);
	ScreenToClient(&ptCursor);

	for (i = 0; i < GetButtonCount(); i++)
	{
		if (GetItemRect(i, &rtItem))
		{
			rtInsersect.IntersectRect(&rtClip, &rtItem);

			if (!rtInsersect.IsRectEmpty())
			{
				bHover = m_mouseMgr.MouseOver() && rtItem.PtInRect(ptCursor);
				DrawItem(&bufDC, i, rtItem, bHover);
			}
		}
	}
}

LRESULT CToolBarCtrlZ::OnGetDesireLength(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	return GetMaxLength() + 8;
}

int CToolBarCtrlZ::GetMaxLength()
{
	int		i;
	int		iLength;
	CRect	rtItem;

	iLength = 0;
	for (i = 0; i < GetButtonCount(); i++)
	{
		if (GetItemRect(i, &rtItem))
		{
			iLength += rtItem.Width();
		}
	}

	return iLength;
}


void CToolBarCtrlZ::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_mouseMgr.OnMouseMove(GetSafeHwnd());
	Invalidate();

	__super::OnMouseMove(nFlags, point);
}
LRESULT CToolBarCtrlZ::OnMouseLeave(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	m_mouseMgr.OnMouseOut(GetSafeHwnd());
	Invalidate();
	return 0;
}
