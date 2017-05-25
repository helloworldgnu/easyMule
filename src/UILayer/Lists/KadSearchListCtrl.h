/* 
 * $Id: KadSearchListCtrl.h 4483 2008-01-02 09:19:06Z soarchin $
 * 
 * this file is part of eMule
 * Copyright (C)2002-2006 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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
#include "MuleListCtrl.h"
#include "kademlia/kademlia/search.h"

class CIni;

class CKadSearchListCtrl : public CMuleListCtrl
{
	DECLARE_DYNAMIC(CKadSearchListCtrl)

public:
	CKadSearchListCtrl();
	virtual ~CKadSearchListCtrl();

	void	SearchAdd(const Kademlia::CSearch* search);
	void	SearchRem(const Kademlia::CSearch* search);
	void	SearchRef(const Kademlia::CSearch* search);

	void	Init();
	void	Localize();
	void	Hide() {ShowWindow(SW_HIDE);}
	void	Visable() {ShowWindow(SW_SHOW);}
	void	UpdateKadSearchCount();

protected:
	void UpdateSearch(int iItem, const Kademlia::CSearch* search);
	void SetAllIcons();

	static int CALLBACK SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

	virtual BOOL OnCommand(WPARAM wParam,LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnSysColorChange();
	afx_msg void OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg	void OnColumnClick( NMHDR* pNMHDR, LRESULT* pResult);
};
