/*
 * $Id: TabWnd.h 4483 2008-01-02 09:19:06Z soarchin $
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


// CTabWnd
#include "TabWndDef.h"
#include "TabBar.h"
#include "Joint.h"
#include "TabItem.h"

typedef struct
{
	NMHDR		hdr;
	POSITION	posTab;
	POSITION	posOld;
} NMTW_TABOP;

using namespace TabWnd;

class CTabWnd : public CWnd
{
	DECLARE_DYNAMIC(CTabWnd)

public:
	CTabWnd();
	virtual ~CTabWnd();

	BOOL		Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID = 0);
	POSITION	AddTab(CTabItem *pItem, BOOL bSetActive = FALSE, POSITION posInsertBeside = NULL, BOOL bAfter = TRUE);	// bAfter:TRUE-InsertAfter, FALSE-InsertBefore


	void		RemoveTab(POSITION posDelTab);
	void		RemoveAllTabs(void);

	POSITION	GetFirstTab(){return m_bar.GetFirstTab();}
	CTabItem*	GetNextTab(POSITION &pos){return m_bar.GetNextTab(pos);}

	CTabItem*	GetTabItem(POSITION posTab);
	UINT		GetTabCount(void){return m_bar.GetTabCount();}
	UINT		GetRelaTabCount(void){return m_bar.GetRelaTabCount();}

	void		SetTabText(POSITION posTab, LPCTSTR lpszText);
	CString		GetTabText(POSITION posTab);

	POSITION	GetActiveTab();
	void		SetActiveTab(POSITION posTab);
	void		SwitchToNextPage(void);
	void		SwitchToPrevPage(void);

	void		SetTabData(POSITION posTab, DWORD dwData);
	DWORD		GetTabData(POSITION posTab) const;

	void		SetBarPos(ETabBarPos ePos);
	ETabBarPos	GetBarPos(void){return (ETabBarPos) m_bar.GetBarPos();}

	int			GetBarBreadth(){return m_bar.GetBarBreadth();}
	void		SetBarBreadth(int iBreadth);

	void		GetBarMarginLogic(CRect &rect){m_bar.GetBarMarginLogic(rect);}
	void		SetBarMarginLogic(const CRect &rect){m_bar.SetBarMarginLogic(rect);}

	void		GetBarRect(CRect &rect);
	void		GetJointRect(CRect &rect);
	void		GetBoardRect(CRect &rect);
	void		GetRelativeWndRect(CRect &rect);

	void		MoveBarRectOnSite(void);
	void		MoveJointOnSite(void);
	void		MoveRelativeWndOnSite(HWND hWnd);
	COLORREF	GetBkColor(){return m_clrBk;}
	void		SetBkColor(COLORREF clrBk, BOOL bRepaint = TRUE);
	void		SetJointColor(COLORREF clrJoint, COLORREF clrGap){if (NULL != m_pJoint) m_pJoint->SetJointColor(clrJoint, clrGap);}
	void		SetBoardColor(COLORREF clrFrame){m_clrBoardFrame = clrFrame;}

	void		SetJoint(CJoint *pJoint);

	void		SetBarBkDraw(CRectDraw *pDraw){m_bar.SetBkDraw(pDraw);}

	virtual void		OnActiveTabChanged(POSITION posOldActiveTab, POSITION posNewActiveTab);
	void				OnActiveTabChangedNotify(POSITION posOldActiveTab, POSITION posNewActiveTab);
	void				OnTabDestroyNotify(POSITION posTab);
	virtual void		OnTabDestroy(POSITION posTab);
	void				OnTabCreateNotify(POSITION posTab);

protected:

	CTabBar		m_bar;
	CJoint		*m_pJoint;
	COLORREF	m_clrBk;
	COLORREF	m_clrBoardFrame;

	CRect		m_rtBoardMarginLogic;
	
	void		Resize(void);
	void		DrawJoint(CDC *pDC);
	void		DrawBoard(CDC *pDC);
	
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};


