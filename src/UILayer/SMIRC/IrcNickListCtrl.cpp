/* 
 * $Id: IrcNickListCtrl.cpp 7701 2008-10-15 07:34:41Z huby $
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
#include "./IrcNickListCtrl.h"
#include "./otherfunctions.h"
#include "./IrcWnd.h"
#include "./IrcMain.h"
#include "./MenuCmds.h"
#include "./HTRichEditCtrl.h"

#if _ENABLE_NOUSE

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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

IMPLEMENT_DYNAMIC(CIrcNickListCtrl, CMuleListCtrl)

BEGIN_MESSAGE_MAP(CIrcNickListCtrl, CMuleListCtrl)
ON_WM_CONTEXTMENU()
ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnLvnColumnclick)
ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclk)
END_MESSAGE_MAP()

CIrcNickListCtrl::CIrcNickListCtrl()
{
	SetName(_T("IrcNickListCtrl"));
	m_pParent = NULL;
}

void CIrcNickListCtrl::Init()
{
	InsertColumn(0,GetResString(IDS_IRC_NICK),LVCFMT_LEFT,90);
	InsertColumn(1,GetResString(IDS_STATUS),LVCFMT_LEFT,70);
	LoadSettings();
	SetSortArrow();
	SortItems(&SortProc, GetSortItem() + ( (GetSortAscending()) ? 0:10) );
}

int CIrcNickListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	Nick* pItem1 = (Nick*)lParam1;
	Nick* pItem2 = (Nick*)lParam2;
	switch(lParamSort)
	{
			//This will sort the list like MIRC
		case 0:
			return pItem2->m_sNick.CompareNoCase(pItem1->m_sNick);
		case 1:
			{
				//TODO - MUST FIX THIS NOW THAT MODES ARE DONE DIFFERENT>
				if( pItem1->m_iLevel == pItem2->m_iLevel )
					return pItem2->m_sNick.CompareNoCase(pItem1->m_sNick);
				if( pItem1->m_iLevel == -1 )
					return 1;
				if( pItem2->m_iLevel == -1 )
					return -1;
				return pItem1->m_iLevel - pItem2->m_iLevel;
			}
		case 11:
			{
				//TODO - MUST FIX THIS NOW THAT MODES ARE DONE DIFFERENT>
				if( pItem1->m_iLevel == pItem2->m_iLevel )
					return pItem1->m_sNick.CompareNoCase(pItem2->m_sNick);
				if( pItem1->m_iLevel == -1 )
					return 1;
				if( pItem2->m_iLevel == -1 )
					return -1;
				return pItem1->m_iLevel - pItem2->m_iLevel;
			}
		case 10:
			//This will put them in alpha order..
			return pItem1->m_sNick.CompareNoCase(pItem2->m_sNick);
		default:
			return 0;
	}
}

void CIrcNickListCtrl::OnLvnColumnclick(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	if (m_iSortIndex != pNMLV->iSubItem)
		m_bSortOrder = 1;
	else
		m_bSortOrder = !m_bSortOrder;
	m_iSortIndex = pNMLV->iSubItem;

	SetSortArrow(m_iSortIndex, m_bSortOrder);
	SortItems(&SortProc, m_iSortIndex + (m_bSortOrder ? 0 : 10));

	*pResult = 0;
}

void CIrcNickListCtrl::OnContextMenu(CWnd*, CPoint point)
{
	int iCurSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iCurSel == -1)
	{
		return;
	}
	Nick* pNick = (Nick*)GetItemData(iCurSel);
	if( !pNick )
	{
		return;
	}

	CTitleMenu menuNick;
	menuNick.CreatePopupMenu();
	menuNick.AddMenuTitle(GetResString(IDS_IRC_NICK) + _T(" : ") + pNick->m_sNick);
	menuNick.AppendMenu(MF_STRING, Irc_Priv, GetResString(IDS_IRC_PRIVMESSAGE));
	menuNick.AppendMenu(MF_STRING, Irc_AddFriend, GetResString(IDS_IRC_ADDTOFRIENDLIST));
	if (!m_pParent->GetSendFileString().IsEmpty())
		menuNick.AppendMenu(MF_STRING, Irc_SendLink, GetResString(IDS_IRC_SENDLINK) + m_pParent->GetSendFileString());
	else
		menuNick.AppendMenu(MF_STRING, Irc_SendLink, GetResString(IDS_IRC_SENDLINK) + GetResString(IDS_IRC_NOSFS));
	menuNick.AppendMenu(MF_STRING, Irc_Kick, GetResString(IDS_IRC_KICK));
	menuNick.AppendMenu(MF_STRING, Irc_Ban, _T("Ban"));
	//Ban currently uses chanserv to ban which seems to kick also.. May change this later..
	//	menuNick.AppendMenu(MF_STRING, Irc_KB, _T("Kick/Ban"));
	menuNick.AppendMenu(MF_STRING, Irc_Slap, GetResString(IDS_IRC_SLAP));
	int iLength = m_sUserModeSettings.GetLength();
	if( iLength > 0 )
	{
		CString sMode, sModeSymbol;
		for( int iIndex = 0; iIndex < iLength; iIndex++)
		{
			sMode = m_sUserModeSettings.Mid(iIndex,1);
			sModeSymbol = m_sUserModeSymbols.Mid(iIndex,1);
			if( pNick->m_sModes.Find(sModeSymbol[0]) == -1 )
				menuNick.AppendMenu(MF_STRING, Irc_OpCommands+iIndex, _T("Set +") + sMode + _T(" ( Add ") + sModeSymbol + _T(" )") );
			else
				menuNick.AppendMenu(MF_STRING, Irc_OpCommands+iIndex+25, _T("Set -") + sMode + _T(" ( Remove ") + sModeSymbol + _T(" )") );
		}
	}
	GetPopupMenuPos(*this, point);
	menuNick.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
	VERIFY( menuNick.DestroyMenu() );
}

void CIrcNickListCtrl::OnNMDblclk(NMHDR*, LRESULT* pResult)
{
	//We double clicked a nick.. Try to open private channel
	int iNickItem = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iNickItem != -1)
	{
		Nick* pNick = (Nick*)GetItemData(iNickItem);
		if (pNick)
		{
			//Valid nick, send a info message to force open a new channel..
			m_pParent->AddInfoMessage(pNick->m_sNick, GetResString(IDS_IRC_PRIVATECHANSTART));
		}
	}
	*pResult = 0;
}

Nick* CIrcNickListCtrl::FindNickByName(CString sChannel, CString sName)
{
	//Find a nick in a specific channel..
	Channel* pCurrChannel = m_pParent->m_tabctrlChannelSelect.FindChannelByName(sChannel);
	if( !pCurrChannel)
	{
		//Channel does not exist.. Abort.
		return NULL;
	}
	//We have a channel, find nick..
	POSITION pos1, pos2;
	Nick* pCurrNick = NULL;
	for (pos1 = pCurrChannel->m_ptrlistNicks.GetHeadPosition();( pos2 = pos1 ) != NULL;)
	{
		pCurrChannel->m_ptrlistNicks.GetNext(pos1);
		pCurrNick = (Nick*)pCurrChannel->m_ptrlistNicks.GetAt(pos2);
		if (pCurrNick->m_sNick == sName)
		{
			//We found our nick, return it..
			return pCurrNick;
		}
	}
	//Nick was not in channel..
	return NULL;
}

Nick* CIrcNickListCtrl::NewNick( CString sChannel, CString sNick )
{
	//Add a new nick to a sChannel..
	Channel* pToAddChan = m_pParent->m_tabctrlChannelSelect.FindChannelByName( sChannel );
	if( !pToAddChan )
	{
		//Channel wasn't found, abort..
		return NULL;
	}
	//This is a little clumsy and makes you think the previous check wasn't needed,
	//But we need the channel object and FindNickByName doesn't do it..
	//TODO: Maybe create a special method to merge the two checks..
	if( FindNickByName( sChannel, sNick ) )
	{
		//Check if we already have this nick..
		return NULL;
	}
	Nick* pToAddNick = new Nick;

	//Remove all modes from the front of this nick
	while( m_sUserModeSymbols.Find(sNick.Left(1)) != -1 )
	{
		pToAddNick->m_sModes += sNick.Left(1);
		sNick = sNick.Mid(1);
	}

	//We now know the true nick
	pToAddNick->m_sNick = sNick;

	//Set user level
	if( pToAddNick->m_sModes.GetLength() > 0 )
		pToAddNick->m_iLevel = m_sUserModeSymbols.Find(pToAddNick->m_sModes[0]);
	else
		pToAddNick->m_iLevel = -1;

	//Add new nick to channel.
	pToAddChan->m_ptrlistNicks.AddTail(pToAddNick);
	if( pToAddChan == m_pParent->m_tabctrlChannelSelect.m_pCurrentChannel )
	{
		//This is our current channel, add it to our nicklist..
		int iItemr = GetItemCount();
		iItemr = InsertItem(LVIF_PARAM,iItemr,0,0,0,0,(LPARAM)pToAddNick);
		SetItemText(iItemr,0,(LPCTSTR)pToAddNick->m_sNick);
		SetItemText(iItemr,1,(LPCTSTR)pToAddNick->m_sModes);
		UpdateNickCount();
	}
	return pToAddNick;
}

void CIrcNickListCtrl::RefreshNickList( CString sChannel )
{
	//Hide nickList to speed things up..
	ShowWindow(SW_HIDE);
	DeleteAllItems();
	Channel* refresh = m_pParent->m_tabctrlChannelSelect.FindChannelByName( sChannel );
	if( !refresh )
	{
		//This is not a channel??? shouldn't happen..
		UpdateNickCount();
		ShowWindow(SW_SHOW);
		return;
	}
	POSITION pos1, pos2;
	Nick* pCurrNick = NULL;
	int iItemr = -1;
	for (pos1 = refresh->m_ptrlistNicks.GetHeadPosition();( pos2 = pos1 ) != NULL;)
	{
		//Add all nicks to list..
		refresh->m_ptrlistNicks.GetNext(pos1);
		pCurrNick = (Nick*)refresh->m_ptrlistNicks.GetAt(pos2);
		iItemr = GetItemCount();
		iItemr = InsertItem(LVIF_PARAM,iItemr,0,0,0,0,(LPARAM)pCurrNick);
		SetItemText(iItemr,0,(LPCTSTR)pCurrNick->m_sNick);
		SetItemText(iItemr,1,(LPCTSTR)pCurrNick->m_sModes);
	}
	UpdateNickCount();
	ShowWindow(SW_SHOW);
}

bool CIrcNickListCtrl::RemoveNick( CString sChannel, CString sNick )
{
	//Remove nick from a channel..
	Channel* pUpdateChannel = m_pParent->m_tabctrlChannelSelect.FindChannelByName( sChannel );
	if( !pUpdateChannel )
	{
		//There was no channel..
		return false;
	}
	POSITION pos1, pos2;
	Nick* pCurrNick = NULL;
	for( pos1 = pUpdateChannel->m_ptrlistNicks.GetHeadPosition();(pos2=pos1)!=NULL;)
	{
		//Go through nicks
		pUpdateChannel->m_ptrlistNicks.GetNext(pos1);
		pCurrNick = (Nick*)pUpdateChannel->m_ptrlistNicks.GetAt(pos2);
		if( pCurrNick->m_sNick == sNick )
		{
			//Found nick..
			if( pUpdateChannel == m_pParent->m_tabctrlChannelSelect.m_pCurrentChannel )
			{
				//If it's our current channel, delete the nick from nickList
				LVFINDINFO find;
				find.flags = LVFI_PARAM;
				find.lParam = (LPARAM)pCurrNick;
				DeleteItem(FindItem(&find));
				UpdateNickCount();
			}
			//remove nick and delete.
			pUpdateChannel->m_ptrlistNicks.RemoveAt(pos2);
			delete pCurrNick;
			return true;
		}
	}
	return false;
}

void CIrcNickListCtrl::DeleteAllNick( CString sChannel )
{
	//Remove all nicks from a channel..
	Channel* pCurrChannel = m_pParent->m_tabctrlChannelSelect.FindChannelByName(sChannel);
	if( !pCurrChannel )
	{
		//Channel was not found?
		return;
	}
	POSITION pos1, pos2;
	Nick* pCurrNick = NULL;
	for(pos1 = pCurrChannel->m_ptrlistNicks.GetHeadPosition();( pos2 = pos1) != NULL;)
	{
		//Remove all nicks..
		pCurrChannel->m_ptrlistNicks.GetNext(pos1);
		pCurrNick = (Nick*)pCurrChannel->m_ptrlistNicks.GetAt(pos2);
		pCurrChannel->m_ptrlistNicks.RemoveAt(pos2);
		delete pCurrNick;
	}
}

void CIrcNickListCtrl::DeleteNickInAll( CString sNick, CString sMessage )
{
	//Remove a nick in all Channels.
	//This is a client that Quit the network, so we have a quit message..
	POSITION pos1, pos2;
	Channel* pCurrChannel = NULL;
	for (pos1 = m_pParent->m_tabctrlChannelSelect.m_ptrlistChannel.GetHeadPosition();( pos2 = pos1 ) != NULL;)
	{
		//Go through all channels..
		m_pParent->m_tabctrlChannelSelect.m_ptrlistChannel.GetNext(pos1);
		pCurrChannel = (Channel*)(m_pParent->m_tabctrlChannelSelect.m_ptrlistChannel).GetAt(pos2);
		if(RemoveNick( pCurrChannel->m_sName, sNick ))
		{
			//If nick was in channel, put message in it saying why user quit..
			if( !thePrefs.GetIrcIgnoreQuitMessage() )
				m_pParent->AddInfoMessage( pCurrChannel->m_sName, GetResString(IDS_IRC_HASQUIT), sNick, sMessage);
		}
	}
}

bool CIrcNickListCtrl::ChangeNick( CString sChannel, CString sOldNick, CString sNewNick )
{
	//Someone changed there nick..
	Channel* pUpdateChannel = m_pParent->m_tabctrlChannelSelect.FindChannelByName( sChannel );
	if( !pUpdateChannel )
	{
		//Didn't find a channel??
		return false;
	}
	POSITION pos1, pos2;
	Nick* pCurrNick = NULL;
	for( pos1 = pUpdateChannel->m_ptrlistNicks.GetHeadPosition();(pos2=pos1)!=NULL;)
	{
		//Go through channel nicks.
		pUpdateChannel->m_ptrlistNicks.GetNext(pos1);
		pCurrNick = (Nick*)pUpdateChannel->m_ptrlistNicks.GetAt(pos2);
		if( pCurrNick->m_sNick == sOldNick )
		{
			//Found nick to change.
			if((pUpdateChannel = m_pParent->m_tabctrlChannelSelect.m_pCurrentChannel) != NULL)
			{
				//This channle is in focuse, update nick in nickList
				LVFINDINFO find;
				find.flags = LVFI_PARAM;
				find.lParam = (LPARAM)pCurrNick;
				int iItemr = FindItem(&find);
				if (iItemr != -1)
					SetItemText(iItemr,0,(LPCTSTR)sNewNick);
			}
			//Set new nick name..
			pCurrNick->m_sNick = sNewNick;
			return true;
		}
	}
	return false;
}

bool CIrcNickListCtrl::ChangeNickMode( CString sChannel, CString sNick, CString sMode )
{
	if( sChannel.Left(1) != "#" )
	{
		//Not a channel, this shouldn't happen
		return true;
	}
	if( sNick == "" )
	{
		//No name, this shouldn't happen..
		return true;
	}
	//We are changing a mode to something..
	Channel* pUpdateChannel = m_pParent->m_tabctrlChannelSelect.FindChannelByName( sChannel );
	if( !pUpdateChannel )
	{
		//No channel found, this shouldn't happen.
		return false;
	}
	POSITION pos1, pos2;
	Nick* pCurrNick = NULL;
	CString sModeSymbol;
	int iIndex, iModeLevel, iItemr;
	for( pos1 = pUpdateChannel->m_ptrlistNicks.GetHeadPosition();(pos2=pos1)!=NULL;)
	{
		//Go through nicks
		pUpdateChannel->m_ptrlistNicks.GetNext(pos1);
		pCurrNick = (Nick*)pUpdateChannel->m_ptrlistNicks.GetAt(pos2);
		if( pCurrNick->m_sNick == sNick )
		{
			//Found nick.
			//Update modes.
			iModeLevel = m_sUserModeSettings.Find(sMode[1]);
			if( iModeLevel != -1 )
			{
				sModeSymbol = m_sUserModeSymbols.Mid(iModeLevel,1);
				//Remove the symbol. This takes care of "-" and makes sure we don't add the same symbol twice.
				pCurrNick->m_sModes.Remove(sModeSymbol[0]);
				if( sMode.Left(1) == "+" )
				{
					//The nick doesn't have any other modes.. Just set it..
					if( pCurrNick->m_sModes == "" )
						pCurrNick->m_sModes = sModeSymbol;
					else
					{
						//The nick does have other modes.. Lets make sure we put things in order..
						iIndex = 0;
						//This will pad the mode string..
						while( iIndex < m_sUserModeSymbols.GetLength() )
						{
							if( pCurrNick->m_sModes.Find(m_sUserModeSymbols[iIndex]) == -1 )
							{
								pCurrNick->m_sModes.Insert(iIndex, _T(" "));
							}
							iIndex++;
						}
						//Insert the new mode
						pCurrNick->m_sModes.Insert(iModeLevel, sModeSymbol[0]);
						//Remove pads
						pCurrNick->m_sModes.Remove(' ');
					}
				}
			}
			else
			{
				//This should never happen
				pCurrNick->m_sModes = "";
				pCurrNick->m_iLevel = -1;
				ASSERT(0);
			}

			//Update user level
			if( pCurrNick->m_sModes.GetLength() > 0 )
				pCurrNick->m_iLevel = m_sUserModeSymbols.Find(pCurrNick->m_sModes[0]);
			else
				pCurrNick->m_iLevel = -1;

			iItemr = -1;
			if( (pUpdateChannel = m_pParent->m_tabctrlChannelSelect.m_pCurrentChannel) != NULL )
			{
				//Channel was in focus, update the nickList.
				LVFINDINFO find;
				find.flags = LVFI_PARAM;
				find.lParam = (LPARAM)pCurrNick;
				iItemr = FindItem(&find);
				if (iItemr != -1)
					SetItemText(iItemr,1,(LPCTSTR)pCurrNick->m_sModes);
			}
			return true;
		}
	}
	//Nick was not found in list??
	return false;
}

void CIrcNickListCtrl::ChangeAllNick( CString sOldNick, CString sNewNick )
{
	//Change a nick in ALL the channels..
	Channel* pCurrChannel = m_pParent->m_tabctrlChannelSelect.FindChannelByName( sOldNick );
	if( pCurrChannel )
	{
		//We had a private room open with this nick.. Update the title of the channel!
		pCurrChannel->m_sName = sNewNick;
		TCITEM item;
		item.mask = TCIF_PARAM;
		item.lParam = -1;
		//Find channel tab..
		int iIndex;
		for (iIndex = 0; iIndex < m_pParent->m_tabctrlChannelSelect.GetItemCount();iIndex++)
		{
			m_pParent->m_tabctrlChannelSelect.GetItem(iIndex,&item);
			if (((Channel*)item.lParam) == pCurrChannel)
			{
				//Found tab, update it..
				item.mask = TCIF_TEXT;
				item.pszText = const_cast<LPTSTR>((LPCTSTR)sNewNick);
				m_pParent->m_tabctrlChannelSelect.SetItem( iIndex, &item);
				break;
			}
		}
	}
	//Go through all other channel nicklists..
	POSITION pos1, pos2;
	for (pos1 = m_pParent->m_tabctrlChannelSelect.m_ptrlistChannel.GetHeadPosition();( pos2 = pos1 ) != NULL;)
	{
		m_pParent->m_tabctrlChannelSelect.m_ptrlistChannel.GetNext(pos1);
		pCurrChannel = (Channel*)(m_pParent->m_tabctrlChannelSelect.m_ptrlistChannel).GetAt(pos2);
		if(ChangeNick( pCurrChannel->m_sName, sOldNick, sNewNick ))
		{
			//Nick change successful, add a message to inform you..
			if( !thePrefs.GetIrcIgnoreMiscMessage() )
				m_pParent->AddInfoMessage( pCurrChannel->m_sName, GetResString(IDS_IRC_NOWKNOWNAS), sOldNick, sNewNick);
		}
	}
}

void CIrcNickListCtrl::UpdateNickCount()
{
	//Get the header control
	CHeaderCtrl* pHeaderCtrl;
	HDITEM hdi;
	hdi.mask = HDI_TEXT;
	CString sResource;
	pHeaderCtrl = GetHeaderCtrl();
	if( GetItemCount() )
	{
		//Set nick count to current channel.
		sResource.Format(_T("%s[%i]"), GetResString(IDS_IRC_NICK), GetItemCount());
	}
	else
	{
		//No nicks in the list.. Your in Status or ChannleList
		sResource.Format(_T("%s"), GetResString(IDS_IRC_NICK));
	}
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)sResource);
	pHeaderCtrl->SetItem(0, &hdi);
}

void CIrcNickListCtrl::Localize()
{
	CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
	CString sResource;
	sResource = GetResString(IDS_STATUS);
	HDITEM hdi;
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)sResource);
	hdi.mask = HDI_TEXT;
	pHeaderCtrl->SetItem(1, &hdi);
	UpdateNickCount();
}

BOOL CIrcNickListCtrl::OnCommand(WPARAM wParam, LPARAM)
{
	int iNickItem = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	int iChanItem = m_pParent->m_tabctrlChannelSelect.GetCurSel();
	Nick* pNick = (Nick*)GetItemData(iNickItem);
	TCITEM item;
	item.mask = TCIF_PARAM;
	m_pParent->m_tabctrlChannelSelect.GetItem(iChanItem,&item);
	Channel* pChannel = (Channel*)item.lParam;

	switch( wParam )
	{
		case Irc_Priv:
			{
				//Right clicked and choose start private chan.
				if(pNick)
				{
					//Send a message with the nick as the channel name, this will create a new window with the default message.
					m_pParent->AddInfoMessage( pNick->m_sNick, GetResString(IDS_IRC_PRIVATECHANSTART));
				}
				return true;
			}
		case Irc_Kick:
			{
				//Kick someone from a channel
				if( pNick && pChannel )
				{
					//We have a nick and chan, send the command.
					CString sSend;
					sSend.Format(_T("KICK %s %s"), pChannel->m_sName, pNick->m_sNick );
					m_pParent->m_pIrcMain->SendString(sSend);
				}
				return true;
			}
		case Irc_Ban:
			{
				//Kick someone from a channel
				if( pNick && pChannel )
				{
					//We have a nick and chan, send the command.
					CString sSend;
					sSend.Format(_T("cs ban %s %s"), pChannel->m_sName, pNick->m_sNick );
					m_pParent->m_pIrcMain->SendString(sSend);
				}
				return true;
			}
		case Irc_Slap:
			{
				//Do a silly slap on someone
				if( pNick && pChannel )
				{
					//We have a nick and chan, send the command.
					CString sSend;
					sSend.Format( GetResString(IDS_IRC_SLAPMSGSEND), pChannel->m_sName, pNick->m_sNick );
					m_pParent->AddInfoMessage( pChannel->m_sName, GetResString(IDS_IRC_SLAPMSG), m_pParent->m_pIrcMain->GetNick(), pNick->m_sNick);
					m_pParent->m_pIrcMain->SendString(sSend);
				}
				return true;
			}
		case Irc_AddFriend:
			{
				//Attempt to add this person as a friend.
				if( pNick && pChannel )
				{
					//We have a nick and chan, send the command.
					//SetVerify() sets a new challenge which is required by the other end to respond with for some protection.
					CString sSend;
					sSend.Format(_T("PRIVMSG %s :\001RQSFRIEND|%i|\001"), pNick->m_sNick, m_pParent->m_pIrcMain->SetVerify() );
					m_pParent->m_pIrcMain->SendString(sSend);
				}
				return true;
			}
		case Irc_SendLink:
			{
				//Send a ED2K link to someone..
				if(!m_pParent->GetSendFileString())
				{
					//There is no link in the buffer, abort.
					return true;
				}
				if( pNick && pChannel )
				{
					//We have a nick and chan, send the command.
					//We send our nick and ClientID to allow the other end to only accept links from friends..
					CString sSend;
					sSend.Format(_T("PRIVMSG %s :\001SENDLINK|%s|%s\001"), pNick->m_sNick, md4str(thePrefs.GetUserHash()), m_pParent->GetSendFileString() );
					m_pParent->m_pIrcMain->SendString(sSend);
				}
				return true;
			}
	}
	if( wParam >= Irc_OpCommands && wParam < Irc_OpCommands+25)
	{
		int iIndex = wParam - Irc_OpCommands;
		CString sMode = m_sUserModeSettings.Mid(iIndex,1);
		if( pNick && pChannel )
		{
			//We have a nick and chan, send the command.
			CString sSend;
			sSend.Format(_T("MODE %s +%s %s"), pChannel->m_sName, sMode, pNick->m_sNick );
			m_pParent->m_pIrcMain->SendString(sSend);
		}
		return true;
	}
	if( wParam >= Irc_OpCommands+25 && wParam < Irc_OpCommands+50)
	{
		int iIndex = wParam - Irc_OpCommands-25;
		CString sMode = m_sUserModeSettings.Mid(iIndex,1);
		if( pNick && pChannel )
		{
			//We have a nick and chan, send the command.
			CString sSend;
			sSend.Format(_T("MODE %s -%s %s"), pChannel->m_sName, sMode, pNick->m_sNick );
			m_pParent->m_pIrcMain->SendString(sSend);
		}
		return true;
	}
	return true;
}
#endif
