/*
 * $Id: TabBar.cpp 6243 2008-07-15 10:49:49Z huby $
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
// TabBar.cpp : 实现文件
//

#include "stdafx.h"
#include "TabBar.h"
#include ".\tabbar.h"
#include "TabWnd.h"

#include "Resizer.h"
#include "Util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CTabBar

IMPLEMENT_DYNAMIC(CTabBar, CWnd)
CTabBar::CTabBar()
{
	m_pParentTabWnd		= NULL;

	m_posCurActive		= NULL;
	m_posLastHoverItem  = NULL;

	m_eBarPos = TBP_TOP;
	m_iBreadth = 30;
}

CTabBar::~CTabBar()
{
	RemoveAllTabs();
}


BEGIN_MESSAGE_MAP(CTabBar, CWnd)
	ON_WM_PAINT()
	//ON_WM_ERASEBKGND()
	ON_MESSAGE(WM_ERASEBKGND, OnEraseBkgndEx)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_SIZE()
	ON_WM_MOUSEMOVE()
	ON_WM_CREATE()
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXT,0,0xFFFF,OnToolTipNotify)
	ON_WM_KEYDOWN()
	ON_WM_GETDLGCODE()

END_MESSAGE_MAP()


BOOL CTabBar::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	RegisterSimpleWndClass();
	//return CWnd::CreateEx(WS_EX_CONTROLPARENT, _T("SimpleWnd"), NULL, dwStyle, rect, pParentWnd, nID);
	return CWnd::Create(_T("SimpleWnd"), NULL, dwStyle | WS_TABSTOP, rect, pParentWnd, nID);
}

void CTabBar::GetBarMarginLogic(CRect &rect)
{
	rect = m_resizer.GetBarMarginLogic();
}
void CTabBar::SetBarMarginLogic(const CRect &rect)
{
	m_resizer.SetBarMarginLogic(rect);
}

void CTabBar::InitNewItem(CTabItem *pItem, POSITION posItem, POSITION posInsertBeside, BOOL bAfter)
{
	pItem->Create(this, posItem);
	pItem->AttachToResizer(&m_resizer);
	if (NULL != posInsertBeside)
	{
		m_resizer.MoveResizeeTo(pItem, m_Items.GetAt(posInsertBeside), bAfter);
	}


	if (NULL == m_posCurActive)		// this tab should be activated
		SetActiveTab(posItem);
	else
		pItem->ShowRelativeWnd(FALSE);

	//CClientRect	rtBarLogic(this);
	//CRect	rtItem(0, 0, 0, 0);
	//m_resizer.CalRectByPrev(rtBarLogic, GetBarPos(), pItem, rtItem);
	//pItem->SetRect(rtItem);

	//CRect		rtItemLogic = rtItem;
	//TabWnd::Real2LogicSolid(rtBarLogic, GetBarPos());
	//TabWnd::Real2LogicSolid(rtItemLogic, GetBarPos());

	ResizeAllTab();

	//if (rtItemLogic.right > rtBarLogic.right - m_rtBarMarginLogic.right)
	//	ResizeAllTab();
}

POSITION CTabBar::AddTab(CTabItem *pItem, BOOL bSetActive, POSITION posInsertBeside, BOOL bAfter)
{
	BOOL	bNeedResizeAll = TRUE;
	POSITION posNewItem = NULL;
	if (bAfter)
	{
		if (NULL == posInsertBeside)
		{
			bNeedResizeAll = FALSE;
			posInsertBeside = m_Items.GetTailPosition();
		}

		posNewItem = m_Items.InsertAfter(posInsertBeside, pItem);
	}
	else
	{
		if (NULL == posInsertBeside)
			posInsertBeside = m_Items.GetHeadPosition();

		posNewItem = m_Items.InsertBefore(posInsertBeside, pItem);
	}


	if (NULL == posNewItem)
		return NULL;

	InitNewItem(pItem, posNewItem, posInsertBeside, bAfter);

	if (bSetActive)
		SetActiveTab(posNewItem);
	else
		InvalidateRect(&pItem->GetRect());

	if (bNeedResizeAll)
		ResizeAllTab();

	return posNewItem;

}

void CTabBar::RemoveTab(POSITION posDelTab)
{

	//if (CanActive(posDelTab))
	//{
		RemoveTabImpl(posDelTab, FALSE);
	//}
	//else
	//{
	//	SetActiveTab(posDelTab);
	//}
}

void CTabBar::RemoveAllTabs(void)
{
	while(m_Items.GetCount() > 0)
	{
		RemoveTabImpl(m_Items.GetHeadPosition(), TRUE);
	}
	//m_Items.RemoveAll();
}

UINT CTabBar::GetRelaTabCount(void)
{
	UINT	uCount;

	POSITION	pos;
	CTabItem	*pItem;

	uCount = 0;
	pos = m_Items.GetHeadPosition();
	while (NULL != pos)
	{
		pItem = m_Items.GetNext(pos);
		if (NULL != pItem && ::IsWindow(pItem->GetRelativeWnd()))
			uCount++;
	}

	return uCount;
}

void CTabBar::SetTabText(POSITION posTab, LPCTSTR lpszText)
{
	if (NULL == posTab)
		return;

	CTabItem	*pItem = m_Items.GetAt(posTab);
	if (NULL == pItem)
		return;

	
	pItem->SetCaption(lpszText);
	pItem->Invalidate();
}
CString CTabBar::GetTabText(POSITION posTab)
{
	if (NULL == posTab)
		return _T("");

	CTabItem	*pItem = m_Items.GetAt(posTab);
	if (NULL == pItem)
		return _T("");

	return pItem->m_strCaption;
}

void CTabBar::SetActiveTab(POSITION posTab)
{
	if (NULL == posTab)
		return;
	if (posTab == m_posCurActive)
		return;

	CTabItem	*pItem = m_Items.GetAt(posTab);
	if (NULL == pItem)
		return;

	if (!pItem->CanActive())
		return;

	if (NULL != m_posCurActive)
	{
		CTabItem	*pOldItem = m_Items.GetAt(m_posCurActive);
		if (NULL != pOldItem)
		{
			pOldItem->SetActive(FALSE);
			pOldItem->ShowRelativeWnd(FALSE);
			InvalidateRect(&pOldItem->GetRect());
		}
	}


	//resize Item's relativewnd
	m_pParentTabWnd->MoveRelativeWndOnSite(pItem->GetRelativeWnd());
	pItem->SetActive(TRUE);
	pItem->ShowRelativeWnd(TRUE);
	InvalidateRect(&pItem->GetRect());
	
	POSITION	posOldActive = m_posCurActive;
	m_posCurActive = posTab;

	m_pParentTabWnd->OnActiveTabChanged(posOldActive, m_posCurActive);
	m_pParentTabWnd->OnActiveTabChangedNotify(posOldActive, m_posCurActive);

}

CTabItem* CTabBar::GetActiveItem(void)
{
	if (NULL == m_posCurActive)
		return NULL;
	CTabItem	*pItem = m_Items.GetAt(m_posCurActive);
	return pItem;
}

HWND CTabBar::GetCurRelativeWnd()
{
	if (NULL == m_posCurActive)
		return NULL;

	CTabItem *pItem = m_Items.GetAt(m_posCurActive);
	return pItem->GetRelativeWnd();
}

void CTabBar::SwitchToNextPage(void)
{
	POSITION	posNextActive = GetNextCanActiveTab(GetActiveTab());
	if (NULL == posNextActive)
		SetActiveTab(GetFirstCanActiveTab());
	else
		SetActiveTab(posNextActive);
}

void CTabBar::SwitchToPrevPage(void)
{
	POSITION	posPrevActive = GetPrevCanActiveTab(GetActiveTab());
	if (NULL == posPrevActive)
		SetActiveTab(GetLastCanActiveTab());
	else
		SetActiveTab(posPrevActive);
}

void CTabBar::SetTabData(POSITION posTab, DWORD dwData)
{
	CTabItem *pItem = m_Items.GetAt(posTab);
	if (NULL != pItem)
		pItem->m_dwCustomData = dwData;
}

DWORD CTabBar::GetTabData(POSITION posTab) const
{
	CTabItem *pItem = m_Items.GetAt(posTab);
	if (NULL != pItem)
		return pItem->m_dwCustomData;
	else
		return 0;
}

void CTabBar::SetBarPos(ETabBarPos ePos)
{
	m_eBarPos = ePos;

	ResizeAllTab();
}

CTabItem* CTabBar::HitTest(const CPoint& point)
{
	CRect rcItem;
	GetClientRect(&rcItem);

	if ( m_Items.IsEmpty() )
	{
		return NULL;
	}

	POSITION	pos;
	CTabItem*	pItem = NULL;

	if (NULL != m_posLastHoverItem)
	{
		pos = m_posLastHoverItem;
		pItem = IsPointInItem(pos, point);
		if (NULL != pItem)
			return pItem;

		pos = m_posLastHoverItem;
		m_Items.GetNext(pos);
		pItem = IsPointInItem(pos, point);
		if (NULL != pItem)
			return pItem;

		pos = m_posLastHoverItem;
		m_Items.GetPrev(pos);
		pItem = IsPointInItem(pos, point);
		if (NULL != pItem)
			return pItem;
	}


	for ( pos = m_Items.GetHeadPosition() ; pos ; )
	{
		pItem = (CTabItem*)m_Items.GetNext( pos );

		rcItem = pItem->GetRect();
		if ( rcItem.PtInRect( point ) )
		{
			return pItem;
		}
	}

	return NULL;
}

CTabItem* CTabBar::IsPointInItem(POSITION pos, const CPoint& point)
{
	if (NULL == pos)
		return NULL;

	CRect		rtItem;
	CTabItem	*pItem = NULL;
	pItem = m_Items.GetAt(pos);
	rtItem = pItem->GetRect();
	if (rtItem.PtInRect(point))
		return pItem;
	else
		return NULL;
}

BOOL CTabBar::ChangeActiveTab()
{
	POSITION	posOldActive;
	POSITION	posNextActive;
	CTabItem	*pItem = NULL;

	posOldActive = m_posCurActive;
	
	posNextActive = GetNextCanActiveTab(m_posCurActive);
	
	if (NULL == posNextActive)
	{
		posNextActive = m_posCurActive;
		m_Items.GetPrev(posNextActive);
		while (NULL != posNextActive)
		{
			pItem = m_Items.GetAt(posNextActive);
			if (pItem->CanActive())
				break;

			m_Items.GetPrev(posNextActive);
		}
	}

	if (NULL != posNextActive)
		SetActiveTab(posNextActive);

	if (posOldActive != m_posCurActive)
		return TRUE;
	else
		return FALSE;
}

BOOL CTabBar::CanActive(POSITION pos)
{
	CTabItem	*pItem = NULL;
	pItem = m_Items.GetAt(pos);
	if (NULL == pItem)
		return FALSE;
	else
		return pItem->CanActive();
}
POSITION CTabBar::GetFirstCanActiveTab()
{
	POSITION	pos;
	pos = m_Items.GetHeadPosition();
	if (CanActive(pos))
		return pos;
	else
		return GetNextCanActiveTab(pos);
}

POSITION CTabBar::GetLastCanActiveTab()
{
	POSITION	pos;
	pos = m_Items.GetTailPosition();
	if (CanActive(pos))
		return pos;
	else
		return GetPrevCanActiveTab(pos);
}
POSITION CTabBar::GetNextCanActiveTab(POSITION pos)
{
	POSITION	posNextActive;

	posNextActive = pos;
	m_Items.GetNext(posNextActive);
	while (NULL != posNextActive)
	{
		if (CanActive(posNextActive))
			return posNextActive;

		m_Items.GetNext(posNextActive);
	}
	return NULL;
}

POSITION CTabBar::GetPrevCanActiveTab(POSITION pos)
{
	POSITION	posPrevActive;

	posPrevActive = pos;
	m_Items.GetPrev(posPrevActive);
	while (NULL != posPrevActive)
	{
		if (CanActive(posPrevActive))
			return posPrevActive;

		m_Items.GetPrev(posPrevActive);
	}
	return NULL;
}
void CTabBar::ResizeAllTab()
{
	CClientRect		rtBar(this);
	m_resizer.ResizeAll(rtBar, GetBarPos());
	Invalidate();
}

void CTabBar::OnItemCreate(CTabItem* pItem)
{
	m_pParentTabWnd->OnTabCreateNotify(pItem->m_myPos);
}

void CTabBar::OnItemDestroy(CTabItem* pItem)
{
	
	m_pParentTabWnd->OnTabDestroyNotify(pItem->m_myPos);
	m_pParentTabWnd->OnTabDestroy(pItem->m_myPos);

	if (pItem->m_myPos == m_posLastHoverItem)
		m_Items.GetNext(m_posLastHoverItem);
}

void CTabBar::LeaveLastHoverItem()
{
	CTabItem	*pItem = NULL;
	if (NULL != m_posLastHoverItem)
	{
		pItem = m_Items.GetAt(m_posLastHoverItem);
		if (NULL != pItem)
			pItem->OnMouseLeave();
		m_posLastHoverItem = NULL;
	}
}

void CTabBar::EnterNewHoverItem(CTabItem* pItem)
{
	if (NULL != pItem)
	{
		m_posLastHoverItem = pItem->m_myPos;
		pItem->OnMouseHover();
	}
	else
	{
		m_posLastHoverItem = NULL;
	}
}

COLORREF CTabBar::GetBkColor()
{
	return m_pParentTabWnd->GetBkColor();
}

void CTabBar::InvalidateItem(POSITION pos)
{
	CTabItem	*pTabItem = NULL;
	if (NULL != pos)
	{
		pTabItem = m_Items.GetAt(pos);
		if (NULL != pTabItem)
			InvalidateRect(&pTabItem->GetRect());
	}
}

void CTabBar::RemoveTabImpl(POSITION posDelTab, BOOL bIsRemoveAll)
{
	CTabItem* pItem = m_Items.GetAt(posDelTab);

	if (posDelTab == m_posCurActive)
	{
		if (!bIsRemoveAll)
		{
			ChangeActiveTab();

			if (posDelTab == m_posCurActive)
			{
				pItem->ShowRelativeWnd(FALSE);
				m_posCurActive = NULL;
			}
		}
		else
		{
			pItem->ShowRelativeWnd(FALSE);
			m_posCurActive = NULL;
		}
	}

	if (posDelTab == m_posLastHoverItem)
		m_posLastHoverItem = NULL;


	pItem->Destroy();
	SAFE_DELETE(pItem);
	m_Items.RemoveAt(posDelTab);

	if (!bIsRemoveAll)
		ResizeAllTab();
}
// CTabBar 消息处理程序

void CTabBar::OnPaint()
{
	CClientRect	rtClient(this);

	CPaintDC dc(this);
	CBufferDC	bufDC(dc.GetSafeHdc(), rtClient);

	SendMessage(WM_ERASEBKGND, (WPARAM) bufDC.GetSafeHdc(), 1);



	//CPen	pen(PS_SOLID, 1, RGB(121, 138, 169));

	//bufDC.SelectObject(&pen);
	//bufDC.MoveTo(0,rtClient.Height() - 1);
	//bufDC.LineTo(rtClient.right, rtClient.bottom - 1);
	CRect	rtClip;
	dc.GetClipBox(&rtClip);
	CRect	rtIntersect;

	for ( POSITION pos = m_Items.GetHeadPosition() ; pos ; )
	{
		CTabItem* pTabItem = (CTabItem*)m_Items.GetNext( pos );

		rtIntersect.IntersectRect(rtClip, pTabItem->GetRect());
		if (!rtIntersect.IsRectEmpty())
			pTabItem->Paint(&bufDC);
	}
}

//BOOL CTabBar::OnEraseBkgnd(CDC* /*pDC*/)
//{
//	
//	return TRUE;
//
//	//return CWnd::OnEraseBkgnd(pDC);
//}

LRESULT	CTabBar::OnEraseBkgndEx(WPARAM wParam, LPARAM lParam)
{
	if (1 == lParam)
	{
		CClientRect	rtClient(this);
		CDC			dcDraw;
		
		dcDraw.Attach((HDC) wParam);
		if (!DrawBk(&dcDraw, rtClient))
		{
			CBrush		brush(GetBkColor());
			dcDraw.FillRect(&rtClient, &brush);
		}
		dcDraw.Detach();
	}

	return 1;
}
void CTabBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	CTabItem* pItem = HitTest( point );

	if(pItem)
	{
		pItem->OnLButtonDown(nFlags, point);
	}
	CWnd::OnLButtonDown(nFlags, point);
	SetFocus();
}

void CTabBar::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	//// TODO: 在此添加消息处理程序代码和/或调用默认值
	CTabItem* pItem = HitTest( point );

	if(pItem)
	{
		pItem->OnLButtonDblClk(nFlags, point);
	}

	CWnd::OnLButtonDblClk(nFlags, point);
}


void CTabBar::OnSize(UINT nType, int cx, int cy)
{
	__try
	{
		CWnd::OnSize(nType, cx, cy);
		ResizeAllTab();
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{		
	}	
}

void CTabBar::SetBarBreadth(int iBreadth)
{
	m_iBreadth = iBreadth;
}
void CTabBar::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CTabItem *pTabItem = HitTest(point);

	if (NULL != pTabItem)
	{
		if (pTabItem->m_myPos == m_posLastHoverItem)
		{
			pTabItem->OnMouseMove(nFlags, point);
		}
		else
		{
			LeaveLastHoverItem();
			EnterNewHoverItem(pTabItem);
			pTabItem->OnMouseMove(nFlags, point);
		}
	}
	else
	{
		LeaveLastHoverItem();
	}
	
	m_mouseMgr.OnMouseMove(GetSafeHwnd());
	CWnd::OnMouseMove(nFlags, point);
}

int CTabBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码
	m_mouseMgr.Init(GetSafeHwnd(), 0);

	EnableToolTips();
	return 0;
}

LRESULT CTabBar::OnMouseLeave(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	LeaveLastHoverItem();
	m_mouseMgr.OnMouseOut(GetSafeHwnd());
	
	return 0;
}

BOOL CTabBar::OnToolTipNotify(UINT id, NMHDR *pNMH,
								  LRESULT *pResult)
{
	TOOLTIPTEXT *pText = (TOOLTIPTEXT *)pNMH;
	pText->hinst = AfxGetInstanceHandle();
	return TRUE;
}

INT_PTR CTabBar::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	POSITION pos = m_Items.GetHeadPosition();
	int i=0;
	while(pos)
	{
		CTabItem * pItem=m_Items.GetNext(pos);
		i++;
		if(pItem->GetRect().PtInRect(point))
		{
			if(pItem->m_strCaption.IsEmpty())
				return -1;

			pTI->cbSize = sizeof(TOOLINFO);
			pTI->uFlags = TTF_IDISHWND;
			pTI->hwnd = m_hWnd;
			pTI->uId = (UINT_PTR)m_hWnd;
			pTI->rect = pItem->GetRect();
			pTI->lpszText = (LPTSTR)malloc(sizeof(TCHAR)*(pItem->m_strCaption.GetLength()+1));// (LPTSTR)(LPCTSTR)pItem->m_strCaption;
			_tcscpy(pTI->lpszText, pItem->m_strCaption);
			return i;
		}
	}

	return -1;
}

CTabItem*	CTabBar::GetTabBarItem(POSITION pos)
{
	if(pos == NULL)
	{
		return NULL;
	}

	CTabItem * pItem = m_Items.GetAt(pos);
	return pItem;
}

void CTabBar::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	UINT	uPrevChar;
	UINT	uNextChar;
	if (IsHorizon())
	{
		uPrevChar = VK_LEFT;
		uNextChar = VK_RIGHT;
	}
	else
	{
		uPrevChar = VK_UP;
		uNextChar = VK_DOWN;
	}

	if (uPrevChar == nChar)
		SwitchToPrevPage();
	else if (uNextChar == nChar)
		SwitchToNextPage();
	
	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}


UINT CTabBar::OnGetDlgCode()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	UINT uRet = CWnd::OnGetDlgCode();
	uRet |= /*DLGC_WANTTAB |*/ DLGC_WANTARROWS;
	return uRet;
}
