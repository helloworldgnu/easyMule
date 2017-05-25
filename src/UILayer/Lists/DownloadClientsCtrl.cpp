/*
 * $Id: DownloadClientsCtrl.cpp 7701 2008-10-15 07:34:41Z huby $
 * 
 * this file is part of easyMule
 * Copyright (C)2002-2008 VeryCD Dev Team ( strEmail.Format("%s@%s", "emuledev", "verycd.com") / http: * www.easymule.org )
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
//--- xrmb:downloadclientslist ---

//this file is part of eMule
//Copyright (C)2002 Merkur ( merkur-@users.sourceforge.net / http://www.emule-project.net )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

//SLAHAM: ADDED/MODIFIED DownloadClientsCtrl =>

#include "stdafx.h"
#include "emule.h"
#include "emuledlg.h"
#include "DownloadClientsCtrl.h"
#include "ClientDetailDialog.h"
#include "MemDC.h"
#include "MenuCmds.h"
#include "FriendList.h"
#include "TransferWnd.h"
#include "ChatWnd.h"
#include "UpDownClient.h"
#include "UploadQueue.h"
#include "ClientCredits.h"
#include "PartFile.h"
#include "Kademlia/Kademlia/Kademlia.h"
#include "SharedFileList.h"

#include "IP2Country.h"//EastShare - added by AndCycle, IP to Country

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif


IMPLEMENT_DYNAMIC(CDownloadClientsCtrl, CMuleListCtrl)

BEGIN_MESSAGE_MAP(CDownloadClientsCtrl, CMuleListCtrl)
	ON_WM_CONTEXTMENU()
	ON_WM_SYSCOLORCHANGE()
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclkDownloadClientlist)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetDispInfo)
END_MESSAGE_MAP()

CDownloadClientsCtrl::CDownloadClientsCtrl()
	: CListCtrlItemWalk(this)
{
	SetGeneralPurposeFind(true, false);
}

void CDownloadClientsCtrl::Init()
{
	SetName(_T("DownloadClientsCtrl"));
	CImageList ilDummyImageList; //dummy list for getting the proper height of listview entries
	ilDummyImageList.Create(1, theApp.GetSmallSytemIconSize().cy,theApp.m_iDfltImageListColorFlags|ILC_MASK, 1, 1); 
	SetImageList(&ilDummyImageList, LVSIL_SMALL);
	ASSERT( (GetStyle() & LVS_SHAREIMAGELISTS) == 0 );
	ilDummyImageList.Detach();

	SetExtendedStyle(LVS_EX_FULLROWSELECT);

	InsertColumn(0,	GetResString(IDS_QL_USERNAME), LVCFMT_LEFT, 165);
	InsertColumn(1,	GetResString(IDS_CD_CSOFT), LVCFMT_LEFT, 90); 
	InsertColumn(2,	GetResString(IDS_FILE), LVCFMT_LEFT, 235);
	InsertColumn(3,	GetResString(IDS_DL_SPEED), LVCFMT_LEFT, 65);
	InsertColumn(4, GetResString(IDS_AVAILABLEPARTS), LVCFMT_LEFT, 150);
	InsertColumn(5,	GetResString(IDS_CL_TRANSFDOWN), LVCFMT_LEFT, 115);
	InsertColumn(6,	GetResString(IDS_CL_TRANSFUP), LVCFMT_LEFT, 115);
	InsertColumn(7,	GetResString(IDS_META_SRCTYPE), LVCFMT_LEFT, 60);


	SetAllIcons();
	Localize();
	LoadSettings();

	// Barry - Use preferred sort order from preferences
	SetSortArrow();
}

CDownloadClientsCtrl::~CDownloadClientsCtrl()
{
}

void CDownloadClientsCtrl::SetAllIcons() 
{
	m_ImageList.DeleteImageList();
	m_ImageList.Create(16,16,theApp.m_iDfltImageListColorFlags|ILC_MASK,0,1);
	m_ImageList.SetBkColor(CLR_NONE);
	m_ImageList.Add(CTempIconLoader(_T("ClientEDonkey")));
	m_ImageList.Add(CTempIconLoader(_T("ClientCompatible")));
	m_ImageList.Add(CTempIconLoader(_T("ClientEDonkeyPlus")));
	m_ImageList.Add(CTempIconLoader(_T("ClientCompatiblePlus")));
	m_ImageList.Add(CTempIconLoader(_T("Friend")));
	m_ImageList.Add(CTempIconLoader(_T("ClientMLDonkey")));
	m_ImageList.Add(CTempIconLoader(_T("ClientMLDonkeyPlus")));
	m_ImageList.Add(CTempIconLoader(_T("ClientEDonkeyHybrid")));
	m_ImageList.Add(CTempIconLoader(_T("ClientEDonkeyHybridPlus")));
	m_ImageList.Add(CTempIconLoader(_T("ClientShareaza")));
	m_ImageList.Add(CTempIconLoader(_T("ClientShareazaPlus")));
	m_ImageList.Add(CTempIconLoader(_T("ClientAMule")));
	m_ImageList.Add(CTempIconLoader(_T("ClientAMulePlus")));
	m_ImageList.Add(CTempIconLoader(_T("ClientLPhant")));
	m_ImageList.Add(CTempIconLoader(_T("ClientLPhantPlus")));
	m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("ClientSecureOvl"))), 1);
	m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("OverlayObfu"))), 2);
	m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("OverlaySecureObfu"))), 3);
}

void CDownloadClientsCtrl::Localize()
{
	CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
	HDITEM hdi;
	hdi.mask = HDI_TEXT;
	CString strRes;

	strRes = GetResString(IDS_QL_USERNAME);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(0, &hdi);

	strRes = GetResString(IDS_CD_CSOFT);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(1, &hdi);

	strRes = GetResString(IDS_FILE);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(2, &hdi);

	strRes = GetResString(IDS_DL_SPEED);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(3, &hdi);
	
	strRes = GetResString(IDS_AVAILABLEPARTS);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(4, &hdi);

	strRes = GetResString(IDS_CL_TRANSFDOWN);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(5, &hdi);

	strRes = GetResString(IDS_CL_TRANSFUP);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(6, &hdi);

	strRes = GetResString(IDS_META_SRCTYPE);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(7, &hdi);
}

void CDownloadClientsCtrl::AddClient(CUpDownClient* client)
{
	if(!theApp.emuledlg->IsRunning())
		return;
       
	InsertItem(LVIF_TEXT|LVIF_PARAM, GetItemCount(), client->GetUserName(), 0, 0, 1, (LPARAM)client);
	RefreshClient(client);
	theApp.emuledlg->transferwnd->UpdateListCount(CTransferWnd::wnd2Downloading, GetItemCount()); 
}

void CDownloadClientsCtrl::RemoveClient(CUpDownClient* client)
{
	if(!theApp.emuledlg->IsRunning())
		return;

	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)client;
	int result = FindItem(&find);
	if (result != -1)
		DeleteItem(result);
	theApp.emuledlg->transferwnd->UpdateListCount(CTransferWnd::wnd2Downloading, GetItemCount()); 
}

void CDownloadClientsCtrl::RefreshClient(CUpDownClient* client)
{
	if( !theApp.emuledlg->IsRunning() )
		return;
	
	//MORPH START - SiRoB, Don't Refresh item if not needed 
	if( theApp.emuledlg->activewnd != theApp.emuledlg->transferwnd  || theApp.emuledlg->transferwnd->downloadclientsctrl.IsWindowVisible() == false ) 
		return; 
	//MORPH END   - SiRoB, Don't Refresh item if not needed 
	
	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)client;
	int result = FindItem(&find);
	if (result != -1)
		Update(result);
	return;
}

#define DLC_DT_TEXT (DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS)

void CDownloadClientsCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if (!theApp.emuledlg->IsRunning())
		return;
	if (!lpDrawItemStruct->itemData)
		return;
	CDC* odc = CDC::FromHandle(lpDrawItemStruct->hDC);
	BOOL bCtrlFocused = ((GetFocus() == this) || (GetStyle() & LVS_SHOWSELALWAYS));
	if (lpDrawItemStruct->itemState & ODS_SELECTED) {
		if (bCtrlFocused)
			odc->SetBkColor(m_crHighlight);
		else
			odc->SetBkColor(m_crNoHighlight);
	}
	else
		odc->SetBkColor(GetBkColor());
	const CUpDownClient* client = (CUpDownClient*)lpDrawItemStruct->itemData;
	CMemDC dc(odc, &lpDrawItemStruct->rcItem);
	CFont* pOldFont = dc.SelectObject(GetFont());
	CRect cur_rec(lpDrawItemStruct->rcItem);
	COLORREF crOldTextColor = dc.SetTextColor((lpDrawItemStruct->itemState & ODS_SELECTED) ? m_crHighlightText : m_crWindowText);
    if(client->GetSlotNumber() > CGlobalVariable::uploadqueue->GetActiveUploadsCount()) {
        dc.SetTextColor(::GetSysColor(COLOR_GRAYTEXT));
    }

	int iOldBkMode;
	if (m_crWindowTextBk == CLR_NONE){
		DefWindowProc(WM_ERASEBKGND, (WPARAM)(HDC)dc, 0);
		iOldBkMode = dc.SetBkMode(TRANSPARENT);
	}
	else
		iOldBkMode = OPAQUE;

	CHeaderCtrl *pHeaderCtrl = GetHeaderCtrl();
	int iCount = pHeaderCtrl->GetItemCount();
	cur_rec.right = cur_rec.left - 8;
	cur_rec.left += 4;
	CString Sbuffer;	
	for(int iCurrent = 0; iCurrent < iCount; iCurrent++){
		int iColumn = pHeaderCtrl->OrderToIndex(iCurrent);
		if( !IsColumnHidden(iColumn) ){
			cur_rec.right += GetColumnWidth(iColumn);
			switch(iColumn){
				case 0:{
					uint8 image;
					if (client->credits != NULL){
						if (client->IsFriend())
							image = 4;
						else if (client->GetClientSoft() == SO_EDONKEYHYBRID){
							if (client->credits && client->credits->GetScoreRatio(client->GetIP()) > 1)
								image = 8;
							else
								image = 7;
						}
						else if (client->GetClientSoft() == SO_MLDONKEY){
							if (client->credits && client->credits->GetScoreRatio(client->GetIP()) > 1)
								image = 6;
							else
								image = 5;
						}
						else if (client->GetClientSoft() == SO_SHAREAZA){
							if(client->credits && client->credits->GetScoreRatio(client->GetIP()) > 1)
								image = 10;
							else
								image = 9;
						}
						else if (client->GetClientSoft() == SO_AMULE){
							if(client->credits && client->credits->GetScoreRatio(client->GetIP()) > 1)
								image = 12;
							else
								image = 11;
						}
						else if (client->GetClientSoft() == SO_LPHANT){
							if(client->credits&&client->credits->GetScoreRatio(client->GetIP()) > 1)
								image = 14;
							else
								image = 13;
						}
						else if (client->ExtProtocolAvailable()){
							if(client->credits && client->credits->GetScoreRatio(client->GetIP()) > 1)
								image = 3;
							else
								image = 1;
						}
						else{
							if (client->credits && client->credits->GetScoreRatio(client->GetIP()) > 1)
								image = 2;
							else
								image = 0;
						}
					}
					else
						image = 0;

					uint32 nOverlayImage = 0;
					if ((client->Credits() && client->Credits()->GetCurrentIdentState(client->GetIP()) == IS_IDENTIFIED))
						nOverlayImage |= 1;
					if (client->IsObfuscatedConnectionEstablished())
						nOverlayImage |= 2;
					POINT point = {cur_rec.left, cur_rec.top+1};
					m_ImageList.Draw(dc,image, point, ILD_NORMAL | ((client->Credits() && client->Credits()->GetCurrentIdentState(client->GetIP()) == IS_IDENTIFIED) ? INDEXTOOVERLAYMASK(1) : 0));
					Sbuffer = client->GetUserName();

					//EastShare Start - added by AndCycle, IP to Country
 					CString tempStr;
 						tempStr.Format(_T("%s%s"), client->GetCountryName(), Sbuffer);
 					Sbuffer = tempStr;
 
 					if(CGlobalVariable::ip2country->ShowCountryFlag()){
 							cur_rec.left+=20;
 							POINT point2= {cur_rec.left,cur_rec.top+1};
 						CGlobalVariable::ip2country->GetFlagImageList()->DrawIndirect(dc, client->GetCountryFlagIndex(), point2, CSize(18,16), CPoint(0,0), ILD_NORMAL);
 					}
 					//EastShare End - added by AndCycle, IP to Country

					cur_rec.left +=20;
					dc.DrawText(Sbuffer,Sbuffer.GetLength(),&cur_rec,DLC_DT_TEXT);
					cur_rec.left -=20;

 					//EastShare Start - added by AndCycle, IP to Country
 					if(CGlobalVariable::ip2country->ShowCountryFlag()){
 						cur_rec.left-=20;
 					}
 					//EastShare End - added by AndCycle, IP to Country

					break;
					}
				case 1:
					Sbuffer.Format(_T("%s"), client->GetClientSoftVer());
					break;
				case 2:
					Sbuffer.Format(_T("%s"), client->GetRequestFile()->GetFileName());
					break;
				case 3:
					Sbuffer=CastItoXBytes( (float)client->GetDownloadDatarate() , false, true);
					dc.DrawText(Sbuffer,Sbuffer.GetLength(),&cur_rec, DLC_DT_TEXT | DT_RIGHT);
					break;
				case 4:
					cur_rec.bottom--;
					cur_rec.top++;
					client->DrawStatusBar(dc, &cur_rec, false, thePrefs.UseFlatBar());
					cur_rec.bottom++;
					cur_rec.top--;
					break;	
				case 5:
					if(client->Credits() && client->GetSessionDown() < client->credits->GetDownloadedTotal())
						Sbuffer.Format(_T("%s (%s)"), CastItoXBytes(client->GetSessionDown()), CastItoXBytes(client->credits->GetDownloadedTotal()));
					else
						Sbuffer.Format(_T("%s"), CastItoXBytes(client->GetSessionDown()));
					break;
				case 6:
					if(client->Credits() && client->GetSessionUp() < client->credits->GetUploadedTotal())
						Sbuffer.Format(_T("%s (%s)"), CastItoXBytes(client->GetSessionUp()), CastItoXBytes(client->credits->GetUploadedTotal()));
					else
						Sbuffer.Format(_T("%s"), CastItoXBytes(client->GetSessionUp()));
					break;
				case 7:
					switch(client->GetSourceFrom()){
					case SF_SERVER:
						Sbuffer = _T("eD2K Server");
						break;
					case SF_KADEMLIA:
						Sbuffer = GetResString(IDS_KADEMLIA);
						break;
					case SF_SOURCE_EXCHANGE:
						Sbuffer = GetResString(IDS_SE);
						break;
					case SF_PASSIVE:
						Sbuffer = GetResString(IDS_PASSIVE);
						break;
					case SF_LINK:
						Sbuffer = GetResString(IDS_SW_LINK);
						break;
                    case SF_LAN:
                        Sbuffer = GetResString(IDS_LAN);
                        break;
					case SF_HTTP:
						Sbuffer = _T("http");
						break;
					case SF_FTP:
						Sbuffer = _T("ftp");
						break;
					default:
						Sbuffer = GetResString(IDS_UNKNOWN);
						break;
					}
					break;
				}
				if( iColumn != 4 && iColumn != 0 && iColumn != 3 && iColumn != 11)
					dc.DrawText(Sbuffer,Sbuffer.GetLength(),&cur_rec,DLC_DT_TEXT);
				cur_rec.left += GetColumnWidth(iColumn);
		}
	}

	//draw rectangle around selected item(s)
	if (lpDrawItemStruct->itemState & ODS_SELECTED)
	{
		RECT outline_rec = lpDrawItemStruct->rcItem;

		outline_rec.top--;
		outline_rec.bottom++;
		dc.FrameRect(&outline_rec, &CBrush(GetBkColor()));
		outline_rec.top++;
		outline_rec.bottom--;
		outline_rec.left++;
		outline_rec.right--;

		if (bCtrlFocused)
			dc.FrameRect(&outline_rec, &CBrush(m_crFocusLine));
		else
			dc.FrameRect(&outline_rec, &CBrush(m_crNoFocusLine));
	}

	if (m_crWindowTextBk == CLR_NONE)
		dc.SetBkMode(iOldBkMode);

	dc.SelectObject(pOldFont);
	dc.SetTextColor(crOldTextColor);
}

void CDownloadClientsCtrl::OnColumnClick( NMHDR* pNMHDR, LRESULT* pResult){

	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// Barry - Store sort order in preferences
	// Determine ascending based on whether already sorted on this column
	bool sortAscending = (GetSortItem()!= pNMListView->iSubItem) ? (pNMListView->iSubItem == 0) : !GetSortAscending();	

	// Sort table
	UpdateSortHistory(pNMListView->iSubItem + (sortAscending ? 0:100), 100);
	SetSortArrow(pNMListView->iSubItem, sortAscending);
	SortItems(SortProc, pNMListView->iSubItem + (sortAscending ? 0:100));

	*pResult = 0;
}

BOOL CDownloadClientsCtrl::OnCommand(WPARAM wParam, LPARAM /*lParam*/)
{
	wParam = LOWORD(wParam);

	switch (wParam)
	{
		case MP_FIND:
			OnFindStart();
			return TRUE;
	}

	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1){
		CUpDownClient* client = (CUpDownClient*)GetItemData(iSel);
		switch (wParam){
			case MP_SHOWLIST:
				client->RequestSharedFileList();
				break;
#if _ENABLE_NOUSE
			case MP_MESSAGE:
				theApp.emuledlg->chatwnd->StartSession(client);
				break;
			case MP_ADDFRIEND:
				if (theApp.friendlist->AddFriend(client))
					Update(iSel);
				break;
#endif
			case MP_DETAIL:
			case MPG_ALTENTER:
			case IDA_ENTER:
			{
				CClientDetailDialog dialog(client, this);
				dialog.DoModal();
				break;
			}
			case MP_BOOT:
				if (client->GetKadPort())
					Kademlia::CKademlia::Bootstrap(ntohl(client->GetIP()), client->GetKadPort(), (client->GetKadVersion() > 1));
				break;
		}
	}
	return true;
}

int CDownloadClientsCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort){
	const CUpDownClient* item1 = (CUpDownClient*)lParam1;
	const CUpDownClient* item2 = (CUpDownClient*)lParam2;
	int iResult=0;
	switch(lParamSort){
		case 0: 
		case 100:
			if(item1->GetUserName() && item2->GetUserName())
				iResult=CompareLocaleStringNoCase(item1->GetUserName(), item2->GetUserName());
			else if(item1->GetUserName())
				iResult=1;
			else
				iResult=-1;
			break;
		case 1:
	    case 101:
		    if (item1->GetClientSoft() == item2->GetClientSoft())
			    iResult=item2->GetVersion() - item1->GetVersion();
		    else 
				iResult=item1->GetClientSoft() - item2->GetClientSoft();
			break;
		case 2:
		case 102:
		{
			CKnownFile* file1 = item1->GetRequestFile();
			CKnownFile* file2 = item2->GetRequestFile();
			if( (file1 != NULL) && (file2 != NULL))
				iResult=CompareLocaleStringNoCase(file1->GetFileName(), file2->GetFileName());
			else if( file1 == NULL )
				iResult=1;
			else
				iResult=-1;
			break;
		}
		case 3:
		case 103:
			iResult=CompareUnsigned(item1->GetDownloadDatarate(), item2->GetDownloadDatarate());
			break;
		case 4:
		case 104: 
			iResult=CompareUnsigned(item1->GetPartCount(), item2->GetPartCount());
			break;
		case 5:
		case 105:
			iResult=CompareUnsigned(item1->GetSessionDown(), item2->GetSessionDown());
			break;
		case 6:
		case 106:
			iResult=CompareUnsigned(item1->GetSessionUp(), item2->GetSessionUp());
			break;
		case 7: 
		case 107: 
			iResult=CompareUnsigned(item1->GetSourceFrom(), item2->GetSourceFrom());
			break;
		default:
			iResult=0;
			break;
	}

	if (lParamSort>=100)
    iResult*=-1;

	int dwNextSort;
	//call secondary sortorder, if this one results in equal
	//(Note: yes I know this call is evil OO wise, but better than changing a lot more code, while we have only one instance anyway - might be fixed later)
	if (iResult == 0 && (dwNextSort = theApp.emuledlg->transferwnd->downloadclientsctrl.GetNextSortOrder(lParamSort)) != (-1)){
		iResult= SortProc(lParam1, lParam2, dwNextSort);
	}

	return iResult;
}

//SLAHAM: ADDED [TPT] - New Menu Styles =>
void CDownloadClientsCtrl::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	const CUpDownClient* client = (iSel != -1) ? (CUpDownClient*)GetItemData(iSel) : NULL;

	CTitleMenu ClientMenu;
	ClientMenu.CreatePopupMenu();
	ClientMenu.AddMenuTitle(GetResString(IDS_CLIENTS), true);
	ClientMenu.AppendMenu(MF_STRING | (client ? MF_ENABLED : MF_GRAYED), MP_DETAIL, GetResString(IDS_SHOWDETAILS), _T("CLIENTDETAILS"));
	ClientMenu.SetDefaultItem(MP_DETAIL);
	//ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && !client->IsFriend()) ? MF_ENABLED : MF_GRAYED), MP_ADDFRIEND, GetResString(IDS_ADDFRIEND), _T("ADDFRIEND"));
	//ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient()) ? MF_ENABLED : MF_GRAYED), MP_MESSAGE, GetResString(IDS_SEND_MSG), _T("SENDMESSAGE"));
	//ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->GetViewSharedFilesSupport()) ? MF_ENABLED : MF_GRAYED), MP_SHOWLIST, GetResString(IDS_VIEWFILES), _T("VIEWFILES"));
	if (Kademlia::CKademlia::IsRunning() && !Kademlia::CKademlia::IsConnected())
		ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->GetKadPort()!=0) ? MF_ENABLED : MF_GRAYED), MP_BOOT, GetResString(IDS_BOOTSTRAP));
	ClientMenu.AppendMenu(MF_STRING | (GetItemCount() > 0 ? MF_ENABLED : MF_GRAYED), MP_FIND, GetResString(IDS_FIND), _T("Search"));
	GetPopupMenuPos(*this, point);
	ClientMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);
}

void CDownloadClientsCtrl::ShowSelectedUserDetails(){
	POINT point;
	::GetCursorPos(&point);
	CPoint p = point; 
    ScreenToClient(&p); 
    int it = HitTest(p); 
    if (it == -1)
		return;

	SetItemState(-1, 0, LVIS_SELECTED);
	SetItemState(it, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	SetSelectionMark(it);   // display selection mark correctly!

	CUpDownClient* client = (CUpDownClient*)GetItemData(GetSelectionMark());
	if (client){
		CClientDetailDialog dialog(client, this);
		dialog.DoModal();
	}
}

void CDownloadClientsCtrl::OnNMDblclkDownloadClientlist(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1) {
		CUpDownClient* client = (CUpDownClient*)GetItemData(iSel);
		if (client){
			CClientDetailDialog dialog(client, this);
			dialog.DoModal();
		}
	}
	*pResult = 0;
}

void CDownloadClientsCtrl::OnSysColorChange()
{
	CMuleListCtrl::OnSysColorChange();
	SetAllIcons();
}

void CDownloadClientsCtrl::OnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);

	if (theApp.emuledlg->IsRunning()){
		// Although we have an owner drawn listview control we store the text for the primary item in the listview, to be
		// capable of quick searching those items via the keyboard. Because our listview items may change their contents,
		// we do this via a text callback function. The listview control will send us the LVN_DISPINFO notification if
		// it needs to know the contents of the primary item.
		//
		// But, the listview control sends this notification all the time, even if we do not search for an item. At least
		// this notification is only sent for the visible items and not for all items in the list. Though, because this
		// function is invoked *very* often, no *NOT* put any time consuming code here in.

		if (pDispInfo->item.mask & LVIF_TEXT){
			const CUpDownClient* pClient = reinterpret_cast<CUpDownClient*>(pDispInfo->item.lParam);
			if (pClient != NULL){
				switch (pDispInfo->item.iSubItem){
					case 0:
						if (pClient->GetUserName() != NULL && pDispInfo->item.cchTextMax > 0){
							_tcsncpy(pDispInfo->item.pszText, pClient->GetUserName(), pDispInfo->item.cchTextMax);
							pDispInfo->item.pszText[pDispInfo->item.cchTextMax-1] = _T('\0');
						}
						break;
					default:
						// shouldn't happen
						pDispInfo->item.pszText[0] = _T('\0');
						break;
				}
			}
		}
	}
	*pResult = 0;
}
