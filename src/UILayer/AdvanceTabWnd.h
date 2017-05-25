/*
 * $Id: AdvanceTabWnd.h 8435 2008-11-24 08:52:24Z huby $
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

#include "TabWnd.h"
#include "Localizee.h"
#include "TbcAdvance.h"
// CAdvanceTabWnd

class CAdvanceTabWnd : public CTabWnd , public CLocalizee
{
	DECLARE_DYNAMIC(CAdvanceTabWnd)
	LOCALIZEE_WND_CANLOCALIZE()
public:
	CAdvanceTabWnd();
	virtual ~CAdvanceTabWnd();

	void Localize();

	enum ETabId
	{
		TI_SERVER,
		TI_KAD,
		TI_STAT,

		TI_MAX
	};

protected:
	POSITION	m_aposTabs[TI_MAX];

	CTbcAdvance	m_toolbar;
	
	void InitToolBar(void);

protected:
	DECLARE_MESSAGE_MAP()
public:
	BOOL		IsTabActive(ETabId eTabId){return m_aposTabs[eTabId]==GetActiveTab();} 
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};


