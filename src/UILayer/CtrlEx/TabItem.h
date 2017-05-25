/*
 * $Id: TabItem.h 4483 2008-01-02 09:19:06Z soarchin $
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

// CTabItem
#include "Resizee.h"
using namespace TabWnd;

class CTabBar;
class CTabItemAffector;
enum TextType{
	NoTexts = 0,
	TextsBelow = 1,
	TextsRight = 2
};

class CTabItem : public CObject, public CResizee
{
	DECLARE_DYNCREATE(CTabItem)
	friend class CTabBar;
public:
	CTabItem();
	virtual ~CTabItem();

	virtual void	Create(CTabBar *pParentBar, POSITION myPos);
	virtual void	OnCreate(void){}
	virtual void	Destroy();

	void	SetAffector(CTabItemAffector *pAffector);
	void	SetRelativeWnd(HWND hWnd, HWND hOldParent = NULL);
	HWND	GetRelativeWnd(void){return m_hRelativeWnd;}
	void	ShowRelativeWnd(BOOL bShow);

	virtual void SetCaption(LPCTSTR lpszCaption){m_strCaption = lpszCaption;}
	CRect	GetRectInScreen(void);
//protected:

	virtual BOOL CanActive(){return TRUE;}
	void	SetActive(BOOL bActive);
	BOOL	GetActive(void){return m_bActive;}
//protected:
	int			GetBarPos(void);
	COLORREF	GetBkColor();
	BOOL		IsHover(){return m_bHover;}
	void		Invalidate();
	CTabBar*	GetParentBar(){return m_pParentBar;}

	void		RequestResize();
//protected:
	BOOL		m_bActive;
	CTabBar		*m_pParentBar;
	POSITION	m_myPos;

	//CWnd		*m_pRelativeWnd;
	HWND		m_hRelativeWnd;
	BOOL		m_bAutoDelRelaWndObject;
	CWnd*		m_pDelRelaWnd;
	HWND		m_hRelaWndOldParent;
	CString		m_strCaption;
	//HICON		m_hIcon;
	DWORD		m_dwCustomData;

	int			m_iItemGap;

	BOOL		m_bHover;

	CTabItemAffector *m_pAffector;
protected:
	TextType	m_TextType;

public:
	virtual	void OnSize(){}
	virtual void Paint(CDC* /*pDC*/){}
	virtual void OnLButtonDown(UINT /*nFlags*/, CPoint /*point*/){}
	virtual void OnLButtonDblClk(UINT /*nFlags*/, CPoint /*point*/){}
	virtual void OnMouseMove(UINT /*nFlags*/, CPoint /*point*/){}
	virtual void OnMouseHover(void){m_bHover = TRUE;}
	virtual void OnMouseLeave(void){m_bHover = FALSE;}

};


