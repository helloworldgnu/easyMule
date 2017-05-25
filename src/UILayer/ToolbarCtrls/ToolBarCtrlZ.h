/*
 * $Id: ToolBarCtrlZ.h 4483 2008-01-02 09:19:06Z soarchin $
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

#include "BkDraw.h"
// CToolBarCtrlZ

class CToolBarCtrlZ : public CToolBarCtrl, public CBkDraw
{
	DECLARE_DYNAMIC(CToolBarCtrlZ)

public:
	CToolBarCtrlZ();
	virtual ~CToolBarCtrlZ();

	int	AddSingleString(LPCTSTR lpszText);
	int GetMaxLength();
protected:
	void	DrawItem(CDC *pDC, int iIndex, const CRect &rtItem, BOOL bHover = FALSE);
	void	CleanupAllImages(void);
	void	AddDisableImageIcon(LPCTSTR lpszResource);
	void	AddImageIcon(LPCTSTR lpszResource);
protected:
	CArray<CxImage*, CxImage*>		m_arrImgs;
	CArray<CxImage*, CxImage*>		m_arrDisableImgs;
	CImageList						m_ilFake;

	CMouseMgr	m_mouseMgr;		// 为兼容windows2000(2000下没有HotItem)
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//afx_msg void OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnPaint();
	LRESULT OnGetDesireLength(WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};


