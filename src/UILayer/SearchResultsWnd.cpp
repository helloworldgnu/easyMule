/* 
 * $Id: SearchResultsWnd.cpp 9297 2008-12-24 09:55:04Z dgkang $
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
#include "SearchDlg.h"
#include "SearchResultsWnd.h"
#include "SearchParamsWnd.h"
#include "SearchParams.h"
#include "Packets.h"
#include "OtherFunctions.h"
#include "SearchFile.h"
#include "SearchList.h"
#include "Sockets.h"
#include "ServerList.h"
#include "Server.h"
#include "SafeFile.h"
#include "DownloadQueue.h"
#include "Statistics.h"
#include "emuledlg.h"
#include "opcodes.h"
#include "ED2KLink.h"
#include "Kademlia/Kademlia/Kademlia.h"
#include "kademlia/kademlia/SearchManager.h"
#include "kademlia/kademlia/search.h"
#include "SearchExpr.h"
#define USE_FLEX
#include "Parser.hpp"
#include "Scanner.h"
#include "HelpIDs.h"
#include "Exceptions.h"
#include "StringConversion.h"
#include "UserMsgs.h"
#include "webbrowserWnd.h" //Added by thilon on 2006.08 for VeryCD Search
#include "Log.h"
#include "MenuCmds.h"
#include "DropDownButton.h"
#include "CmdFuncs.h"
#include ".\searchresultswnd.h"
#include "SearchParams.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern int yyparse();
extern int yyerror(const char* errstr);
extern int yyerror(LPCTSTR errstr);
extern LPCTSTR _aszInvKadKeywordChars;

enum ESearchTimerID
{
	TimerServerTimeout = 1,
	TimerGlobalSearch
};

enum ESearchResultImage
{
	sriServerActive,
	sriGlobalActive,
	sriKadActice,
	sriClient,
	sriServer,
	sriGlobal,
	sriKad
};

#define	SEARCH_LIST_MENU_BUTTON_XOFF	8
#define	SEARCH_LIST_MENU_BUTTON_WIDTH	170
#define	SEARCH_LIST_MENU_BUTTON_HEIGHT	22	// don't set the height do something different than 22 unless you know exactly what you are doing!

// CSearchResultsWnd dialog

IMPLEMENT_DYNCREATE(CSearchResultsWnd, CResizableFormView)

BEGIN_MESSAGE_MAP(CSearchResultsWnd, CResizableFormView)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_SDOWNLOAD, OnBnClickedDownloadSelected)
	ON_BN_CLICKED(IDC_CLEARALL, OnBnClickedClearAll)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, OnSelChangeTab)
	ON_MESSAGE(UM_CLOSETAB, OnCloseTab)
	ON_MESSAGE(UM_DBLCLICKTAB, OnDblClickTab)
	ON_WM_DESTROY()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_WM_CREATE()
	ON_WM_HELPINFO()
	ON_MESSAGE(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
	//ON_BN_CLICKED(IDC_OPEN_PARAMS_WND, OnBnClickedOpenParamsWnd)
	ON_WM_SYSCOMMAND()
	ON_MESSAGE(UM_DELAYED_EVALUATE, OnChangeFilter)
	ON_NOTIFY(TBN_DROPDOWN, IDC_SEARCHLST_ICO, OnSearchListMenuBtnDropDown)
	ON_BN_CLICKED(IDC_BUTTON_SEARCH, OnBnClickedButtonSearch)
END_MESSAGE_MAP()

CSearchResultsWnd::CSearchResultsWnd(CWnd* /*pParent*/)
	: CResizableFormView(CSearchResultsWnd::IDD)
{
	m_nEd2kSearchID = 0x80000000;
	global_search_timer = 0;
	searchpacket = NULL;
	m_b64BitSearchPacket = false;
	canceld = false;
	servercount = 0;
	globsearch = false;
	icon_search = NULL;
	m_uTimerLocalServer = 0;
	m_iSentMoreReq = 0;
	searchselect.m_bCloseable = true;
	m_btnSearchListMenu = new CDropDownButton;
	m_nFilterColumn = 0;

	m_iCurSearchIndexInRes = 0;
}

CSearchResultsWnd::~CSearchResultsWnd()
{
	delete m_btnSearchListMenu;
	if (globsearch)
		delete searchpacket;
	if (icon_search)
		VERIFY( DestroyIcon(icon_search) );
	if (m_uTimerLocalServer)
		VERIFY( KillTimer(m_uTimerLocalServer) );
}

void CSearchResultsWnd::OnInitialUpdate()
{
	CResizableFormView::OnInitialUpdate();
	InitWindowStyles(this);
	//CGlobalVariable::searchlist->SetOutputWnd(&searchlistctrl);
	searchlistctrl.Init(CGlobalVariable::searchlist);
	searchlistctrl.SetName(_T("SearchListCtrl"));

	CRect rc;
	rc.top = 2;
	rc.left = SEARCH_LIST_MENU_BUTTON_XOFF;
	rc.right = rc.left + SEARCH_LIST_MENU_BUTTON_WIDTH;
	rc.bottom = rc.top + SEARCH_LIST_MENU_BUTTON_HEIGHT;
	m_btnSearchListMenu->Init(true, true);
	m_btnSearchListMenu->MoveWindow(&rc);
	m_btnSearchListMenu->AddBtnStyle(IDC_SEARCHLST_ICO, TBSTYLE_AUTOSIZE);
	m_btnSearchListMenu->ModifyStyle(TBSTYLE_TOOLTIPS, 0);
	m_btnSearchListMenu->SetExtendedStyle(m_btnSearchListMenu->GetExtendedStyle() & ~TBSTYLE_EX_MIXEDBUTTONS);
	m_btnSearchListMenu->RecalcLayout(true);

	m_ctlFilter.OnInit(searchlistctrl.GetHeaderCtrl());

	SetAllIcons();
	Localize();
	searchprogress.SetStep(1);
	global_search_timer = 0;
	globsearch = false;

	AddAnchor(*m_btnSearchListMenu, TOP_LEFT);
	AddAnchor(IDC_FILTER, TOP_RIGHT);
	AddAnchor(IDC_SDOWNLOAD, BOTTOM_LEFT);
	AddAnchor(IDC_SEARCHLIST, TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(IDC_PROGRESS1, BOTTOM_LEFT, BOTTOM_RIGHT);
	AddAnchor(IDC_CLEARALL, BOTTOM_RIGHT);
	//AddAnchor(IDC_OPEN_PARAMS_WND, TOP_RIGHT);
	AddAnchor(searchselect.m_hWnd, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_STATIC_DLTOof, BOTTOM_LEFT);
	AddAnchor(IDC_CATTAB2, BOTTOM_LEFT, BOTTOM_RIGHT);

	AddAnchor(IDC_EDIT_SEARCH, TOP_LEFT);
	AddAnchor(IDC_BUTTON_SEARCH, TOP_LEFT);


	
	m_BtnSearch.SetWindowText(GetResString(IDS_SW_SEARCHBOX));

	ShowSearchSelector(false);

	if (theApp.m_fontSymbol.m_hObject)
	{
		GetDlgItem(IDC_STATIC_DLTOof)->SetFont(&theApp.m_fontSymbol);
		GetDlgItem(IDC_STATIC_DLTOof)->SetWindowText(GetExStyle() & WS_EX_LAYOUTRTL ? _T("3") : _T("4")); // show a right-arrow
	}

	m_ctlMethod.ResetContent();
	VERIFY( m_ctlMethod.AddItem(GetResString(IDS_GLOBALSEARCH), 0) == SearchAreaEd2kGlobal );
	VERIFY( m_ctlMethod.AddItem(GetResString(IDS_KADEMLIA) + _T(" ") + GetResString(IDS_NETWORK), 1) == SearchAreaKademlia );
	UpdateHorzExtent(m_ctlMethod, 16); // adjust dropped width to ensure all strings are fully visible
	m_ctlMethod.SetCurSel(SearchAreaEd2kGlobal);

}

void CSearchResultsWnd::DoDataExchange(CDataExchange* pDX)
{
	CResizableFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SEARCHLIST, searchlistctrl);
	DDX_Control(pDX, IDC_PROGRESS1, searchprogress);
	DDX_Control(pDX, IDC_TAB1, searchselect);
	DDX_Control(pDX, IDC_CATTAB2, m_cattabs);
	DDX_Control(pDX, IDC_FILTER, m_ctlFilter);
	/*DDX_Control(pDX, IDC_OPEN_PARAMS_WND, m_ctlOpenParamsWnd);*/
	DDX_Control(pDX, IDC_SEARCHLST_ICO, *m_btnSearchListMenu);
	DDX_Control(pDX, IDC_EDIT_SEARCH, m_SearchCtrl);
	DDX_Control(pDX, IDC_BUTTON_SEARCH, m_BtnSearch);
	DDX_Control(pDX, IDC_COMBO_AREA, m_ctlMethod);
}

//MODIFIED by VC-fengwen on 2007/09/11 <begin> : 返回值标识pParams是否被删除了。
//void CSearchResultsWnd::StartSearch(SSearchParams* pParams)
bool CSearchResultsWnd::StartSearch(SSearchParams* pParams)
//MODIFIED by VC-fengwen on 2007/09/11 <end> : 返回值标识pParams是否被删除了。
{
	switch (pParams->eType)
	{
		case SearchTypeEd2kServer:
		case SearchTypeEd2kGlobal:
		case SearchTypeKademlia:
			return StartNewSearch(pParams);
			break;

		case SearchTypeFileDonkey:
			ShellOpenFile(CreateWebQuery(pParams));
			delete pParams;
			return false;
			break;
/*
		case SearchTypeVeryCD:						// Added by thilon on 2006.09.05 for VeryCD search
			if(thePrefs.m_bShowBroswer && IsWindow(theApp.emuledlg->webbrowser->m_hWnd))
			{
				theApp.emuledlg->webbrowser->Navigate(CreateWebQuery(pParams));
				theApp.emuledlg->SetActiveDialog(theApp.emuledlg->webbrowser);
			}
			else
			{
                ShellOpenFile(CreateWebQuery(pParams));
			}

			delete pParams;
			return false;
			break;
*/

		default:
			ASSERT(0);
			delete pParams;
			return false;
	}
}

void CSearchResultsWnd::OnTimer(UINT nIDEvent)
{
	CResizableFormView::OnTimer(nIDEvent);

	if (m_uTimerLocalServer != 0 && nIDEvent == m_uTimerLocalServer)
	{
		if (thePrefs.GetDebugServerSearchesLevel() > 0)
			Debug(_T("Timeout waiting on search results of local server\n"));
		// the local server did not answer within the timeout
		VERIFY( KillTimer(m_uTimerLocalServer) );
		m_uTimerLocalServer = 0;

		// start the global search
		if (globsearch)
		{
			if (global_search_timer == 0)
				VERIFY( (global_search_timer = SetTimer(TimerGlobalSearch, 750, 0)) != NULL );
		}
		else
			CancelEd2kSearch();
	}
	else if (nIDEvent == global_search_timer)
	{
	    if (CGlobalVariable::serverconnect->IsConnected())
		{
			CServer* pConnectedServer = CGlobalVariable::serverconnect->GetCurrentServer();
			if (pConnectedServer)
				pConnectedServer = CGlobalVariable::serverlist->GetServerByAddress(pConnectedServer->GetAddress(), pConnectedServer->GetPort());

			CServer* toask = NULL;
			while (servercount < CGlobalVariable::serverlist->GetServerCount()-1)
			{
				servercount++;
				searchprogress.StepIt();

				toask = CGlobalVariable::serverlist->GetNextSearchServer();
				if (toask == NULL)
					break;
				if (toask == pConnectedServer) {
					toask = NULL;
					continue;
				}
				if (toask->GetFailedCount() >= thePrefs.GetDeadServerRetries()) {
					toask = NULL;
					continue;
				}
				break;
			}

			if (toask)
			{
				if (toask->SupportsLargeFilesUDP() && (toask->GetUDPFlags() & SRV_UDPFLG_EXT_GETFILES))
				{
					CSafeMemFile data(50);
					uint32 nTagCount = 1;
					data.WriteUInt32(nTagCount);
					CTag tagFlags(CT_SERVER_UDPSEARCH_FLAGS, SRVCAP_UDP_NEWTAGS_LARGEFILES);
					tagFlags.WriteNewEd2kTag(&data);
					Packet* pExtSearchPacket = new Packet(OP_GLOBSEARCHREQ3, searchpacket->size + (uint32)data.GetLength());
					data.SeekToBegin();
					data.Read(pExtSearchPacket->pBuffer, (uint32)data.GetLength());
					memcpy(pExtSearchPacket->pBuffer+(uint32)data.GetLength(), searchpacket->pBuffer, searchpacket->size);
					theStats.AddUpDataOverheadServer(pExtSearchPacket->size);
					CGlobalVariable::serverconnect->SendUDPPacket(pExtSearchPacket, toask, true);
					if (thePrefs.GetDebugServerUDPLevel() > 0)
						Debug(_T(">>> Sending %s  to server %-21s (%3u of %3u)\n"),  _T("OP__GlobSearchReq3"), ipstr(toask->GetAddress(), toask->GetPort()), servercount, CGlobalVariable::serverlist->GetServerCount());

				}
				else if (toask->GetUDPFlags() & SRV_UDPFLG_EXT_GETFILES)
				{
					if (!m_b64BitSearchPacket || toask->SupportsLargeFilesUDP()){
						searchpacket->opcode = OP_GLOBSEARCHREQ2;
						if (thePrefs.GetDebugServerUDPLevel() > 0)
							Debug(_T(">>> Sending %s  to server %-21s (%3u of %3u)\n"), _T("OP__GlobSearchReq2"), ipstr(toask->GetAddress(), toask->GetPort()), servercount, CGlobalVariable::serverlist->GetServerCount());
						theStats.AddUpDataOverheadServer(searchpacket->size);
						CGlobalVariable::serverconnect->SendUDPPacket(searchpacket, toask, false);
					}
					else{
						if (thePrefs.GetDebugServerUDPLevel() > 0)
							Debug(_T(">>> Skipped UDP search on server %-21s (%3u of %3u): No large file support\n"), ipstr(toask->GetAddress(), toask->GetPort()), servercount, CGlobalVariable::serverlist->GetServerCount());
					}
				}
				else
				{
					if (!m_b64BitSearchPacket || toask->SupportsLargeFilesUDP()){
						searchpacket->opcode = OP_GLOBSEARCHREQ;
						if (thePrefs.GetDebugServerUDPLevel() > 0)
							Debug(_T(">>> Sending %s  to server %-21s (%3u of %3u)\n"), _T("OP__GlobSearchReq1"), ipstr(toask->GetAddress(), toask->GetPort()), servercount, CGlobalVariable::serverlist->GetServerCount());
						theStats.AddUpDataOverheadServer(searchpacket->size);
						CGlobalVariable::serverconnect->SendUDPPacket(searchpacket, toask, false);
					}
					else{
						if (thePrefs.GetDebugServerUDPLevel() > 0)
							Debug(_T(">>> Skipped UDP search on server %-21s (%3u of %3u): No large file support\n"), ipstr(toask->GetAddress(), toask->GetPort()), servercount, CGlobalVariable::serverlist->GetServerCount());
					}
				}
			}
			else
				CancelEd2kSearch();
	    }
	    else
			CancelEd2kSearch();
    }
	else
		ASSERT( 0 );
}

void CSearchResultsWnd::SetSearchResultsIcon(UINT uSearchID, int iImage)
{
    int iTabItems = searchselect.GetItemCount();
    for (int i = 0; i < iTabItems; i++)
	{
        TCITEM tci;
        tci.mask = TCIF_PARAM;
		if (searchselect.GetItem(i, &tci) && tci.lParam != NULL && ((const SSearchParams*)tci.lParam)->dwSearchID == uSearchID)
		{
			tci.mask = TCIF_IMAGE;
			tci.iImage = iImage;
			searchselect.SetItem(i, &tci);
			break;
		}
    }
}

void CSearchResultsWnd::SetActiveSearchResultsIcon(UINT uSearchID)
{
	SSearchParams* pParams = GetSearchResultsParams(uSearchID);
	if (pParams)
	{
		int iImage;
		if (pParams->eType == SearchTypeKademlia)
			iImage = sriKadActice;
		else if (pParams->eType == SearchTypeEd2kGlobal)
			iImage = sriGlobalActive;
		else
			iImage = sriServerActive;
		SetSearchResultsIcon(uSearchID, iImage);
	}
}

void CSearchResultsWnd::SetInactiveSearchResultsIcon(UINT uSearchID)
{
	SSearchParams* pParams = GetSearchResultsParams(uSearchID);
	if (pParams)
	{
		int iImage;
		if (pParams->eType == SearchTypeKademlia)
			iImage = sriKad;
		else if (pParams->eType == SearchTypeEd2kGlobal)
			iImage = sriGlobal;
		else
			iImage = sriServer;
		SetSearchResultsIcon(uSearchID, iImage);
	}
}

SSearchParams* CSearchResultsWnd::GetSearchResultsParams(UINT uSearchID) const
{
    int iTabItems = searchselect.GetItemCount();
    for (int i = 0; i < iTabItems; i++)
	{
        TCITEM tci;
        tci.mask = TCIF_PARAM;
		if (searchselect.GetItem(i, &tci) && tci.lParam != NULL && ((const SSearchParams*)tci.lParam)->dwSearchID == uSearchID)
			return (SSearchParams*)tci.lParam;
    }
	return NULL;
}

void CSearchResultsWnd::CancelSearch(UINT uSearchID)
{
	if (uSearchID == 0)
	{
		int iCurSel = searchselect.GetCurSel();
		if (iCurSel == -1)
			return;
		TCITEM item;
		item.mask = TCIF_PARAM;
		if (searchselect.GetItem(iCurSel, &item) && item.lParam != NULL)
			uSearchID = ((const SSearchParams*)item.lParam)->dwSearchID;
		if (uSearchID == 0)
			return;
	}

	SSearchParams* pParams = GetSearchResultsParams(uSearchID);
	if (pParams == NULL)
		return;

	if (pParams->eType == SearchTypeEd2kServer || pParams->eType == SearchTypeEd2kGlobal)
		CancelEd2kSearch();
	else if (pParams->eType == SearchTypeKademlia)
	{
		Kademlia::CSearchManager::StopSearch(pParams->dwSearchID, false);
		CancelKadSearch(pParams->dwSearchID);
	}
}

void CSearchResultsWnd::CancelEd2kSearch()
{
	SetInactiveSearchResultsIcon(m_nEd2kSearchID);

	canceld = true;

	// delete any global search timer
	if (globsearch){
		delete searchpacket;
		searchpacket = NULL;
		m_b64BitSearchPacket = false;
	}
	globsearch = false;
	if (global_search_timer){
		VERIFY( KillTimer(global_search_timer) );
		global_search_timer = 0;
		searchprogress.SetPos(0);
	}

	// delete local server timeout timer
	if (m_uTimerLocalServer){
		VERIFY( KillTimer(m_uTimerLocalServer) );
		m_uTimerLocalServer = 0;
	}

	SearchCanceled(m_nEd2kSearchID);
}

void CSearchResultsWnd::CancelKadSearch(UINT uSearchID)
{
	SearchCanceled(uSearchID);
}

void CSearchResultsWnd::SearchStarted()
{
	//CWnd* pWndFocus = GetFocus();
	//m_pwndParams->m_ctlStart.EnableWindow(FALSE);
	//if (pWndFocus && pWndFocus->m_hWnd  == m_pwndParams->m_ctlStart.m_hWnd)
	//	m_pwndParams->m_ctlName.SetFocus();
	//m_pwndParams->m_ctlCancel.EnableWindow(TRUE);
}

void CSearchResultsWnd::SearchCanceled(UINT uSearchID)
{
	SetInactiveSearchResultsIcon(uSearchID);

	int iCurSel = searchselect.GetCurSel();
	if (iCurSel != -1)
	{
		TCITEM item;
		item.mask = TCIF_PARAM;
		if (searchselect.GetItem(iCurSel, &item) && item.lParam != NULL && uSearchID == ((const SSearchParams*)item.lParam)->dwSearchID)
		{
			//CWnd* pWndFocus = GetFocus();
			//m_pwndParams->m_ctlCancel.EnableWindow(FALSE);
			//if (pWndFocus && pWndFocus->m_hWnd == m_pwndParams->m_ctlCancel.m_hWnd)
			//	m_pwndParams->m_ctlName.SetFocus();
			//m_pwndParams->m_ctlStart.EnableWindow(TRUE);
		}
	}
}

void CSearchResultsWnd::LocalEd2kSearchEnd(UINT count, bool /*bMoreResultsAvailable*/)
{
	// local server has answered, kill the timeout timer
	if (m_uTimerLocalServer) {
		VERIFY( KillTimer(m_uTimerLocalServer) );
		m_uTimerLocalServer = 0;
	}

	if (!canceld && count > MAX_RESULTS)
		CancelEd2kSearch();
	if (!canceld) {
		if (!globsearch)
			SearchCanceled(m_nEd2kSearchID);
		else
			VERIFY( (global_search_timer = SetTimer(TimerGlobalSearch, 750, 0)) != NULL );
	}
	//m_pwndParams->m_ctlMore.EnableWindow(bMoreResultsAvailable && m_iSentMoreReq < MAX_MORE_SEARCH_REQ);
}

void CSearchResultsWnd::AddGlobalEd2kSearchResults(UINT count)
{
	if (!canceld && count > MAX_RESULTS)
		CancelEd2kSearch();
}

void CSearchResultsWnd::OnBnClickedDownloadSelected()
{
	//start download(s)
	DownloadSelected();
}

void CSearchResultsWnd::OnDblClkSearchList(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	OnBnClickedDownloadSelected();
	*pResult = 0;
}

CString	CSearchResultsWnd::CreateWebQuery(SSearchParams* pParams)
{
	CString query;
	switch (pParams->eType)
	{
	case SearchTypeFileDonkey:
		query = _T("http://www.filedonkey.com/search.html?");
		query += _T("pattern=") + EncodeURLQueryParam(pParams->strExpression);
		if (pParams->strFileType == ED2KFTSTR_AUDIO)
			query += _T("&media=Audio");
		else if (pParams->strFileType == ED2KFTSTR_VIDEO)
			query += _T("&media=Video");
		else if (pParams->strFileType == ED2KFTSTR_PROGRAM)
			query += _T("&media=Pro");
		query += _T("&requestby=emule");

		if (pParams->ullMinSize > 0)
			query.AppendFormat(_T("&min_size=%I64u"),pParams->ullMinSize);
		
		if (pParams->ullMaxSize > 0)
			query.AppendFormat(_T("&max_size=%I64u"),pParams->ullMaxSize);

		break;
		case SearchTypeVeryCD: // Added by thilon on 2006.09.05 for VeryCD search
		query = "http://www.verycd.com/search/folders/";
		query += EncodeUrlUtf8(pParams->strExpression);
		query += "/";
		break;
	default:
		return _T("");
	}
	return query;
}

void CSearchResultsWnd::DownloadSelected()
{
	DownloadSelected(thePrefs.AddNewFilesPaused());
}

void CSearchResultsWnd::DownloadSelected(bool bPaused)
{
	CWaitCursor curWait;
	POSITION pos = searchlistctrl.GetFirstSelectedItemPosition();
	while (pos != NULL)
	{
		int iIndex = searchlistctrl.GetNextSelectedItem(pos);
		if (iIndex >= 0)
		{
			// get selected listview item (may be a child item from an expanded search result)
			const CSearchFile* sel_file = (CSearchFile*)searchlistctrl.GetItemData(iIndex);

			// get parent
			const CSearchFile* parent;
			if (sel_file->GetListParent() != NULL)
				parent = sel_file->GetListParent();
			else
				parent = sel_file;

			if (parent->IsComplete() == 0 && parent->GetSourceCount() >= 50)
			{
				CString strMsg;
				strMsg.Format(GetResString(IDS_ASKDLINCOMPLETE), sel_file->GetFileName());
				int iAnswer = AfxMessageBox(strMsg, MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2);
				if (iAnswer != IDYES)
					continue;
			}

			// create new DL-queue entry with all properties of parent (e.g. already received sources!)
			// but with the filename of the selected listview item.
			CSearchFile tempFile(parent);
			tempFile.SetFileName(sel_file->GetFileName());
			tempFile.SetStrTagValue(FT_FILENAME, sel_file->GetFileName());
			//MODIFIED by fengwen on 2007/02/09	<begin> :
			//CGlobalVariable::downloadqueue->AddSearchToDownload(&tempFile, bPaused, m_cattabs.GetCurSel());
			CmdFuncs::AddSearchToDownload(&tempFile, bPaused, m_cattabs.GetCurSel());
			//MODIFIED by fengwen on 2007/02/09	<end> :

			// update parent and all childs
			searchlistctrl.UpdateSources(parent);
		}
	}
}

void CSearchResultsWnd::OnSysColorChange()
{
	CResizableFormView::OnSysColorChange();
	SetAllIcons();
	searchlistctrl.CreateMenues();
}

void CSearchResultsWnd::SetAllIcons()
{
	m_btnSearchListMenu->SetIcon(_T("SearchResults"));
	if (icon_search)
		VERIFY( DestroyIcon(icon_search) );
	icon_search = theApp.LoadIcon(_T("SearchResults"), 16, 16);
	((CStatic*)GetDlgItem(IDC_SEARCHLST_ICO))->SetIcon(icon_search);

	CImageList iml;
	iml.Create(16,16,theApp.m_iDfltImageListColorFlags|ILC_MASK,0,1);
	iml.SetBkColor(CLR_NONE);
	iml.Add(CTempIconLoader(_T("SearchMethod_ServerActive"), 16, 16));
	iml.Add(CTempIconLoader(_T("SearchMethod_GlobalActive"), 16, 16));
	iml.Add(CTempIconLoader(_T("SearchMethod_KademliaActive"), 16, 16));
	iml.Add(CTempIconLoader(_T("StatsClients"), 16, 16));
	iml.Add(CTempIconLoader(_T("SearchMethod_SERVER"), 16, 16));
	iml.Add(CTempIconLoader(_T("SearchMethod_GLOBAL"), 16, 16));
	iml.Add(CTempIconLoader(_T("SearchMethod_KADEMLIA"), 16, 16));
	searchselect.SetImageList(&iml);
	m_imlSearchResults.DeleteImageList();
	m_imlSearchResults.Attach(iml.Detach());
	searchselect.SetPadding(CSize(10, 3));

	iml.Create(16,16,theApp.m_iDfltImageListColorFlags|ILC_MASK,0,1);
	iml.SetBkColor(CLR_NONE);
	iml.Add(CTempIconLoader(_T("SearchMethod_GLOBAL"), 16, 16));
	iml.Add(CTempIconLoader(_T("SearchMethod_KADEMLIA"), 16, 16));
	m_ctlMethod.SetImageList(&iml);
	m_imlSearchMethods.DeleteImageList();
	m_imlSearchMethods.Attach(iml.Detach());

}

void CSearchResultsWnd::Localize()
{
	searchlistctrl.Localize();
	UpdateCatTabs();

    GetDlgItem(IDC_CLEARALL)->SetWindowText(GetResString(IDS_REMOVEALLSEARCH));
	m_btnSearchListMenu->SetWindowText(GetResString(IDS_SW_RESULT));
    GetDlgItem(IDC_SDOWNLOAD)->SetWindowText(GetResString(IDS_SW_DOWNLOAD));
	GetDlgItem(IDC_BUTTON_SEARCH)->SetWindowText(GetResString(IDS_SW_SEARCHBOX));

	//{begin} VC-dgkang 2008年7月17日
	int Sel = m_ctlMethod.GetCurSel();
	m_ctlMethod.ResetContent();
	VERIFY( m_ctlMethod.AddItem(GetResString(IDS_GLOBALSEARCH), 0) == SearchAreaEd2kGlobal );
	VERIFY( m_ctlMethod.AddItem(GetResString(IDS_KADEMLIA) + _T(" ") + GetResString(IDS_NETWORK), 1) == SearchAreaKademlia );
	UpdateHorzExtent(m_ctlMethod, 16); // adjust dropped width to ensure all strings are fully visible
	m_ctlMethod.SetCurSel(Sel);
	//{end}

	/*m_ctlOpenParamsWnd.SetWindowText(GetResString(IDS_SEARCHPARAMS)+_T("..."));*/
}

void CSearchResultsWnd::OnBnClickedClearAll()
{
	DeleteAllSearches();
}

CString DbgGetFileMetaTagName(UINT uMetaTagID)
{
	switch (uMetaTagID)
	{
		case FT_FILENAME:			return _T("@Name");
		case FT_FILESIZE:			return _T("@Size");
		case FT_FILESIZE_HI:		return _T("@SizeHI");
		case FT_FILETYPE:			return _T("@Type");
		case FT_FILEFORMAT:			return _T("@Format");
		case FT_LASTSEENCOMPLETE:	return _T("@LastSeenComplete");
		case FT_SOURCES:			return _T("@Sources");
		case FT_COMPLETE_SOURCES:	return _T("@Complete");
		case FT_MEDIA_ARTIST:		return _T("@Artist");
		case FT_MEDIA_ALBUM:		return _T("@Album");
		case FT_MEDIA_TITLE:		return _T("@Title");
		case FT_MEDIA_LENGTH:		return _T("@Length");
		case FT_MEDIA_BITRATE:		return _T("@Bitrate");
		case FT_MEDIA_CODEC:		return _T("@Codec");
		case FT_FILECOMMENT:		return _T("@Comment");
		case FT_FILERATING:			return _T("@Rating");
		case FT_FILEHASH:			return _T("@Filehash");
	}

	CString buffer;
	buffer.Format(_T("Tag0x%02X"), uMetaTagID);
	return buffer;
}

CString DbgGetFileMetaTagName(LPCSTR pszMetaTagID)
{
	if (strlen(pszMetaTagID) == 1)
		return DbgGetFileMetaTagName(((BYTE*)pszMetaTagID)[0]);
	CString strName;
	strName.Format(_T("\"%hs\""), pszMetaTagID);
	return strName;
}

CString DbgGetSearchOperatorName(UINT uOperator)
{
	static const LPCTSTR _aszEd2kOps[] = 
	{
		_T("="),
		_T(">"),
		_T("<"),
		_T(">="),
		_T("<="),
		_T("<>"),
	};

	if (uOperator >= ARRSIZE(_aszEd2kOps)){
		ASSERT(0);
		return _T("*UnkOp*");
	}
	return _aszEd2kOps[uOperator];
}

static CStringA _strCurKadKeywordA;
static CSearchExpr _SearchExpr;
CStringArray _astrParserErrors;

static TCHAR _chLastChar = 0;
static CString _strSearchTree;

bool DumpSearchTree(int& iExpr, const CSearchExpr& rSearchExpr, int iLevel, bool bFlat)
{
	if (iExpr >= rSearchExpr.m_aExpr.GetCount())
		return false;
	if (!bFlat)
		_strSearchTree += _T('\n') + CString(_T(' '), iLevel);
	const CSearchAttr& rSearchAttr = rSearchExpr.m_aExpr[iExpr++];
	CStringA strTok = rSearchAttr.m_str;
	if (strTok == SEARCHOPTOK_AND || strTok == SEARCHOPTOK_OR || strTok == SEARCHOPTOK_NOT)
	{
		if (bFlat) {
			if (_chLastChar != _T('(') && _chLastChar != _T('\0'))
				_strSearchTree.AppendFormat(_T(" "));
		}
		_strSearchTree.AppendFormat(_T("(%hs "), strTok.Mid(1));
		_chLastChar = _T('(');
		DumpSearchTree(iExpr, rSearchExpr, iLevel + 4, bFlat);
		DumpSearchTree(iExpr, rSearchExpr, iLevel + 4, bFlat);
		_strSearchTree.AppendFormat(_T(")"));
		_chLastChar = _T(')');
	}
	else
	{
		if (bFlat) {
			if (_chLastChar != _T('(') && _chLastChar != _T('\0'))
				_strSearchTree.AppendFormat(_T(" "));
		}
		_strSearchTree += rSearchAttr.DbgGetAttr();
		_chLastChar = _T('\1');
	}
	return true;
}

bool DumpSearchTree(const CSearchExpr& rSearchExpr, bool bFlat)
{
	_chLastChar = _T('\0');
	int iExpr = 0;
	int iLevel = 0;
	return DumpSearchTree(iExpr, rSearchExpr, iLevel, bFlat);
}

void ParsedSearchExpression(const CSearchExpr* pexpr)
{
	int iOpAnd = 0;
	int iOpOr = 0;
	int iOpNot = 0;
	int iNonDefTags = 0;
	//CStringA strDbg;
	for (int i = 0; i < pexpr->m_aExpr.GetCount(); i++)
	{
		const CSearchAttr& rSearchAttr = pexpr->m_aExpr[i];
		const CStringA& rstr = rSearchAttr.m_str;
		if (rstr == SEARCHOPTOK_AND)
		{
			iOpAnd++;
			//strDbg.AppendFormat("%s ", rstr.Mid(1));
		}
		else if (rstr == SEARCHOPTOK_OR)
		{
			iOpOr++;
			//strDbg.AppendFormat("%s ", rstr.Mid(1));
		}
		else if (rstr == SEARCHOPTOK_NOT)
		{
			iOpNot++;
			//strDbg.AppendFormat("%s ", rstr.Mid(1));
		}
		else
		{
			if (rSearchAttr.m_iTag != FT_FILENAME)
				iNonDefTags++;
			//strDbg += rSearchAttr.DbgGetAttr() + " ";
		}
	}
	//if (thePrefs.GetDebugServerSearchesLevel() > 0)
	//	Debug(_T("Search Expr: %hs\n"), strDbg);

	// this limit (+ the additional operators which will be added later) has to match the limit in 'CreateSearchExpressionTree'
	//	+1 Type (Audio, Video)
	//	+1 MinSize
	//	+1 MaxSize
	//	+1 Avail
	//	+1 Extension
	//	+1 Complete sources
	//	+1 Codec
	//	+1 Bitrate
	//	+1 Length
	//	+1 Title
	//	+1 Album
	//	+1 Artist
	// ---------------
	//  12
	if (iOpAnd + iOpOr + iOpNot > 10)
		yyerror(GetResString(IDS_SEARCH_TOOCOMPLEX));

	_SearchExpr.m_aExpr.RemoveAll();
	// optimize search expression, if no OR nor NOT specified
	if (iOpAnd > 0 && iOpOr == 0 && iOpNot == 0 && iNonDefTags == 0)
	{
		CStringA strAndTerms;
		for (int i = 0; i < pexpr->m_aExpr.GetCount(); i++)
		{
			if (pexpr->m_aExpr[i].m_str != SEARCHOPTOK_AND)
			{
				ASSERT( pexpr->m_aExpr[i].m_iTag == FT_FILENAME );
				// Minor optimization: Because we added the Kad keyword to the boolean search expression,
				// we remove it here (and only here) again because we know that the entire search expression
				// does only contain (implicit) ANDed strings.
				if (pexpr->m_aExpr[i].m_str != _strCurKadKeywordA)
				{
					if (!strAndTerms.IsEmpty())
						strAndTerms += ' ';
					strAndTerms += pexpr->m_aExpr[i].m_str;
				}
			}
		}
		ASSERT( _SearchExpr.m_aExpr.GetCount() == 0);
		_SearchExpr.m_aExpr.Add(CSearchAttr(strAndTerms));
	}
	else
	{
		if (pexpr->m_aExpr.GetCount() != 1
			|| !(pexpr->m_aExpr[0].m_iTag == FT_FILENAME && pexpr->m_aExpr[0].m_str == _strCurKadKeywordA))
			_SearchExpr.m_aExpr.Append(pexpr->m_aExpr);
	}
}

class CSearchExprTarget
{
public:
	CSearchExprTarget(CSafeMemFile* pData, EUtf8Str eStrEncode, bool bSupports64Bit, bool* pbPacketUsing64Bit)
	{
		m_data = pData;
		m_eStrEncode = eStrEncode;
		m_bSupports64Bit = bSupports64Bit;
		m_pbPacketUsing64Bit = pbPacketUsing64Bit;
		if (m_pbPacketUsing64Bit)
			*m_pbPacketUsing64Bit = false;
	}

	const CString& GetDebugString() const
	{
		return m_strDbg;
	}

	void WriteBooleanAND()
	{
		m_data->WriteUInt8(0);				// boolean operator parameter type
		m_data->WriteUInt8(0x00);			// "AND"
		m_strDbg.AppendFormat(_T("AND "));
	}

	void WriteBooleanOR()
	{
		m_data->WriteUInt8(0);				// boolean operator parameter type
		m_data->WriteUInt8(0x01);			// "OR"
		m_strDbg.AppendFormat(_T("OR "));
	}

	void WriteBooleanNOT()
	{
		m_data->WriteUInt8(0);				// boolean operator parameter type
		m_data->WriteUInt8(0x02);			// "NOT"
		m_strDbg.AppendFormat(_T("NOT "));
	}

	void WriteMetaDataSearchParam(const CString& rstrValue)
	{
		m_data->WriteUInt8(1);						// string parameter type
		m_data->WriteString(rstrValue, m_eStrEncode); // string value
		m_strDbg.AppendFormat(_T("\"%s\" "), rstrValue);
	}

	void WriteMetaDataSearchParam(UINT uMetaTagID, const CString& rstrValue)
	{
		m_data->WriteUInt8(2);						// string parameter type
		m_data->WriteString(rstrValue, m_eStrEncode); // string value
		m_data->WriteUInt16(sizeof uint8);			// meta tag ID length
		m_data->WriteUInt8((uint8)uMetaTagID);		// meta tag ID name
		m_strDbg.AppendFormat(_T("%s=\"%s\" "), DbgGetFileMetaTagName(uMetaTagID), rstrValue);
	}

	void WriteMetaDataSearchParamA(UINT uMetaTagID, const CStringA& rstrValueA)
	{
		m_data->WriteUInt8(2);						// string parameter type
		m_data->WriteString(rstrValueA);			// string value
		m_data->WriteUInt16(sizeof uint8);			// meta tag ID length
		m_data->WriteUInt8((uint8)uMetaTagID);		// meta tag ID name
		m_strDbg.AppendFormat(_T("%s=\"%hs\" "), DbgGetFileMetaTagName(uMetaTagID), rstrValueA);
	}

	void WriteMetaDataSearchParam(LPCSTR pszMetaTagID, const CString& rstrValue)
	{
		m_data->WriteUInt8(2);						// string parameter type
		m_data->WriteString(rstrValue, m_eStrEncode); // string value
		m_data->WriteString(pszMetaTagID);			// meta tag ID
		m_strDbg.AppendFormat(_T("%s=\"%s\" "), DbgGetFileMetaTagName(pszMetaTagID), rstrValue);
	}

	void WriteMetaDataSearchParam(UINT uMetaTagID, UINT uOperator, uint64 ullValue)
	{
		bool b64BitValue = ullValue > 0xFFFFFFFFui64;
		if (b64BitValue && m_bSupports64Bit) {
			if (m_pbPacketUsing64Bit)
				*m_pbPacketUsing64Bit = true;
			m_data->WriteUInt8(8);					// numeric parameter type (int64)
			m_data->WriteUInt64(ullValue);			// numeric value
		}
		else {
			if (b64BitValue)
				ullValue = 0xFFFFFFFFU;
			m_data->WriteUInt8(3);					// numeric parameter type (int32)
			m_data->WriteUInt32((uint32)ullValue);	// numeric value
		}
		m_data->WriteUInt8((uint8)uOperator);	// comparison operator
		m_data->WriteUInt16(sizeof uint8);		// meta tag ID length
		m_data->WriteUInt8((uint8)uMetaTagID);	// meta tag ID name
		m_strDbg.AppendFormat(_T("%s%s%I64u "), DbgGetFileMetaTagName(uMetaTagID), DbgGetSearchOperatorName(uOperator), ullValue);
	}

	void WriteMetaDataSearchParam(LPCSTR pszMetaTagID, UINT uOperator, uint64 ullValue)
	{
		bool b64BitValue = ullValue > 0xFFFFFFFFui64;
		if (b64BitValue && m_bSupports64Bit) {
			if (m_pbPacketUsing64Bit)
				*m_pbPacketUsing64Bit = true;
			m_data->WriteUInt8(8);					// numeric parameter type (int64)
			m_data->WriteUInt64(ullValue);			// numeric value
		}
		else {
			if (b64BitValue)
				ullValue = 0xFFFFFFFFU;
			m_data->WriteUInt8(3);					// numeric parameter type (int32)
			m_data->WriteUInt32((uint32)ullValue);	// numeric value
		}
		m_data->WriteUInt8((uint8)uOperator);	// comparison operator
		m_data->WriteString(pszMetaTagID);		// meta tag ID
		m_strDbg.AppendFormat(_T("%s%s%I64u "), DbgGetFileMetaTagName(pszMetaTagID), DbgGetSearchOperatorName(uOperator), ullValue);
	}

protected:
	CSafeMemFile* m_data;
	CString m_strDbg;
	EUtf8Str m_eStrEncode;
	bool m_bSupports64Bit;
	bool* m_pbPacketUsing64Bit;
};

static CSearchExpr _SearchExpr2;

static void AddAndAttr(UINT uTag, const CString& rstr)
{
	_SearchExpr2.m_aExpr.InsertAt(0, CSearchAttr(uTag, StrToUtf8(rstr)));
	if (_SearchExpr2.m_aExpr.GetCount() > 1)
		_SearchExpr2.m_aExpr.InsertAt(0, CSearchAttr(SEARCHOPTOK_AND));
}

static void AddAndAttr(UINT uTag, UINT uOpr, uint64 ullVal)
{
	_SearchExpr2.m_aExpr.InsertAt(0, CSearchAttr(uTag, uOpr, ullVal));
	if (_SearchExpr2.m_aExpr.GetCount() > 1)
		_SearchExpr2.m_aExpr.InsertAt(0, CSearchAttr(SEARCHOPTOK_AND));
}

bool GetSearchPacket(CSafeMemFile* pData, SSearchParams* pParams, bool bTargetSupports64Bit, bool* pbPacketUsing64Bit)
{
	CStringA strFileType;
	if (pParams->strFileType == ED2KFTSTR_ARCHIVE){
		// eDonkeyHybrid 0.48 uses type "Pro" for archives files
		// www.filedonkey.com uses type "Pro" for archives files
		strFileType = ED2KFTSTR_PROGRAM;
	}
	else if (pParams->strFileType == ED2KFTSTR_CDIMAGE){
		// eDonkeyHybrid 0.48 uses *no* type for iso/nrg/cue/img files
		// www.filedonkey.com uses type "Pro" for CD-image files
		strFileType = ED2KFTSTR_PROGRAM;
	}
	else{
		//TODO: Support "Doc" types
		strFileType = pParams->strFileType;
	}

	_strCurKadKeywordA.Empty();
	ASSERT( !pParams->strExpression.IsEmpty() );
	if (pParams->eType == SearchTypeKademlia)
	{
		ASSERT( !pParams->strKeyword.IsEmpty() );
		_strCurKadKeywordA = StrToUtf8(pParams->strKeyword);
	}
	if (pParams->strBooleanExpr.IsEmpty())
		pParams->strBooleanExpr = pParams->strExpression;
	if (pParams->strBooleanExpr.IsEmpty())
		return false;

	//TRACE(_T("Raw search expr:\n"));
	//TRACE(_T("%s"), pParams->strBooleanExpr);
	//TRACE(_T("  %s\n"), DbgGetHexDump((uchar*)(LPCTSTR)pParams->strBooleanExpr, pParams->strBooleanExpr.GetLength()*sizeof(TCHAR)));
	_astrParserErrors.RemoveAll();
	_SearchExpr.m_aExpr.RemoveAll();
	if (!pParams->strBooleanExpr.IsEmpty())
	{
		// check this here again, we could have been called from Webinterface or MM
		if (!pParams->bUnicode)
		{
			CStringA strACP(pParams->strBooleanExpr);
			if (!IsValidEd2kStringA(strACP)){
				CString strError(GetResString(IDS_SEARCH_EXPRERROR) + _T("\n\n") + GetResString(IDS_SEARCH_INVALIDCHAR));
				throw new CMsgBoxException(strError, MB_ICONWARNING | MB_HELP, eMule_FAQ_Search - HID_BASE_PROMPT);
			}
		}

	    LexInit(pParams->strBooleanExpr, true);
	    int iParseResult = yyparse();
	    LexFree();
	    if (_astrParserErrors.GetSize() > 0)
		{
		    _SearchExpr.m_aExpr.RemoveAll();
			CString strError(GetResString(IDS_SEARCH_EXPRERROR) + _T("\n\n") + _astrParserErrors[_astrParserErrors.GetSize() - 1]);
		    throw new CMsgBoxException(strError, MB_ICONWARNING | MB_HELP, eMule_FAQ_Search - HID_BASE_PROMPT);
	    }
	    else if (iParseResult != 0)
		{
		    _SearchExpr.m_aExpr.RemoveAll();
			CString strError(GetResString(IDS_SEARCH_EXPRERROR) + _T("\n\n") + GetResString(IDS_SEARCH_GENERALERROR));
		    throw new CMsgBoxException(strError, MB_ICONWARNING | MB_HELP, eMule_FAQ_Search - HID_BASE_PROMPT);
	    }
	}
	//TRACE(_T("Parsed search expr:\n"));
	//for (int i = 0; i < _SearchExpr.m_aExpr.GetCount(); i++){
	//	TRACE(_T("%hs"), _SearchExpr.m_aExpr[i]);
	//	TRACE(_T("  %s\n"), DbgGetHexDump((uchar*)(LPCSTR)_SearchExpr.m_aExpr[i], _SearchExpr.m_aExpr[i].GetLength()*sizeof(CHAR)));
	//}

	// create ed2k search expression
	CSearchExprTarget target(pData, pParams->bUnicode ? utf8strRaw : utf8strNone, bTargetSupports64Bit, pbPacketUsing64Bit);

	_SearchExpr2.m_aExpr.RemoveAll();

	if (!pParams->strExtension.IsEmpty())
		AddAndAttr(FT_FILEFORMAT, pParams->strExtension);

	if (pParams->uAvailability > 0)
		AddAndAttr(FT_SOURCES, ED2K_SEARCH_OP_GREATER_EQUAL, pParams->uAvailability);
	
	if (pParams->ullMaxSize > 0)
		AddAndAttr(FT_FILESIZE, ED2K_SEARCH_OP_LESS_EQUAL, pParams->ullMaxSize);
    
	if (pParams->ullMinSize > 0)
		AddAndAttr(FT_FILESIZE, ED2K_SEARCH_OP_GREATER_EQUAL, pParams->ullMinSize);
    
	if (!strFileType.IsEmpty())
		AddAndAttr(FT_FILETYPE, CString(strFileType));
    
	if (pParams->uComplete > 0)
		AddAndAttr(FT_COMPLETE_SOURCES, ED2K_SEARCH_OP_GREATER_EQUAL, pParams->uComplete);

	if (pParams->ulMinBitrate > 0)
		AddAndAttr(FT_MEDIA_BITRATE, ED2K_SEARCH_OP_GREATER_EQUAL, pParams->ulMinBitrate);

	if (pParams->ulMinLength > 0)
		AddAndAttr(FT_MEDIA_LENGTH, ED2K_SEARCH_OP_GREATER_EQUAL, pParams->ulMinLength);

	if (!pParams->strCodec.IsEmpty())
		AddAndAttr(FT_MEDIA_CODEC, pParams->strCodec);

	if (!pParams->strTitle.IsEmpty())
		AddAndAttr(FT_MEDIA_TITLE, pParams->strTitle);

	if (!pParams->strAlbum.IsEmpty())
		AddAndAttr(FT_MEDIA_ALBUM, pParams->strAlbum);

	if (!pParams->strArtist.IsEmpty())
		AddAndAttr(FT_MEDIA_ARTIST, pParams->strArtist);

	if (_SearchExpr2.m_aExpr.GetCount() > 0)
	{
		if (_SearchExpr.m_aExpr.GetCount() > 0)
			_SearchExpr.m_aExpr.InsertAt(0, CSearchAttr(SEARCHOPTOK_AND));
		_SearchExpr.Add(&_SearchExpr2);
	}

	if (thePrefs.GetVerbose())
	{
		_strSearchTree.Empty();
		DumpSearchTree(_SearchExpr, true);
		DebugLog(_T("Search Expr: %s"), _strSearchTree);
	}

	for (int j = 0; j < _SearchExpr.m_aExpr.GetCount(); j++)
	{
		const CSearchAttr& rSearchAttr = _SearchExpr.m_aExpr[j];
		const CStringA& rstrA = rSearchAttr.m_str;
		if (rstrA == SEARCHOPTOK_AND)
		{
			target.WriteBooleanAND();
		}
		else if (rstrA == SEARCHOPTOK_OR)
		{
			target.WriteBooleanOR();
		}
		else if (rstrA == SEARCHOPTOK_NOT)
		{
			target.WriteBooleanNOT();
		}
		else if (rSearchAttr.m_iTag == FT_FILESIZE			||
				 rSearchAttr.m_iTag == FT_SOURCES			||
				 rSearchAttr.m_iTag == FT_COMPLETE_SOURCES	||
				 rSearchAttr.m_iTag == FT_FILERATING		||
				 rSearchAttr.m_iTag == FT_MEDIA_BITRATE		||
				 rSearchAttr.m_iTag == FT_MEDIA_LENGTH)
		{
			// 11-Sep-2005 []: Kad comparison operators where changed to match the ED2K operators. For backward
			// compatibility with old Kad nodes, we map ">=val" and "<=val" to ">val-1" and "<val+1". This way,
			// the older Kad nodes will perform a ">=val" and "<=val".
			//
			// TODO: This should be removed in couple of months!
			if (rSearchAttr.m_uIntegerOperator == ED2K_SEARCH_OP_GREATER_EQUAL)
				target.WriteMetaDataSearchParam(rSearchAttr.m_iTag, ED2K_SEARCH_OP_GREATER, rSearchAttr.m_nNum - 1);
			else if (rSearchAttr.m_uIntegerOperator == ED2K_SEARCH_OP_LESS_EQUAL)
				target.WriteMetaDataSearchParam(rSearchAttr.m_iTag, ED2K_SEARCH_OP_LESS, rSearchAttr.m_nNum + 1);
			else
				target.WriteMetaDataSearchParam(rSearchAttr.m_iTag, rSearchAttr.m_uIntegerOperator, rSearchAttr.m_nNum);
		}
		else if (rSearchAttr.m_iTag == FT_FILETYPE			||
				 rSearchAttr.m_iTag == FT_FILEFORMAT		||
				 rSearchAttr.m_iTag == FT_MEDIA_CODEC		||
				 rSearchAttr.m_iTag == FT_MEDIA_TITLE		|| 
				 rSearchAttr.m_iTag == FT_MEDIA_ALBUM		|| 
				 rSearchAttr.m_iTag == FT_MEDIA_ARTIST)
		{
			ASSERT( rSearchAttr.m_uIntegerOperator == ED2K_SEARCH_OP_EQUAL );
			target.WriteMetaDataSearchParam(rSearchAttr.m_iTag, OptUtf8ToStr(rSearchAttr.m_str));
		}
		else
		{
			ASSERT( rSearchAttr.m_iTag == FT_FILENAME );
			ASSERT( rSearchAttr.m_uIntegerOperator == ED2K_SEARCH_OP_EQUAL );
			target.WriteMetaDataSearchParam(OptUtf8ToStr(rstrA));
		}
	}

	if (thePrefs.GetDebugServerSearchesLevel() > 0)
		Debug(_T("Search Data: %s\n"), target.GetDebugString());
	_SearchExpr.m_aExpr.RemoveAll();
	_SearchExpr2.m_aExpr.RemoveAll();
	return true;
}

bool CSearchResultsWnd::StartNewSearch(SSearchParams* pParams)
{
	ESearchType eSearchType = pParams->eType;

	if (eSearchType == SearchTypeEd2kServer || eSearchType == SearchTypeEd2kGlobal)
	{
		if (!CGlobalVariable::serverconnect->IsConnected()) 
		{   
			MessageBox(GetResString(IDS_ERR_NOTCONNECTED),GetResString(IDS_CAPTION),MB_ICONWARNING);
			delete pParams;
			//if (!CGlobalVariable::serverconnect->IsConnecting() && !CGlobalVariable::serverconnect->IsConnected())
			//	CGlobalVariable::serverconnect->ConnectToAnyServer();
			return false;
		}

		try
		{
			if (!DoNewEd2kSearch(pParams)) 
			{
				delete pParams;
				return false;
			}
		}
		catch (CMsgBoxException* ex)
		{
			AfxMessageBox(ex->m_strMsg, ex->m_uType, ex->m_uHelpID);
			ex->Delete();
			delete pParams;
			return false;
		}

		SearchStarted();
		return true;
	}

	if (eSearchType == SearchTypeKademlia)
	{
		// Kademlia 返回错误，但不Delete  pParams.在删除标签页一起删除。
		if (!Kademlia::CKademlia::IsRunning() || !Kademlia::CKademlia::IsConnected()) {
			MessageBox(GetResString(IDS_ERR_NOTCONNECTEDKAD),GetResString(IDS_CAPTION),MB_ICONWARNING);
			//VC-dgkang 2008年7月17日
			//delete pParams;
			//if (!Kademlia::CKademlia::IsRunning())
			//	Kademlia::CKademlia::Start();
			return false;
		}

		try
		{
			if (!DoNewKadSearch(pParams)) {
				//VC-dgkang 2008年7月17日
				//delete pParams;
				return false;
			}
		}
		catch (CMsgBoxException* ex)
		{
			AfxMessageBox(ex->m_strMsg, ex->m_uType, ex->m_uHelpID);
			ex->Delete();
			//VC-dgkang 2008年7月17日
			//delete pParams;
			return false;
		}

		SearchStarted();
		return true;
	}

	ASSERT(0);
	delete pParams;
	return false;
}

bool CSearchResultsWnd::DoNewEd2kSearch(SSearchParams* pParams)
{
	if (!CGlobalVariable::serverconnect->IsConnected())
		return false;

	bool bServerSupports64Bit = CGlobalVariable::serverconnect->GetCurrentServer() != NULL
								&& (CGlobalVariable::serverconnect->GetCurrentServer()->GetTCPFlags() & SRV_TCPFLG_LARGEFILES);
	bool bPacketUsing64Bit = false;
	CSafeMemFile data(100);
	if (!GetSearchPacket(&data, pParams, bServerSupports64Bit, &bPacketUsing64Bit) || data.GetLength() == 0)
		return false;

	CancelEd2kSearch();

	CStringA strResultType = pParams->strFileType;
	if (strResultType == ED2KFTSTR_PROGRAM)
		strResultType.Empty();
	m_nEd2kSearchID++;
	pParams->dwSearchID = m_nEd2kSearchID;
	CGlobalVariable::searchlist->NewSearch(&searchlistctrl, strResultType, m_nEd2kSearchID, pParams->eType, pParams->strExpression);
	canceld = false;

	if (m_uTimerLocalServer)
	{
		VERIFY( KillTimer(m_uTimerLocalServer) );
		m_uTimerLocalServer = 0;
	}

	// once we've sent a new search request, any previously received 'More' gets invalid.
	//CWnd* pWndFocus = GetFocus();
	//m_pwndParams->m_ctlMore.EnableWindow(FALSE);
	//if (pWndFocus && pWndFocus->m_hWnd == m_pwndParams->m_ctlMore.m_hWnd)
	//	m_pwndParams->m_ctlCancel.SetFocus();
	m_iSentMoreReq = 0;

	Packet* packet = new Packet(&data);
	packet->opcode = OP_SEARCHREQUEST;
	if (thePrefs.GetDebugServerTCPLevel() > 0)
		Debug(_T(">>> Sending OP__SearchRequest\n"));
	theStats.AddUpDataOverheadServer(packet->size);
	CGlobalVariable::serverconnect->SendPacket(packet,false);

	if (pParams->eType == SearchTypeEd2kGlobal && CGlobalVariable::serverconnect->IsUDPSocketAvailable())
	{
		// set timeout timer for local server
		m_uTimerLocalServer = SetTimer(TimerServerTimeout, 50000, NULL);

		if (thePrefs.GetUseServerPriorities())
			CGlobalVariable::serverlist->ResetSearchServerPos();

		if (globsearch)
		{
			delete searchpacket;
			searchpacket = NULL;
			m_b64BitSearchPacket = false;
		}
		searchpacket = packet;
		searchpacket->opcode = OP_GLOBSEARCHREQ; // will be changed later when actually sending the packet!!
		m_b64BitSearchPacket = bPacketUsing64Bit;
		servercount = 0;
		searchprogress.SetRange32(0, CGlobalVariable::serverlist->GetServerCount() - 1);
		globsearch = true;
	}
	else
	{
		globsearch = false;
		delete packet;
	}

	//CreateNewTab(pParams);

	searchlistctrl.ShowResults(pParams->dwSearchID);

	return true;
}
	
bool CSearchResultsWnd::SearchMore()
{
	if (!CGlobalVariable::serverconnect->IsConnected())
		return false;

	SetActiveSearchResultsIcon(m_nEd2kSearchID);
	canceld = false;

	Packet* packet = new Packet();
	packet->opcode = OP_QUERY_MORE_RESULT;
	if (thePrefs.GetDebugServerTCPLevel() > 0)
		Debug(_T(">>> Sending OP__QueryMoreResults\n"));
	theStats.AddUpDataOverheadServer(packet->size);
	CGlobalVariable::serverconnect->SendPacket(packet);
	m_iSentMoreReq++;
	return true;
}

bool CSearchResultsWnd::DoNewKadSearch(SSearchParams* pParams)
{
	if (!Kademlia::CKademlia::IsConnected())
		return false;

	int iPos = 0;
	pParams->strKeyword = pParams->strExpression.Tokenize(_T(" "), iPos);
	pParams->strKeyword.Trim();
	if (pParams->strKeyword.IsEmpty() || pParams->strKeyword.FindOneOf(_aszInvKadKeywordChars) != -1){
		CString strError;
		strError.Format(GetResString(IDS_KAD_SEARCH_KEYWORD_INVALID), _aszInvKadKeywordChars);
		throw new CMsgBoxException(strError, MB_ICONWARNING | MB_HELP, eMule_FAQ_Search - HID_BASE_PROMPT);
	}

	CSafeMemFile data(100);
	if (!GetSearchPacket(&data, pParams, true, NULL)/* || (!pParams->strBooleanExpr.IsEmpty() && data.GetLength() == 0)*/)
		return false;

	LPBYTE pSearchTermsData = NULL;
	UINT uSearchTermsSize = (UINT)data.GetLength();
	if (uSearchTermsSize){
		pSearchTermsData = new BYTE[uSearchTermsSize];
		data.SeekToBegin();
		data.Read(pSearchTermsData, uSearchTermsSize);
	}

	Kademlia::CSearch* pSearch = NULL;
	try
	{
		pSearch = Kademlia::CSearchManager::PrepareFindKeywords(pParams->bUnicode, pParams->strKeyword, uSearchTermsSize, pSearchTermsData);
		delete[] pSearchTermsData;
		if (!pSearch){
			ASSERT(0);
			return false;
		}
	}
	catch (CString strException)
	{
		delete[] pSearchTermsData;
		throw new CMsgBoxException(strException, MB_ICONWARNING | MB_HELP, eMule_FAQ_Search - HID_BASE_PROMPT);
	}
	pParams->dwSearchID = pSearch->GetSearchID();
	CStringA strResultType = pParams->strFileType;
	if (strResultType == ED2KFTSTR_PROGRAM)
		strResultType.Empty();
	CGlobalVariable::searchlist->NewSearch(&searchlistctrl, strResultType, pParams->dwSearchID, pParams->eType, pParams->strExpression);
	//CreateNewTab(pParams);
	searchlistctrl.ShowResults(pParams->dwSearchID);
	return true;
}

bool CSearchResultsWnd::CreateNewTab(SSearchParams* pParams)
{
    int iTabItems = searchselect.GetItemCount();
    for (int i = 0; i < iTabItems; i++)
	{
        TCITEM tci;
        tci.mask = TCIF_PARAM;
		if (searchselect.GetItem(i, &tci) && tci.lParam != NULL && ((const SSearchParams*)tci.lParam)->dwSearchID == pParams->dwSearchID)
			return false;
    }

	// add new tab
	TCITEM newitem;
	if (pParams->strExpression.IsEmpty())
		pParams->strExpression = _T("-");
	newitem.mask = TCIF_PARAM | TCIF_TEXT | TCIF_IMAGE;
	newitem.lParam = (LPARAM)pParams;
	pParams->strSearchTitle = (pParams->strSpecialTitle.IsEmpty() ? pParams->strExpression : pParams->strSpecialTitle);
	newitem.pszText = const_cast<LPTSTR>((LPCTSTR)pParams->strSearchTitle);
	newitem.cchTextMax = 0;

	if (pParams->bClientSharedFiles)
		newitem.iImage = sriClient;
	else if (pParams->eType == SearchTypeKademlia)
		newitem.iImage = sriKadActice;
	else if (pParams->eType == SearchTypeEd2kGlobal)
		newitem.iImage = sriGlobalActive;
	else
	{
		ASSERT( pParams->eType == SearchTypeEd2kServer );
		newitem.iImage = sriServerActive;
	}
	int itemnr = searchselect.InsertItem(INT_MAX, &newitem);
	if (!searchselect.IsWindowVisible())
		ShowSearchSelector(true);
	searchselect.SetCurSel(itemnr);
	//searchlistctrl.ShowResults(pParams->dwSearchID);
	return true;
}

bool CSearchResultsWnd::CanDeleteSearch(uint32 /*nSearchID*/) const
{
	return (searchselect.GetItemCount() > 0);
}

void CSearchResultsWnd::DeleteSearch(uint32 nSearchID)
{
	Kademlia::CSearchManager::StopSearch(nSearchID, false);

	TCITEM item;
	item.mask = TCIF_PARAM;
	item.lParam = -1;
	int i;
	for (i = 0; i < searchselect.GetItemCount(); i++) {
		if (searchselect.GetItem(i, &item) && item.lParam != -1 && item.lParam != NULL && ((const SSearchParams*)item.lParam)->dwSearchID == nSearchID)
			break;
	}
	if (item.lParam == -1 || item.lParam == NULL || ((const SSearchParams*)item.lParam)->dwSearchID != nSearchID)
		return;

	// delete search results
	if (!canceld && nSearchID == m_nEd2kSearchID)
		CancelEd2kSearch();
	//if (nSearchID == m_nEd2kSearchID)
		//m_pwndParams->m_ctlMore.EnableWindow(FALSE);
	CGlobalVariable::searchlist->RemoveResults(nSearchID);
	
	// clean up stored states (scrollingpos etc) for this search
	searchlistctrl.ClearResultViewState(nSearchID);
	
	// delete search tab
	int iCurSel = searchselect.GetCurSel();
	searchselect.DeleteItem(i);
	delete (SSearchParams*)item.lParam;

	int iTabItems = searchselect.GetItemCount();
	if (iTabItems > 0){
		// select next search tab
		if (iCurSel == CB_ERR)
			iCurSel = 0;
		else if (iCurSel >= iTabItems)
			iCurSel = iTabItems - 1;
		(void)searchselect.SetCurSel(iCurSel);	// returns CB_ERR if error or no prev. selection(!)
		iCurSel = searchselect.GetCurSel();		// get the real current selection
		if (iCurSel == CB_ERR)					// if still error
			iCurSel = searchselect.SetCurSel(0);
		if (iCurSel != CB_ERR){
			item.mask = TCIF_PARAM;
			item.lParam = NULL;
			if (searchselect.GetItem(iCurSel, &item) && item.lParam != NULL){
				searchselect.HighlightItem(iCurSel, FALSE);
				ShowResults((const SSearchParams*)item.lParam);
			}
		}
	}
	else{
		searchlistctrl.DeleteAllItems();
		ShowSearchSelector(false);
		searchlistctrl.NoTabs();
	}
}

bool CSearchResultsWnd::CanDeleteAllSearches() const
{
	return (searchselect.GetItemCount() > 0);
}

void CSearchResultsWnd::DeleteAllSearches()
{
	CancelEd2kSearch();

	for (int i = 0; i < searchselect.GetItemCount(); i++){
		TCITEM item;
		item.mask = TCIF_PARAM;
		item.lParam = -1;
		if (searchselect.GetItem(i, &item) && item.lParam != -1 && item.lParam != NULL){
			Kademlia::CSearchManager::StopSearch(((const SSearchParams*)item.lParam)->dwSearchID, false);
			delete (SSearchParams*)item.lParam;
		}
	}
	CGlobalVariable::searchlist->Clear();
	searchlistctrl.DeleteAllItems();
	ShowSearchSelector(false);
	searchselect.DeleteAllItems();

	//CWnd* pWndFocus = GetFocus();
	//m_pwndParams->m_ctlMore.EnableWindow(FALSE);
	//m_pwndParams->m_ctlCancel.EnableWindow(FALSE);
	//m_pwndParams->m_ctlStart.EnableWindow(TRUE);
	//if (pWndFocus && (pWndFocus->m_hWnd == m_pwndParams->m_ctlMore.m_hWnd || pWndFocus->m_hWnd == m_pwndParams->m_ctlCancel.m_hWnd))
	//	m_pwndParams->m_ctlStart.SetFocus();
}

void CSearchResultsWnd::ShowResults(const SSearchParams* pParams)
{
	// restoring the params works and is nice during development/testing but pretty annoying in practice.
	// TODO: maybe it should be done explicitly via a context menu function or such.
	//if (GetAsyncKeyState(VK_CONTROL) < 0)
		//m_pwndParams->SetParameters(pParams);

	//if (pParams->eType == SearchTypeEd2kServer)
	//{
	//	 m_pwndParams->m_ctlCancel.EnableWindow(pParams->dwSearchID == m_nEd2kSearchID && IsLocalEd2kSearchRunning());
	//}
	//else if (pParams->eType == SearchTypeEd2kGlobal)
	//{
	//	 m_pwndParams->m_ctlCancel.EnableWindow(pParams->dwSearchID == m_nEd2kSearchID && (IsLocalEd2kSearchRunning() || IsGlobalEd2kSearchRunning()));
	//}
	//else if (pParams->eType == SearchTypeKademlia)
	//{
	//	 m_pwndParams->m_ctlCancel.EnableWindow(Kademlia::CSearchManager::IsSearching(pParams->dwSearchID));
	//}
	searchlistctrl.ShowResults(pParams->dwSearchID);
}

void CSearchResultsWnd::OnSelChangeTab(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	CWaitCursor curWait; // this may take a while
	int cur_sel = searchselect.GetCurSel();
	if (cur_sel == -1)
		return;
	TCITEM item;
	item.mask = TCIF_PARAM;
	if (searchselect.GetItem(cur_sel, &item) && item.lParam != NULL)
	{
		searchselect.HighlightItem(cur_sel, FALSE);
		ShowResults((const SSearchParams*)item.lParam);
	}
	*pResult = 0;
}

LRESULT CSearchResultsWnd::OnCloseTab(WPARAM wParam, LPARAM /*lParam*/)
{
	TCITEM item;
	item.mask = TCIF_PARAM;
	if (searchselect.GetItem((int)wParam, &item) && item.lParam != NULL)
	{
		int nSearchID = ((const SSearchParams*)item.lParam)->dwSearchID;
		if (!canceld && (UINT)nSearchID == m_nEd2kSearchID)
			CancelEd2kSearch();
		DeleteSearch(nSearchID);
	}
	return TRUE;
}

LRESULT CSearchResultsWnd::OnDblClickTab(WPARAM wParam, LPARAM /*lParam*/)
{
	TCITEM item;
	item.mask = TCIF_PARAM;
	if (searchselect.GetItem((int)wParam, &item) && item.lParam != NULL)
	{
		//m_pwndParams->SetParameters((const SSearchParams*)item.lParam);
	}
	return TRUE;
}

void CSearchResultsWnd::UpdateCatTabs()
{
	int oldsel=m_cattabs.GetCurSel();
	m_cattabs.DeleteAllItems();
	for (int ix=0;ix<thePrefs.GetCatCount();ix++) {
		CString label=(ix==0)?GetResString(IDS_ALL):thePrefs.GetCategory(ix)->strTitle;
		label.Replace(_T("&"),_T("&&"));
		m_cattabs.InsertItem(ix,label);
	}
	if (oldsel>=m_cattabs.GetItemCount() || oldsel==-1)
		oldsel=0;

	m_cattabs.SetCurSel(oldsel);
	int flag;
	flag=(m_cattabs.GetItemCount()>1) ? SW_SHOW:SW_HIDE;
	
	GetDlgItem(IDC_CATTAB2)->ShowWindow(flag);
	GetDlgItem(IDC_STATIC_DLTOof)->ShowWindow(flag);
}

void CSearchResultsWnd::ShowSearchSelector(bool visible)
{
	WINDOWPLACEMENT wpSearchWinPos;
	WINDOWPLACEMENT wpSelectWinPos;
	searchselect.GetWindowPlacement(&wpSelectWinPos);
	searchlistctrl.GetWindowPlacement(&wpSearchWinPos);
	if (visible)
		wpSearchWinPos.rcNormalPosition.top = wpSelectWinPos.rcNormalPosition.bottom;
	else
		wpSearchWinPos.rcNormalPosition.top = wpSelectWinPos.rcNormalPosition.top;
	searchselect.ShowWindow(visible ? SW_SHOW : SW_HIDE);
	RemoveAnchor(searchlistctrl);
	searchlistctrl.SetWindowPlacement(&wpSearchWinPos);
	AddAnchor(searchlistctrl, TOP_LEFT, BOTTOM_RIGHT);
	GetDlgItem(IDC_CLEARALL)->ShowWindow(visible ? SW_SHOW : SW_HIDE);
	m_ctlFilter.ShowWindow(visible ? SW_SHOW : SW_HIDE);

}

void CSearchResultsWnd::OnDestroy()
{
	int iTabItems = searchselect.GetItemCount();
	__try{
		for (int i = 0; i < iTabItems; i++){
			TCITEM tci;
			tci.mask = TCIF_PARAM;
			if (searchselect.GetItem(i, &tci) && tci.lParam != NULL){
				delete (SSearchParams*)tci.lParam;
			}
		}
	}
	__except(true)
	{
	}
	m_imlSearchMethods.DeleteImageList();

	CResizableFormView::OnDestroy();
}

void CSearchResultsWnd::OnSize(UINT nType, int cx, int cy)
{
	CResizableFormView::OnSize(nType, cx, cy);
}

void CSearchResultsWnd::OnClose()
{
	// Do not pass the WM_CLOSE to the base class. Since we have a rich edit control *and* an attached auto complete
	// control, the WM_CLOSE will get generated by the rich edit control when user presses ESC while the auto complete
	// is open.
	//__super::OnClose();
}

BOOL CSearchResultsWnd::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	theApp.ShowHelp(eMule_FAQ_Search);
	return TRUE;
}

LRESULT CSearchResultsWnd::OnIdleUpdateCmdUI(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	//BOOL bSearchParamsWndVisible = theApp.emuledlg->searchwnd->IsSearchParamsWndVisible();
	//if (!bSearchParamsWndVisible) {
	//	m_ctlOpenParamsWnd.ShowWindow(SW_SHOW);
	//}
	//else {
	//	m_ctlOpenParamsWnd.ShowWindow(SW_HIDE);
	//}
	return 0;
}

void CSearchResultsWnd::OnBnClickedOpenParamsWnd()
{
	theApp.emuledlg->searchwnd->OpenParametersWnd();
}

void CSearchResultsWnd::OnSysCommand(UINT nID, LPARAM lParam)
{
	if (nID == SC_KEYMENU)
	{
		if (lParam == EMULE_HOTMENU_ACCEL)
			theApp.emuledlg->SendMessage(WM_COMMAND, IDC_HOTMENU);
		else
			theApp.emuledlg->SendMessage(WM_SYSCOMMAND, nID, lParam);
		return;
	}
	__super::OnSysCommand(nID, lParam);
}

bool CSearchResultsWnd::CanSearchRelatedFiles() const
{
	return CGlobalVariable::serverconnect->IsConnected() 
		&& CGlobalVariable::serverconnect->GetCurrentServer() != NULL 
		&& CGlobalVariable::serverconnect->GetCurrentServer()->GetRelatedSearchSupport();
}

void CSearchResultsWnd::SearchRelatedFiles(const CAbstractFile* file)
{
	SSearchParams* pParams = new SSearchParams;
	pParams->strExpression = _T("related::") + md4str(file->GetFileHash());
	pParams->strSpecialTitle = GetResString(IDS_RELATED) + _T(": ") + file->GetFileName();
	if (pParams->strSpecialTitle.GetLength() > 50)
		pParams->strSpecialTitle = pParams->strSpecialTitle.Left(50) + _T("...");
	StartSearch(pParams);
}


///////////////////////////////////////////////////////////////////////////////
// CSearchResultsSelector

BEGIN_MESSAGE_MAP(CSearchResultsSelector, CClosableTabCtrl)
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

BOOL CSearchResultsSelector::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case MP_RESTORESEARCHPARAMS:{
		int iTab = GetTabUnderContextMenu();
		if (iTab != -1) {
			GetParent()->SendMessage(UM_DBLCLICKTAB, (WPARAM)iTab);
			return TRUE;
		}
		break;
	  }
	}
	return CClosableTabCtrl::OnCommand(wParam, lParam);
}

void CSearchResultsSelector::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	CTitleMenu menu;
	menu.CreatePopupMenu();
	menu.AddMenuTitle(GetResString(IDS_SW_RESULT));
	menu.AppendMenu(MF_STRING, MP_RESTORESEARCHPARAMS, GetResString(IDS_RESTORESEARCHPARAMS));
	menu.AppendMenu(MF_STRING, MP_REMOVE, GetResString(IDS_FD_CLOSE));
	menu.SetDefaultItem(MP_RESTORESEARCHPARAMS);
	m_ptCtxMenu = point;
	ScreenToClient(&m_ptCtxMenu);
	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
}

LRESULT CSearchResultsWnd::OnChangeFilter(WPARAM wParam, LPARAM lParam)
{
	CWaitCursor curWait; // this may take a while

	m_nFilterColumn = (uint32)wParam;

	CStringArray astrFilter;
	CString strFullFilterExpr = (LPCTSTR)lParam;
	int iPos = 0;
	CString strFilter(strFullFilterExpr.Tokenize(_T(" "), iPos));
	while (!strFilter.IsEmpty()) {
		if (strFilter != _T("-"))
			astrFilter.Add(strFilter);
		strFilter = strFullFilterExpr.Tokenize(_T(" "), iPos);
	}

	bool bFilterDiff = (astrFilter.GetSize() != m_astrFilter.GetSize());
	if (!bFilterDiff) {
		for (int i = 0; i < astrFilter.GetSize(); i++) {
			if (astrFilter[i] != m_astrFilter[i]) {
				bFilterDiff = true;
				break;
			}
		}
	}

	if (!bFilterDiff)
		return 0;
	m_astrFilter.RemoveAll();
	m_astrFilter.Append(astrFilter);

	int iCurSel = searchselect.GetCurSel();
	if (iCurSel == -1)
		return 0;
	TCITEM item;
	item.mask = TCIF_PARAM;
	if (searchselect.GetItem(iCurSel, &item) && item.lParam != NULL)
		ShowResults((const SSearchParams*)item.lParam);
	return 0;
}

void CSearchResultsWnd::OnSearchListMenuBtnDropDown(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	CTitleMenu menu;
	menu.CreatePopupMenu();

	menu.AppendMenu(MF_STRING | (searchselect.GetItemCount() > 0 ? MF_ENABLED : MF_GRAYED), MP_REMOVEALL, GetResString(IDS_REMOVEALLSEARCH));
	menu.AppendMenu(MF_SEPARATOR);
	CMenu menuFileSizeFormat;
	menuFileSizeFormat.CreateMenu();
	menuFileSizeFormat.AppendMenu(MF_STRING, MP_SHOW_FILESIZE_DFLT, GetResString(IDS_DEFAULT));
	menuFileSizeFormat.AppendMenu(MF_STRING, MP_SHOW_FILESIZE_KBYTE, GetResString(IDS_KBYTES));
	menuFileSizeFormat.AppendMenu(MF_STRING, MP_SHOW_FILESIZE_MBYTE, GetResString(IDS_MBYTES));
	menuFileSizeFormat.CheckMenuRadioItem(MP_SHOW_FILESIZE_DFLT, MP_SHOW_FILESIZE_MBYTE, MP_SHOW_FILESIZE_DFLT + searchlistctrl.GetFileSizeFormat(), 0);
	menu.AppendMenu(MF_POPUP, (UINT_PTR)menuFileSizeFormat.m_hMenu, GetResString(IDS_DL_SIZE));

	CRect rc;
	m_btnSearchListMenu->GetWindowRect(&rc);
	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, rc.left, rc.bottom, this);
}

BOOL CSearchResultsWnd::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case MP_REMOVEALL:
		DeleteAllSearches();
		return TRUE;
	case MP_SHOW_FILESIZE_DFLT:
		searchlistctrl.SetFileSizeFormat(fsizeDefault);
		return TRUE;
	case MP_SHOW_FILESIZE_KBYTE:
		searchlistctrl.SetFileSizeFormat(fsizeKByte);
		return TRUE;
	case MP_SHOW_FILESIZE_MBYTE:
		searchlistctrl.SetFileSizeFormat(fsizeMByte);
		return TRUE;
	}
	return CResizableFormView::OnCommand(wParam, lParam);
}

void CSearchResultsWnd::OnBnClickedButtonSearch()
{
	CString str;
	m_SearchCtrl.GetWindowText(str);

	if(str.IsEmpty())
	{
		return;
	}
	else
	{
		//VC-dgkang 2008年7月9日
		if (searchlistctrl.GetItemCount() != 0)
		{
			ESearchType SearchType = SearchAreaToType((ESearchArea)m_ctlMethod.GetCurSel());
			CmdFuncs::CreateNewTabForSearchED2K(str,&SearchType);
		}
		else
		{
			CmdFuncs::SetResActiveSearchTabText(GetResString(IDS_TABTITLE_SEARCH_RESULT) + str);

			SSearchParams * pSearchParams = GetParameters(str);
			if (StartSearch(pSearchParams))
				CmdFuncs::UpdateResSearchParam(m_iCurSearchIndexInRes, pSearchParams);
			//VC-dgkang 2008年7月9日
			theApp.emuledlg->m_mainTabWnd.m_dlgResource.UpdateEMsClosableStatus();//当ED2K搜索的Caption发生变化时，亦要坐些更新。
		}
	}
}

SSearchParams* CSearchResultsWnd::GetParameters(CString expression)
{
	CString strExpression = expression;

	CString strExtension;
	UINT uAvailability = 0;
	UINT uComplete = 0;
	CString strCodec;
	ULONG ulMinBitrate = 0;
	ULONG ulMinLength = 0;

	SSearchParams* pParams = new SSearchParams;
	pParams->strExpression = strExpression;
	pParams->eType         = SearchTypeEd2kGlobal;
	pParams->strFileType   = _T("");
	pParams->strMinSize    = _T("");
	pParams->ullMinSize    = 0;
	pParams->strMaxSize    = _T("");
	pParams->ullMaxSize    = 0;
	pParams->uAvailability = uAvailability;
	pParams->strExtension  = strExtension;
	pParams->uComplete     = uComplete;
	pParams->strCodec      = strCodec;
	pParams->ulMinBitrate  = ulMinBitrate;
	pParams->ulMinLength   = ulMinLength;

	pParams->eType         = SearchAreaToType((ESearchArea)m_ctlMethod.GetCurSel());

	return pParams;
}

void CSearchResultsWnd::UpdateParamDisplay(SSearchParams* pParams)
{
	m_ctlMethod.SetCurSel(SearchTypeToArea(pParams->eType));
	m_SearchCtrl.SetWindowText(pParams->strExpression);
}

ESearchType CSearchResultsWnd::SearchAreaToType(ESearchArea eArea)
{
	switch(eArea)
	{
	case SearchAreaEd2kGlobal:
		return SearchTypeEd2kGlobal;
		break;
	case SearchAreaKademlia:
		return SearchTypeKademlia;
		break;
	default:
		break;
	}

	return SearchTypeEd2kGlobal;
}

CSearchResultsWnd::ESearchArea CSearchResultsWnd::SearchTypeToArea(ESearchType eType)
{
	switch(eType)
	{
	case SearchTypeEd2kGlobal:
		return SearchAreaEd2kGlobal;
		break;
	case SearchTypeKademlia:
		return SearchAreaKademlia;
		break;
	default:
		break;
	}
	return SearchAreaEd2kGlobal;
}

BOOL CSearchResultsWnd::PreTranslateMessage(MSG* pMsg)
{

	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN)
		{
			OnBnClickedButtonSearch();
		}
	}

	return CResizableFormView::PreTranslateMessage(pMsg);
}

void CSearchResultsWnd::SetPos(int pos)
{
	searchprogress.SetPos(pos);
	CancelEd2kSearch();
}
