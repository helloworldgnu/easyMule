/*
 * $Id: TabItem_Normal.h 6147 2008-07-10 03:16:33Z dgkang $
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


// CTabItem_Normal
#include "TabItem.h"
#include "CxImage\xImage.h"

class CTabItem_Normal : public CTabItem
{
	DECLARE_DYNCREATE(CTabItem_Normal)

public:
	enum {MARGIN_H = 10};
	CTabItem_Normal();
	virtual ~CTabItem_Normal();

	virtual void SetCaption(LPCTSTR lpszText);
	//VC-dgkang 2008Äê7ÔÂ8ÈÕ
	CString GetCaption () const {return m_strCaption;}

	void	SetCustomData(DWORD dwData){m_dwCustomData = dwData;}
	//void	SetIcon(HICON hIcon){m_hIcon = hIcon;}
	void	SetIcon(LPCTSTR lpszPngResource);
	void	SetRelativeWnd(HWND hRelativeWnd, HWND hRelaWndOldParent = NULL,
							BOOL bAutoDelRelaWndObject = FALSE, CWnd* pRelaWndObjectToDel = NULL);
	virtual	int	GetDesireLength(void);


protected:
	virtual void	OnCreate(void);

	void	SetRelaWndParent();
	virtual BOOL	GetIconRect(CRect &rect);
	virtual BOOL	GetTextRect(CRect &rect);

	CxImage		m_imgIcon;
	bool		m_bHasIcon;

	//DECLARE_MESSAGE_MAP()
protected:
	void	DrawActiveBk(CDC* pDC, const CRect &rect);
	void	DrawHover(CDC* pDC, const CRect &rect);
	void	DrawInactiveBk(CDC* pDC, const CRect &rect);
	void	DrawItem(CDC* pDC, const CRect &rect, COLORREF clrFrmOutside, COLORREF clrFrmInside, COLORREF aclrLayers[4]);
	
public:
	virtual void Paint(CDC* pDC);
	virtual void OnLButtonDown(UINT nFlags, CPoint point);
	virtual void OnMouseHover(void);
	virtual void OnMouseLeave(void);
};
