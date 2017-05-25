/*
 * $Id: TabItem_NormalCloseable.h 4483 2008-01-02 09:19:06Z soarchin $
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


#include "TabItem_Normal.h"
// CTabItem_NormalCloseable ÃüÁîÄ¿±ê

class CTabItem_NormalCloseable : public CTabItem_Normal
{
	DECLARE_DYNCREATE(CTabItem_NormalCloseable)

public:
	CTabItem_NormalCloseable();
	virtual ~CTabItem_NormalCloseable();

	void	EnableClose(BOOL bEnable){m_bEnableClose = bEnable;}
	virtual void OnSize(void);
	virtual void Paint(CDC* pDC);
	virtual void OnLButtonDown(UINT nFlags, CPoint point);
	virtual void OnLButtonDblClk(UINT nFlags, CPoint point);
	virtual void OnMouseMove(UINT nFlags, CPoint point);

	enum {CLOSEBN_SIZE = 8};
	virtual BOOL	GetTextRect(CRect &rect);
	virtual int		GetDesireLength();

protected:
	BOOL	m_bEnableClose;
	CRect	m_rtCloseBn;
	BOOL	m_isCloseBtnOver;
};


