/*
 * $Id: TabItem_Wnd.h 4483 2008-01-02 09:19:06Z soarchin $
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

#include "TabItem.h"

class CTabItem_Wnd : public CTabItem
{
	DECLARE_DYNCREATE(CTabItem_Wnd)
public:
	CTabItem_Wnd(void);
	~CTabItem_Wnd(void);

	void	SetItemWnd(CWnd *pWnd, BOOL bAutoDelete);
	void	SetWindowLength(int iLength){m_iWndLength = iLength;}
	virtual	int	GetDesireLength(void);

	virtual void	Create(CTabBar *pParentBar, POSITION myPos);
	//virtual void	Destroy();

	virtual void Paint(CDC* pDC);
	virtual	void OnSize();

protected:
	virtual BOOL CanActive(){return FALSE;}
protected:
	CWnd	*m_pWnd;
	BOOL	m_bAutoDelete;
	int		m_iWndLength;
};
