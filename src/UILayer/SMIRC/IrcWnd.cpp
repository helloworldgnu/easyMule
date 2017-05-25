/* 
 * $Id: IrcWnd.cpp 7701 2008-10-15 07:34:41Z huby $
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

#include "stdafx.h"
#include "./emule.h"
#include "./emuleDlg.h"
#include "./IrcWnd.h"
#include "./IrcMain.h"
#include "./otherfunctions.h"
#include "./MenuCmds.h"
#include "./HTRichEditCtrl.h"
#include "./ClosableTabCtrl.h"
#include "./HelpIDs.h"
#include "./opcodes.h"
#include "./InputBox.h"
#include "./UserMsgs.h"
#include "./ColourPopup.h"

#if _ENABLE_NOUSE

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define NICK_LV_PROFILE_NAME _T("IRCNicksLv")
#define CHAN_LV_PROFILE_NAME _T("IRCChannelsLv")

struct Nick
{
	CString m_sNick;
	CString m_sModes;
	int m_iLevel;
};

struct Channel
{
	CString	m_sName;
	CString m_sModesA;
	CString m_sModesB;
	CString m_sModesC;
	CString m_sModesD;
	CHTRichEditCtrl m_editctrlLog;
	CString m_sTitle;
	CPtrList m_ptrlistNicks;
	uint8 m_uType;
	CStringArray m_sarrayHistory;
	int m_iHistoryPos;
	// Type is mainly so that we can use this for IRC and the eMule Messages..
	// 1-Status, 2-Channel list, 4-Channel, 5-Private Channel, 6-eMule Message(Add later)
};

IMPLEMENT_DYNAMIC(CIrcWnd, CDialog)

BEGIN_MESSAGE_MAP(CIrcWnd, CResizableDialog)
// Tab control
ON_WM_SIZE()
ON_WM_CREATE()
ON_WM_CONTEXTMENU()
ON_WM_SYSCOLORCHANGE()
ON_WM_HELPINFO()
ON_MESSAGE(UM_CLOSETAB, OnCloseTab)
ON_MESSAGE(UM_QUERYTAB, OnQueryTab)
ON_MESSAGE(UM_CPN_SELENDOK, OnSelEndOK)
ON_MESSAGE(UM_CPN_SELENDCANCEL, OnSelEndCancel)
END_MESSAGE_MAP()

CIrcWnd::CIrcWnd(CWnd* pParent ) : CResizableDialog(CIrcWnd::IDD, pParent)
{
	m_pIrcMain = NULL;
	m_bConnected = false;
	m_bLoggedIn = false;
	m_listctrlNickList.m_pParent = this;
	m_listctrlServerChannelList.m_pParent = this;
	m_tabctrlChannelSelect.m_bCloseable = true;
	m_tabctrlChannelSelect.m_pParent = this;
	m_pToolTip = NULL;
}

CIrcWnd::~CIrcWnd()
{
	if( m_bConnected )
	{
		//Do a safe disconnect
		m_pIrcMain->Disconnect(true);
	}
	//Delete our core client..
	delete m_pIrcMain;
	//destroy tooltips if they were created
	if(m_pToolTip!=NULL)
		m_pToolTip->DestroyToolTipCtrl();
}

void CIrcWnd::OnSysColorChange()
{
	CResizableDialog::OnSysColorChange();
}

void CIrcWnd::Localize()
{
	//Set all controls to the correct language.
	if( m_bConnected )
		GetDlgItem(IDC_BN_IRCCONNECT)->SetWindowText(GetResString(IDS_IRC_DISCONNECT));
	else
		GetDlgItem(IDC_BN_IRCCONNECT)->SetWindowText(GetResString(IDS_IRC_CONNECT));
	GetDlgItem(IDC_CHATSEND)->SetWindowText(GetResString(IDS_IRC_SEND));
	GetDlgItem(IDC_CLOSECHAT)->SetWindowText(GetResString(IDS_FD_CLOSE));
	m_listctrlServerChannelList.Localize();
	m_tabctrlChannelSelect.Localize();
	m_listctrlNickList.Localize();

	if (m_pToolTip)
		m_pToolTip->DestroyToolTipCtrl();
	m_pToolTip = new CToolTipCtrl();
	m_pToolTip->Create(this);
	m_pToolTip->AddTool(GetDlgItem(IDC_BOLD),	GetResString(IDS_BOLD) );
	m_pToolTip->AddTool(GetDlgItem(IDC_COLOUR),	GetResString(IDS_COLOUR) );
	m_pToolTip->AddTool(GetDlgItem(IDC_RESET),	GetResString(IDS_RESETFORMAT) );
	m_pToolTip->AddTool(GetDlgItem(IDC_UNDERLINE),GetResString(IDS_UNDERLINE) );
	m_pToolTip->Activate(TRUE);
}

BOOL CIrcWnd::OnInitDialog()
{
	CResizableDialog::OnInitDialog();
#ifdef _DEBUG

	CString sBuffer;
	m_listctrlNickList.GetWindowText(sBuffer);
	ASSERT( sBuffer == NICK_LV_PROFILE_NAME );

	sBuffer.Empty();
	m_listctrlServerChannelList.GetWindowText(sBuffer);
	ASSERT( sBuffer == CHAN_LV_PROFILE_NAME );
#endif

	m_bConnected = false;
	m_bLoggedIn = false;
	Localize();
	m_pIrcMain = new CIrcMain();
	m_pIrcMain->SetIRCWnd(this);

	UpdateFonts(&theApp.m_fontHyperText);
	InitWindowStyles(this);

	((CEdit*)GetDlgItem(IDC_INPUTWINDOW))->SetLimitText(MAX_IRC_MSG_LEN);

	CRect rc, rcSpl;

	GetDlgItem(IDC_NICKLIST)->GetWindowRect(rcSpl);
	ScreenToClient(rcSpl);

	GetWindowRect(rc);
	ScreenToClient(rc);

	rcSpl.bottom=rc.bottom-10;
	rcSpl.left=rcSpl.right +3;
	rcSpl.right=rcSpl.left+4;
	m_wndSplitterIRC.Create(WS_CHILD | WS_VISIBLE, rcSpl, this, IDC_SPLITTER_IRC);

	AddAnchor(IDC_BN_IRCCONNECT,BOTTOM_LEFT);
	AddAnchor(IDC_CLOSECHAT,BOTTOM_LEFT);
	AddAnchor(IDC_CHATSEND,BOTTOM_RIGHT);
	AddAnchor(IDC_BOLD,BOTTOM_LEFT);
	AddAnchor(IDC_RESET,BOTTOM_LEFT);
	AddAnchor(IDC_COLOUR,BOTTOM_LEFT);
	AddAnchor(IDC_UNDERLINE,BOTTOM_LEFT);
	AddAnchor(IDC_INPUTWINDOW,BOTTOM_LEFT,BOTTOM_RIGHT);
	AddAnchor(IDC_NICKLIST,TOP_LEFT,BOTTOM_LEFT);
	AddAnchor(IDC_TITLEWINDOW,TOP_LEFT,TOP_RIGHT);
	AddAnchor(IDC_SERVERCHANNELLIST,TOP_LEFT,BOTTOM_RIGHT);
	AddAnchor(IDC_TAB2,TOP_LEFT, TOP_RIGHT);
	AddAnchor(m_wndSplitterIRC,TOP_LEFT, BOTTOM_LEFT);

	((CButton*)GetDlgItem(IDC_BOLD))->SetIcon(theApp.LoadIcon(_T("BOLD")));
	((CButton*)GetDlgItem(IDC_COLOUR))->SetIcon(theApp.LoadIcon(_T("COLOUR")));
	((CButton*)GetDlgItem(IDC_RESET))->SetIcon(theApp.LoadIcon(_T("RESETFORMAT")));
	((CButton*)GetDlgItem(IDC_UNDERLINE))->SetIcon(theApp.LoadIcon(_T("UNDERLINE")));

	int iPosStatInit = rcSpl.left;
	int iPosStatNew = thePrefs.GetSplitterbarPositionIRC();
	if (thePrefs.GetSplitterbarPositionIRC() > 600)
		iPosStatNew = 600;
	else if (thePrefs.GetSplitterbarPositionIRC() < 200)
		iPosStatNew = 200;
	rcSpl.left = iPosStatNew;
	rcSpl.right = iPosStatNew+5;

	m_wndSplitterIRC.MoveWindow(rcSpl);
	DoResize(iPosStatNew-iPosStatInit);

	m_listctrlServerChannelList.Init();
	m_listctrlNickList.Init();
	m_tabctrlChannelSelect.Init();
	OnChatTextChange();

	return true;
}

void CIrcWnd::DoResize(int iDelta)
{

	CSplitterControl::ChangeWidth(GetDlgItem(IDC_NICKLIST), iDelta);
	CSplitterControl::ChangeWidth(GetDlgItem(IDC_INPUTWINDOW), -iDelta, CW_RIGHTALIGN);
	CSplitterControl::ChangeWidth(GetDlgItem(IDC_TITLEWINDOW), -iDelta, CW_RIGHTALIGN);
	CSplitterControl::ChangeWidth(GetDlgItem(IDC_SERVERCHANNELLIST), -iDelta, CW_RIGHTALIGN);
	CSplitterControl::ChangeWidth(GetDlgItem(IDC_STATUSWINDOW), -iDelta, CW_RIGHTALIGN);
	CSplitterControl::ChangeWidth(GetDlgItem(IDC_TAB2), -iDelta, CW_RIGHTALIGN);
	CSplitterControl::ChangePos(GetDlgItem(IDC_BOLD), -iDelta, 0);
	CSplitterControl::ChangePos(GetDlgItem(IDC_RESET), -iDelta, 0);
	CSplitterControl::ChangePos(GetDlgItem(IDC_COLOUR), -iDelta, 0);
	CSplitterControl::ChangePos(GetDlgItem(IDC_UNDERLINE), -iDelta, 0);

	CRect rcChannel;
	m_listctrlServerChannelList.GetWindowRect(&rcChannel);
	ScreenToClient(&rcChannel);
	if (m_tabctrlChannelSelect.m_pCurrentChannel)
		m_tabctrlChannelSelect.m_pCurrentChannel->m_editctrlLog.SetWindowPos(NULL, rcChannel.left, rcChannel.top, rcChannel.Width(), rcChannel.Height(), SWP_NOZORDER);

	CRect rcW;

	GetWindowRect(rcW);
	ScreenToClient(rcW);

	CRect rcspl;
	GetDlgItem(IDC_NICKLIST)->GetClientRect(rcspl);

	thePrefs.SetSplitterbarPositionIRC(rcspl.right);

	RemoveAnchor(IDC_BN_IRCCONNECT);
	AddAnchor(IDC_BN_IRCCONNECT,BOTTOM_LEFT);
	RemoveAnchor(IDC_CLOSECHAT);
	AddAnchor(IDC_CLOSECHAT,BOTTOM_LEFT);
	RemoveAnchor(IDC_BOLD);
	AddAnchor(IDC_BOLD,BOTTOM_LEFT);
	RemoveAnchor(IDC_RESET);
	AddAnchor(IDC_RESET,BOTTOM_LEFT);
	RemoveAnchor(IDC_COLOUR);
	AddAnchor(IDC_COLOUR,BOTTOM_LEFT);
	RemoveAnchor(IDC_UNDERLINE);
	AddAnchor(IDC_UNDERLINE,BOTTOM_LEFT);
	RemoveAnchor(IDC_INPUTWINDOW);
	AddAnchor(IDC_INPUTWINDOW,BOTTOM_LEFT,BOTTOM_RIGHT);
	RemoveAnchor(IDC_NICKLIST);
	AddAnchor(IDC_NICKLIST,TOP_LEFT,BOTTOM_LEFT);
	RemoveAnchor(IDC_TITLEWINDOW);
	AddAnchor(IDC_TITLEWINDOW,TOP_LEFT,TOP_RIGHT);
	RemoveAnchor(IDC_SERVERCHANNELLIST);
	AddAnchor(IDC_SERVERCHANNELLIST,TOP_LEFT,BOTTOM_RIGHT);
	RemoveAnchor(IDC_TAB2);
	AddAnchor(IDC_TAB2,TOP_LEFT, TOP_RIGHT);
	RemoveAnchor(m_wndSplitterIRC);
	AddAnchor(m_wndSplitterIRC,TOP_LEFT, BOTTOM_LEFT);

	m_wndSplitterIRC.SetRange(rcW.left+190, rcW.left+600);

	Invalidate();
	UpdateWindow();
}

LRESULT CIrcWnd::DefWindowProc(UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	switch (uMessage)
	{
		case WM_PAINT:
			if (m_wndSplitterIRC)
			{
				CRect rctree, rcSpl, rcW;
				CWnd* pWnd;

				GetWindowRect(rcW);
				ScreenToClient(rcW);

				pWnd = GetDlgItem(IDC_NICKLIST);
				pWnd->GetWindowRect(rctree);

				ScreenToClient(rctree);

				if (rcW.Width()>0)
				{
					rcSpl.left=rctree.right;
					rcSpl.right=rcSpl.left+5;
					rcSpl.top=rctree.top;
					rcSpl.bottom=rcW.bottom-40;
					m_wndSplitterIRC.MoveWindow(rcSpl, true);
				}
			}
			break;
		case WM_NOTIFY:
			if (wParam == IDC_SPLITTER_IRC)
			{
				SPC_NMHDR* pHdr = (SPC_NMHDR*) lParam;
				DoResize(pHdr->delta);
			}
			break;
		case WM_WINDOWPOSCHANGED :
			{
				CRect rcW;
				GetWindowRect(rcW);
				ScreenToClient(rcW);
				if (m_wndSplitterIRC && rcW.Width()>0)
					Invalidate();
				break;
			}
		case WM_SIZE:
			{
				//set range
				if (m_wndSplitterIRC)
				{
					CRect rc;
					GetWindowRect(rc);
					ScreenToClient(rc);
					m_wndSplitterIRC.SetRange(rc.left+190 , rc.left+600);
				}
				break;
			}
	}

	return CResizableDialog::DefWindowProc(uMessage, wParam, lParam);

}

void CIrcWnd::UpdateFonts(CFont* pFont)
{
	TCITEM tci;
	tci.mask = TCIF_PARAM;
	int iIndex = 0;
	while (m_tabctrlChannelSelect.GetItem(iIndex++, &tci))
	{
		Channel* pChannel = (Channel*)tci.lParam;
		if (pChannel->m_editctrlLog.m_hWnd != NULL)
			pChannel->m_editctrlLog.SetFont(pFont);
	}
}

void CIrcWnd::OnSize(UINT uType, int iCx, int iCy)
{
	CResizableDialog::OnSize(uType, iCx, iCy);

	if (m_tabctrlChannelSelect.m_pCurrentChannel && m_tabctrlChannelSelect.m_pCurrentChannel->m_editctrlLog.m_hWnd)
	{
		CRect rcChannel;
		m_listctrlServerChannelList.GetWindowRect(&rcChannel);
		ScreenToClient(&rcChannel);
		m_tabctrlChannelSelect.m_pCurrentChannel->m_editctrlLog.SetWindowPos(NULL, rcChannel.left, rcChannel.top, rcChannel.Width(), rcChannel.Height(), SWP_NOZORDER);
	}
}

int CIrcWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	return CResizableDialog::OnCreate(lpCreateStruct);
}

void CIrcWnd::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_NICKLIST, m_listctrlNickList);
	DDX_Control(pDX, IDC_INPUTWINDOW, m_editInputWindow);
	DDX_Control(pDX, IDC_TITLEWINDOW, m_editTitleWindow);
	DDX_Control(pDX, IDC_SERVERCHANNELLIST, m_listctrlServerChannelList);
	DDX_Control(pDX, IDC_TAB2, m_tabctrlChannelSelect);
}

BOOL CIrcWnd::OnCommand(WPARAM wParam, LPARAM)
{
	switch( wParam )
	{
		case IDC_BN_IRCCONNECT:
			{
				//Pressed the connect button..
				OnBnClickedBnIrcconnect();
				return true;
			}
		case IDC_CHATSEND:
			{
				//Pressed the send button..
				OnBnClickedChatsend();
				return true;
			}
		case IDC_CLOSECHAT:
			{
				//Pressed the close button
				OnBnClickedClosechat();
				return true;
			}
		case IDC_BOLD:
			{
				OnBnClickedBold();
				return true;
			}
		case IDC_COLOUR:
			{
				OnBnClickedColour();
				return true;
			}
		case IDC_UNDERLINE:
			{
				OnBnClickedUnderline();
				return true;
			}
		case IDC_RESET:
			{
				OnBnClickedReset();
				return true;
			}
	}
	return true;
}

BOOL CIrcWnd::PreTranslateMessage(MSG* pMsg)
{
	if(NULL != m_pToolTip)
		m_pToolTip->RelayEvent(pMsg);

	if (pMsg->message == WM_KEYDOWN)
	{
		// Don't handle Ctrl+Tab in this window. It will be handled by main window.
		if (pMsg->wParam == VK_TAB && GetAsyncKeyState(VK_CONTROL) < 0)
			return FALSE;

		if (pMsg->hwnd == GetDlgItem(IDC_INPUTWINDOW)->m_hWnd)
		{
			if (pMsg->wParam == VK_RETURN)
			{
				//If we press the enter key, treat is as if we pressed the send button.
				OnBnClickedChatsend();
				return TRUE;
			}

			if (pMsg->wParam == VK_UP || pMsg->wParam == VK_DOWN)
			{
				//If we press page up/down scroll..
				m_tabctrlChannelSelect.ScrollHistory(pMsg->wParam == VK_DOWN);
				return TRUE;
			}

			if (pMsg->wParam == VK_TAB)
			{
				AutoComplete();
				return TRUE;
			}
		}
	}
	OnChatTextChange();
	return CResizableDialog::PreTranslateMessage(pMsg);
}

void CIrcWnd::AutoComplete()
{
	CString sSend;
	CString sName;
	GetDlgItem(IDC_INPUTWINDOW)->GetWindowText(sSend);
	if( sSend.ReverseFind(_T(' ')) == -1 )
	{
		if(!sSend.GetLength())
			return;
		sName = sSend;
		sSend = _T("");
	}
	else
	{
		sName = sSend.Mid(sSend.ReverseFind(_T(' '))+1);
		sSend = sSend.Mid(0, sSend.ReverseFind(_T(' '))+1);
	}

	POSITION pos1, pos2;
	for (pos1 = m_tabctrlChannelSelect.m_pCurrentChannel->m_ptrlistNicks.GetHeadPosition();( pos2 = pos1 ) != NULL;)
	{
		m_tabctrlChannelSelect.m_pCurrentChannel->m_ptrlistNicks.GetNext(pos1);
		Nick* pCurrNick = (Nick*)(m_tabctrlChannelSelect.m_pCurrentChannel)->m_ptrlistNicks.GetAt(pos2);
		if (pCurrNick->m_sNick.Left(sName.GetLength()) == sName)
		{
			sName = pCurrNick->m_sNick;
			GetDlgItem(IDC_INPUTWINDOW)->SetWindowText(sSend+sName);
			GetDlgItem(IDC_INPUTWINDOW)->SetFocus();
			GetDlgItem(IDC_INPUTWINDOW)->SendMessage(WM_KEYDOWN, VK_END);
			break;
		}
	}
}

void CIrcWnd::OnBnClickedBnIrcconnect()
{
	if(!m_bConnected)
	{
		CString sInput = thePrefs.GetIRCNick();
		sInput.Trim();
		sInput = sInput.SpanExcluding(_T(" !@#$%^&*():;<>,.?{}~`+=-"));
		sInput = sInput.Left(25);
		if( thePrefs.GetIRCNick().MakeLower() == _T("emule") || thePrefs.GetIRCNick().MakeLower().Find(_T("emuleirc")) != -1 || sInput == "" )
		{
			do
			{
				InputBox inputBox;
				inputBox.SetLabels(GetResString(IDS_IRC_NEWNICK), GetResString(IDS_IRC_NEWNICKDESC), _T("eMule"));
				if (inputBox.DoModal() == IDOK)
				{
					sInput = inputBox.GetInput();
					sInput.Trim();
					sInput = sInput.SpanExcluding(_T(" !@#$%^&*():;<>,.?{}~`+=-"));
					sInput = sInput.Left(25);
				}
				else
				{
					if(sInput.IsEmpty())
					{
						sInput = _T("eMule");
					}
				}
			}
			while(sInput.IsEmpty());
		}
		thePrefs.SetIRCNick(sInput);
		//if not connected, connect..
		m_pIrcMain->Connect();
	}
	else
	{
		//If connected, disconnect..
		m_pIrcMain->Disconnect();
	}
}

void CIrcWnd::OnBnClickedClosechat(int iItem)
{
	//Remove a channel..
	TCITEM item;
	item.mask = TCIF_PARAM;
	if (iItem == -1)
	{
		//If no item was send, get our current channel..
		iItem = m_tabctrlChannelSelect.GetCurSel();
	}

	if (iItem == -1)
	{
		//We have no channel, abort.
		return;
	}

	if (!m_tabctrlChannelSelect.GetItem(iItem, &item))
	{
		//We had no valid item here.. Something isn't right..
		//TODO: this should never happen, so maybe we should remove this tab?
		return;
	}
	Channel* pPartChannel = (Channel*)item.lParam;
	if( pPartChannel->m_uType == 4 &&  m_bConnected)
	{
		//If this was a channel and we were connected, do not just delete the channel!!
		//Send a part command and the server must respond with a successful part which will remove the channel!
		CString sPart;
		sPart = _T("PART ") + pPartChannel->m_sName;
		m_pIrcMain->SendString( sPart );
		return;
	}
	else if (pPartChannel->m_uType == 5 || pPartChannel->m_uType == 4)
	{
		//If this is a private room, we just remove it as the server doesn't track this.
		//If this was a channel, but we are disconnected, remove the channel..
		m_tabctrlChannelSelect.RemoveChannel(pPartChannel->m_sName);
		return;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
// Messages
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

void CIrcWnd::AddStatus( CString sLine,...)
{
	//Add entry to status window with arguments..
	va_list argptr;
	va_start(argptr, sLine);
	CString sTemp;
	sTemp.FormatV(sLine, argptr);
	va_end(argptr);
	sLine = sTemp + _T("\r\n");
	//Now that incoming arguments are finished, it's now safe to put back the % chars.
	sLine.Replace( _T("\004"), _T("%") );

	CString sTimeStamp;
	if( thePrefs.GetIRCAddTimestamp() )
		sTimeStamp = CTime::GetCurrentTime().Format(_T("%X: "));

	Channel* pUpdateChannel = (Channel*)(m_tabctrlChannelSelect.m_ptrlistChannel).GetHead();

	//This should never happen!
	if( !pUpdateChannel )
		return;

	//This allows for us to add blank lines to the status..
	if (sLine == _T("\r\n") )
		pUpdateChannel->m_editctrlLog.AppendText(sLine);
	else if (sLine.Mid(0,1) == _T("*"))
	{
		AddColourLine(CString((TCHAR)0x03)+CString(_T("02"))+sTimeStamp,pUpdateChannel);
		pUpdateChannel->m_editctrlLog.AppendKeyWord(sLine.Left(2),RGB(255,0,0));
		AddColourLine(sLine.Mid(1),pUpdateChannel);
	}
	else
	{
		AddColourLine(CString((TCHAR)0x03)+CString(_T("02"))+sTimeStamp,pUpdateChannel);
		AddColourLine(sLine,pUpdateChannel);
	}
	if( m_tabctrlChannelSelect.m_pCurrentChannel == pUpdateChannel )
		return;
	m_tabctrlChannelSelect.SetActivity( pUpdateChannel->m_sName, true );
}

void CIrcWnd::AddInfoMessage( CString sChannelName, CString sLine,...)
{
	if(sChannelName.IsEmpty())
		return;
	va_list argptr;
	va_start(argptr, sLine);
	CString sTemp;
	sTemp.FormatV(sLine, argptr);
	va_end(argptr);
	sLine = sTemp + _T("\r\n");
	//Now that incoming arguments are finished, it's now safe to put back the % chars.
	sLine.Replace( _T("\004"), _T("%") );

	CString sTimeStamp;
	if( thePrefs.GetIRCAddTimestamp() )
		sTimeStamp = CTime::GetCurrentTime().Format(_T("%X: "));

	Channel* pUpdateChannel = m_tabctrlChannelSelect.FindChannelByName(sChannelName);
	if( !pUpdateChannel )
	{
		if( sChannelName.Left(1) == _T("#") )
			pUpdateChannel = m_tabctrlChannelSelect.NewChannel( sChannelName, 4);
		else
			pUpdateChannel = m_tabctrlChannelSelect.NewChannel( sChannelName, 5);
	}
	if (sLine.Mid(0,1) == _T("*"))
	{
		AddColourLine(CString((TCHAR)0x03)+CString(_T("02"))+sTimeStamp,pUpdateChannel);
		pUpdateChannel->m_editctrlLog.AppendKeyWord(sLine.Left(2),RGB(255,0,0));
		AddColourLine(sLine.Mid(1),pUpdateChannel);
	}
	else if (sLine.Mid(0,1) == _T("-") && sLine.Find( _T("-"), 1 ) != -1)
	{
		int iIndex = sLine.Find( _T("-"), 1 );
		AddColourLine(CString((TCHAR)0x03)+CString(_T("02"))+sTimeStamp,pUpdateChannel);
		pUpdateChannel->m_editctrlLog.AppendKeyWord(sLine.Left(iIndex),RGB(150,0,0));
		AddColourLine(sLine.Mid(iIndex),pUpdateChannel);
	}
	else
	{
		AddColourLine(CString((TCHAR)0x03)+CString(_T("02"))+sTimeStamp,pUpdateChannel);
		AddColourLine(sLine,pUpdateChannel);
	}
	if( m_tabctrlChannelSelect.m_pCurrentChannel == pUpdateChannel )
		return;
	m_tabctrlChannelSelect.SetActivity( pUpdateChannel->m_sName, true );
}

void CIrcWnd::AddMessage( CString sChannelName, CString sTargetName, CString sLine,...)
{
	if(sChannelName.IsEmpty() || sTargetName.IsEmpty())
		return;
	va_list argptr;
	va_start(argptr, sLine);
	CString sTemp;
	sTemp.FormatV(sLine, argptr);
	va_end(argptr);
	sLine = sTemp + _T("\r\n");
	//Now that incoming arguments are finished, it's now safe to put back the % chars.
	sLine.Replace( _T("\004"), _T("%") );

	CString sTimeStamp;
	if( thePrefs.GetIRCAddTimestamp() )
		sTimeStamp = CTime::GetCurrentTime().Format(_T("%X: "));

	Channel* pUpdateChannel = m_tabctrlChannelSelect.FindChannelByName(sChannelName);
	if( !pUpdateChannel )
	{
		if( sChannelName.Left(1) == _T("#") )
			pUpdateChannel = m_tabctrlChannelSelect.NewChannel( sChannelName, 4);
		else
			pUpdateChannel = m_tabctrlChannelSelect.NewChannel( sChannelName, 5);
	}
	COLORREF color;
	if (m_pIrcMain->GetNick() == sTargetName)
		color = RGB(1,100,1);
	else
		color = RGB(1,20,130);
	sTargetName = CString(_T("<"))+ sTargetName + CString(_T(">"));
	AddColourLine(CString((TCHAR)0x03)+CString(_T("02"))+sTimeStamp,pUpdateChannel);
	pUpdateChannel->m_editctrlLog.AppendKeyWord(sTargetName, color);
	AddColourLine(CString(_T(" "))+sLine,pUpdateChannel);
	if( m_tabctrlChannelSelect.m_pCurrentChannel == pUpdateChannel )
		return;
	m_tabctrlChannelSelect.SetActivity( pUpdateChannel->m_sName, true );
}

//To add colour functionality we need to isolate hyperlinks and send them to AppendColoredText! :)
static const struct
{
	LPCTSTR pszScheme;
	int iLen;
}
_apszSchemes[] =
    {
        { _T("ed2k://"),  7 },
        { _T("http://"),  7 },
        { _T("https://"), 8 },
        { _T("ftp://"),   6 },
        { _T("www."),     4 },
        { _T("ftp."),     4 },
        { _T("mailto:"),  7 }
    };

//colours in an array
static const COLORREF _colours[16] =
    {
        RGB(0xff,0xff,0xff),//white
        RGB(0x0,0x0,0x0),   //black
        RGB(0x0,0x0,0xb8),  //dark blue
        RGB(0x0,0x64,0x0),  //dark green
        RGB(0xff,0x0,0x0),  //red
        RGB(0xa5,0x2a,0x2a),//brown
        RGB(0x80,0x0,0x80), //purple
        RGB(0xff,0xa5,0x00),//orange
        RGB(0xff,0xff,0x0), //yellow
        RGB(0x0,0xff,0x0),  //green
        RGB(0x0,0x80,0x80), //teal
        RGB(0x0,0xff,0xff), //cyan
        RGB(0x0,0x0,0xff),  //blue
        RGB(0xff,0x69,0xb4),//pink
        RGB(0x80,0x80,0x80),//dark grey
        RGB(0xd3,0xd3,0xd3) //light grey
    };

//New Colour functionality + Bold & Italic
void CIrcWnd::AddColourLine(CString line,Channel* pUpdateChannel)
{//write a colour line to the screen
	TCHAR aChar;
	DWORD dwMask = 0;//text characteristics
	int index = 0;
	int linkfoundat = 0;//This variable is to save needless costly string manipulation
	COLORREF foregroundColour = GetSysColor(COLOR_WINDOWTEXT);//default foreground colour
	COLORREF cr = foregroundColour;//set start foreground colour
	COLORREF backgroundColour = GetSysColor(COLOR_WINDOW);//default background colour
	COLORREF bgcr = backgroundColour;//set start background colour COMMENTED left for possible future use
	CString text("");
	while(line.GetLength() > index)
	{
		aChar = line[index];//get TCHAR at point index
		//find any hyperlinks
		if(index == linkfoundat)//only run the link finding code once it a line with no links
		{
			for(int i = 0; i < ARRSIZE(_apszSchemes);)
			{
				CString CStr = line.Right(line.GetLength() - index);//make a string of what we have left
				int foundat = CStr.Find(_apszSchemes[i].pszScheme);//get position of link -1 == not found
				if(foundat==0)//link starts at this character
				{//link found
					if(!text.IsEmpty())
					{
						pUpdateChannel->m_editctrlLog.AppendColoredText(text, cr, bgcr, dwMask);//if any preceeding data write it
						text.Empty();//clear written text
					}
					int iLen = CStr.FindOneOf(_T(" \n\r\t"));//return URL length  // search next space or EOL
					if(iLen==-1)
					{
						pUpdateChannel->m_editctrlLog.AddLine(CStr, -1, true);//len = -1 add it all
						index = line.GetLength();//the whole line has been written
					}
					else
					{
						CString str= CStr.Left(iLen);//create a string of the URL
						pUpdateChannel->m_editctrlLog.AddLine(str, -1, true);//add it
						index+=iLen;//update our point in the line
						i = 0;//searh from the new position, using the whole array hence 'i' RESET to 0
						foundat = -1;//do not record this processed location as a future target location
						linkfoundat = index;//reset previous finds as i=0 we re-search
						aChar = line[index];//get a new char
					}
				}//end if scope
				else
				{//no link at this exact location
					i++;//only increment if not found at this position so if we find http at this position we check for further http occurances
					//foundat A Valid Position && (no valid position recorded || a farther position previously recorded)
					if(foundat!=-1 && (linkfoundat==index || (index + foundat)<linkfoundat))
						linkfoundat = index + foundat;//set the next closest link to process
				}
			}//end for scope
		}//end if scope
		switch ((int)aChar)
		{
			case 0x02://Bold toggle
				{
					if(!text.IsEmpty())
					{
						pUpdateChannel->m_editctrlLog.AppendColoredText(text, cr, bgcr, dwMask);//if any preceeding data write it
						text.Empty();//clear written text
					}
					index++;  //get next char
					if(dwMask & CFM_BOLD)
						dwMask ^= CFM_BOLD;//remove bold
					else
						dwMask |= CFM_BOLD;//add bold
					break;
				}
			case 0x03://foreground & background colour
				{
					if(!text.IsEmpty())
					{
						pUpdateChannel->m_editctrlLog.AppendColoredText(text, cr, bgcr, dwMask);//if any preceeding data write it
						text.Empty();//clear written text
					}
					index++;//get next char
					int iColour = (int)(line[index] - 0x30);//convert to a number
					if(iColour>=0 && iColour<10)//IS VALID. we include white the reset to default colour later to reset any colour in effect
					{
						if(iColour == 0x01 && line[index + 1]>=_T('0') && line[index + 1]<=_T('5'))//is there a second digit
						{
							index++;//get next char
							iColour = 0x0a + (int)(line[index] - 0x30);//make a two digit number
						} else if(iColour == 0x0 && line[index + 1]>=_T('0') && line[index + 1]<=_T('9'))//if first digit is zero and there is a second digit eg: 3 in 03
						{
							index++;//get next char
							iColour = (int)(line[index] - 0x30);//make a two digit number
						}

						if(iColour>=0 && iColour<16)
						{//If the first colour is not valid don't look for a second background colour!
							cr = _colours[iColour];//if the number is a valid colour index set new foreground colour
							index++;              //get next char
							if(line[index]==_T(',') && line[index + 1]>=_T('0') && line[index + 1]<=_T('9'))//is there a background colour
							{
								index++;//get next char
								int iColour = (int)(line[index] - 0x30);//convert to a number
								if(iColour == 0x01 && line[index + 1]>=_T('0') && line[index + 1]<=_T('5'))//is there a second digit
								{
									index++;//get next char
									iColour = 0x0a + (int)(line[index] - 0x30);//make a two digit number
								} else if(iColour == 0x0 && line[index + 1]>=_T('0') && line[index + 1]<=_T('9'))//if first digit is zero and there is a second digit eg: 3 in 03
								{
									index++;//get next char
									iColour = (int)(line[index] - 0x30);//make a two digit number
								}
								index++;//get next char
								if(iColour>=0 && iColour<16)
									bgcr = _colours[iColour];//if the number is a valid colour index set new foreground colour
							}
						}
					}//end of valid first colour scope
					break;
				}
			case 0x0F://attributes reset
				{
					if(!text.IsEmpty())
					{
						pUpdateChannel->m_editctrlLog.AppendColoredText(text, cr, bgcr, dwMask);//if any preceeding data write it
						text.Empty();//clear written text
					}
					index++;   //get next char
					dwMask = 0;//reset attributes mask
					cr   = foregroundColour;//reset foreground colour
					bgcr = backgroundColour;//reset background colour
					break;
				}
			case 0x16://Reverse (as per Mirc) toggle
				{         //NOTE:This does not reset the bold/underline,(dwMask = 0), attributes but does reset colours 'As per mIRC 6.16!!'
					if(!text.IsEmpty())
					{
						pUpdateChannel->m_editctrlLog.AppendColoredText(text, cr, bgcr, dwMask);//if any preceeding data write it
						text.Empty();//clear written text
					}
					index++;  //get next char
					if(cr != backgroundColour || bgcr != foregroundColour)
					{//set inverse
						cr   = backgroundColour;//foreground = background colour
						bgcr = foregroundColour;//background = foreground colour
					}
					else
					{
						cr   = foregroundColour;//reset foreground colour
						bgcr = backgroundColour;//reset background colour
					}
					//this tag used to represent italic?
					// if(dwMask & CFM_ITALIC) dwMask ^= CFM_ITALIC;//remove italic
					// else                    dwMask |= CFM_ITALIC;//add italic
					break;
				}
			case 0x1f://Underlined toggle
				{
					if(!text.IsEmpty())
					{
						pUpdateChannel->m_editctrlLog.AppendColoredText(text, cr, bgcr, dwMask);//if any preceeding data write it
						text.Empty();//clear written text
					}
					index++;  //get next char
					if(dwMask & CFM_UNDERLINE)
						dwMask ^= CFM_UNDERLINE;//remove underlined
					else
						dwMask |= CFM_UNDERLINE;//add underlined
					break;
				}
			default:
				{
					text += aChar;//add TCHAR to TCHAR array
					index++;      //get next char
				}
		}

	}
	if(text.GetLength()!=0)
		pUpdateChannel->m_editctrlLog.AppendColoredText(text, cr, bgcr, dwMask);//write the remainder if any
}

void CIrcWnd::SetConnectStatus( bool bFlag )
{
	if(bFlag)
	{
		GetDlgItem(IDC_BN_IRCCONNECT)->SetWindowText(GetResString(IDS_IRC_DISCONNECT));
		AddStatus( GetResString(IDS_CONNECTED));
		m_bConnected = true;
	}
	else
	{
		GetDlgItem(IDC_BN_IRCCONNECT)->SetWindowText(GetResString(IDS_IRC_CONNECT));
		AddStatus( GetResString(IDS_DISCONNECTED));
		m_bConnected = false;
		m_bLoggedIn = false;
		while( m_tabctrlChannelSelect.m_ptrlistChannel.GetCount() > 2 )
		{
			Channel* pToDel = (Channel*)(m_tabctrlChannelSelect.m_ptrlistChannel).GetTail();
			m_tabctrlChannelSelect.RemoveChannel( pToDel->m_sName );
		}
	}
}

void CIrcWnd::NoticeMessage( CString sSource, CString sTarget, CString sMessage )
{
	bool bFlag = false;
	if( m_tabctrlChannelSelect.FindChannelByName( sTarget ) )
	{
		AddInfoMessage( sTarget, _T("-%s:%s- %s"), sSource, sTarget, sMessage);
		bFlag = true;
	}
	else
	{
		for (POSITION pos1 = m_tabctrlChannelSelect.m_ptrlistChannel.GetHeadPosition(); pos1 != NULL;)
		{
			Channel* pCurrChannel = (Channel*)(m_tabctrlChannelSelect.m_ptrlistChannel).GetNext(pos1);
			if(pCurrChannel)
			{
				Nick* pCurrNick = m_listctrlNickList.FindNickByName(pCurrChannel->m_sName, sSource );
				if( pCurrNick)
				{
					AddInfoMessage( pCurrChannel->m_sName, _T("-%s:%s- %s"), sSource, sTarget, sMessage);
					bFlag = true;
				}
			}
		}
	}
	if( bFlag == false )
		AddStatus( _T("-%s:%s- %s"), sSource, sTarget, sMessage );
}

//We cannot support color within the text since HyperTextCtrl does not detect hyperlinks with color. So I will filter it.
CString CIrcWnd::StripMessageOfFontCodes( CString sTemp )
{
	sTemp = StripMessageOfColorCodes( sTemp );
	sTemp.Replace(_T("\002"),_T(""));//0x02 - BOLD
	sTemp.Replace(_T("\003"),_T(""));//0x03 - COLOUR
	sTemp.Replace(_T("\017"),_T(""));//0x0f - RESET
	sTemp.Replace(_T("\026"),_T(""));//0x16 - REVERSE/INVERSE was once italic?
	sTemp.Replace(_T("\037"),_T(""));//0x1f - UNDERLINE
	return sTemp;
}

CString CIrcWnd::StripMessageOfColorCodes( CString sTemp )
{
	if( !sTemp.IsEmpty() )
	{
		CString sTemp1, sTemp2;
		int iTest = sTemp.Find( 3 );
		if( iTest != -1 )
		{
			int iTestLength = sTemp.GetLength() - iTest;
			if( iTestLength < 2 )
				return sTemp;
			sTemp1 = sTemp.Left( iTest );
			sTemp2 = sTemp.Mid( iTest + 2);
			if( iTestLength < 4 )
				return sTemp1+sTemp2;
			if( sTemp2[0] == 44 && sTemp2.GetLength() > 2)
			{
				sTemp2 = sTemp2.Mid(2);
				for( int iIndex = 48; iIndex < 58; iIndex++ )
				{
					if( sTemp2[0] == iIndex )
						sTemp2 = sTemp2.Mid(1);
				}
			}
			else
			{
				for( int iIndex = 48; iIndex < 58; iIndex++ )
				{
					if( sTemp2[0] == iIndex )
					{
						sTemp2 = sTemp2.Mid(1);
						if( sTemp2[0] == 44 && sTemp2.GetLength() > 2)
						{
							sTemp2 = sTemp2.Mid(2);
							for( int iIndex = 48; iIndex < 58; iIndex++ )
							{
								if( sTemp2[0] == iIndex )
									sTemp2 = sTemp2.Mid(1);
							}
						}
					}
				}
			}
			sTemp = sTemp1 + sTemp2;
			sTemp = StripMessageOfColorCodes(sTemp);
		}
	}
	return sTemp;
}

void CIrcWnd::SetTitle( CString sChannel, CString sTitle )
{
	Channel* pCurrChannel = m_tabctrlChannelSelect.FindChannelByName(sChannel);
	if(!pCurrChannel)
		return;
	pCurrChannel->m_sTitle = StripMessageOfFontCodes(sTitle);
	if( pCurrChannel == m_tabctrlChannelSelect.m_pCurrentChannel )
	{
		pCurrChannel->m_sTitle.Replace(_T("\004"), _T("%"));
		m_editTitleWindow.SetWindowText( pCurrChannel->m_sTitle );
	}
}

void CIrcWnd::OnBnClickedChatsend()
{
	CString sSend;
	GetDlgItem(IDC_INPUTWINDOW)->GetWindowText(sSend);
	GetDlgItem(IDC_INPUTWINDOW)->SetWindowText(_T(""));
	GetDlgItem(IDC_INPUTWINDOW)->SetFocus();
	m_tabctrlChannelSelect.Chatsend(sSend);
}

void CIrcWnd::SendString( CString sSend )
{
	if( this->m_bConnected )
		m_pIrcMain->SendString( sSend );
}

BOOL CIrcWnd::OnHelpInfo(HELPINFO*)
{
	theApp.ShowHelp(eMule_FAQ_IRC_Chat);
	return TRUE;
}

void CIrcWnd::OnChatTextChange()
{
	GetDlgItem(IDC_CHATSEND)->EnableWindow( GetDlgItem(IDC_INPUTWINDOW)->GetWindowTextLength()>0 );
}

void CIrcWnd::ParseChangeMode( CString sChannel, CString sChanger, CString sCommands, CString sParams )
{
	CString sCommandsOrig = sCommands;
	CString sParamsOrig = sParams;
	try
	{
		if( sCommands.GetLength() >= 2 )
		{
			CString sDir;
			int iParamIndex = 0;
			while( !sCommands.IsEmpty() )
			{
				if( sCommands.Left(1) == _T("+") || sCommands.Left(1) == _T("-") )
				{
					sDir = sCommands.Left(1);
					sCommands = sCommands.Right(sCommands.GetLength()-1);
				}
				if( !sCommands.IsEmpty() && !sDir.IsEmpty() )
				{
					CString sCommand = sCommands.Left(1);
					sCommands = sCommands.Right(sCommands.GetLength()-1);

					if(m_listctrlNickList.m_sUserModeSettings.Find(sCommand) != -1 )
					{
						//This is a user mode change and must have a param!
						CString sParam = sParams.Tokenize(_T(" "), iParamIndex);
						m_listctrlNickList.ChangeNickMode( sChannel, sParam, sDir + sCommand);
					}
					if(m_tabctrlChannelSelect.m_sChannelModeSettingsTypeA.Find(sCommand) != -1)
					{
						//We do not use these messages yet.. But we can display them for the user to see
						//These modes always have a param and will add or remove a user from some type of list.
						CString sParam = sParams.Tokenize(_T(" "), iParamIndex);
						m_tabctrlChannelSelect.ChangeChanMode( sChannel, sParam, sDir, sCommand);
					}
					if(m_tabctrlChannelSelect.m_sChannelModeSettingsTypeB.Find(sCommand) != -1)
					{
						//We do not use these messages yet.. But we can display them for the user to see
						//These modes will always have a param..
						CString sParam = sParams.Tokenize(_T(" "), iParamIndex);
						m_tabctrlChannelSelect.ChangeChanMode( sChannel, sParams, sDir, sCommand);
					}
					if(m_tabctrlChannelSelect.m_sChannelModeSettingsTypeC.Find(sCommand) != -1 )
					{
						//We do not use these messages yet.. But we can display them for the user to see
						//These modes will only have a param if your setting it!
						CString sParam = _T("");
						if( sDir == _T("+") )
							sParam = sParams.Tokenize(_T(" "), iParamIndex);

						m_tabctrlChannelSelect.ChangeChanMode( sChannel, sParam, sDir, sCommand);
					}
					if(m_tabctrlChannelSelect.m_sChannelModeSettingsTypeD.Find(sCommand) != -1 )
					{
						//We do not use these messages yet.. But we can display them for the user to see
						//These modes will never have a param for it!
						CString sParam = _T("");
						m_tabctrlChannelSelect.ChangeChanMode( sChannel, sParam, sDir, sCommand);

					}
				}
			}
			if( !thePrefs.GetIrcIgnoreMiscMessage() )
				AddInfoMessage( sChannel, GetResString(IDS_IRC_SETSMODE), sChanger, sCommandsOrig, sParamsOrig);
		}
	}
	catch(...)
	{
		AddInfoMessage( sChannel, GetResString(IDS_IRC_NOTSUPPORTED));
		ASSERT(0);
	}
}

LRESULT CIrcWnd::OnCloseTab(WPARAM wparam, LPARAM)
{

	OnBnClickedClosechat( (int)wparam );

	return TRUE;
}

LRESULT CIrcWnd::OnQueryTab(WPARAM wParam, LPARAM)
{
	int iItem = (int)wParam;

	TCITEM item;
	item.mask = TCIF_PARAM;
	m_tabctrlChannelSelect.GetItem(iItem, &item);
	Channel* pPartChannel = (Channel*)item.lParam;
	if (pPartChannel)
	{
		if (pPartChannel->m_uType == 4 && m_bConnected)
		{
			return 0;
		}
		else if (pPartChannel->m_uType == 5 || pPartChannel->m_uType == 4)
		{
			return 0;
		}
	}
	return 1;
}
bool CIrcWnd::GetLoggedIn()
{
	return m_bLoggedIn;
}
void CIrcWnd::SetLoggedIn( bool bFlag )
{
	m_bLoggedIn = bFlag;
}
void CIrcWnd::SetSendFileString( CString sInFile )
{
	m_sSendString = sInFile;
}
CString CIrcWnd::GetSendFileString()
{
	return m_sSendString;
}
bool CIrcWnd::IsConnected()
{
	return m_bConnected;
}

void CIrcWnd::OnBnClickedColour()
{
	CRect rDraw;
	int iColor = 0;
	GetDlgItem(IDC_COLOUR)->GetWindowRect(rDraw);
	new CColourPopup(CPoint(rDraw.left+1, rDraw.bottom-89),	// Point to display popup
	                 iColor,	 				            // Selected colour
	                 this,									// parent
	                 GetResString(IDS_DEFAULT),				// "Default" text area
	                 NULL,                                  // Custom Text
	                 (COLORREF *)&_colours,                 // Pointer to a COLORREF array
	                 16);                                   // Size of the array

	CWnd *pParent = GetParent();
	if(pParent)
		pParent->SendMessage(UM_CPN_DROPDOWN, (LPARAM)iColor, (WPARAM) GetDlgCtrlID());

	return;
}

LONG CIrcWnd::OnSelEndOK(UINT lParam, LONG /*wParam*/)
{
	if(lParam != CLR_DEFAULT)
	{
		int iColour = 0;

		while(iColour<16 && (COLORREF)lParam!=_colours[iColour])
			iColour++;

		if(iColour>=0 && iColour<16)//iColour in valid range
		{
			CString sAddAttribute;
			int	iSelStart;
			int	iSelEnd;
			TCHAR iSelEnd3Char;
			TCHAR iSelEnd6Char;

			m_editInputWindow.GetSel(iSelStart, iSelEnd);//get selection area
			m_editInputWindow.GetWindowText(sAddAttribute);//get the whole line
			if(iSelEnd > iSelStart)
			{
				sAddAttribute.Insert(iSelEnd, _T('1'));//if a selection add default black colour tag
				sAddAttribute.Insert(iSelEnd, _T('0'));//add first half of colour tag
				sAddAttribute.Insert(iSelEnd, _T('\x03'));//if a selection add 'end' tag
			}
			iColour += 0x30;
			//a number greater than 9
			if(iColour>0x39)
			{
				sAddAttribute.Insert(iSelStart, (TCHAR)(iColour-10));//add second half of colour tag 1 for range 10 to 15
				sAddAttribute.Insert(iSelStart,  _T('1'));          //add first half of colour tag
			}
			else
			{
				sAddAttribute.Insert(iSelStart, (TCHAR)(iColour));//add second half of colour tag 1 for range 0 to 9
				sAddAttribute.Insert(iSelStart,  _T('0'));       //add first half of colour tag
			}
			//if this is the start of the line not a selection in the line and a colour has already just been set allow background to be set
			if(iSelEnd>2)
				iSelEnd3Char = sAddAttribute[iSelEnd-3];
			else
				iSelEnd3Char = _T(' ');
			if(iSelEnd>5)
				iSelEnd6Char = sAddAttribute[iSelEnd-6];
			else
				iSelEnd6Char = _T(' ');
			if(iSelEnd == iSelStart &&  iSelEnd3Char == _T('\x03') && iSelEnd6Char!= _T('\x03'))
				sAddAttribute.Insert(iSelStart, _T(','));//separator for background colour
			else
				sAddAttribute.Insert(iSelStart, _T('\x03'));//add start tag
			iSelStart+=3;//add 3 to start position
			iSelEnd+=3;//add 3 to end position
			m_editInputWindow.SetWindowText(sAddAttribute);//write new line to edit control
			m_editInputWindow.SetSel(iSelStart, iSelEnd);//update selection info
			m_editInputWindow.SetFocus();//set focus (from button) to edit control
		}
	}
	else
	{//Default button clicked set black
		CString sAddAttribute;
		int iSelStart;
		int iSelEnd;
		TCHAR iSelEnd3Char;
		TCHAR iSelEnd6Char;

		m_editInputWindow.GetSel(iSelStart, iSelEnd);//get selection area
		m_editInputWindow.GetWindowText(sAddAttribute);//get the whole line
		//if this is the start of the line not a selection in the line and a colour has already just been set allow background to be set
		if(iSelEnd>2)
			iSelEnd3Char = sAddAttribute[iSelEnd-3];
		else
			iSelEnd3Char = _T(' ');
		if(iSelEnd>5)
			iSelEnd6Char = sAddAttribute[iSelEnd-6];
		else
			iSelEnd6Char = _T(' ');
		if(iSelEnd == iSelStart &&  iSelEnd3Char == _T('\x03') && iSelEnd6Char!= _T('\x03'))
		{//Set DEFAULT white background
			sAddAttribute.Insert(iSelStart, _T('0'));//add second half of colour tag 0 for range 0 to 9
			sAddAttribute.Insert(iSelStart, _T('0'));//add first half of colour tag
			sAddAttribute.Insert(iSelStart, _T(','));//separator for background colour
		}
		else
		{//Set DEFAULT black foreground
			sAddAttribute.Insert(iSelStart, _T('1'));//add second half of colour tag 1 for range 0 to 9
			sAddAttribute.Insert(iSelStart, _T('0'));//add first half of colour tag
			sAddAttribute.Insert(iSelStart, _T('\x03'));//add start tag
		}
		iSelStart+=3;//add 2 to start position
		iSelEnd+=3;
		m_editInputWindow.SetWindowText(sAddAttribute);//write new line to edit control
		m_editInputWindow.SetSel(iSelStart, iSelEnd);//update selection info
		m_editInputWindow.SetFocus();//set focus (from button) to edit control
	}

	CWnd *pParent = GetParent();
	if(pParent)
	{
		pParent->SendMessage(UM_CPN_CLOSEUP, lParam, (WPARAM) GetDlgCtrlID());
		pParent->SendMessage(UM_CPN_SELENDOK, lParam, (WPARAM) GetDlgCtrlID());
	}

	return TRUE;
}

LONG CIrcWnd::OnSelEndCancel(UINT lParam, LONG /*wParam*/)
{
	CWnd *pParent = GetParent();
	if(pParent)
	{
		pParent->SendMessage(UM_CPN_CLOSEUP, lParam, (WPARAM) GetDlgCtrlID());
		pParent->SendMessage(UM_CPN_SELENDCANCEL, lParam, (WPARAM) GetDlgCtrlID());
	}
	return TRUE;
}

void CIrcWnd::OnBnClickedUnderline()
{
	CString sAddAttribute;
	int	iSelStart;
	int	iSelEnd;

	m_editInputWindow.GetSel(iSelStart, iSelEnd);//get selection area
	m_editInputWindow.GetWindowText(sAddAttribute);//get the whole line
	if(iSelEnd > iSelStart)
		sAddAttribute.Insert(iSelEnd, _T('\x1f'));//if a selection add end tag
	sAddAttribute.Insert(iSelStart, _T('\x1f'));//add start tag
	iSelStart++;//increment start position
	iSelEnd++;//increment end position
	m_editInputWindow.SetWindowText(sAddAttribute);//write new line to edit control
	m_editInputWindow.SetSel(iSelStart, iSelEnd);//update selection info
	m_editInputWindow.SetFocus();//set focus (from button) to edit control
}

void CIrcWnd::OnBnClickedBold()
{
	CString sAddAttribute;
	int	iSelStart;
	int	iSelEnd;

	m_editInputWindow.GetSel(iSelStart, iSelEnd);//get selection area
	m_editInputWindow.GetWindowText(sAddAttribute);//get the whole line
	if(iSelEnd > iSelStart)
		sAddAttribute.Insert(iSelEnd, _T('\x02'));//if a selection add end tag
	sAddAttribute.Insert(iSelStart, _T('\x02'));//add start tag
	iSelStart++;//increment start position
	iSelEnd++;//increment end position
	m_editInputWindow.SetWindowText(sAddAttribute);//write new line to edit control
	m_editInputWindow.SetSel(iSelStart, iSelEnd);//update selection info
	m_editInputWindow.SetFocus();//set focus (from button) to edit control
}

void CIrcWnd::OnBnClickedReset()
{
	CString sAddAttribute;
	int iSelStart;
	int	iSelEnd;

	m_editInputWindow.GetSel(iSelStart, iSelEnd);//get selection area
	if(!iSelStart)
		return;//reset is not a first character
	m_editInputWindow.GetWindowText(sAddAttribute);//get the whole line
	//Note the 'else' below! this tag resets all atttribute so only one tag needed at current position or end of selection
	if(iSelEnd > iSelStart)
		sAddAttribute.Insert(iSelEnd, _T('\x0f'));//if a selection add end tag
	else
		sAddAttribute.Insert(iSelStart, _T('\x0f'));//add start tag
	iSelStart++;//increment start position
	iSelEnd++;//increment end position
	m_editInputWindow.SetWindowText(sAddAttribute);//write new line to edit control
	m_editInputWindow.SetSel(iSelStart, iSelEnd);//update selection info
	m_editInputWindow.SetFocus();//set focus (from button) to edit control
}
#endif
