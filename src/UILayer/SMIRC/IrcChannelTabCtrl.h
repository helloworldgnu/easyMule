/* 
 * $Id: IrcChannelTabCtrl.h 7701 2008-10-15 07:34:41Z huby $
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
#include "./ClosableTabCtrl.h"

#if _ENABLE_NOUSE

struct Channel;

class CIrcChannelTabCtrl : public CClosableTabCtrl
{
		DECLARE_DYNAMIC(CIrcChannelTabCtrl)
	public:
		CIrcChannelTabCtrl();
		virtual ~CIrcChannelTabCtrl();
		void Init();
		void Localize();
		Channel* FindChannelByName(CString sName);
		Channel* NewChannel(CString sName, uint8 uType);
		void RemoveChannel( CString sChannel );
		void DeleteAllChannel();
		bool ChangeChanMode( CString sChannel, CString sParam, CString sDir, CString sCommand );
		void ScrollHistory(bool bDown);
		void Chatsend( CString sSend );
		CString m_sChannelModeSettingsTypeA;
		CString m_sChannelModeSettingsTypeB;
		CString m_sChannelModeSettingsTypeC;
		CString m_sChannelModeSettingsTypeD;
		CPtrList m_ptrlistChannel;
		Channel* m_pCurrentChannel;
	protected:
		friend class CIrcWnd;
		DECLARE_MESSAGE_MAP()
		afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
		afx_msg void OnTcnSelchangeTab2(NMHDR *pNMHDR, LRESULT *pResult);
		afx_msg void OnSysColorChange();
		virtual BOOL OnCommand(WPARAM wParam,LPARAM lParam );
	private:
		void SetActivity( CString sChannel, bool bFlag);
		int	GetTabUnderMouse(CPoint point);
		void SetAllIcons();
		CIrcWnd* m_pParent;
		CImageList m_imlistIRC;
};
#endif
