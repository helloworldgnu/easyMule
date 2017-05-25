/*
 * $Id: TabWnd.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
// TabWnd.cpp : 实现文件
//

#include "stdafx.h"
#include "TabWnd.h"
#include ".\tabwnd.h"
#include "Util.h"
#include "UserMsgs.h"
// CTabWnd

IMPLEMENT_DYNAMIC(CTabWnd, CWnd)
CTabWnd::CTabWnd()
{
	m_bar.SetParentTabWnd(this);

	m_clrBk = ::GetSysColor(COLOR_3DFACE);
	m_clrBoardFrame = RGB(121, 138, 169);

	m_rtBoardMarginLogic.SetRect(0, 0, 0, 0);

	m_pJoint = NULL;
}

CTabWnd::~CTabWnd()
{
	SAFE_DELETE(m_pJoint);
}


BEGIN_MESSAGE_MAP(CTabWnd, CWnd)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_CREATE()
END_MESSAGE_MAP()

BOOL CTabWnd::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	RegisterSimpleWndClass();
	return CWnd::CreateEx(WS_EX_CONTROLPARENT, _T("SimpleWnd"), NULL, dwStyle /*| WS_TABSTOP*/ , rect, pParentWnd, nID);
}

POSITION CTabWnd::AddTab(CTabItem *pItem, BOOL bSetActive, POSITION posInsertBeside, BOOL bAfter)
{
	return m_bar.AddTab(pItem, bSetActive, posInsertBeside, bAfter);
}

void CTabWnd::RemoveTab(POSITION posDelTab)
{
	return m_bar.RemoveTab(posDelTab);
}

void CTabWnd::RemoveAllTabs(void)
{
	m_bar.RemoveAllTabs();
}

void CTabWnd::SetTabText(POSITION posTab, LPCTSTR lpszText)
{
	m_bar.SetTabText(posTab, lpszText);
}

CString CTabWnd::GetTabText(POSITION posTab)
{
	return m_bar.GetTabText(posTab);
}

POSITION CTabWnd::GetActiveTab()
{
	return m_bar.GetActiveTab();
}

void CTabWnd::SetActiveTab(POSITION posTab)
{
	m_bar.SetActiveTab(posTab);
}

void CTabWnd::SwitchToNextPage(void)
{
	m_bar.SwitchToNextPage();
}
void CTabWnd::SwitchToPrevPage(void)
{
	m_bar.SwitchToPrevPage();
}
void CTabWnd::SetTabData(POSITION posTab, DWORD dwData)
{
	m_bar.SetTabData(posTab, dwData);
}

DWORD CTabWnd::GetTabData(POSITION posTab) const
{
	return m_bar.GetTabData(posTab);
}

void CTabWnd::SetBarPos(ETabBarPos ePos)
{
	m_bar.SetBarPos(ePos);
	Resize();
}

void CTabWnd::GetBarRect(CRect &rect)
{
	CClientRect	rtClient(this);
	TabWnd::Real2LogicSolid(rtClient, m_bar.GetBarPos());
	rect.SetRect(rtClient.left, rtClient.top, rtClient.right, rtClient.top + m_bar.GetBarBreadth() - 1);
	TabWnd::LogicSolid2Real(rect, m_bar.GetBarPos());
}

void CTabWnd::GetJointRect(CRect &rect)
{
	CClientRect		rtClient(this);
	CWindowRect		rtBar(&m_bar);
	ScreenToClient(&rtBar);

	TabWnd::Real2LogicSolid(rtClient, m_bar.GetBarPos());
	TabWnd::Real2LogicSolid(rtBar, m_bar.GetBarPos());

	rect = rtClient;
	rect.top = rtBar.bottom + 1;
	
	if (NULL == m_pJoint)
		rect.bottom = rect.top - 1;

	else
		rect.bottom = rect.top;

	TabWnd::LogicSolid2Real(rect, m_bar.GetBarPos());

}

void CTabWnd::MoveBarRectOnSite(void)
{
	CRect		rtBar;
	GetBarRect(rtBar);
	if (::IsWindow(m_bar.m_hWnd))
		m_bar.MoveWindow(rtBar);
}

void CTabWnd::MoveJointOnSite(void)
{
	if (NULL == m_pJoint)
		return;

	CRect		rtJoint;
	GetJointRect(rtJoint);
	m_pJoint->Move(rtJoint);
}

void CTabWnd::GetBoardRect(CRect &rect)
{
	//CClientRect		rtClient(this);
	//CWindowRect		rtBar(&m_bar);
	//ScreenToClient(&rtBar);

	//TabWnd::Real2LogicSolid(rtClient, m_bar.GetBarPos());
	//TabWnd::Real2LogicSolid(rtBar, m_bar.GetBarPos());

	//rect = rtClient;
	//rect.top = rtBar.bottom + 1;

	//TabWnd::LogicSolid2Real(rect, m_bar.GetBarPos());

	CClientRect		rtClient(this);
	CRect			rtJoint;
	GetJointRect(rtJoint);

	TabWnd::Real2LogicSolid(rtClient, m_bar.GetBarPos());
	TabWnd::Real2LogicSolid(rtJoint, m_bar.GetBarPos());

	rect = rtClient;
	rect.top = rtJoint.bottom + 1;

	TabWnd::LogicSolid2Real(rect, m_bar.GetBarPos());

}

void CTabWnd::GetRelativeWndRect(CRect &rect)
{
	GetBoardRect(rect);
	TabWnd::Real2Logic(rect, GetBarPos());
	rect.DeflateRect(m_rtBoardMarginLogic.left, m_rtBoardMarginLogic.top, m_rtBoardMarginLogic.right, m_rtBoardMarginLogic.bottom);
	TabWnd::Logic2Real(rect, GetBarPos());
}

void CTabWnd::MoveRelativeWndOnSite(HWND hWnd)
{
	CRect	rtRela;
	if (IsWindow(hWnd))
	{
		GetRelativeWndRect(rtRela);
		::MoveWindow(hWnd, rtRela.left, rtRela.top, rtRela.Width(), rtRela.Height(), TRUE);
	}
}

void CTabWnd::SetBkColor(COLORREF clrBk, BOOL bRepaint)
{
	m_clrBk = clrBk;
	if (bRepaint)
		Invalidate();
}

void CTabWnd::SetJoint(CJoint *pJoint)
{
	SAFE_DELETE(m_pJoint);
	m_pJoint = pJoint;

	m_pJoint->SetJointColor(m_clrBoardFrame, m_clrBk);
}

void CTabWnd::OnActiveTabChanged(POSITION /*posOldActiveTab*/, POSITION /*posNewActiveTab*/)
{
	//CRect	rtBoard;
	//GetBoardRect(rtBoard);
	//InvalidateRect(&rtBoard);

	CRect	rtJoint;
	GetJointRect(rtJoint);
	InvalidateRect(&rtJoint);
}

void CTabWnd::OnActiveTabChangedNotify(POSITION posOldActiveTab, POSITION posNewActiveTab)
{
	if (!IsWindow(m_hWnd))
		return;
	
	NMTW_TABOP	nm;
	CWnd *pOwner = GetOwner();
	if (IsWindow(pOwner))
	{
		nm.hdr.code = NMC_TW_ACTIVE_TAB_CHANDED;
		nm.hdr.hwndFrom = m_hWnd;
		nm.hdr.idFrom = GetDlgCtrlID();
		nm.posTab = posNewActiveTab;
		nm.posOld = posOldActiveTab;
		pOwner->SendMessage(WM_NOTIFY, nm.hdr.idFrom, (LPARAM) &nm);
	}
}

void CTabWnd::OnTabDestroyNotify(POSITION posTab)
{
	if (!IsWindow(m_hWnd))
		return;


	NMTW_TABOP	nm;
	CWnd *pOwner = GetOwner();
	if (IsWindow(pOwner))
	{
		nm.hdr.code = NMC_TW_TAB_DESTROY;
		nm.hdr.hwndFrom = m_hWnd;
		nm.hdr.idFrom = GetDlgCtrlID();
		nm.posTab = posTab;
		nm.posOld = NULL;
		pOwner->SendMessage(WM_NOTIFY, nm.hdr.idFrom, (LPARAM) &nm);
	}
}

void CTabWnd::OnTabCreateNotify(POSITION posTab)
{
	if (!IsWindow(m_hWnd))
		return;


	NMTW_TABOP	nm;
	CWnd *pOwner = GetOwner();
	if (IsWindow(pOwner))
	{
		nm.hdr.code = NMC_TW_TAB_CREATE;
		nm.hdr.hwndFrom = m_hWnd;
		nm.hdr.idFrom = GetDlgCtrlID();
		nm.posTab = posTab;
		nm.posOld = NULL;
		pOwner->SendMessage(WM_NOTIFY, nm.hdr.idFrom, (LPARAM) &nm);
	}
}

void CTabWnd::OnTabDestroy(POSITION /*posTab*/)
{
}

void CTabWnd::Resize(void)
{
	MoveBarRectOnSite();
	MoveJointOnSite();
	MoveRelativeWndOnSite(m_bar.GetCurRelativeWnd());
}

void CTabWnd::DrawJoint(CDC *pDC)
{
	if (NULL == m_pJoint)
		return;

	CRect	rtActiveItem;
	CTabItem *pItem;
	pItem = m_bar.GetActiveItem();
	if (NULL != pItem)
	{
		rtActiveItem = pItem->GetRect();
		m_bar.ClientToScreen(&rtActiveItem);
		ScreenToClient(rtActiveItem);

		CRect	rtJointLogic;
		GetJointRect(rtJointLogic);
		TabWnd::Real2LogicSolid(rtJointLogic, m_bar.GetBarPos());

		CRect	rtActiveItemLogic = rtActiveItem;
		TabWnd::Real2LogicSolid(rtActiveItemLogic, m_bar.GetBarPos());
		
		CRect	rtLine;
		rtLine.left		= rtActiveItemLogic.left + 1;
		rtLine.top		= rtJointLogic.top;
		rtLine.right	= rtActiveItemLogic.right - pItem->m_iItemGap - 1;
		rtLine.bottom	= rtJointLogic.top;
		TabWnd::LogicSolid2Real(rtLine, m_bar.GetBarPos());
		
		m_pJoint->SetGap(rtLine.TopLeft(), rtLine.BottomRight());
		m_pJoint->Draw(pDC);
	}
}

void CTabWnd::DrawBoard(CDC *pDC)
{
	CRect	rtBoard;
	CBrush	brush(m_clrBoardFrame);
	GetBoardRect(rtBoard);
	pDC->FillSolidRect(&rtBoard, m_clrBk);
	pDC->FrameRect(&rtBoard, &brush);

	//CRect	rtActiveItem;
	//CTabItem *pItem;
	//pItem = m_bar.GetActiveItem();
	//if (NULL != pItem)
	//{
	//	rtActiveItem = pItem->GetRect();
	//	m_bar.ClientToScreen(&rtActiveItem);
	//	ScreenToClient(rtActiveItem);

	//	CRect	rtBoardLogic = rtBoard;
	//	TabWnd::Real2LogicSolid(rtBoardLogic, m_bar.GetBarPos());

	//	CRect	rtActiveItemLogic = rtActiveItem;
	//	TabWnd::Real2LogicSolid(rtActiveItemLogic, m_bar.GetBarPos());
	//	
	//	CRect	rtLine;
	//	rtLine.left		= rtActiveItemLogic.left + 1;
	//	rtLine.top		= rtBoardLogic.top;
	//	rtLine.right	= rtActiveItemLogic.right - pItem->m_iItemGap - 1;
	//	rtLine.bottom	= rtBoardLogic.top;
	//	TabWnd::LogicSolid2Real(rtLine, m_bar.GetBarPos());
	//	
	//	CBrush	brush(m_clrBk);
	//	pDC->FrameRect(&rtLine, &brush);
	//}
}

// CTabWnd 消息处理程序


void CTabWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 在此处添加消息处理程序代码
	// 不为绘图消息调用 CWnd::OnPaint()

	CClientRect	rtClient(this);
	CBufferDC	bufDC(dc.GetSafeHdc(), rtClient);
	DrawJoint(&bufDC);
	DrawBoard(&bufDC);
}

void CTabWnd::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	Resize();
}

BOOL CTabWnd::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CTabWnd::SetBarBreadth(int iBreadth)
{
	m_bar.SetBarBreadth(iBreadth);
	SendMessage(WM_SIZE,0,0);
}

int CTabWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码
	if (!m_bar.Create(WS_VISIBLE | WS_CHILD, CRect(0, 0, 0, 0), this, 1001))
		return -1;

	Resize();
	ModifyStyle(0, WS_CLIPCHILDREN);

	return 0;
}

CTabItem* CTabWnd::GetTabItem(POSITION posTab)
{
	if (posTab == NULL)
	{
		return NULL;
	}
	CTabItem* pItem = m_bar.GetTabBarItem(posTab);
	return pItem;
}

