/* 
 * $Id: ChatSelector.h 7701 2008-10-15 07:34:41Z huby $
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
#include "ClosableTabCtrl.h"

#if _ENABLE_NOUSE

class CHTRichEditCtrl;
class CUpDownClient;


///////////////////////////////////////////////////////////////////////////////
// CChatItem

class CChatItem
{
public:
	CChatItem();
	~CChatItem();

	CUpDownClient*		client;
	CHTRichEditCtrl*	log;
	CString				strMessagePending;
	bool				notify;
	CStringArray		history;
	int					history_pos;
};


///////////////////////////////////////////////////////////////////////////////
// CChatSelector

class CChatSelector : public CClosableTabCtrl
{
	DECLARE_DYNAMIC(CChatSelector)

public:
	CChatSelector();
	virtual	~CChatSelector();

	void		Init();
	void		Localize(void);
	CChatItem*	StartSession(CUpDownClient* client, bool show = true);
	void		EndSession(CUpDownClient* client = 0);
	int			GetTabByClient(CUpDownClient* client);
	CChatItem*	GetItemByClient(CUpDownClient* client);
	void		ProcessMessage(CUpDownClient* sender, const CString& message);
	bool		SendMessage(const CString& rstrMessage);
	void		DeleteAllItems();
	void		ShowChat();
	void		ConnectingResult(CUpDownClient* sender,bool success);
	void		Send();
	void		UpdateFonts(CFont* pFont);
	CChatItem*	GetCurrentChatItem();
	BOOL		RemoveItem(int nItem) { return DeleteItem(nItem); }

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnTcnSelchangeChatsel(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedCsend();
	afx_msg void OnBnClickedCclose();
	afx_msg void OnSysColorChange();
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual int InsertItem(int nItem, TCITEM* pTabCtrlItem);
	virtual BOOL DeleteItem(int nItem);
	//virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	void AddTimeStamp(CChatItem*);
	bool IsSpam(CString strMessage, CUpDownClient* client);
	void SetAllIcons();
	void GetChatSize(CRect& rcChat);

private:
	UINT_PTR m_Timer;
	bool m_blinkstate;
	bool m_lastemptyicon;
	HWND m_hwndMessageBox;
	HWND m_hwndCloseBtn;
	HWND m_hwndSendBtn;
	CImageList m_imlChat;
};
#endif
