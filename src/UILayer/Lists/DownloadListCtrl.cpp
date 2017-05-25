/* 
 * $Id: DownloadListCtrl.cpp 7701 2008-10-15 07:34:41Z huby $
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
#include "DownloadListCtrl.h"
#include "otherfunctions.h" 
#include "updownclient.h"
#include "MenuCmds.h"
#include "ClientDetailDialog.h"
#include "FileDetailDialog.h"
#include "commentdialoglst.h"
#include "MetaDataDlg.h"
#include "InputBox.h"
#include "KademliaWnd.h"
#include "emuledlg.h"
#include "DownloadQueue.h"
#include "FriendList.h"
#include "PartFile.h"
#include "ClientCredits.h"
#include "MemDC.h"
#include "ChatWnd.h"
#include "TransferWnd.h"
#include "Kademlia/Kademlia/Kademlia.h"
#include "Kademlia/Kademlia/Prefs.h"
#include "Kademlia/net/KademliaUDPListener.h"
#include "WebServices.h"
#include "Preview.h"
#include "StringConversion.h"
#include "AddSourceDlg.h"
#include "ToolTipCtrlX.h"
#include "CollectionViewDialog.h"
#include "SearchDlg.h"
#include "SharedFileList.h"
#include "WndMgr.h"
#include "UserMsgs.h"
#include "Ini2.h"
#include "UpdateInfo.h"

#include "CmdFuncs.h"
#include "cif.h"

#include "UpdateInfo.h"
#include "UIMessage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "IP2Country.h"//EastShare - added by AndCycle, IP to Country
#include ".\downloadlistctrl.h"

// CDownloadListCtrl

#define DLC_DT_TEXT (DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS)
#define DLC_BARUPDATE 512

#define	FILE_ITEM_MARGIN_X	4
#define RATING_ICON_WIDTH	16

//Added by thilon on 2006.09.01, Comment
#ifndef IDC_HAND
	#define IDC_HAND            MAKEINTRESOURCE(32649)
#endif

IMPLEMENT_DYNAMIC(CtrlItem_Struct, CObject)

IMPLEMENT_DYNAMIC(CDownloadListCtrl, CMuleListCtrl)

BEGIN_MESSAGE_MAP(CDownloadListCtrl, CMuleListCtrl)
	ON_WM_CONTEXTMENU()
	ON_WM_SYSCOLORCHANGE()
	ON_NOTIFY_REFLECT(LVN_ITEMACTIVATE, OnItemActivate)
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, OnListModified)
	ON_NOTIFY_REFLECT(LVN_INSERTITEM, OnListModified)
	ON_NOTIFY_REFLECT(LVN_DELETEITEM, OnListModified)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclkDownloadlist)
	ON_NOTIFY_REFLECT(NM_CLICK, OnCommentClick)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetDispInfo)
	ON_NOTIFY_REFLECT(LVN_GETINFOTIP, OnLvnGetInfoTip)
	ON_WM_MOUSEMOVE()
	
	ON_MESSAGE(WM_UPDATE_GUI_START, OnUpdateGUIStart)
	ON_MESSAGE(WM_UPDATE_GUI_STOP, OnUpdateGUIStop)

	ON_MESSAGE(UM_RECREATE_PARTFILE, OnReCreatePartFile)	//Added by thilon on 2008.04.29
ON_WM_DRAWITEM()
ON_WM_MEASUREITEM()
END_MESSAGE_MAP()

CDownloadListCtrl::CDownloadListCtrl()
	: CDownloadListListCtrlItemWalk(this)
{
	m_tooltip = new CToolTipCtrlX;
	SetGeneralPurposeFind(true, false);

	m_ToolBarUI.iFilesToCancel = 0;
	m_ToolBarUI.iFilesToPause = 0;
	m_ToolBarUI.iFilesToResume = 0;
	m_ToolBarUI.iFilesToStop = 0;

	m_MenuXP = NULL;
	m_pDialog = NULL;
}

CDownloadListCtrl::~CDownloadListCtrl()
{
	try {
	if (m_PrioMenu)
		VERIFY( m_PrioMenu.DestroyMenu() );
    if (m_SourcesMenu)
		VERIFY( m_SourcesMenu.DestroyMenu() );
	if (m_FileMenu)
		VERIFY( m_FileMenu.DestroyMenu() );

	while (m_ListItems.empty() == false) {
		delete m_ListItems.begin()->second; // second = CtrlItem_Struct*
		m_ListItems.erase(m_ListItems.begin());
	}
	delete m_tooltip;

	if(m_MenuXP)
	{
		delete m_MenuXP;
	}

	theWndMgr.SetWndHandle(CWndMgr::WI_DOWNLOADING_LISTCTRL, NULL);	//Added by thilon on 2008.04.29
	}
	catch(...)
	{
	}
}

void CDownloadListCtrl::Init()
{
	theWndMgr.SetWndHandle(CWndMgr::WI_DOWNLOADING_LISTCTRL, m_hWnd);		//Added by thilon on 2008.04.29

	SetName(_T("DownloadListCtrl"));
	CImageList ilDummyImageList; //dummy list for getting the proper height of listview entries
	ilDummyImageList.Create(1, theApp.GetSmallSytemIconSize().cy, theApp.m_iDfltImageListColorFlags|ILC_MASK, 1, 1); 
	SetImageList(&ilDummyImageList, LVSIL_SMALL);
	ASSERT( (GetStyle() & LVS_SHAREIMAGELISTS) == 0 );
	ilDummyImageList.Detach();

	SetStyle();
	ModifyStyle(LVS_SINGLESEL,0);
	
	CToolTipCtrl* tooltip = GetToolTips();
	if (tooltip){
		m_tooltip->SubclassWindow(*tooltip);
		tooltip->ModifyStyle(0, TTS_NOPREFIX);
		tooltip->SetDelayTime(TTDT_AUTOPOP, 20000);
		tooltip->SetDelayTime(TTDT_INITIAL, thePrefs.GetToolTipDelay()*1000);
	}

	InsertColumn(0,GetResString(IDS_DL_FILENAME),LVCFMT_LEFT, 260);
	InsertColumn(1,GetResString(IDS_DL_COMMENT),LVCFMT_CENTER, 60);		//Added by thilon on 2006.08.29, 评论
	InsertColumn(2,GetResString(IDS_DL_SIZE),LVCFMT_LEFT, 60);
	InsertColumn(3,GetResString(IDS_DL_TRANSF),LVCFMT_LEFT, 65);
	InsertColumn(4,GetResString(IDS_DL_TRANSFCOMPL),LVCFMT_LEFT, 65);
	InsertColumn(5,GetResString(IDS_DL_SPEED),LVCFMT_LEFT, 65);
	InsertColumn(6,GetResString(IDS_DL_PROGRESS),LVCFMT_LEFT, 170);
	InsertColumn(7,GetResString(IDS_DL_SOURCES),LVCFMT_LEFT, 50);
	InsertColumn(8,GetResString(IDS_PRIORITY),LVCFMT_LEFT, 55);
	InsertColumn(9,GetResString(IDS_STATUS),LVCFMT_LEFT, 70);
	InsertColumn(10,GetResString(IDS_DL_REMAINS),LVCFMT_LEFT, 110);
	/*InsertColumn(11,GetResString(IDS_DL_FILENAME),LVCFMT_LEFT, 60);*/
	CString lsctitle=GetResString(IDS_LASTSEENCOMPL);
	lsctitle.Remove(_T(':'));
	InsertColumn(11, lsctitle,LVCFMT_LEFT, 220);
	lsctitle=GetResString(IDS_FD_LASTCHANGE);
	lsctitle.Remove(_T(':'));
	InsertColumn(12, lsctitle,LVCFMT_LEFT, 220);
	//InsertColumn(13, GetResString(IDS_CAT) ,LVCFMT_LEFT, 100);

	SetAllIcons();
	Localize();
	LoadSettings();
	curTab=0;

	if (thePrefs.GetShowActiveDownloadsBold())
	{
		CFont* pFont = GetFont();
		LOGFONT lfFont = {0};
		pFont->GetLogFont(&lfFont);
		lfFont.lfWeight = FW_BOLD;
		m_fontBold.CreateFontIndirect(&lfFont);
	}

	// Barry - Use preferred sort order from preferences
	m_bRemainSort=thePrefs.TransferlistRemainSortStyle();

	uint8 adder=0;
	if (GetSortItem()!=9 || !m_bRemainSort)
		SetSortArrow();
	else {
		SetSortArrow(GetSortItem(), GetSortAscending()?arrowDoubleUp : arrowDoubleDown);
		adder=81;
	}
	
	SortItems(SortProc, GetSortItem() + (GetSortAscending()? 0:100) + adder);
}

void CDownloadListCtrl::OnSysColorChange()
{
	CMuleListCtrl::OnSysColorChange();
	SetAllIcons();
	//CreateMenues();
}

void CDownloadListCtrl::SetAllIcons()
{
	m_ImageList.DeleteImageList();
	m_ImageList.Create(16,16,theApp.m_iDfltImageListColorFlags|ILC_MASK,0,1);
	m_ImageList.SetBkColor(CLR_NONE);
	m_ImageList.Add(CTempIconLoader(_T("SrcDownloading")));
	m_ImageList.Add(CTempIconLoader(_T("SrcOnQueue")));
	m_ImageList.Add(CTempIconLoader(_T("SrcConnecting")));
	m_ImageList.Add(CTempIconLoader(_T("SrcNNPQF")));
	m_ImageList.Add(CTempIconLoader(_T("SrcUnknown")));
	m_ImageList.Add(CTempIconLoader(_T("ClientCompatible")));
	m_ImageList.Add(CTempIconLoader(_T("Friend")));
	m_ImageList.Add(CTempIconLoader(_T("ClientEDonkey")));
	m_ImageList.Add(CTempIconLoader(_T("ClientMLDonkey")));
	m_ImageList.Add(CTempIconLoader(_T("ClientEDonkeyHybrid")));
	m_ImageList.Add(CTempIconLoader(_T("ClientShareaza")));
	m_ImageList.Add(CTempIconLoader(_T("Server")));
	m_ImageList.Add(CTempIconLoader(_T("ClientAMule")));
	m_ImageList.Add(CTempIconLoader(_T("ClientLPhant")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_NotRated")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_Fake")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_Poor")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_Fair")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_Good")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_Excellent")));
	m_ImageList.Add(CTempIconLoader(_T("Collection_Search"))); // rating for comments are searched on kad
	m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("ClientSecureOvl"))), 1);
	m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("OverlayObfu"))), 2);
	m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("OverlaySecureObfu"))), 3);
}

void CDownloadListCtrl::Localize()
{
	CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
	HDITEM hdi;
	hdi.mask = HDI_TEXT;
	CString strRes;

	strRes = GetResString(IDS_DL_FILENAME);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(0, &hdi);

	strRes = GetResString(IDS_DL_COMMENT);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(1, &hdi);

	strRes = GetResString(IDS_DL_SIZE);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(2, &hdi);

	strRes = GetResString(IDS_DL_TRANSF);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(3, &hdi);

	strRes = GetResString(IDS_DL_TRANSFCOMPL);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(4, &hdi);

	strRes = GetResString(IDS_DL_SPEED);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(5, &hdi);

	strRes = GetResString(IDS_DL_PROGRESS);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(6, &hdi);

	strRes = GetResString(IDS_DL_SOURCES);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(7, &hdi);

	strRes = GetResString(IDS_PRIORITY);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(8, &hdi);

	strRes = GetResString(IDS_STATUS);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(9, &hdi);

	strRes = GetResString(IDS_DL_REMAINS);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(10, &hdi);

	strRes = GetResString(IDS_LASTSEENCOMPL);
	strRes.Remove(_T(':'));
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(11, &hdi);

	strRes = GetResString(IDS_FD_LASTCHANGE);
	strRes.Remove(_T(':'));
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(12, &hdi);

	//strRes = GetResString(IDS_CAT);
	//hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	//pHeaderCtrl->SetItem(13, &hdi);

	//CreateMenues();
	ShowFilesCount();
}

void CDownloadListCtrl::AddFile(CPartFile* toadd)
{
	// Create new Item
	CtrlItem_Struct* newitem = new CtrlItem_Struct;
	int itemnr = GetItemCount();
	newitem->owner = NULL;
	newitem->type = FILE_TYPE;
	newitem->value = toadd;
	newitem->parent = NULL;
	newitem->dwUpdated = 0; 

	// The same file shall be added only once
	ASSERT(m_ListItems.find(toadd) == m_ListItems.end());
	m_ListItems.insert(ListItemsPair(toadd, newitem));

	if (toadd->CheckShowItemInGivenCat(curTab))
		InsertItem(LVIF_PARAM|LVIF_TEXT,itemnr,LPSTR_TEXTCALLBACK,0,0,0,(LPARAM)newitem);

	ShowFilesCount();
}

void CDownloadListCtrl::AddSource(CPartFile* owner, CUpDownClient* source, bool notavailable)
{
	// Create new Item
	CtrlItem_Struct* newitem = new CtrlItem_Struct;
	newitem->owner = owner;
	newitem->type = (notavailable) ? UNAVAILABLE_SOURCE : AVAILABLE_SOURCE;
	newitem->value = source;
	newitem->dwUpdated = 0; 

	// Update cross link to the owner
	ListItems::const_iterator ownerIt = m_ListItems.find(owner);
	ASSERT(ownerIt != m_ListItems.end());
	CtrlItem_Struct* ownerItem = ownerIt->second;
	ASSERT(ownerItem->value == owner);
	newitem->parent = ownerItem;

	// The same source could be added a few time but only one time per file 
	{
		// Update the other instances of this source
		bool bFound = false;
		std::pair<ListItems::const_iterator, ListItems::const_iterator> rangeIt = m_ListItems.equal_range(source);
		for(ListItems::const_iterator it = rangeIt.first; it != rangeIt.second; it++){
			CtrlItem_Struct* cur_item = it->second;

			// Check if this source has been already added to this file => to be sure
			if(cur_item->owner == owner){
				// Update this instance with its new setting
				cur_item->type = newitem->type;
				cur_item->dwUpdated = 0;
				bFound = true;
			}
			else if(notavailable == false){
				// The state 'Available' is exclusive
				cur_item->type = UNAVAILABLE_SOURCE;
				cur_item->dwUpdated = 0;
			}
		}

		if(bFound == true){
			delete newitem; 
			return;
		}
	}
	m_ListItems.insert(ListItemsPair(source, newitem));

	if (owner->srcarevisible) {
		// find parent from the CListCtrl to add source
		LVFINDINFO find;
		find.flags = LVFI_PARAM;
		find.lParam = (LPARAM)ownerItem;
		int result = FindItem(&find);
		if (result != -1)
			InsertItem(LVIF_PARAM|LVIF_TEXT,result+1,LPSTR_TEXTCALLBACK,0,0,0,(LPARAM)newitem);
	}
    
}

void CDownloadListCtrl::RemoveSource(CUpDownClient* source, CPartFile* owner)
{
	if (!theApp.emuledlg->IsRunning())
		return;

	// Retrieve all entries matching the source
	std::pair<ListItems::iterator, ListItems::iterator> rangeIt = m_ListItems.equal_range(source);
	for(ListItems::iterator it = rangeIt.first; it != rangeIt.second; ){
		CtrlItem_Struct* delItem  = it->second;
		if(owner == NULL || owner == delItem->owner){
			// Remove it from the m_ListItems			
			it = m_ListItems.erase(it);

			// Remove it from the CListCtrl
 			LVFINDINFO find;
			find.flags = LVFI_PARAM;
			find.lParam = (LPARAM)delItem;
			int result = FindItem(&find);
			if (result != -1)
				DeleteItem(result);

			// finally it could be delete
			delete delItem;
		}
		else{
			it++;
		}
	}
}

bool CDownloadListCtrl::RemoveFile(const CPartFile* toremove)
{
	__try
	{
		bool bResult = false;
		if (!theApp.emuledlg->IsRunning())
			return bResult;
		// Retrieve all entries matching the File or linked to the file
		// Remark: The 'asked another files' clients must be removed from here
		ASSERT(toremove != NULL);
		for(ListItems::iterator it = m_ListItems.begin(); it != m_ListItems.end(); ){
			CtrlItem_Struct* delItem = it->second;
			if(delItem->owner == toremove || delItem->value == (void*)toremove){
				// Remove it from the m_ListItems
				it = m_ListItems.erase(it);

				// Remove it from the CListCtrl
				LVFINDINFO find;
				find.flags = LVFI_PARAM;
				find.lParam = (LPARAM)delItem;
				int result = FindItem(&find);
				if (result != -1)
					DeleteItem(result);

				// finally it could be delete
				delete delItem;
				bResult = true;
			}
			else {
				it++;
			}
		}
		ShowFilesCount();
		return bResult;
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		return false;
	}
}

void CDownloadListCtrl::UpdateItem(void* toupdate)
{
	if (!theApp.emuledlg->IsRunning())
		return;

	// Retrieve all entries matching the source
	std::pair<ListItems::const_iterator, ListItems::const_iterator> rangeIt = m_ListItems.equal_range(toupdate);
	for(ListItems::const_iterator it = rangeIt.first; it != rangeIt.second; it++){
		CtrlItem_Struct* updateItem  = it->second;

		// Find entry in CListCtrl and update object
 		LVFINDINFO find;
		find.flags = LVFI_PARAM;
		find.lParam = (LPARAM)updateItem;
		int result = FindItem(&find);
		if (result != -1){
			updateItem->dwUpdated = 0;
			Update(result);
		}
	}
}

void CDownloadListCtrl::DrawFileItem(CDC *dc, int nColumn, LPCRECT lpRect, CtrlItem_Struct *lpCtrlItem)
{
	if(lpRect->left < lpRect->right)
	{
		CString buffer;
		/*const*/ CPartFile *lpPartFile = (CPartFile*)lpCtrlItem->value;

		//Added by thilon on 2006.09.01, comment Item
		LOGFONT lf = {0};
		CFont font;
		CFont* pOldFont;

		switch(nColumn)
		{
		case 0:{	// file name
			CRect rcDraw(lpRect);
			int iImage = theApp.GetFileTypeSystemImageIdx(lpPartFile->GetFileName());
			if (theApp.GetSystemImageList() != NULL)
				::ImageList_Draw(theApp.GetSystemImageList(), iImage, dc->GetSafeHdc(), rcDraw.left, rcDraw.top, ILD_NORMAL|ILD_TRANSPARENT);
			rcDraw.left += theApp.GetSmallSytemIconSize().cx;

			if (thePrefs.ShowRatingIndicator() && (lpPartFile->HasComment() || lpPartFile->HasRating() || lpPartFile->IsKadCommentSearchRunning())){
				m_ImageList.Draw(dc, lpPartFile->UserRating(true)+14, rcDraw.TopLeft(), ILD_NORMAL);
				rcDraw.left += RATING_ICON_WIDTH;
			}

			rcDraw.left += 3;
			dc->DrawText(lpPartFile->GetFileName(), lpPartFile->GetFileName().GetLength(), &rcDraw, DLC_DT_TEXT);
			break;
		}
		// Added by thilon on 2006.08.30, comment
	   case 1:		
			buffer = GetResString(IDS_DL_COMMENT);

			lf.lfHeight = 12;
			//lf.lfWeight = FW_BOLD;
			lf.lfUnderline = 1;
			lf.lfQuality = ANTIALIASED_QUALITY;
			_tcscpy(lf.lfFaceName, _T("宋体"));

			
			font.CreateFontIndirect(&lf);
			pOldFont = dc->SelectObject(&font);
			dc->SetTextColor(RGB(0,0,255));
			dc->DrawText(buffer,buffer.GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT | DT_CENTER);
			dc->SetTextColor(RGB(0,0,0));
			dc->SelectObject(pOldFont);
			break;

		case 2:		// size			
			if ( (lpPartFile->GetPartFileSizeStatus()==FS_KNOWN || lpPartFile->GetPartFileSizeStatus()==FS_KNOWN_FROM_ORIGINAL) && lpPartFile->GetFileSize()>(uint64)0 )
			{
				buffer = CastItoXBytes(lpPartFile->GetFileSize(), false, false);
			}
			else if( lpPartFile->GetPartFileSizeStatus() == FS_NOSIZE && lpPartFile->GetFileSize()>(uint64)0)
			{
				buffer = CastItoXBytes(lpPartFile->GetFileSize(), false, false);
			}
			else if( lpPartFile->GetPartFileSizeStatus() == FS_UNKNOWN && lpPartFile->GetFileSize() == (uint64)0)
			{
				buffer = CastItoXBytes(lpPartFile->GetFileSize(), false, false);
			}
			else
			{
				buffer = GetResString(IDS_UNKNOWN);  //
			}			
			dc->DrawText(buffer,buffer.GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT | DT_RIGHT);

			break;

		case 3:		// transferred
			buffer = CastItoXBytes(lpPartFile->GetTransferred(), false, false);
			dc->DrawText(buffer,buffer.GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT | DT_RIGHT);
			break;

		case 4:		// transferred complete
			buffer = CastItoXBytes(lpPartFile->GetCompletedSize(), false, false);
			dc->DrawText(buffer,buffer.GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT | DT_RIGHT);
			break;

		case 5:		// speed
			if (lpPartFile->GetTransferringSrcCount())
				buffer.Format(_T("%s"), CastItoXBytes(lpPartFile->GetDatarate(), false, true));
			dc->DrawText(buffer,buffer.GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT | DT_RIGHT);
			break;

		case 6:		// progress
			{
				CRect rcDraw(*lpRect);
				rcDraw.bottom--;
				rcDraw.top++;

				// added
				int iWidth = rcDraw.Width();
				int iHeight = rcDraw.Height();
				if (lpCtrlItem->status == (HBITMAP)NULL)
					VERIFY(lpCtrlItem->status.CreateBitmap(1, 1, 1, 8, NULL));
				CDC cdcStatus;
				HGDIOBJ hOldBitmap;
				cdcStatus.CreateCompatibleDC(dc);
				int cx = lpCtrlItem->status.GetBitmapDimension().cx; 
				DWORD dwTicks = GetTickCount();
				if(lpCtrlItem->dwUpdated + DLC_BARUPDATE < dwTicks || cx !=  iWidth || !lpCtrlItem->dwUpdated) {
					lpCtrlItem->status.DeleteObject(); 
					lpCtrlItem->status.CreateCompatibleBitmap(dc,  iWidth, iHeight); 
					lpCtrlItem->status.SetBitmapDimension(iWidth,  iHeight); 
					hOldBitmap = cdcStatus.SelectObject(lpCtrlItem->status); 

					RECT rec_status; 
					rec_status.left = 0; 
					rec_status.top = 0; 
					rec_status.bottom = iHeight; 
					rec_status.right = iWidth; 
					lpPartFile->DrawStatusBar(&cdcStatus,  &rec_status, thePrefs.UseFlatBar()); 

					lpCtrlItem->dwUpdated = dwTicks + (rand() % 128); 
				} else 
					hOldBitmap = cdcStatus.SelectObject(lpCtrlItem->status); 

				dc->BitBlt(rcDraw.left, rcDraw.top, iWidth, iHeight,  &cdcStatus, 0, 0, SRCCOPY); 
				cdcStatus.SelectObject(hOldBitmap);
				//added end

				if (thePrefs.GetUseDwlPercentage()) {
					// HoaX_69: BEGIN Display percent in progress bar
					COLORREF oldclr = dc->SetTextColor(RGB(255,255,255));
					int iOMode = dc->SetBkMode(TRANSPARENT);
					buffer.Format(_T("%.1f%%"), lpPartFile->GetPercentCompleted());
					dc->DrawText(buffer, buffer.GetLength(), &rcDraw, (DLC_DT_TEXT & ~DT_LEFT) | DT_CENTER);
					dc->SetBkMode(iOMode);
					dc->SetTextColor(oldclr);
					// HoaX_69: END
				}
			}
			break;

		case 7:		// sources
			{
				UINT sc = lpPartFile->GetSourceCount();
				UINT ncsc = lpPartFile->GetNotCurrentSourcesCount();
// ZZ:DownloadManager -->
                if(!(lpPartFile->GetStatus() == PS_PAUSED && sc == 0) && lpPartFile->GetStatus() != PS_COMPLETE) {
                    buffer.Format(_T("%i"), sc-ncsc);
				    if(ncsc>0) buffer.AppendFormat(_T("/%i"), sc);
                    if(thePrefs.IsExtControlsEnabled() && lpPartFile->GetSrcA4AFCount() > 0) buffer.AppendFormat(_T("+%i"), lpPartFile->GetSrcA4AFCount());
				    if(lpPartFile->GetTransferringSrcCount() > 0) buffer.AppendFormat(_T(" (%i)"), lpPartFile->GetTransferringSrcCount());
                } else {
                    buffer = _T("");
				}
// <-- ZZ:DownloadManager
				if (thePrefs.IsExtControlsEnabled() && lpPartFile->GetPrivateMaxSources() != 0)
					buffer.AppendFormat(_T(" [%i]"), lpPartFile->GetPrivateMaxSources());
				dc->DrawText(buffer,buffer.GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT | DT_RIGHT);
			}
			break;

		case 8:		// prio
			switch(lpPartFile->GetDownPriority()) {
			case PR_LOW:
				if( lpPartFile->IsAutoDownPriority() )
					dc->DrawText(GetResString(IDS_PRIOAUTOLOW),GetResString(IDS_PRIOAUTOLOW).GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
				else
					dc->DrawText(GetResString(IDS_PRIOLOW),GetResString(IDS_PRIOLOW).GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
				break;
			case PR_NORMAL:
				if( lpPartFile->IsAutoDownPriority() )
					dc->DrawText(GetResString(IDS_PRIOAUTONORMAL),GetResString(IDS_PRIOAUTONORMAL).GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
				else
					dc->DrawText(GetResString(IDS_PRIONORMAL),GetResString(IDS_PRIONORMAL).GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
				break;
			case PR_HIGH:
				if( lpPartFile->IsAutoDownPriority() )
					dc->DrawText(GetResString(IDS_PRIOAUTOHIGH),GetResString(IDS_PRIOAUTOHIGH).GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
				else
					dc->DrawText(GetResString(IDS_PRIOHIGH),GetResString(IDS_PRIOHIGH).GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
				break;
			}
			break;

		case 9:		// <<--9/21/02
			{
				buffer = lpPartFile->getPartfileStatus();
#ifdef _DEBUG
				CString sReqedBlockCount;
				sReqedBlockCount.Format( _T("-reqedCnt=%d"),lpPartFile->GetReqedBlockCount() );
				buffer += sReqedBlockCount;
#endif
				dc->DrawText(buffer,buffer.GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
				break;
			}

		case 10:		// remaining time & size
			{
				if (lpPartFile->GetStatus()!=PS_COMPLETING && lpPartFile->GetStatus()!=PS_COMPLETE ){
					// time 
					time_t restTime;
					if (!thePrefs.UseSimpleTimeRemainingComputation())
						restTime = lpPartFile->getTimeRemaining();
					else
						restTime = lpPartFile->getTimeRemainingSimple();

					buffer.Format(_T("%s (%s)"), CastSecondsToHM(restTime), CastItoXBytes((lpPartFile->GetFileSize() - lpPartFile->GetCompletedSize()), false, false));
				}
				dc->DrawText(buffer,buffer.GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
			}
			break;
		case 11: // last seen complete
			{
				CString tempbuffer;
				if (lpPartFile->m_nCompleteSourcesCountLo == 0)
				{
					tempbuffer.Format(_T("< %u"), lpPartFile->m_nCompleteSourcesCountHi);
				}
				else if (lpPartFile->m_nCompleteSourcesCountLo == lpPartFile->m_nCompleteSourcesCountHi)
				{
					tempbuffer.Format(_T("%u"), lpPartFile->m_nCompleteSourcesCountLo);
				}
				else
				{
					tempbuffer.Format(_T("%u - %u"), lpPartFile->m_nCompleteSourcesCountLo, lpPartFile->m_nCompleteSourcesCountHi);
				}
				if (lpPartFile->lastseencomplete==NULL)
					buffer.Format(_T("%s (%s)"),GetResString(IDS_NEVER),tempbuffer);
				else
					buffer.Format(_T("%s (%s)"),lpPartFile->lastseencomplete.Format( thePrefs.GetDateTimeFormat()),tempbuffer);
				dc->DrawText(buffer,buffer.GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
			}
			break;
		case 12: // last receive
			if (!IsColumnHidden(11)) {
				if(lpPartFile->GetFileDate()!=NULL && lpPartFile->GetCompletedSize() > (uint64)0)
					buffer=lpPartFile->GetCFileDate().Format( thePrefs.GetDateTimeFormat());
				else
					buffer.Format(_T("%s"),GetResString(IDS_NEVER));

				dc->DrawText(buffer,buffer.GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
			}
			break;
		case 13: // cat
			if (!IsColumnHidden(12)) {
				buffer=(lpPartFile->GetCategory()!=0)?
					thePrefs.GetCategory(lpPartFile->GetCategory())->strTitle:_T("");
				dc->DrawText(buffer,buffer.GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
			}
			break;
		}
	}
}

void CDownloadListCtrl::DrawSourceItem(CDC *dc, int nColumn, LPCRECT lpRect, CtrlItem_Struct *lpCtrlItem) {
	if(lpRect->left < lpRect->right) { 

		CString buffer;
		CUpDownClient *lpUpDownClient = (CUpDownClient*)lpCtrlItem->value;
		switch(nColumn) {

		case 0:		// icon, name, status
			{
				RECT cur_rec = *lpRect;
				POINT point = {cur_rec.left, cur_rec.top+1};
				if (lpCtrlItem->type == AVAILABLE_SOURCE){
					switch (lpUpDownClient->GetDownloadState()) {
					case DS_CONNECTING:
						m_ImageList.Draw(dc, 2, point, ILD_NORMAL);
						break;
					case DS_CONNECTED:
						m_ImageList.Draw(dc, 2, point, ILD_NORMAL);
						break;
					case DS_WAITCALLBACKKAD:
					case DS_WAITCALLBACK:
						m_ImageList.Draw(dc, 2, point, ILD_NORMAL);
						break;
					case DS_ONQUEUE:
						if(lpUpDownClient->IsRemoteQueueFull())
							m_ImageList.Draw(dc, 3, point, ILD_NORMAL);
						else
							m_ImageList.Draw(dc, 1, point, ILD_NORMAL);
						break;
					case DS_DOWNLOADING:
						m_ImageList.Draw(dc, 0, point, ILD_NORMAL);
						break;
					case DS_REQHASHSET:
						m_ImageList.Draw(dc, 0, point, ILD_NORMAL);
						break;
					case DS_NONEEDEDPARTS:
						m_ImageList.Draw(dc, 3, point, ILD_NORMAL);
						break;
					case DS_ERROR:
						m_ImageList.Draw(dc, 3, point, ILD_NORMAL);
						break;
					case DS_TOOMANYCONNS:
					case DS_TOOMANYCONNSKAD:
						m_ImageList.Draw(dc, 2, point, ILD_NORMAL);
						break;
					default:
						m_ImageList.Draw(dc, 4, point, ILD_NORMAL);
					}
				}
				else {

					m_ImageList.Draw(dc, 3, point, ILD_NORMAL);
				}
				cur_rec.left += 20;
				UINT uOvlImg = 0;
				if ((lpUpDownClient->Credits() && lpUpDownClient->Credits()->GetCurrentIdentState(lpUpDownClient->GetIP()) == IS_IDENTIFIED))
					uOvlImg |= 1;
				if (lpUpDownClient->IsObfuscatedConnectionEstablished())
					uOvlImg |= 2;

				POINT point2= {cur_rec.left,cur_rec.top+1};
				if (lpUpDownClient->IsFriend())
					m_ImageList.Draw(dc, 6, point2, ILD_NORMAL | INDEXTOOVERLAYMASK(uOvlImg));
				else if (lpUpDownClient->GetClientSoft() == SO_EDONKEYHYBRID)
					m_ImageList.Draw(dc, 9, point2, ILD_NORMAL | INDEXTOOVERLAYMASK(uOvlImg));
				else if (lpUpDownClient->GetClientSoft() == SO_MLDONKEY)
					m_ImageList.Draw(dc, 8, point2, ILD_NORMAL | INDEXTOOVERLAYMASK(uOvlImg));
				else if (lpUpDownClient->GetClientSoft() == SO_SHAREAZA)
					m_ImageList.Draw(dc, 10, point2, ILD_NORMAL | INDEXTOOVERLAYMASK(uOvlImg));
				else if (lpUpDownClient->GetClientSoft() == SO_URL)
					m_ImageList.Draw(dc, 11, point2, ILD_NORMAL | INDEXTOOVERLAYMASK(uOvlImg));
				else if (lpUpDownClient->GetClientSoft() == SO_AMULE)
					m_ImageList.Draw(dc, 12, point2, ILD_NORMAL | INDEXTOOVERLAYMASK(uOvlImg));
				else if (lpUpDownClient->GetClientSoft() == SO_LPHANT)
					m_ImageList.Draw(dc, 13, point2, ILD_NORMAL | INDEXTOOVERLAYMASK(uOvlImg));
				else if (lpUpDownClient->ExtProtocolAvailable())
					m_ImageList.Draw(dc, 5, point2, ILD_NORMAL | INDEXTOOVERLAYMASK(uOvlImg));
				else
					m_ImageList.Draw(dc, 7, point2, ILD_NORMAL | INDEXTOOVERLAYMASK(uOvlImg));
				cur_rec.left += 20;

 				//Morph Start - added by AndCycle, IP to Country
 				if(CGlobalVariable::ip2country->ShowCountryFlag()){
 					POINT point3= {cur_rec.left,cur_rec.top+1};
 					CGlobalVariable::ip2country->GetFlagImageList()->DrawIndirect(dc, lpUpDownClient->GetCountryFlagIndex(), point3, CSize(18,16), CPoint(0,0), ILD_NORMAL);
 					cur_rec.left+=20;
 				}
 				//Morph End - added by AndCycle, IP to Country

				if (!lpUpDownClient->GetUserName())
					buffer = _T("?");
				else
					buffer = lpUpDownClient->GetUserName();

 				//EastShare Start - added by AndCycle, IP to Country
 				CString tempStr2;
#ifdef _DEBUG_PEER
				tempStr2.Format(_T("%s%s-(%d)"), lpUpDownClient->GetCountryName(), buffer,lpUpDownClient->m_iPeerIndex);
#else
 				tempStr2.Format(_T("%s%s"), lpUpDownClient->GetCountryName(), buffer);
#endif
 				buffer = tempStr2;
 				//EastShare End - added by AndCycle, IP to Country

				dc->DrawText(buffer,buffer.GetLength(),&cur_rec, DLC_DT_TEXT);
			}
			break;

		case 1:		//Added by thilon on 2006.08.30, comment
			break;
		case 2:		// size
			switch(lpUpDownClient->GetSourceFrom()){
				case SF_SERVER:
					buffer = _T("eD2K Server");
					break;
				case SF_KADEMLIA:
					buffer = GetResString(IDS_KADEMLIA);
					break;
				case SF_SOURCE_EXCHANGE:
					buffer = GetResString(IDS_SE);
					break;
				case SF_PASSIVE:
					buffer = GetResString(IDS_PASSIVE);
					break;
				case SF_LINK:
					buffer = GetResString(IDS_SW_LINK);
					break;
				case SF_HTTP:
					buffer = _T("http");
					break;
				case SF_FTP:
					buffer = _T("ftp");
					break;
			}
			dc->DrawText(buffer, buffer.GetLength(), const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
			break;

		case 3:		// transferred
		case 4:		// completed
			// - 'Transferred' column: Show transferred data
			// - 'Completed' column: If 'Transferred' column is hidden, show the amount of transferred data
			//	  in 'Completed' column. This is plain wrong (at least when receiving compressed data), but
			//	  users seem to got used to it.
			if (nColumn == 2 || IsColumnHidden(2)) {
				if (lpCtrlItem->type == AVAILABLE_SOURCE && lpUpDownClient->GetTransferredDown()) {
					buffer = CastItoXBytes(lpUpDownClient->GetTransferredDown(), false, false);
					dc->DrawText(buffer,buffer.GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT | DT_RIGHT);
				}
			}
			break;

		case 5:		// speed
			if (lpCtrlItem->type == AVAILABLE_SOURCE && lpUpDownClient->GetDownloadDatarate()){
				if (lpUpDownClient->GetDownloadDatarate())
					buffer.Format(_T("%s"), CastItoXBytes(lpUpDownClient->GetDownloadDatarate(), false, true));
				dc->DrawText(buffer,buffer.GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT | DT_RIGHT);
			}
			break;

		case 6:		// file info
			{
				CRect rcDraw(*lpRect);
				rcDraw.bottom--; 
				rcDraw.top++; 

				int iWidth = rcDraw.Width();
				int iHeight = rcDraw.Height();
				if (lpCtrlItem->status == (HBITMAP)NULL)
					VERIFY(lpCtrlItem->status.CreateBitmap(1, 1, 1, 8, NULL)); 
				CDC cdcStatus;
				HGDIOBJ hOldBitmap;
				cdcStatus.CreateCompatibleDC(dc);
				int cx = lpCtrlItem->status.GetBitmapDimension().cx;
				DWORD dwTicks = GetTickCount();
				if(lpCtrlItem->dwUpdated + DLC_BARUPDATE < dwTicks || cx !=  iWidth  || !lpCtrlItem->dwUpdated) { 
					lpCtrlItem->status.DeleteObject(); 
					lpCtrlItem->status.CreateCompatibleBitmap(dc,  iWidth, iHeight); 
					lpCtrlItem->status.SetBitmapDimension(iWidth,  iHeight); 
					hOldBitmap = cdcStatus.SelectObject(lpCtrlItem->status); 

					RECT rec_status; 
					rec_status.left = 0; 
					rec_status.top = 0; 
					rec_status.bottom = iHeight; 
					rec_status.right = iWidth; 
					lpUpDownClient->DrawStatusBar(&cdcStatus,  &rec_status,(lpCtrlItem->type == UNAVAILABLE_SOURCE), thePrefs.UseFlatBar()); 

					lpCtrlItem->dwUpdated = dwTicks + (rand() % 128); 
				} else 
					hOldBitmap = cdcStatus.SelectObject(lpCtrlItem->status); 

				dc->BitBlt(rcDraw.left, rcDraw.top, iWidth, iHeight,  &cdcStatus, 0, 0, SRCCOPY); 
				cdcStatus.SelectObject(hOldBitmap);
			}
			break;

		case 7:		// sources
		{
			buffer = lpUpDownClient->GetClientSoftVer();
            if(lpUpDownClient->GetSourceFrom() == SF_LAN)buffer = GetResString(IDS_LAN);
			if (buffer.IsEmpty())
				buffer = GetResString(IDS_UNKNOWN);
			CRect rc(lpRect);
			dc->DrawText(buffer, buffer.GetLength(), &rc, DLC_DT_TEXT);
			break;
		}

		case 8:		// prio
			if (lpUpDownClient->GetDownloadState()==DS_ONQUEUE){
				if (lpUpDownClient->IsRemoteQueueFull()){
					buffer = GetResString(IDS_QUEUEFULL);
					dc->DrawText(buffer, buffer.GetLength(), const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
				}
				else{
					if (lpUpDownClient->GetRemoteQueueRank()){
						buffer.Format(_T("QR: %u"), lpUpDownClient->GetRemoteQueueRank());
						dc->DrawText(buffer, buffer.GetLength(), const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
					}
					else{
						dc->DrawText(buffer, buffer.GetLength(), const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
					}
				}
			}
			else{
				dc->DrawText(buffer, buffer.GetLength(), const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
			}
			break;

		case 9:	{	// status
			if (lpCtrlItem->type == AVAILABLE_SOURCE){
				buffer = lpUpDownClient->GetDownloadStateDisplayString();
#ifdef _DEBUG
				buffer += lpUpDownClient->HasLowID() ? _T("-Low"): _T("-High");
				CString sBlockReqCount;
				sBlockReqCount.Format( _T("-(%d-%d)"),lpUpDownClient->m_PendingBlocks_list.GetCount(),lpUpDownClient->m_DownloadBlocks_list.GetCount() );
				buffer += sBlockReqCount;
#endif 
			}
			else 
			{
				buffer = GetResString(IDS_ASKED4ANOTHERFILE);

// ZZ:DownloadManager -->
                if(thePrefs.IsExtControlsEnabled()) {
                    if(lpUpDownClient->IsInNoNeededList(lpCtrlItem->owner)) {
                        buffer += _T(" (") + GetResString(IDS_NONEEDEDPARTS) + _T(")");
                    } else if(lpUpDownClient->GetDownloadState() == DS_DOWNLOADING) {
                        buffer += _T(" (") + GetResString(IDS_TRANSFERRING) + _T(")");
                    } else if(lpUpDownClient->IsSwapSuspended(lpUpDownClient->GetRequestFile())) {
                        buffer += _T(" (") + GetResString(IDS_SOURCESWAPBLOCKED) + _T(")");
                    }

                    if (lpUpDownClient && lpUpDownClient->GetRequestFile() && lpUpDownClient->GetRequestFile()->GetFileName()){
                        buffer.AppendFormat(_T(": \"%s\""),lpUpDownClient->GetRequestFile()->GetFileName());
                    }
                }
			}

            if(thePrefs.IsExtControlsEnabled() && !lpUpDownClient->m_OtherRequests_list.IsEmpty()) {
                buffer.Append(_T("*"));
            }
// ZZ:DownloadManager <--

			dc->DrawText(buffer,buffer.GetLength(),const_cast<LPRECT>(lpRect), DLC_DT_TEXT);
			break;
		}
		case 10:		// remaining time & size
			break;
		case 11:	// last seen complete
			break;
		case 12:	// last received
			break;
		case 13:	// category
			break;
		}
	}
}

void CDownloadListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
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
	CtrlItem_Struct* content = (CtrlItem_Struct*)lpDrawItemStruct->itemData;
	CMemDC dc(odc, &lpDrawItemStruct->rcItem);
	CFont* pOldFont;
	if (m_fontBold.m_hObject){
		if (content->type == FILE_TYPE){
			if (((const CPartFile*)content->value)->GetTransferringSrcCount())
				pOldFont = dc.SelectObject(&m_fontBold);
			else
				pOldFont = dc.SelectObject(GetFont());
		}
		else if (content->type == UNAVAILABLE_SOURCE || content->type == AVAILABLE_SOURCE){
			if (((const CUpDownClient*)content->value)->GetDownloadState() == DS_DOWNLOADING)
				pOldFont = dc.SelectObject(&m_fontBold);
			else
				pOldFont = dc.SelectObject(GetFont());
		}
		else
			pOldFont = dc.SelectObject(GetFont());
	}
	else
		pOldFont = dc.SelectObject(GetFont());
	CRect cur_rec(lpDrawItemStruct->rcItem);
	COLORREF crOldTextColor = dc.SetTextColor((lpDrawItemStruct->itemState & ODS_SELECTED) ? m_crHighlightText : m_crWindowText);

	int iOldBkMode;
	if (m_crWindowTextBk == CLR_NONE){
		DefWindowProc(WM_ERASEBKGND, (WPARAM)(HDC)dc, 0);
		iOldBkMode = dc.SetBkMode(TRANSPARENT);
	}
	else
		iOldBkMode = OPAQUE;

	BOOL notLast = lpDrawItemStruct->itemID + 1 != (UINT)GetItemCount();
	BOOL notFirst = lpDrawItemStruct->itemID != 0;
	int tree_start=0;
	int tree_end=0;

	//offset was 4, now it's the standard 2 spaces
	int iTreeOffset = dc.GetTextExtent(_T(" "), 1 ).cx*2;
	CHeaderCtrl *pHeaderCtrl = GetHeaderCtrl();
	int iCount = pHeaderCtrl->GetItemCount();
	cur_rec.right = cur_rec.left;
	cur_rec.right -= FILE_ITEM_MARGIN_X;
	cur_rec.left += FILE_ITEM_MARGIN_X;

	if (content->type == FILE_TYPE)
	{
		if (!g_bLowColorDesktop || (lpDrawItemStruct->itemState & ODS_SELECTED) == 0) {
			DWORD dwCatColor = thePrefs.GetCatColor(((/*const*/ CPartFile*)content->value)->GetCategory());
			if (dwCatColor > 0)
				dc.SetTextColor(dwCatColor);
		}

		for (int iCurrent = 0; iCurrent < iCount; iCurrent++)
		{
			int iColumn = pHeaderCtrl->OrderToIndex(iCurrent);
			int cx = CListCtrl::GetColumnWidth(iColumn);
			if (iColumn == 6) {	//Changed by thilon on 2006.08.30, 由5改为6, 此处先改回5
				int iNextLeft = cur_rec.left + cx;
				//set up tree vars
				cur_rec.left = cur_rec.right + iTreeOffset;
				cur_rec.right = cur_rec.left + min(8, cx);
				tree_start = cur_rec.left + 1;
				tree_end = cur_rec.right;
				//normal column stuff
				cur_rec.left = cur_rec.right + 1;
				cur_rec.right = tree_start + cx - iTreeOffset;
				DrawFileItem(dc, 6, &cur_rec, content);	//Changed by thilon on 2006.08.30, 由5改为6
				cur_rec.left = iNextLeft;
			} else {
				cur_rec.right += cx;
				DrawFileItem(dc, iColumn, &cur_rec, content);
				cur_rec.left += cx;
			}
		}
	}
	else if (content->type == UNAVAILABLE_SOURCE || content->type == AVAILABLE_SOURCE)
	{
		for (int iCurrent = 0; iCurrent < iCount; iCurrent++)
		{
			int iColumn = pHeaderCtrl->OrderToIndex(iCurrent);
			int cx = CListCtrl::GetColumnWidth(iColumn);
			if (iColumn == 6) {	//Changed by thilon on 2006.08.30, 由5改为6
				int iNextLeft = cur_rec.left + cx;
				//set up tree vars
				cur_rec.left = cur_rec.right + iTreeOffset;
				cur_rec.right = cur_rec.left + min(8, cx);
				tree_start = cur_rec.left + 1;
				tree_end = cur_rec.right;
				//normal column stuff
				cur_rec.left = cur_rec.right + 1;
				cur_rec.right = tree_start + cx - iTreeOffset;
				DrawSourceItem(dc, 6, &cur_rec, content);	//Changed by thilon on 2006.08.30, 由5改为6
				cur_rec.left = iNextLeft;
			} else {
				cur_rec.right += cx;
				DrawSourceItem(dc, iColumn, &cur_rec, content);
				cur_rec.left += cx;
			}
		}
	}

	//draw rectangle around selected item(s)
	if (content->type == FILE_TYPE && (lpDrawItemStruct->itemState & ODS_SELECTED))
	{
		RECT outline_rec = lpDrawItemStruct->rcItem;

		outline_rec.top--;
		outline_rec.bottom++;
		dc.FrameRect(&outline_rec, &CBrush(GetBkColor()));
		outline_rec.top++;
		outline_rec.bottom--;
		outline_rec.left++;
		outline_rec.right--;

		if(notFirst && (GetItemState(lpDrawItemStruct->itemID - 1, LVIS_SELECTED))) {
			CtrlItem_Struct* prev = (CtrlItem_Struct*)this->GetItemData(lpDrawItemStruct->itemID - 1);
			if(prev->type == FILE_TYPE)
				outline_rec.top--;
		} 

		if(notLast && (GetItemState(lpDrawItemStruct->itemID + 1, LVIS_SELECTED))) {
			CtrlItem_Struct* next = (CtrlItem_Struct*)this->GetItemData(lpDrawItemStruct->itemID + 1);
			if(next->type == FILE_TYPE)
				outline_rec.bottom++;
		} 

		if(bCtrlFocused)
			dc.FrameRect(&outline_rec, &CBrush(m_crFocusLine));
		else
			dc.FrameRect(&outline_rec, &CBrush(m_crNoFocusLine));
	}
	//draw focus rectangle around non-highlightable items when they have the focus
	else if (((lpDrawItemStruct->itemState & ODS_FOCUS) == ODS_FOCUS) && (GetFocus() == this))
	{
		RECT focus_rec;
		focus_rec.top    = lpDrawItemStruct->rcItem.top;
		focus_rec.bottom = lpDrawItemStruct->rcItem.bottom;
		focus_rec.left   = lpDrawItemStruct->rcItem.left + 1;
		focus_rec.right  = lpDrawItemStruct->rcItem.right - 1;
		dc.FrameRect(&focus_rec, &CBrush(m_crNoFocusLine));
	}

	//draw tree last so it draws over selected and focus (looks better)
	if(tree_start < tree_end) {
		//set new bounds
		RECT tree_rect;
		tree_rect.top    = lpDrawItemStruct->rcItem.top;
		tree_rect.bottom = lpDrawItemStruct->rcItem.bottom;
		tree_rect.left   = tree_start;
		tree_rect.right  = tree_end;
		dc.SetBoundsRect(&tree_rect, DCB_DISABLE);

		// VC-yunchenn.chen[2007-07-13]: 导致最后以行绘制不正确
		if(GetItemData(lpDrawItemStruct->itemID + 1)==NULL)
			notLast = false;
			//return;
		//gather some information
		BOOL hasNext = notLast &&
			((CtrlItem_Struct*)this->GetItemData(lpDrawItemStruct->itemID + 1))->type != FILE_TYPE;
		BOOL isOpenRoot = hasNext && content->type == FILE_TYPE;
		BOOL isChild = content->type != FILE_TYPE;
		//BOOL isExpandable = !isChild && ((CPartFile*)content->value)->GetSourceCount() > 0;
		//might as well calculate these now
		int treeCenter = tree_start + 3;
		int middle = (cur_rec.top + cur_rec.bottom + 1) / 2;

		//set up a new pen for drawing the tree
		CPen pn, *oldpn;
		pn.CreatePen(PS_SOLID, 1, m_crWindowText);
		oldpn = dc.SelectObject(&pn);

		if(isChild) {
			//draw the line to the status bar
			dc.MoveTo(tree_end, middle);
			dc.LineTo(tree_start + 3, middle);

			//draw the line to the child node
			if(hasNext) {
				dc.MoveTo(treeCenter, middle);
				dc.LineTo(treeCenter, cur_rec.bottom + 1);
			}
		} else if(isOpenRoot) {
			//draw circle
			RECT circle_rec;
			COLORREF crBk = dc.GetBkColor();
			circle_rec.top    = middle - 2;
			circle_rec.bottom = middle + 3;
			circle_rec.left   = treeCenter - 2;
			circle_rec.right  = treeCenter + 3;
			dc.FrameRect(&circle_rec, &CBrush(m_crWindowText));
			dc.SetPixelV(circle_rec.left,      circle_rec.top,    crBk);
			dc.SetPixelV(circle_rec.right - 1, circle_rec.top,    crBk);
			dc.SetPixelV(circle_rec.left,      circle_rec.bottom - 1, crBk);
			dc.SetPixelV(circle_rec.right - 1, circle_rec.bottom - 1, crBk);
			//draw the line to the child node
			if(hasNext) {
				dc.MoveTo(treeCenter, middle + 3);
				dc.LineTo(treeCenter, cur_rec.bottom + 1);
			}
		} /*else if(isExpandable) {
			//draw a + sign
			dc.MoveTo(treeCenter, middle - 2);
			dc.LineTo(treeCenter, middle + 3);
			dc.MoveTo(treeCenter - 2, middle);
			dc.LineTo(treeCenter + 3, middle);
		}*/

		//draw the line back up to parent node
		if(notFirst && isChild) {
			dc.MoveTo(treeCenter, middle);
			dc.LineTo(treeCenter, cur_rec.top - 1);
		}

		//put the old pen back
		dc.SelectObject(oldpn);
		pn.DeleteObject();
	}

	//put the original objects back
	if (m_crWindowTextBk == CLR_NONE)
		dc.SetBkMode(iOldBkMode);
	dc.SelectObject(pOldFont);
	dc.SetTextColor(crOldTextColor);
}

void CDownloadListCtrl::HideSources(CPartFile* toCollapse)
{
	SetRedraw(false);
	int pre = 0;
	int post = 0;
	for (int i = 0; i < GetItemCount(); i++)
	{
		CtrlItem_Struct* item = (CtrlItem_Struct*)GetItemData(i);
		if (item->owner == toCollapse)
		{
			pre++;
			item->dwUpdated = 0;
			item->status.DeleteObject();
			DeleteItem(i--);
			post++;
		}
	}
	if (pre - post == 0)
		toCollapse->srcarevisible = false;
	SetRedraw(true);
}

void CDownloadListCtrl::ExpandCollapseItem(int iItem, int iAction, bool bCollapseSource)
{
	if (iItem == -1)
		return;
	CtrlItem_Struct* content = (CtrlItem_Struct*)GetItemData(iItem);

	// to collapse/expand files when one of its source is selected
	if (bCollapseSource && content->parent != NULL)
	{
		content=content->parent;
		
 		LVFINDINFO find;
		find.flags = LVFI_PARAM;
		find.lParam = (LPARAM)content;
		iItem = FindItem(&find);
		if (iItem == -1)
			return;
	}

	if (!content || content->type != FILE_TYPE)
		return;
	
	CPartFile* partfile = reinterpret_cast<CPartFile*>(content->value);
	if (!partfile)
		return;
	if(PathFileExists(partfile->GetFilePath()))
	{
		if (partfile->CanOpenFile()) {
			partfile->OpenFile();
			return;
		}
	}
	else if (partfile->getPartfileStatus() == PS_COMPLETE || partfile->getPartfileStatus()== PS_COMPLETING )
	{
		MessageBox(GetResString(IDS_DELETEFILEINFO),GetResString(IDS_CAPTION),MB_OK|MB_ICONINFORMATION);
		return;
	}

	// Check if the source branch is disable
	if (!partfile->srcarevisible)
	{
		if (iAction > COLLAPSE_ONLY)
		{
			SetRedraw(false);
			
			// Go throught the whole list to find out the sources for this file
			// Remark: don't use GetSourceCount() => UNAVAILABLE_SOURCE
			for (ListItems::const_iterator it = m_ListItems.begin(); it != m_ListItems.end(); it++)
			{
				const CtrlItem_Struct* cur_item = it->second;
				if (cur_item->owner == partfile)
				{
					partfile->srcarevisible = true;
					InsertItem(LVIF_PARAM|LVIF_TEXT, iItem+1, LPSTR_TEXTCALLBACK, 0, 0, 0, (LPARAM)cur_item);
				}
			}

			SetRedraw(true);
		}
	}
	else {
		if (iAction == EXPAND_COLLAPSE || iAction == COLLAPSE_ONLY)
		{
			if (GetItemState(iItem, LVIS_SELECTED | LVIS_FOCUSED) != (LVIS_SELECTED | LVIS_FOCUSED))
			{
				SetItemState(iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				SetSelectionMark(iItem);
			}
			HideSources(partfile);
		}
	}
}

void CDownloadListCtrl::OnItemActivate(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	if (thePrefs.IsDoubleClickEnabled() || pNMIA->iSubItem > 0)
	{
		ExpandCollapseItem(pNMIA->iItem, EXPAND_COLLAPSE);
	}
	*pResult = 0;
}

void CDownloadListCtrl::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	CreateMenues();
	int iSel = GetNextItem(-1, LVIS_SELECTED);
	if (iSel != -1)
	{
		const CtrlItem_Struct* content = (CtrlItem_Struct*)GetItemData(iSel);
		if (NULL != content)
		{
			if (content->type == FILE_TYPE)
			{
				// get merged settings
				bool bFirstItem = true;
				int iSelectedItems = 0;
				int iFilesNotDone = 0;
				int iFilesToPause = 0;
				int iFilesToStop = 0;
				int iFilesToResume = 0;
				int iFilesToOpen = 0;
				int iFilesGetPreviewParts = 0;
				int iFilesPreviewType = 0;
				int iFilesToPreview = 0;
				int iFilesToCancel = 0;
				UINT uPrioMenuItem = 0;
				const CPartFile* file1 = NULL;
				POSITION pos = GetFirstSelectedItemPosition();
				while (pos)
				{
					const CtrlItem_Struct* pItemData = (CtrlItem_Struct*)GetItemData(GetNextSelectedItem(pos));
					
					if (pItemData->type != FILE_TYPE)
					{
						continue;
					}

					const CPartFile* pFile = (CPartFile*)pItemData->value;
					
					if (bFirstItem)
					{
						file1 = pFile;
					}

					iSelectedItems++;

					bool bFileDone = (pFile->GetStatus()==PS_COMPLETE || pFile->GetStatus()==PS_COMPLETING);
					iFilesToCancel += pFile->GetStatus() != PS_COMPLETING ? 1 : 0;
					iFilesNotDone += !bFileDone ? 1 : 0;
					iFilesToStop += pFile->CanStopFile() ? 1 : 0;
					iFilesToPause += pFile->CanPauseFile() ? 1 : 0;
					iFilesToResume += pFile->CanResumeFile() ? 1 : 0;
					iFilesToOpen += pFile->CanOpenFile() ? 1 : 0;
					iFilesGetPreviewParts += pFile->GetPreviewPrio() ? 1 : 0;
					iFilesPreviewType += pFile->IsPreviewableFileType() ? 1 : 0;
					iFilesToPreview += pFile->IsReadyForPreview() ? 1 : 0;

					UINT uCurPrioMenuItem = 0;
					if (pFile->IsAutoDownPriority())
						uCurPrioMenuItem = MP_PRIOAUTO;
					else if (pFile->GetDownPriority() == PR_HIGH)
						uCurPrioMenuItem = MP_PRIOHIGH;
					else if (pFile->GetDownPriority() == PR_NORMAL)
						uCurPrioMenuItem = MP_PRIONORMAL;
					else if (pFile->GetDownPriority() == PR_LOW)
						uCurPrioMenuItem = MP_PRIOLOW;
					else
						ASSERT(0);

					if (bFirstItem)
						uPrioMenuItem = uCurPrioMenuItem;
					else if (uPrioMenuItem != uCurPrioMenuItem)
						uPrioMenuItem = 0;
					bFirstItem = false;
				}

				m_FileMenu.EnableMenuItem((UINT_PTR)m_PrioMenu.m_hMenu, iFilesNotDone > 0 ? MF_ENABLED : MF_GRAYED);
				m_PrioMenu.CheckMenuRadioItem(MP_PRIOLOW, MP_PRIOAUTO, uPrioMenuItem, 0);

				// enable commands if there is at least one item which can be used for the action
				m_FileMenu.EnableMenuItem(MP_CANCEL, iFilesToCancel > 0 ? MF_ENABLED : MF_GRAYED);
				m_FileMenu.EnableMenuItem(MP_STOP, iFilesToStop > 0 ? MF_ENABLED : MF_GRAYED);
				m_FileMenu.EnableMenuItem(MP_PAUSE, iFilesToPause > 0 ? MF_ENABLED : MF_GRAYED);
				m_FileMenu.EnableMenuItem(MP_RESUME, iFilesToResume > 0 ? MF_ENABLED : MF_GRAYED);
#ifdef _PLAY_WHILE_DOWNLOADING			
				m_FileMenu.EnableMenuItem(MP_PLAY, MF_ENABLED); // VC-SearchDream[2007-05-16]: for see movie while downloading
#endif			

#ifdef _PLAY_WHILE_DOWNLOADING		
				// VC-SearchDream[2007-05-16]: for see movie while downloading
				if (file1 && iSelectedItems <= 1)
				{
					CString strFileName = file1->GetFileName();

					strFileName = strFileName.Right(strFileName.GetLength() - strFileName.ReverseFind('.') - 1);

					if (strFileName.CompareNoCase(_T("rm")) == 0  || strFileName.CompareNoCase(_T("rmvb")) == 0 
						|| strFileName.CompareNoCase(_T("wmv")) == 0)
					{
						m_FileMenu.EnableMenuItem(MP_PLAY, MF_ENABLED); 
					}
					else
					{
						m_FileMenu.EnableMenuItem(MP_PLAY, MF_GRAYED);
					}
				}
				else
				{
					m_FileMenu.EnableMenuItem(MP_PLAY, MF_GRAYED);
				}			
#endif		

				bool bOpenEnabled = (iSelectedItems == 1 && iFilesToOpen == 1);
				m_FileMenu.EnableMenuItem(MP_OPEN, bOpenEnabled ? MF_ENABLED : MF_GRAYED);
				if(thePrefs.IsExtControlsEnabled() && !thePrefs.GetPreviewPrio()) {
					m_FileMenu.EnableMenuItem(MP_TRY_TO_GET_PREVIEW_PARTS, (iSelectedItems == 1 && iFilesPreviewType == 1 && iFilesToPreview == 0 && iFilesNotDone == 1) ? MF_ENABLED : MF_GRAYED);
					m_FileMenu.CheckMenuItem(MP_TRY_TO_GET_PREVIEW_PARTS, (iSelectedItems == 1 && iFilesGetPreviewParts == 1) ? MF_CHECKED : MF_UNCHECKED);
				}
				m_FileMenu.EnableMenuItem(MP_PREVIEW, (iSelectedItems == 1 && iFilesToPreview == 1) ? MF_ENABLED : MF_GRAYED);
				CMenu PreviewMenu;
				PreviewMenu.CreateMenu();
				int iPreviewMenuEntries = thePreviewApps.GetAllMenuEntries(PreviewMenu, (iSelectedItems == 1) ? file1 : NULL);
				if (iPreviewMenuEntries)
					m_FileMenu.InsertMenu(MP_METINFO, MF_POPUP | (iSelectedItems == 1 ? MF_ENABLED : MF_GRAYED), (UINT_PTR)PreviewMenu.m_hMenu, GetResString(IDS_DL_PREVIEW));

				bool bDetailsEnabled = (iSelectedItems > 0);
				m_FileMenu.EnableMenuItem(MP_METINFO, bDetailsEnabled ? MF_ENABLED : MF_GRAYED);
				if (thePrefs.IsDoubleClickEnabled() && bOpenEnabled)
					m_FileMenu.SetDefaultItem(MP_OPEN);
				else if (!thePrefs.IsDoubleClickEnabled() && bDetailsEnabled)
					m_FileMenu.SetDefaultItem(MP_METINFO);
				else
					m_FileMenu.SetDefaultItem((UINT)-1);
				m_FileMenu.EnableMenuItem(MP_VIEWFILECOMMENTS, (iSelectedItems >= 1 /*&& iFilesNotDone == 1*/) ? MF_ENABLED : MF_GRAYED);

				int total;
				m_FileMenu.EnableMenuItem(MP_CLEARCOMPLETED, GetCompleteDownloads(curTab, total) > 0 ? MF_ENABLED : MF_GRAYED);

				if (m_SourcesMenu && thePrefs.IsExtControlsEnabled()) {
					m_FileMenu.EnableMenuItem((UINT_PTR)m_SourcesMenu.m_hMenu, MF_ENABLED);
					m_SourcesMenu.EnableMenuItem(MP_ADDSOURCE, (iSelectedItems == 1 && iFilesToStop == 1) ? MF_ENABLED : MF_GRAYED);
					m_SourcesMenu.EnableMenuItem(MP_SETSOURCELIMIT, (iFilesNotDone == iSelectedItems) ? MF_ENABLED : MF_GRAYED);
				}

				m_FileMenu.EnableMenuItem(thePrefs.GetShowCopyEd2kLinkCmd() ? MP_GETED2KLINK : MP_SHOWED2KLINK, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
				m_FileMenu.EnableMenuItem(MP_PASTE, theApp.IsEd2kFileLinkInClipboard() ? MF_ENABLED : MF_GRAYED);
				m_FileMenu.EnableMenuItem(MP_FIND, GetItemCount() > 0 ? MF_ENABLED : MF_GRAYED);
				m_FileMenu.EnableMenuItem(MP_SEARCHRELATED, iSelectedItems == 1 && theApp.emuledlg->searchwnd->CanSearchRelatedFiles() ? MF_ENABLED : MF_GRAYED);

				CTitleMenu WebMenu;
				WebMenu.CreateMenu();
				//WebMenu.AddMenuTitle(NULL, true);
				int iWebMenuEntries = theWebServices.GetFileMenuEntries(&WebMenu);
				UINT flag = (iWebMenuEntries == 0 || iSelectedItems != 1) ? MF_GRAYED : MF_ENABLED;

#ifndef _FOREIGN_VERSION
				m_FileMenu.AppendMenu(MF_POPUP | flag, (UINT_PTR)WebMenu.m_hMenu, GetResString(IDS_WEBSERVICES), _T("WEB"));
#endif
				cif.AddIcon((UINT_PTR)WebMenu.m_hMenu, theApp.LoadIcon(_T("WEB")));
				cif.AddIcon((UINT_PTR)WebMenu.m_hMenu, theApp.LoadIcon(_T("WEB")), TRUE);

				// create cat-submenue
				CMenu CatsMenu;
				CatsMenu.CreateMenu();
				flag = (thePrefs.GetCatCount() == 1) ? MF_GRAYED : MF_ENABLED;
				CString label;
				if (thePrefs.GetCatCount()>1) {
					for (int i = 0; i < thePrefs.GetCatCount(); i++){
						if (i>0) {
							label=thePrefs.GetCategory(i)->strTitle;
							label.Replace(_T("&"), _T("&&") );
						}
						CatsMenu.AppendMenu(MF_STRING,MP_ASSIGNCAT+i, (i==0)?GetResString(IDS_CAT_UNASSIGN):label);
					}
				}
				//m_FileMenu.AppendMenu(MF_POPUP | flag, (UINT_PTR)CatsMenu.m_hMenu, GetResString(IDS_TOCAT), _T("CATEGORY"));

				m_MenuXP = new CMenuXP();
				m_MenuXP->AddMenu(&m_FileMenu, TRUE);

				GetPopupMenuPos(*this, point);
				m_FileMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);
				
				

				VERIFY( m_FileMenu.RemoveMenu(m_FileMenu.GetMenuItemCount() - 1, MF_BYPOSITION) );
				VERIFY( m_FileMenu.RemoveMenu(m_FileMenu.GetMenuItemCount() - 1, MF_BYPOSITION) );
				if (iPreviewMenuEntries)
				{
					VERIFY( m_FileMenu.RemoveMenu((UINT)PreviewMenu.m_hMenu, MF_BYCOMMAND) );
				}
				VERIFY( WebMenu.DestroyMenu() );
				VERIFY( CatsMenu.DestroyMenu() );
				VERIFY( PreviewMenu.DestroyMenu() );

				delete m_MenuXP;
				m_MenuXP = NULL;
			}
			else
			{
				const CUpDownClient* client = (CUpDownClient*)content->value;
				CTitleMenu ClientMenu;
				ClientMenu.CreatePopupMenu();
				//ClientMenu.AddMenuTitle(GetResString(IDS_CLIENTS), true);
				ClientMenu.AppendMenu(MF_STRING, MP_DETAIL, GetResString(IDS_SHOWDETAILS), _T("CLIENTDETAILS"));

				//VC-dgkang 2008年7月10日
				ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->GetViewSharedFilesSupport()) ? MF_ENABLED : MF_GRAYED), MP_SHOWLIST, GetResString(IDS_VIEWFILES), _T("VIEWFILES"));
				ClientMenu.SetDefaultItem(MP_DETAIL);

				//ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && !client->IsFriend()) ? MF_ENABLED : MF_GRAYED), MP_ADDFRIEND, GetResString(IDS_ADDFRIEND), _T("ADDFRIEND"));
				//ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient()) ? MF_ENABLED : MF_GRAYED), MP_MESSAGE, GetResString(IDS_SEND_MSG), _T("SENDMESSAGE"));

				//if (Kademlia::CKademlia::IsRunning() && !Kademlia::CKademlia::IsConnected())
				//	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->GetKadPort()!=0) ? MF_ENABLED : MF_GRAYED), MP_BOOT, GetResString(IDS_BOOTSTRAP));
				//ClientMenu.AppendMenu(MF_STRING | (GetItemCount() > 0 ? MF_ENABLED : MF_GRAYED), MP_FIND, GetResString(IDS_FIND), _T("Search"));

				CMenu A4AFMenu;
				A4AFMenu.CreateMenu();
				if (thePrefs.IsExtControlsEnabled()) 
				{
					// ZZ:DownloadManager -->
#ifdef _DEBUG
					if (content->type == UNAVAILABLE_SOURCE) {
						A4AFMenu.AppendMenu(MF_STRING,MP_A4AF_CHECK_THIS_NOW,GetResString(IDS_A4AF_CHECK_THIS_NOW));
					}
# endif
					// <-- ZZ:DownloadManager
					if (A4AFMenu.GetMenuItemCount()>0)
						ClientMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)A4AFMenu.m_hMenu, GetResString(IDS_A4AF));
				}

				m_MenuXP = new CMenuXP();
				m_MenuXP->AddMenu(&ClientMenu, TRUE);

				GetPopupMenuPos(*this, point);
				ClientMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);

				delete m_MenuXP;
				m_MenuXP = NULL;

				VERIFY( A4AFMenu.DestroyMenu() );
				VERIFY( ClientMenu.DestroyMenu() );
			}
		}
	}
	else
	{
		int total;
		m_FileMenu.EnableMenuItem((UINT_PTR)m_PrioMenu.m_hMenu, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_CANCEL, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_PAUSE, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_STOP, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_RESUME, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_OPEN, MF_GRAYED);
#ifdef _PLAY_WHILE_DOWNLOADING
		m_FileMenu.EnableMenuItem(MP_PLAY, MF_GRAYED); // VC-SearchDream[2007-05-16]: for see movie while downloading
#endif

		if (thePrefs.IsExtControlsEnabled() && !thePrefs.GetPreviewPrio()) 
		{
			m_FileMenu.EnableMenuItem(MP_TRY_TO_GET_PREVIEW_PARTS, MF_GRAYED);
			m_FileMenu.CheckMenuItem(MP_TRY_TO_GET_PREVIEW_PARTS, MF_UNCHECKED);
        }
		m_FileMenu.EnableMenuItem(MP_PREVIEW, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_METINFO, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_VIEWFILECOMMENTS, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_CLEARCOMPLETED, GetCompleteDownloads(curTab,total) > 0 ? MF_ENABLED : MF_GRAYED);
		m_FileMenu.EnableMenuItem(thePrefs.GetShowCopyEd2kLinkCmd() ? MP_GETED2KLINK : MP_SHOWED2KLINK, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_PASTE, theApp.IsEd2kFileLinkInClipboard() ? MF_ENABLED : MF_GRAYED);
		m_FileMenu.SetDefaultItem((UINT)-1);
		if (m_SourcesMenu)
		{
			m_FileMenu.EnableMenuItem((UINT_PTR)m_SourcesMenu.m_hMenu, MF_GRAYED);
		}

		m_FileMenu.EnableMenuItem(MP_SEARCHRELATED, MF_GRAYED);
		m_FileMenu.EnableMenuItem(MP_FIND, GetItemCount() > 0 ? MF_ENABLED : MF_GRAYED);

		// also show the "Web Services" entry, even if its disabled and therefore not useable, it though looks a little 
		// less confusing this way.
		CTitleMenu WebMenu;
		WebMenu.CreateMenu();
		WebMenu.AddMenuTitle(NULL, true);
		theWebServices.GetFileMenuEntries(&WebMenu);

#ifndef _FOREIGN_VERSION
		m_FileMenu.AppendMenu(MF_POPUP | MF_GRAYED, (UINT_PTR)WebMenu.m_hMenu, GetResString(IDS_WEBSERVICES), _T("WEB"));
#endif
		cif.AddIcon((UINT_PTR)WebMenu.m_hMenu, theApp.LoadIcon(_T("WEB")));
		cif.AddIcon((UINT_PTR)WebMenu.m_hMenu, theApp.LoadIcon(_T("WEB")), TRUE);

		m_MenuXP = new CMenuXP();
		m_MenuXP->AddMenu(&m_FileMenu, TRUE);

		GetPopupMenuPos(*this, point);
		m_FileMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);

		m_FileMenu.RemoveMenu(m_FileMenu.GetMenuItemCount() - 1, MF_BYPOSITION);
		VERIFY( WebMenu.DestroyMenu() );

		delete m_MenuXP;
		m_MenuXP = NULL;
	}

	cif.RemoveIcon((UINT_PTR)m_PrioMenu.m_hMenu, TRUE);
	cif.RemoveIcon((UINT_PTR)m_PrioMenu.m_hMenu, FALSE);
}

BOOL CDownloadListCtrl::OnCommand(WPARAM wParam, LPARAM /*lParam*/)
{
	wParam = LOWORD(wParam);

	switch (wParam)
	{
		case MP_PASTE:
			if (theApp.IsEd2kFileLinkInClipboard())
				theApp.PasteClipboard(curTab);
			return TRUE;
		case MP_FIND:
			OnFindStart();
			return TRUE;
		case MP_NEW:
			CmdFuncs::PopupNewTaskDlg();
			return TRUE;
	}

	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel == -1)
		iSel = GetNextItem(-1, LVIS_SELECTED);
	if (iSel != -1)
	{
		const CtrlItem_Struct* content = (CtrlItem_Struct*)GetItemData(iSel);
		if (content->type == FILE_TYPE)
		{
			//for multiple selections 
			UINT selectedCount = 0;
			CTypedPtrList<CPtrList, CPartFile*> selectedList; 
			POSITION pos = GetFirstSelectedItemPosition();
			while(pos != NULL) 
			{ 
				int index = GetNextSelectedItem(pos);
				if(index > -1) 
				{
					if (((const CtrlItem_Struct*)GetItemData(index))->type == FILE_TYPE)
					{
						selectedCount++;
						selectedList.AddTail((CPartFile*)((const CtrlItem_Struct*)GetItemData(index))->value);
					}
				} 
			} 

			CPartFile* file = (CPartFile*)content->value;
			
			switch (wParam)
			{
				case MP_CANCEL:
				case MPG_DELETE: // keyboard del will continue to remove completed files from the screen while cancel will now also be available for complete files
				{
					if (selectedCount > 0)
					{
						SetRedraw(false);
						CString fileList;
				//		bool validdelete = false;
				//		bool removecompl = false;
						for (pos = selectedList.GetHeadPosition(); pos != 0; )
						{
							CPartFile* cur_file = selectedList.GetNext(pos);

							//changed by thilon on 2007.07.13, 判断删除的是不是Update更新包
							CUpdateInfo updateinfo;
							CString hash = md4str(cur_file->GetFileHash());
							if(updateinfo.isUpdateFile(hash))
							{
								if(updateinfo.UpdateInfoFileExists())
								{
									DeleteFile(updateinfo.GetUpdateInfoFile());
								}
							}

							if (cur_file->GetStatus() != PS_COMPLETING || (cur_file->GetStatus() != PS_COMPLETE && wParam == MP_CANCEL))
							{
						//		validdelete = true;
								if (selectedCount < 50)
								{
									fileList.Append(_T("\n") + CString(cur_file->GetFileName()));
								}
							}
					//		else if (cur_file->GetStatus() == PS_COMPLETE)
					//		{
					//			removecompl = true;
					//		}

							m_ToolBarUI.iFilesToCancel = cur_file->GetStatus() != PS_COMPLETING ? 1 : 0;
						}

						CString quest;
						if (selectedCount == 1)
							quest = GetResString(IDS_Q_CANCELDL2);
						else
							quest = GetResString(IDS_Q_CANCELDL);
				//	  if ((removecompl && !validdelete) || (validdelete/* && AfxMessageBox(quest + fileList, MB_DEFBUTTON2 | MB_ICONQUESTION | MB_YESNO) == IDYES*/))
				//	  {
							bool bRemovedItems = false;
							CAffirmDeleteDlg affirmDelete; 
							if(selectedList.GetCount()>1)
							{   
								bool bCompleteSign = false;
								POSITION pos = selectedList.GetHeadPosition();
								while(pos)
								{   
									CPartFile *pFile = (CPartFile *)selectedList.GetNext(pos);
									if(pFile->GetStatus() == PS_COMPLETE)
									{
										bCompleteSign = true;
                                        break;
									}
								}
								if(bCompleteSign == true)
								{   
									SetRedraw(true);
									if(affirmDelete.DoModal() == IDCANCEL)	
										break;
								}
								else
								{
									SetRedraw(true);
									if(MessageBox(quest + fileList,GetResString(IDS_CAPTION),MB_DEFBUTTON2 | MB_ICONQUESTION | MB_YESNO) == IDNO)
									    break;
								}
								
							}
							else if(selectedList.GetCount() == 1 && selectedList.GetHead()->GetStatus() == PS_COMPLETE)
							{   
								SetRedraw(true);
								if(affirmDelete.DoModal() == IDCANCEL)
									break;
							}
							else if(selectedList.GetCount() == 1 && selectedList.GetHead()->GetStatus() != PS_COMPLETE)
							{    
								 SetRedraw(true);
								 if(MessageBox(quest + fileList,GetResString(IDS_CAPTION),MB_DEFBUTTON2 | MB_ICONQUESTION | MB_YESNO) == IDNO)
									 break;
							}
							SetRedraw(false);
							while (!selectedList.IsEmpty())
							{
								HideSources(selectedList.GetHead());
								switch (selectedList.GetHead()->GetStatus())
								{
									case PS_WAITINGFORHASH:
									case PS_HASHING:
									case PS_COMPLETING:
										selectedList.GetHead()->DeleteFile();
										selectedList.RemoveHead();
										bRemovedItems = true;
										break;
									case PS_COMPLETE:
								//		if (wParam == MP_CANCEL)
										{
											BOOL delsucc = FALSE;
											CKnownFile *pmyfile = (CKnownFile *)selectedList.GetHead();
											if (!PathFileExists(selectedList.GetHead()->GetFilePath())||pmyfile->GetFileSize() == (uint64)0)
											{  
												delsucc = TRUE;
												::SendMessage(theWndMgr.GetWndHandle(CWndMgr::WI_DOWNLOADED_LISTCTRL), UM_DLED_LC_REMOVEFILE, 0, (LPARAM)pmyfile);
												CGlobalVariable::filemgr.RemoveFileItem(pmyfile,false);
												CGlobalVariable::filemgr.RemoveURLTask(pmyfile->GetPartFileURL());
											}
											else{
												// Delete
												if(affirmDelete.bIsDeleteFile)
												{
												if (!thePrefs.GetRemoveToBin()){
													delsucc = DeleteFile(selectedList.GetHead()->GetFilePath());
												}
												else{
													// delete to recycle bin :(
													TCHAR todel[MAX_PATH+1];
													memset(todel, 0, sizeof todel);
													_tcsncpy(todel, selectedList.GetHead()->GetFilePath(), ARRSIZE(todel)-2);

													SHFILEOPSTRUCT fp = {0};
													fp.wFunc = FO_DELETE;
													fp.hwnd = theApp.emuledlg->m_hWnd;
													fp.pFrom = todel;
													fp.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_SILENT;// | FOF_NOERRORUI
													delsucc = (SHFileOperation(&fp) == 0);
													if (delsucc){
														CGlobalVariable::sharedfiles->RemoveFile(selectedList.GetHead());
														CGlobalVariable::filemgr.RemoveFileItem(selectedList.GetHead());
													}
													else{
														CString strError;
														strError.Format( GetResString(IDS_ERR_DELFILE) + _T("\r\n\r\n%s"), selectedList.GetHead()->GetFilePath(), GetErrorMessage(GetLastError()));
														MessageBox(strError,GetResString(IDS_CAPTION),MB_OK);
													}
												}
												}
												else
												{
													::SendMessage(theWndMgr.GetWndHandle(CWndMgr::WI_DOWNLOADED_LISTCTRL), UM_DLED_LC_REMOVEFILE, 0, (LPARAM)pmyfile);
													CGlobalVariable::filemgr.RemoveFileItem(pmyfile);
												}
											}
											
								//		}
										RemoveFile(selectedList.GetHead());
										selectedList.RemoveHead();
										bRemovedItems = true;
										break;
										}
									case PS_PAUSED:
										selectedList.GetHead()->DeleteFile();
										selectedList.RemoveHead();
										bRemovedItems = true;
										break;
									default:
										if (selectedList.GetHead()->GetCategory())
											CGlobalVariable::downloadqueue->StartNextFileIfPrefs(selectedList.GetHead()->GetCategory());
										selectedList.GetHead()->DeleteFile();
										selectedList.RemoveHead();
										
										bRemovedItems = true;
								}
							}
							if (bRemovedItems)
							{
								::PostMessage(CGlobalVariable::m_hListenWnd, WM_FILE_REMOVE_EVENTLOG, 0, 0);
								AutoSelectItem();
								theApp.emuledlg->transferwnd->UpdateCatTabTitles();
							}
						//}
						SetRedraw(true);
					}
					break;
				}
				case MP_PRIOHIGH:
					SetRedraw(false);
					while (!selectedList.IsEmpty()){
						CPartFile* partfile = selectedList.GetHead();
						partfile->SetAutoDownPriority(false);
						partfile->SetDownPriority(PR_HIGH);
						selectedList.RemoveHead();
						UpdateItem(partfile);
						theApp.emuledlg->m_mainTabWnd.m_dlgDownload.m_dlgDetailInfo.UpdateInfo(partfile,CDetailInfo::IM_COMBINE_DOWNLOAD);
					}
					SetRedraw(true);
					break;
				case MP_PRIOLOW:
					SetRedraw(false);
					while (!selectedList.IsEmpty()){
						CPartFile* partfile = selectedList.GetHead();
						partfile->SetAutoDownPriority(false);
						partfile->SetDownPriority(PR_LOW);
						selectedList.RemoveHead();
						UpdateItem(partfile);
						theApp.emuledlg->m_mainTabWnd.m_dlgDownload.m_dlgDetailInfo.UpdateInfo(partfile,CDetailInfo::IM_COMBINE_DOWNLOAD);
					}
					SetRedraw(true);
					break;
				case MP_PRIONORMAL:
					SetRedraw(false);
					while (!selectedList.IsEmpty()){
						CPartFile* partfile = selectedList.GetHead();
						partfile->SetAutoDownPriority(false);
						partfile->SetDownPriority(PR_NORMAL);
						selectedList.RemoveHead();
						UpdateItem(partfile);
						theApp.emuledlg->m_mainTabWnd.m_dlgDownload.m_dlgDetailInfo.UpdateInfo(partfile,CDetailInfo::IM_COMBINE_DOWNLOAD);
					}
					SetRedraw(true);
					break;
				case MP_PRIOAUTO:
					SetRedraw(false);
					while (!selectedList.IsEmpty()){
						CPartFile* partfile = selectedList.GetHead();
						partfile->SetAutoDownPriority(true);
						partfile->SetDownPriority(PR_HIGH);
						selectedList.RemoveHead();
						UpdateItem(partfile);
						theApp.emuledlg->m_mainTabWnd.m_dlgDownload.m_dlgDetailInfo.UpdateInfo(partfile,CDetailInfo::IM_COMBINE_DOWNLOAD);
					}
					SetRedraw(true);
					break;
				case MP_PAUSE:
					SetRedraw(false);
					while (!selectedList.IsEmpty())
					{
						CPartFile* partfile = selectedList.GetHead();
						if (partfile->CanPauseFile())
						{
							partfile->PauseFile();
						}
						selectedList.RemoveHead();

						m_ToolBarUI.iFilesToCancel = partfile->GetStatus() != PS_COMPLETING ? 1 : 0;
						m_ToolBarUI.iFilesToPause = partfile->CanPauseFile() ? 1 : 0;
						m_ToolBarUI.iFilesToResume = partfile->CanResumeFile() ? 1 : 0;
						m_ToolBarUI.iFilesToStop = partfile->CanStopFile() ? 1 : 0;
					}
					SetRedraw(true);
					break;
				case MP_RESUME:
					SetRedraw(false);
					while (!selectedList.IsEmpty())
					{
						CPartFile* partfile = selectedList.GetHead();
						if (partfile->CanResumeFile())
						{
							if (partfile->GetStatus() == PS_INSUFFICIENT)
								partfile->ResumeFileInsufficient();
							else
								partfile->ResumeFile();
						}
						selectedList.RemoveHead();

						m_ToolBarUI.iFilesToCancel = partfile->GetStatus() != PS_COMPLETING ? 1 : 0;
						m_ToolBarUI.iFilesToPause = partfile->CanPauseFile() ? 1 : 0;
						m_ToolBarUI.iFilesToResume = partfile->CanResumeFile() ? 1 : 0;
						m_ToolBarUI.iFilesToStop = partfile->CanStopFile() ? 1 : 0;
					}
					SetRedraw(true);
					break;
				case MP_STOP:
					SetRedraw(false);
					while (!selectedList.IsEmpty())
					{
						CPartFile *partfile = selectedList.GetHead();
						if (partfile->CanStopFile())
						{
							HideSources(partfile);
							partfile->StopFile();
						}
						selectedList.RemoveHead();

						m_ToolBarUI.iFilesToCancel = partfile->GetStatus() != PS_COMPLETING ? 1 : 0;
						m_ToolBarUI.iFilesToPause = partfile->CanPauseFile() ? 1 : 0;
						m_ToolBarUI.iFilesToResume = partfile->CanResumeFile() ? 1 : 0;
						m_ToolBarUI.iFilesToStop = partfile->CanStopFile() ? 1 : 0;
					}
					SetRedraw(true);
					theApp.emuledlg->transferwnd->UpdateCatTabTitles();
					break;
#ifdef _PLAY_WHILE_DOWNLOADING
				case MP_PLAY: // VC-SearchDream[2007-05-16]: for see movie while downloading
					{
						if (!selectedList.IsEmpty())
						{
							CPartFile *partfile = selectedList.GetHead();
							m_pPartFile = partfile;
							partfile->SetSeeOnDownloading(this, true);
						}
					}
					break;
#endif
				case MP_CLEARCOMPLETED:
					SetRedraw(false);
					ClearCompleted();
					SetRedraw(true);
					break;
				case MPG_F2:
					if (GetAsyncKeyState(VK_CONTROL) < 0 || selectedCount > 1) {
						// when ctrl is pressed -> filename cleanup
						if (IDYES==AfxMessageBox(GetResString(IDS_MANUAL_FILENAMECLEANUP),MB_YESNO))
							while (!selectedList.IsEmpty()){
								CPartFile *partfile = selectedList.GetHead();
								if (partfile->IsPartFile()) {
									partfile->SetFileName(CleanupFilename(partfile->GetFileName()));
								}
								selectedList.RemoveHead();
							}
					} else {
						if (file->GetStatus() != PS_COMPLETE && file->GetStatus() != PS_COMPLETING)
						{
							InputBox inputbox;
							CString title = GetResString(IDS_RENAME);
							title.Remove(_T('&'));
							inputbox.SetLabels(title, GetResString(IDS_DL_FILENAME), file->GetFileName());
							inputbox.SetEditFilenameMode();
							if (inputbox.DoModal()==IDOK && !inputbox.GetInput().IsEmpty() && IsValidEd2kString(inputbox.GetInput()))
							{
								file->SetFileName(inputbox.GetInput(), true);
								file->UpdateDisplayedInfo();
								file->SavePartFile();
							}
						}
						else
							MessageBeep(MB_OK);
					}
					break;
				case MP_METINFO:
				case MPG_ALTENTER:
					ShowFileDialog(0);
					break;
				case MP_COPYSELECTED:
				case MP_GETED2KLINK:{
					CString str;
					while (!selectedList.IsEmpty()){
						if (!str.IsEmpty())
							str += _T("\r\n");
						str += CreateED2kLink(selectedList.GetHead());
						selectedList.RemoveHead();
					}
					theApp.CopyTextToClipboard(str);
					break;
				}
				case MP_SEARCHRELATED:
					if (selectedCount > 1)
						break;
					theApp.emuledlg->searchwnd->SearchRelatedFiles(file);
					theApp.emuledlg->SetActiveDialog(theApp.emuledlg->searchwnd);
					break;
				case MP_OPEN:
				case IDA_ENTER:
					if (selectedCount > 1)
						break;
					if(file->GetFileSize() == (uint64)0)
					{
						MessageBox(GetResString(IDS_ZERO_FILE),GetResString(IDS_CAPTION),MB_OK|MB_ICONINFORMATION);
					}
					else
					{
						if (PathFileExists(file->GetFilePath()))
						{
							if(file->CanOpenFile())
								file->OpenFile();
						}
						else
							MessageBox(GetResString(IDS_DELETEFILEINFO),GetResString(IDS_CAPTION),MB_OK|MB_ICONINFORMATION);
					}
					break;
				case MP_OPENFOLDER:
					CmdFuncs::OpenFolder(file);
					break;
				case MP_TRY_TO_GET_PREVIEW_PARTS:
					if (selectedCount > 1)
						break;
                    file->SetPreviewPrio(!file->GetPreviewPrio());
                    break;
				case MP_PREVIEW:
					if (selectedCount > 1)
						break;
					file->PreviewFile();
					break;
				case MP_VIEWFILECOMMENTS:
					ShowFileDialog(IDD_COMMENTLST);
					break;
				case MP_SHOWED2KLINK:
					ShowFileDialog(IDD_ED2KLINK);
					break;
				case MP_SETSOURCELIMIT: {
					CString temp;
					temp.Format(_T("%u"),file->GetPrivateMaxSources());
					InputBox inputbox;
					CString title = GetResString(IDS_SETPFSLIMIT);
					inputbox.SetLabels(title, GetResString(IDS_SETPFSLIMITEXPLAINED), temp );

					if (inputbox.DoModal() == IDOK)
					{
						temp = inputbox.GetInput();
						int newlimit = _tstoi(temp);
						while (!selectedList.IsEmpty()){
							CPartFile *partfile = selectedList.GetHead();
							partfile->SetPrivateMaxSources(newlimit);
							selectedList.RemoveHead();
							partfile->UpdateDisplayedInfo(true);
						}
					}
					break;
				}
 
				case MP_ADDSOURCE: {
					if (selectedCount > 1)
						break;
					CAddSourceDlg as;
					as.SetFile(file);
					as.DoModal();
					break;
				}
				default:
					if (wParam>=MP_WEBURL && wParam<=MP_WEBURL+99){
						theWebServices.RunURL(file, wParam);
					}
					else if (wParam>=MP_ASSIGNCAT && wParam<=MP_ASSIGNCAT+99){
						SetRedraw(FALSE);
						while (!selectedList.IsEmpty()){
							CPartFile *partfile = selectedList.GetHead();
							partfile->SetCategory(wParam - MP_ASSIGNCAT);
							partfile->UpdateDisplayedInfo(true);
							selectedList.RemoveHead();
						}
						SetRedraw(TRUE);
						UpdateCurrentCategoryView();
						if (thePrefs.ShowCatTabInfos())
							theApp.emuledlg->transferwnd->UpdateCatTabTitles();
					}
					else if (wParam>=MP_PREVIEW_APP_MIN && wParam<=MP_PREVIEW_APP_MAX){
						thePreviewApps.RunApp(file, wParam);
					}
					break;
			}
		}
		else{
			CUpDownClient* client = (CUpDownClient*)content->value;

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
						UpdateItem(client);
					break;
#endif
				case MP_DETAIL:
				case MPG_ALTENTER:
					ShowClientDialog(client);
					break;
				case MP_BOOT:
					if (client->GetKadPort())
						Kademlia::CKademlia::Bootstrap(ntohl(client->GetIP()), client->GetKadPort(), (client->GetKadVersion() > 1));
					break;
// ZZ:DownloadManager -->
#ifdef _DEBUG
				case MP_A4AF_CHECK_THIS_NOW: {
					CPartFile* file = (CPartFile*)content->owner;
					if (file->GetStatus(false) == PS_READY || file->GetStatus(false) == PS_EMPTY)
					{
						if (client->GetDownloadState() != DS_DOWNLOADING)
						{
							client->SwapToAnotherFile(_T("Manual init of source check. Test to be like ProcessA4AFClients(). CDownloadListCtrl::OnCommand() MP_SWAP_A4AF_DEBUG_THIS"), false, false, false, NULL, true, true, true); // ZZ:DownloadManager
							UpdateItem(file);
						}
					}
					break;
				}
#endif
// <-- ZZ:DownloadManager
			}
		}
	}
	else /*nothing selected*/
	{
		switch (wParam){
			case MP_CLEARCOMPLETED:
				ClearCompleted();
				break;
		}
	}
	
	UpdateToolBarItem(&m_ToolBarUI);
	return TRUE;
}

void CDownloadListCtrl::OnColumnClick( NMHDR* pNMHDR, LRESULT* pResult){
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	// Barry - Store sort order in preferences
	// Determine ascending based on whether already sorted on this column
	int sortItem = GetSortItem();
	bool m_oldSortAscending = GetSortAscending();

	if (sortItem==9) {
		m_bRemainSort=(sortItem != pNMListView->iSubItem) ? false : (m_oldSortAscending?m_bRemainSort:!m_bRemainSort);
	}

	bool sortAscending = (sortItem != pNMListView->iSubItem) ? true : !m_oldSortAscending;
	
	// Item is column clicked
	sortItem = pNMListView->iSubItem;
	UpdateSortHistory(sortItem + (sortAscending ? 0:100), 100);
	
	// Save new preferences
	thePrefs.TransferlistRemainSortStyle(m_bRemainSort);
	
	// Sort table
	uint8 adder=0;
	if (sortItem!=9 || !m_bRemainSort)
		SetSortArrow(sortItem, sortAscending);
	else {
        SetSortArrow(sortItem, sortAscending?arrowDoubleUp : arrowDoubleDown);
		adder=81;
	}
	

	SortItems(SortProc, sortItem + (sortAscending ? 0:100) + adder );

	*pResult = 0;
}

int CDownloadListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort){
	CtrlItem_Struct* item1 = (CtrlItem_Struct*)lParam1;
	CtrlItem_Struct* item2 = (CtrlItem_Struct*)lParam2;

	int dwOrgSort = lParamSort;

	int sortMod = 1;
	if(lParamSort >= 100) 
	{
		sortMod = -1;
		lParamSort -= 100;
	}

	int comp;

	if(item1->type == FILE_TYPE && item2->type != FILE_TYPE) 
	{
		if(item1->value == item2->parent->value)
			return -1;
		comp = Compare((CPartFile*)item1->value, (CPartFile*)(item2->parent->value), lParamSort);
	}
	else if(item2->type == FILE_TYPE && item1->type != FILE_TYPE) 
	{
		if(item1->parent->value == item2->value)
			return 1;
		comp = Compare((CPartFile*)(item1->parent->value), (CPartFile*)item2->value, lParamSort);
	}
	else if (item1->type == FILE_TYPE) 
	{
		CPartFile* file1 = (CPartFile*)item1->value;
		CPartFile* file2 = (CPartFile*)item2->value;
		comp = Compare(file1, file2, lParamSort);
	}
	else
	{
		if (item1->parent->value!=item2->parent->value) 
		{
			comp = Compare((CPartFile*)(item1->parent->value), (CPartFile*)(item2->parent->value), lParamSort);
			return sortMod * comp;
		}
		if (item1->type != item2->type)
			return item1->type - item2->type;

		CUpDownClient* client1 = (CUpDownClient*)item1->value;
		CUpDownClient* client2 = (CUpDownClient*)item2->value;
		comp = Compare(client1, client2, lParamSort);
	}
	int dwNextSort;
	//call secondary sortorder, if this one results in equal
	//(Note: yes I know this call is evil OO wise, but better than changing a lot more code, while we have only one instance anyway - might be fixed later)
	if (comp == 0 && (dwNextSort = theApp.emuledlg->transferwnd->downloadlistctrl.GetNextSortOrder(dwOrgSort)) != (-1)){
		return SortProc(lParam1, lParam2, dwNextSort);
	}
	else
		return sortMod * comp;
}

void CDownloadListCtrl::ClearCompleted(int incat){
	if (incat==-2)
		incat=curTab;

	// Search for completed file(s)
	for(ListItems::iterator it = m_ListItems.begin(); it != m_ListItems.end(); ){
		CtrlItem_Struct* cur_item = it->second;
		it++; // Already point to the next iterator. 
		if(cur_item->type == FILE_TYPE){
			CPartFile* file = reinterpret_cast<CPartFile*>(cur_item->value);
			if(file->IsPartFile() == false && (file->CheckShowItemInGivenCat(incat) || incat==-1) ){
				if (RemoveFile(file))
					it = m_ListItems.begin();
			}
		}
	}
	if (thePrefs.ShowCatTabInfos())
		theApp.emuledlg->transferwnd->UpdateCatTabTitles();
}

void CDownloadListCtrl::ClearCompleted(const CPartFile* pFile)
{
	if (!pFile->IsPartFile())
	{
		for (ListItems::iterator it = m_ListItems.begin(); it != m_ListItems.end(); )
		{
			CtrlItem_Struct* cur_item = it->second;
			it++;
			if (cur_item->type == FILE_TYPE)
			{
				const CPartFile* pCurFile = reinterpret_cast<CPartFile*>(cur_item->value);
				if (pCurFile == pFile)
				{
					RemoveFile(pCurFile);
					return;
				}
			}
		}
	}
}

void CDownloadListCtrl::SetStyle()
{
	if (thePrefs.IsDoubleClickEnabled())
		SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
	else
		SetExtendedStyle(LVS_EX_ONECLICKACTIVATE | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
}

void CDownloadListCtrl::OnListModified(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	NM_LISTVIEW *pNMListView = (NM_LISTVIEW*)pNMHDR;

	//this works because true is equal to 1 and false equal to 0
	BOOL notLast = pNMListView->iItem + 1 != GetItemCount();
	BOOL notFirst = pNMListView->iItem != 0;
	RedrawItems(pNMListView->iItem - notFirst, pNMListView->iItem + notLast);

	if(pNMListView->uChanged != LVIF_STATE || (pNMListView->uOldState & LVIS_SELECTED) == (pNMListView->uNewState & LVIS_SELECTED))
		return;

	//Toolbar
	int iSel = GetNextItem(-1, LVIS_SELECTED);

	if (iSel != -1)
	{
		const CtrlItem_Struct* content = (CtrlItem_Struct*)GetItemData(iSel);
		//ASSERT(content);
		if(!content) 
			return;

		if (content->type == FILE_TYPE)
		{
			bool bFirstItem				= true;
			int iSelectedItems			= 0;
			//int iFilesNotDone			= 0;
			int iFilesToPause			= 0;
			int iFilesToStop			= 0;
			int iFilesToResume			= 0;
			int iFilesToOpen			= 0;
			//int iFilesGetPreviewParts	= 0;
			//int iFilesPreviewType		= 0;
			//int iFilesToPreview			= 0;
			int iFilesToCancel			= 0;
			int iFilesToOpenFolder		= 0;
			//UINT uPrioMenuItem			= 0;

			const CPartFile* file1 = NULL;
			POSITION pos = GetFirstSelectedItemPosition();

			while (pos)
			{
				const CtrlItem_Struct* pItemData = (CtrlItem_Struct*)GetItemData(GetNextSelectedItem(pos));

				if (pItemData==NULL || pItemData->type != FILE_TYPE)
				{
					continue;
				}

				const CPartFile* pFile = (CPartFile*)pItemData->value;

				if (bFirstItem)
				{
					file1 = pFile;
				}

				iSelectedItems++;

				//bool bFileDone = (pFile->GetStatus() == PS_COMPLETE || pFile->GetStatus() == PS_COMPLETING);
				iFilesToCancel += pFile->GetStatus() != PS_COMPLETING ? 1 : 0;
				//iFilesNotDone += !bFileDone ? 1 : 0;
				iFilesToStop += pFile->CanStopFile() ? 1 : 0;
				iFilesToPause += pFile->CanPauseFile() ? 1 : 0;
				iFilesToResume += pFile->CanResumeFile() ? 1 : 0;
				iFilesToOpen += pFile->CanOpenFile() ? 1 : 0;
				//iFilesGetPreviewParts += pFile->GetPreviewPrio() ? 1 : 0;
				//iFilesPreviewType += pFile->IsPreviewableFileType() ? 1 : 0;
				//iFilesToPreview += pFile->IsReadyForPreview() ? 1 : 0;
				iFilesToOpenFolder += pFile->GetStatus()==PS_COMPLETE ? 1 : 0;
				bFirstItem = false;
			}

			((CDownloadTabWnd*)GetParent())->m_Toolbar.EnableButton(MP_CANCEL, (iFilesToCancel ? TRUE : FALSE));
			((CDownloadTabWnd*)GetParent())->m_Toolbar.EnableButton(MP_STOP, (iFilesToStop ? TRUE : FALSE));
			((CDownloadTabWnd*)GetParent())->m_Toolbar.EnableButton(MP_PAUSE, (iFilesToPause ? TRUE : FALSE));
			((CDownloadTabWnd*)GetParent())->m_Toolbar.EnableButton(MP_RESUME, (iFilesToResume ? TRUE : FALSE));
			((CDownloadTabWnd*)GetParent())->m_Toolbar.EnableButton(MP_OPENFOLDER, ( (iSelectedItems==1 && iFilesToOpenFolder==1) ? TRUE : FALSE));

			m_ToolBarUI.iFilesToCancel = iFilesToCancel;
			m_ToolBarUI.iFilesToPause = iFilesToPause;
			m_ToolBarUI.iFilesToResume = iFilesToResume;
			m_ToolBarUI.iFilesToStop = iFilesToStop;
			m_ToolBarUI.iFilesToOpenFolder = iFilesToOpenFolder;

			if (iSelectedItems == 1)
			{
				theWndMgr.SendMsgTo(CWndMgr::WI_MAINTAB_DOWNLOAD_DLG, UM_MTDD_CUR_SEL_FILE, 0, (LPARAM) content->value);
			}
		}
		else
		{
			//Peerlog
			theWndMgr.SendMsgTo(CWndMgr::WI_MAINTAB_DOWNLOAD_DLG, UM_MTDD_CUR_SEL_PEER, 0, (LPARAM)content);
		}
	}
	else
	{	
		theWndMgr.SendMsgTo(CWndMgr::WI_MAINTAB_DOWNLOAD_DLG, UM_MTDD_CUR_SEL_FILE, 0, 0);

		((CDownloadTabWnd*)GetParent())->m_Toolbar.EnableButton(MP_CANCEL, FALSE);
		((CDownloadTabWnd*)GetParent())->m_Toolbar.EnableButton(MP_STOP, FALSE);
		((CDownloadTabWnd*)GetParent())->m_Toolbar.EnableButton(MP_PAUSE, FALSE);
		((CDownloadTabWnd*)GetParent())->m_Toolbar.EnableButton(MP_RESUME, FALSE);

		m_ToolBarUI.iFilesToCancel = 0;
		m_ToolBarUI.iFilesToPause = 0;
		m_ToolBarUI.iFilesToResume = 0;
		m_ToolBarUI.iFilesToStop = 0;
	}
	((CDownloadTabWnd*)GetParent())->m_Toolbar.Invalidate();
}

int CDownloadListCtrl::Compare(const CPartFile* file1, const CPartFile* file2, LPARAM lParamSort)
{
	if(file1==NULL||file2==NULL)
		return 0;
	int comp=0;
	switch(lParamSort)
	{
		case 0: //filename asc
			comp=CompareLocaleStringNoCase(file1->GetFileName(),file2->GetFileName());
			break;
		case 2: //size asc
			comp=CompareUnsigned64(file1->GetFileSize(), file2->GetFileSize());
			break;
		case 3: //transferred asc
			comp=CompareUnsigned64(file1->GetTransferred(), file2->GetTransferred());
			break;
		case 4: //completed asc
			comp=CompareUnsigned64(file1->GetCompletedSize(), file2->GetCompletedSize());
			break;
		case 5: //speed asc
			comp=CompareUnsigned(file1->GetDatarate(), file2->GetDatarate());
			break;
		case 6: //progress asc
			comp = CompareFloat(file1->GetPercentCompleted(), file2->GetPercentCompleted());
			break;
		case 7: //sources asc
			comp=CompareUnsigned(file1->GetSourceCount(), file2->GetSourceCount());
			break;
		case 8: //priority asc
			comp=CompareUnsigned(file1->GetDownPriority(), file2->GetDownPriority());
			break;
		case 9: //Status asc 
			comp=CompareUnsigned(file1->getPartfileStatusRang(),file2->getPartfileStatusRang());
			break;
		case 10: //Remaining Time asc
		{
			//Make ascending sort so we can have the smaller remaining time on the top 
			//instead of unknowns so we can see which files are about to finish better..
			time_t f1 = file1->getTimeRemaining();
			time_t f2 = file2->getTimeRemaining();
			//Same, do nothing.
			if( f1 == f2 ) 
			{
				comp=0;
				break;
			}
			//If descending, put first on top as it is unknown
			//If ascending, put first on bottom as it is unknown
			if( f1 == -1 ) 
			{
				comp=1;
				break;
			}
			//If descending, put second on top as it is unknown
			//If ascending, put second on bottom as it is unknown
			if( f2 == -1 ) 
			{
				comp=-1;
				break;
			}
			//If descending, put first on top as it is bigger.
			//If ascending, put first on bottom as it is bigger.
			comp = CompareUnsigned(f1, f2);
			break;
		}
		case 90: //Remaining SIZE asc
			comp=CompareUnsigned64(file1->GetFileSize()-file1->GetCompletedSize(), file2->GetFileSize()-file2->GetCompletedSize());
			break;
		case 11: //last seen complete asc
			if (file1->lastseencomplete > file2->lastseencomplete)
				comp=1;
			else if(file1->lastseencomplete < file2->lastseencomplete)
				comp=-1;
			else
				comp=0;
			break;
		case 12: //last received Time asc
			if (file1->GetFileDate() > file2->GetFileDate())
				comp=1;
			else if(file1->GetFileDate() < file2->GetFileDate())
				comp=-1;
			else
				comp=0;
			break;
		case 13:
			//TODO: 'GetCategory' SHOULD be a 'const' function and 'GetResString' should NOT be called..
			comp=CompareLocaleStringNoCase(	(const_cast<CPartFile*>(file1)->GetCategory()!=0)?thePrefs.GetCategory(const_cast<CPartFile*>(file1)->GetCategory())->strTitle:GetResString(IDS_ALL),
											(const_cast<CPartFile*>(file2)->GetCategory()!=0)?thePrefs.GetCategory(const_cast<CPartFile*>(file2)->GetCategory())->strTitle:GetResString(IDS_ALL) );
			break;
		default:
			comp=0;
	}
	return comp;
 }

int CDownloadListCtrl::Compare(const CUpDownClient *client1, const CUpDownClient *client2, LPARAM lParamSort)
{
	if(client1==NULL||client2==NULL)
		return 0;
	switch (lParamSort)
	{
	case 0: //name asc
		if (client1->GetUserName() == client2->GetUserName())
			return 0;
		else if (!client1->GetUserName())	// place clients with no usernames at bottom
			return 1;
		else if (!client2->GetUserName())	// place clients with no usernames at bottom
			return -1;
		return CompareLocaleStringNoCase(client1->GetUserName(), client2->GetUserName());

	//case 1:	//comment	//MODIFIED by fengwen on 2006/09/30 : 由于插入了“评论列”，所以后面的都往后错一列。
		//return 0;

	case 2: //size but we use status asc
		return client1->GetSourceFrom() - client2->GetSourceFrom();

	case 3://transferred asc
	case 4://completed asc
		return CompareUnsigned(client1->GetTransferredDown(), client2->GetTransferredDown());

	case 5: //speed asc
		return CompareUnsigned(client1->GetDownloadDatarate(), client2->GetDownloadDatarate());

	case 6: //progress asc
		return CompareUnsigned(client1->GetAvailablePartCount(), client2->GetAvailablePartCount());

	case 7:
		if (client1->GetClientSoft() == client2->GetClientSoft())
			return CompareUnsigned(client2->GetVersion(), client1->GetVersion());
		return CompareUnsigned(client1->GetClientSoft(), client2->GetClientSoft());
	case 8: //qr asc
		if (client1->GetDownloadState() == DS_DOWNLOADING){
			if (client2->GetDownloadState() == DS_DOWNLOADING)
				return 0;
			else
				return -1;
		}
		else if (client2->GetDownloadState() == DS_DOWNLOADING)
			return 1;
		if (client1->GetRemoteQueueRank() == 0 
			&& client1->GetDownloadState() == DS_ONQUEUE && client1->IsRemoteQueueFull() == true)
			return 1;
		if (client2->GetRemoteQueueRank() == 0 
			&& client2->GetDownloadState() == DS_ONQUEUE && client2->IsRemoteQueueFull() == true)
			return -1;
		if (client1->GetRemoteQueueRank() == 0)
			return 1;
		if (client2->GetRemoteQueueRank() == 0)
			return -1;
		return CompareUnsigned(client1->GetRemoteQueueRank(), client2->GetRemoteQueueRank());

	case 9:
		if (client1->GetDownloadState() == client2->GetDownloadState()){
			if (client1->IsRemoteQueueFull() && client2->IsRemoteQueueFull())
				return 0;
			else if (client1->IsRemoteQueueFull())
				return 1;
			else if (client2->IsRemoteQueueFull())
				return -1;
			else if ((client1->socket && client1->socket->m_bUseNat) != (client2->socket && client2->socket->m_bUseNat))
				return (client1->socket && client1->socket->m_bUseNat) - (client2->socket && client2->socket->m_bUseNat);
		}
		return client1->GetDownloadState() - client2->GetDownloadState();

	default:
		return 0;
	}
}

void CDownloadListCtrl::OnNMDblclkDownloadlist(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	int iSel = GetSelectionMark();
	if (iSel != -1)
	{
		const CtrlItem_Struct* content = (CtrlItem_Struct*)GetItemData(iSel);
		if (content && content->value)
		{
			if (content->type == FILE_TYPE)
			{
				if (!thePrefs.IsDoubleClickEnabled())
				{
					CPoint pt;
					::GetCursorPos(&pt);
					ScreenToClient(&pt);
					LVHITTESTINFO hit;
					hit.pt = pt;
					if (HitTest(&hit) >= 0 && (hit.flags & LVHT_ONITEM))
					{
						LVHITTESTINFO subhit;
						subhit.pt = pt;
						if (SubItemHitTest(&subhit) >= 0 && subhit.iSubItem == 0)
						{
							CPartFile* file = (CPartFile*)content->value;
							if (thePrefs.ShowRatingIndicator() 
								&& (file->HasComment() || file->HasRating() || file->IsKadCommentSearchRunning()) 
								&& pt.x >= FILE_ITEM_MARGIN_X+theApp.GetSmallSytemIconSize().cx 
								&& pt.x <= FILE_ITEM_MARGIN_X+theApp.GetSmallSytemIconSize().cx+RATING_ICON_WIDTH)
								ShowFileDialog(IDD_COMMENTLST);
							else if (thePrefs.GetPreviewOnIconDblClk()
									 && pt.x >= FILE_ITEM_MARGIN_X 
									 && pt.x < FILE_ITEM_MARGIN_X+theApp.GetSmallSytemIconSize().cx) {
								if (file->IsReadyForPreview())
									file->PreviewFile();
								else
									MessageBeep(MB_OK);
							}
							else
								ShowFileDialog(0);
						}
					}
				}
			}
			else
			{
				ShowClientDialog((CUpDownClient*)content->value);
			}
		}
	}
	
	*pResult = 0;
}

void CDownloadListCtrl::CreateMenues()
{
	if (m_PrioMenu)
		VERIFY( m_PrioMenu.DestroyMenu() );
	if (m_SourcesMenu)
		VERIFY( m_SourcesMenu.DestroyMenu() );
	if (m_FileMenu)
		VERIFY( m_FileMenu.DestroyMenu() );

	m_FileMenu.CreatePopupMenu();
	//m_FileMenu.AddMenuTitle(GetResString(IDS_DOWNLOADMENUTITLE), true);

	// Add 'Download Priority' sub menu
	//
	m_PrioMenu.CreateMenu();
	//m_PrioMenu.AddMenuTitle(NULL, true);
	m_PrioMenu.AppendMenu(MF_STRING, MP_PRIOLOW, GetResString(IDS_PRIOLOW));
	m_PrioMenu.AppendMenu(MF_STRING, MP_PRIONORMAL, GetResString(IDS_PRIONORMAL));
	m_PrioMenu.AppendMenu(MF_STRING, MP_PRIOHIGH, GetResString(IDS_PRIOHIGH));
	m_PrioMenu.AppendMenu(MF_STRING, MP_PRIOAUTO, GetResString(IDS_PRIOAUTO));
	m_FileMenu.AppendMenu(MF_STRING|MF_POPUP, (UINT_PTR)m_PrioMenu.m_hMenu, GetResString(IDS_PRIORITY) + _T(" (") + GetResString(IDS_DOWNLOAD) + _T(")"), _T("FILEPRIORITY"));
	cif.AddIcon((UINT_PTR)m_PrioMenu.m_hMenu, theApp.LoadIcon(_T("FILEPRIORITY")));
	cif.AddIcon((UINT_PTR)m_PrioMenu.m_hMenu, theApp.LoadIcon(_T("FILEPRIORITY")), TRUE);

	// Add file commands
	//
	m_FileMenu.AppendMenu(MF_STRING, MP_PAUSE, GetResString(IDS_DL_PAUSE), _T("PAUSE"));
	m_FileMenu.AppendMenu(MF_STRING, MP_STOP, GetResString(IDS_DL_STOP), _T("STOP"));
	m_FileMenu.AppendMenu(MF_STRING, MP_RESUME, GetResString(IDS_DL_RESUME), _T("RESUME"));
#ifdef _PLAY_WHILE_DOWNLOADING
	m_FileMenu.AppendMenu(MF_STRING, MP_PLAY, GetResString(IDS_DL_PLAY), _T("PLAY")); // VC-SearchDream[2007-05-16]: for see movie while downloading
#endif
	m_FileMenu.AppendMenu(MF_STRING, MP_CANCEL, GetResString(IDS_DELETE_FILE), _T("DELETE"));
	m_FileMenu.AppendMenu(MF_SEPARATOR);
	m_FileMenu.AppendMenu(MF_STRING, MP_OPEN, GetResString(IDS_DL_OPEN), _T("OPENFILE"));
	if (thePrefs.IsExtControlsEnabled() && !thePrefs.GetPreviewPrio())
    	m_FileMenu.AppendMenu(MF_STRING, MP_TRY_TO_GET_PREVIEW_PARTS, GetResString(IDS_DL_TRY_TO_GET_PREVIEW_PARTS));
	m_FileMenu.AppendMenu(MF_STRING, MP_PREVIEW, GetResString(IDS_DL_PREVIEW), _T("PREVIEW"));
	m_FileMenu.AppendMenu(MF_STRING, MP_METINFO, GetResString(IDS_DL_INFO), _T("FILEINFO"));
	m_FileMenu.AppendMenu(MF_STRING, MP_VIEWFILECOMMENTS, GetResString(IDS_CMT_SHOWALL), _T("FILECOMMENTS"));
	m_FileMenu.AppendMenu(MF_SEPARATOR);
	m_FileMenu.AppendMenu(MF_STRING, MP_CLEARCOMPLETED, GetResString(IDS_DL_CLEAR), _T("CLEARCOMPLETE"));

	// Add (extended user mode) 'Source Handling' sub menu
	//
//	if (thePrefs.IsExtControlsEnabled()) {
		m_SourcesMenu.CreateMenu();
		m_SourcesMenu.AppendMenu(MF_STRING, MP_ADDSOURCE, GetResString(IDS_ADDSRCMANUALLY));
		m_SourcesMenu.AppendMenu(MF_STRING, MP_SETSOURCELIMIT, GetResString(IDS_SETPFSLIMIT));
		m_FileMenu.AppendMenu(MF_STRING|MF_POPUP, (UINT_PTR)m_SourcesMenu.m_hMenu, GetResString(IDS_A4AF));
//	}
	m_FileMenu.AppendMenu(MF_SEPARATOR);

	// Add 'Copy & Paste' commands
	//
	if (thePrefs.GetShowCopyEd2kLinkCmd())
		m_FileMenu.AppendMenu(MF_STRING, MP_GETED2KLINK, GetResString(IDS_DL_LINK1), _T("ED2KLINK"));
	else
		m_FileMenu.AppendMenu(MF_STRING, MP_SHOWED2KLINK, GetResString(IDS_DL_SHOWED2KLINK), _T("ED2KLINK"));
	m_FileMenu.AppendMenu(MF_STRING, MP_PASTE, GetResString(IDS_SW_DIRECTDOWNLOAD), _T("PASTELINK"));
	m_FileMenu.AppendMenu(MF_SEPARATOR);

	// Search commands
	//
	m_FileMenu.AppendMenu(MF_STRING, MP_FIND, GetResString(IDS_FIND), _T("Search"));
	//m_FileMenu.AppendMenu(MF_STRING, MP_SEARCHRELATED, GetResString(IDS_SEARCHRELATED), _T("KadFileSearch"));
	// Web-services and categories will be added on-the-fly..
}

CString CDownloadListCtrl::getTextList()
{
	CString out;

	for (ListItems::iterator it = m_ListItems.begin(); it != m_ListItems.end(); it++)
	{
		const CtrlItem_Struct* cur_item = it->second;
		if (cur_item->type == FILE_TYPE)
		{
			const CPartFile* file = reinterpret_cast<CPartFile*>(cur_item->value);

			CString temp;
			temp.Format(_T("\n%s\t [%.1f%%] %i/%i - %s"),
						file->GetFileName(),
						file->GetPercentCompleted(),
						file->GetTransferringSrcCount(),
						file->GetSourceCount(), 
						file->getPartfileStatus());

			out += temp;
		}
	}

	return out;
}

int CDownloadListCtrl::GetFilesCountInCurCat()
{
	int iCount = 0;
	for (ListItems::const_iterator it = m_ListItems.begin(); it != m_ListItems.end(); it++)
	{
		CtrlItem_Struct* cur_item = it->second;
		if (cur_item->type == FILE_TYPE)
		{
			CPartFile* file = (CPartFile*)cur_item->value;
			if (file->CheckShowItemInGivenCat(curTab))
				iCount++;
		}
	}
	return iCount;
}

void CDownloadListCtrl::ShowFilesCount()
{
	theApp.emuledlg->transferwnd->UpdateFilesCount(GetFilesCountInCurCat());
}

void CDownloadListCtrl::ShowSelectedFileDetails()
{
	POINT point;
	::GetCursorPos(&point);
	CPoint pt = point; 
    ScreenToClient(&pt); 
    int it = HitTest(pt);
    if (it == -1)
		return;

	SetItemState(-1, 0, LVIS_SELECTED);
	SetItemState(it, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	SetSelectionMark(it);   // display selection mark correctly! 

	CtrlItem_Struct* content = (CtrlItem_Struct*)GetItemData(GetSelectionMark());
	if (content->type == FILE_TYPE)
	{
		CPartFile* file = (CPartFile*)content->value;
		if (thePrefs.ShowRatingIndicator() 
			&& (file->HasComment() || file->HasRating() || file->IsKadCommentSearchRunning()) 
			&& pt.x >= FILE_ITEM_MARGIN_X+theApp.GetSmallSytemIconSize().cx 
			&& pt.x <= FILE_ITEM_MARGIN_X+theApp.GetSmallSytemIconSize().cx+RATING_ICON_WIDTH)
			ShowFileDialog(IDD_COMMENTLST);
		else
			ShowFileDialog(0);
	}
	else
	{
		ShowClientDialog((CUpDownClient*)content->value);
	}
}

int CDownloadListCtrl::GetCompleteDownloads(int cat, int& total)
{
	total = 0;
	int count = 0;
	for (ListItems::const_iterator it = m_ListItems.begin(); it != m_ListItems.end(); it++)
	{
		const CtrlItem_Struct* cur_item = it->second;
		if (cur_item->type == FILE_TYPE)
		{
			/*const*/ CPartFile* file = reinterpret_cast<CPartFile*>(cur_item->value);
			if (file->CheckShowItemInGivenCat(cat) || cat==-1)
			{
				total++;
				if (file->GetStatus() == PS_COMPLETE)
					count++;
			}
		}
	}
	return count;
}

void CDownloadListCtrl::UpdateCurrentCategoryView(){
	ChangeCategory(curTab);
}

void CDownloadListCtrl::UpdateCurrentCategoryView(CPartFile* thisfile) {

	ListItems::const_iterator it = m_ListItems.find(thisfile);
	if (it != m_ListItems.end()) {
		const CtrlItem_Struct* cur_item = it->second;
		if (cur_item->type == FILE_TYPE){
			CPartFile* file = reinterpret_cast<CPartFile*>(cur_item->value);
			
			if (!file->CheckShowItemInGivenCat(curTab))
				HideFile(file);
			else
				ShowFile(file);
		}
	}

}

void CDownloadListCtrl::ChangeCategory(int newsel){

	SetRedraw(FALSE);

	// remove all displayed files with a different cat and show the correct ones
	for(ListItems::const_iterator it = m_ListItems.begin(); it != m_ListItems.end(); it++){
		const CtrlItem_Struct* cur_item = it->second;
		if (cur_item->type == FILE_TYPE){
			CPartFile* file = reinterpret_cast<CPartFile*>(cur_item->value);
			
			if (!file->CheckShowItemInGivenCat(newsel))
				HideFile(file);
			else
				ShowFile(file);
		}
	}

	SetRedraw(TRUE);
	curTab=newsel;
	ShowFilesCount();
}

void CDownloadListCtrl::HideFile(CPartFile* tohide)
{
	HideSources(tohide);

	// Retrieve all entries matching the source
	std::pair<ListItems::const_iterator, ListItems::const_iterator> rangeIt = m_ListItems.equal_range(tohide);
	for(ListItems::const_iterator it = rangeIt.first; it != rangeIt.second; it++){
		CtrlItem_Struct* updateItem  = it->second;

		// Find entry in CListCtrl and update object
 		LVFINDINFO find;
		find.flags = LVFI_PARAM;
		find.lParam = (LPARAM)updateItem;
		int result = FindItem(&find);
		if (result != -1){
			DeleteItem(result);
			return;
		}
	}
}

void CDownloadListCtrl::ShowFile(CPartFile* toshow){
	// Retrieve all entries matching the source
	std::pair<ListItems::const_iterator, ListItems::const_iterator> rangeIt = m_ListItems.equal_range(toshow);
	ListItems::const_iterator it = rangeIt.first;
	if(it != rangeIt.second){
		CtrlItem_Struct* updateItem  = it->second;

		// Check if entry is already in the List
 		LVFINDINFO find;
		find.flags = LVFI_PARAM;
		find.lParam = (LPARAM)updateItem;
		int result = FindItem(&find);
		if (result == -1)
			InsertItem(LVIF_PARAM|LVIF_TEXT,GetItemCount(),LPSTR_TEXTCALLBACK,0,0,0,(LPARAM)updateItem);
	}
}

void CDownloadListCtrl::GetDisplayedFiles(CArray<CPartFile*,CPartFile*> *list){
	for(ListItems::iterator it = m_ListItems.begin(); it != m_ListItems.end(); ){
		CtrlItem_Struct* cur_item = it->second;
		it++; // Already point to the next iterator. 
		if(cur_item->type == FILE_TYPE){
			CPartFile* file = reinterpret_cast<CPartFile*>(cur_item->value);
			list->Add(file);
		}
	}	
}

void CDownloadListCtrl::MoveCompletedfilesCat(uint8 from, uint8 to)
{
	int mycat;

	for(ListItems::iterator it = m_ListItems.begin(); it != m_ListItems.end(); ){
		CtrlItem_Struct* cur_item = it->second;
		it++; // Already point to the next iterator.
		if(cur_item->type == FILE_TYPE){
			CPartFile* file = reinterpret_cast<CPartFile*>(cur_item->value);
			if (!file->IsPartFile()){
				mycat=file->GetCategory();
				if ( mycat>=min(from,to) && mycat<=max(from,to)) {
					if (mycat==from) 
						file->SetCategory(to); 
					else
						if (from<to)
							file->SetCategory(mycat-1);
						else
							file->SetCategory(mycat+1);
				}
			}
		}
	}
}

void CDownloadListCtrl::OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult)
{
    NMLVDISPINFO* pDispInfo = (NMLVDISPINFO*)pNMHDR;
	/*TRACE("CDownloadListCtrl::OnGetDispInfo iItem=%d iSubItem=%d", pDispInfo->item.iItem, pDispInfo->item.iSubItem);
	if (pDispInfo->item.mask & LVIF_TEXT)
		TRACE(" LVIF_TEXT");
	if (pDispInfo->item.mask & LVIF_IMAGE)
		TRACE(" LVIF_IMAGE");
	if (pDispInfo->item.mask & LVIF_STATE)
		TRACE(" LVIF_STATE");
	TRACE("\n");*/

	// Although we have an owner drawn listview control we store the text for the primary item in the listview, to be
	// capable of quick searching those items via the keyboard. Because our listview items may change their contents,
	// we do this via a text callback function. The listview control will send us the LVN_DISPINFO notification if
	// it needs to know the contents of the primary item.
	//
	// But, the listview control sends this notification all the time, even if we do not search for an item. At least
	// this notification is only sent for the visible items and not for all items in the list. Though, because this
	// function is invoked *very* often, no *NOT* put any time consuming code here in.

    if (pDispInfo->item.mask & LVIF_TEXT){
        const CtrlItem_Struct* pItem = reinterpret_cast<CtrlItem_Struct*>(pDispInfo->item.lParam);
        if (pItem != NULL && pItem->value != NULL){
			if (pItem->type == FILE_TYPE){
				switch (pDispInfo->item.iSubItem){
					case 0:
						if (pDispInfo->item.cchTextMax > 0){
							_tcsncpy(pDispInfo->item.pszText, ((CPartFile*)pItem->value)->GetFileName(), pDispInfo->item.cchTextMax);
							pDispInfo->item.pszText[pDispInfo->item.cchTextMax-1] = _T('\0');
						}
						break;
					default:
						// shouldn't happen
						pDispInfo->item.pszText[0] = _T('\0');
						break;
				}
			}
			else if (pItem->type == UNAVAILABLE_SOURCE || pItem->type == AVAILABLE_SOURCE){
				switch (pDispInfo->item.iSubItem){
					case 0:
						if (((CUpDownClient*)pItem->value)->GetUserName() != NULL && pDispInfo->item.cchTextMax > 0){
							_tcsncpy(pDispInfo->item.pszText, ((CUpDownClient*)pItem->value)->GetUserName(), pDispInfo->item.cchTextMax);
							pDispInfo->item.pszText[pDispInfo->item.cchTextMax-1] = _T('\0');
						}
						break;
					default:
						// shouldn't happen
						pDispInfo->item.pszText[0] = _T('\0');
						break;
				}
			}
			else
				ASSERT(0);
        }
    }
    *pResult = 0;
}

void CDownloadListCtrl::OnLvnGetInfoTip(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVGETINFOTIP pGetInfoTip = reinterpret_cast<LPNMLVGETINFOTIP>(pNMHDR);
	if (pGetInfoTip->iSubItem == 0)
	{
		LVHITTESTINFO hti = {0};
		::GetCursorPos(&hti.pt);
		ScreenToClient(&hti.pt);
		if (SubItemHitTest(&hti) == -1 || hti.iItem != pGetInfoTip->iItem || hti.iSubItem != 0){
			// don' show the default label tip for the main item, if the mouse is not over the main item
			if ((pGetInfoTip->dwFlags & LVGIT_UNFOLDED) == 0 && pGetInfoTip->cchTextMax > 0 && pGetInfoTip->pszText[0] != '\0')
				pGetInfoTip->pszText[0] = '\0';
			return;
		}

		const CtrlItem_Struct* content = (CtrlItem_Struct*)GetItemData(pGetInfoTip->iItem);
		if (content && pGetInfoTip->pszText && pGetInfoTip->cchTextMax > 0)
		{
			CString info;

			// build info text and display it
			if (content->type == FILE_TYPE) // for downloading files
			{
				const CPartFile* partfile = (CPartFile*)content->value;
				info = partfile->GetInfoSummary();
			}
			else if (content->type == UNAVAILABLE_SOURCE || content->type == AVAILABLE_SOURCE) // for sources
			{
				const CUpDownClient* client = (CUpDownClient*)content->value;
				if (client->IsEd2kClient())
				{
					in_addr server;
					server.S_un.S_addr = client->GetServerIP();
					info.Format(GetResString(IDS_USERINFO)
								+ GetResString(IDS_SERVER) + _T(":%s:%u\n\n")
								+ GetResString(IDS_NEXT_REASK) + _T(":%s"),
								client->GetUserName() ? client->GetUserName() : _T("?"),
								ipstr(server), client->GetServerPort(),
								CastSecondsToHM(client->GetTimeUntilReask(client->GetRequestFile()) / 1000));
					if (thePrefs.IsExtControlsEnabled())
						info.AppendFormat(_T(" (%s)"), CastSecondsToHM(client->GetTimeUntilReask(content->owner) / 1000));
					info += _T('\n');
					info.AppendFormat(GetResString(IDS_SOURCEINFO), client->GetAskedCountDown(), client->GetAvailablePartCount());
					info += _T('\n');

					if (content->type == AVAILABLE_SOURCE)
					{
						info += GetResString(IDS_CLIENTSOURCENAME) + (!client->GetClientFilename().IsEmpty() ? client->GetClientFilename() : _T("-"));
						if (!client->GetFileComment().IsEmpty())
							info += _T('\n') + GetResString(IDS_CMT_READ) + _T(' ') + client->GetFileComment();
						if (client->GetFileRating())
							info += _T('\n') + GetResString(IDS_QL_RATING) + _T(':') + GetRateString(client->GetFileRating());
					}
					else
					{	// client asked twice
						info += GetResString(IDS_ASKEDFAF);
                        if (client->GetRequestFile() && client->GetRequestFile()->GetFileName())
                            info.AppendFormat(_T(":%s"), client->GetRequestFile()->GetFileName());
					}

                    if (thePrefs.IsExtControlsEnabled() && !client->m_OtherRequests_list.IsEmpty())
					{
						CSimpleArray<const CString*> apstrFileNames;
						POSITION pos = client->m_OtherRequests_list.GetHeadPosition();
						while (pos)
							apstrFileNames.Add(&client->m_OtherRequests_list.GetNext(pos)->GetFileName());
						Sort(apstrFileNames);
						if (content->type == AVAILABLE_SOURCE)
							info += _T('\n');
						info += _T('\n');
						info += GetResString(IDS_A4AF_FILES);
						info += _T(':');
						for (int i = 0; i < apstrFileNames.GetSize(); i++)
						{
							const CString* pstrFileName = apstrFileNames[i];
							if (info.GetLength() + (i > 0 ? 2 : 0) + pstrFileName->GetLength() >= pGetInfoTip->cchTextMax) {
								static const TCHAR szEllipses[] = _T("\n:...");
								if (info.GetLength() + (int)ARRSIZE(szEllipses) - 1 < pGetInfoTip->cchTextMax)
									info += szEllipses;
								break;
							}
							if (i > 0)
								info += _T("\n:");
							info += *pstrFileName;
						}
                    }
				}
				else
				{
					info.Format(_T("URL:%s\nAvailable parts:%u"), client->GetUserName(), client->GetAvailablePartCount());
				}
			}

			_tcsncpy(pGetInfoTip->pszText, info, pGetInfoTip->cchTextMax);
			pGetInfoTip->pszText[pGetInfoTip->cchTextMax-1] = _T('\0');
		}
	}
	*pResult = 0;
}

void CDownloadListCtrl::ShowFileDialog(UINT uInvokePage)
{
	CSimpleArray<CPartFile*> aFiles;
	POSITION pos = GetFirstSelectedItemPosition();
	while (pos != NULL)
	{
		int iItem = GetNextSelectedItem(pos);
		if (iItem != -1)
		{
			const CtrlItem_Struct* pCtrlItem = (CtrlItem_Struct*)GetItemData(iItem);
			if (pCtrlItem->type == FILE_TYPE)
				aFiles.Add((CPartFile*)pCtrlItem->value);
		}
	}

	if (aFiles.GetSize() > 0)
	{
		CDownloadListListCtrlItemWalk::SetItemType(FILE_TYPE);
		CFileDetailDialog dialog(&aFiles, uInvokePage, this);
		dialog.DoModal();
	}
}

CDownloadListListCtrlItemWalk::CDownloadListListCtrlItemWalk(CDownloadListCtrl* pListCtrl)
	: CListCtrlItemWalk(pListCtrl)
{
	m_pDownloadListCtrl = pListCtrl;
	m_eItemType = (ItemType)-1;
}

CObject* CDownloadListListCtrlItemWalk::GetPrevSelectableItem()
{
	ASSERT( m_pDownloadListCtrl != NULL );
	if (m_pDownloadListCtrl == NULL)
		return NULL;
	ASSERT( m_eItemType != (ItemType)-1 );

	int iItemCount = m_pDownloadListCtrl->GetItemCount();
	if (iItemCount >= 2)
	{
		POSITION pos = m_pDownloadListCtrl->GetFirstSelectedItemPosition();
		if (pos)
		{
			int iItem = m_pDownloadListCtrl->GetNextSelectedItem(pos);
			int iCurSelItem = iItem;
			while (iItem-1 >= 0)
			{
				iItem--;

				const CtrlItem_Struct* ctrl_item = (CtrlItem_Struct*)m_pDownloadListCtrl->GetItemData(iItem);
				if (ctrl_item->type == m_eItemType || (m_eItemType != FILE_TYPE && ctrl_item->type != FILE_TYPE))
				{
					m_pDownloadListCtrl->SetItemState(iCurSelItem, 0, LVIS_SELECTED | LVIS_FOCUSED);
					m_pDownloadListCtrl->SetItemState(iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
					m_pDownloadListCtrl->SetSelectionMark(iItem);
					m_pDownloadListCtrl->EnsureVisible(iItem, FALSE);
					return STATIC_DOWNCAST(CObject, (CObject*)ctrl_item->value);
				}
			}
		}
	}
	return NULL;
}

CObject* CDownloadListListCtrlItemWalk::GetNextSelectableItem()
{
	ASSERT( m_pDownloadListCtrl != NULL );
	if (m_pDownloadListCtrl == NULL)
		return NULL;
	ASSERT( m_eItemType != (ItemType)-1 );

	int iItemCount = m_pDownloadListCtrl->GetItemCount();
	if (iItemCount >= 2)
	{
		POSITION pos = m_pDownloadListCtrl->GetFirstSelectedItemPosition();
		if (pos)
		{
			int iItem = m_pDownloadListCtrl->GetNextSelectedItem(pos);
			int iCurSelItem = iItem;
			while (iItem+1 < iItemCount)
			{
				iItem++;

				const CtrlItem_Struct* ctrl_item = (CtrlItem_Struct*)m_pDownloadListCtrl->GetItemData(iItem);
				if (ctrl_item->type == m_eItemType || (m_eItemType != FILE_TYPE && ctrl_item->type != FILE_TYPE))
				{
					m_pDownloadListCtrl->SetItemState(iCurSelItem, 0, LVIS_SELECTED | LVIS_FOCUSED);
					m_pDownloadListCtrl->SetItemState(iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
					m_pDownloadListCtrl->SetSelectionMark(iItem);
					m_pDownloadListCtrl->EnsureVisible(iItem, FALSE);
					return STATIC_DOWNCAST(CObject, (CObject*)ctrl_item->value);
				}
			}
		}
	}
	return NULL;
}

void CDownloadListCtrl::ShowClientDialog(CUpDownClient* pClient)
{
	CDownloadListListCtrlItemWalk::SetItemType(AVAILABLE_SOURCE); // just set to something !=FILE_TYPE
	CClientDetailDialog dialog(pClient, this);
	dialog.DoModal();
}

//Added by thilon and Chocobo on 2006.08.31
void CDownloadListCtrl::OnCommentClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	LVHITTESTINFO test;
	test.pt = pNMListView->ptAction;
	test.flags = 0;
	test.iItem = -1;
	test.iSubItem = -1;

	if(ListView_SubItemHitTest(m_hWnd, &test) == -1)
	{
		return;
	}

	CtrlItem_Struct* content = (CtrlItem_Struct*)GetItemData(test.iItem);
	if (content->type == FILE_TYPE && test.iSubItem == 1)
	{
		CPartFile* file = (CPartFile*)content->value;
		CString strFileUrl = file->GetPartFileURL();
		if(file->GetStatus() != PS_COMPLETE && (strFileUrl.Left(7).CompareNoCase(_T("http://")) == 0 
			|| strFileUrl.Left(6).CompareNoCase(_T("ftp://")) == 0))	
		{  
            OnNoComment(NULL);
		}
		else
		{
			CString strURL = _T("http://www.verycd.com/files/");
			//CString strHash = md4str(file->GetFileHash());

			strURL += CreateED2kLink(file, false);
			strURL.Replace(_T("|"), _T("%7C"));
			ShellOpenFile(strURL);
    	}
	}

	*pResult = 0;
}

void CDownloadListCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	LVHITTESTINFO test;
	test.pt = point;
	test.flags = 0;
	test.iItem = -1;
	test.iSubItem = -1;

	if(ListView_SubItemHitTest(m_hWnd, &test) == -1)
	{
		return;
	}

	CtrlItem_Struct* content = (CtrlItem_Struct*)GetItemData(test.iItem);
	if (content->type == FILE_TYPE && test.iSubItem == 1)
	{
		::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_HAND));
	}

	__super::OnMouseMove(nFlags, point);
}
void CDownloadListCtrl::OnDrawItem(int /*nIDCtl*/, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if ( m_MenuXP ) 
	{
		m_MenuXP->DrawItem( lpDrawItemStruct );
	}
}

void CDownloadListCtrl::OnMeasureItem(int /*nIDCtl*/, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	if ( m_MenuXP )
	{
		m_MenuXP->MeasureItem( lpMeasureItemStruct );
	}
}

void CDownloadListCtrl::UpdateToolBarItem(ToolBarUI* pToolBarUI)
{

	((CDownloadTabWnd*)GetParent())->m_Toolbar.EnableButton(MP_CANCEL, (pToolBarUI->iFilesToCancel ? TRUE : FALSE));
	((CDownloadTabWnd*)GetParent())->m_Toolbar.EnableButton(MP_STOP, (pToolBarUI->iFilesToStop ? TRUE : FALSE));
	((CDownloadTabWnd*)GetParent())->m_Toolbar.EnableButton(MP_PAUSE, (pToolBarUI->iFilesToPause ? TRUE : FALSE));
	((CDownloadTabWnd*)GetParent())->m_Toolbar.EnableButton(MP_RESUME, (pToolBarUI->iFilesToResume ? TRUE : FALSE));
}

void CDownloadListCtrl::UpdateToolBarItem(void)
{
	((CDownloadTabWnd*)GetParent())->m_Toolbar.EnableButton(MP_CANCEL, m_ToolBarUI.iFilesToCancel ? TRUE : FALSE);
	((CDownloadTabWnd*)GetParent())->m_Toolbar.EnableButton(MP_STOP, m_ToolBarUI.iFilesToStop ? TRUE : FALSE);
	((CDownloadTabWnd*)GetParent())->m_Toolbar.EnableButton(MP_PAUSE, m_ToolBarUI.iFilesToPause ? TRUE : FALSE);
	((CDownloadTabWnd*)GetParent())->m_Toolbar.EnableButton(MP_RESUME, m_ToolBarUI.iFilesToResume ? TRUE : FALSE);
	((CDownloadTabWnd*)GetParent())->m_Toolbar.EnableButton(MP_OPENFOLDER, m_ToolBarUI.iFilesToOpenFolder==1 ? TRUE : FALSE);
}

LRESULT CDownloadListCtrl::OnUpdateGUIStart(WPARAM /*wParam*/, LPARAM lParam)
{
	m_pDialog = (CWnd*)lParam;
	m_pDialog->SendMessage(WM_UPDATE_GUI_FILEPROGRESS, 0, (LPARAM)m_pPartFile);
	return S_OK;
}

LRESULT CDownloadListCtrl::OnUpdateGUIStop(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	m_pDialog = NULL;
	return S_OK;
}
void CDownloadListCtrl::OnNoComment(CHtmlCtrl *pHtml)
{ 
	CString strFilePath;
	strFilePath = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("Default.htm");

	CFile file;
	if(file.Open(strFilePath,CFile::modeCreate| CFile::modeWrite|CFile::modeNoTruncate))
	{   
		CString strComment = GetResString(IDS_NOCOMMENT);
		CStringA strBuffer ;
		strBuffer = strComment;
		file.Write(strBuffer,strBuffer.GetLength());
	}
	file.Close();
	if(pHtml == NULL)
		ShellOpenFile(strFilePath);
	else
        pHtml->Navigate2(strFilePath,0,NULL);
}

int CDownloadListCtrl::FindFile(CPartFile* pPartFile)
{
	for(int i = 0;i<GetItemCount();i++)
	{   
		CtrlItem_Struct* newitem = NULL;
		newitem = (CtrlItem_Struct*)GetItemData(i);
		if(newitem->value == pPartFile)
		{
			return i;
		}
	}
	return -1;
}

int CDownloadListCtrl::FindFile(CUpDownClient* pClient)
{
	for(int i = 0;i<GetItemCount();i++)
	{   
		CtrlItem_Struct* newitem = NULL;
		newitem = (CtrlItem_Struct*)GetItemData(i);
		if(newitem->value == pClient)
		{
			return i;
		}
	}
	return -1;
}

//Added by thilon on 2008.04.29, for 发现大小不一致时，重新创建PartFile下载
LRESULT CDownloadListCtrl::OnReCreatePartFile(WPARAM /*wParam*/, LPARAM lParam)
{
	if (lParam == 0)
	{
		return 0;
	}

	CPartFile* partfile = (CPartFile*) lParam;
	partfile->InitFromOriginalURL();

	return 0;
}
