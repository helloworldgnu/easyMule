/* 
 * $Id: IrcChannelTabCtrl.cpp 7701 2008-10-15 07:34:41Z huby $
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

#include "StdAfx.h"
#define MMNODRV			// mmsystem: Installable driver support
//#define MMNOSOUND		// mmsystem: Sound support
#define MMNOWAVE		// mmsystem: Waveform support
#define MMNOMIDI		// mmsystem: MIDI support
#define MMNOAUX			// mmsystem: Auxiliary audio support
#define MMNOMIXER		// mmsystem: Mixer support
#define MMNOTIMER		// mmsystem: Timer support
#define MMNOJOY			// mmsystem: Joystick support
#define MMNOMCI			// mmsystem: MCI support
#define MMNOMMIO		// mmsystem: Multimedia file I/O support
#define MMNOMMSYSTEM	// mmsystem: General MMSYSTEM functions
#include <Mmsystem.h>
#include "./IrcChannelTabCtrl.h"
#include "./emule.h"
#include "./IrcWnd.h"
#include "./IrcMain.h"
#include "./otherfunctions.h"
#include "./MenuCmds.h"
#include "./HTRichEditCtrl.h"

#if _ENABLE_NOUSE

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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

IMPLEMENT_DYNAMIC(CIrcChannelTabCtrl, CClosableTabCtrl)

BEGIN_MESSAGE_MAP(CIrcChannelTabCtrl, CClosableTabCtrl)
ON_WM_CONTEXTMENU()
ON_NOTIFY_REFLECT(TCN_SELCHANGE, OnTcnSelchangeTab2)
END_MESSAGE_MAP()

CIrcChannelTabCtrl::CIrcChannelTabCtrl()
{
	m_pCurrentChannel = NULL;
	m_pParent = NULL;
	m_bCloseable = true;
	memset(&m_iiCloseButton, 0, sizeof m_iiCloseButton);
	m_ptCtxMenu.SetPoint(-1, -1);
}

CIrcChannelTabCtrl::~CIrcChannelTabCtrl()
{
	//Remove and delete all our open channels.
	DeleteAllChannel();
}

void CIrcChannelTabCtrl::Init()
{
	//This adds the two static windows, Status and ChanneList
	NewChannel( GetResString(IDS_STATUS), 1 );
	NewChannel( GetResString(IDS_IRC_CHANNELLIST), 2);
	//Initialize the IRC window to be in the ChannelList
	m_pCurrentChannel = (Channel*)m_ptrlistChannel.GetTail();
	SetCurSel(0);
	OnTcnSelchangeTab2( NULL, NULL );
	SetAllIcons();
}

void CIrcChannelTabCtrl::OnSysColorChange()
{
	CClosableTabCtrl::OnSysColorChange();
	SetAllIcons();
}

void CIrcChannelTabCtrl::SetAllIcons()
{
	CImageList imlist;
	imlist.Create(16, 16, theApp.m_iDfltImageListColorFlags | ILC_MASK, 0, 1);
	imlist.Add(CTempIconLoader(_T("Chat")));
	imlist.Add(CTempIconLoader(_T("Message")));
	imlist.Add(CTempIconLoader(_T("MessagePending")));
	SetImageList(&imlist);
	m_imlistIRC.DeleteImageList();
	m_imlistIRC.Attach(imlist.Detach());
	SetPadding(CSize(10, 3));
}

void CIrcChannelTabCtrl::OnContextMenu(CWnd*, CPoint point)
{
	int iCurTab = GetTabUnderMouse(point);
	if (iCurTab < 2)
	{
		return;
	}
	TCITEM item;
	item.mask = TCIF_PARAM;
	GetItem(iCurTab, &item);

	Channel* pChan = FindChannelByName(((Channel*)item.lParam)->m_sName);
	if( !pChan )
	{
		return;
	}

	CTitleMenu menuChat;
	menuChat.CreatePopupMenu();
	menuChat.AddMenuTitle(GetResString(IDS_IRC)+_T(" : ")+pChan->m_sName);
	menuChat.AppendMenu(MF_STRING, Irc_Close, GetResString(IDS_FD_CLOSE));
	if (iCurTab < 2) // no 'Close' for status log and channel list
		menuChat.EnableMenuItem(Irc_Close, MF_GRAYED);

	int iCurIndex = 0;
	int iLength = m_sChannelModeSettingsTypeA.GetLength();
	CString sMode;
	for( int iIndex = 0; iIndex < iLength; iIndex++)
	{
		sMode = m_sChannelModeSettingsTypeA.Mid(iIndex,1);
		if( pChan->m_sModesA.Find(sMode) == -1 )
		{
			menuChat.AppendMenu(MF_STRING, Irc_ChanCommands+iCurIndex, _T("ModeA Set +") + sMode + _T(" ( Not Supported Yet )") );
			menuChat.EnableMenuItem(Irc_ChanCommands+iCurIndex, MF_GRAYED);
		}
		else
		{
			menuChat.AppendMenu(MF_STRING, Irc_ChanCommands+iCurIndex+50, _T("ModeA Set -") + sMode + _T(" ( Not Supported Yet )") );
			menuChat.EnableMenuItem(Irc_ChanCommands+iCurIndex+50, MF_GRAYED);
		}
		iCurIndex++;
	}
	iLength = m_sChannelModeSettingsTypeB.GetLength();
	for( int iIndex = 0; iIndex < iLength; iIndex++)
	{
		sMode = m_sChannelModeSettingsTypeB.Mid(iIndex,1);
		if( pChan->m_sModesB.Find(sMode) == -1 )
		{
			menuChat.AppendMenu(MF_STRING, Irc_ChanCommands+iCurIndex, _T("ModeB Set +") + sMode + _T(" ( Not Supported Yet )") );
			menuChat.EnableMenuItem(Irc_ChanCommands+iCurIndex, MF_GRAYED);
		}
		else
		{
			menuChat.AppendMenu(MF_STRING, Irc_ChanCommands+iCurIndex+50, _T("ModeB Set -") + sMode + _T(" ( Not Supported Yet )") );
			menuChat.EnableMenuItem(Irc_ChanCommands+iCurIndex+50, MF_GRAYED);
		}
		iCurIndex++;
	}
	iLength = m_sChannelModeSettingsTypeC.GetLength();
	for( int iIndex = 0; iIndex < iLength; iIndex++)
	{
		sMode = m_sChannelModeSettingsTypeC.Mid(iIndex,1);
		if( pChan->m_sModesC.Find(sMode) == -1 )
		{
			menuChat.AppendMenu(MF_STRING, Irc_ChanCommands+iCurIndex, _T("ModeC Set +") + sMode + _T(" ( Not Supported Yet )") );
			menuChat.EnableMenuItem(Irc_ChanCommands+iCurIndex, MF_GRAYED);
		}
		else
		{
			menuChat.AppendMenu(MF_STRING, Irc_ChanCommands+iCurIndex+50, _T("ModeC Set -") + sMode + _T(" ( Remove ") + sMode + _T(" )") );
		}
		iCurIndex++;
	}
	iLength = m_sChannelModeSettingsTypeD.GetLength();
	for( int iIndex = 0; iIndex < iLength; iIndex++)
	{
		sMode = m_sChannelModeSettingsTypeD.Mid(iIndex,1);
		if( pChan->m_sModesD.Find(sMode) == -1 )
		{
			menuChat.AppendMenu(MF_STRING, Irc_ChanCommands+iCurIndex, _T("ModeD Set +") + sMode + _T(" ( Add ") + sMode + _T(" )") );
		}
		else
		{
			menuChat.AppendMenu(MF_STRING, Irc_ChanCommands+iCurIndex+50, _T("ModeD Set -") + sMode + _T(" ( Remove ") + sMode + _T(" )") );
		}
		iCurIndex++;
	}

	menuChat.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
	VERIFY( menuChat.DestroyMenu() );
}

int CIrcChannelTabCtrl::GetTabUnderMouse(CPoint point)
{
	TCHITTESTINFO hitinfo;
	CRect rect;
	GetWindowRect(&rect);
	point.Offset(0-rect.left,0-rect.top);
	hitinfo.pt = point;

	if( GetItemRect( 0, &rect ) )
		if (hitinfo.pt.y< rect.top+30 && hitinfo.pt.y >rect.top-30)
			hitinfo.pt.y = rect.top;

	// Find the destination tab...
	unsigned int uTab = HitTest( &hitinfo );
	if( hitinfo.flags != TCHT_NOWHERE )
		return uTab;
	else
		return -1;
}

Channel* CIrcChannelTabCtrl::FindChannelByName(CString sName)
{
	POSITION pos1, pos2;
	for (pos1 = m_ptrlistChannel.GetHeadPosition();( pos2 = pos1 ) != NULL;)
	{
		m_ptrlistChannel.GetNext(pos1);
		Channel* pCurChannel = (Channel*)m_ptrlistChannel.GetAt(pos2);
		if (pCurChannel->m_sName.CompareNoCase(sName.Trim()) == 0 && (pCurChannel->m_uType == 4 || pCurChannel->m_uType == 5))
			return pCurChannel;
	}
	return 0;
}

Channel* CIrcChannelTabCtrl::NewChannel( CString sName, uint8 uType )
{
	Channel* pToAdd = new Channel;
	pToAdd->m_sName = sName;
	pToAdd->m_sTitle = sName;
	pToAdd->m_uType = uType;
	pToAdd->m_iHistoryPos = 0;
	if (uType != 2)
	{
		CRect rcChannel;
		m_pParent->m_listctrlServerChannelList.GetWindowRect(&rcChannel);
		pToAdd->m_editctrlLog.Create(WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VSCROLL | ES_MULTILINE | ES_READONLY, rcChannel, m_pParent, (UINT)-1);
		pToAdd->m_editctrlLog.ModifyStyleEx(0, WS_EX_STATICEDGE, SWP_FRAMECHANGED);
		pToAdd->m_editctrlLog.SendMessage(EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELONG(3, 3));
		pToAdd->m_editctrlLog.SetEventMask(pToAdd->m_editctrlLog.GetEventMask() | ENM_LINK);
		pToAdd->m_editctrlLog.SetFont(&theApp.m_fontHyperText);
		pToAdd->m_editctrlLog.SetTitle(sName);
		pToAdd->m_editctrlLog.SetProfileSkinKey(_T("IRCChannel"));
		pToAdd->m_editctrlLog.ApplySkin();
	}
	m_ptrlistChannel.AddTail(pToAdd);

	TCITEM newitem;
	newitem.mask = TCIF_PARAM|TCIF_TEXT|TCIF_IMAGE;

	newitem.lParam = (LPARAM)pToAdd;
	newitem.pszText = const_cast<LPTSTR>((LPCTSTR)sName);
	newitem.iImage = 1; // 'Message'
	uint32 uPos = GetItemCount();
	InsertItem(uPos,&newitem);
	if(uType == 4)
	{
		SetCurSel(uPos);
		SetCurFocus(uPos);
		OnTcnSelchangeTab2( NULL, NULL );
	}
	return pToAdd;
}

void CIrcChannelTabCtrl::RemoveChannel( CString sChannel )
{
	Channel* pToDel = FindChannelByName( sChannel );
	if( !pToDel )
		return;
	TCITEM item;
	item.mask = TCIF_PARAM;
	item.lParam = -1;
	int iIndex;
	for(iIndex = 0; iIndex < GetItemCount();iIndex++)
	{
		GetItem(iIndex,&item);
		if (((Channel*)item.lParam) == pToDel)
			break;
	}
	if (((Channel*)item.lParam) != pToDel)
		return;
	DeleteItem(iIndex);

	if( pToDel == m_pCurrentChannel )
	{
		m_pParent->m_listctrlNickList.DeleteAllItems();
		if( GetItemCount() > 2 && iIndex > 1 )
		{
			if ( iIndex == 2 )
				iIndex++;
			SetCurSel(iIndex-1);
			SetCurFocus(iIndex-1);
			OnTcnSelchangeTab2( NULL, NULL );
		}
		else
		{
			SetCurSel(0);
			SetCurFocus(0);
			OnTcnSelchangeTab2( NULL, NULL );
		}
	}
	m_pParent->m_listctrlNickList.DeleteAllNick(pToDel->m_sName);
	m_ptrlistChannel.RemoveAt(m_ptrlistChannel.Find(pToDel));
	delete pToDel;
}

void CIrcChannelTabCtrl::DeleteAllChannel()
{
	POSITION pos1, pos2;
	Channel* pCurChannel = NULL;
	for (pos1 = m_ptrlistChannel.GetHeadPosition();( pos2 = pos1 ) != NULL;)
	{
		m_ptrlistChannel.GetNext(pos1);
		pCurChannel =	(Channel*)m_ptrlistChannel.GetAt(pos2);
		m_pParent->m_listctrlNickList.DeleteAllNick(pCurChannel->m_sName);
		m_ptrlistChannel.RemoveAt(pos2);
		delete pCurChannel;
	}
}


bool CIrcChannelTabCtrl::ChangeChanMode( CString sChannel, CString sParam, CString sDir, CString sCommand )
{
	//Must have a Command.
	if( sCommand.IsEmpty() )
		return false;

	//Must be a channel!
	if( sChannel.IsEmpty() || sChannel.Left(1) != _T("#") )
		return false;

	//Get Channel.
	Channel* pUpdate = FindChannelByName( sChannel );
	//Make sure we have this channel in our list.
	if( !pUpdate )
		return false;

	//Update modes.
	int iCommandIndex = m_sChannelModeSettingsTypeA.Find(sCommand);
	if( iCommandIndex != -1 )
	{
		//Remove the setting. This takes care of "-" and makes sure we don't add the same symbol twice.
		pUpdate->m_sModesA.Replace(sCommand, _T(""));
		if( sDir == _T("+") )
		{
			//Update mode.
			if( pUpdate->m_sModesA.IsEmpty() )
				pUpdate->m_sModesA = sCommand;
			else
			{
				//The chan does have other modes.. Lets make sure we put things in order..
				int iChannelModeSettingsTypeALength = m_sChannelModeSettingsTypeA.GetLength();
				//This will pad the mode string..
				for( int iIndex = 0; iIndex < iChannelModeSettingsTypeALength; iIndex++)
				{
					if( pUpdate->m_sModesA.Find(m_sChannelModeSettingsTypeA[iIndex]) == -1 )
						pUpdate->m_sModesA.Insert(iIndex, _T(" "));
				}
				//Insert the new mode
				pUpdate->m_sModesA.Insert(iCommandIndex, sCommand);
				//Remove pads
				pUpdate->m_sModesA.Remove(_T(' '));
			}
		}
		return true;
	}
	iCommandIndex = m_sChannelModeSettingsTypeB.Find(sCommand);
	if( iCommandIndex != -1 )
	{
		//Remove the setting. This takes care of "-" and makes sure we don't add the same symbol twice.
		pUpdate->m_sModesB.Replace(sCommand, _T(""));
		if( sDir == _T("+") )
		{
			//Update mode.
			if( pUpdate->m_sModesB.IsEmpty() )
				pUpdate->m_sModesB = sCommand;
			else
			{
				//The chan does have other modes.. Lets make sure we put things in order..
				int iChannelModeSettingsTypeBLength = m_sChannelModeSettingsTypeB.GetLength();
				//This will pad the mode string..
				for( int iIndex = 0; iIndex < iChannelModeSettingsTypeBLength; iIndex++)
				{
					if( pUpdate->m_sModesB.Find(m_sChannelModeSettingsTypeB[iIndex]) == -1 )
						pUpdate->m_sModesB.Insert(iIndex, _T(" "));
				}
				//Insert the new mode
				pUpdate->m_sModesB.Insert(iCommandIndex, sCommand);
				//Remove pads
				pUpdate->m_sModesB.Remove(_T(' '));
			}
		}
		return true;
	}
	iCommandIndex = m_sChannelModeSettingsTypeC.Find(sCommand);
	if( iCommandIndex != -1 )
	{
		//Remove the setting. This takes care of "-" and makes sure we don't add the same symbol twice.
		pUpdate->m_sModesC.Replace(sCommand, _T(""));
		if( sDir == _T("+") )
		{
			//Update mode.
			if( pUpdate->m_sModesC.IsEmpty() )
				pUpdate->m_sModesC = sCommand;
			else
			{
				//The chan does have other modes.. Lets make sure we put things in order..
				int iChannelModeSettingsTypeCLength = m_sChannelModeSettingsTypeC.GetLength();
				//This will pad the mode string..
				for( int iIndex = 0; iIndex < iChannelModeSettingsTypeCLength; iIndex++)
				{
					if( pUpdate->m_sModesC.Find(m_sChannelModeSettingsTypeC[iIndex]) == -1 )
						pUpdate->m_sModesC.Insert(iIndex, _T(" "));
				}
				//Insert the new mode
				pUpdate->m_sModesC.Insert(iCommandIndex, sCommand);
				//Remove pads
				pUpdate->m_sModesC.Remove(_T(' '));
			}
		}
		return true;
	}
	iCommandIndex = m_sChannelModeSettingsTypeD.Find(sCommand);
	if( iCommandIndex != -1 )
	{
		//Remove the setting. This takes care of "-" and makes sure we don't add the same symbol twice.
		pUpdate->m_sModesD.Replace(sCommand, _T(""));
		if( sDir == _T("+") )
		{
			//Update mode.
			if( pUpdate->m_sModesD.IsEmpty() )
				pUpdate->m_sModesD = sCommand;
			else
			{
				//The chan does have other modes.. Lets make sure we put things in order..
				int iChannelModeSettingsTypeDLength = m_sChannelModeSettingsTypeD.GetLength();
				//This will pad the mode string..
				for( int iIndex = 0; iIndex < iChannelModeSettingsTypeDLength; iIndex++)
				{
					if( pUpdate->m_sModesD.Find(m_sChannelModeSettingsTypeD[iIndex]) == -1 )
						pUpdate->m_sModesD.Insert(iIndex, _T(" "));
				}
				//Insert the new mode
				pUpdate->m_sModesD.Insert(iCommandIndex, sCommand);
				//Remove pads
				pUpdate->m_sModesD.Remove(_T(' '));
			}
		}
		return true;
	}
	return false;
}

void CIrcChannelTabCtrl::OnTcnSelchangeTab2(NMHDR*, LRESULT* pResult)
{
	TCITEM item;
	item.mask = TCIF_PARAM;
	//What channel did we select?
	int iCurSell = GetCurSel();
	if (iCurSell == -1)
	{
		//No channel, abort..
		return;
	}
	if (!GetItem(iCurSell, &item))
	{
		//We had no valid item here.. Something isn't right..
		//TODO: this should never happen, so maybe we should remove this tab?
		return;
	}
	Channel* pUpdate = (Channel*)item.lParam;
	Channel* pCh2 = NULL;

	//Set our current channel to the new one for quick reference..
	m_pCurrentChannel = pUpdate;

	if (m_pCurrentChannel->m_uType == 1)
		m_pParent->m_editTitleWindow.SetWindowText(GetResString(IDS_STATUS));
	if (m_pCurrentChannel->m_uType == 2)
	{
		//Since some channels can have a LOT of nicks, hide the window then remove them to speed it up..
		m_pParent->m_listctrlNickList.ShowWindow(SW_HIDE);
		m_pParent->m_listctrlNickList.DeleteAllItems();
		m_pParent->m_listctrlNickList.UpdateNickCount();
		m_pParent->m_listctrlNickList.ShowWindow(SW_SHOW);
		//Set title to ChanList
		m_pParent->m_editTitleWindow.SetWindowText(GetResString(IDS_IRC_CHANNELLIST));
		//Show our ChanList..
		m_pParent->m_listctrlServerChannelList.ShowWindow(SW_SHOW);
		TCITEM tci;
		tci.mask = TCIF_PARAM;
		//Go through the channel tabs and hide the channels..
		//Maybe overkill? Maybe just remember our previous channel and hide it?
		int iIndex = 0;
		while (GetItem(iIndex++, &tci))
		{
			pCh2 = (Channel*)tci.lParam;
			if (pCh2 != m_pCurrentChannel && pCh2->m_editctrlLog.m_hWnd != NULL)
				pCh2->m_editctrlLog.ShowWindow(SW_HIDE);
		}
		return;
	}
	//We entered the channel, set activity flag off.
	SetActivity( m_pCurrentChannel->m_sName, false );
	CRect rcChannel;
	m_pParent->m_listctrlServerChannelList.GetWindowRect(&rcChannel);
	m_pParent->ScreenToClient(&rcChannel);
	//Show new current channel..
	m_pCurrentChannel->m_editctrlLog.SetWindowPos(NULL, rcChannel.left, rcChannel.top, rcChannel.Width(), rcChannel.Height(), SWP_NOZORDER);
	m_pCurrentChannel->m_editctrlLog.ShowWindow(SW_SHOW);
	TCITEM tci;
	tci.mask = TCIF_PARAM;
	//Hide all channels not in focus..
	//Maybe overkill? Maybe remember previous channel and hide?
	int iIndex = 0;
	while (GetItem(iIndex++, &tci))
	{
		pCh2 = (Channel*)tci.lParam;
		if (pCh2 != m_pCurrentChannel && pCh2->m_editctrlLog.m_hWnd != NULL)
			pCh2->m_editctrlLog.ShowWindow(SW_HIDE);
	}
	//Make sure channelList is hidden.
	m_pParent->m_listctrlServerChannelList.ShowWindow(SW_HIDE);
	//Update nicklist to the new channel..
	m_pParent->m_listctrlNickList.RefreshNickList( pUpdate->m_sName );
	//Set title to the new channel..
	m_pParent->SetTitle( pUpdate->m_sName, pUpdate->m_sTitle );
	//Push focus back to the input box..
	m_pParent->GetDlgItem(IDC_INPUTWINDOW)->SetFocus();
	if( pResult )
		*pResult = 0;
}

void CIrcChannelTabCtrl::ScrollHistory(bool bDown)
{
	CString sBuffer;

	if ( (m_pCurrentChannel->m_iHistoryPos==0 && !bDown) || (m_pCurrentChannel->m_iHistoryPos==m_pCurrentChannel->m_sarrayHistory.GetCount() && bDown))
		return;

	if (bDown)
		++m_pCurrentChannel->m_iHistoryPos;
	else
		--m_pCurrentChannel->m_iHistoryPos;

	sBuffer= (m_pCurrentChannel->m_iHistoryPos==m_pCurrentChannel->m_sarrayHistory.GetCount()) ? _T("") : m_pCurrentChannel->m_sarrayHistory.GetAt(m_pCurrentChannel->m_iHistoryPos);

	m_pParent->GetDlgItem(IDC_INPUTWINDOW)->SetWindowText(sBuffer);
	m_pParent->m_editInputWindow.SetSel(sBuffer.GetLength(),sBuffer.GetLength());
}

void CIrcChannelTabCtrl::SetActivity( CString sChannel, bool bFlag)
{
	Channel* pRefresh = FindChannelByName( sChannel );
	if( !pRefresh )
	{
		pRefresh = (Channel*)m_ptrlistChannel.GetHead();
		if( !pRefresh )
			return;
	}
	TCITEM item;
	item.mask = TCIF_PARAM;
	item.lParam = -1;
	int iIndex;
	for (iIndex = 0; iIndex < GetItemCount();iIndex++)
	{
		GetItem(iIndex,&item);
		if (((Channel*)item.lParam) == pRefresh)
			break;
	}
	if (((Channel*)item.lParam) != pRefresh)
		return;
	if( bFlag )
	{
		item.mask = TCIF_IMAGE;
		item.iImage = 2; // 'MessagePending'
		SetItem( iIndex, &item );
		HighlightItem(iIndex, TRUE);
	}
	else
	{
		item.mask = TCIF_IMAGE;
		item.iImage = 1; // 'Message'
		SetItem( iIndex, &item );
		HighlightItem(iIndex, FALSE);
	}
}


void CIrcChannelTabCtrl::Chatsend( CString sSend )
{
	if ((UINT)m_pCurrentChannel->m_sarrayHistory.GetCount() == thePrefs.GetMaxChatHistoryLines())
		m_pCurrentChannel->m_sarrayHistory.RemoveAt(0);
	m_pCurrentChannel->m_sarrayHistory.Add(sSend);
	m_pCurrentChannel->m_iHistoryPos = m_pCurrentChannel->m_sarrayHistory.GetCount();

	if( sSend.IsEmpty() )
		return;

	if( !m_pParent->IsConnected() )
		return;
	if( sSend.Left(4) == _T("/hop") )
	{
		if( m_pCurrentChannel->m_sName.Left(1) == _T("#") )
		{
			CString sChannel = m_pCurrentChannel->m_sName;
			m_pParent->m_pIrcMain->SendString( _T("PART ") + sChannel );
			m_pParent->m_pIrcMain->SendString( _T("JOIN ") + sChannel );
			return;
		}
		return;
	}
	if( sSend.Left(1) == _T("/") && sSend.Left(3).MakeLower() != _T("/me") && sSend.Left(6).MakeLower() != _T("/sound") )
	{
		if (sSend.Left(4) == _T("/msg"))
		{
			if( m_pCurrentChannel->m_uType == 4 || m_pCurrentChannel->m_uType == 5)
			{
				sSend.Replace( _T("%"), _T("\004") );
				m_pParent->AddInfoMessage( m_pCurrentChannel->m_sName ,CString(_T("* >> "))+sSend.Mid(5));
				sSend.Replace( _T("\004"), _T("%") );
			}
			else
			{
				sSend.Replace( _T("%"), _T("\004") );
				m_pParent->AddStatus( CString(_T("* >> "))+sSend.Mid(5));
				sSend.Replace( _T("\004"), _T("%") );
			}
			sSend = CString(_T("/PRIVMSG")) + sSend.Mid(4);
		}
		if( ((CString)sSend.Left(17)).CompareNoCase( _T("/PRIVMSG nickserv")  )== 0)
		{
			sSend = CString(_T("/ns")) + sSend.Mid(17);
		}
		else if( ((CString)sSend.Left(17)).CompareNoCase( _T("/PRIVMSG chanserv") )== 0)
		{
			sSend = CString(_T("/cs")) + sSend.Mid(17);
		}
		else if( ((CString)sSend.Left(8)).CompareNoCase( _T("/PRIVMSG") )== 0)
		{
			int iIndex = sSend.Find(_T(" "), sSend.Find(_T(" "))+1);
			sSend.Insert(iIndex+1, _T(":"));
		}
		else if( ((CString)sSend.Left(6)).CompareNoCase( _T("/TOPIC") )== 0)
		{
			int iIndex = sSend.Find(_T(" "), sSend.Find(_T(" "))+1);
			sSend.Insert(iIndex+1, _T(":"));
		}
		m_pParent->m_pIrcMain->SendString(sSend.Mid(1));
		return;
	}
	if( m_pCurrentChannel->m_uType < 4 )
	{
		m_pParent->m_pIrcMain->SendString(sSend);
		return;
	}
	CString sBuild;
	if( sSend.Left(3) == _T("/me") )
	{
		sBuild.Format( _T("PRIVMSG %s :\001ACTION %s\001"), m_pCurrentChannel->m_sName, sSend.Mid(4) );
		sSend.Replace( _T("%"), _T("\004") );
		m_pParent->AddInfoMessage( m_pCurrentChannel->m_sName, _T("* %s %s"), m_pParent->m_pIrcMain->GetNick(), sSend.Mid(4));
		m_pParent->m_pIrcMain->SendString(sBuild);
		return;
	}
	if( sSend.Left(6) == _T("/sound") )
	{
		CString sound;
		sBuild.Format( _T("PRIVMSG %s :\001SOUND %s\001"), m_pCurrentChannel->m_sName, sSend.Mid(7) );
		m_pParent->m_pIrcMain->SendString(sBuild);
		sSend = sSend.Mid(7);
		int soundlen = sSend.Find( _T(" ") );
		sSend.Replace( _T("%"), _T("\004") );
		if( soundlen != -1 )
		{
			sBuild = sSend.Left(soundlen);
			sBuild.Replace(_T("\\"),_T(""));
			sSend = sSend.Left(soundlen);
		}
		else
		{
			sBuild = sSend;
			sSend = _T("[SOUND]");
		}
		sound.Format(_T("%sSounds\\IRC\\%s"), thePrefs.GetMuleDirectory(EMULE_EXECUTEABLEDIR), sBuild);
		m_pParent->AddInfoMessage( m_pCurrentChannel->m_sName, _T("* %s %s"), m_pParent->m_pIrcMain->GetNick(), sSend);
		PlaySound(sound, NULL, SND_FILENAME | SND_NOSTOP | SND_NOWAIT | SND_ASYNC);
		return;
	}
	sBuild = _T("PRIVMSG ") + m_pCurrentChannel->m_sName + _T(" :") + sSend;
	m_pParent->m_pIrcMain->SendString(sBuild);
	sSend.Replace( _T("%"), _T("\004") );
	m_pParent->AddMessage( m_pCurrentChannel->m_sName, m_pParent->m_pIrcMain->GetNick(), sSend );
	sSend.Replace( _T("\004"), _T("%") );
}

void CIrcChannelTabCtrl::Localize()
{
	for (int iIndex = 0; iIndex < GetItemCount();iIndex++)
	{
		TCITEM item;
		item.mask = TCIF_PARAM;
		item.lParam = -1;
		GetItem(iIndex,&item);
		Channel* pCurChan = (Channel*)item.lParam;
		if (pCurChan != NULL)
		{
			if( pCurChan->m_uType == 1 )
			{
				pCurChan->m_sTitle = GetResString(IDS_STATUS);
				item.mask = TCIF_TEXT;
				item.pszText = const_cast<LPTSTR>((LPCTSTR)pCurChan->m_sTitle);
				SetItem(iIndex,&item);
				pCurChan->m_sTitle.ReleaseBuffer();
			}
			if( pCurChan->m_uType == 2 )
			{
				pCurChan->m_sTitle = GetResString(IDS_IRC_CHANNELLIST);
				item.mask = TCIF_TEXT;
				item.pszText = const_cast<LPTSTR>((LPCTSTR)pCurChan->m_sTitle);
				SetItem(iIndex,&item);
				pCurChan->m_sTitle.ReleaseBuffer();
			}
		}
	}
	if (m_pCurrentChannel)
	{
		if( m_pCurrentChannel->m_uType == 1 )
			m_pParent->m_editTitleWindow.SetWindowText(GetResString(IDS_STATUS));
		if( m_pCurrentChannel->m_uType == 2 )
			m_pParent->m_editTitleWindow.SetWindowText(GetResString(IDS_IRC_CHANNELLIST));
	}
	SetAllIcons();
}

BOOL CIrcChannelTabCtrl::OnCommand(WPARAM wParam, LPARAM)
{
	int iChanItem = m_pParent->m_tabctrlChannelSelect.GetCurSel();
	switch( wParam )
	{
		case Irc_Close:
			{
				//Pressed the close button
				m_pParent->OnBnClickedClosechat();
				return true;
			}
	}
	uint32 uCommand = Irc_ChanCommands;
	if( wParam >= uCommand && wParam < uCommand+50)
	{
		CString sMode, sSend;
		int iIndex = wParam - uCommand;
		if( iIndex < m_sChannelModeSettingsTypeA.GetLength() )
		{
			sMode = m_sChannelModeSettingsTypeA.Mid(iIndex,1);
			TCITEM item;
			item.mask = TCIF_PARAM;
			GetItem(iChanItem,&item);
			Channel* pChan = (Channel*)item.lParam;
			if( pChan )
			{
				//We have a chan, send the command.
				sSend.Format( _T("MODE %s +%s"), pChan->m_sName, sMode );
				m_pParent->m_pIrcMain->SendString(sSend);
			}
			return true;
		}
		if( iIndex < m_sChannelModeSettingsTypeA.GetLength()+m_sChannelModeSettingsTypeB.GetLength() )
		{
			sMode = m_sChannelModeSettingsTypeB.Mid(iIndex-m_sChannelModeSettingsTypeA.GetLength(),1);
			TCITEM item;
			item.mask = TCIF_PARAM;
			GetItem(iChanItem,&item);
			Channel* pChan = (Channel*)item.lParam;
			if( pChan )
			{
				//We have a chan, send the command.
				sSend.Format( _T("MODE %s +%s"), pChan->m_sName, sMode );
				m_pParent->m_pIrcMain->SendString(sSend);
			}
			return true;
		}
		if( iIndex < m_sChannelModeSettingsTypeA.GetLength()+m_sChannelModeSettingsTypeB.GetLength()+m_sChannelModeSettingsTypeC.GetLength() )
		{
			sMode = m_sChannelModeSettingsTypeC.Mid(iIndex-m_sChannelModeSettingsTypeA.GetLength()-m_sChannelModeSettingsTypeB.GetLength(),1);
			TCITEM item;
			item.mask = TCIF_PARAM;
			GetItem(iChanItem,&item);
			Channel* pChan = (Channel*)item.lParam;
			if( pChan )
			{
				//We have a chan, send the command.
				sSend.Format( _T("MODE %s +%s"), pChan->m_sName, sMode );
				m_pParent->m_pIrcMain->SendString(sSend);
			}
			return true;
		}
		if( iIndex < m_sChannelModeSettingsTypeA.GetLength()+m_sChannelModeSettingsTypeB.GetLength()+m_sChannelModeSettingsTypeC.GetLength()+m_sChannelModeSettingsTypeD.GetLength() )
		{
			sMode = m_sChannelModeSettingsTypeD.Mid(iIndex-m_sChannelModeSettingsTypeA.GetLength()-m_sChannelModeSettingsTypeB.GetLength()-m_sChannelModeSettingsTypeC.GetLength(),1);
			TCITEM item;
			item.mask = TCIF_PARAM;
			GetItem(iChanItem,&item);
			Channel* pChan = (Channel*)item.lParam;
			if( pChan )
			{
				//We have a chan, send the command.
				sSend.Format( _T("MODE %s +%s"), pChan->m_sName, sMode );
				m_pParent->m_pIrcMain->SendString(sSend);
			}
			return true;
		}
		return true;
	}
	if( wParam >= uCommand+50 && wParam < uCommand+100)
	{
		int iIndex = wParam - uCommand - 50;
		CString sMode, sSend;
		if( iIndex < m_sChannelModeSettingsTypeA.GetLength() )
		{
			sMode = m_sChannelModeSettingsTypeA.Mid(iIndex,1);
			TCITEM item;
			item.mask = TCIF_PARAM;
			GetItem(iChanItem,&item);
			Channel* pChan = (Channel*)item.lParam;
			if( pChan )
			{
				//We have a chan, send the command.
				sSend.Format( _T("MODE %s -%s"), pChan->m_sName, sMode );
				m_pParent->m_pIrcMain->SendString(sSend);
			}
			return true;
		}
		if( iIndex < m_sChannelModeSettingsTypeA.GetLength()+m_sChannelModeSettingsTypeB.GetLength() )
		{
			sMode = m_sChannelModeSettingsTypeB.Mid(iIndex-m_sChannelModeSettingsTypeA.GetLength(),1);
			TCITEM item;
			item.mask = TCIF_PARAM;
			GetItem(iChanItem,&item);
			Channel* pChan = (Channel*)item.lParam;
			if( pChan )
			{
				//We have a chan, send the command.
				sSend.Format( _T("MODE %s -%s"), pChan->m_sName, sMode );
				m_pParent->m_pIrcMain->SendString(sSend);
			}
			return true;
		}
		if( iIndex < m_sChannelModeSettingsTypeA.GetLength()+m_sChannelModeSettingsTypeB.GetLength()+m_sChannelModeSettingsTypeC.GetLength() )
		{
			sMode = m_sChannelModeSettingsTypeC.Mid(iIndex-m_sChannelModeSettingsTypeA.GetLength()-m_sChannelModeSettingsTypeB.GetLength(),1);
			TCITEM item;
			item.mask = TCIF_PARAM;
			GetItem(iChanItem,&item);
			Channel* pChan = (Channel*)item.lParam;
			if( pChan )
			{
				//We have a chan, send the command.
				sSend.Format( _T("MODE %s -%s"), pChan->m_sName, sMode );
				m_pParent->m_pIrcMain->SendString(sSend);
			}
			return true;
		}
		if( iIndex < m_sChannelModeSettingsTypeA.GetLength()+m_sChannelModeSettingsTypeB.GetLength()+m_sChannelModeSettingsTypeC.GetLength()+m_sChannelModeSettingsTypeD.GetLength() )
		{
			sMode = m_sChannelModeSettingsTypeD.Mid(iIndex-m_sChannelModeSettingsTypeA.GetLength()-m_sChannelModeSettingsTypeB.GetLength()-m_sChannelModeSettingsTypeC.GetLength(),1);
			TCITEM item;
			item.mask = TCIF_PARAM;
			GetItem(iChanItem,&item);
			Channel* pChan = (Channel*)item.lParam;
			if( pChan )
			{
				//We have a chan, send the command.
				sSend.Format( _T("MODE %s -%s"), pChan->m_sName, sMode );
				m_pParent->m_pIrcMain->SendString(sSend);
			}
			return true;
		}
		return true;
	}
	return true;
}
#endif
