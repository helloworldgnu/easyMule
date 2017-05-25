/* 
 * $Id: ChatWnd.h 7701 2008-10-15 07:34:41Z huby $
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
#include "ResizableLib\ResizableDialog.h"
#include "ChatSelector.h"
#include "FriendListCtrl.h"
#include "SplitterControl.h"
#include "IconStatic.h"

#if _ENABLE_NOUSE

class CChatWnd : public CResizableDialog
{
	DECLARE_DYNAMIC(CChatWnd)

public:
	CChatWnd(CWnd* pParent = NULL);   // standard constructor
	virtual ~CChatWnd();

	void ScrollHistory(bool down);
	CChatSelector chatselector;

// Dialog Data
	enum { IDD = IDD_CHAT };
	void StartSession(CUpDownClient* client);
	void Localize();
	void UpdateFriendlistCount(UINT count);

	CFriendListCtrl m_FriendListCtrl;

private:
	void ShowFriendMsgDetails(CFriend* pFriend); // [TPT] - New friend message window
	CIconStatic m_cUserInfo;

protected:
	CEdit inputtext;
	HICON icon_friend;
	HICON icon_msg;
	CSplitterControl m_wndSplitterchat;

	void SetAllIcons();
	void DoResize(int delta);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support	
	virtual BOOL OnInitDialog(); 
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSysColorChange();
	afx_msg LRESULT OnCloseTab(WPARAM wparam, LPARAM lparam);
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnLvnItemActivateFrlist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickFrlist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnStnDblclickFriendsicon();
};
#endif
