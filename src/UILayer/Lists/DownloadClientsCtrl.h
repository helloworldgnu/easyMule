/*
 * $Id: DownloadClientsCtrl.h 4483 2008-01-02 09:19:06Z soarchin $
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
//--- xrmb:downloadclientslist ---

//this file is part of eMule
//Copyright (C)2002 Merkur ( merkur-@users.sourceforge.net / http://www.emule-project.net )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#pragma once
#include "MuleListCtrl.h"
#include "ListCtrlItemWalk.h"

class CUpDownClient;
class CPartFile;

class CDownloadClientsCtrl : public CMuleListCtrl, public CListCtrlItemWalk
{
	DECLARE_DYNAMIC(CDownloadClientsCtrl)

public:
	CDownloadClientsCtrl();
	virtual ~CDownloadClientsCtrl();

	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	void	Init();
	void	AddClient(CUpDownClient* client);
	void	RemoveClient(CUpDownClient* client);
	void	RefreshClient(CUpDownClient* client);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	void	Hide() { ShowWindow(SW_HIDE); }
	void	Show() { ShowWindow(SW_SHOW); }
	void	Localize();
	void	ShowSelectedUserDetails();

protected:
	CImageList  m_ImageList;
	CImageList  m_overlayimages;
	
	void SetAllIcons();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnSysColorChange();
	afx_msg void OnNMRclick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg	void OnColumnClick( NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMDblclkDownloadClientlist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	static int CALLBACK SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	static int Compare(CUpDownClient* client1, CUpDownClient* client2, CPartFile* file1, CPartFile* file2, LPARAM lParamSort, int sortMod);
};
