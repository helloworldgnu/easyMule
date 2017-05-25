/* 
 * $Id: IrcNickListCtrl.h 7701 2008-10-15 07:34:41Z huby $
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
#include "./MuleListCtrl.h"

#if _ENABLE_NOUSE

struct Nick;

class CIrcNickListCtrl : public CMuleListCtrl
{
		DECLARE_DYNAMIC(CIrcNickListCtrl)

	public:
		CIrcNickListCtrl();
		void Init();
		Nick* FindNickByName(CString sChannel, CString sName);
		Nick* NewNick(CString sChannel, CString sNick);
		void RefreshNickList( CString sChannel );
		bool RemoveNick( CString sChannel, CString sNick );
		void DeleteAllNick( CString sChannel );
		void DeleteNickInAll ( CString sNice, CString sMessage );
		bool ChangeNick( CString sChannel, CString sOldnick, CString sNewnick );
		bool ChangeNickMode( CString sChannel, CString sNice, CString sMode );
		void ChangeAllNick( CString sOldnick, CString sNewnick );
		void UpdateNickCount();
		void Localize();
		CString m_sUserModeSettings;
		CString m_sUserModeSymbols;
	protected:
		friend class CIrcWnd;
		int		m_iSortIndex;
		bool	m_bSortOrder;
		static int CALLBACK SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
		DECLARE_MESSAGE_MAP()
		afx_msg void OnLvnColumnclick(NMHDR *pNMHDR, LRESULT *pResult);
		afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
		afx_msg void OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);
		virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam );
	private:
		CIrcWnd* m_pParent;
};
#endif
