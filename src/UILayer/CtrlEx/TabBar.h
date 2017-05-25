/*
 * $Id: TabBar.h 4483 2008-01-02 09:19:06Z soarchin $
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
#pragma once


// CTabBar
#include <AfxTempl.h>
#include "TabWndDef.h"
#include "Util.h"
#include "TabItem.h"
#include "Resizer.h"
#include "BkDraw.h"

class CTabWnd;

class CTabBar : public CWnd, public CBkDraw
{
	DECLARE_DYNAMIC(CTabBar)

public:
	CTabBar();
	virtual ~CTabBar();

	void		SetParentTabWnd(CTabWnd *pParentTabWnd){m_pParentTabWnd = pParentTabWnd;}
	BOOL		Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
	void		InitNewItem(CTabItem *pItem, POSITION posItem, POSITION posInsertBeside, BOOL bAfter);
	CTabItem*	GetTabBarItem(POSITION pos);

	POSITION	AddTab(CTabItem *pItem, BOOL bSetActive = FALSE,
						POSITION posInsertBeside = NULL, BOOL bAfter = TRUE);	// bAfter:TRUE-InsertAfter, FALSE-InsertBefore

	void		RemoveTab(POSITION posDelTab);
	void		RemoveAllTabs(void);

	POSITION	GetFirstTab(){return m_Items.GetHeadPosition();}
	CTabItem*	GetNextTab(POSITION &pos){return m_Items.GetNext(pos);}

	UINT		GetTabCount(void){return m_Items.GetCount();}
	UINT		GetRelaTabCount(void);

	void		SetTabText(POSITION posTab, LPCTSTR lpszText);
	CString		GetTabText(POSITION posTab);
	void		SetActiveTab(POSITION posTab);
	POSITION	GetActiveTab(){return m_posCurActive;}
	CTabItem*	GetActiveItem(void);
	HWND		GetCurRelativeWnd();
	void		SwitchToNextPage(void);
	void		SwitchToPrevPage(void);

	void		SetTabData(POSITION posTab, DWORD dwData);
	DWORD		GetTabData(POSITION posTab) const;

	void		SetBarPos(ETabBarPos ePos);
	ETabBarPos	GetBarPos(void){return m_eBarPos;}

	int			GetBarBreadth(){return m_iBreadth;}
	void		SetBarBreadth(int iBreadth);
	void		GetBarMarginLogic(CRect &rect);
	void		SetBarMarginLogic(const CRect &rect);

	void		InvalidateItem(POSITION pos);

	CTabWnd*	GetParentTabWnd(){return m_pParentTabWnd;}

	BOOL		IsHorizon(){return TBP_TOP == m_eBarPos || TBP_BOTTOM == m_eBarPos;}

protected:
	void		RemoveTabImpl(POSITION posDelTab, BOOL bIsRemoveAll);
	CTabItem*	HitTest(const CPoint& point);
	CTabItem*	IsPointInItem(POSITION pos, const CPoint& point);
	BOOL		ChangeActiveTab();
	BOOL		CanActive(POSITION pos);
	POSITION	GetFirstCanActiveTab();
	POSITION	GetLastCanActiveTab();

	POSITION	GetNextCanActiveTab(POSITION pos);
	POSITION	GetPrevCanActiveTab(POSITION pos);

	void		ResizeAllTab();
	
	void		LeaveLastHoverItem();
	void		EnterNewHoverItem(CTabItem* pItem);

	COLORREF	GetBkColor();

	virtual void OnItemCreate(CTabItem* pItem);
	virtual void OnItemDestroy(CTabItem* pItem);
	INT_PTR OnToolHitTest(CPoint point, TOOLINFO* pTI) const;

protected:
	CTabWnd							*m_pParentTabWnd;
	CList<CTabItem*, CTabItem*>		m_Items;
	
	POSITION	m_posCurActive;
	POSITION	m_posLastHoverItem;
	
	ETabBarPos	m_eBarPos;
	int			m_iBreadth;


	CMouseMgr	m_mouseMgr;

	TabWnd::CResizer	m_resizer;

protected:
	friend class CTabItem;
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	//afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	LRESULT	OnEraseBkgndEx(WPARAM wParam, LPARAM lParam);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnToolTipNotify(UINT id, NMHDR *pNMH, LRESULT *pResult);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg UINT OnGetDlgCode();
};

