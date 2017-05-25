/* 
 * $Id: ChatSelector.cpp 7701 2008-10-15 07:34:41Z huby $
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
#include "emule.h"
#include "ChatSelector.h"
#include "packets.h"
#include "HTRichEditCtrl.h"
#include "emuledlg.h"
#include "Statistics.h"
#include "OtherFunctions.h"
#include "UpDownClient.h"
#include "Preferences.h"
#include "TaskbarNotifier.h"
#include "ListenSocket.h"
#include "ChatWnd.h"
#include "SafeFile.h"
#include "Log.h"
#include "MenuCmds.h"
#include "ClientDetailDialog.h"
#include "FriendList.h"
#include "ClientList.h"

#if _ENABLE_NOUSE

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define URLINDICATOR	_T("http:|www.|.de |.net |.com |.org |.to |.tk |.cc |.fr |ftp:|ed2k:|https:|ftp.|.info|.biz|.uk|.eu|.es|.tv|.cn|.tw|.ws|.nu|.jp")


///////////////////////////////////////////////////////////////////////////////
// CChatItem

CChatItem::CChatItem()
{
	client = NULL;
	log = NULL;
	notify = false;
	history_pos = 0;
}

CChatItem::~CChatItem()
{
	delete log;
}

///////////////////////////////////////////////////////////////////////////////
// CChatSelector

IMPLEMENT_DYNAMIC(CChatSelector, CClosableTabCtrl)

BEGIN_MESSAGE_MAP(CChatSelector, CClosableTabCtrl)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_SYSCOLORCHANGE()
	ON_NOTIFY_REFLECT(TCN_SELCHANGE, OnTcnSelchangeChatsel)
	ON_BN_CLICKED(IDC_CCLOSE, OnBnClickedCclose)
	ON_BN_CLICKED(IDC_CSEND, OnBnClickedCsend)
	//ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

CChatSelector::CChatSelector()
{
	m_hwndCloseBtn = NULL;
	m_hwndMessageBox = NULL;
	m_hwndSendBtn = NULL;
	m_lastemptyicon = false;
	m_blinkstate = false;
	m_Timer = 0;
	m_bCloseable = true;
}

CChatSelector::~CChatSelector()
{
}

void CChatSelector::Init()
{
	m_hwndCloseBtn = GetParent()->GetDlgItem(IDC_CCLOSE)->m_hWnd;
	::SetParent(m_hwndCloseBtn, m_hWnd);

	m_hwndSendBtn = GetParent()->GetDlgItem(IDC_CSEND)->m_hWnd;
	::SetParent(m_hwndSendBtn, m_hWnd);

	m_hwndMessageBox = GetParent()->GetDlgItem(IDC_CMESSAGE)->m_hWnd;
	::SetParent(m_hwndMessageBox, m_hWnd);

	ModifyStyle(0, WS_CLIPCHILDREN);

	SetAllIcons();

	VERIFY( (m_Timer = SetTimer(20, 1500, 0)) != NULL );
}

void CChatSelector::OnSysColorChange()
{
	CClosableTabCtrl::OnSysColorChange();
	SetAllIcons();
}

void CChatSelector::SetAllIcons()
{
	CImageList iml;
	iml.Create(16, 16, theApp.m_iDfltImageListColorFlags | ILC_MASK, 0, 1);
	iml.Add(CTempIconLoader(_T("Chat")));
	iml.Add(CTempIconLoader(_T("Message")));
	iml.Add(CTempIconLoader(_T("MessagePending")));
	SetImageList(&iml);
	m_imlChat.DeleteImageList();
	m_imlChat.Attach(iml.Detach());
	SetPadding(CSize(10, 0));
}

void CChatSelector::UpdateFonts(CFont* pFont)
{
	TCITEM item;
	item.mask = TCIF_PARAM;
	int i = 0;
	while (GetItem(i++, &item)){
		CChatItem* ci = (CChatItem*)item.lParam;
		ci->log->SetFont(pFont);
	}
}

CChatItem* CChatSelector::StartSession(CUpDownClient* client, bool show)
{
	if (show)
		::SetFocus(m_hwndMessageBox);
	if (GetTabByClient(client) != -1){
		if (show){
			SetCurSel(GetTabByClient(client));
			ShowChat();
		}
		return NULL;
	}

	CChatItem* chatitem = new CChatItem();
	chatitem->client = client;
	chatitem->log = new CHTRichEditCtrl;

	CRect rcChat;
	GetChatSize(rcChat);
	if (GetItemCount() == 0)
		rcChat.top += 20;
	chatitem->log->Create(WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VSCROLL | ES_MULTILINE | ES_READONLY, rcChat, this, (UINT)-1);
	chatitem->log->ModifyStyleEx(0, WS_EX_STATICEDGE, SWP_FRAMECHANGED);
	chatitem->log->SendMessage(EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELONG(3, 3));
	chatitem->log->SetEventMask(chatitem->log->GetEventMask() | ENM_LINK);
	chatitem->log->SetFont(&theApp.m_fontHyperText);
	chatitem->log->SetProfileSkinKey(_T("Chat"));
	chatitem->log->ApplySkin();

	CTime theTime = CTime::GetCurrentTime();
	CString sessions = GetResString(IDS_CHAT_START) + client->GetUserName() + CString(_T(" - ")) + theTime.Format(_T("%c")) + _T("\n");
	chatitem->log->AppendKeyWord(sessions, RGB(255,0,0));
	client->SetChatState(MS_CHATTING);

	CString name;
	if (client->GetUserName() != NULL)
		name = client->GetUserName();
	else
		name.Format(_T("(%s)"), GetResString(IDS_UNKNOWN));
	chatitem->log->SetTitle(name);

	TCITEM newitem;
	newitem.mask = TCIF_PARAM | TCIF_TEXT | TCIF_IMAGE;
	newitem.lParam = (LPARAM)chatitem;
	newitem.pszText = const_cast<LPTSTR>((LPCTSTR)name);
	newitem.iImage = 0;
	int iItemNr = InsertItem(GetItemCount(), &newitem);
	if (show || IsWindowVisible()){
		SetCurSel(iItemNr);
		ShowChat();
	}
	return chatitem;
}

int CChatSelector::GetTabByClient(CUpDownClient* client)
{
	for (int i = 0; i < GetItemCount(); i++){
		TCITEM cur_item;
		cur_item.mask = TCIF_PARAM;
		if (GetItem(i, &cur_item) && ((CChatItem*)cur_item.lParam)->client == client)
			return i;
	}
	return -1;
}

CChatItem* CChatSelector::GetItemByClient(CUpDownClient* client)
{
	for (int i = 0; i < GetItemCount(); i++){
		TCITEM cur_item;
		cur_item.mask = TCIF_PARAM;
		if (GetItem(i, &cur_item) && ((CChatItem*)cur_item.lParam)->client == client)
			return (CChatItem*)cur_item.lParam;
	}
	return NULL;
}

void CChatSelector::ProcessMessage(CUpDownClient* sender, const CString& message)
{
	sender->IncMessagesReceived();
	CChatItem* ci = GetItemByClient(sender);

	CString strMessage(message);
	strMessage.MakeLower();
	CString resToken;
	int curPos = 0;
	resToken = thePrefs.GetMessageFilter().Tokenize(_T("|"), curPos);
	while (!resToken.IsEmpty())
	{
		resToken.Trim();
		if (strMessage.Find(resToken.MakeLower()) > -1){
			if ( thePrefs.IsAdvSpamfilterEnabled() && !sender->IsFriend() && sender->GetMessagesSent() == 0 ){
				sender->SetSpammer(true);
				if (ci)
					EndSession(sender);
			}
			return;
		}
		resToken = thePrefs.GetMessageFilter().Tokenize(_T("|"), curPos);
	}

	// advanced spamfilter check
	if (IsSpam(strMessage, sender))
	{
		if (!sender->IsSpammer()){
			if (thePrefs.GetVerbose())
				AddDebugLogLine(false, _T("'%s' has been marked as spammer"), sender->GetUserName());
		}
		sender->SetSpammer(true);
		if (ci)
			EndSession(sender);
		return;
	}

	AddLogLine(true, GetResString(IDS_NEWMSG), sender->GetUserName(), ipstr(sender->GetConnectIP()));
	
	bool isNewChatWindow = false;
	if (!ci)
	{
		if ((UINT)GetItemCount() >= thePrefs.GetMsgSessionsMax())
			return;
		ci = StartSession(sender, false);
		isNewChatWindow = true; 
	}
	if (thePrefs.GetIRCAddTimestamp())
		AddTimeStamp(ci);
	ci->log->AppendKeyWord(sender->GetUserName(), RGB(50,200,250));
	ci->log->AppendText(_T(": "));
	ci->log->AppendText(message + _T("\n"));
	int iTabItem = GetTabByClient(sender);
	if (GetCurSel() == iTabItem && GetParent()->IsWindowVisible())
	{
		// chat window is already visible
		;
	}
	else if (GetCurSel() != iTabItem)
	{
		// chat window is already visible, but tab is not selected
		ci->notify = true;
	}
	else
	{
		ci->notify = true;
        if (isNewChatWindow || thePrefs.GetNotifierOnEveryChatMsg())
			theApp.emuledlg->ShowNotifier(GetResString(IDS_TBN_NEWCHATMSG) + _T(" ") + CString(sender->GetUserName()) + _T(":'") + message + _T("'\n"), TBN_CHAT);
		isNewChatWindow = false;
	}
}

bool CChatSelector::SendMessage(const CString& rstrMessage)
{
	CChatItem* ci = GetCurrentChatItem();
	if (!ci)
		return false;

	if ((UINT)ci->history.GetCount() == thePrefs.GetMaxChatHistoryLines())
		ci->history.RemoveAt(0);
	ci->history.Add(rstrMessage);
	ci->history_pos = ci->history.GetCount();

	// advance spamfilter stuff
	ci->client->IncMessagesSent();
	ci->client->SetSpammer(false);
	if (ci->client->GetChatState() == MS_CONNECTING)
		return false;

	if (thePrefs.GetIRCAddTimestamp())
		AddTimeStamp(ci);
	if (ci->client->socket && ci->client->socket->IsConnected())
	{
		CSafeMemFile data;
		data.WriteString(rstrMessage, ci->client->GetUnicodeSupport());
		Packet* packet = new Packet(&data, OP_EDONKEYPROT, OP_MESSAGE);
		theStats.AddUpDataOverheadOther(packet->size);
		ci->client->socket->SendPacket(packet, true, true);

		ci->log->AppendKeyWord(thePrefs.GetUserNick(), RGB(1,180,20));
		ci->log->AppendText(_T(": "));
		ci->log->AppendText(rstrMessage + _T("\n"));
	}
	else
	{
		ci->log->AppendKeyWord(_T("*** ") + GetResString(IDS_CONNECTING), RGB(255,0,0));
		ci->strMessagePending = rstrMessage;
		ci->client->SetChatState(MS_CONNECTING);
		ci->client->TryToConnect();
	}
	return true;
}

void CChatSelector::ConnectingResult(CUpDownClient* sender, bool success)
{
	CChatItem* ci = GetItemByClient(sender);
	if (!ci)
		return;

	ci->client->SetChatState(MS_CHATTING);
	if (!success){
		if (!ci->strMessagePending.IsEmpty()){
			ci->log->AppendKeyWord(_T(" ") + GetResString(IDS_FAILED) + _T("\n"), RGB(255,0,0));
			ci->strMessagePending.Empty();
		}
		else{
			if (thePrefs.GetIRCAddTimestamp())
				AddTimeStamp(ci);
			ci->log->AppendKeyWord(GetResString(IDS_CHATDISCONNECTED) + _T("\n"), RGB(255,0,0));
		}
	}
	else if (!ci->strMessagePending.IsEmpty()){
		ci->log->AppendKeyWord(_T(" ok\n"), RGB(255,0,0));
		
		CSafeMemFile data;
		data.WriteString(ci->strMessagePending, ci->client->GetUnicodeSupport());
		Packet* packet = new Packet(&data, OP_EDONKEYPROT, OP_MESSAGE);
		theStats.AddUpDataOverheadOther(packet->size);
		ci->client->socket->SendPacket(packet, true, true);

		if (thePrefs.GetIRCAddTimestamp())
			AddTimeStamp(ci);
		ci->log->AppendKeyWord(thePrefs.GetUserNick(), RGB(1,180,20));
		ci->log->AppendText(_T(": "));
		ci->log->AppendText(ci->strMessagePending + _T("\n"));
		
		ci->strMessagePending.Empty();
	}
	else{
		if (thePrefs.GetIRCAddTimestamp())
			AddTimeStamp(ci);
		ci->log->AppendKeyWord(_T("*** Connected\n"), RGB(255,0,0));
	}
}

void CChatSelector::DeleteAllItems()
{
	for (int i = 0; i < GetItemCount(); i++){
		TCITEM cur_item;
		cur_item.mask = TCIF_PARAM;
		if (GetItem(i, &cur_item))
			delete (CChatItem*)cur_item.lParam;
	}
}

void CChatSelector::OnTimer(UINT_PTR /*nIDEvent*/)
{
	m_blinkstate = !m_blinkstate;
	bool globalnotify = false;
	for (int i = 0; i < GetItemCount();i++)
	{
		TCITEM cur_item;
		cur_item.mask = TCIF_PARAM | TCIF_IMAGE;
		if (!GetItem(i, &cur_item))
			break;

		cur_item.mask = TCIF_IMAGE;
		if (((CChatItem*)cur_item.lParam)->notify){
			cur_item.iImage = (m_blinkstate) ? 1 : 2;
			SetItem(i, &cur_item);
			HighlightItem(i, TRUE);
			globalnotify = true;
		}
		else if (cur_item.iImage != 0){
			cur_item.iImage = 0;
			SetItem(i, &cur_item);
			HighlightItem(i, FALSE);
		}
	}

	if (globalnotify) {
		theApp.emuledlg->ShowMessageState(m_blinkstate ? 1 : 2);
		m_lastemptyicon = false;
	}
	else if (!m_lastemptyicon) {
		theApp.emuledlg->ShowMessageState(0);
		m_lastemptyicon = true;
	}
}

CChatItem* CChatSelector::GetCurrentChatItem()
{
	int iCurSel = GetCurSel();
	if (iCurSel == -1)
		return NULL;

	TCITEM cur_item;
	cur_item.mask = TCIF_PARAM;
	if (!GetItem(iCurSel, &cur_item))
		return NULL;

	return (CChatItem*)cur_item.lParam;
}

void CChatSelector::ShowChat()
{
	CChatItem* ci = GetCurrentChatItem();
	if (!ci)
		return;

	// show current chat window
	ci->log->ShowWindow(SW_SHOW);
	::SetFocus(m_hwndMessageBox);

	TCITEM item;
	item.mask = TCIF_IMAGE;
	item.iImage = 0;
	SetItem(GetCurSel(), &item);
	HighlightItem(GetCurSel(), FALSE);

	// hide all other chat windows
	item.mask = TCIF_PARAM;
	int i = 0;
	while (GetItem(i++, &item)){
		CChatItem* ci2 = (CChatItem*)item.lParam;
		if (ci2 != ci)
			ci2->log->ShowWindow(SW_HIDE);
	}

	ci->notify = false;
}

void CChatSelector::OnTcnSelchangeChatsel(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	ShowChat();
	*pResult = 0;
}

int CChatSelector::InsertItem(int nItem, TCITEM* pTabCtrlItem)
{
	int iResult = CClosableTabCtrl::InsertItem(nItem, pTabCtrlItem);
	RedrawWindow();
	return iResult;
}

BOOL CChatSelector::DeleteItem(int nItem)
{
	CClosableTabCtrl::DeleteItem(nItem);
	RedrawWindow();
	return TRUE;
}

void CChatSelector::EndSession(CUpDownClient* client)
{
	int iCurSel;
	if (client)
		iCurSel = GetTabByClient(client);
	else
		iCurSel = GetCurSel();
	if (iCurSel == -1)
		return;
	
	TCITEM item;
	item.mask = TCIF_PARAM;
	if (!GetItem(iCurSel, &item) || item.lParam == 0)
		return;
	CChatItem* ci = (CChatItem*)item.lParam;
	ci->client->SetChatState(MS_NONE);

	DeleteItem(iCurSel);
	delete ci;

	int iTabItems = GetItemCount();
	if (iTabItems > 0){
		// select next tab
		if (iCurSel == CB_ERR)
			iCurSel = 0;
		else if (iCurSel >= iTabItems)
			iCurSel = iTabItems - 1;
		(void)SetCurSel(iCurSel);				// returns CB_ERR if error or no prev. selection(!)
		iCurSel = GetCurSel();					// get the real current selection
		if (iCurSel == CB_ERR)					// if still error
			iCurSel = SetCurSel(0);
		ShowChat();
	}
}

void CChatSelector::GetChatSize(CRect& rcChat)
{
	CRect rcClose, rcSend, rcMessage;
	::GetWindowRect(m_hwndCloseBtn, &rcClose);
	::GetWindowRect(m_hwndSendBtn, &rcSend);
	::GetWindowRect(m_hwndMessageBox, &rcMessage);

	int iTop = rcClose.Height() > rcSend.Height() ? rcClose.Height() : rcSend.Height();
	if (iTop < rcMessage.Height())
		iTop = rcMessage.Height();
	
	CRect rcClient;
	GetClientRect(&rcClient);
	AdjustRect(FALSE, rcClient);
	rcChat.left = rcClient.left + 7;
	rcChat.top = rcClient.top + 7;
	rcChat.right = rcChat.left + rcClient.right - 18;
	rcChat.bottom = rcChat.top + rcClient.Height() - 7 - iTop - 14;
}

void CChatSelector::OnSize(UINT nType, int cx, int cy)
{
	CClosableTabCtrl::OnSize(nType, cx, cy);

	CRect rect;
	GetClientRect(&rect);
	AdjustRect(FALSE, rect);

	CRect rClose;
	::GetWindowRect(m_hwndCloseBtn, &rClose);
	::SetWindowPos(m_hwndCloseBtn, NULL, rect.right-7-rClose.Width(), rect.bottom-7-rClose.Height(),
				   rClose.Width(), rClose.Height(), SWP_NOZORDER);
	
	CRect rSend;
	::GetWindowRect(m_hwndSendBtn, &rSend);
	::SetWindowPos(m_hwndSendBtn, NULL, rect.right-7-rClose.Width()-7-rSend.Width(), rect.bottom-7-rSend.Height(),
				   rSend.Width(), rSend.Height(), SWP_NOZORDER);
	
	CRect rMessage;
	::GetWindowRect(m_hwndMessageBox, &rMessage);
	::SetWindowPos(m_hwndMessageBox, NULL, rect.left+7, rect.bottom-9-rMessage.Height(), 
				   rect.right-7-rClose.Width()-7-rSend.Width()-21, rMessage.Height(), SWP_NOZORDER);

	CRect rcChat;
	GetChatSize(rcChat);

	TCITEM item;
	item.mask = TCIF_PARAM;
	int i = 0;
	while (GetItem(i++, &item)){
		CChatItem* ci = (CChatItem*)item.lParam;
		ci->log->SetWindowPos(NULL, rcChat.left, rcChat.top, rcChat.Width(), rcChat.Height(), SWP_NOZORDER);
	}
}

void CChatSelector::OnBnClickedCclose()
{
	EndSession();
}

void CChatSelector::OnBnClickedCsend()
{
	CString strMessage;
	::GetWindowText(m_hwndMessageBox, strMessage.GetBuffer(MAX_CLIENT_MSG_LEN), MAX_CLIENT_MSG_LEN+1);
	strMessage.ReleaseBuffer();
	strMessage.Trim();
	if (!strMessage.IsEmpty())
	{
		if (SendMessage(strMessage))
			::SetWindowText(m_hwndMessageBox, _T(""));
	}

	::SetFocus(m_hwndMessageBox);
}

BOOL CChatSelector::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN){
			if (pMsg->hwnd == m_hwndMessageBox)
				OnBnClickedCsend();
		}

		if (pMsg->hwnd == m_hwndMessageBox && (pMsg->wParam == VK_UP || pMsg->wParam == VK_DOWN)){
			theApp.emuledlg->chatwnd->ScrollHistory(pMsg->wParam == VK_DOWN);
			return TRUE;
		}
	}
	return CClosableTabCtrl::PreTranslateMessage(pMsg);
}

void CChatSelector::Localize(void)
{
	if (m_hWnd)
	{
		if (m_hwndSendBtn)
			::SetWindowText(m_hwndSendBtn, GetResString(IDS_CW_SEND));
		else
			GetParent()->GetDlgItem(IDC_CSEND)->SetWindowText(GetResString(IDS_CW_SEND));

		if (m_hwndCloseBtn)
			::SetWindowText(m_hwndCloseBtn, GetResString(IDS_CW_CLOSE));
		else
			GetParent()->GetDlgItem(IDC_CCLOSE)->SetWindowText(GetResString(IDS_CW_CLOSE));
	}
}

bool CChatSelector::IsSpam(CString strMessage, CUpDownClient* client)
{
	// first step, spam dectection will be further improved in future versions
	if ( !thePrefs.IsAdvSpamfilterEnabled() || client->IsFriend() ) // friends are never spammer... (but what if two spammers are friends :P )
		return false;

	if (client->IsSpammer())
		return true;

	// first fixed criteria: If a client  sends me an URL in his first message before I response to him
	// there is a 99,9% chance that it is some poor guy advising his leech mod, or selling you .. well you know :P
	if (client->GetMessagesSent() == 0){
		int curPos=0;
		CString resToken = CString(URLINDICATOR).Tokenize(_T("|"), curPos);
		while (resToken != _T("")){
			if (strMessage.Find(resToken) > (-1) )
				return true;
			resToken= CString(URLINDICATOR).Tokenize(_T("|"),curPos);
		}
	}
	// second fixed criteria: he sent me 4  or more messages and I didn't answered him once
	if (client->GetMessagesReceived() > 3 && client->GetMessagesSent() == 0)
		return true;

	// to be continued
	return false;
}

void CChatSelector::AddTimeStamp(CChatItem* ci)
{
	ci->log->AppendText(CTime::GetCurrentTime().Format(_T("[%X] ")));
}

void CChatSelector::OnDestroy()
{
	if (m_Timer){
		KillTimer(m_Timer);
		m_Timer = NULL;
	}
	CClosableTabCtrl::OnDestroy();
}

/*BOOL CChatSelector::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch (wParam){
		case MP_DETAIL:{
			const CChatItem* ci = GetCurrentChatItem();
			if (ci) {
				CClientDetailDialog dialog(ci->client);
				dialog.DoModal();
			}
			return TRUE;
		}
		case MP_ADDFRIEND:{
			const CChatItem* ci = GetCurrentChatItem();
			if (ci) {
				CFriend* fr = theApp.friendlist->SearchFriend(ci->client->GetUserHash(), 0, 0);
				if (!fr)
					theApp.friendlist->AddFriend(ci->client);
			}
			return TRUE;
		}
		case MP_REMOVEFRIEND:{
			const CChatItem* ci = GetCurrentChatItem();
			if (ci) {
				CFriend* fr = theApp.friendlist->SearchFriend(ci->client->GetUserHash(), 0, 0);
				if (fr)
					theApp.friendlist->RemoveFriend(fr);
			}
			return TRUE;
		}
		case MP_REMOVE:{
			const CChatItem* ci = GetCurrentChatItem();
			if (ci)
				EndSession(ci->client);
			return TRUE;
		}
	}
	return CClosableTabCtrl::OnCommand(wParam, lParam);
}*/

// This does not work! This offers context menu entries which always
// work for the selected tab item only! The offered context menu entries have to work for
// the item which is under the cursor. Thus one can e.g. no longer use the 'Close' function
// for other (non-selected) items.
/*void CChatSelector::OnContextMenu(CWnd*, CPoint point)
{
	const CChatItem* ci = GetCurrentChatItem();
	if (ci == NULL)
		return;
	CFriend* pFriend = theApp.friendlist->SearchFriend(ci->client->GetUserHash(), 0, 0);

	CTitleMenu menu;
	menu.CreatePopupMenu();
	menu.AddMenuTitle(GetResString(IDS_CLIENT), true);

	menu.AppendMenu(MF_STRING, MP_REMOVE, GetResString(IDS_FD_CLOSE));
	menu.AppendMenu(MF_STRING, MP_DETAIL, GetResString(IDS_SHOWDETAILS), _T("CLIENTDETAILS"));
	menu.SetDefaultItem(MP_DETAIL);

	GetCurrentChatItem();
	if (pFriend == NULL)
		menu.AppendMenu(MF_STRING, MP_ADDFRIEND, GetResString(IDS_IRC_ADDTOFRIENDLIST), _T("ADDFRIEND"));
	else
		menu.AppendMenu(MF_STRING, MP_REMOVEFRIEND, GetResString(IDS_REMOVEFRIEND), _T("DELETEFRIEND"));
	
	m_ptCtxMenu = point;
	ScreenToClient(&m_ptCtxMenu);
	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
}
*/
#endif
