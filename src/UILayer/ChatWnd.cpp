/* 
 * $Id: ChatWnd.cpp 7701 2008-10-15 07:34:41Z huby $
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
#include "ChatWnd.h"
#include "HTRichEditCtrl.h"
#include "FriendList.h"
#include "emuledlg.h"
#include "UpDownClient.h"
#include "OtherFunctions.h"
#include "HelpIDs.h"
#include "Opcodes.h"
#include "friend.h"
#include "ClientCredits.h"
#include "IconStatic.h"
#include "UserMsgs.h"

#if _ENABLE_NOUSE

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define	SPLITTER_RANGE_WIDTH	200
#define	SPLITTER_RANGE_HEIGHT	700

#define	SPLITTER_MARGIN			2
#define	SPLITTER_WIDTH			4


// CChatWnd dialog

IMPLEMENT_DYNAMIC(CChatWnd, CDialog)

BEGIN_MESSAGE_MAP(CChatWnd, CResizableDialog)
	ON_WM_KEYDOWN()
	ON_WM_SHOWWINDOW()
	ON_MESSAGE(UM_CLOSETAB, OnCloseTab)
	ON_WM_SYSCOLORCHANGE()
    ON_WM_CONTEXTMENU()
	ON_WM_HELPINFO()
	ON_NOTIFY(LVN_ITEMACTIVATE, IDC_LIST2, OnLvnItemActivateFrlist)
	ON_NOTIFY(NM_CLICK, IDC_LIST2, OnNMClickFrlist)
	ON_STN_DBLCLK(IDC_FRIENDSICON, OnStnDblclickFriendsicon)
END_MESSAGE_MAP()

CChatWnd::CChatWnd(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CChatWnd::IDD, pParent)
{
	icon_friend = NULL;
	icon_msg = NULL;
}

CChatWnd::~CChatWnd()
{
	if (icon_friend)
		VERIFY( DestroyIcon(icon_friend) );
	if (icon_msg)
		VERIFY( DestroyIcon(icon_msg) );
}

void CChatWnd::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHATSEL, chatselector);
	DDX_Control(pDX, IDC_LIST2, m_FriendListCtrl);
	DDX_Control(pDX, IDC_CMESSAGE, inputtext);
	DDX_Control(pDX, IDC_FRIENDS_MSG, m_cUserInfo);
}

void CChatWnd::OnLvnItemActivateFrlist(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	int iSel = m_FriendListCtrl.GetSelectionMark();
	if (iSel != -1) {
		CFriend* pFriend = (CFriend*)m_FriendListCtrl.GetItemData(iSel);
		ShowFriendMsgDetails(pFriend);
	}
	else
		ShowFriendMsgDetails(NULL);
}

void CChatWnd::ShowFriendMsgDetails(CFriend* pFriend)
{
	if (pFriend)
	{
		CString buffer;

		// Name
		if (pFriend->GetLinkedClient())
			GetDlgItem(IDC_FRIENDS_NAME_EDIT)->SetWindowText(pFriend->GetLinkedClient()->GetUserName());
		else if (pFriend->m_strName != _T(""))
			GetDlgItem(IDC_FRIENDS_NAME_EDIT)->SetWindowText(pFriend->m_strName);
		else
			GetDlgItem(IDC_FRIENDS_NAME_EDIT)->SetWindowText(_T("?"));

		// Hash
		if (pFriend->GetLinkedClient())
			GetDlgItem(IDC_FRIENDS_USERHASH_EDIT)->SetWindowText(md4str(pFriend->GetLinkedClient()->GetUserHash()));
		else if (pFriend->m_dwHasHash)
			GetDlgItem(IDC_FRIENDS_USERHASH_EDIT)->SetWindowText(md4str(pFriend->m_abyUserhash));
		else
			GetDlgItem(IDC_FRIENDS_USERHASH_EDIT)->SetWindowText(_T("?"));

		// Client
		if (pFriend->GetLinkedClient())
			GetDlgItem(IDC_FRIENDS_CLIENTE_EDIT)->SetWindowText(pFriend->GetLinkedClient()->GetClientSoftVer());
		else
			GetDlgItem(IDC_FRIENDS_CLIENTE_EDIT)->SetWindowText(_T("?"));

		// Identification
		if (pFriend->GetLinkedClient() && pFriend->GetLinkedClient()->Credits())
		{
			if (CGlobalVariable::clientcredits->CryptoAvailable())
			{
				switch (pFriend->GetLinkedClient()->Credits()->GetCurrentIdentState(pFriend->GetLinkedClient()->GetIP()))
				{
					case IS_NOTAVAILABLE:
						GetDlgItem(IDC_FRIENDS_IDENTIFICACION_EDIT)->SetWindowText(GetResString(IDS_IDENTNOSUPPORT));
						break;
					case IS_IDFAILED:
					case IS_IDNEEDED:
					case IS_IDBADGUY:
						GetDlgItem(IDC_FRIENDS_IDENTIFICACION_EDIT)->SetWindowText(GetResString(IDS_IDENTFAILED));
						break;
					case IS_IDENTIFIED:
						GetDlgItem(IDC_FRIENDS_IDENTIFICACION_EDIT)->SetWindowText(GetResString(IDS_IDENTOK));
						break;
				}
			}
			else
				GetDlgItem(IDC_FRIENDS_IDENTIFICACION_EDIT)->SetWindowText(GetResString(IDS_IDENTNOSUPPORT));
		}
		else
			GetDlgItem(IDC_FRIENDS_IDENTIFICACION_EDIT)->SetWindowText(_T("?"));

		// Upload and downloaded
		if (pFriend->GetLinkedClient() && pFriend->GetLinkedClient()->Credits())
			GetDlgItem(IDC_FRIENDS_DESCARGADO_EDIT)->SetWindowText(CastItoXBytes(pFriend->GetLinkedClient()->Credits()->GetDownloadedTotal(), false, false));
		else
			GetDlgItem(IDC_FRIENDS_DESCARGADO_EDIT)->SetWindowText(_T("?"));

		if (pFriend->GetLinkedClient() && pFriend->GetLinkedClient()->Credits())
			GetDlgItem(IDC_FRIENDS_SUBIDO_EDIT)->SetWindowText(CastItoXBytes(pFriend->GetLinkedClient()->Credits()->GetUploadedTotal(), false, false));
		else
			GetDlgItem(IDC_FRIENDS_SUBIDO_EDIT)->SetWindowText(_T("?"));
	}
	else
	{
		GetDlgItem(IDC_FRIENDS_NAME_EDIT)->SetWindowText(_T("-"));
		GetDlgItem(IDC_FRIENDS_USERHASH_EDIT)->SetWindowText(_T("-"));
		GetDlgItem(IDC_FRIENDS_CLIENTE_EDIT)->SetWindowText(_T("-"));
		GetDlgItem(IDC_FRIENDS_IDENTIFICACION_EDIT)->SetWindowText(_T("-"));
		GetDlgItem(IDC_FRIENDS_DESCARGADO_EDIT)->SetWindowText(_T("-"));
		GetDlgItem(IDC_FRIENDS_SUBIDO_EDIT)->SetWindowText(_T("-"));
	}
}

BOOL CChatWnd::OnInitDialog()
{
	CResizableDialog::OnInitDialog();

	inputtext.SetLimitText(MAX_CLIENT_MSG_LEN);
	chatselector.Init();
	m_FriendListCtrl.Init();

	SetAllIcons();

	CRect rcSpl;
	GetDlgItem(IDC_LIST2)->GetWindowRect(rcSpl);
	ScreenToClient(rcSpl);

	CRect rc;
	GetWindowRect(rc);
	ScreenToClient(rc);

	rcSpl.bottom = rc.bottom - 5;
	rcSpl.left = rcSpl.right + SPLITTER_MARGIN;
	rcSpl.right = rcSpl.left + SPLITTER_WIDTH;
	m_wndSplitterchat.Create(WS_CHILD | WS_VISIBLE, rcSpl, this, IDC_SPLITTER_FRIEND);

	int PosStatVinit = rcSpl.left;
	int PosStatVnew = thePrefs.GetSplitterbarPositionFriend();
	UINT max = SPLITTER_RANGE_HEIGHT;
	UINT min = SPLITTER_RANGE_WIDTH;
	if (thePrefs.GetSplitterbarPositionFriend() > max)
		PosStatVnew = max;
	else if (thePrefs.GetSplitterbarPositionFriend() < min)
		PosStatVnew = min;
	rcSpl.left = PosStatVnew;
	rcSpl.right = PosStatVnew + SPLITTER_WIDTH;
	m_wndSplitterchat.MoveWindow(rcSpl);

	DoResize(PosStatVnew - PosStatVinit);

	AddAnchor(IDC_FRIENDSICON, TOP_LEFT);
	AddAnchor(IDC_FRIENDS_LBL, TOP_LEFT);
	AddAnchor(IDC_FRIENDS_NAME, BOTTOM_LEFT);
	AddAnchor(IDC_FRIENDS_USERHASH, BOTTOM_LEFT);
	AddAnchor(IDC_FRIENDS_CLIENT, BOTTOM_LEFT);
	AddAnchor(IDC_FRIENDS_IDENT, BOTTOM_LEFT);
	AddAnchor(IDC_FRIENDS_UPLOADED, BOTTOM_LEFT);
	AddAnchor(IDC_FRIENDS_DOWNLOADED, BOTTOM_LEFT);

	Localize();
	theApp.friendlist->ShowFriends();

	return TRUE;
}

void CChatWnd::DoResize(int delta)
{
	CSplitterControl::ChangeWidth(GetDlgItem(IDC_LIST2), delta);
	CSplitterControl::ChangeWidth(GetDlgItem(IDC_FRIENDS_MSG), delta);
	CSplitterControl::ChangeWidth(GetDlgItem(IDC_FRIENDS_NAME_EDIT), delta);
	CSplitterControl::ChangeWidth(GetDlgItem(IDC_FRIENDS_USERHASH_EDIT), delta);
	CSplitterControl::ChangeWidth(GetDlgItem(IDC_FRIENDS_CLIENTE_EDIT), delta);
	CSplitterControl::ChangeWidth(GetDlgItem(IDC_FRIENDS_IDENTIFICACION_EDIT), delta);
	CSplitterControl::ChangeWidth(GetDlgItem(IDC_FRIENDS_SUBIDO_EDIT), delta);
	CSplitterControl::ChangeWidth(GetDlgItem(IDC_FRIENDS_DESCARGADO_EDIT), delta);
	CSplitterControl::ChangeWidth(GetDlgItem(IDC_CHATSEL), -delta, CW_RIGHTALIGN);
	CSplitterControl::ChangePos(GetDlgItem(IDC_MESSAGES_LBL), -delta, 0);
	CSplitterControl::ChangePos(GetDlgItem(IDC_MESSAGEICON), -delta, 0);

	CRect rcW;
	GetWindowRect(rcW);
	ScreenToClient(rcW);

	CRect rcspl;
	GetDlgItem(IDC_LIST2)->GetClientRect(rcspl);

	thePrefs.SetSplitterbarPositionFriend(rcspl.right);

	RemoveAnchor(m_wndSplitterchat);
	AddAnchor(m_wndSplitterchat, TOP_LEFT);

	RemoveAnchor(IDC_LIST2);
	AddAnchor(IDC_LIST2, TOP_LEFT, BOTTOM_LEFT);

	RemoveAnchor(IDC_FRIENDS_MSG);
	AddAnchor(IDC_FRIENDS_MSG, BOTTOM_LEFT, BOTTOM_LEFT);

	RemoveAnchor(IDC_CHATSEL);
	AddAnchor(IDC_CHATSEL, TOP_LEFT, BOTTOM_RIGHT);

	RemoveAnchor(IDC_MESSAGES_LBL);
	AddAnchor(IDC_MESSAGES_LBL, TOP_LEFT);

	RemoveAnchor(IDC_MESSAGEICON);
	AddAnchor(IDC_MESSAGEICON, TOP_LEFT);

	RemoveAnchor(IDC_FRIENDS_NAME_EDIT);
	RemoveAnchor(IDC_FRIENDS_USERHASH_EDIT);
	RemoveAnchor(IDC_FRIENDS_CLIENTE_EDIT);
	RemoveAnchor(IDC_FRIENDS_IDENTIFICACION_EDIT);
	RemoveAnchor(IDC_FRIENDS_SUBIDO_EDIT);
	RemoveAnchor(IDC_FRIENDS_DESCARGADO_EDIT);
	AddAnchor(IDC_FRIENDS_NAME_EDIT, BOTTOM_LEFT);
	AddAnchor(IDC_FRIENDS_USERHASH_EDIT, BOTTOM_LEFT);
	AddAnchor(IDC_FRIENDS_CLIENTE_EDIT, BOTTOM_LEFT);
	AddAnchor(IDC_FRIENDS_IDENTIFICACION_EDIT, BOTTOM_LEFT);
	AddAnchor(IDC_FRIENDS_SUBIDO_EDIT, BOTTOM_LEFT);
	AddAnchor(IDC_FRIENDS_DESCARGADO_EDIT, BOTTOM_LEFT);

	m_wndSplitterchat.SetRange(rcW.left+SPLITTER_RANGE_WIDTH, rcW.left+SPLITTER_RANGE_HEIGHT);

	m_FriendListCtrl.SaveSettings();
	m_FriendListCtrl.DeleteColumn(0);
	m_FriendListCtrl.Init();

	Invalidate();
	UpdateWindow();
}

LRESULT CChatWnd::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch (message)
	{
	case WM_PAINT:
		if (m_wndSplitterchat)
		{
			CRect rcW;
			GetWindowRect(rcW);
			ScreenToClient(rcW);
			if (rcW.Width() > 0)
			{
				CRect rctree;
				GetDlgItem(IDC_LIST2)->GetWindowRect(rctree);
				ScreenToClient(rctree);

				CRect rcSpl;
				rcSpl.left = rctree.right + SPLITTER_MARGIN;
				rcSpl.right = rcSpl.left + SPLITTER_WIDTH;
				rcSpl.top = rctree.top;
				rcSpl.bottom = rcW.bottom - 5;

				m_wndSplitterchat.MoveWindow(rcSpl, TRUE);
				m_FriendListCtrl.DeleteColumn(0);
				m_FriendListCtrl.Init();
			}
		}
		break;

	case WM_NOTIFY:
		if (wParam == IDC_SPLITTER_FRIEND)
		{ 
			SPC_NMHDR* pHdr = (SPC_NMHDR*)lParam;
			DoResize(pHdr->delta);
		}
		break;

	case WM_WINDOWPOSCHANGED:
		{
			CRect rcW;
			GetWindowRect(rcW);
			ScreenToClient(rcW);
			if (m_wndSplitterchat && rcW.Width()>0)
				Invalidate();
			break;
		}
	case WM_SIZE:
		if (m_wndSplitterchat)
		{
			CRect rc;
			GetWindowRect(rc);
			ScreenToClient(rc);
			m_wndSplitterchat.SetRange(rc.left+SPLITTER_RANGE_WIDTH, rc.left+SPLITTER_RANGE_HEIGHT);
		}
		break;
	}
	return CResizableDialog::DefWindowProc(message, wParam, lParam);
}

void CChatWnd::StartSession(CUpDownClient* client){
	if (!client->GetUserName())
		return;
	theApp.emuledlg->SetActiveDialog(this);
	chatselector.StartSession(client,true);
}

void CChatWnd::OnShowWindow(BOOL bShow, UINT /*nStatus*/)
{
	if (bShow)
		chatselector.ShowChat();
}

BOOL CChatWnd::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_KEYDOWN)
	{
		// Don't handle Ctrl+Tab in this window. It will be handled by main window.
		if (pMsg->wParam == VK_TAB && GetAsyncKeyState(VK_CONTROL) < 0)
			return FALSE;
	}
	else if (pMsg->message == WM_KEYUP)
	{
		if (pMsg->hwnd == GetDlgItem(IDC_LIST2)->m_hWnd)
			OnLvnItemActivateFrlist(0, 0);
	}

	return CResizableDialog::PreTranslateMessage(pMsg);
}

void CChatWnd::OnNMClickFrlist(NMHDR *pNMHDR, LRESULT *pResult){
	OnLvnItemActivateFrlist(pNMHDR,pResult);
	*pResult = 0;
}

void CChatWnd::SetAllIcons()
{
	InitWindowStyles(this);

	if (icon_friend)
		VERIFY( DestroyIcon(icon_friend) );
	if (icon_msg)
		VERIFY( DestroyIcon(icon_msg) );
	icon_friend = theApp.LoadIcon(_T("Friend"), 16, 16);
	icon_msg = theApp.LoadIcon(_T("Message"), 16, 16);
	((CStatic*)GetDlgItem(IDC_MESSAGEICON))->SetIcon(icon_msg);
	((CStatic*)GetDlgItem(IDC_FRIENDSICON))->SetIcon(icon_friend);
	m_cUserInfo.SetIcon(_T("Info"));
}

void CChatWnd::Localize()
{
	GetDlgItem(IDC_FRIENDS_LBL)->SetWindowText(GetResString(IDS_CW_FRIENDS));
	GetDlgItem(IDC_MESSAGES_LBL)->SetWindowText(GetResString(IDS_CW_MESSAGES));
	m_cUserInfo.SetWindowText(GetResString(IDS_INFO));
	GetDlgItem(IDC_FRIENDS_DOWNLOADED)->SetWindowText(GetResString(IDS_CHAT_DOWNLOADED));
	GetDlgItem(IDC_FRIENDS_UPLOADED)->SetWindowText(GetResString(IDS_CHAT_UPLOADED));
	GetDlgItem(IDC_FRIENDS_IDENT)->SetWindowText(GetResString(IDS_CHAT_IDENT));
	GetDlgItem(IDC_FRIENDS_CLIENT)->SetWindowText(GetResString(IDS_CD_CSOFT));
	GetDlgItem(IDC_FRIENDS_NAME)->SetWindowText(GetResString(IDS_CD_UNAME));
	GetDlgItem(IDC_FRIENDS_USERHASH)->SetWindowText(GetResString(IDS_CD_UHASH));	

	chatselector.Localize();
	m_FriendListCtrl.Localize();
}

LRESULT CChatWnd::OnCloseTab(WPARAM wParam, LPARAM /*lParam*/)
{
	TCITEM item = {0};
	item.mask = TCIF_PARAM;
	if (chatselector.GetItem((int)wParam, &item))
		chatselector.EndSession(((CChatItem*)item.lParam)->client);
	return TRUE;
}

void CChatWnd::ScrollHistory(bool down) {
	CString buffer;

	CChatItem* ci = chatselector.GetCurrentChatItem();
	if (ci==NULL) return;

	if ( (ci->history_pos==0 && !down) || (ci->history_pos==ci->history.GetCount() && down)) return;
	
	if (down) ++ci->history_pos; else --ci->history_pos;

	buffer = (ci->history_pos == ci->history.GetCount()) ? _T("") : ci->history.GetAt(ci->history_pos);

	inputtext.SetWindowText(buffer);
	inputtext.SetSel(buffer.GetLength(),buffer.GetLength());
}

void CChatWnd::OnSysColorChange()
{
	CResizableDialog::OnSysColorChange();
	SetAllIcons();
}

void CChatWnd::UpdateFriendlistCount(UINT count) {
	CString temp;
	temp.Format(_T(" (%i)"),count);
	temp=GetResString(IDS_CW_FRIENDS)+temp;

	GetDlgItem(IDC_FRIENDS_LBL)->SetWindowText(temp);
}

BOOL CChatWnd::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	theApp.ShowHelp(eMule_FAQ_Friends);
	return TRUE;
}

void CChatWnd::OnStnDblclickFriendsicon()
{
	theApp.emuledlg->ShowPreferences(IDD_PPG_FILES);
}

#endif
