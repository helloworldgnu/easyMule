/*
 * $Id: ClosableTabCtrl.h 4483 2008-01-02 09:19:06Z soarchin $
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

class CClosableTabCtrl : public CTabCtrl
{
	DECLARE_DYNAMIC(CClosableTabCtrl)

public:
	CClosableTabCtrl();
	virtual ~CClosableTabCtrl();

	bool m_bCloseable;

protected:
	CImageList m_ImgLstCloseButton;
	IMAGEINFO m_iiCloseButton;
	CPoint m_ptCtxMenu;

	void InternalInit();
	void SetAllIcons();
	void GetCloseButtonRect(const CRect& rcItem, CRect& rcCloseButton);
	int GetTabUnderContextMenu() const;
	int GetTabUnderPoint(CPoint point) const;

	virtual void PreSubclassWindow();
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnSysColorChange();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg LRESULT _OnThemeChanged();
};
