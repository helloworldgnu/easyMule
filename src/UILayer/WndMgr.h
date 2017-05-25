/*
 * $Id: WndMgr.h 5411 2008-04-29 08:21:19Z thilon $
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

class CWndMgr
{
public:
	CWndMgr(void);
	~CWndMgr(void);

	enum EWndId
	{
		WI_DOWNLOADED_LISTCTRL,
		WI_DOWNLOADING_LISTCTRL,
		WI_MAINTAB_DOWNLOAD_DLG,
		WI_ADVANCE_TOOLBAR,
		WI_MAX
	};

	void	SetWndHandle(EWndId eWndId, HWND hWnd){m_ahWnds[eWndId] = hWnd;}
	HWND	GetWndHandle(EWndId eWndId){return m_ahWnds[eWndId];}

	LRESULT	SendMsgTo(EWndId eWndId, UINT uMsg, WPARAM wParam = 0, LPARAM lParam = 0)
	{
		return ::SendMessage(GetWndHandle(eWndId), uMsg, wParam, lParam);
	}
	BOOL PostMsgTo(EWndId eWndId, UINT uMsg, WPARAM wParam = 0, LPARAM lParam = 0)
	{
		return ::PostMessage(GetWndHandle(eWndId), uMsg, wParam, lParam);
	}

	CToolBarCtrl	*m_pTbcShare;

protected:
	HWND	m_ahWnds[WI_MAX];
};

extern CWndMgr	theWndMgr;
