/*
 * $Id: ServerListCtrl.h 4483 2008-01-02 09:19:06Z soarchin $
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
#include "MuleListCtrl.h"
#include "MenuXP.h"

class CServerList; 
class CServer;
class CToolTipCtrlX;

class CServerListCtrl : public CMuleListCtrl 
{
	DECLARE_DYNAMIC(CServerListCtrl)
public:
	void	RefreshAllServer();//EastShare - added by AndCycle, IP to Country
	CServerListCtrl();
	virtual ~CServerListCtrl();

	bool	Init();
	bool	AddServer(const CServer* pServer, bool bAddToList = true);
	void	RemoveServer(const CServer* pServer);
	bool	AddServerMetToList(const CString& strFile);
	void	RefreshServer(const CServer* pServer);
	void	RemoveAllDeadServers();
	void	RemoveAllFilteredServers();
	void	Hide()		{ ShowWindow(SW_HIDE); }
	void	Visable()	{ ShowWindow(SW_SHOW); }
	void	Localize();
	void	ShowServerCount();
	bool	StaticServerFileAppend(CServer* pServer);
	bool	StaticServerFileRemove(CServer* pServer);

protected:
	CToolTipCtrlX*	m_tooltip;
	CMenuXP*	m_pMenuXP;

	CString CreateSelectedServersURLs();
	void DeleteSelectedServers();

	void SetSelectedServersPriority(UINT uPriority);
	void SetAllIcons();
	static int CALLBACK SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct); //与 IP to Country 有关

	DECLARE_MESSAGE_MAP()
	afx_msg void OnSysColorChange();
	afx_msg	void OnColumnClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnNMDblClk(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnGetInfoTip(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomDraw(NMHDR *pNMHDR, LRESULT *pResult);
private:
	//Chocobo Start
	//server icon related
	//服务器图标相关,  added by Chocobo
 	CImageList imagelist;
	//Chocobo End
public:
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
};
