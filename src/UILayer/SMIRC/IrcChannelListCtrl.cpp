/* 
 * $Id: IrcChannelListCtrl.cpp 7701 2008-10-15 07:34:41Z huby $
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
#include "./IrcChannelListCtrl.h"
#include "./emuleDlg.h"
#include "./otherfunctions.h"
#include "./MenuCmds.h"
#include "./IrcWnd.h"
#include "./IrcMain.h"
#include "./emule.h"
#include "./MemDC.h"

#if _ENABLE_NOUSE

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

struct ChannelList
{
	CString m_sName;
	CString m_sUsers;
	CString m_sDesc;
};

IMPLEMENT_DYNAMIC(CIrcChannelListCtrl, CMuleListCtrl)

BEGIN_MESSAGE_MAP(CIrcChannelListCtrl, CMuleListCtrl)
ON_WM_CONTEXTMENU()
ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnLvnColumnclick)
ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclk)
END_MESSAGE_MAP()

CIrcChannelListCtrl::CIrcChannelListCtrl()
{
	m_pParent = NULL;
	SetName(_T("IrcChannelListCtrl"));
}

CIrcChannelListCtrl::~CIrcChannelListCtrl()
{
	//Remove and delete serverChannelList.
	ResetServerChannelList(true);
}

int CIrcChannelListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	ChannelList* pItem1 = (ChannelList*)lParam1;
	ChannelList* pItem2 = (ChannelList*)lParam2;
	switch(lParamSort)
	{
		case 0:
			return pItem1->m_sName.CompareNoCase(pItem2->m_sName);
		case 10:
			return pItem2->m_sName.CompareNoCase(pItem1->m_sName);
		case 1:
			return  _tstoi(pItem1->m_sUsers) - _tstoi(pItem2->m_sUsers);
		case 11:
			return _tstoi(pItem2->m_sUsers) - _tstoi(pItem1->m_sUsers);
		case 2:
			return pItem1->m_sDesc.CompareNoCase(pItem2->m_sDesc);
		case 12:
			return pItem2->m_sDesc.CompareNoCase(pItem1->m_sDesc);
		default:
			return 0;
	}
}

void CIrcChannelListCtrl::OnLvnColumnclick(NMHDR* pNMHDR, LRESULT* pResult)
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

void CIrcChannelListCtrl::OnContextMenu(CWnd*, CPoint point)
{
	int iCurSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	CTitleMenu menuChannel;
	menuChannel.CreatePopupMenu();
	menuChannel.AddMenuTitle(GetResString(IDS_IRC_CHANNEL));
	menuChannel.AppendMenu(MF_STRING, Irc_Join, GetResString(IDS_IRC_JOIN));
	if (iCurSel == -1)
		menuChannel.EnableMenuItem(Irc_Join, MF_GRAYED);
	GetPopupMenuPos(*this, point);
	menuChannel.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
	VERIFY( menuChannel.DestroyMenu() );
}

void CIrcChannelListCtrl::OnNMDblclk(NMHDR*, LRESULT* pResult)
{
	JoinChannels();
	*pResult = 0;
}

void CIrcChannelListCtrl::ResetServerChannelList( bool bShutDown )
{
	//Delete our ServerChannelList..
	POSITION pos1, pos2;
	ChannelList* pCurChannel = NULL;
	for (pos1 = m_ptrlistChannel.GetHeadPosition();( pos2 = pos1 ) != NULL;)
	{
		m_ptrlistChannel.GetNext(pos1);
		pCurChannel =	(ChannelList*)m_ptrlistChannel.GetAt(pos2);
		m_ptrlistChannel.RemoveAt(pos2);
		delete pCurChannel;
	}
	if( !bShutDown )
	{
		//Only do this if eMule is still running..
		DeleteAllItems();
	}
}

void CIrcChannelListCtrl::AddChannelToList( CString sName, CString sUser, CString sDescription )
{
	//Add a new channel to Server Channel List
	CString sNameTemp = sName;
	CString sDescTemp = sDescription;
	UINT uUserTest = _tstoi(sUser);
	if( thePrefs.GetIRCUseChanFilter() )
	{
		//We need to filter the channels..
		if( uUserTest < thePrefs.GetIRCChannelUserFilter() )
		{
			//There were not enough users in the channel.
			return;
		}
		if( sDescTemp.MakeLower().Find(thePrefs.GetIRCChanNameFilter().MakeLower()) == -1 && sNameTemp.MakeLower().Find(thePrefs.GetIRCChanNameFilter().MakeLower()) == -1)
		{
			//The word we wanted was not in the channel name or description..
			return;
		}
	}
	//Create new ChannelList object.
	ChannelList* plistToAdd = new ChannelList;
	plistToAdd->m_sName = sName;
	plistToAdd->m_sUsers = sUser;
	//Strip any color codes out of the description..
	plistToAdd->m_sDesc = m_pParent->StripMessageOfFontCodes(sDescription);
	plistToAdd->m_sDesc.Replace(_T("\004"), _T("%"));
	//Add to tail and update list.
	m_ptrlistChannel.AddTail( plistToAdd);
	int itemnr = GetItemCount();
	itemnr = InsertItem(LVIF_PARAM,itemnr,0,0,0,0,(LPARAM)plistToAdd);
	SetItemText(itemnr,0,plistToAdd->m_sName);
	SetItemText(itemnr,1,plistToAdd->m_sUsers);
	SetItemText(itemnr,2,plistToAdd->m_sDesc);
}

void CIrcChannelListCtrl::JoinChannels()
{
	if( !m_pParent->IsConnected() )
		return;
	int iIndex = -1;
	POSITION pos = GetFirstSelectedItemPosition();
	while(pos != NULL)
	{
		iIndex = GetNextSelectedItem(pos);
		if(iIndex > -1)
		{
			CString sJoin;
			sJoin = _T("JOIN ") + GetItemText(iIndex, 0 );
			m_pParent->m_pIrcMain->SendString( sJoin );
		}
	}
}

void CIrcChannelListCtrl::Localize()
{
	CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
	HDITEM hdi;
	hdi.mask = HDI_TEXT;
	CString strRes;

	strRes = GetResString(IDS_UUSERS);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(1, &hdi);

	strRes = GetResString(IDS_DESCRIPTION);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(2, &hdi);

	strRes = GetResString(IDS_IRC_NAME);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(0, &hdi);
}

void CIrcChannelListCtrl::Init()
{
	InsertColumn(0, GetResString(IDS_IRC_NAME), LVCFMT_LEFT, 203 );
	InsertColumn(1, GetResString(IDS_UUSERS), LVCFMT_LEFT, 50 );
	InsertColumn(2, GetResString(IDS_DESCRIPTION), LVCFMT_LEFT, 350 );

	LoadSettings();
	SetSortArrow();
	SortItems(&SortProc, GetSortItem() + ( (GetSortAscending()) ? 0:10) );
}

BOOL CIrcChannelListCtrl::OnCommand(WPARAM wParam, LPARAM)
{
	switch( wParam )
	{
		case Irc_Join:
			{
				//Pressed the join button.
				JoinChannels();
				return true;
			}
	}
	return true;
}

void  CIrcChannelListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if( !theApp.emuledlg->IsRunning() )
		return;
	if (!lpDrawItemStruct->itemData)
		return;

	RECT clientRect;
	GetClientRect(&clientRect);
	RECT cur_rec = lpDrawItemStruct->rcItem;
	if (cur_rec.top >= clientRect.bottom || cur_rec.bottom <= clientRect.top)
		return;

	CDC* odc = CDC::FromHandle(lpDrawItemStruct->hDC);
	BOOL bCtrlFocused = ((GetFocus() == this ) || (GetStyle() & LVS_SHOWSELALWAYS));

	if( (lpDrawItemStruct->itemAction | ODA_SELECT) && (lpDrawItemStruct->itemState & ODS_SELECTED ))
	{
		if(bCtrlFocused)
			odc->SetBkColor(m_crHighlight);
		else
			odc->SetBkColor(m_crNoHighlight);
	}
	else
		odc->SetBkColor(GetBkColor());

	CMemDC dc(CDC::FromHandle(lpDrawItemStruct->hDC), &lpDrawItemStruct->rcItem);
	CFont* pOldFont = dc.SelectObject(GetFont());
	COLORREF crOldTextColor = dc.SetTextColor(m_crWindowText);

	int iOldBkMode;
	if (m_crWindowTextBk == CLR_NONE)
	{
		DefWindowProc(WM_ERASEBKGND, (WPARAM)(HDC)dc, 0);
		iOldBkMode = dc.SetBkMode(TRANSPARENT);
	}
	else
		iOldBkMode = OPAQUE;

	CString Sbuffer;
	CHeaderCtrl *pHeaderCtrl = GetHeaderCtrl();
	int iCount = pHeaderCtrl->GetItemCount();
	cur_rec.right = cur_rec.left - 8;
	cur_rec.left += 4;

	for(int iCurrent = 0; iCurrent < iCount; iCurrent++)
	{
		int iColumn = pHeaderCtrl->OrderToIndex(iCurrent);
		if( !IsColumnHidden(iColumn) )
		{
			cur_rec.right += GetColumnWidth(iColumn);
			ChannelList* toadd =(ChannelList*)lpDrawItemStruct->itemData;
			LOGFONT lfFont;
			CFont fontCustom;
			GetFont()->GetLogFont(&lfFont);

			switch(iColumn)
			{

				case 0:
					{
						lfFont.lfWeight = FW_BOLD;
						fontCustom.CreateFontIndirect(&lfFont);
						dc.SelectObject(&fontCustom);
						dc->SetTextColor(RGB(0,0,130));
						Sbuffer.Format(_T("%s"), toadd->m_sName);
						break;
					}
				case 1:
					{

						lfFont.lfWeight = FW_NORMAL;
						fontCustom.CreateFontIndirect(&lfFont);
						dc.SelectObject(&fontCustom);
						int usercount = _wtoi(toadd->m_sUsers);
						if(usercount<10)
							dc->SetTextColor(RGB(0,0,130));
						else if(usercount<100)
							dc->SetTextColor(RGB(0,0,200));
						else
							dc->SetTextColor(RGB(0,0,255));
						Sbuffer.Format(_T("%s"), toadd->m_sUsers);
						break;
					}
				case 2:
					{
						lfFont.lfWeight = FW_NORMAL;
						fontCustom.CreateFontIndirect(&lfFont);
						dc.SelectObject(&fontCustom);
						dc->SetTextColor(RGB(0,0,130));
						Sbuffer.Format(_T("%s"), toadd->m_sDesc);
						break;
					}

			}//End of Switch

			dc->DrawText(Sbuffer,Sbuffer.GetLength(),&cur_rec, DT_LEFT);
			cur_rec.left += GetColumnWidth(iColumn);
		}
	}
	//draw rectangle around selected item(s)
	if ((lpDrawItemStruct->itemAction | ODA_SELECT) && (lpDrawItemStruct->itemState & ODS_SELECTED))
	{
		RECT outline_rec = lpDrawItemStruct->rcItem;
		outline_rec.top--;
		outline_rec.bottom++;
		dc->FrameRect(&outline_rec, &CBrush(GetBkColor()));
		outline_rec.top++;
		outline_rec.bottom--;
		outline_rec.left++;
		outline_rec.right--;
		if(bCtrlFocused)
			dc->FrameRect(&outline_rec, &CBrush(m_crFocusLine));
		else
			dc->FrameRect(&outline_rec, &CBrush(m_crNoFocusLine));
	}
	if (m_crWindowTextBk == CLR_NONE)
		dc.SetBkMode(iOldBkMode);
	dc.SelectObject(pOldFont);
	dc.SetTextColor(crOldTextColor);
}

#endif
