/* 
 * $Id: IrcWnd.h 7701 2008-10-15 07:34:41Z huby $
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
#include "./ResizableLib/ResizableDialog.h"
#include "./IrcNickListCtrl.h"
#include "./IrcChannelListCtrl.h"
#include "./IrcChannelTabCtrl.h"
#include "./SplitterControl.h"

#if _ENABLE_NOUSE

class CIrcMain;

class CIrcWnd : public CResizableDialog
{
		DECLARE_DYNAMIC(CIrcWnd)

	public:
		CIrcWnd(CWnd* pParent = NULL);
		virtual ~CIrcWnd();
		void Localize();
		bool GetLoggedIn();
		void SetLoggedIn( bool bFlag );
		void SetSendFileString( CString sInFile );
		CString GetSendFileString();
		bool IsConnected();
		void UpdateFonts(CFont* pFont);
		void ParseChangeMode( CString sChannel, CString sChanger, CString sCommands, CString sParams );
		void AddStatus( CString sReceived, ... );
		void AddInfoMessage( CString sChannelName, CString sReceived, ... );
		void AddColourLine(CString line, Channel* update_channel);//Interprets colour and other formatting tags
		void AddMessage( CString sChannelName, CString sTargetname, CString sLine,...);
		void SetConnectStatus( bool bConnected );
		void NoticeMessage( CString sSource, CString sTarget, CString sMessage );
		CString StripMessageOfFontCodes( CString sTemp );
		CString StripMessageOfColorCodes( CString sTemp );
		void SetTitle( CString sChannel, CString sTitle );
		void SendString( CString sSend );
		enum { IDD = IDD_IRC };
		afx_msg void OnBnClickedClosechat(int iItem=-1);
		CEdit m_editTitleWindow;
		CEdit m_editInputWindow;
		CIrcMain* m_pIrcMain;
		CToolTipCtrl* m_pToolTip;
		CIrcChannelTabCtrl m_tabctrlChannelSelect;
		CIrcNickListCtrl m_listctrlNickList;
		CIrcChannelListCtrl m_listctrlServerChannelList;
	protected:
		virtual BOOL OnInitDialog();
		virtual void OnSize(UINT iType, int iCx, int iCy);
		virtual int OnCreate(LPCREATESTRUCT lpCreateStruct);
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		virtual BOOL OnCommand(WPARAM wParam,LPARAM lParam );
		virtual BOOL PreTranslateMessage(MSG* pMsg);
		DECLARE_MESSAGE_MAP()
		afx_msg void OnSysColorChange();
		afx_msg void OnBnClickedBnIrcconnect();
		afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
		afx_msg void OnBnClickedChatsend();
		afx_msg LRESULT OnCloseTab(WPARAM wParam, LPARAM lParam);
		afx_msg LRESULT OnQueryTab(WPARAM wParam, LPARAM lParam);
		afx_msg void OnBnClickedColour();
		afx_msg void OnBnClickedUnderline();
		afx_msg void OnBnClickedBold();
		afx_msg void OnBnClickedReset();
		afx_msg LONG OnSelEndOK(UINT lParam, LONG /*wParam*/);
		afx_msg LONG OnSelEndCancel(UINT lParam, LONG /*wParam*/);
		void DoResize(int iDelta);
		virtual LRESULT DefWindowProc(UINT uMessage, WPARAM wParam, LPARAM lParam);
		CSplitterControl m_wndSplitterIRC;
	private:
		void OnChatTextChange();
		void AutoComplete();
		CString m_sSendString;
		bool m_bLoggedIn;
		bool m_bConnected;
};

#endif
