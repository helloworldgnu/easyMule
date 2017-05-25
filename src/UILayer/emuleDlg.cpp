/* 
 * $Id: emuleDlg.cpp 11398 2009-03-17 11:00:27Z huby $
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
#include <math.h>
#include <afxinet.h>
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
#include <HtmlHelp.h>
#include <share.h>
#include "emule.h"
#include "emuleDlg.h"
#include "ServerWnd.h"
#include "KademliaWnd.h"
#include "TransferWnd.h"
#include "SearchResultsWnd.h"
#include "SearchDlg.h"
#include "SharedFilesWnd.h"
#include "ChatWnd.h"
#include "IrcWnd.h"
#include "StatisticsDlg.h"
#include "CreditsDlg.h"
#include "PreferencesDlg.h"
#include "Sockets.h"
#include "KnownFileList.h"
#include "ServerList.h"
#include "Opcodes.h"
#include "SharedFileList.h"
#include "ED2KLink.h"
#include "Splashscreen.h"
#include "PartFileConvert.h"
#include "EnBitmap.h"
#include "Wizard.h"
#include "Exceptions.h"
#include "SearchList.h"
#include "HTRichEditCtrl.h"
#include "FrameGrabThread.h"
#include "kademlia/kademlia/kademlia.h"
#include "kademlia/kademlia/SearchManager.h"
#include "kademlia/routing/RoutingZone.h"
#include "kademlia/routing/contact.h"
#include "kademlia/kademlia/prefs.h"
#include "KadSearchListCtrl.h"
#include "KadContactListCtrl.h"
#include "PerfLog.h"
#include "DropTarget.h"
#include "LastCommonRouteFinder.h"
#include "WebServer.h"
#include "MMServer.h"
#include "DownloadQueue.h"
#include "ClientUDPSocket.h"
#include "UploadQueue.h"
#include "ClientList.h"
#include "UploadBandwidthThrottler.h"
#include "FriendList.h"
#include "IPFilter.h"
#include "Statistics.h"
//#include "MuleToolbarCtrl.h"
#include "TaskbarNotifier.h"
#include "MuleStatusbarCtrl.h"
#include "ListenSocket.h"
#include "Server.h"
#include "PartFile.h"
#include "Scheduler.h"
#include "ClientCredits.h"
#include "MenuCmds.h"
#include "MuleSystrayDlg.h"
#include "IPFilterDlg.h"
#include "WebServices.h"
#include "DirectDownloadDlg.h"
#include "PeerCacheFinder.h"
#include "Statistics.h"
#include "FirewallOpener.h"
#include "StringConversion.h"
#include "aichsyncthread.h"
#include "Log.h"
#include "MiniMule.h"
#include "UserMsgs.h"
#include "TextToSpeech.h"
#include "Collection.h"
#include "CollectionViewDialog.h"

#include "VisualStylesXP.h"
#include "Preferences.h"

#include "DlgAddTask.h" // Added by Soar Chin 09/06/2007
#include "WebbrowserWnd.h" // Added by thilon on 2006.08.01
#include "UPnpAsynThreads.h"//ADDED by fengwen on 2006/11/01 : 
#include "Ini2.h"//ADDED by fengwen on 2006/11/01 : 
#include "ThreadsMgr.h" //ADDED by fengwen on 2007/01/11 :
#include "UIMessage.h"
#include "CmdFuncs.h"
#include "WndMgr.h"
#include "MessageLog.h"
#include "Internal/InternalSocket.h" // VC-kernel[2007-01-22]:
#include "PopMule.h" // VC-SearchDream[2007-05-18]: Pop Mule
#include "SyncHelper.h" // VC-SearchDream[2007-05-24]: Sync Helper
#include "UpdateInfo.h"

#include "NatTraversal/NatThread.h"

#include "commctrl.h"	//Added by thilon on 2007.02.12, for MenuXP
#include "CIF.h"		//Added by thilon on 2007.02.12, for MenuXP

#include "LocalizeMgr.h"
#include "Util.h"
#include "GlobalVariable.h"

#include "PopMule.h" // VC-SearchDream[2007-05-18]: Pop Mule
#include "SyncHelper.h" // VC-SearchDream[2007-05-24]: Sync Helper

#include "CloseModeDlg.h"

#include "UpdateInfo.h"

#include "Version.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define BUF_SIZE 1024 * 2 //有需求扩大共享内存 VC-dgkang
#define  TIMER_ID 1234

#include "IP2Country.h" //EastShare - added by AndCycle, IP to Country
#include ".\emuledlg.h"

#define	SYS_TRAY_ICON_COOKIE_FORCE_UPDATE	(UINT)-1

BOOL (WINAPI *_TransparentBlt)(HDC, int, int, int, int, HDC, int, int, int, int, UINT) = NULL;
const static UINT UWM_ARE_YOU_EMULE = RegisterWindowMessage(EMULE_GUID);
UINT _uMainThreadId = 0;

///////////////////////////////////////////////////////////////////////////
// CemuleDlg Dialog

IMPLEMENT_DYNAMIC(CMsgBoxException, CException)

BEGIN_MESSAGE_MAP(CemuleDlg, CTrayDialog)
	///////////////////////////////////////////////////////////////////////////
	// Windows messages
	//
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_ENDSESSION()
	ON_WM_SIZE()
//	ON_WM_CLOSE()
	ON_WM_MENUCHAR()
	ON_WM_QUERYENDSESSION()
	ON_WM_SYSCOLORCHANGE()
	ON_MESSAGE(WM_COPYDATA, OnWMData)
	ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
	ON_MESSAGE(WM_USERCHANGED, OnUserChanged)
	ON_MESSAGE(WM_NAT_DISCONNECTED, OnNatDisconnect)
	ON_WM_SHOWWINDOW()
	ON_WM_DESTROY()
	ON_WM_SETTINGCHANGE()

	///////////////////////////////////////////////////////////////////////////
	// WM_COMMAND messages
	//
	ON_COMMAND(MP_CONNECT, StartConnection)
	ON_COMMAND(MP_DISCONNECT, CloseConnection)
	ON_COMMAND(MP_EXIT, OnClose)
	ON_COMMAND(MP_RESTORE, RestoreWindow)
	// quick-speed changer -- 
	ON_COMMAND_RANGE(MP_QS_U10, MP_QS_UP10, QuickSpeedUpload)
	ON_COMMAND_RANGE(MP_QS_D10, MP_QS_DC, QuickSpeedDownload)
	//--- quickspeed - paralize all ---
	ON_COMMAND_RANGE(MP_QS_PA, MP_QS_UA, QuickSpeedOther)
	// quick-speed changer -- based on xrmb	
	ON_NOTIFY_EX_RANGE(RBN_CHEVRONPUSHED, 0, 0xFFFF, OnChevronPushed)

	ON_REGISTERED_MESSAGE(UWM_ARE_YOU_EMULE, OnAreYouEmule)
	ON_BN_CLICKED(IDC_HOTMENU, OnBnClickedHotmenu)

	///////////////////////////////////////////////////////////////////////////
	// WM_USER messages
	//
	ON_MESSAGE(UM_TASKBARNOTIFIERCLICKED, OnTaskbarNotifierClicked)
	ON_MESSAGE(UM_CLOSE_MINIMULE, OnCloseMiniMule)
	ON_MESSAGE(UM_STARTED2KUPDATE, OnStartED2KUpdate)
	ON_MESSAGE(UM_EASYMULECHECKUPDATEFINISHED, OnCheckUpdateFinished)
	ON_MESSAGE(UM_ED2KUPDATECOMPLETE, OnED2KUpdateComplete)
	
	// Webserver messages
	ON_MESSAGE(WEB_GUI_INTERACTION, OnWebGUIInteraction)
	ON_MESSAGE(WEB_CLEAR_COMPLETED, OnWebServerClearCompleted)
	ON_MESSAGE(WEB_FILE_RENAME, OnWebServerFileRename)
	ON_MESSAGE(WEB_ADDDOWNLOADS, OnWebAddDownloads)
	ON_MESSAGE(WEB_CATPRIO, OnWebSetCatPrio)
#if _ENABLE_NOUSE
	ON_MESSAGE(WEB_ADDREMOVEFRIEND, OnAddRemoveFriend)
#endif
	// Version Check DNS
	ON_MESSAGE(UM_VERSIONCHECK_RESPONSE, OnVersionCheckResponse)

	// PeerCache DNS
	ON_MESSAGE(UM_PEERCHACHE_RESPONSE, OnPeerCacheResponse)

	///////////////////////////////////////////////////////////////////////////
	// WM_APP messages
	//
	ON_MESSAGE(WM_FILE_FINISHEDHASHING, OnFileHashed)
	ON_MESSAGE(WM_FILE_OPPROGRESS, OnFileOpProgress)
	ON_MESSAGE(WM_FILE_HASHFAILED, OnHashFailed)
	ON_MESSAGE(TM_FRAMEGRABFINISHED, OnFrameGrabFinished)
	ON_MESSAGE(TM_FILEALLOCEXC, OnFileAllocExc)
	ON_MESSAGE(WM_FILE_DOWNLOAD_COMPLETED, OnFileCompleted)
	ON_MESSAGE(WM_FILE_ADD_COMPLETED, OnFileAddCompleted)
	
	ON_MESSAGE(TM_CONSOLETHREADEVENT, OnConsoleThreadEvent)
		
	ON_MESSAGE(WM_PLAYER_DATA_REQ, OnPlayerRequestData) // VC-SearchDream[2007-05-24]: For Player Request
	///////////////////////////////////////////////////////////////////////////
	//Added by thilon on 2006.10.27, for UPnP
	//
	ON_MESSAGE(UM_USER_MAPPING_FINISHED, OnMappingFinished)
	ON_MESSAGE(UM_PORT_CHANGED, OnPortChanged)	//ADDED by fengwen on 2007/01/10 : redo upnp when ports have been changed.
	ON_WM_ERASEBKGND()
	ON_WM_MEASUREITEM()
	ON_WM_DRAWITEM()
	ON_WM_INITMENUPOPUP()

	ON_MESSAGE(WM_HOTKEY,OnHotKey)	// VC-kernel[2007-05-14]:hotkey
	ON_WM_CLOSE()
	ON_WM_TIMER()

	ON_MESSAGE(UM_NAT_CHECK_STRATEGIES, OnNatCheckStrageties) //ADDED by VC-fengwen on 2007/09/19 : 为防止多线程访问数据冲突，暂时把操作放到主线程。
	ON_MESSAGE(UM_REMOVEFROMALLQ_IN_THROTTLER, OnRemoveFromAllQ_In_Throttler)
END_MESSAGE_MAP()

CemuleDlg::CemuleDlg(CWnd* pParent /*=NULL*/)
	: CTrayDialog(CemuleDlg::IDD, pParent),
	m_mgrShortcuts(FALSE)
{
	_uMainThreadId = GetCurrentThreadId();
	preferenceswnd = new CPreferencesDlg;
	serverwnd = new CServerWnd;
	kademliawnd = new CKademliaWnd;
	transferwnd = new CTransferWnd;
	sharedfileswnd = new CSharedFilesWnd;
	searchwnd = new CSearchDlg;
#if _ENABLE_NOUSE
	chatwnd = new CChatWnd;
	ircwnd = new CIrcWnd;
#endif
	statisticswnd = new CStatisticsDlg;
	//toolbar = new CMuleToolbarCtrl;
	statusbar = new CMuleStatusBarCtrl;
	m_wndTaskbarNotifier = new CTaskbarNotifier;
	
	//Chocobo Start
	//eMule自动更新，added by Chocobo on 2006.07.31
	m_bCreated = false;
	//m_pEmuleUpdater = NULL;// 自动升级窗口指针置零
	m_pUpdaterThread = NULL;
	//Chocobo End

	m_hIcon = NULL;
	CGlobalVariable::m_app_state = APP_STATE_RUNNING;
	ready = false; 
	m_bStartMinimizedChecked = false;
	m_bStartMinimized = false;
	memset(&m_wpFirstRestore, 0, sizeof m_wpFirstRestore);
	m_uUpDatarate = 0;
	m_uDownDatarate = 0;
	status = 0;
	activewnd = NULL;
	for (int i = 0; i < ARRSIZE(connicons); i++)
		connicons[i] = NULL;
	transicons[0] = NULL;
	transicons[1] = NULL;
	transicons[2] = NULL;
	transicons[3] = NULL;
	imicons[0] = NULL;
	imicons[1] = NULL;
	imicons[2] = NULL;
	m_iMsgIcon = 0;
	m_iMsgBlinkState = false;
	m_icoSysTrayConnected = NULL;
	m_icoSysTrayDisconnected = NULL;
	m_icoSysTrayLowID = NULL;
	usericon = NULL;
	m_icoSysTrayCurrent = NULL;
	m_hTimer = 0;
	notifierenabled = false;
	m_pDropTarget = new CMainFrameDropTarget;

	m_pSystrayDlg = NULL;
	m_pMiniMule = NULL;
	m_pPopMule  = NULL; // VC-SearchDream[2007-05-18]: Pop Mule
	m_uLastSysTrayIconCookie = SYS_TRAY_ICON_COOKIE_FORCE_UPDATE;
	m_pPopMule  = NULL; // VC-SearchDream[2007-05-18]: Pop Mule
	m_boss = true;		// VC-kernel[2007-05-14]:hotkey

	m_hMapping = NULL;
	m_lpData = NULL;
	m_dwTimer_ConnectingTray = 0;
	m_bConnectingIconSide = FALSE;
	m_called_OnMappingFinished = false;
	m_MappingValueList.RemoveAll();
}

CemuleDlg::~CemuleDlg()
{
	try {
	CloseTTS();
	DestroyMiniMule();
	if (m_icoSysTrayCurrent) VERIFY( DestroyIcon(m_icoSysTrayCurrent) );
	if (m_hIcon) VERIFY( ::DestroyIcon(m_hIcon) );
	for (int i = 0; i < ARRSIZE(connicons); i++){
		if (connicons[i]) VERIFY( ::DestroyIcon(connicons[i]) );
	}
	if (transicons[0]) VERIFY( ::DestroyIcon(transicons[0]) );
	if (transicons[1]) VERIFY( ::DestroyIcon(transicons[1]) );
	if (transicons[2]) VERIFY( ::DestroyIcon(transicons[2]) );
	if (transicons[3]) VERIFY( ::DestroyIcon(transicons[3]) );
	if (imicons[0]) VERIFY( ::DestroyIcon(imicons[0]) );
	if (imicons[1]) VERIFY( ::DestroyIcon(imicons[1]) );
	if (imicons[2]) VERIFY( ::DestroyIcon(imicons[2]) );
	if (m_icoSysTrayConnected) VERIFY( ::DestroyIcon(m_icoSysTrayConnected) );
	if (m_icoSysTrayDisconnected) VERIFY( ::DestroyIcon(m_icoSysTrayDisconnected) );
	if (m_icoSysTrayLowID) VERIFY( ::DestroyIcon(m_icoSysTrayLowID) );
	if (usericon) VERIFY( ::DestroyIcon(usericon) );

	// already destroyed by windows?
	//VERIFY( m_menuUploadCtrl.DestroyMenu() );
	//VERIFY( m_menuDownloadCtrl.DestroyMenu() );
	//VERIFY( m_SysMenuOptions.DestroyMenu() );
	try{

	delete preferenceswnd;
	delete serverwnd;
	delete kademliawnd;
	delete transferwnd;
	delete sharedfileswnd;
#if _ENABLE_NOUSE
	delete chatwnd;
	delete ircwnd;
#endif
	delete statisticswnd;
	//delete toolbar;
	delete statusbar;
	delete m_wndTaskbarNotifier;
	delete m_pDropTarget;
	
	}
	catch(...){
	}
	//Chocobo Start
	//eMule自动更新，added by Chocobo on 2006.07.31
	//delete m_pEmuleUpdater;
	//Chocobo End
/*   // VC-Huby[2007-02-07]: chang to autodelete the updaterThread
	if (NULL != m_pUpdaterThread)
	{
		delete m_pUpdaterThread;
		m_pUpdaterThread = NULL;
	}
*/
	if(m_lpData != NULL)
	{
		UnmapViewOfFile(m_lpData);
		m_lpData = NULL;

	}

	if(m_hMapping != NULL)
	{
		CloseHandle(m_hMapping);
		m_hMapping = NULL;
	}

	}
	catch(...)
	{
	}
}

void CemuleDlg::DoDataExchange(CDataExchange* pDX)
{
	CTrayDialog::DoDataExchange(pDX);
}

LRESULT CemuleDlg::OnAreYouEmule(WPARAM, LPARAM)
{
	return UWM_ARE_YOU_EMULE;
} 

BOOL CemuleDlg::OnInitDialog()
{
	InitShortcutManager();
	// VC-kernel[2007-05-14]:hotkey
	if(thePrefs.m_wHotKeyValue && thePrefs.m_wHotKeyMod)
	{
		WORD vk_mod,vk_mod2;
		vk_mod = thePrefs.m_wHotKeyMod;
		vk_mod2 = 0;

		if		(vk_mod == HOTKEYF_ALT)
			vk_mod2 = MOD_ALT;
		else if	(vk_mod == HOTKEYF_CONTROL)
			vk_mod2 = MOD_CONTROL;
		else if	(vk_mod == HOTKEYF_SHIFT)
			vk_mod2 = MOD_SHIFT;
		else if	(vk_mod == (HOTKEYF_CONTROL|HOTKEYF_ALT))
			vk_mod2 = (MOD_CONTROL|MOD_ALT);
		else if	(vk_mod == (HOTKEYF_CONTROL|HOTKEYF_SHIFT))
			vk_mod2 = (MOD_CONTROL|MOD_SHIFT);
		else if	(vk_mod == (HOTKEYF_ALT|HOTKEYF_SHIFT))
			vk_mod2 = (MOD_ALT|MOD_SHIFT);
		else if	(vk_mod == (HOTKEYF_CONTROL|HOTKEYF_ALT|HOTKEYF_SHIFT))
			vk_mod2 = (MOD_CONTROL|MOD_ALT|MOD_SHIFT);

		RegisterHotKey(this->GetSafeHwnd(),IDC_EMULEHOTKEY,vk_mod2,thePrefs.m_wHotKeyValue);
	}


	////Added by thilon on 2006.10.27, for UPnP
	//THREADPARMS* ptp = new THREADPARMS;
	//ptp->hWnd = m_hWnd;
	//AfxBeginThread(UPnPNatPort, ptp);
	ModifyStyle(0, WS_CLIPCHILDREN);


	InitMenuIcon();

	//ADDED by fengwen on 2006/11/01 <begin> : 异步执行upnp
	AddNatPortMappingAsyn();
	//ADDED by fengwen on 2006/11/01 <end> : 异步执行upnp

	m_bStartMinimized = theApp.DidWeAutoStart();//thePrefs.GetStartMinimized();
	if (!m_bStartMinimized)
		m_bStartMinimized = theApp.DidWeAutoStart();

	// temporary disable the 'startup minimized' option, otherwise no window will be shown at all
	if (thePrefs.IsFirstStart())
		m_bStartMinimized = false;

	// show splashscreen as early as possible to "entertain" user while starting emule up
	//if (thePrefs.UseSplashScreen() && !m_bStartMinimized)
	//	ShowSplash();

	// Create global GUI objects
	theApp.CreateAllFonts();
	theApp.CreateBackwardDiagonalBrush();

	CTrayDialog::OnInitDialog();
	InitWindowStyles(this);
	//CreateToolbarCmdIconMap();

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL){
		pSysMenu->AppendMenu(MF_SEPARATOR);

		ASSERT( (MP_ABOUTBOX & 0xFFF0) == MP_ABOUTBOX && MP_ABOUTBOX < 0xF000);
		pSysMenu->AppendMenu(MF_STRING, MP_ABOUTBOX, GetResString(IDS_ABOUTBOX));

		ASSERT( (MP_VERSIONCHECK & 0xFFF0) == MP_VERSIONCHECK && MP_VERSIONCHECK < 0xF000);
		pSysMenu->AppendMenu(MF_STRING, MP_VERSIONCHECK, GetResString(IDS_VERSIONCHECK));

		// remaining system menu entries are created later...
	}

#if _ENABLE_NOUSE
	CWnd* pwndToolbarX = toolbar;
	if (toolbar->Create(WS_CHILD | WS_VISIBLE, CRect(0,0,0,0), this, IDC_TOOLBAR))
	{
		toolbar->Init();
		if (thePrefs.GetUseReBarToolbar())
		{
		    if (m_ctlMainTopReBar.Create(WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
									     RBS_BANDBORDERS | RBS_AUTOSIZE | CCS_NODIVIDER, 
									     CRect(0, 0, 0, 0), this, AFX_IDW_REBAR))
		    {
			    CSize sizeBar;
			    VERIFY( toolbar->GetMaxSize(&sizeBar) );
			    REBARBANDINFO rbbi = {0};
			    rbbi.cbSize = sizeof(rbbi);
				rbbi.fMask = RBBIM_STYLE | RBBIM_SIZE | RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_IDEALSIZE | RBBIM_ID;
			    rbbi.fStyle = RBBS_NOGRIPPER | RBBS_BREAK | RBBS_USECHEVRON;
			    rbbi.hwndChild = toolbar->m_hWnd;
			    rbbi.cxMinChild = sizeBar.cy;
			    rbbi.cyMinChild = sizeBar.cy;
			    rbbi.cxIdeal = sizeBar.cx;
			    rbbi.cx = rbbi.cxIdeal;
				rbbi.wID = 0;
			    VERIFY( m_ctlMainTopReBar.InsertBand((UINT)-1, &rbbi) );
				toolbar->SaveCurHeight();
		    	toolbar->UpdateBackground();
    
			    pwndToolbarX = &m_ctlMainTopReBar;

//#ifdef EASY_UI
				m_ctlMainTopReBar.ShowWindow(SW_HIDE);
//#endif

		    }
		}
//#ifdef EASY_UI
		toolbar->ShowWindow(SW_HIDE);
//#endif
	}
#endif

	// set title
	SetWindowText( _T("VeryCD ") + GetResString(IDS_CAPTION) + _T(" v")+ CGlobalVariable::GetCurVersionLong());
//#ifdef _ENGLISH_VERSION
//	SetWindowText(GetResString(IDS_CAPTION) + CGlobalVariable::GetCurVersionLong());
//#else
//	SetWindowText(GetResString(IDS_CAPTION) + CGlobalVariable::GetCurVersionLong());
//#endif

	// Init taskbar notifier
	m_wndTaskbarNotifier->Create(CWnd::GetDesktopWindow()/*this*/);
	LoadNotifier(thePrefs.GetNotifierConfiguration());

	// set statusbar
	// the statusbar control is created as a custom control in the dialog resource,
	// this solves font and sizing problems when using large system fonts
	statusbar->SubclassWindow(GetDlgItem(IDC_STATUSBAR)->m_hWnd);
	statusbar->EnableToolTips(true);
	SetStatusBarPartsSize();

	// create main window dialog pages
	serverwnd->Create(IDD_SERVER);
	sharedfileswnd->Create(IDD_FILES);
	searchwnd->Create(this);
#if _ENABLE_NOUSE
	chatwnd->Create(IDD_CHAT);
	ircwnd->Create(IDD_IRC);
#endif
	transferwnd->Create(IDD_TRANSFER);
	statisticswnd->Create(IDD_STATISTICS);
	kademliawnd->Create(IDD_KADEMLIAWND);		

	// with the top rebar control, some XP themes look better with some additional lite borders.. some not..
	//serverwnd->ModifyStyleEx(0, WS_EX_STATICEDGE);
	//sharedfileswnd->ModifyStyleEx(0, WS_EX_STATICEDGE);
	//searchwnd->ModifyStyleEx(0, WS_EX_STATICEDGE);
	//chatwnd->ModifyStyleEx(0, WS_EX_STATICEDGE);
	//transferwnd->ModifyStyleEx(0, WS_EX_STATICEDGE);
	//statisticswnd->ModifyStyleEx(0, WS_EX_STATICEDGE);
	//kademliawnd->ModifyStyleEx(0, WS_EX_STATICEDGE);
	//ircwnd->ModifyStyleEx(0, WS_EX_STATICEDGE);

	// optional: restore last used main window dialog
	if (thePrefs.GetRestoreLastMainWndDlg()){
		switch (thePrefs.GetLastMainWndDlgID()){
		case IDD_SERVER:
			SetActiveDialog(serverwnd);
			break;
		case IDD_FILES:
			SetActiveDialog(sharedfileswnd);
			break;
		case IDD_SEARCH:
			SetActiveDialog(searchwnd);
			break;
		case IDD_TRANSFER:
			SetActiveDialog(transferwnd);
			break;
		case IDD_STATISTICS:
			SetActiveDialog(statisticswnd);
			break;
		case IDD_KADEMLIAWND:
			SetActiveDialog(kademliawnd);
			break;
#if _ENABLE_NOUSE
		case IDD_CHAT:
			SetActiveDialog(chatwnd);
			break;
		case IDD_IRC:
			SetActiveDialog(ircwnd);
			break;
		case IDD_WEBBROWSER:				//added by thilon on 2006.08.01
			SetActiveDialog(webbrowser);
			break;
#endif
		}
	}

#if _ENABLE_NOUSE
	// if still no active window, activate server window
	if (activewnd == NULL)
	{
		if(thePrefs.m_bShowBroswer && webbrowser!=NULL)
		{
			SetActiveDialog(webbrowser);
		}
		else
		{
            // VC-SearchDream[2006-11-09]:fix the bug : 0000009: eMule启动后的界面
			CString config = thePrefs.GetToolbarSettings();

			bool bServer = false;

			for (int i = 0; i < config.GetLength(); i += 2)
			{
				int index = _tstoi(config.Mid(i, 2));
				if (index == 2)
				{
					bServer = true;
					break;
				}
			}

			if (bServer)
			{
				SetActiveDialog(serverwnd);
			}
			else
			{
			    SetActiveDialog(transferwnd);
			}
			// VC-SearchDream[2006-11-09]:fix the bug : 0000009: eMule启动后的界面
		}
	}

#endif	
	SetAllIcons();
	Localize();

	// set updateintervall of graphic rate display (in seconds)
	//ShowConnectionState(false);

//#ifdef EASY_UI
	CRect rcClient, rcStatusbar;
	GetClientRect(&rcClient);
	statusbar->GetWindowRect(&rcStatusbar);
	rcClient.bottom -= rcStatusbar.Height();

	CRect		rtMainTabWnd;
	rtMainTabWnd = rcClient;
	rtMainTabWnd.DeflateRect(1, 0);
	

	m_mainTabWnd.CreateEx(rtMainTabWnd, this, IDC_MAIN_TAB_WND);
//#else
//	// adjust all main window sizes for toolbar height and maximize the child windows
//	CRect rcClient, rcToolbar, rcStatusbar;
//	GetClientRect(&rcClient);
//	pwndToolbarX->GetWindowRect(&rcToolbar);
//	statusbar->GetWindowRect(&rcStatusbar);
//	rcClient.top += rcToolbar.Height();
//	rcClient.bottom -= rcStatusbar.Height();
//#endif
	

//	CWnd* apWnds[] =
//	{
//		serverwnd,
//		kademliawnd,
//		transferwnd,
//		sharedfileswnd,
//		searchwnd,
//#if _ENABLE_NOUSE
//		chatwnd,
//		ircwnd,
//#endif
//		statisticswnd,
//		webbrowser	// Added by thilon on 2006.08.01
//	};
	//for (int i = 0; i < ARRSIZE(apWnds); i++)
	//	apWnds[i]->SetWindowPos(NULL, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), SWP_NOZORDER);

	// anchors
//#ifdef EASY_UI
	//AddAnchor(*statusbar,		BOTTOM_LEFT, BOTTOM_RIGHT);
//#else
//	AddAnchor(*serverwnd,		TOP_LEFT, BOTTOM_RIGHT);
//	AddAnchor(*kademliawnd,		TOP_LEFT, BOTTOM_RIGHT);
//	AddAnchor(*transferwnd,		TOP_LEFT, BOTTOM_RIGHT);
//	AddAnchor(*sharedfileswnd,	TOP_LEFT, BOTTOM_RIGHT);
//	AddAnchor(*searchwnd,		TOP_LEFT, BOTTOM_RIGHT);
//	AddAnchor(*chatwnd,			TOP_LEFT, BOTTOM_RIGHT);
//	AddAnchor(*ircwnd,			TOP_LEFT, BOTTOM_RIGHT);
//	AddAnchor(*statisticswnd,	TOP_LEFT, BOTTOM_RIGHT);
//	//AddAnchor(*webbrowser,      TOP_LEFT, BOTTOM_RIGHT); // Added by thilon on 2006.08.01
//	AddAnchor(*pwndToolbarX,	TOP_LEFT, TOP_RIGHT);
//	//AddAnchor(*statusbar,		BOTTOM_LEFT, BOTTOM_RIGHT);
//#endif

	AddAnchor(m_mainTabWnd.m_hWnd, TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(*statusbar,BOTTOM_LEFT, BOTTOM_RIGHT);
	statisticswnd->ShowInterval();

	// tray icon
	TraySetMinimizeToTray(thePrefs.GetMinTrayPTR());
	TrayMinimizeToTrayChange();
	MinTrayBtnHide();	//ADDED by fengwen on 2007/06/13	:不再显示此小按钮

	ShowTransferRate(true);
    ShowPing();
	searchwnd->UpdateCatTabs();

	////////////////////////////////////////////////////////////////////////////
	//  after creating all windows
	CGlobalVariable::Initialize(m_hWnd);

	sharedfileswnd->m_ctlSharedDirTree.ExpandList();		//Added by thilon on 2008.03.27 for 默认展开一层

	///////////////////////////////////////////////////////////////////////////
	// Restore saved window placement
	//
	WINDOWPLACEMENT wp = {0};
	wp.length = sizeof(wp);
	wp = thePrefs.GetEmuleWindowPlacement();

	if (m_bStartMinimized)
	{
		// To avoid the window flickering during startup we try to set the proper window show state right here.
		if (*thePrefs.GetMinTrayPTR())
		{
			// Minimize to System Tray
			//
			// Unfortunately this does not work. The eMule main window is a modal dialog which is invoked
			// by CDialog::DoModal which eventually calls CWnd::RunModalLoop. Look at 'MLF_SHOWONIDLE' and
			// 'bShowIdle' in the above noted functions to see why it's not possible to create the window
			// right in hidden state.

			//--- attempt #1
			//wp.showCmd = SW_HIDE;
			//TrayShow();
			//--- doesn't work at all

			//--- attempt #2
			//if (wp.showCmd == SW_SHOWMAXIMIZED)
			//	wp.flags = WPF_RESTORETOMAXIMIZED;
			//m_bStartMinimizedChecked = false; // post-hide the window..
			//--- creates window flickering

			//--- attempt #3
			// Minimize the window into the task bar and later move it into the tray bar
			if (wp.showCmd == SW_SHOWMAXIMIZED)
				wp.flags = WPF_RESTORETOMAXIMIZED;
			wp.showCmd = SW_MINIMIZE;
			m_bStartMinimizedChecked = false;

			// to get properly restored from tray bar (after attempt #3) we have to use a patched 'restore' window cmd..
			m_wpFirstRestore = thePrefs.GetEmuleWindowPlacement();
			m_wpFirstRestore.length = sizeof(m_wpFirstRestore);
			if (m_wpFirstRestore.showCmd != SW_SHOWMAXIMIZED)
				m_wpFirstRestore.showCmd = SW_SHOWNORMAL;
		}
		else 
		{
			// Minimize to System Taskbar
			//
			if (wp.showCmd == SW_SHOWMAXIMIZED)
				wp.flags = WPF_RESTORETOMAXIMIZED;
			wp.showCmd = SW_MINIMIZE; // Minimize window but do not activate it.
			m_bStartMinimizedChecked = true;
		}
	}
	else
	{
		// Allow only SW_SHOWNORMAL and SW_SHOWMAXIMIZED. Ignore SW_SHOWMINIMIZED to make sure the window
		// becomes visible. If user wants SW_SHOWMINIMIZED, we already have an explicit option for this (see above).
		if (wp.showCmd != SW_SHOWMAXIMIZED)
		{
			wp.showCmd = SW_SHOWNORMAL;
		}
		m_bStartMinimizedChecked = true;
	}
	SetWindowPlacement(&wp);

	if (thePrefs.GetWSIsEnabled())
		CGlobalVariable::webserver->StartServer();
	CGlobalVariable::mmserver->Init();

	VERIFY( (m_hTimer = ::SetTimer(NULL, NULL, 300, StartupTimer)) != NULL );
	if (thePrefs.GetVerbose() && !m_hTimer)
		AddDebugLogLine(true,_T("Failed to create 'startup' timer - %s"),GetErrorMessage(GetLastError()));

	theStats.starttime = GetTickCount();

	if (thePrefs.IsFirstStart())
	{
		if (NULL != theApp.m_pSplashThread)
		{
			theApp.m_pSplashThread->PostThreadMessage(WM_QUIT,NULL,NULL);
			theApp.m_pSplashThread=NULL;
		}
		
		// temporary disable the 'startup minimized' option, otherwise no window will be shown at all
		m_bStartMinimized = false;
		//DestroySplash();

		extern BOOL FirstTimeWizard();
		//MODIFIED by VC-fengwen on 2007/07/17 <begin>	: 两个对话框合并
		//if (FirstTimeWizard()){
		//	// start connection wizard
		//	CConnectionWizardDlg conWizard;
		//	conWizard.DoModal();
		//}
		FirstTimeWizard();
		//MODIFIED by VC-fengwen on 2007/07/17 <end>	: 两个对话框合并
	}

	VERIFY( m_pDropTarget->Register(this) );

	// initalize PeerCache
	CGlobalVariable::m_pPeerCache->Init(thePrefs.GetPeerCacheLastSearch(), thePrefs.WasPeerCacheFound(), thePrefs.IsPeerCacheDownloadEnabled(), thePrefs.GetPeerCachePort());
	
	// start aichsyncthread
	//MODIFIED by fengwen on 2007/01/11	<begin> : 退出时清理线程。
	//AfxBeginThread(RUNTIME_CLASS(CAICHSyncThread), THREAD_PRIORITY_BELOW_NORMAL,0);
	CThreadsMgr::BegingThreadAndRecDown(CThreadsMgr::CleanProc_WaitAndDelWinThd,
										RUNTIME_CLASS(CAICHSyncThread), THREAD_PRIORITY_BELOW_NORMAL,0);
	//MODIFIED by fengwen on 2007/01/11	<end> : 退出时清理线程。

	if (NULL != theApp.m_pSplashThread)
	{
		theApp.m_pSplashThread->PostThreadMessage(WM_QUIT,NULL,NULL);
		theApp.m_pSplashThread=NULL;
	}

	// debug info
	DebugLog(_T("Using '%s' as config directory"), thePrefs.GetMuleDirectory(EMULE_CONFIGDIR)); 
//#ifdef EASY_UI
	//CRect		rtMainTabWnd;
	//rtMainTabWnd = rcClient;
	//rtMainTabWnd.bottom = rcStatusbar.top - 1;

	//m_mainTabWnd.CreateEx(rtMainTabWnd, this, IDC_MAIN_TAB_WND);

	//InitAnchors();
//#endif

//COMMENTED by fengwen on 2007/04/05	<begin> :	移到初始完DownloadQueue之后，以避免重复添加下载任务。
	//ADDED by fengwen on 2007/02/07	<begin> :	防止eMuleDlg未初始化完成而丢失消息。
	//if (NULL != theApp.m_pSingleInst)
	//	theApp.m_pSingleInst->InitCompleted( &m_hWnd, sizeof (HWND) );
	//ADDED by fengwen on 2007/02/07	<end> :	防止eMuleDlg未初始化完成而丢失消息。
//COMMENTED by fengwen on 2007/04/05	<end> :	移到初始完DownloadQueue之后，以避免重复添加下载任务。
	TrayShow();

	//Added by Soar Chin 11/09/2007
	SetForegroundWindow();
	BringWindowToTop();

	SetTimer(TIMER_ID,BUFFER_TIME_LIMIT,NULL);

	return TRUE;
}


//Chocobo Start
//eMule自动更新，modified by Chocobo on 2006.07.31
//自动更新判断
// modders: dont remove or change the original versioncheck! (additionals are ok)
void CemuleDlg::DoVersioncheck(bool manual) 
{
	if (!manual && thePrefs.GetLastVC()!=0) 
	{
		CTime last(thePrefs.GetLastVC());
		struct tm tmTemp;
		time_t tLast=safe_mktime(last.GetLocalTm(&tmTemp));
		time_t tNow=safe_mktime(CTime::GetCurrentTime().GetLocalTm(&tmTemp));

		if ( (difftime(tNow,tLast) / 86400) < thePrefs.GetUpdateDays() )
		{
			return;
		}
	}

	m_bManualUpdte = manual;

	AddLogLine(false, _T("Start Check Emule VeryCD new version."));

	if (WSAAsyncGetHostByName(m_hWnd, UM_VERSIONCHECK_RESPONSE, "update.emule.org.cn", m_acVCDNSBuffer, sizeof(m_acVCDNSBuffer)) == 0)
	{ 
		AddLogLine(true,GetResString(IDS_NEWVERSIONFAILED));
	}
}
//Chocobo End

void CALLBACK CemuleDlg::StartupTimer(HWND /*hwnd*/, UINT /*uiMsg*/, UINT /*idEvent*/, DWORD /*dwTime*/)
{
	// NOTE: Always handle all type of MFC exceptions in TimerProcs - otherwise we'll get mem leaks
	try
	{
		switch(theApp.emuledlg->status){
			case 0:
				theApp.emuledlg->status++;
				theApp.emuledlg->ready = true;
				//  Comment UI
				//CGlobalVariable::sharedfiles->SetOutputCtrl(&theApp.emuledlg->sharedfileswnd->sharedfilesctrl);
				theApp.emuledlg->status++;
				break;
			case 1:
				break;
			case 2:
				theApp.emuledlg->status++;
				try{
					CGlobalVariable::serverlist->Init();
 					if(!theApp.emuledlg->m_called_OnMappingFinished)
					{
						POSITION pos = theApp.emuledlg->m_MappingValueList.GetHeadPosition();
						while(pos)
						{
							CMappingValue &pValue = theApp.emuledlg->m_MappingValueList.GetNext(pos);
							theApp.emuledlg->OnMappingFinished(pValue.m_wParam,pValue.m_lParam);
						}
						theApp.emuledlg->m_MappingValueList.RemoveAll();
					}
						
				   }
				catch(...){
					ASSERT(0);
					LogError(LOG_STATUSBAR,_T("Failed to initialize server list - Unknown exception"));
				}
				theApp.emuledlg->status++;
				break;
			case 3:
				break;
			case 4:{
				bool bError = false;
				theApp.emuledlg->status++;

				// NOTE: If we have an unhandled exception in CDownloadQueue::Init, MFC will silently catch it
				// and the creation of the TCP and the UDP socket will not be done -> client will get a LowID!
				try{
					//CGlobalVariable::downloadqueue->SetListenWnd(theApp.emuledlg->m_hWnd);
					//CGlobalVariable::filemgr.LoadDownloadFiles();
					CGlobalVariable::downloadqueue->Init();
				}
				catch(...){
					ASSERT(0);
					LogError(LOG_STATUSBAR,_T("Failed to initialize download queue - Unknown exception"));
					bError = true;
				}

				//ADDED by fengwen on 2007/04/05	<begin> :	标识eMule初始化已完成，可以接收另一个进程传进来的下载任务。
				if (NULL != theApp.m_pSingleInst)
					theApp.m_pSingleInst->InitCompleted(&(theApp.emuledlg->m_hWnd), sizeof (HWND) );
				//ADDED by fengwen on 2007/04/05	<end> :	标识eMule初始化已完成，可以接收另一个进程传进来的下载任务。
				if (NULL != theApp.m_pSingleInst2Loader)
					theApp.m_pSingleInst2Loader->InitCompleted(&(theApp.emuledlg->m_hWnd), sizeof (HWND) );


				if (!CGlobalVariable::listensocket->StartListening()) {
					CString strError;
					strError.Format(GetResString(IDS_MAIN_SOCKETERROR), thePrefs.GetPort());
					LogError(LOG_STATUSBAR, _T("%s"), strError);
					if (thePrefs.GetNotifierOnImportantError())
						theApp.emuledlg->ShowNotifier(strError, TBN_IMPORTANTEVENT);
					bError = true;
				}

				if (!CGlobalVariable::clientudp->Create()) {
					CString strError;
					strError.Format(GetResString(IDS_MAIN_SOCKETERROR), thePrefs.GetUDPPort());
					LogError(LOG_STATUSBAR, _T("%s"), strError);
					if (thePrefs.GetNotifierOnImportantError())
						theApp.emuledlg->ShowNotifier(strError, TBN_IMPORTANTEVENT);
				}
				
				if (!bError) // show the success msg, only if we had no serious error
					AddLogLine(true, GetResString(IDS_MAIN_READY), CGlobalVariable::GetCurVersionLong());

				// VC-kernel[2007-03-02]:
				//if (thePrefs.DoAutoConnect())
					theApp.emuledlg->OnBnClickedButton2();

				theApp.m_app_ready = TRUE;
				theApp.emuledlg->status++;
				break;
			}
			case 5:
				break;
			default:
				theApp.emuledlg->StopTimer();
		}
	}
	CATCH_DFLT_EXCEPTIONS(_T("CemuleDlg::StartupTimer"))
}

void CemuleDlg::StopTimer()
{
	if (m_hTimer)
	{
		VERIFY( ::KillTimer(NULL, m_hTimer) );
		m_hTimer = 0;
	}
	
	//Chocobo Start
	//eMuel自动更新,modified by Chocobo on 2006.07.31
	//强制自动更新
	//Changed by thilon on 2007.01.17, 在第一次启动时不检查更新
	if (thePrefs.UpdateNotify() && !thePrefs.IsFirstStart())
	{
		DoVersioncheck(false);
	}
	//Chocobo End
	
	if (theApp.pstrPendingLink != NULL)
	{
		OnWMData(NULL, (LPARAM)&theApp.sendstruct);
		delete theApp.pstrPendingLink;
	}
}

void CemuleDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	// Systemmenu-Speedselector
	if (nID >= MP_QS_U10 && nID <= MP_QS_UP10) {
		QuickSpeedUpload(nID);
		return;
	}
	if (nID >= MP_QS_D10 && nID <= MP_QS_DC) {
		QuickSpeedDownload(nID);
		return;
	}
	if (nID == MP_QS_PA || nID == MP_QS_UA) {
		QuickSpeedOther(nID);
		return;
	}

	switch (nID /*& 0xFFF0*/)
	{
		case MP_ABOUTBOX: {
			CCreditsDlg dlgAbout;
			dlgAbout.DoModal();
			break;
		}
		case MP_VERSIONCHECK:
			DoVersioncheck(true);
			break;
		case MP_CONNECT:
			StartConnection();
			break;
		case MP_DISCONNECT:
			CloseConnection();
			break;
		case SC_CLOSE:
			// VC-yunchenn.chen[2007-07-18]: REENTRY BUG. to check if CCloseModeDlg is DoModal.
			//  if so, do nothing.
			if(CCloseModeDlg::m_bAskingClose) break;

			if(thePrefs.GetCloseMode() == 0)
			{
				CCloseModeDlg dlg;
				if ( dlg.DoModal() != IDOK ) 
				{
					return;
				}
			}

			if( thePrefs.GetCloseMode() == 1 )
			{
				CloseImmeditely();
				return;
			}

			if(thePrefs.GetCloseMode() == 2)
			{
				CloseToTray();
				return;
			}
			break;
		case SC_MINIMIZE:
			ShowWindow(SW_HIDE);		//Added by thilon on 2008.04.01 for MiniToTray
			return;
		default:
			CTrayDialog::OnSysCommand(nID, lParam);
	}

	if ((nID & 0xFFF0) == SC_MINIMIZE		||
		(nID & 0xFFF0) == MP_MINIMIZETOTRAY	||
		(nID & 0xFFF0) == SC_RESTORE		||
		(nID & 0xFFF0) == SC_MAXIMIZE)
	{
		ShowTransferRate(true);
		ShowPing();
		transferwnd->UpdateCatTabTitles();
	}

	//不知为何，会有这一句，先注释掉。2007.07.10
	/*CTrayDialog::OnSysCommand(nID, lParam);*/ 
}

void CemuleDlg::PostStartupMinimized()
{
	if (!m_bStartMinimizedChecked)
	{
		//TODO: Use full initialized 'WINDOWPLACEMENT' and remove the 'OnCancel' call...
		// Isn't that easy.. Read comments in OnInitDialog..
		m_bStartMinimizedChecked = true;
		if (m_bStartMinimized)
		{
			if (theApp.DidWeAutoStart())
			{
				if (!thePrefs.mintotray) {
					thePrefs.mintotray = true;
					MinimizeWindow();
					thePrefs.mintotray = false;
				}
				else
					MinimizeWindow();
			}
			else
				MinimizeWindow();
		}
	}
}

void CemuleDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this);

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon(x, y, m_hIcon);
	}
	else
		CTrayDialog::OnPaint();
}

HCURSOR CemuleDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CemuleDlg::OnBnClickedButton2(){
	if (!CGlobalVariable::IsConnected())
		//connect if not currently connected
		if (!CGlobalVariable::serverconnect->IsConnecting() && !Kademlia::CKademlia::IsRunning() ){
			StartConnection();
		}
		else {
			CloseConnection();
			StartConnection();
		}
	else{
		//disconnect if currently connected
		CloseConnection();
	}
}

void CemuleDlg::ResetServerInfo(){
	serverwnd->servermsgbox->Reset();
}
void CemuleDlg::ResetLog(){
	serverwnd->logbox->Reset();
}

void CemuleDlg::ResetDebugLog(){
	serverwnd->debuglog->Reset();
}

//Xman Anti-Leecher-Log
void CemuleDlg::ResetLeecherLog()
{
	serverwnd->leecherlog->Reset();
}
//Xman end

void CemuleDlg::AddLogText(UINT uFlags, LPCTSTR pszText)
{
	if (GetCurrentThreadId() != _uMainThreadId)
	{
		CGlobalVariable::QueueLogLineEx(uFlags, _T("%s"), pszText);

		return;
	}

	if (uFlags & LOG_STATUSBAR)
	{
        if (statusbar->m_hWnd /*&& ready*/)
		{
			if (CGlobalVariable::m_app_state != APP_STATE_SHUTTINGDOWN)
				statusbar->SetText(pszText, SBarLog, 0);
		}
		else
			AfxMessageBox(pszText);
	}
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
	Debug(_T("%s\n"), pszText);
#endif

	if ((uFlags & LOG_DEBUG) && !thePrefs.GetVerbose())
		return;

	//Xman Anti-Leecher-Log
	if ((uFlags & LOG_LEECHER) && !thePrefs.GetAntiLeecherLog())
		return;

	TCHAR temp[1060];
	int iLen = _sntprintf(temp, ARRSIZE(temp), _T("%s: %s\r\n"), CTime::GetCurrentTime().Format(thePrefs.GetDateTimeFormat4Log()), pszText);
	if (iLen >= 0)
	{
		 //Xman Anti-Leecher-Log
		if (!(uFlags & LOG_DEBUG) && !(uFlags & LOG_LEECHER))
		{
			serverwnd->logbox->AddTyped(temp, iLen, uFlags & LOGMSGTYPEMASK);
			if (IsWindow(serverwnd->StatusSelector) && serverwnd->StatusSelector.GetCurSel() != CServerWnd::PaneLog)
				serverwnd->StatusSelector.HighlightItem(CServerWnd::PaneLog, TRUE);
			if (!(uFlags & LOG_DONTNOTIFY) && ready)
				ShowNotifier(pszText, TBN_LOG);
			if (thePrefs.GetLog2Disk())
				theLog.Log(temp, iLen);
		}
		else
		//Xman Anti-Leecher-Log
		if (thePrefs.GetAntiLeecherLog() && (uFlags & LOG_LEECHER))
		{
			serverwnd->leecherlog->AddTyped(temp, iLen, uFlags & LOGMSGTYPEMASK);
			if (IsWindow(serverwnd->StatusSelector) && serverwnd->StatusSelector.GetCurSel() != CServerWnd::PaneLeecherLog)
				serverwnd->StatusSelector.HighlightItem(CServerWnd::PaneLeecherLog, TRUE);

			if (thePrefs.GetDebug2Disk())
				theVerboseLog.Log(temp, iLen);
		}
		//Xman end
		else
		if (thePrefs.GetVerbose() && ((uFlags & LOG_DEBUG) || thePrefs.GetFullVerbose()))
		{
			serverwnd->debuglog->AddTyped(temp, iLen, uFlags & LOGMSGTYPEMASK);
			if (IsWindow(serverwnd->StatusSelector) && serverwnd->StatusSelector.GetCurSel() != CServerWnd::PaneVerboseLog)
				serverwnd->StatusSelector.HighlightItem(CServerWnd::PaneVerboseLog, TRUE);

			if (thePrefs.GetDebug2Disk())
				theVerboseLog.Log(temp, iLen);
		}
	}
}


CString CemuleDlg::GetLastLogEntry()
{
	return serverwnd->logbox->GetLastLogEntry();
}

CString CemuleDlg::GetAllLogEntries()
{
	return serverwnd->logbox->GetAllLogEntries();
}

CString CemuleDlg::GetLastDebugLogEntry()
{
	return serverwnd->debuglog->GetLastLogEntry();
	//return CString(_T(""));
}

CString CemuleDlg::GetAllDebugLogEntries()
{
	return serverwnd->debuglog->GetAllLogEntries();
	//return CString(_T(""));
}
CString CemuleDlg::GetServerInfoText()
{
	return serverwnd->servermsgbox->GetText();
}

void CemuleDlg::AddServerMessageLine(UINT uFlags, LPCTSTR pszLine)
{
	CString strMsgLine(pszLine);
	strMsgLine += _T('\n');
	if ((uFlags & LOGMSGTYPEMASK) == LOG_INFO)
		serverwnd->servermsgbox->AppendText(strMsgLine);
	else
		serverwnd->servermsgbox->AddTyped(strMsgLine, strMsgLine.GetLength(), uFlags & LOGMSGTYPEMASK);
	if (IsWindow(serverwnd->StatusSelector) && serverwnd->StatusSelector.GetCurSel() != CServerWnd::PaneServerInfo)
		serverwnd->StatusSelector.HighlightItem(CServerWnd::PaneServerInfo, TRUE);
}

UINT CemuleDlg::GetConnectionStateIconIndex() const
{
	

	if (CGlobalVariable::serverconnect->IsConnected() && !Kademlia::CKademlia::IsConnected())
	{
		if (CGlobalVariable::serverconnect->IsLowID())
			return 3; // LowNot
		else
			return 6; // HighNot
	}
	else if (!CGlobalVariable::serverconnect->IsConnected() && Kademlia::CKademlia::IsConnected())
	{
		if (Kademlia::CKademlia::IsFirewalled())
			return 1; // NotLow
		else
			return 2; // NotHigh
	}
	else if (CGlobalVariable::serverconnect->IsConnected() && Kademlia::CKademlia::IsConnected())
	{
		if (CGlobalVariable::serverconnect->IsLowID() && Kademlia::CKademlia::IsFirewalled())
			return 4; // LowLow
		else if (CGlobalVariable::serverconnect->IsLowID())
			return 5; // LowHigh
		else if (Kademlia::CKademlia::IsFirewalled())
			return 7; // HighLow
		else
			return 8; // HighHigh
	}
	else
	{
		return 0; // NotNot
	}
}

void CemuleDlg::ShowConnectionStateIcon()
{
	UINT uIconIdx = GetConnectionStateIconIndex();
	if (uIconIdx >= ARRSIZE(connicons)){
		ASSERT(0);
		uIconIdx = 0;
	}
	statusbar->SetIcon(SBarConnected, connicons[uIconIdx]);
}

CString CemuleDlg::GetConnectionStateString()
{
	CString status;

//MODIFIED by fengwen on 2007/06/04 <begin> 只显示“未连接”“连接中”“外网”“内网”
	//if (CGlobalVariable::serverconnect->IsConnected())
	//	status = _T("eD2K:") + GetResString(IDS_CONNECTED);
	//else if (CGlobalVariable::serverconnect->IsConnecting())
	//	status = _T("eD2K:") + GetResString(IDS_CONNECTING);
	//else
	//	status = _T("eD2K:") + GetResString(IDS_NOTCONNECTED);

	//if (Kademlia::CKademlia::IsConnected())
	//	status += _T("|Kad:") + GetResString(IDS_CONNECTED);
	//else if (Kademlia::CKademlia::IsRunning())
	//	status += _T("|Kad:") + GetResString(IDS_CONNECTING);
	//else
	//	status += _T("|Kad:") + GetResString(IDS_NOTCONNECTED);
	
	
	if (CGlobalVariable::serverconnect->IsConnected())
	{
		//MODIFIED by VC-fengwen on 2007/07/18 <begin>	:	不分内外网
		//if (CGlobalVariable::serverconnect->IsLowID())
		//	status = /*_T("eD2K:") + */GetResString(IDS_NAT_LAN);
		//else
		//	status = /*_T("eD2K:") + */GetResString(IDS_NAT_WAN);
		status = GetResString(IDS_CONNECTED);
		//MODIFIED by VC-fengwen on 2007/07/18 <end>	:	不分内外网

		//status += _T("  ");
		//if (0 == CGlobalVariable::GetPublicIP(true))
		//	status += ipstr(CGlobalVariable::serverconnect->GetLocalIP());
		//else
		//	status += ipstr(CGlobalVariable::GetPublicIP(true));

	}
	else if (CGlobalVariable::serverconnect->IsConnecting())
		status = /*_T("eD2K:") +*/ GetResString(IDS_CONNECTING);
	else
		status = /*_T("eD2K:") +*/ GetResString(IDS_NOTCONNECTED);

//MODIFIED by fengwen on 2007/06/04 <end> 只显示“未连接”“连接中”“外网”“内网”
	
	return status;
}

void CemuleDlg::ShowConnectionState()
{
	if(! CGlobalVariable::downloadqueue) return;

	CGlobalVariable::downloadqueue->OnConnectionState(CGlobalVariable::IsConnected());
	serverwnd->UpdateMyInfo();
	serverwnd->UpdateControlsState();
	kademliawnd->UpdateControlsState();

	ShowConnectionStateIcon();
	statusbar->SetText(GetConnectionStateString(), SBarConnected, 0);

	if (CGlobalVariable::IsConnected())
	{
#if _ENABLE_NOUSE		
		CString strPane(GetResString(IDS_MAIN_BTN_DISCONNECT));
		TBBUTTONINFO tbi;
		tbi.cbSize = sizeof(TBBUTTONINFO);
		tbi.dwMask = TBIF_IMAGE | TBIF_TEXT;
		tbi.iImage = 1;
		tbi.pszText = const_cast<LPTSTR>((LPCTSTR)strPane);
		toolbar->SetButtonInfo(TBBTN_CONNECT, &tbi);
#endif
		theWndMgr.SendMsgTo(CWndMgr::WI_ADVANCE_TOOLBAR, UM_TB_CHANGE_CONN_STATE, 2);

		TriggerConnectingTray(FALSE);
		UpdateTrayIcon(0);
	}
	else
	{
		if (CGlobalVariable::serverconnect->IsConnecting() || Kademlia::CKademlia::IsRunning()) 
		{
#if _ENABLE_NOUSE				
			CString strPane(GetResString(IDS_MAIN_BTN_CANCEL));
			TBBUTTONINFO tbi;
			tbi.cbSize = sizeof(TBBUTTONINFO);
			tbi.dwMask = TBIF_IMAGE | TBIF_TEXT;
			tbi.iImage = 2;
			tbi.pszText = const_cast<LPTSTR>((LPCTSTR)strPane);
			toolbar->SetButtonInfo(TBBTN_CONNECT, &tbi);
			ShowUserCount();
#endif
			theWndMgr.SendMsgTo(CWndMgr::WI_ADVANCE_TOOLBAR, UM_TB_CHANGE_CONN_STATE, 1);

			TriggerConnectingTray(TRUE);
		} 
		else 
		{
#if _ENABLE_NOUSE			
			CString strPane(GetResString(IDS_MAIN_BTN_CONNECT));
			TBBUTTONINFO tbi;
			tbi.cbSize = sizeof(TBBUTTONINFO);
			tbi.dwMask = TBIF_IMAGE | TBIF_TEXT;
			tbi.iImage = 0;
			tbi.pszText = const_cast<LPTSTR>((LPCTSTR)strPane);
			toolbar->SetButtonInfo(TBBTN_CONNECT, &tbi);
			ShowUserCount();
			
#endif
			theWndMgr.SendMsgTo(CWndMgr::WI_ADVANCE_TOOLBAR, UM_TB_CHANGE_CONN_STATE, 0);

			TriggerConnectingTray(FALSE);
		}


	}
}

void CemuleDlg::ShowUserCount()
{
//COMMENTED by fengwen on 2007/06/04 <begin>	状态栏不显示用户数
	//uint32 totaluser, totalfile;
	//totaluser = totalfile = 0;
	//CGlobalVariable::serverlist->GetUserFileStatus( totaluser, totalfile );
	//CString buffer;
	//buffer.Format(_T("%s:%s(%s)|%s:%s(%s)"), GetResString(IDS_UUSERS), CastItoIShort(totaluser, false, 1), CastItoIShort(Kademlia::CKademlia::GetKademliaUsers(), false, 1), GetResString(IDS_FILES), CastItoIShort(totalfile, false, 1), CastItoIShort(Kademlia::CKademlia::GetKademliaFiles(), false, 1));
	//statusbar->SetText(buffer, SBarUsers, 0);
//COMMENTED by fengwen on 2007/06/04 <end>	状态栏不显示用户数
}

void CemuleDlg::ShowMessageState(UINT iconnr)
{
	m_iMsgIcon = iconnr;
	statusbar->SetIcon(SBarChatMsg, imicons[m_iMsgIcon]);
}

void CemuleDlg::ShowTransferStateIcon()
{
	if (m_uUpDatarate && m_uDownDatarate)
		statusbar->SetIcon(SBarUpDown, transicons[3]);
	else if (m_uUpDatarate)
		statusbar->SetIcon(SBarUpDown, transicons[2]);
	else if (m_uDownDatarate)
		statusbar->SetIcon(SBarUpDown, transicons[1]);
	else
		statusbar->SetIcon(SBarUpDown, transicons[0]);
}

CString CemuleDlg::GetUpDatarateString(UINT uUpDatarate)
{
	m_uUpDatarate = uUpDatarate != (UINT)-1 ? uUpDatarate : CGlobalVariable::uploadqueue->GetDatarate();
	TCHAR szBuff[128];
	if (thePrefs.ShowOverhead())
		_sntprintf(szBuff, ARRSIZE(szBuff), _T("%.1f (%.1f)"), (float)m_uUpDatarate/1024, (float)theStats.GetUpDatarateOverhead()/1024);
	else
		_sntprintf(szBuff, ARRSIZE(szBuff), _T("%.1f"), (float)m_uUpDatarate/1024);
	return szBuff;
}

CString CemuleDlg::GetDownDatarateString(UINT uDownDatarate)
{
	m_uDownDatarate = uDownDatarate != (UINT)-1 ? uDownDatarate : CGlobalVariable::downloadqueue->GetDatarate();
	TCHAR szBuff[128];
	if (thePrefs.ShowOverhead())
		_sntprintf(szBuff, ARRSIZE(szBuff), _T("%.1f (%.1f)"), (float)m_uDownDatarate/1024, (float)theStats.GetDownDatarateOverhead()/1024);
	else
		_sntprintf(szBuff, ARRSIZE(szBuff), _T("%.1f"), (float)m_uDownDatarate/1024);
	return szBuff;
}

CString CemuleDlg::GetTransferRateString()
{
	TCHAR szBuff[128];
	if (thePrefs.ShowOverhead())
		_sntprintf(szBuff, ARRSIZE(szBuff), GetResString(IDS_UPDOWN),
				  (float)m_uUpDatarate/1024, (float)theStats.GetUpDatarateOverhead()/1024,
				  (float)m_uDownDatarate/1024, (float)theStats.GetDownDatarateOverhead()/1024);
	else
		_sntprintf(szBuff, ARRSIZE(szBuff), GetResString(IDS_UPDOWNSMALL), (float)m_uUpDatarate/1024, (float)m_uDownDatarate/1024);
	return szBuff;
}

void CemuleDlg::ShowTransferRate(bool bForceAll)
{
	if(!IsRunning())
		return;

	if (bForceAll)
		m_uLastSysTrayIconCookie = SYS_TRAY_ICON_COOKIE_FORCE_UPDATE;

	m_uDownDatarate = CGlobalVariable::downloadqueue->GetDatarate();
	m_uUpDatarate = CGlobalVariable::uploadqueue->GetDatarate();

	CString strTransferRate = GetTransferRateString();
	if (TrayIsVisible() || bForceAll)
	{
		TCHAR buffer2[128]; // VC-SearchDream[2006-11-14]:Change from 100 to 128 for bug 0000010
		// set trayicon-icon
		int iDownRateProcent = (int)ceil((m_uDownDatarate/10.24) / min(thePrefs.GetMaxGraphDownloadRate(), 200));
		if (iDownRateProcent > 100)
			iDownRateProcent = 100;
		UpdateTrayIcon(iDownRateProcent);

		if (CGlobalVariable::IsConnected()) 
			_sntprintf(buffer2, ARRSIZE(buffer2), _T("VeryCD %s v%s (%s)\r\n%s"), GetResString(IDS_CAPTION), CGlobalVariable::GetCurVersionLong(), GetResString(IDS_CONNECTED), strTransferRate);
		else
			_sntprintf(buffer2, ARRSIZE(buffer2), _T("VeryCD %s v%s (%s)\r\n%s"), GetResString(IDS_CAPTION), CGlobalVariable::GetCurVersionLong(), GetResString(IDS_DISCONNECTED), strTransferRate);

		buffer2[127] = _T('\0'); // VC-SearchDream[2006-11-14]:Change from 63 to 127 for bug 0000010
		TraySetToolTip(buffer2);
	}

	if (IsWindowVisible() || bForceAll)
	{
		statusbar->SetText(strTransferRate, SBarUpDown, 0);
		ShowTransferStateIcon();
	}
	if (IsWindowVisible() && thePrefs.ShowRatesOnTitle())
	{
		TCHAR szBuff[128];
		_sntprintf(szBuff, ARRSIZE(szBuff), _T("(U:%.1f D:%.1f) VeryCD %s v%s"), (float)m_uUpDatarate/1024, (float)m_uDownDatarate/1024, GetResString(IDS_CAPTION),CGlobalVariable::GetCurVersionLong());
		SetWindowText(szBuff);
	}
	if (m_pMiniMule && m_pMiniMule->m_hWnd && m_pMiniMule->IsWindowVisible() && !m_pMiniMule->GetAutoClose())
	{
		m_pMiniMule->UpdateContent(m_uUpDatarate, m_uDownDatarate);
	}
}

void CemuleDlg::ShowPing()
{
    if (IsWindowVisible())
	{

#ifdef _ENABLE_USS
        CString buffer;
        if (thePrefs.IsDynUpEnabled())
		{
			CurrentPingStruct lastPing = CGlobalVariable::lastCommonRouteFinder->GetCurrentPing();
            if (lastPing.state.GetLength() == 0)
			{
                if (lastPing.lowest > 0 && !thePrefs.IsDynUpUseMillisecondPingTolerance())
                    buffer.Format(_T("%.1f | %ims | %i%%"),lastPing.currentLimit/1024.0f, lastPing.latency, lastPing.latency*100/lastPing.lowest);
                else
                    buffer.Format(_T("%.1f | %ims"),lastPing.currentLimit/1024.0f, lastPing.latency);
            }
			else
                buffer.SetString(lastPing.state);
        }
		statusbar->SetText(buffer, SBarChatMsg, 0);
#endif

    }
}

void CemuleDlg::OnOK()
{
}

void CemuleDlg::OnCancel()
{
	if (!thePrefs.GetStraightWindowStyles())
		MinimizeWindow();
}

void CemuleDlg::MinimizeWindow()
{
	if (*thePrefs.GetMinTrayPTR())
	{
		TrayShow();
		ShowWindow(SW_HIDE);
	}
	else
	{
		ShowWindow(SW_MINIMIZE);
	}
	ShowTransferRate();
	ShowPing();
}

void CemuleDlg::SetActiveDialog(CWnd* /*dlg*/)
{
	return;	//ADDED by fengwen on 2007/02/26	: 使用了新的界面，防止以前的窗口不经意地显示出来。
}

void CemuleDlg::SetStatusBarPartsSize()
{
	CRect rect;
	statusbar->GetClientRect(&rect);
	int ussShift = 0;
	if(thePrefs.IsDynUpEnabled())
	{
        if (thePrefs.IsDynUpUseMillisecondPingTolerance())
            ussShift = 45;
        else
            ussShift = 90;
	}

	//MODIFIED by fengwen on %DATE% <begin>	状态栏不显示用户数
	//int aiWidths[5] =
	//{ 
	//	rect.right - 675 - ussShift,
	//	rect.right - 440 - ussShift,
	//	rect.right - 250 - ussShift,
	//	rect.right -  25 - ussShift,
	//	-1
	//};
	int aiWidths[4] =
	{ 
		//rect.right - 675 - ussShift,
		rect.right - 340 - ussShift,
		rect.right - 150 - ussShift,
		rect.right -  25 - ussShift,
		-1
	};
	//MODIFIED by fengwen on %DATE% <end>	状态栏不显示用户数
	statusbar->SetParts(ARRSIZE(aiWidths), aiWidths);
}

void CemuleDlg::OnSize(UINT nType, int cx, int cy)
{
	CTrayDialog::OnSize(nType, cx, cy);

//#ifdef EASY_UI
	CRect rcWnd;
	GetWindowRect(&rcWnd);
	Invalidate(FALSE);
	SetStatusBarPartsSize();
	transferwnd->VerifyCatTabSize();
//#endif
}

void CemuleDlg::ProcessFileLink(LPCTSTR pszData, BOOL bTestUrl)
{
	try {
		CString link2;
		CString link;
		link2 = pszData;
		link2.Replace(_T("%7c"),_T("|"));
		link = OptUtf8ToStr(URLDecode(link2));
		LPCTSTR urls = link;
		CString aurl;

		CList<CString> list;
		::ConvertStrToStrList(&list, CString(urls));
		POSITION	pos = list.GetHeadPosition();
		while (NULL != pos)
		{
			aurl = list.GetNext(pos);

			if(TestUrl(aurl, bTestUrl))
			{
				if(_tcsnicmp(aurl, _T("http://"), 7) == 0 || _tcsnicmp(aurl, _T("ftp://"), 6) == 0)
				{
					CmdFuncs::AddUrlToDownload(aurl);
				}
				else
				{
					CED2KLink* pLink = CED2KLink::CreateLinkFromUrl(aurl);
					_ASSERT( pLink !=0 );
					switch (pLink->GetKind()) {
					case CED2KLink::kFile:
						{
							CED2KFileLink* pFileLink = pLink->GetFileLink();
							_ASSERT(pFileLink !=0);
							//MODIFIED by fengwen on 2007/02/08	<begin> :
							//CGlobalVariable::downloadqueue->AddFileLinkToDownload(pFileLink,searchwnd->GetSelectedCat());
							//CmdFuncs::AddFileLinkToDownload(pFileLink, searchwnd->GetSelectedCat());
							CmdFuncs::AddEd2kLinksToDownload(aurl, searchwnd->GetSelectedCat());
							//MODIFIED by fengwen on 2007/02/08	<end> :
						}
						break;
					case CED2KLink::kServerList:
						{
							CED2KServerListLink* pListLink = pLink->GetServerListLink(); 
							_ASSERT( pListLink !=0 ); 
							CString strAddress = pListLink->GetAddress(); 
							if(strAddress.GetLength() != 0)
								serverwnd->UpdateServerMetFromURL(strAddress);
						}
						break;
					case CED2KLink::kServer:
						{
							CString defName;
							CED2KServerLink* pSrvLink = pLink->GetServerLink();
							_ASSERT( pSrvLink !=0 );
							CServer* pSrv = new CServer(pSrvLink->GetPort(), pSrvLink->GetAddress());
							_ASSERT( pSrv !=0 );
							pSrvLink->GetDefaultName(defName);
							pSrv->SetListName(defName);

							// Barry - Default all new servers to high priority
							if (thePrefs.GetManualAddedServersHighPriority())
								pSrv->SetPreference(SRV_PR_HIGH);

							if (!serverwnd->serverlistctrl.AddServer(pSrv,true)) 
								delete pSrv; 
							else
								AddLogLine(true,GetResString(IDS_SERVERADDED), pSrv->GetListName());
						}
						break;
					default:
						break;
					}
					delete pLink;
				}
			}
		}

	}
	catch(CString strError){
		LogWarning(LOG_STATUSBAR, GetResString(IDS_LINKNOTADDED) + _T(" - ") + strError);
	}
	catch(...){
		LogWarning(LOG_STATUSBAR, GetResString(IDS_LINKNOTADDED));
	}
}

LRESULT CemuleDlg::OnWMData(WPARAM /*wParam*/, LPARAM lParam)
{
	if (!IsRunning())
		return false;

	PCOPYDATASTRUCT data = (PCOPYDATASTRUCT)lParam;
	if(data == NULL)
		return false;
	if (data->dwData == OP_ED2KLINK)
	{
		if (thePrefs.IsBringToFront() && !thePrefs.m_bShowNewTaskDlg)
		{
			if (IsIconic())
				RestoreWindow();
			else if (!IsWindowVisible())
				ShowWindow(SW_SHOW);
			ForceBringWindowToTop(GetSafeHwnd());
		}
		ProcessFileLink((LPCTSTR)data->lpData);
	}
	else if(data->dwData == OP_COLLECTION){
		FlashWindow(TRUE);
		if (IsIconic())
			ShowWindow(SW_SHOWNORMAL);
		//else if (TrayHide())
		else if (IsWindowHide(GetSafeHwnd()))
			RestoreWindow();
		else
			SetForegroundWindow();

		CCollection* pCollection = new CCollection();
		CString strPath = CString((LPCTSTR)data->lpData);
		if (pCollection->InitCollectionFromFile(strPath, strPath.Right((strPath.GetLength()-1)-strPath.ReverseFind('\\')))){
			CCollectionViewDialog dialog;
			dialog.SetCollection(pCollection);
			dialog.DoModal();
		}
		delete pCollection;
	}
	else if (data->dwData == OP_CLCOMMAND){
		// command line command received
		CString clcommand((LPCTSTR)data->lpData);
		clcommand.MakeLower();
		AddLogLine(true,_T("CLI: %s"),clcommand);

		if (clcommand==_T("connect")) {StartConnection(); return true;}
		if (clcommand==_T("disconnect")) {CGlobalVariable::serverconnect->Disconnect(); return true;}
		if (clcommand==_T("resume")) {CGlobalVariable::downloadqueue->StartNextFile(); return true;}
		if (clcommand==_T("exit")) {OnClose(); return true;}
		if (clcommand==_T("restore")) {RestoreWindow();return true;}
		if (clcommand==_T("reloadipf")) {CGlobalVariable::ipfilter->LoadFromDefaultFile(); return true;}
		if (clcommand.Left(7).MakeLower()==_T("limits=") && clcommand.GetLength()>8) {
			CString down;
			CString up=clcommand.Mid(7);
			int pos=up.Find(_T(','));
			if (pos>0) {
				down=up.Mid(pos+1);
				up=up.Left(pos);
			}
			if (down.GetLength()>0) thePrefs.SetMaxDownload(_tstoi(down));
			if (up.GetLength()>0) thePrefs.SetMaxUpload(_tstoi(up));

			return true;
		}

		if (clcommand==_T("help") || clcommand==_T("/?")) {
			// show usage
			return true;
		}

		if (clcommand==_T("status")) {
			CString strBuff;
			strBuff.Format(_T("%sstatus.log"), thePrefs.GetMuleDirectory(EMULE_CONFIGBASEDIR));
			FILE* file = _tfsopen(strBuff, _T("wt"), _SH_DENYWR);
			if (file){
				if (CGlobalVariable::serverconnect->IsConnected())
					strBuff = GetResString(IDS_CONNECTED);
				else if (CGlobalVariable::serverconnect->IsConnecting())
					strBuff = GetResString(IDS_CONNECTING);
				else
					strBuff = GetResString(IDS_DISCONNECTED);
				_ftprintf(file, _T("%s\n"), strBuff);

				strBuff.Format(GetResString(IDS_UPDOWNSMALL), (float)CGlobalVariable::uploadqueue->GetDatarate()/1024, (float)CGlobalVariable::downloadqueue->GetDatarate()/1024);
				_ftprintf(file, _T("%s"), strBuff); // next string (getTextList) is already prefixed with '\n'!
				_ftprintf(file, _T("%s\n"), transferwnd->downloadlistctrl.getTextList());
				
				fclose(file);
			}
			return true;
		}
		// show "unknown command";
	}
	else if(data->dwData == OP_QUERYSTATUS)
	{
		return CDlgAddTask::GetAddState();
	}
	else if (data->dwData == OP_COMEOUT)
	{
		if (!m_boss)
		{
			TrayShow();
			ShowWindow(SW_SHOW);
			m_boss = !m_boss;
		}
		else
		{
			BringWindowToSight(GetSafeHwnd());
		}
		ForceBringWindowToTop(GetSafeHwnd());
	}
	return true;
}

LRESULT CemuleDlg::OnFileHashed(WPARAM wParam, LPARAM lParam)
{
	if (CGlobalVariable::m_app_state == APP_STATE_SHUTTINGDOWN)
		return FALSE;

	CKnownFile* result = (CKnownFile*)lParam;
	ASSERT( result->IsKindOf(RUNTIME_CLASS(CKnownFile)) );

	if (wParam)
	{
		// File hashing finished for a part file when:
		// - part file just completed
		// - part file was rehashed at startup because the file date of part.met did not match the part file date

		CPartFile* requester = (CPartFile*)wParam;
		ASSERT( requester->IsKindOf(RUNTIME_CLASS(CPartFile)) );

		// SLUGFILLER: SafeHash - could have been canceled
		if (CGlobalVariable::downloadqueue->IsPartFile(requester))
			requester->PartFileHashFinished(result);
		else
			delete result;
		// SLUGFILLER: SafeHash
	}
	else
	{
		ASSERT( !result->IsKindOf(RUNTIME_CLASS(CPartFile)) );

		// File hashing finished for a shared file (none partfile)
		//	- reading shared directories at startup and hashing files which were not found in known.met
		//	- reading shared directories during runtime (user hit Reload button, added a shared directory, ...)
		CKnownFile* pFileInKnownList=NULL;
		CString sFilePath = result->GetFilePath();
		//[VC-Huby-080326]: from hashing changed to shared or unshared
		if( CGlobalVariable::sharedfiles->FileHashingFinished(result,&pFileInKnownList) )
		{
			sharedfileswnd->sharedfilesctrl.RedrawCheckBox(1,result);
			if(pFileInKnownList && !sharedfileswnd->sharedfilesctrl.m_UnSharedFileList.Find(pFileInKnownList) )
				sharedfileswnd->sharedfilesctrl.m_UnSharedFileList.AddTail(pFileInKnownList);
		}
		else
		{
			sharedfileswnd->sharedfilesctrl.RedrawCheckBox(sFilePath);
		}
	}

	return TRUE;
}

LRESULT CemuleDlg::OnFileOpProgress(WPARAM wParam, LPARAM lParam)
{
	if (CGlobalVariable::m_app_state == APP_STATE_SHUTTINGDOWN)
		return FALSE;

	CKnownFile* pKnownFile = (CKnownFile*)lParam;
	ASSERT( pKnownFile->IsKindOf(RUNTIME_CLASS(CKnownFile)) );

	if (pKnownFile->IsKindOf(RUNTIME_CLASS(CPartFile)))
	{
		CPartFile* pPartFile = static_cast<CPartFile*>(pKnownFile);
		pPartFile->SetFileOpProgress(wParam);
		pPartFile->UpdateDisplayedInfo(true);
	}

	return 0;
}

// SLUGFILLER: SafeHash
LRESULT CemuleDlg::OnHashFailed(WPARAM /*wParam*/, LPARAM lParam)
{
	CGlobalVariable::sharedfiles->HashFailed((UnknownFile_Struct*)lParam);
	return 0;
}
// SLUGFILLER: SafeHash

LRESULT CemuleDlg::OnFileAllocExc(WPARAM wParam,LPARAM lParam)
{
	if (lParam == 0)
		((CPartFile*)wParam)->FlushBuffersExceptionHandler();
	else
		((CPartFile*)wParam)->FlushBuffersExceptionHandler((CFileException*)lParam);
	return 0;
}

LRESULT CemuleDlg::OnFileCompleted(WPARAM wParam, LPARAM lParam)
{
	if( 0==lParam)
		return 0;

	CPartFile* partfile = (CPartFile*)lParam;
	ASSERT( partfile != NULL );
	//Changed by thilon on 2008.1.16
	//修改执行顺序
	bool bDeleted = false;
	partfile->PerformFileCompleteEnd(wParam,bDeleted);
	if(bDeleted)
		return 0;

	CUpdateInfo updateinfo;
	CString hash = md4str(partfile->GetFileHash());

	if(updateinfo.isUpdateFile(hash))
	{
		//
		if (thePrefs.m_bForceUpdate)
		{
			if( MessageBox(GetResString(IDS_UPDATE_INSTALL), GetResString(IDS_CAPTION), MB_YESNO | MB_ICONQUESTION) != IDYES)
				return 0;
		}
		//
		CString update;
		update.Format(_T("%s\\%s"), thePrefs.GetMuleDirectory(EMULE_UPDATEDIR), partfile->GetFileName());

		TCHAR szFileName[MAX_PATH] = {0};
		::GetModuleFileName(NULL,szFileName,MAX_PATH);
		CString tcsFileName = szFileName;
		int nPos = tcsFileName.ReverseFind('\\');
		CString tcsAppDir = _T("");
		if (nPos != -1)
		{
			tcsFileName = tcsFileName.Left(nPos);
			//tcsAppDir.Format(_T("/EMULEPATH=\"%s\""),tcsFileName);
			tcsAppDir.Format(_T("/EMULEPATH=\"%s\" /FORCEUPDATE=%d"),tcsFileName,thePrefs.m_bForceUpdate);
		}
		ShellExecute(NULL , _T("open"), update, tcsAppDir, NULL, SW_SHOWNORMAL);
	}

	//Added by thilon on 2007.12.26 for Anti-Virus <begin>:
	CString strScanFormat = thePrefs.m_strScanFormat;
	strScanFormat.MakeUpper();

	int result = -1;

	TCHAR *tmp = (TCHAR *)_tcsrchr(partfile->GetFileName(), _T('.'));

	TCHAR *ext = _wcsupr(tmp);

	if (ext != NULL && !strScanFormat.IsEmpty())
	{
		result = strScanFormat.Find(ext);
	}

	_wcslwr(ext);

	if (strScanFormat.IsEmpty())
	{
		result = 0;
	}

	if (thePrefs.IsEnableScanVirus() && result != -1)
	{
		if (thePrefs.m_AntiVirusModel && !thePrefs.m_strAntiVirusPath.IsEmpty())
		{//other program
			CString FilePath = partfile->GetFilePath();
			FilePath = _T("\"") + FilePath + _T("\"");
			ShellExecute(NULL, _T("open"), thePrefs.m_strAntiVirusPath, FilePath, NULL, SW_SHOWNORMAL);
		}
		else
		{//model
			HMODULE hDll = GetSecurityDLL();

			if (hDll && !thePrefs.m_AntiVirusModel)
			{
				typedef HRESULT (*PFNCreate)(CString);

				PFNCreate pfnCreate = (PFNCreate)GetProcAddress(hDll, "GetScanEngine");

				pfnCreate(partfile->GetFilePath());
			}
		}
	}

	//Added by thilon on 2007.12.26 for Anti-Virus <end>:

	//ADDED by fengwen on 2007/02/09	<begin> :
	::SendMessage(theWndMgr.GetWndHandle(CWndMgr::WI_DOWNLOADED_LISTCTRL), UM_DLED_LC_ADDFILE, 0, (LPARAM) partfile);

	NMLISTVIEW nmlv;
	memset (&nmlv, 0, sizeof (nmlv));
	nmlv.hdr.hwndFrom = theWndMgr.GetWndHandle(CWndMgr::WI_DOWNLOADED_LISTCTRL);
	nmlv.hdr.idFrom = IDC_DOWNLOADLIST;
	nmlv.hdr.code = LVN_ITEMCHANGED;
	nmlv.iItem = -1;
	nmlv.uNewState = LVIS_SELECTED | LVIS_FOCUSED;

	SendMessage(WM_NOTIFY, IDC_DOWNLOADLIST, (LONG)&nmlv) ;
	//ADDED by fengwen on 2007/02/09	<end> :

	return 0;
}

LRESULT CemuleDlg::OnFileAddCompleted(WPARAM /*wParam*/,LPARAM lParam)
{
	if (0 != lParam)
		::SendMessage(theWndMgr.GetWndHandle(CWndMgr::WI_DOWNLOADED_LISTCTRL), UM_DLED_LC_ADDFILE, 0, lParam);

	return 0;
}

#ifdef _DEBUG
void BeBusy(UINT uSeconds, LPCSTR pszCaller)
{
	UINT s = 0;
	while (uSeconds--) {
		theVerboseLog.Logf(_T("%hs: called=%hs, waited %u sec."), __FUNCTION__, pszCaller, s++);
		Sleep(1000);
	}
}
#endif

BOOL CemuleDlg::OnQueryEndSession()
{
	AddDebugLogLine(DLP_VERYLOW, _T("%hs"), __FUNCTION__);
	if (!CTrayDialog::OnQueryEndSession())
		return FALSE;

	AddDebugLogLine(DLP_VERYLOW, _T("%hs: returning TRUE"), __FUNCTION__);
	return TRUE;
}

void CemuleDlg::OnEndSession(BOOL bEnding)
{
	AddDebugLogLine(DLP_VERYLOW, _T("%hs: bEnding=%d"), __FUNCTION__, bEnding);
	if (bEnding && CGlobalVariable::m_app_state == APP_STATE_RUNNING)
	{
		// If eMule was *not* started with "RUNAS":
		// When user is logging of (or reboots or shutdown system), Windows sends the
		// WM_QUERYENDSESSION/WM_ENDSESSION to all top level windows.
		// Here we can consume as much time as we need to perform our shutdown. Even if we
		// take longer than 20 seconds, Windows will just show a dialog box that 'emule'
		// is not terminating in time and gives the user a chance to cancel that. If the user
		// does not cancel the Windows dialog, Windows will though wait until eMule has 
		// terminated by itself - no data loss, no file corruption, everything is fine.
		CGlobalVariable::m_app_state = APP_STATE_SHUTTINGDOWN;
		OnClose();
	}

	CTrayDialog::OnEndSession(bEnding);
	AddDebugLogLine(DLP_VERYLOW, _T("%hs: returning"), __FUNCTION__);
}

LRESULT CemuleDlg::OnUserChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	AddDebugLogLine(DLP_VERYLOW, _T("%hs"), __FUNCTION__);
	// Just want to know if we ever get this message. Maybe it helps us to handle the
	// logoff/reboot/shutdown problem when eMule was started with "RUNAS".
	return Default();
}

LRESULT CemuleDlg::OnConsoleThreadEvent(WPARAM wParam, LPARAM lParam)
{
	AddDebugLogLine(DLP_VERYLOW, _T("%hs: nEvent=%u, nThreadID=%u"), __FUNCTION__, wParam, lParam);

	// If eMule was started with "RUNAS":
	// This message handler receives a 'console event' from the concurrently and thus
	// asynchronously running console control handler thread which was spawned by Windows
	// in case the user logs off/reboots/shutdown. Even if the console control handler thread
	// is waiting on the result from this message handler (is waiting until the main thread
	// has finished processing this inter-application message), the application will get
	// forcefully terminated by Windows after 20 seconds! There is no known way to prevent
	// that. This means, that if we would invoke our standard shutdown code ('OnClose') here
	// and the shutdown takes longer than 20 sec, we will get forcefully terminated by 
	// Windows, regardless of what we are doing. This means, MET-file and PART-file corruption
	// may occure. Because the shutdown code in 'OnClose' does also shutdown Kad (which takes
	// a noticeable amount of time) it is not that unlikely that we run into problems with
	// not being finished with our shutdown in 20 seconds.
	// 
	if (CGlobalVariable::m_app_state == APP_STATE_RUNNING)
	{
#if 1
		// And it really should be OK to expect that emule can shutdown in 20 sec on almost
		// all computers. So, use the proper shutdown.
		CGlobalVariable::m_app_state = APP_STATE_SHUTTINGDOWN;
		OnClose();	// do not invoke if shutdown takes longer than 20 sec, read above
#else
		// As a minimum action we at least set the 'shutting down' flag, this will help e.g.
		// the CUploadQueue::UploadTimer to not start any file save actions which could get
		// interrupted by windows and which would then lead to corrupted MET-files.
		// Setting this flag also helps any possible running threads to stop their work.
		CGlobalVariable::m_app_state = APP_STATE_SHUTTINGDOWN;

#ifdef _DEBUG
		// Simulate some work.
		//
		// NOTE: If the console thread has already exited, Windows may terminate the process
		// even before the 20 sec. timeout!
		//BeBusy(70, __FUNCTION__);
#endif

		// Actually, just calling 'ExitProcess' should be the most safe thing which we can
		// do here. Because we received this message via the main message queue we are 
		// totally in-sync with the application and therefore we know that we are currently
		// not within a file save action and thus we simply can not cause any file corruption
		// when we exit right now.
		//
		// Of course, there may be some data loss. But it's the same amount of data loss which
		// could occure if we keep running. But if we keep running and wait until Windows
		// terminates us after 20 sec, there is also the chance for file corruption.
		if (thePrefs.GetDebug2Disk()) {
			theVerboseLog.Logf(_T("%hs: ExitProcess"), __FUNCTION__);
			theVerboseLog.Close();
		}
		ExitProcess(0);
#endif
	}

	AddDebugLogLine(DLP_VERYLOW, _T("%hs: returning"), __FUNCTION__);
	return 1;
}

void CemuleDlg::OnDestroy()
{
	CGlobalVariable::m_hListenWnd = NULL;
	AddDebugLogLine(DLP_VERYLOW, _T("%hs"), __FUNCTION__);

	CPlayerMgr::StopAllPlayer();

	TriggerConnectingTray(FALSE);

	// If eMule was started with "RUNAS":
	// When user is logging of (or reboots or shutdown system), Windows may or may not send 
	// a WM_DESTROY (depends on how long the application needed to process the 
	// CTRL_LOGOFF_EVENT). But, regardless of what happened and regardless of how long any
	// application specific shutdown took, Windows fill forcefully terminate the process 
	// after 1-2 seconds after WM_DESTROY! So, we can not use WM_DESTROY for any lengthy
	// shutdown actions in that case.
	CTrayDialog::OnDestroy();
}

bool CemuleDlg::CanClose()
{
	if (CGlobalVariable::m_app_state == APP_STATE_RUNNING && thePrefs.IsConfirmExitEnabled())
	{
		if (AfxMessageBox(GetResString(IDS_MAIN_EXIT), MB_YESNO | MB_DEFBUTTON2) == IDNO)
			return false;
	}
	return true;
}

void CemuleDlg::OnClose()
{
	// VC-yunchenn.chen[2007-07-18]: user doesn't decide how to close then.
	KillTimer(TIMER_ID);
	if(CCloseModeDlg::m_bAskingClose) return;

	CloseImmeditely();

}

void CemuleDlg::DestroyMiniMule()
{
	if (m_pMiniMule)
	{
		if (!m_pMiniMule->IsInCallback()) // for safety
		{
			TRACE("%s - m_pMiniMule->DestroyWindow();\n", __FUNCTION__);
			m_pMiniMule->DestroyWindow();
			ASSERT( m_pMiniMule == NULL );
			m_pMiniMule = NULL;
		}
		//else//注释掉这里 发生异常过 原因不明 -GGSoSo
		//	ASSERT(0);
	}
}


// VC-SearchDream[2007-05-18]: for Pop Mule Begin
void CemuleDlg::DestroyPopMule()
{
	if (m_pPopMule)
	{
		TRACE("%s - m_pPopMule->DestroyWindow();\n", __FUNCTION__);
		m_pPopMule->DestroyWindow();
		ASSERT( m_pPopMule == NULL );
		m_pPopMule = NULL;
	}
}

void CemuleDlg::ShowPopMule(CPartFile* pPartFile)
{
	if(!IsRunning())
	{
		return;
	}

	if (m_pPopMule) 
	{
		TRACE("%s - m_pPopMule->ShowWindow(SW_SHOW);\n", __FUNCTION__);
		m_pPopMule->ShowWindow(SW_SHOW);
		m_pPopMule->SetPartFile(pPartFile);
		m_pPopMule->SetForegroundWindow();
		m_pPopMule->BringWindowToTop();
		return;
	}

	try
	{
		TRACE("%s - m_pPopMule = new CPopMule(this);\n", __FUNCTION__);
		ASSERT( m_pPopMule == NULL );
		m_pPopMule = new CPopMule(this);
		m_pPopMule->Create(CPopMule::IDD, this);
		m_pPopMule->SetPartFile(pPartFile);
		m_pPopMule->SetForegroundWindow();
		m_pPopMule->BringWindowToTop();
	}
	catch(...)
	{
		ASSERT(0);
		m_pPopMule = NULL;
	}
}
// VC-SearchDream[2007-05-18]: for Pop Mule End

LRESULT CemuleDlg::OnCloseMiniMule(WPARAM wParam, LPARAM /*lParam*/)
{
	TRACE("%s -> DestroyMiniMule();\n", __FUNCTION__);
	DestroyMiniMule();
	if (wParam)
		RestoreWindow();
	return 0;
}

void CemuleDlg::OnTrayLButtonUp(CPoint /*pt*/)
{
	if(!IsRunning())
		return;

	// Avoid reentrancy problems with main window, options dialog and mini mule window
	if (IsPreferencesDlgOpen()) {
		MessageBeep(MB_OK);
		preferenceswnd->SetForegroundWindow();
		preferenceswnd->BringWindowToTop();
		return;
	}

	if (m_pMiniMule) {
		TRACE("%s - m_pMiniMule->ShowWindow(SW_SHOW);\n", __FUNCTION__);
		m_pMiniMule->ShowWindow(SW_SHOW);
		m_pMiniMule->SetForegroundWindow();
		m_pMiniMule->BringWindowToTop();
		return;
	}

	if (thePrefs.GetEnableMiniMule())
	{
		try
		{
			TRACE("%s - m_pMiniMule = new CMiniMule(this);\n", __FUNCTION__);
			ASSERT( m_pMiniMule == NULL );
			m_pMiniMule = new CMiniMule(this);
			m_pMiniMule->Create(CMiniMule::IDD, this);
			//m_pMiniMule->ShowWindow(SW_SHOW);	// do not explicitly show the window, it will do that for itself when it's ready..
			m_pMiniMule->SetForegroundWindow();
			m_pMiniMule->BringWindowToTop();
		}
		catch(...)
		{
			ASSERT(0);
			m_pMiniMule = NULL;
		}
	}
}

void CemuleDlg::OnTrayRButtonUp(CPoint pt)
{
	if (!IsRunning())
		return;

	// Avoid reentrancy problems with main window, options dialog and mini mule window
	if (IsPreferencesDlgOpen()) {
		MessageBeep(MB_OK);
		preferenceswnd->SetForegroundWindow();
		preferenceswnd->BringWindowToTop();
		return;
	}

	if (m_pMiniMule)
	{
		if (m_pMiniMule->GetAutoClose())
		{
			TRACE("%s - m_pMiniMule->GetAutoClose() -> DestroyMiniMule();\n", __FUNCTION__);
			DestroyMiniMule();
		}
		else
		{
			// Avoid reentrancy problems with main window, options dialog and mini mule window
			if (m_pMiniMule->m_hWnd && !m_pMiniMule->IsWindowEnabled()) {
				MessageBeep(MB_OK);
				return;
			}
		}
	}

	if (m_pSystrayDlg) {
		m_pSystrayDlg->BringWindowToTop();
		return;
	}

	m_pSystrayDlg = new CMuleSystrayDlg(this, pt,
										thePrefs.GetMaxGraphUploadRate(true), thePrefs.GetMaxGraphDownloadRate(),
										thePrefs.GetMaxUpload(), thePrefs.GetMaxDownload());
	if (m_pSystrayDlg)
	{
		UINT nResult = m_pSystrayDlg->DoModal();
		delete m_pSystrayDlg;
		m_pSystrayDlg = NULL;
		switch (nResult)
		{
			case IDC_TOMAX:
				QuickSpeedOther(MP_QS_UA);
				break;
			case IDC_TOMIN:
				QuickSpeedOther(MP_QS_PA);
				break;
			case IDC_RESTORE:
				{
					if(IsWindowVisible())
					{
						if(IsIconic())
						{
							ShowWindow(SW_RESTORE);
						}
						else
						{
							ShowWindow(SW_MINIMIZE);
						}
					}
					else
					{
						RestoreWindow();
					}
				}
				//RestoreWindow();
				break;
			case IDC_CONNECT:
				StartConnection();
				break;
			case IDC_DISCONNECT:
				CloseConnection();
				break;
			case IDC_EXIT:
				OnClose();
				break;
			case IDC_PREFERENCES:
				ShowPreferences();
				break;
			case IDC_UPDATE:
				DoVersioncheck(true);
				break;
		}
	}
}

void CemuleDlg::AddSpeedSelectorMenus(CMenu* addToMenu)
{
	CString text;

	// Create UploadPopup Menu
	ASSERT( m_menuUploadCtrl.m_hMenu == NULL );
	if (m_menuUploadCtrl.CreateMenu())
	{
		text.Format(_T("20%%\t%i %s"),  (uint16)(thePrefs.GetMaxGraphUploadRate(true)*0.2),GetResString(IDS_KBYTESPERSEC));	m_menuUploadCtrl.AppendMenu(MF_STRING, MP_QS_U20,  text);
		text.Format(_T("40%%\t%i %s"),  (uint16)(thePrefs.GetMaxGraphUploadRate(true)*0.4),GetResString(IDS_KBYTESPERSEC));	m_menuUploadCtrl.AppendMenu(MF_STRING, MP_QS_U40,  text);
		text.Format(_T("60%%\t%i %s"),  (uint16)(thePrefs.GetMaxGraphUploadRate(true)*0.6),GetResString(IDS_KBYTESPERSEC));	m_menuUploadCtrl.AppendMenu(MF_STRING, MP_QS_U60,  text);
		text.Format(_T("80%%\t%i %s"),  (uint16)(thePrefs.GetMaxGraphUploadRate(true)*0.8),GetResString(IDS_KBYTESPERSEC));	m_menuUploadCtrl.AppendMenu(MF_STRING, MP_QS_U80,  text);
		text.Format(_T("100%%\t%i %s"), (uint16)(thePrefs.GetMaxGraphUploadRate(true)),GetResString(IDS_KBYTESPERSEC));		m_menuUploadCtrl.AppendMenu(MF_STRING, MP_QS_U100, text);
		m_menuUploadCtrl.AppendMenu(MF_SEPARATOR);
		
		if (GetRecMaxUpload() > 0) {
			text.Format(GetResString(IDS_PW_MINREC) + GetResString(IDS_KBYTESPERSEC), GetRecMaxUpload());
			m_menuUploadCtrl.AppendMenu(MF_STRING, MP_QS_UP10, text);
		}

		text.Format(_T("%s:"), GetResString(IDS_PW_UPL));
		addToMenu->AppendMenu(MF_STRING|MF_POPUP, (UINT_PTR)m_menuUploadCtrl.m_hMenu, text);
	}

	// Create DownloadPopup Menu
	ASSERT( m_menuDownloadCtrl.m_hMenu == NULL );
	if (m_menuDownloadCtrl.CreateMenu())
	{
		text.Format(_T("20%%\t%i %s"),  (uint16)(thePrefs.GetMaxGraphDownloadRate()*0.2),GetResString(IDS_KBYTESPERSEC));	m_menuDownloadCtrl.AppendMenu(MF_STRING|MF_POPUP, MP_QS_D20,  text);
		text.Format(_T("40%%\t%i %s"),  (uint16)(thePrefs.GetMaxGraphDownloadRate()*0.4),GetResString(IDS_KBYTESPERSEC));	m_menuDownloadCtrl.AppendMenu(MF_STRING|MF_POPUP, MP_QS_D40,  text);
		text.Format(_T("60%%\t%i %s"),  (uint16)(thePrefs.GetMaxGraphDownloadRate()*0.6),GetResString(IDS_KBYTESPERSEC));	m_menuDownloadCtrl.AppendMenu(MF_STRING|MF_POPUP, MP_QS_D60,  text);
		text.Format(_T("80%%\t%i %s"),  (uint16)(thePrefs.GetMaxGraphDownloadRate()*0.8),GetResString(IDS_KBYTESPERSEC));	m_menuDownloadCtrl.AppendMenu(MF_STRING|MF_POPUP, MP_QS_D80,  text);
		text.Format(_T("100%%\t%i %s"), (uint16)(thePrefs.GetMaxGraphDownloadRate()),GetResString(IDS_KBYTESPERSEC));		m_menuDownloadCtrl.AppendMenu(MF_STRING|MF_POPUP, MP_QS_D100, text);

		text.Format(_T("%s:"), GetResString(IDS_PW_DOWNL));
		addToMenu->AppendMenu(MF_STRING|MF_POPUP, (UINT_PTR)m_menuDownloadCtrl.m_hMenu, text);
	}

	addToMenu->AppendMenu(MF_SEPARATOR);
	addToMenu->AppendMenu(MF_STRING, MP_CONNECT, GetResString(IDS_MAIN_BTN_CONNECT));
	addToMenu->AppendMenu(MF_STRING, MP_DISCONNECT, GetResString(IDS_MAIN_BTN_DISCONNECT)); 
}

void CemuleDlg::StartConnection()
{
	if (   (!CGlobalVariable::serverconnect->IsConnecting() && !CGlobalVariable::serverconnect->IsConnected())
		|| !Kademlia::CKademlia::IsRunning())
	{
		AddLogLine(true, GetResString(IDS_CONNECTING));

		// ed2k
		// VC-kernel[2007-03-02]:
		//if (thePrefs.GetNetworkED2K() && !CGlobalVariable::serverconnect->IsConnecting() && !CGlobalVariable::serverconnect->IsConnected()) 
		if ( !CGlobalVariable::serverconnect->IsConnecting() && !CGlobalVariable::serverconnect->IsConnected()) 
		{
			CGlobalVariable::serverconnect->ConnectToAnyServer();
		}

		// kad
		// VC-kernel[2007-03-02]:
		//if (thePrefs.GetNetworkKademlia() && !Kademlia::CKademlia::IsRunning()) 
		if (!Kademlia::CKademlia::IsRunning())
		{
			Kademlia::CKademlia::Start();
		}

		ShowConnectionState();
	}
}

void CemuleDlg::CloseConnection()
{
	if (CGlobalVariable::serverconnect->IsConnected()){
		CGlobalVariable::serverconnect->Disconnect();
	}

	if (CGlobalVariable::serverconnect->IsConnecting()){
		CGlobalVariable::serverconnect->StopConnectionTry();
	}
	Kademlia::CKademlia::Stop();
	theApp.OnlineSig(); // Added By Bouc7 
	ShowConnectionState();
}

void CemuleDlg::RestoreWindow()
{
	if (IsPreferencesDlgOpen()) 
	{
		MessageBeep(MB_OK);
		preferenceswnd->SetForegroundWindow();
		preferenceswnd->BringWindowToTop();
		return;
	}
	//COMMENTED by fengwen on 2007/06/21 <begin>	:	永远不隐藏TrayIcon
	//if (TrayIsVisible())
		//TrayHide();
	//COMMENTED by fengwen on 2007/06/21 <end>		:	永远不隐藏TrayIcon
	DestroyMiniMule();
	

	if (m_wpFirstRestore.length)
	{
		SetWindowPlacement(&m_wpFirstRestore);
		memset(&m_wpFirstRestore, 0, sizeof m_wpFirstRestore);
		SetForegroundWindow();
		BringWindowToTop();
	}
	else
	{
		if(IsIconic())
		{
			ShowWindow(SW_RESTORE);
		}
		CTrayDialog::RestoreWindow();
		//Added by thilon on 2007.07.18, 修复窗口还原问题
		/*if(IsWindowVisible())
		{
			if(IsIconic())
			{
				ShowWindow(SW_RESTORE);
			}
			else
			{
				ShowWindow(SW_MINIMIZE);
			}
		}
		else
		{
			CTrayDialog::RestoreWindow();
		}*/
		
	}
}

void CemuleDlg::UpdateTrayIcon(int iPercent)
{
	const int iSpeedBarWidth = 1;

	// compute an id of the icon to be generated
	UINT uSysTrayIconCookie = (iPercent > 0) ? (16 - ((iPercent*15/100) + 1)) : 0;
	if (CGlobalVariable::IsConnected()) {
		if (!CGlobalVariable::IsFirewalled())
			uSysTrayIconCookie += 50;
	}
	else
		uSysTrayIconCookie += 100;
	
	// dont update if the same icon as displayed would be generated
	if ( NULL!=CGlobalVariable::serverconnect && !CGlobalVariable::serverconnect->IsConnecting())
		if (m_uLastSysTrayIconCookie == uSysTrayIconCookie)
			return;
	m_uLastSysTrayIconCookie = uSysTrayIconCookie;

	// prepare it up
	if (m_iMsgIcon!=0 && thePrefs.DoFlashOnNewMessage()==true ) {
		m_iMsgBlinkState=!m_iMsgBlinkState;

		if (m_iMsgBlinkState)
			m_TrayIcon.Init(imicons[1], 100, 1, 16-iSpeedBarWidth, 16, 16, thePrefs.GetStatsColor(11));
	} else m_iMsgBlinkState=false;

	if (!m_iMsgBlinkState) {
		if (CGlobalVariable::IsConnected()) {
			//if (CGlobalVariable::IsFirewalled())
			//	m_TrayIcon.Init(m_icoSysTrayLowID, 100, 1, 1, 16, 16, thePrefs.GetStatsColor(11));
			//else
				m_TrayIcon.Init(m_icoSysTrayConnected, 100, 1, 16-iSpeedBarWidth, 16, 16, thePrefs.GetStatsColor(11));
		}
		else if ( NULL!=CGlobalVariable::serverconnect && CGlobalVariable::serverconnect->IsConnecting())
		{
			if (m_bConnectingIconSide)
				m_TrayIcon.Init(m_icoSysTrayConnected, 100, 1, 16-iSpeedBarWidth, 16, 16, thePrefs.GetStatsColor(11));
			else
				m_TrayIcon.Init(m_icoSysTrayDisconnected, 100, 1, 16-iSpeedBarWidth, 16, 16, thePrefs.GetStatsColor(11));
			m_bConnectingIconSide = !m_bConnectingIconSide;

		}
		else
			m_TrayIcon.Init(m_icoSysTrayDisconnected, 100, 1, 16-iSpeedBarWidth, 16, 16, thePrefs.GetStatsColor(11));
	}

	// load our limit and color info
	static const int aiLimits[1] = { 100 }; // set the limits of where the bar color changes (low-high)
	COLORREF aColors[1] = { thePrefs.GetStatsColor(11) }; // set the corresponding color for each level
	m_TrayIcon.SetColorLevels(aiLimits, aColors, ARRSIZE(aiLimits));

	// generate the icon (do *not* destroy that icon using DestroyIcon(), that's done in 'TrayUpdate')
	int aiVals[1] = { iPercent };
	m_icoSysTrayCurrent = m_TrayIcon.Create(aiVals);
	ASSERT( m_icoSysTrayCurrent != NULL );
	if (m_icoSysTrayCurrent)
		TraySetIcon(m_icoSysTrayCurrent, true);
	TrayUpdate();
}

int CemuleDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	return CTrayDialog::OnCreate(lpCreateStruct);
}

void CemuleDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	if (IsRunning()){
		ShowTransferRate(true);
#if _ENABLE_NOUSE
		if (bShow == TRUE && activewnd == chatwnd)
			chatwnd->chatselector.ShowChat();
#endif
	}
	CTrayDialog::OnShowWindow(bShow, nStatus);
}

void CemuleDlg::ShowNotifier(LPCTSTR pszText, int iMsgType, LPCTSTR pszLink, bool bForceSoundOFF)
{
	if (!notifierenabled)
		return;

	LPCTSTR pszSoundEvent = NULL;
	int iSoundPrio = 0;
	bool bShowIt = false;
	switch (iMsgType)
	{
		case TBN_CHAT:
            if (thePrefs.GetNotifierOnChat())
			{
				m_wndTaskbarNotifier->Show(pszText, iMsgType, pszLink);
				bShowIt = true;
				pszSoundEvent = _T("eMule_Chat");
				iSoundPrio = 1;
			}
			break;
		case TBN_DOWNLOADFINISHED:
            if (thePrefs.GetNotifierOnDownloadFinished())
			{
				m_wndTaskbarNotifier->Show(pszText, iMsgType, pszLink);
				bShowIt = true;
				pszSoundEvent = _T("eMule_DownloadFinished");
				iSoundPrio = 1;
				//SendNotificationMail(iMsgType, pszText); //Deleted by thilon on 2006.11.07 
			}
			break;
		case TBN_DOWNLOADADDED:
            if (thePrefs.GetNotifierOnNewDownload())
			{
				m_wndTaskbarNotifier->Show(pszText, iMsgType, pszLink);
				bShowIt = true;
				pszSoundEvent = _T("eMule_DownloadAdded");
				iSoundPrio = 1;
			}
			break;
		case TBN_LOG:
            if (thePrefs.GetNotifierOnLog())
			{
				m_wndTaskbarNotifier->Show(pszText, iMsgType, pszLink);
				bShowIt = true;
				pszSoundEvent = _T("eMule_LogEntryAdded");
			}
			break;
		case TBN_IMPORTANTEVENT:
			//if (thePrefs.GetNotifierOnImportantError())
			//{
				m_wndTaskbarNotifier->Show(pszText, iMsgType, pszLink);
				bShowIt = true;
				pszSoundEvent = _T("eMule_Urgent");
				iSoundPrio = 1;
				//SendNotificationMail(iMsgType, pszText);		//Deleted by thilon on 2006.11.07
			//}
			break;

		case TBN_NEWVERSION:
			if (thePrefs.GetNotifierOnNewVersion())
			{
				m_wndTaskbarNotifier->Show(pszText, iMsgType, pszLink);
				bShowIt = true;
				pszSoundEvent = _T("eMule_NewVersion");
				iSoundPrio = 1;
			}
			break;
		case TBN_NULL:
            m_wndTaskbarNotifier->Show(pszText, iMsgType, pszLink);
			bShowIt = true;
			break;
	}
	
	if (bShowIt && !bForceSoundOFF && thePrefs.GetNotifierSoundType() != ntfstNoSound)
	{
		bool bNotifiedWithAudio = false;
		if (thePrefs.GetNotifierSoundType() == ntfstSpeech)
			bNotifiedWithAudio = Speak(pszText);

		if (!bNotifiedWithAudio)
		{
			if (!thePrefs.GetNotifierSoundFile().IsEmpty())
			{
				PlaySound(thePrefs.GetNotifierSoundFile(), NULL, SND_FILENAME | SND_NOSTOP | SND_NOWAIT | SND_ASYNC);
			}
			else if (pszSoundEvent)
			{
				// use 'SND_NOSTOP' only for low priority events, otherwise the 'Log message' event may overrule a more important
				// event which is fired nearly at the same time.
				PlaySound(pszSoundEvent, NULL, SND_APPLICATION | SND_ASYNC | SND_NODEFAULT | SND_NOWAIT | ((iSoundPrio > 0) ? 0 : SND_NOSTOP));
			}
		}
	}
}

void CemuleDlg::LoadNotifier(CString configuration)
{
	notifierenabled = m_wndTaskbarNotifier->LoadConfiguration(configuration)!=FALSE;
}

LRESULT CemuleDlg::OnTaskbarNotifierClicked(WPARAM /*wParam*/, LPARAM lParam)
{
	if (lParam)
	{
		LPTSTR pszLink = (LPTSTR)lParam;
		ShellOpenFile(pszLink, NULL);
		free(pszLink);
		pszLink = NULL;
	}

	switch (m_wndTaskbarNotifier->GetMessageType())
	{
#if _ENABLE_NOUSE
		case TBN_CHAT:
			RestoreWindow();
			SetActiveDialog(chatwnd);
			break;
#endif
		case TBN_DOWNLOADFINISHED:
			// if we had a link and opened the downloaded file, dont restore the app window
			//if (lParam==0)
			{
				if(IsWindowVisible())
				{
					if(IsIconic())
					{
						ShowWindow(SW_RESTORE);
					}
				}
				else
				{
					RestoreWindow();
				}
				//SetActiveDialog(transferwnd);
				m_mainTabWnd.SetActiveTabById(CMainTabWnd::TI_DOWNLOAD);
				m_mainTabWnd.m_dlgDownload.SetDownloadlistActiveTab(CDlgMaintabDownload::TI_DOWNLOADED);
			}
			break;

		case TBN_DOWNLOADADDED:
			{
				if(IsWindowVisible())
				{
					if(IsIconic())
					{
						ShowWindow(SW_RESTORE);
					}
				}
				else
				{
					RestoreWindow();
				}

				SetActiveDialog(transferwnd);
			}
			//RestoreWindow();
			//SetActiveDialog(transferwnd);
			break;

		case TBN_IMPORTANTEVENT:
			RestoreWindow();
			SetActiveDialog(serverwnd);	
			break;

		case TBN_LOG:
			RestoreWindow();
			SetActiveDialog(serverwnd);	
			break;

		case TBN_NEWVERSION:
		{
			//Chocobo Start
			//eMule自动更新，modified by Chocobo on 2006.07.31
			//屏蔽原版自动更新
			/*CString theUrl;
			theUrl.Format(_T("/en/version_check.php?version=%i&language=%i"),theApp.m_uCurVersionCheck,thePrefs.GetLanguageID());
			theUrl = thePrefs.GetVersionCheckBaseURL()+theUrl;
			ShellExecute(NULL, NULL, theUrl, NULL, thePrefs.GetMuleDirectory(EMULE_EXECUTEABLEDIR), SW_SHOWDEFAULT);*/
			break;
			//Chocobo End
		}
	}
    return 0;
}

void CemuleDlg::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	theApp.UpdateDesktopColorDepth();
	CTrayDialog::OnSettingChange(uFlags, lpszSection);
}

void CemuleDlg::OnSysColorChange()
{
	theApp.UpdateDesktopColorDepth();
	CTrayDialog::OnSysColorChange();
	SetAllIcons();
}

void CemuleDlg::SetAllIcons()
{
	// application icon (although it's not customizable, we may need to load a different color resolution)
	if (m_hIcon)
		VERIFY( ::DestroyIcon(m_hIcon) );
	// NOTE: the application icon name is prefixed with "AAA" to make sure it's alphabetically sorted by the
	// resource compiler as the 1st icon in the resource table!
	m_hIcon = AfxGetApp()->LoadIcon(_T("AAAEMULEAPP"));
	SetIcon(m_hIcon, TRUE);
	// this scales the 32x32 icon down to 16x16, does not look nice at least under WinXP
	//SetIcon(m_hIcon, FALSE);

	// connection state
	for (int i = 0; i < ARRSIZE(connicons); i++)
	{
		if (connicons[i]) VERIFY( ::DestroyIcon(connicons[i]) );
	}
	connicons[0] = theApp.LoadIcon(_T("ConnectedNotNot"), 16, 16);
	connicons[1] = theApp.LoadIcon(_T("ConnectedNotLow"), 16, 16);
	connicons[2] = theApp.LoadIcon(_T("ConnectedNotHigh"), 16, 16);
	connicons[3] = theApp.LoadIcon(_T("ConnectedLowNot"), 16, 16);
	connicons[4] = theApp.LoadIcon(_T("ConnectedLowLow"), 16, 16);
	connicons[5] = theApp.LoadIcon(_T("ConnectedLowHigh"), 16, 16);
	connicons[6] = theApp.LoadIcon(_T("ConnectedHighNot"), 16, 16);
	connicons[7] = theApp.LoadIcon(_T("ConnectedHighLow"), 16, 16);
	connicons[8] = theApp.LoadIcon(_T("ConnectedHighHigh"), 16, 16);
	ShowConnectionStateIcon();

	// transfer state
	if (transicons[0]) VERIFY( ::DestroyIcon(transicons[0]) );
	if (transicons[1]) VERIFY( ::DestroyIcon(transicons[1]) );
	if (transicons[2]) VERIFY( ::DestroyIcon(transicons[2]) );
	if (transicons[3]) VERIFY( ::DestroyIcon(transicons[3]) );
	transicons[0] = theApp.LoadIcon(_T("UP0DOWN0"), 16, 16);
	transicons[1] = theApp.LoadIcon(_T("UP0DOWN1"), 16, 16);
	transicons[2] = theApp.LoadIcon(_T("UP1DOWN0"), 16, 16);
	transicons[3] = theApp.LoadIcon(_T("UP1DOWN1"), 16, 16);
	ShowTransferStateIcon();

	// users state
	if (usericon) VERIFY( ::DestroyIcon(usericon) );
	usericon = theApp.LoadIcon(_T("StatsClients"), 16, 16);
	ShowUserStateIcon();

	// traybar icons
	if (m_icoSysTrayConnected) VERIFY( ::DestroyIcon(m_icoSysTrayConnected) );
	if (m_icoSysTrayDisconnected) VERIFY( ::DestroyIcon(m_icoSysTrayDisconnected) );
	if (m_icoSysTrayLowID) VERIFY( ::DestroyIcon(m_icoSysTrayLowID) );
	m_icoSysTrayConnected = theApp.LoadIcon(_T("TrayConnected"), 16, 16);
	m_icoSysTrayDisconnected = theApp.LoadIcon(_T("TrayNotConnected"), 16, 16);
	m_icoSysTrayLowID = theApp.LoadIcon(_T("TrayLowID"), 16, 16);
	ShowTransferRate(true);

	if (imicons[0]) VERIFY( ::DestroyIcon(imicons[0]) );
	if (imicons[1]) VERIFY( ::DestroyIcon(imicons[1]) );
	if (imicons[2]) VERIFY( ::DestroyIcon(imicons[2]) );
	imicons[0] = NULL;
	imicons[1] = theApp.LoadIcon(_T("Message"), 16, 16);
	imicons[2] = theApp.LoadIcon(_T("MessagePending"), 16, 16);
	ShowMessageState(m_iMsgIcon);
}

void CemuleDlg::Localize()
{
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu)
	{
		VERIFY( pSysMenu->ModifyMenu(MP_ABOUTBOX, MF_BYCOMMAND | MF_STRING, MP_ABOUTBOX, GetResString(IDS_ABOUTBOX)) );
		VERIFY( pSysMenu->ModifyMenu(MP_VERSIONCHECK, MF_BYCOMMAND | MF_STRING, MP_VERSIONCHECK, GetResString(IDS_VERSIONCHECK)) );

		switch (thePrefs.GetWindowsVersion())
		{
			case _WINVER_98_:
			case _WINVER_95_:
			case _WINVER_ME_:
				// NOTE: I think the reason why the old version of the following code crashed under Win9X was because
				// of the menus were destroyed right after they were added to the system menu. New code should work
				// under Win9X too but I can't test it.
				break;
			default:{
				// localize the 'speed control' sub menus by deleting the current menus and creating a new ones.

				// remove any already available 'speed control' menus from system menu
				UINT uOptMenuPos = pSysMenu->GetMenuItemCount() - 1;
				CMenu* pAccelMenu = pSysMenu->GetSubMenu(uOptMenuPos);
				if (pAccelMenu)
				{
					ASSERT( pAccelMenu->m_hMenu == m_SysMenuOptions.m_hMenu );
					VERIFY( pSysMenu->RemoveMenu(uOptMenuPos, MF_BYPOSITION) );
					pAccelMenu = NULL;
				}

				// destroy all 'speed control' menus
				if (m_menuUploadCtrl)
					VERIFY( m_menuUploadCtrl.DestroyMenu() );
				if (m_menuDownloadCtrl)
					VERIFY( m_menuDownloadCtrl.DestroyMenu() );
				if (m_SysMenuOptions)
					VERIFY( m_SysMenuOptions.DestroyMenu() );

				// create new 'speed control' menus
				if (m_SysMenuOptions.CreateMenu())
				{
					AddSpeedSelectorMenus(&m_SysMenuOptions);
					pSysMenu->AppendMenu(MF_STRING|MF_POPUP, (UINT_PTR)m_SysMenuOptions.m_hMenu, GetResString(IDS_EM_PREFS));
				}
			}
		}
	}
	ShowUserStateIcon();
	//toolbar->Localize();
	ShowConnectionState();
	ShowTransferRate(true);
	ShowUserCount();
	CPartFileConvert::Localize();
	if (m_pMiniMule)
		m_pMiniMule->Localize();

	theLocalizeMgr.LocalizeAll();
}

void CemuleDlg::ShowUserStateIcon()
{
	//COMMENTED by fengwen on 2007/06/04 <begin>	状态栏不显示用户数
	//statusbar->SetIcon(SBarUsers, usericon);
	//COMMENTED by fengwen on 2007/06/04 <end>	状态栏不显示用户数
}

void CemuleDlg::QuickSpeedOther(UINT nID)
{
	switch (nID) {
		case MP_QS_PA: 
			//{begin} VC-dgkang 2008年8月12日
			thePrefs.SetMaxUpload(thePrefs.GetMaxGraphUploadRate(true) / 2);
			thePrefs.SetMaxDownload(thePrefs.GetMaxGraphDownloadRate() / 2);
			//{end}
			break ;
		case MP_QS_UA: 
			thePrefs.SetMaxUpload(thePrefs.GetMaxGraphUploadRate(true));
			thePrefs.SetMaxDownload(thePrefs.GetMaxGraphDownloadRate());
			break ;
	}
}


void CemuleDlg::QuickSpeedUpload(UINT nID)
{
	switch (nID) {
		case MP_QS_U10: thePrefs.SetMaxUpload((UINT)(thePrefs.GetMaxGraphUploadRate(true)*0.1)); break ;
		case MP_QS_U20: thePrefs.SetMaxUpload((UINT)(thePrefs.GetMaxGraphUploadRate(true)*0.2)); break ;
		case MP_QS_U30: thePrefs.SetMaxUpload((UINT)(thePrefs.GetMaxGraphUploadRate(true)*0.3)); break ;
		case MP_QS_U40: thePrefs.SetMaxUpload((UINT)(thePrefs.GetMaxGraphUploadRate(true)*0.4)); break ;
		case MP_QS_U50: thePrefs.SetMaxUpload((UINT)(thePrefs.GetMaxGraphUploadRate(true)*0.5)); break ;
		case MP_QS_U60: thePrefs.SetMaxUpload((UINT)(thePrefs.GetMaxGraphUploadRate(true)*0.6)); break ;
		case MP_QS_U70: thePrefs.SetMaxUpload((UINT)(thePrefs.GetMaxGraphUploadRate(true)*0.7)); break ;
		case MP_QS_U80: thePrefs.SetMaxUpload((UINT)(thePrefs.GetMaxGraphUploadRate(true)*0.8)); break ;
		case MP_QS_U90: thePrefs.SetMaxUpload((UINT)(thePrefs.GetMaxGraphUploadRate(true)*0.9)); break ;
		case MP_QS_U100: thePrefs.SetMaxUpload((UINT)thePrefs.GetMaxGraphUploadRate(true)); break ;
//		case MP_QS_UPC: thePrefs.SetMaxUpload(UNLIMITED); break ;
		case MP_QS_UP10: thePrefs.SetMaxUpload(GetRecMaxUpload()); break ;
	}
}

void CemuleDlg::QuickSpeedDownload(UINT nID)
{
	switch (nID) {
		case MP_QS_D10: thePrefs.SetMaxDownload((UINT)(thePrefs.GetMaxGraphDownloadRate()*0.1)); break ;
		case MP_QS_D20: thePrefs.SetMaxDownload((UINT)(thePrefs.GetMaxGraphDownloadRate()*0.2)); break ;
		case MP_QS_D30: thePrefs.SetMaxDownload((UINT)(thePrefs.GetMaxGraphDownloadRate()*0.3)); break ;
		case MP_QS_D40: thePrefs.SetMaxDownload((UINT)(thePrefs.GetMaxGraphDownloadRate()*0.4)); break ;
		case MP_QS_D50: thePrefs.SetMaxDownload((UINT)(thePrefs.GetMaxGraphDownloadRate()*0.5)); break ;
		case MP_QS_D60: thePrefs.SetMaxDownload((UINT)(thePrefs.GetMaxGraphDownloadRate()*0.6)); break ;
		case MP_QS_D70: thePrefs.SetMaxDownload((UINT)(thePrefs.GetMaxGraphDownloadRate()*0.7)); break ;
		case MP_QS_D80: thePrefs.SetMaxDownload((UINT)(thePrefs.GetMaxGraphDownloadRate()*0.8)); break ;
		case MP_QS_D90: thePrefs.SetMaxDownload((UINT)(thePrefs.GetMaxGraphDownloadRate()*0.9)); break ;
		case MP_QS_D100: thePrefs.SetMaxDownload((UINT)thePrefs.GetMaxGraphDownloadRate()); break ;
//		case MP_QS_DC: thePrefs.SetMaxDownload(UNLIMITED); break ;
	}
}
// quick-speed changer -- based on xrmb

int CemuleDlg::GetRecMaxUpload() {
	
	if (thePrefs.GetMaxGraphUploadRate(true)<7) return 0;
	if (thePrefs.GetMaxGraphUploadRate(true)<15) return thePrefs.GetMaxGraphUploadRate(true)-3;
	return (thePrefs.GetMaxGraphUploadRate(true)-4);

}

BOOL CemuleDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch(wParam)
	{	
#if _ENABLE_NOUSE
		case TBBTN_CONNECT:
			OnBnClickedButton2();
			break;
#endif
		case MP_HM_KAD:
		//case TBBTN_KAD:
			SetActiveDialog(kademliawnd);
			break;
		//case TBBTN_SERVER:
		case MP_HM_SRVR:
			SetActiveDialog(serverwnd);
			break;
		//case TBBTN_TRANSFERS:
		case MP_HM_TRANSFER:
			SetActiveDialog(transferwnd);
			break;
		//case TBBTN_SEARCH:
		case MP_HM_SEARCH:
			SetActiveDialog(searchwnd);
			break;
		//case TBBTN_SHARED:
		case MP_HM_FILES:
			SetActiveDialog(sharedfileswnd);
			break;
#if _ENABLE_NOUSE
		case TBBTN_MESSAGES:
		case MP_HM_MSGS:
			SetActiveDialog(chatwnd);
			break;

		case TBBTN_IRC:
		case MP_HM_IRC:
			SetActiveDialog(ircwnd);
			break;
#endif
		//case TBBTN_STATS:
		case MP_HM_STATS:
			SetActiveDialog(statisticswnd);
			break;
		//case TBBTN_OPTIONS:
		case MP_HM_PREFS:
			//toolbar->CheckButton(TBBTN_OPTIONS, TRUE);
			ShowPreferences();
			//toolbar->CheckButton(TBBTN_OPTIONS, FALSE);
			break;

		case MP_SEARCH + 1:
			CmdFuncs::SetMainActiveTab(CMainTabWnd::TI_RESOURCE);
			break;
		case MP_SEARCH + 2:
			CmdFuncs::SetMainActiveTab(CMainTabWnd::TI_DOWNLOAD);
			break;
		case MP_SEARCH + 3:
			CmdFuncs::SetMainActiveTab(CMainTabWnd::TI_SHARE);
			break;
		case MP_SEARCH + 4:
			CmdFuncs::SetMainActiveTab(CMainTabWnd::TI_ADVANCE);
			break;

#if _ENABLE_NOUSE
		case TBBTN_TOOLS:
			ShowToolPopup(true);
			break;
#endif
		case MP_HM_OPENINC:
			ShellExecute(NULL, _T("open"), thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR),NULL, NULL, SW_SHOW); 
			break;
		case MP_HM_HELP:
		//case TBBTN_HELP:
			CmdFuncs::GotoGuide();
			break;
		case MP_HM_CON:
			OnBnClickedButton2();
			break;
		case MP_HM_EXIT:
			OnClose();
			break;
		case MP_HM_LINK1: // MOD: dont remove!
			ShellExecute(NULL, NULL, thePrefs.GetHomepageBaseURL(), NULL, thePrefs.GetMuleDirectory(EMULE_EXECUTEABLEDIR), SW_SHOWDEFAULT);
			break;
		case MP_HM_LINK2:
			//Chocobo Start on 2006.07.31
			//eMule帮助链接，modified by Chocobo
			//ShellExecute(NULL, NULL, _T("http://www.emule.org.cn/guide/"), NULL, thePrefs.GetMuleDirectory(EMULE_EXECUTEABLEDIR), SW_SHOWDEFAULT);
			//Chocobo End
			break;
		case MP_HM_LINK3: {
			//Chocobo Start on 2006.07.31
			//eMule自动更新，modified by Chocobo
			//手动更新链接
			ShellExecute(NULL, NULL, _T("http://www.emule.org.cn/download/"), NULL, thePrefs.GetMuleDirectory(EMULE_EXECUTEABLEDIR), SW_SHOWDEFAULT);
			break;
			//Chocobo End
		}
		case MP_WEBSVC_EDIT:
			theWebServices.Edit();
			break;
		case MP_SEARCHTYPE:
			{
				UINT type = m_mainTabWnd.m_dlgSidePanel.m_SearchBarCtrl.GetType();
				m_mainTabWnd.m_dlgSidePanel.m_SearchBarCtrl.SendMessage(WM_COMMAND, type, 0);
			}
			break;
		case MP_SEARCH:
			m_mainTabWnd.m_dlgSidePanel.m_SearchBarCtrl.GetEditor()->SetFocus();
			m_mainTabWnd.m_dlgSidePanel.m_SearchBarCtrl.GetEditor()->SetSel(0, -1);
			break;
		case MP_HM_CONVERTPF:
			CPartFileConvert::ShowGUI();
			break;
		case MP_HM_SCHEDONOFF:
			thePrefs.SetSchedulerEnabled(!thePrefs.IsSchedulerEnabled());
			theApp.scheduler->Check(true);
			break;
		case MP_HM_1STSWIZARD:
			extern BOOL FirstTimeWizard();
			//MODIFIED by VC-fengwen on 2007/07/17 <begin>	: 两个对话框合并
			//if (FirstTimeWizard()){
			//	// start connection wizard
			//	CConnectionWizardDlg conWizard;
			//	conWizard.DoModal();
			//}
			FirstTimeWizard();
			//MODIFIED by VC-fengwen on 2007/07/17 <end>	: 两个对话框合并
			break;
		case MP_HM_IPFILTER:{
			CIPFilterDlg dlg;
			dlg.DoModal();
			break;
		}
		case MP_HM_DIRECT_DOWNLOAD:{
			CDirectDownloadDlg dlg;
			dlg.DoModal();
			break;
		}

#if _ENABLE_NOUSE
		//Added by thilon on 2006.08.01
		case MP_HM_WEBBROWSER:
	    case TBBTN_WEBBROWSER:
			SetActiveDialog(webbrowser);
			break;
#endif
		case MP_NEW:
			CmdFuncs::PopupNewTaskDlg();
			break;
	}	
	if (wParam>=MP_WEBURL && wParam<=MP_WEBURL+99) {
		theWebServices.RunURL(NULL, wParam);
	}
	else if (wParam>=MP_SCHACTIONS && wParam<=MP_SCHACTIONS+99) {
		theApp.scheduler->ActivateSchedule(wParam-MP_SCHACTIONS);
		theApp.scheduler->SaveOriginals(); // use the new settings as original
	}

	return CTrayDialog::OnCommand(wParam, lParam);
}

LRESULT CemuleDlg::OnMenuChar(UINT nChar, UINT nFlags, CMenu* pMenu)
{
/*
	UINT nCmdID;
	if (toolbar->MapAccelerator((TCHAR)nChar, &nCmdID)){
		OnCommand(nCmdID, 0);
		return MAKELONG(0,MNC_CLOSE);
	}
*/
	return CTrayDialog::OnMenuChar(nChar, nFlags, pMenu);
}

// Barry - To find out if app is running or shutting/shut down
bool CemuleDlg::IsRunning()
{
	return (CGlobalVariable::m_app_state == APP_STATE_RUNNING);
}


void CemuleDlg::OnBnClickedHotmenu()
{
	ShowToolPopup(false);
}

void CemuleDlg::ShowToolPopup(bool toolsonly)
{
	POINT point;
	::GetCursorPos(&point);

	CTitleMenu menu;
	menu.CreatePopupMenu();
	if (!toolsonly)
		menu.AddMenuTitle(GetResString(IDS_HOTMENU), true);
	else
		menu.AddMenuTitle(GetResString(IDS_TOOLS), true);

	CTitleMenu Links;
	Links.CreateMenu();
	Links.AddMenuTitle(NULL, true);
	Links.AppendMenu(MF_STRING, MP_HM_LINK1, GetResString(IDS_HM_LINKHP), _T("WEB"));
	Links.AppendMenu(MF_STRING, MP_HM_LINK2, GetResString(IDS_HM_LINKFAQ), _T("WEB"));
	Links.AppendMenu(MF_STRING, MP_HM_LINK3, GetResString(IDS_HM_LINKVC), _T("WEB"));
	theWebServices.GetGeneralMenuEntries(&Links);
	Links.InsertMenu(3, MF_BYPOSITION | MF_SEPARATOR);
	Links.AppendMenu(MF_STRING, MP_WEBSVC_EDIT, GetResString(IDS_WEBSVEDIT));

	CMenu scheduler;
	scheduler.CreateMenu();
	CString schedonoff= (!thePrefs.IsSchedulerEnabled())?GetResString(IDS_HM_SCHED_ON):GetResString(IDS_HM_SCHED_OFF);

	scheduler.AppendMenu(MF_STRING,MP_HM_SCHEDONOFF, schedonoff);
	if (theApp.scheduler->GetCount()>0) {
		scheduler.AppendMenu(MF_SEPARATOR);
		for (UINT i=0; i<theApp.scheduler->GetCount();i++)
			scheduler.AppendMenu(MF_STRING,MP_SCHACTIONS+i, theApp.scheduler->GetSchedule(i)->title );
	}

	if (!toolsonly) {
		if (CGlobalVariable::serverconnect->IsConnected())
			menu.AppendMenu(MF_STRING,MP_HM_CON,GetResString(IDS_MAIN_BTN_DISCONNECT), _T("DISCONNECT"));
		else if (CGlobalVariable::serverconnect->IsConnecting())
			menu.AppendMenu(MF_STRING,MP_HM_CON,GetResString(IDS_MAIN_BTN_CANCEL), _T("STOPCONNECTING"));
		else
			menu.AppendMenu(MF_STRING,MP_HM_CON,GetResString(IDS_MAIN_BTN_CONNECT), _T("CONNECT"));

		menu.AppendMenu(MF_STRING,MP_HM_KAD, GetResString(IDS_EM_KADEMLIA), _T("KADEMLIA") );
		menu.AppendMenu(MF_STRING,MP_HM_SRVR, GetResString(IDS_EM_SERVER), _T("SERVER") );
		menu.AppendMenu(MF_STRING,MP_HM_TRANSFER, GetResString(IDS_EM_TRANS),_T("TRANSFER") );
		menu.AppendMenu(MF_STRING,MP_HM_SEARCH, GetResString(IDS_EM_SEARCH), _T("SEARCH"));
		menu.AppendMenu(MF_STRING,MP_HM_FILES, GetResString(IDS_EM_FILES), _T("SharedFiles"));
		menu.AppendMenu(MF_STRING,MP_HM_MSGS, GetResString(IDS_EM_MESSAGES), _T("MESSAGES"));
		menu.AppendMenu(MF_STRING,MP_HM_IRC, GetResString(IDS_IRC), _T("IRC"));
		menu.AppendMenu(MF_STRING,MP_HM_STATS, GetResString(IDS_EM_STATISTIC), _T("STATISTICS"));
		menu.AppendMenu(MF_STRING,MP_HM_PREFS, GetResString(IDS_EM_PREFS), _T("PREFERENCES"));
		menu.AppendMenu(MF_STRING,MP_HM_HELP, GetResString(IDS_HELP_), _T("HELP"));
		menu.AppendMenu(MF_STRING,MP_HM_WEBBROWSER, GetResString(IDS_EM_WEBBROWSER), _T("WEBBROWSER"));// Added by thilon on 2006.08.01
		menu.AppendMenu(MF_SEPARATOR);
	}

	menu.AppendMenu(MF_STRING,MP_HM_OPENINC, GetResString(IDS_OPENINC) + _T("..."), _T("OPENFOLDER"));
	menu.AppendMenu(MF_STRING,MP_HM_CONVERTPF, GetResString(IDS_IMPORTSPLPF) + _T("..."), _T("CONVERT"));
	menu.AppendMenu(MF_STRING,MP_HM_1STSWIZARD, GetResString(IDS_WIZ1) + _T("..."), _T("WIZARD"));
	menu.AppendMenu(MF_STRING,MP_HM_IPFILTER, GetResString(IDS_IPFILTER) + _T("..."), _T("IPFILTER"));
	//Chocobo Start
	//TCP连接补丁链接，added by Chocobo on 2006.08.04
	//menu.AppendMenu(MF_STRING,MP_HM_TCP_PATCH, GetResString(IDS_TCP_PATCH_LINK), NULL);
	//Chocobo End
	menu.AppendMenu(MF_STRING,MP_HM_DIRECT_DOWNLOAD, GetResString(IDS_SW_DIRECTDOWNLOAD) + _T("..."), _T("PASTELINK"));

	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)Links.m_hMenu, GetResString(IDS_LINKS), _T("WEB") );
	menu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)scheduler.m_hMenu, GetResString(IDS_SCHEDULER), _T("SCHEDULER") );

	if (!toolsonly) {
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING,MP_HM_EXIT, GetResString(IDS_EXIT), _T("EXIT"));
	}
	menu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);
	VERIFY( Links.DestroyMenu() );
	VERIFY( scheduler.DestroyMenu() );
	VERIFY( menu.DestroyMenu() );
}


void CemuleDlg::ApplyHyperTextFont(LPLOGFONT plf)
{
	theApp.m_fontHyperText.DeleteObject();
	if (theApp.m_fontHyperText.CreateFontIndirect(plf))
	{
		thePrefs.SetHyperTextFont(plf);
		serverwnd->servermsgbox->SetFont(&theApp.m_fontHyperText);
#if _ENABLE_NOUSE
		chatwnd->chatselector.UpdateFonts(&theApp.m_fontHyperText);
		ircwnd->UpdateFonts(&theApp.m_fontHyperText);
#endif
	}
}

void CemuleDlg::ApplyLogFont(LPLOGFONT plf)
{
	theApp.m_fontLog.DeleteObject();
	if (theApp.m_fontLog.CreateFontIndirect(plf))
	{
		thePrefs.SetLogFont(plf);
		serverwnd->logbox->SetFont(&theApp.m_fontLog);
		serverwnd->leecherlog->SetFont(&theApp.m_fontLog);
	}
}

LRESULT CemuleDlg::OnFrameGrabFinished(WPARAM wParam,LPARAM lParam){
	CKnownFile* pOwner = (CKnownFile*)wParam;
	FrameGrabResult_Struct* result = (FrameGrabResult_Struct*)lParam;
	
	if (CGlobalVariable::knownfiles->IsKnownFile(pOwner) || CGlobalVariable::downloadqueue->IsPartFile(pOwner) ){
		pOwner->GrabbingFinished(result->imgResults,result->nImagesGrabbed, result->pSender);
	}
	else{
		ASSERT ( false );
	}

	delete result;
	return 0;
}

void StraightWindowStyles(CWnd* pWnd)
{
	CWnd* pWndChild = pWnd->GetWindow(GW_CHILD);
	while (pWndChild)
	{
		StraightWindowStyles(pWndChild);
		pWndChild = pWndChild->GetNextWindow();
	}

	CHAR szClassName[MAX_PATH];
	if (::GetClassNameA(*pWnd, szClassName, ARRSIZE(szClassName)))
	{
		if (__ascii_stricmp(szClassName, "Button") == 0)
			pWnd->ModifyStyle(BS_FLAT, 0);
		else if (   (__ascii_stricmp(szClassName, "EDIT") == 0 && (pWnd->GetExStyle() & WS_EX_STATICEDGE))
			|| __ascii_stricmp(szClassName, "SysListView32") == 0
			|| __ascii_stricmp(szClassName, "msctls_trackbar32") == 0
			)
		{
			pWnd->ModifyStyleEx(WS_EX_STATICEDGE, WS_EX_CLIENTEDGE);
		}
	}
}

static bool s_bIsXPStyle;

void FlatWindowStyles(CWnd* pWnd)
{
	CWnd* pWndChild = pWnd->GetWindow(GW_CHILD);
	while (pWndChild)
	{
		FlatWindowStyles(pWndChild);
		pWndChild = pWndChild->GetNextWindow();
	}

	CHAR szClassName[MAX_PATH];
	if (::GetClassNameA(*pWnd, szClassName, ARRSIZE(szClassName)))
	{
		if (__ascii_stricmp(szClassName, "Button") == 0)
		{
			if (!s_bIsXPStyle || (pWnd->GetStyle() & BS_ICON) == 0)
				pWnd->ModifyStyle(0, BS_FLAT);
		}
		else if (__ascii_stricmp(szClassName, "SysListView32") == 0)
		{
			pWnd->ModifyStyleEx(WS_EX_CLIENTEDGE, WS_EX_STATICEDGE);
		}
		else if (__ascii_stricmp(szClassName, "SysTreeView32") == 0)
		{
			pWnd->ModifyStyleEx(WS_EX_CLIENTEDGE, WS_EX_STATICEDGE);
		}
	}
}

void InitWindowStyles(CWnd* pWnd)
{
	if (thePrefs.GetStraightWindowStyles() < 0)
		return;
	else if (thePrefs.GetStraightWindowStyles() > 0)
		/*StraightWindowStyles(pWnd)*/;	// no longer needed
	else
	{
		s_bIsXPStyle = g_xpStyle.IsAppThemed() && g_xpStyle.IsThemeActive();
		if (!s_bIsXPStyle)
			FlatWindowStyles(pWnd);
	}
}


LRESULT CemuleDlg::OnVersionCheckResponse(WPARAM /*wParam*/, LPARAM lParam)
{
	if (WSAGETASYNCERROR(lParam) == 0)
	{
		int iBufLen = WSAGETASYNCBUFLEN(lParam);
		if (iBufLen >= sizeof(HOSTENT))
		{
			LPHOSTENT pHost = (LPHOSTENT)m_acVCDNSBuffer;
			if (pHost->h_length == 4 && pHost->h_addr_list && pHost->h_addr_list[0])
			{
				uint32 dwResult = ((LPIN_ADDR)(pHost->h_addr_list[0]))->s_addr;		
				// last byte contains informations about mirror urls, to avoid effects of future DDoS Attacks against eMules Homepage
				thePrefs.SetWebMirrorAlertLevel((uint8)(dwResult >> 24));

				//Chocobo Start on 2006.07.31
				//eMule自动更新，added by Chocobo
				thePrefs.UpdateLastVC();//刷新最后更新时间
				SetActiveWindow();

				if(!m_bCreated)
				{
					//创建共享内存
					m_hMapping = CreateFileMapping((HANDLE)0xFFFFFFFF, NULL, PAGE_READWRITE, 0, BUF_SIZE, _T("Update"));

					if(m_hMapping != NULL && m_hMapping != INVALID_HANDLE_VALUE)
					{
						//m_pUpdaterThread = (CUpdaterThread*)::AfxBeginThread(RUNTIME_CLASS(CUpdaterThread), THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED, NULL);
						m_pUpdaterThread = (CUpdaterThread*)CThreadsMgr::BegingThreadAndRecDown(CThreadsMgr::CleanProc_WaitAndDelWinThd,RUNTIME_CLASS(CUpdaterThread), 
							THREAD_PRIORITY_BELOW_NORMAL,0,CREATE_SUSPENDED);

						if (NULL != m_pUpdaterThread)
						{
							m_pUpdaterThread->SetParent(this);
							m_pUpdaterThread->m_bAutoDelete = TRUE;
							m_pUpdaterThread->ResumeThread();

							m_bCreated = true;
						}
						
					}

					
				}
				//Chocobo End
				return 0;
			}
		}
	}
	LogWarning(LOG_STATUSBAR,GetResString(IDS_NEWVERSIONFAILED));
	return 0;
}

LRESULT CemuleDlg::OnKickIdle(UINT /*nWhy*/, long /*lIdleCount*/)
{
	LRESULT lResult = 0;

	if (m_bStartMinimized)
		PostStartupMinimized();

	if (searchwnd && searchwnd->m_hWnd)
	{
		if (CGlobalVariable::m_app_state != APP_STATE_SHUTTINGDOWN)
		{
			//extern void Mfc_IdleUpdateCmdUiTopLevelFrameList(CWnd* pMainFrame);
			//Mfc_IdleUpdateCmdUiTopLevelFrameList(this);
			theApp.OnIdle(0/*lIdleCount*/);	// NOTE: DO **NOT** CALL THIS WITH 'lIdleCount>0'

#ifdef _DEBUG
			// We really should call this to free up the temporary object maps from MFC.
			// It may/will show bugs (wrong usage of temp. MFC data) on couple of (hidden) places,
			// therefore it's right now too dangerous to put this in 'Release' builds..
			// ---
			// The Microsoft Foundation Class (MFC) Libraries create temporary objects that are 
			// used inside of message handler functions. In MFC applications, these temporary 
			// objects are automatically cleaned up in the CWinApp::OnIdle() function that is 
			// called in between processing messages.

			// To slow to be called on each KickIdle. Need a timer
			//extern void Mfc_IdleFreeTempMaps();
			//if (lIdleCount >= 0)
			//	Mfc_IdleFreeTempMaps();
#endif
		}
	}

	return lResult;
}

#if _ENABLE_NOUSE
int CemuleDlg::MapWindowToToolbarButton(CWnd* pWnd) const
{
	int iButtonID = -1;
	if (pWnd == transferwnd)        iButtonID = TBBTN_TRANSFERS;
	else if (pWnd == serverwnd)     iButtonID = TBBTN_SERVER;
#if _ENABLE_NOUSE
	else if (pWnd == chatwnd)       iButtonID = TBBTN_MESSAGES;
	else if (pWnd == ircwnd)        iButtonID = TBBTN_IRC;
#endif
	else if (pWnd == sharedfileswnd)iButtonID = TBBTN_SHARED;
	else if (pWnd == searchwnd)     iButtonID = TBBTN_SEARCH;
	else if (pWnd == statisticswnd)	iButtonID = TBBTN_STATS;
	else if	(pWnd == kademliawnd)	iButtonID = TBBTN_KAD;
	//else if (pWnd == webbrowser)    iButtonID = TBBTN_WEBBROWSER;		//Added by thilon on 2006.08.01
	else ASSERT(0);
	return iButtonID;
}
#endif

#if _ENABLE_NOUSE
CWnd* CemuleDlg::MapToolbarButtonToWindow(int iButtonID) const
{
	CWnd* pWnd;
	switch (iButtonID)
	{
		case TBBTN_TRANSFERS:	pWnd = transferwnd;		break;
		case TBBTN_SERVER:		pWnd = serverwnd;		break;

		case TBBTN_MESSAGES:	pWnd = chatwnd;			break;
		case TBBTN_IRC:			pWnd = ircwnd;			break;
		case TBBTN_SHARED:		pWnd = sharedfileswnd;	break;
		case TBBTN_SEARCH:		pWnd = searchwnd;		break;
		case TBBTN_STATS:		pWnd = statisticswnd;	break;
		case TBBTN_KAD:			pWnd = kademliawnd;		break;
		default:				pWnd = NULL; ASSERT(0);
	}
	return pWnd;
}

bool CemuleDlg::IsWindowToolbarButton(int iButtonID) const
{
	switch (iButtonID)
	{
		case TBBTN_TRANSFERS:	return true;
		case TBBTN_SERVER:		return true;
		case TBBTN_MESSAGES:	return true;
		case TBBTN_IRC:			return true;
		case TBBTN_SHARED:		return true;
		case TBBTN_SEARCH:		return true;
		case TBBTN_STATS:		return true;
		case TBBTN_KAD:			return true;
	}
	return false;
}

int CemuleDlg::GetNextWindowToolbarButton(int iButtonID, int iDirection) const
{
	ASSERT( iDirection == 1 || iDirection == -1 );
	int iButtonCount = toolbar->GetButtonCount();
	if (iButtonCount > 0)
	{
		int iButtonIdx = toolbar->CommandToIndex(iButtonID);
		if (iButtonIdx >= 0 && iButtonIdx < iButtonCount)
		{
			int iEvaluatedButtons = 0;
			while (iEvaluatedButtons < iButtonCount)
			{
				iButtonIdx = iButtonIdx + iDirection;
				if (iButtonIdx < 0)
					iButtonIdx = iButtonCount - 1;
				else if (iButtonIdx >= iButtonCount)
					iButtonIdx = 0;

				TBBUTTON tbbt = {0};
				if (toolbar->GetButton(iButtonIdx, &tbbt))
				{
					if (IsWindowToolbarButton(tbbt.idCommand))
						return tbbt.idCommand;
				}
				iEvaluatedButtons++;
			}
		}
	}
	return -1;
}
#endif

BOOL CemuleDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		// Handle Ctrl+Tab and Ctrl+Shift+Tab
		if (pMsg->wParam == VK_TAB && GetAsyncKeyState(VK_CONTROL) < 0)
		{
			m_mainTabWnd.SwitchToNextPage();
			return TRUE;
		}

		if (pMsg->wParam == VK_ESCAPE)
		{
			HWND hWnd = ::FindWindow(0,GetResString(IDS_ADD_TASK));

			if (hWnd)
			{
				::SendMessage(hWnd, WM_COMMAND, IDCANCEL, 0);
				return TRUE;
			}
			return CTrayDialog::PreTranslateMessage(pMsg);
		}
	}

	DWORD dwShortcut = 0;
	UINT nCmdID = m_mgrShortcuts.ProcessMessage(pMsg, &dwShortcut);

	if (nCmdID)
	{
		BOOL bSendMessage = TRUE;

		if (bSendMessage)
		{
			SendMessage(WM_COMMAND, nCmdID);
			return TRUE;
		}
	}
	else
	{
		return FALSE;
	}

	return CTrayDialog::PreTranslateMessage(pMsg);
}

#pragma warning(push)				//	for aMessageParam's autoptr
#pragma warning(disable : 4239)

LRESULT CemuleDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
#ifdef _DEBUG
	if(message==WM_SYSCOMMAND)
	{
		CString str;
		str.Format(_T("sys_command, wParam=%d, lParam=%d\n"), wParam, lParam);
		OutputDebugString(str);
	}
#endif
	switch(message)
	{
	case WM_FILE_ADD_DOWNLOAD:
		transferwnd->downloadlistctrl.AddFile((CPartFile*)lParam);
		break;
	case WM_UPDATTREEITEM:
		{
			aMessageParam param=CMessageLog::GetInstace()->GetMessage(wParam);
			if(param==NULL) break;

			sharedfileswnd->m_ctlSharedDirTree.UpdateTreeItem((CKnownFile*)param->lParam);
			break;
		}

	case WM_SHAREDFILE_UPDATECHECKBOX:
		{
			aMessageParam param=CMessageLog::GetInstace()->GetMessage(wParam);
			if(param==NULL) break;

			sharedfileswnd->sharedfilesctrl.RedrawCheckBox(param->wParam, (CKnownFile*)param->lParam);
			break;
		}
	//// VC-Huby[2007-01-24]: process sharedfile message
	case WM_SHAREDFILE_UPDATE:
		{
			aMessageParam param=CMessageLog::GetInstace()->GetMessage(wParam);
			if(param==NULL) break;

			if (sharedfileswnd->sharedfilesctrl.IsWindowVisible())
				sharedfileswnd->sharedfilesctrl.UpdateFile((CKnownFile*)param->lParam);
		}
		//sharedfileswnd->sharedfilesctrl.UpdateFile((CKnownFile*)lParam);
		break;
	case WM_SHAREDFILE_SHOWCOUNT:
		sharedfileswnd->sharedfilesctrl.ShowFilesCount();
		break;
	case WM_SHAREDFILE_SETAICHHASHING:
		{
			aMessageParam param=CMessageLog::GetInstace()->GetMessage(wParam);
			if(param==NULL) break;
			sharedfileswnd->sharedfilesctrl.SetAICHHashing(param->lParam);
		}
		break;
    // VC-SearchDream[2007-01-25]:process search file message
	case WM_SEARCH_UPDATESEARCH:
		searchwnd->UpdateSearch((CSearchFile*)lParam);
		break; 
	case WM_SEARCH_LOCALED2KEND:
		searchwnd->LocalEd2kSearchEnd((UINT)wParam, BOOL2bool(lParam));
		break;
	case WM_SEARCH_NEWED2KSEARCH:
		searchwnd->DoNewEd2kSearch((SSearchParams*)lParam);
		break;
	case WM_SEARCH_NEWKADSEARCH:
		searchwnd->DoNewKadSearch((SSearchParams*)lParam);
		break;
	case WM_SEARCH_REMOVERESULT:
		searchwnd->RemoveResult((CSearchFile*)lParam);
		break;
	case WM_SEARCH_NEWTAB:

		//VC-dgkang 2008年7月13日
		//searchwnd->CreateNewTab((SSearchParams*)lParam);

		return m_mainTabWnd.m_dlgResource.CreateNewShareFileTab((SSearchParams*)lParam);

		break;
	case WM_SEARCH_CANCELSEARCH:
		searchwnd->CancelEd2kSearch();
		break;
	case WM_SEARCH_CANCELKADSEARCH:
		{
			if (theApp.emuledlg && theApp.emuledlg->searchwnd)
			{
				searchwnd->CancelKadSearch((UINT)lParam);
			}
			break;
		}
	case WM_SEARCH_SHOWRESULT:
		{
			SearchFileList * pSearchFileList = (SearchFileList*)lParam;

			searchwnd->m_pwndResults->searchlistctrl.SetRedraw(FALSE);
			CMuleListCtrl::EUpdateMode bCurUpdateMode = searchwnd->m_pwndResults->searchlistctrl.SetUpdateMode(CMuleListCtrl::none);

			for (POSITION pos = pSearchFileList->GetHeadPosition(); pos != NULL; )
			{
				const CSearchFile* cur_file = pSearchFileList->GetNext(pos);

				searchwnd->m_pwndResults->searchlistctrl.AddResult(cur_file);
				if (cur_file->IsListExpanded() && cur_file->GetListChildCount() > 0)
				{
					searchwnd->m_pwndResults->searchlistctrl.UpdateSources(cur_file);
				}
			}

			searchwnd->m_pwndResults->searchlistctrl.SetUpdateMode(bCurUpdateMode);
			searchwnd->m_pwndResults->searchlistctrl.SetRedraw(TRUE);

			delete pSearchFileList; // 09/27/2007 Added by Soar Chin to resolve memory leaking

			break;
		}
	case WM_SEARCH_UPDATESOURCE:
		{
			//VC-dgkang 2008年8月4日
			searchwnd->m_pwndResults->searchlistctrl.UpdateSources((CSearchFile*)lParam);
			break;
		}
	case WM_SEARCH_ADDRESULT:
		{
			aMessageParam param=CMessageLog::GetInstace()->GetMessage(wParam);
			searchwnd->m_pwndResults->searchlistctrl.AddResult((CSearchFile*)param->lParam);
			break;
		}
	case WM_SEARCH_ADDED2KRESULT:
		{
			searchwnd->AddGlobalEd2kSearchResults((UINT)lParam);
			break;
		}
	case WM_ADD_LOGTEXT:
		{
			aMessageParam param=CMessageLog::GetInstace()->GetMessage(wParam);
			if(param==NULL) break;

			CString * pInfo=(CString *)param->lParam;
			AddLogText(param->wParam, (LPCTSTR)*pInfo);
			delete pInfo;
			return 1;
			break;
		}
	case WM_SERVER_ADD_SVR:
		{
			bool bAddtoList = BOOL2bool(lParam & 0xff);			
			bool bRet=serverwnd->serverlistctrl.AddServer((CServer*)wParam, bAddtoList);
			bool bRefresh = BOOL2bool(lParam & 0xff00);
			if( !bRet && bRefresh) //添加不成功,才有必要检查是否要刷新
			{
				CServer * newserver = (CServer*)wParam;
				CServer* update = CGlobalVariable::serverlist->GetServerByAddress(newserver->GetAddress(), newserver->GetPort());
				if (update)
				{
					update->SetListName(newserver->GetListName());
					update->SetIsStaticMember(newserver->IsStaticMember());
					update->SetPreference(newserver->GetPreference());
					update->SetDescription(newserver->GetDescription());
					serverwnd->serverlistctrl.RefreshServer(update);
				}
			}
			return bRet;
			break;
		}
	case WM_SHOW_CONNECTION_STATE:
		ShowConnectionState();
		break;
	case WM_FILE_UPDATE_PEER:
		{
			
			aMessageParam param=CMessageLog::GetInstace()->GetMessage(wParam);
			if(param==NULL) break;

			//{begin}VC-dgkang 2008年8月4日

			CUpDownClient * client =(CUpDownClient*)param->lParam;
			if(param->wParam & UI_UPDATE_DOWNLOAD_PEERLIST)
			{
				if(transferwnd->downloadclientsctrl.IsWindowVisible())
					transferwnd->downloadclientsctrl.RefreshClient(client);
			}
			if(param->wParam & UI_UPDATE_DOWNLOADLIST)
			{	
				if (transferwnd->downloadlistctrl.IsWindowVisible())
					transferwnd->downloadlistctrl.UpdateItem(client);
			}
			if(param->wParam & UI_UPDATE_PEERLIST)
			{
				if(transferwnd->clientlistctrl.IsWindowVisible())
					transferwnd->clientlistctrl.RefreshClient(client);
			}
			if(param->wParam & UI_UPDATE_QUEUELIST)
			{
				if (transferwnd->queuelistctrl.IsWindowVisible())
					transferwnd->queuelistctrl.RefreshClient(client);
			}
			if(param->wParam & UI_UPDATE_UPLOADLIST)
			{
				if (sharedfileswnd->m_UpLoading.uploadlistctrl.IsWindowVisible())
					sharedfileswnd->m_UpLoading.uploadlistctrl.RefreshClient(client);
				
				if(transferwnd->uploadlistctrl.IsWindowVisible())
					transferwnd->uploadlistctrl.RefreshClient(client);
			}
			//{end}
		}
		break;
	case WM_FILE_ADD_SOURCE:
		{
			transferwnd->downloadlistctrl.AddSource((CPartFile*)wParam,(CUpDownClient*)lParam,true);
		}
		break;
	case WM_FILE_ADD_SOURCE_NA:
		{
			transferwnd->downloadlistctrl.AddSource((CPartFile*)wParam,(CUpDownClient*)lParam,false);
		}
		break;
	case WM_FILE_ADD_PEER:
		{
			aMessageParam param=CMessageLog::GetInstace()->GetMessage(wParam);
			if(param==NULL) break;

			CUpDownClient * toadd=(CUpDownClient *)param->lParam;
			switch(param->wParam)
			{
			case 1:
				transferwnd->clientlistctrl.AddClient(toadd);
				break;
			case 2:
				transferwnd->downloadclientsctrl.AddClient(toadd);
				break;
			case 3:
				{
					bool bAdd = theApp.emuledlg->sharedfileswnd->AddNewClient(toadd);
					if (!bAdd)
					{
						break;
					}					
					sharedfileswnd->m_UpLoading.uploadlistctrl.AddClient(toadd); 
				}
				break;
			case 4:
				transferwnd->queuelistctrl.AddClient(toadd, true);
				break;
			}
		}
		break;
	case WM_FILE_REMOVE_PEER:
		{
			aMessageParam param=CMessageLog::GetInstace()->GetMessage(wParam);
			if(param==NULL) break;

			CUpDownClient * toremove=(CUpDownClient*)param->lParam;
			switch(param->wParam)
			{
			case 1:
				transferwnd->clientlistctrl.RemoveClient(toremove);
				break;
			case 2:
				transferwnd->downloadclientsctrl.RemoveClient(toremove);
				break;
			case 3:
				{
					sharedfileswnd->m_UpLoading.uploadlistctrl.RemoveClient(toremove);
				}
				
				break;
			case 4:
				transferwnd->queuelistctrl.RemoveClient(toremove);
				break;
			}
		}
		break;
	case WM_FILE_REMOVE_DOWNLOAD:
		{
			aMessageParam param=CMessageLog::GetInstace()->GetMessage(wParam);
			if(param==NULL) break;
			transferwnd->downloadlistctrl.RemoveFile((CPartFile*)param->lParam);
			if(param->wParam==1) //是文件被删除的情况
				m_mainTabWnd.m_dlgDownload.m_lcDownloaded.OnRemoveFile(0,param->lParam); //也可以判断一下该PartFile是否是已下载完成的.
		}
		break;
	case WM_FILE_UPDATE_FILECOUNT:
		{
			transferwnd->downloadlistctrl.ShowFilesCount();
		}
		break;
	case WM_FILE_HIDE_DOWNLOAD:
		{
			aMessageParam param=CMessageLog::GetInstace()->GetMessage(wParam);
			if(param==NULL) break;
			transferwnd->downloadlistctrl.HideSources((CPartFile*)param->lParam);
		}
		break;
	case WM_SHAREDFILE_RELOADFILELIST:
		sharedfileswnd->sharedfilesctrl.ReloadFileList();
		break;
	case WM_LINE_SIG:
		theApp.OnlineSig();
		break;
	case WM_SHOW_TRANSFER_RATE:
		{
			//VC-dgkang 2008年8月4日
			theApp.OnlineSig(); // Added By Bouc7 
			if (!IsTrayIconToFlash())
				ShowTransferRate();
			thePrefs.EstimateMaxUploadCap(CGlobalVariable::uploadqueue->GetDatarate()/1024);

			if (!thePrefs.TransferFullChunks())
				CGlobalVariable::uploadqueue->UpdateMaxClientScore();

			// update cat-titles with downloadinfos only when needed

			if (thePrefs.ShowCatTabInfos() && activewnd == transferwnd &&  IsWindowVisible()) 
				transferwnd->UpdateCatTabTitles(false);

			if (thePrefs.IsSchedulerEnabled())
			theApp.scheduler->Check();

			transferwnd->UpdateListCount(CTransferWnd::wnd2Uploading, -1);
		}
		break;
	case WM_SHAREDFILE_ADDFILE:
		{
			aMessageParam param=CMessageLog::GetInstace()->GetMessage(wParam);
			if(param!=NULL)
				sharedfileswnd->sharedfilesctrl.AddFile((const CKnownFile*)param->lParam);
		}
		break;
	case WM_FILE_NOTIFYSTATUSCHANGE:
		transferwnd->downloadlistctrl.UpdateCurrentCategoryView((CPartFile*)lParam);
		break;
	case WM_FILE_UPDATE_DOWNLOAD:
		{
			aMessageParam param=CMessageLog::GetInstace()->GetMessage(wParam);
			if(param!=NULL && transferwnd->downloadlistctrl.IsWindowVisible())		//VC-dgakng 2008年8月4日
				transferwnd->downloadlistctrl.UpdateItem((CPartFile*)param->lParam);
		}
		break;
	case WM_FILE_UPDATE_DOWNLOADING:
		{
			LRESULT lRet=0;
			if (transferwnd->downloadlistctrl.curTab == 0)
				transferwnd->downloadlistctrl.ChangeCategory(0); 
			else lRet = 1;
			if (thePrefs.ShowCatTabInfos() )
				transferwnd->UpdateCatTabTitles();

			return lRet;
		}
		break;
	case WM_SHAREDFILE_REMOVEFILE:
		{
			aMessageParam param=CMessageLog::GetInstace()->GetMessage(wParam);
			if(param==NULL) break;

			sharedfileswnd->sharedfilesctrl.RemoveFile((const CKnownFile*)param->lParam);
			::SendMessage(theWndMgr.GetWndHandle(CWndMgr::WI_DOWNLOADED_LISTCTRL), UM_DLED_LC_REMOVEFILE, 0, param->lParam);
		}
		break;
	case WM_SHAREDFILE_REMOVESHARE:
		{
			aMessageParam param=CMessageLog::GetInstace()->GetMessage(wParam);
			if(param==NULL) 
				break;
			//sharedfileswnd->sharedfilesctrl.RemoveFile((const CKnownFile*)param->lParam);
		}
		break;
	case WM_FILE_REMOVE_SOURCE:
		{
			aMessageParam param=CMessageLog::GetInstace()->GetMessage(wParam);
			if(param==NULL) break;
			transferwnd->downloadlistctrl.RemoveSource((CUpDownClient*)param->lParam,(CPartFile*)param->wParam);

			if (((CUpDownClient*)param->lParam)->m_iPeerType == ptHttp)
			{
				theApp.emuledlg->m_mainTabWnd.m_dlgDownload.m_dlgPeerLog.m_LogListCtrl.DeleteAllItems();
			}
		}
		break;
	case WM_FILE_UPDATE_UPLOADRANK:
		{
			aMessageParam param=CMessageLog::GetInstace()->GetMessage(wParam);
			if(param==NULL) break;
			transferwnd->ShowQueueCount(param->lParam);
		}
		break;
	case WM_FILE_SHOWNOTIFIER:
		ShowNotifier(*(CString*)wParam, lParam);
		break;
	case WM_GET_IL_COLORFLAGS:
		return theApp.m_iDfltImageListColorFlags;
	case WM_SERVER_REFRESH:
		{
			aMessageParam param=CMessageLog::GetInstace()->GetMessage(wParam);
			if(param==NULL) break;

			serverwnd->serverlistctrl.RefreshServer((CServer*)param->lParam);
			serverwnd->serverlistctrl.Invalidate();
			if(param->wParam==1)
				serverwnd->UpdateMyInfo();
		}
		break;
	case WM_SHOW_USERCOUNT:
		ShowUserCount();
		break;
	case WM_KAD_CONTACTADD:
		return kademliawnd->ContactAdd((Kademlia::CContact*)lParam);
		break;
	case WM_KAD_CONTACTREF:
		kademliawnd->ContactRef((Kademlia::CContact*)lParam);		
		break;
	case WM_KAD_CONTACTREM:
		{
			aMessageParam param=CMessageLog::GetInstace()->GetMessage(wParam);
			if(param==NULL) break;
			kademliawnd->ContactRem((Kademlia::CContact*)param->lParam);
		}
		break;
	case WM_KAD_SEARCHADD:
		kademliawnd->searchList->SearchAdd((const Kademlia::CSearch*)lParam);
		break;
	case WM_KAD_SEARCHREF:
		kademliawnd->searchList->SearchRef((const Kademlia::CSearch*)lParam);
		break;
	case WM_KAD_SEARCHREM:
		kademliawnd->searchList->SearchRem((const Kademlia::CSearch*)lParam);
		break;
	case WM_KAD_SHOWCONTACTS:
		kademliawnd->ShowContacts();
		break;
	case WM_KAD_HIDECONTACTS:
		kademliawnd->HideContacts();
		break;
	case WM_FILE_UPDATE_PEERLOG:
		{
			aMessageParam param=CMessageLog::GetInstace()->GetMessage(wParam);
			if(param==NULL)
			{
				break;
			}

			CUpDownClient* client = (CUpDownClient*)param->wParam;

			if (client == NULL)
			{
				break;
			}

			int result = theApp.emuledlg->transferwnd->downloadlistctrl.FindFile(client);

			if (result != -1)
			{
				int iState = theApp.emuledlg->transferwnd->downloadlistctrl.GetItemState(result, LVIS_SELECTED | LVIS_FOCUSED);

				if (iState == (LVIS_SELECTED | LVIS_FOCUSED))
				{
					theApp.emuledlg->m_mainTabWnd.m_dlgDownload.m_dlgPeerLog.m_LogListCtrl.AddLog((CTraceEvent*)param->lParam);
				}
			}
		}
		break;
	case WM_FILE_REMOVE_EVENTLOG:
		theApp.emuledlg->m_mainTabWnd.m_dlgDownload.m_dlgPeerLog.m_LogListCtrl.RemoveEvents();
		break;
	case WM_FILE_UPDATE_FILELOG:
		{
			aMessageParam param=CMessageLog::GetInstace()->GetMessage(wParam);
			if(param==NULL)
			{
				break;
			}

			CPartFile* file = (CPartFile*)param->wParam;

			if (file == NULL)
			{
				break;
			}

			int result = theApp.emuledlg->transferwnd->downloadlistctrl.FindFile(file);

			if (result != -1)
			{
				int iState = theApp.emuledlg->transferwnd->downloadlistctrl.GetItemState(result, LVIS_SELECTED | LVIS_FOCUSED);

				if (iState == (LVIS_SELECTED | LVIS_FOCUSED))
				{
					theApp.emuledlg->m_mainTabWnd.m_dlgDownload.m_dlgPeerLog.m_LogListCtrl.AddLog((CTraceEvent*)param->lParam);
				}
			}			
		}
		break;
	case WM_GET_PARTLIST:
		{
			transferwnd->downloadlistctrl.GetDisplayedFiles(reinterpret_cast<CArray<CPartFile*,CPartFile*>*>(lParam));
		}
		break;
	}
	return CTrayDialog::WindowProc(message, wParam, lParam);
}

#pragma warning(pop)

HWND WINAPI AfxHtmlHelp(HWND hWnd, LPCTSTR szHelpFilePath, UINT nCmd, DWORD_PTR dwData);

void CemuleDlg::HtmlHelp(DWORD_PTR dwData, UINT nCmd)
{
	CWinApp* pApp = AfxGetApp();
	ASSERT_VALID(pApp);
	ASSERT(pApp->m_pszHelpFilePath != NULL);
	// to call HtmlHelp the m_fUseHtmlHelp must be set in
	// the application's constructor
	ASSERT(pApp->m_eHelpType == afxHTMLHelp);

	CWaitCursor wait;

	PrepareForHelp();

	// need to use top level parent (for the case where m_hWnd is in DLL)
	CWnd* pWnd = GetTopLevelParent();

	TRACE(traceAppMsg, 0, _T("HtmlHelp: pszHelpFile = '%s', dwData: $%lx, fuCommand: %d.\n"), pApp->m_pszHelpFilePath, dwData, nCmd);

	bool bHelpError = false;
	CString strHelpError;
	int iTry = 0;
	while (iTry++ < 2)
	{
		if (!AfxHtmlHelp(pWnd->m_hWnd, pApp->m_pszHelpFilePath, nCmd, dwData))
		{
			bHelpError = true;
			strHelpError.LoadString(AFX_IDP_FAILED_TO_LAUNCH_HELP);

			typedef struct tagHH_LAST_ERROR
			{
				int      cbStruct;
				HRESULT  hr;
				BSTR     description;
			} HH_LAST_ERROR;
			HH_LAST_ERROR hhLastError = {0};
			hhLastError.cbStruct = sizeof hhLastError;
			HWND hwndResult = AfxHtmlHelp(pWnd->m_hWnd, NULL, HH_GET_LAST_ERROR, reinterpret_cast<DWORD>(&hhLastError));
			if (hwndResult != 0)
			{
				if (FAILED(hhLastError.hr))
				{
					if (hhLastError.description)
					{
						USES_CONVERSION;
						strHelpError = OLE2T(hhLastError.description);
						::SysFreeString(hhLastError.description);
					}
					if (   hhLastError.hr == 0x8004020A  /*no topics IDs available in Help file*/
						|| hhLastError.hr == 0x8004020B) /*requested Help topic ID not found*/
					{
						// try opening once again without help topic ID
						if (nCmd != HH_DISPLAY_TOC)
						{
							nCmd = HH_DISPLAY_TOC;
							dwData = 0;
							continue;
						}
					}
				}
			}
			break;
		}
		else
		{
			bHelpError = false;
			strHelpError.Empty();
			break;
		}
	}

	if (bHelpError)
	{
		if (AfxMessageBox(CString(pApp->m_pszHelpFilePath) + _T("\n\n") + strHelpError + _T("\n\n") + GetResString(IDS_ERR_NOHELP), MB_YESNO | MB_ICONERROR) == IDYES)
		{
			CmdFuncs::GotoGuide();
		}
	}
}

LRESULT CemuleDlg::OnPeerCacheResponse(WPARAM wParam, LPARAM lParam)
{
	return CGlobalVariable::m_pPeerCache->OnPeerCacheCheckResponse(wParam,lParam);
}

#if _ENABLE_NOUSE
void CemuleDlg::CreateToolbarCmdIconMap()
{
	m_mapTbarCmdToIcon.SetAt(TBBTN_CONNECT, _T("Connect"));
	m_mapTbarCmdToIcon.SetAt(TBBTN_KAD, _T("Kademlia"));
	m_mapTbarCmdToIcon.SetAt(TBBTN_SERVER, _T("Server"));
	m_mapTbarCmdToIcon.SetAt(TBBTN_TRANSFERS, _T("Transfer"));
	m_mapTbarCmdToIcon.SetAt(TBBTN_SEARCH, _T("Search"));
	m_mapTbarCmdToIcon.SetAt(TBBTN_SHARED, _T("SharedFiles"));
	m_mapTbarCmdToIcon.SetAt(TBBTN_MESSAGES, _T("Messages"));
	m_mapTbarCmdToIcon.SetAt(TBBTN_IRC, _T("IRC"));
	m_mapTbarCmdToIcon.SetAt(TBBTN_STATS, _T("Statistics"));
	m_mapTbarCmdToIcon.SetAt(TBBTN_OPTIONS, _T("Preferences"));
	m_mapTbarCmdToIcon.SetAt(TBBTN_TOOLS, _T("Tools"));
	m_mapTbarCmdToIcon.SetAt(TBBTN_HELP, _T("Help"));
#ifndef _DISABLE_WEBBROWSER
	m_mapTbarCmdToIcon.SetAt(TBBTN_WEBBROWSER, _T("Webbrowser")); //Added by thilon on 2006.08.01
#endif
}

LPCTSTR CemuleDlg::GetIconFromCmdId(UINT uId)
{
	LPCTSTR pszIconId = NULL;
	if (m_mapTbarCmdToIcon.Lookup(uId, pszIconId))
		return pszIconId;
	return NULL;
}
#endif

BOOL CemuleDlg::OnChevronPushed(UINT id, NMHDR* pNMHDR, LRESULT* plResult)
{
	return FALSE;
#if _ENABLE_NOUSE
	UNREFERENCED_PARAMETER(id);
	if (!thePrefs.GetUseReBarToolbar())
		return FALSE;

	NMREBARCHEVRON* pnmrc = (NMREBARCHEVRON*)pNMHDR;

	ASSERT( id == AFX_IDW_REBAR );
	ASSERT( pnmrc->uBand == 0 );
	ASSERT( pnmrc->wID == 0 );
	ASSERT( m_mapTbarCmdToIcon.GetSize() != 0 );

	// get visible area of rebar/toolbar
	CRect rcVisibleButtons;
	toolbar->GetClientRect(&rcVisibleButtons);

	// search the first toolbar button which is not fully visible
	int iButtons = toolbar->GetButtonCount();
	int i;
	for (i = 0; i < iButtons; i++)
	{
		CRect rcButton;
		toolbar->GetItemRect(i, &rcButton);

		CRect rcVisible;
		if (!rcVisible.IntersectRect(&rcVisibleButtons, &rcButton) || !EqualRect(rcButton, rcVisible))
			break;
	}

	// create menu for all toolbar buttons which are not (fully) visible
	BOOL bLastMenuItemIsSep = TRUE;
	CTitleMenu menu;
	menu.CreatePopupMenu();
	menu.AddMenuTitle(_T("eMule"), true);
	while (i < iButtons)
	{
		TCHAR szString[256];
		szString[0] = _T('\0');
		TBBUTTONINFO tbbi = {0};
		tbbi.cbSize = sizeof tbbi;
		tbbi.dwMask = TBIF_BYINDEX | TBIF_COMMAND | TBIF_STYLE | TBIF_STATE | TBIF_TEXT;
		tbbi.cchText = ARRSIZE(szString);
		tbbi.pszText = szString;
		if (toolbar->GetButtonInfo(i, &tbbi) != -1)
		{
			if (tbbi.fsStyle & TBSTYLE_SEP)
			{
				if (!bLastMenuItemIsSep)
					bLastMenuItemIsSep = menu.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
			}
			else
			{
				if (szString[0] != _T('\0') && menu.AppendMenu(MF_STRING, tbbi.idCommand, szString, GetIconFromCmdId(tbbi.idCommand)))
				{
					bLastMenuItemIsSep = FALSE;
					if (tbbi.fsState & TBSTATE_CHECKED)
						menu.CheckMenuItem(tbbi.idCommand, MF_BYCOMMAND | MF_CHECKED);
					if ((tbbi.fsState & TBSTATE_ENABLED) == 0)
						menu.EnableMenuItem(tbbi.idCommand, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
				}
			}
		}

		i++;
	}

	CPoint ptMenu(pnmrc->rc.left, pnmrc->rc.top);
	ClientToScreen(&ptMenu);
	ptMenu.y += rcVisibleButtons.Height();
	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, ptMenu.x, ptMenu.y, this);
	*plResult = 1;
	return FALSE;
#endif
}

bool CemuleDlg::IsPreferencesDlgOpen() const
{
	return (preferenceswnd->m_hWnd != NULL);
}

int CemuleDlg::ShowPreferences(UINT uStartPageID)
{
	if (IsPreferencesDlgOpen())
	{
		preferenceswnd->SetForegroundWindow();
		preferenceswnd->BringWindowToTop();
		return -1;
	}
	else
	{
		if (uStartPageID != (UINT)-1)
			preferenceswnd->SetStartPage(uStartPageID);
		return preferenceswnd->DoModal();
	}
}



//////////////////////////////////////////////////////////////////
// Webserver related

LRESULT CemuleDlg::OnWebAddDownloads(WPARAM wParam, LPARAM lParam)
{
	CString link=CString((TCHAR*)wParam);
	if (link.GetLength()==32 && link.Left(4).CompareNoCase(_T("ed2k"))!=0) {
		uchar fileid[16];
		DecodeBase16(link, link.GetLength(), fileid, ARRSIZE(fileid));
		CGlobalVariable::searchlist->AddFileToDownloadByHash(fileid,(uint8)lParam);

	} else
		CGlobalVariable::filemgr.NewDownloadFile((TCHAR*)wParam, _T(""), (int)lParam);
		//theApp.AddEd2kLinksToDownload((TCHAR*)wParam, (int)lParam);

	return 0;
}

#if _ENABLE_NOUSE
LRESULT CemuleDlg::OnAddRemoveFriend(WPARAM wParam, LPARAM lParam)
{
	if (lParam==0) { // remove
		theApp.friendlist->RemoveFriend((CFriend*)wParam);
	} else {		// add
		theApp.friendlist->AddFriend((CUpDownClient*)wParam);
	}

	return 0;
}
#endif

LRESULT CemuleDlg::OnWebSetCatPrio(WPARAM wParam, LPARAM lParam)
{
	CGlobalVariable::downloadqueue->SetCatPrio(wParam,(uint8)lParam);
	return 0;
}
LRESULT CemuleDlg::OnWebServerClearCompleted(WPARAM wParam, LPARAM lParam)
{
	if(!wParam)
	{
		int cat=(int)lParam;
		transferwnd->downloadlistctrl.ClearCompleted(cat);
	}
	else
	{
		uchar* pFileHash = reinterpret_cast<uchar*>(lParam);
		CKnownFile* file=CGlobalVariable::knownfiles->FindKnownFileByID(pFileHash);
		if (file)
			transferwnd->downloadlistctrl.RemoveFile((CPartFile*)file);
		delete[] pFileHash;
	}

	return 0;
}

LRESULT CemuleDlg::OnWebServerFileRename(WPARAM wParam, LPARAM lParam)
{
	CString sNewName = ((LPCTSTR)(lParam));

	((CPartFile*)wParam)->SetFileName(sNewName);
	((CPartFile*)wParam)->SavePartFile();
	((CPartFile*)wParam)->UpdateDisplayedInfo();
	sharedfileswnd->sharedfilesctrl.UpdateFile( (CKnownFile*)((CPartFile*)wParam));

	return 0;
}

LRESULT CemuleDlg::OnWebGUIInteraction(WPARAM wParam, LPARAM lParam) {

	switch (wParam) {
		case WEBGUIIA_UPDATEMYINFO:
			serverwnd->UpdateMyInfo();
			break;
		case WEBGUIIA_WINFUNC:{
			if (thePrefs.GetWebAdminAllowedHiLevFunc())
			{
				try {
					HANDLE hToken;
					TOKEN_PRIVILEGES tkp;	// Get a token for this process.
					
					if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
						throw;
					LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
					tkp.PrivilegeCount = 1;  // one privilege to set
					tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;	// Get the shutdown privilege for this process.
					AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);

					if (lParam==1) {	// shutdown
						ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE, 0);
					} else 
					if (lParam==2) {
						ExitWindowsEx(EWX_REBOOT | EWX_FORCE, 0);
					}

				} catch(...)
					{
						AddLogLine(true, GetResString(IDS_WEB_REBOOT) + _T(' ') + GetResString(IDS_FAILED));
				}
			}
			else 
				AddLogLine(true, GetResString(IDS_WEB_REBOOT) + _T(' ') + GetResString(IDS_ACCESSDENIED));
			break;
		}
		case WEBGUIIA_UPD_CATTABS:
			theApp.emuledlg->transferwnd->UpdateCatTabTitles();
			break;
		case WEBGUIIA_UPD_SFUPDATE: {
			CKnownFile* kf=(CKnownFile*)lParam;
			if (kf)
				CGlobalVariable::sharedfiles->UpdateFile(kf);
			}
			break;
		case WEBGUIIA_UPDATESERVER:
			serverwnd->serverlistctrl.RefreshServer((CServer*)lParam);
			break;
		case WEBGUIIA_STOPCONNECTING:
			CGlobalVariable::serverconnect->StopConnectionTry();
			break;
		case WEBGUIIA_CONNECTTOSERVER: {
			CServer* server=(CServer*)lParam;
			if (server==NULL) 
				CGlobalVariable::serverconnect->ConnectToAnyServer();
			else 
				CGlobalVariable::serverconnect->ConnectToServer(server);
			break;
			}
		case WEBGUIIA_DISCONNECT:
			if (lParam!=2)	// !KAD
				CGlobalVariable::serverconnect->Disconnect();
			if (lParam!=1)	// !ED2K
				Kademlia::CKademlia::Stop();
			break;

		case WEBGUIIA_SERVER_REMOVE: {
			serverwnd->serverlistctrl.RemoveServer((CServer*)lParam);
			break;
		}
		case WEBGUIIA_SHARED_FILES_RELOAD: {
			CGlobalVariable::sharedfiles->Reload();
			break;
		}
		case WEBGUIIA_ADD_TO_STATIC: {
			serverwnd->serverlistctrl.StaticServerFileAppend((CServer*)lParam);
			break;
		}
		case WEBGUIIA_REMOVE_FROM_STATIC: {
			serverwnd->serverlistctrl.StaticServerFileRemove((CServer*)lParam);
			break;
		}
		case WEBGUIIA_UPDATESERVERMETFROMURL:
			theApp.emuledlg->serverwnd->UpdateServerMetFromURL((TCHAR*)lParam);
			break;
		case WEBGUIIA_SHOWSTATISTICS:
			theApp.emuledlg->statisticswnd->ShowStatistics(lParam!=0);
			break;
		case WEBGUIIA_DELETEALLSEARCHES:
			theApp.emuledlg->searchwnd->DeleteAllSearches();
			break;

		case WEBGUIIA_KAD_BOOTSTRAP:{
			CString dest=CString((TCHAR*)lParam);
			int pos=dest.Find(_T(':'));
			if (pos!=-1) {
				uint16 port = (uint16)_tstoi(dest.Right(dest.GetLength() - pos - 1));
				CString ip = dest.Left(pos);
				// JOHNTODO - Switch between Kad1 and Kad2
				Kademlia::CKademlia::Bootstrap(ip, port, true);
			}
			break;
		}
		case WEBGUIIA_KAD_START:
			Kademlia::CKademlia::Start();
			break;
		case WEBGUIIA_KAD_STOP:
			Kademlia::CKademlia::Stop();
			break;
		case WEBGUIIA_KAD_RCFW:
			Kademlia::CKademlia::RecheckFirewalled();
			break;
	}

	return 0;
}


void CemuleDlg::TrayMinimizeToTrayChange()
{
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		if (!thePrefs.GetMinToTray())
		{
			// just for safety, ensure that we are not adding duplicate menu entries..
			if (pSysMenu->EnableMenuItem(MP_MINIMIZETOTRAY, MF_BYCOMMAND | MF_ENABLED) == -1)
			{
				ASSERT( (MP_MINIMIZETOTRAY & 0xFFF0) == MP_MINIMIZETOTRAY && MP_MINIMIZETOTRAY < 0xF000);
				VERIFY( pSysMenu->InsertMenu(SC_MINIMIZE, MF_BYCOMMAND, MP_MINIMIZETOTRAY, GetResString(IDS_PW_TRAY)) );
			}
			else
				ASSERT(0);
		}
		else
		{
			(void)pSysMenu->RemoveMenu(MP_MINIMIZETOTRAY, MF_BYCOMMAND);
		}
	}
	CTrayDialog::TrayMinimizeToTrayChange();
}


//Chocobo Start
//eMule自动更新，added by Chocobo on 2006.07.31
//自动更新后的响应
//added by thilon
void CemuleDlg::OnUpdateAvailable() // 发现新版本时弹出提示框
{
	Log(LOG_SUCCESS|LOG_STATUSBAR,GetResString(IDS_NEWVERSIONAVL));
	//ShowNotifier(GetResString(IDS_NEWVERSIONAVLPOPUP), TBN_NEWVERSION);
}

//added by thilon
void CemuleDlg::OnUpdateNotAvailable(void) //当前为最新版本，不需要下载
{
	Log(LOG_SUCCESS|LOG_STATUSBAR,GetResString(IDS_NONEWERVERSION));
	//ShowNotifier(GetResString(IDS_NONEWERVERSION), TBN_NEWVERSION);
}

//added by thilon
void CemuleDlg::OnUpdateNoCheck(void) //与服务器连接失败
{
	Log(LOG_SUCCESS|LOG_STATUSBAR,GetResString(IDS_NONEWERVERSION));
	//ShowNotifier(GetResString(IDS_NEWVERSIONFAILED), TBN_NEWVERSION);
}
//Chocobo End

//Added by thilon on 2006.10.27, for UPnP
LONG CemuleDlg::OnMappingFinished(WPARAM wParam,LPARAM lParam)
{   
 	if(!CGlobalVariable::serverlist->m_bHasInit)
 	{
       CMappingValue pValue;
	   pValue.m_wParam = wParam;
	   pValue.m_lParam = lParam;
	   m_MappingValueList.AddTail(pValue);
	   return 0;
 	}
    m_called_OnMappingFinished = true;
	HRESULT					hr = wParam;
	CUPnpAsynThreadsResult	*pResult = (CUPnpAsynThreadsResult*) lParam;
	if (NULL == pResult)
		return 0;

	if (pResult->bCleanedFillupBug)
	{
		//重设“是否已清除EntryFillupBug”标识	<begin>
		CIni ini(thePrefs.GetConfigFile());
		ini.WriteBool(_T("HasCleanedFillupBug"), true, _T("UPnP"));
		//重设“是否已清除EntryFillupBug”标识	<end>
	}
	
	CString		strErrorMessage;
	CString		strLogText;
	switch(pResult->dwCustomParam) 
	{
	case 1:			//TCP
		{
			if (NULL != CGlobalVariable::serverconnect)
			{
				if ( SUCCEEDED(hr) || CGlobalVariable::serverconnect->IsLowID() )		//ADDED by VC-fengwen on 2007/09/04 : 如果失败了，但已经是高Id了则不提示。
				{
					strErrorMessage = CUPnpMgr::Result2String(hr, pResult->dwActionErrorCode);
					strLogText.Format(GetResString(IDS_TCP_MAPPING),
						pResult->wInternalPort,
						pResult->wExternalPort,
						strErrorMessage);

					AddLogLine(false, strLogText);
				}


				if (SUCCEEDED(hr))
				{
					thePrefs.SetUPnPTCPExternal(pResult->wExternalPort);

					if (CGlobalVariable::serverconnect->IsLowID())
					{
						ASSERT( CGlobalVariable::listensocket->StartListening() );
						CloseConnection();
                        StartConnection();
					}
				}
			}
			break;
		}
	case 2:			//UDP
		{
			if ( SUCCEEDED(hr) || Kademlia::CKademlia::IsFirewalled() ) 	//ADDED by VC-fengwen on 2007/09/04 : 如果失败了，但已经是高Id了则不提示。
			{
				strErrorMessage = CUPnpMgr::Result2String(hr, pResult->dwActionErrorCode);
				strLogText.Format(GetResString(IDS_UDP_MAPPING),
									pResult->wInternalPort,
									pResult->wExternalPort,
									strErrorMessage);

				AddLogLine(false, strLogText);
			}

			
			if (SUCCEEDED(hr))
			{
				thePrefs.SetUPnPUDPExternal(pResult->wExternalPort);
			}

			break;
		}
	default:
		break;
	}


	if (NULL != pResult)
	{
		delete pResult;
		pResult = NULL;
	}

	return 0;
}

LRESULT CemuleDlg::OnPortChanged(WPARAM /*wParam*/,LPARAM /*lParam*/)
{
	CUPnpAsynThreads::CleanMapedPortQuickly(&(CGlobalVariable::m_upnpMgr));
	Sleep(100);
	AddNatPortMappingAsyn();

	return 0;
}

void CemuleDlg::AddNatPortMappingAsyn()
{
	if (!thePrefs.GetUPnPNat())
	{
		thePrefs.SetUPnPTCPExternal(thePrefs.GetPort());
		thePrefs.SetUPnPUDPExternal(thePrefs.GetUDPPort());
		return;
	}

	CString		strDescription;
	strDescription.Format(_T("[eMule %u.%u.%u]"),
							CGlobalVariable::m_nVersionMjr,
							CGlobalVariable::m_nVersionMin,
							CGlobalVariable::m_nVersionUpd);

	CIni ini(thePrefs.GetConfigFile());
	bool bHasCleanedFillupBug = ini.GetBool(_T("HasCleanedFillupBug"), false, _T("UPnP"));

	CUPnpNatMapping		mapping;
	
	//	添加TCP端口映射
	mapping.m_strProtocol = _T("TCP");
	mapping.m_wExternalPort = thePrefs.GetPort();
	mapping.m_wInternalPort = mapping.m_wExternalPort;
	mapping.m_strDescription = strDescription;

	CUPnpAsynThreads::AddNatPortMappingAsyn(&(CGlobalVariable::m_upnpMgr),
											mapping,
											GetSafeHwnd(),
											UM_USER_MAPPING_FINISHED,
											thePrefs.GetUPnPNatTryRandom(),
											1,											//	1表示添加TCP端口，以在收到“添加完成到消息”时，作区别。
											bHasCleanedFillupBug);						//	如果没有清除Entryfillup的bug，则在这里清除一次。在收到完成的消息里，重设标识。
	
	//	添加UDP端口映射
	mapping.m_strProtocol = _T("UDP");
	mapping.m_wExternalPort = thePrefs.GetUDPPort();
	mapping.m_wInternalPort = mapping.m_wExternalPort;
	mapping.m_strDescription = strDescription;

	CUPnpAsynThreads::AddNatPortMappingAsyn(&(CGlobalVariable::m_upnpMgr),
											mapping,
											GetSafeHwnd(),
											UM_USER_MAPPING_FINISHED,
											thePrefs.GetUPnPNatTryRandom(),
											2);	//	2表示添加UDP端口，以在收到“添加完成到消息”时，作区别。



}

LRESULT CemuleDlg::OnCheckUpdateFinished(WPARAM wParam, LPARAM  lParam)
{
	if( wParam==1 )
	{
		if( m_bManualUpdte && lParam==ERROR_NONEWVERSION ) //there is no update
		{
			ShowNotifier(GetResString(IDS_LATEST_VERSION),TBN_IMPORTANTEVENT); //TODO
		}
	}
	//else //check udpate failed, 

	return S_OK;
}
LRESULT CemuleDlg::OnStartED2KUpdate(WPARAM/* wParam*/, LPARAM/* lParam*/)
{
	//获取ED2K链接
	if(m_hMapping == NULL)
	{
		return S_FALSE;
	}

	m_lpData = (LPSTR)MapViewOfFile(m_hMapping, FILE_MAP_ALL_ACCESS, 0, 0, BUF_SIZE);
	if(m_lpData == NULL)
	{   
		return S_FALSE;
	}

	LPSTR lpLink = m_lpData,lpForce = NULL;
	CString tcsForce = _T("");
	BOOL bForce = FALSE;

	if (BUF_SIZE > strlen(lpLink) + 1)
	{
		lpForce = m_lpData + strlen(lpLink) + 1;
		tcsForce = lpForce;
	}

	if (!tcsForce.IsEmpty())
	{
		CString sMyModVersion;
		sMyModVersion.Format(_T("0%u"),VC_VERSION_BUILD);

		if (tcsForce.Find(sMyModVersion) != -1)
			bForce = TRUE;
	}
	thePrefs.m_bForceUpdate = bForce;

	CString strLinks(m_lpData);
	UnmapViewOfFile(m_lpData);
	CUpdateInfo updateinfo;

	//做下载判断
	try
	{
		CED2KLink* pLink = CED2KLink::CreateLinkFromUrl(strLinks.Trim());

		if (!pLink)
		{
			return S_FALSE;	//如果ED2K有缺陷，直接返回
		}

		const uchar* hash = pLink->GetFileLink()->GetHashKey();	//更新服务器上“最新的更新文件”的Hash值

		if(updateinfo.Compare(md4str(hash)) != 0)
		{
			updateinfo.DeleteUpdate(updateinfo.GetUpdateHash()); //[VC-Huby-080311]:应该把旧的hash更新删除,不是删除最新hash值的文件
		}

		int istate = updateinfo.GetUpdateState(hash);//获取Update文件的状态

		int ret = IDYES;
		switch(istate)
		{
		case UPDATE_NODOWNLOAD:		//没有下载

			ret = IDYES;
			if (!bForce)
			{
				ret = MessageBox(GetResString(IDS_CAPTION) + GetResString(IDS_UPDATE_NEWVERSION), GetResString(IDS_CAPTION), MB_YESNO | MB_ICONQUESTION);
			}

			if(IDYES == ret)
			{
				updateinfo.SetUpdateHash(md4str(hash));
				theApp.AddED2KUpdateToDownload(strLinks);
			}
			break;

		case UPDATE_DOWNLOADING:	//下载中
			break;

		case UPDATE_DOWNLOADED:		//已下载, 没有安装
			//{begin} dgkang 传递参数

			ret = IDYES;
			if (!bForce)
				ret = MessageBox(GetResString(IDS_UPDATE_INSTALL), GetResString(IDS_CAPTION), MB_YESNO | MB_ICONQUESTION);

			if(IDYES == ret)
			{
				TCHAR szFileName[MAX_PATH] = {0};
				::GetModuleFileName(NULL,szFileName,MAX_PATH);
				CString tcsFileName = szFileName;
				int nPos = tcsFileName.ReverseFind('\\');
				CString tcsAppDir = _T("");
				if (nPos != -1)
				{
					tcsFileName = tcsFileName.Left(nPos);
					tcsAppDir.Format(_T("/EMULEPATH=\"%s\" /FORCEUPDATE=%d"),tcsFileName,bForce);
				}
				ShellExecute(NULL , _T("open"), CGlobalVariable::sharedfiles->GetFileByID(pLink->GetFileLink()->GetHashKey())->GetFilePath(),tcsAppDir,NULL,SW_SHOWNORMAL);
			}
			break;
		default:
			ASSERT(0);
		}
		
		delete pLink;

	}
	catch (CString error)
	{
		CString strBuffer;
		strBuffer.Format(GetResString(IDS_ERR_INVALIDLINK),error);
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_LINKERROR), strBuffer);
		return S_FALSE;
	}

	return S_OK;
}

LRESULT CemuleDlg::OnED2KUpdateComplete(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	return S_OK;
}

// VC-SearchDream[2007-05-24]: Player Request End

LRESULT CemuleDlg::OnNatDisconnect(WPARAM wParam, LPARAM lParam)
{
	try {
		if(wParam==0)
		{
			CClientReqSocket * pClientSock=(CClientReqSocket *)lParam;
			pClientSock->Disconnect(_T(""));
		}
		else if(wParam==1)
		{
			CUpDownClient * client = (CUpDownClient*)lParam;
			client->Disconnected(_T(""), true);
		}
	}
	catch(...){
	}
	return 0;
}

bool CemuleDlg::ProcessMessage(UINT /*msg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, LRESULT & /*lResult*/)
{
	//switch(msg)
	//{
	//case WM_SERVER_ADD_SVR:
	//	//serverwnd->serverlistctrl.AddServer(addsrv, true));
	//	break;			
	//}
	return false;
}
BOOL CemuleDlg::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CRect	rtClient;
	GetClientRect(&rtClient);
	pDC->FillSolidRect(&rtClient, RGB(223, 223, 223));

	//return m_Anchor.EraseBackground(pDC->GetSafeHdc());
	return TRUE;
}

void CemuleDlg::OnMeasureItem(int /*nIDCtl*/, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	MenuXP.MeasureItem(lpMeasureItemStruct);
}

void CemuleDlg::OnDrawItem(int /*nIDCtl*/, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	MenuXP.DrawItem(lpDrawItemStruct);
}

void CemuleDlg::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
	CTrayDialog::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);

	UINT nID = pPopupMenu->GetMenuItemID( 0 );
	if ( nID != SC_RESTORE )
	{
		MenuXP.AddMenu( pPopupMenu, TRUE );
	}
}

void CemuleDlg::InitMenuIcon(void)
{
	//初始化图形菜单图标
	cif.AddIcon(IDI_CHECKMARK,theApp.LoadIcon(_T("CHECKMARK")));

	//下载文件列表
	cif.AddIcon(MP_NEW, theApp.LoadIcon(_T("NEWTASK")));
	cif.AddIcon(MP_PAUSE, theApp.LoadIcon(_T("PAUSE")));
	cif.AddIcon(MP_STOP, theApp.LoadIcon(_T("STOP")));
	cif.AddIcon(MP_RESUME, theApp.LoadIcon(_T("RESUME")));

	cif.AddIcon(MP_CANCEL, theApp.LoadIcon(_T("CANCEL")));
	cif.AddIcon(MP_PREVIEW, theApp.LoadIcon(_T("PREVIEW")));
	cif.AddIcon(MP_METINFO, theApp.LoadIcon(_T("FILEINFO")));

	cif.AddIcon(MP_VIEWFILECOMMENTS, theApp.LoadIcon(_T("FILECOMMENTS")));
	cif.AddIcon(MP_CLEARCOMPLETED, theApp.LoadIcon(_T("CLEARCOMPLETE")));
	cif.AddIcon(MP_GETED2KLINK, theApp.LoadIcon(_T("ED2KLINK")));
	cif.AddIcon(MP_SHOWED2KLINK, theApp.LoadIcon(_T("ED2KLINK")));
	cif.AddIcon(MP_PASTE, theApp.LoadIcon(_T("PASTELINK")));
	cif.AddIcon(MP_FIND, theApp.LoadIcon(_T("Search")));
	cif.AddIcon(MP_SEARCHRELATED, theApp.LoadIcon(_T("KadFileSearch")));

	cif.AddIcon(MP_DETAIL, theApp.LoadIcon(_T("CLIENTDETAILS")));
	cif.AddIcon(MP_ADDFRIEND, theApp.LoadIcon(_T("ADDFRIEND")));
	cif.AddIcon(MP_MESSAGE, theApp.LoadIcon(_T("SENDMESSAGE")));
	cif.AddIcon(MP_SHOWLIST, theApp.LoadIcon(_T("VIEWFILES")));
	cif.AddIcon(MP_FIND, theApp.LoadIcon(_T("Search")));

	cif.AddIcon(MP_PLAY, theApp.LoadIcon(_T("PLAY")));

	//共享文件列表
	cif.AddIcon(MP_OPEN, theApp.LoadIcon(_T("OPENFILE")));
	cif.AddIcon(MP_OPENFOLDER, theApp.LoadIcon(_T("OPENFOLDER")));
	cif.AddIcon(MP_RENAME, theApp.LoadIcon(_T("RENAME")));
	cif.AddIcon(MP_CANCEL, theApp.LoadIcon(_T("DELETE")));
	cif.AddIcon(MP_REMOVE, theApp.LoadIcon(_T("DELETE")));
	cif.AddIcon(MP_REFRESH, theApp.LoadIcon(_T("RELOAD")));

	cif.AddIcon(MP_CREATECOLLECTION, theApp.LoadIcon(_T("COLLECTION_ADD")));
	cif.AddIcon(MP_MODIFYCOLLECTION, theApp.LoadIcon(_T("COLLECTION_EDIT")));
	cif.AddIcon(MP_VIEWCOLLECTION, theApp.LoadIcon(_T("COLLECTION_VIEW")));
	cif.AddIcon(MP_SEARCHAUTHOR, theApp.LoadIcon(_T("COLLECTION_SEARCH")));

	cif.AddIcon(MP_DETAIL, theApp.LoadIcon(_T("FILEINFO")));
	cif.AddIcon(MP_CMT, theApp.LoadIcon(_T("FILECOMMENTS")));
	cif.AddIcon(MP_GETED2KLINK, theApp.LoadIcon(_T("ED2KLINK")));
	cif.AddIcon(MP_VIRUS, theApp.LoadIcon(_T("SHIELD")));

	//服务器
	cif.AddIcon(MP_CONNECTTO, theApp.LoadIcon(_T("CONNECT")));
	cif.AddIcon(MP_ADDTOSTATIC, theApp.LoadIcon(_T("ListAdd")));
	cif.AddIcon(MP_REMOVEFROMSTATIC, theApp.LoadIcon(_T("ListRemove")));
	cif.AddIcon(MP_REMOVEALL, theApp.LoadIcon(_T("DELETE")));


	//灰度图标
	//初始化图形菜单图标
	cif.AddIcon(IDI_CHECKMARK,theApp.LoadIcon(_T("CHECKMARK")), TRUE);
	//下载文件列表
	cif.AddIcon(MP_NEW, theApp.LoadIcon(_T("NEWTASK")), TRUE);
	cif.AddIcon(MP_PAUSE, theApp.LoadIcon(_T("PAUSE")), TRUE);
	cif.AddIcon(MP_STOP, theApp.LoadIcon(_T("STOP")), TRUE);
	cif.AddIcon(MP_RESUME, theApp.LoadIcon(_T("RESUME")), TRUE);

	cif.AddIcon(MP_CANCEL, theApp.LoadIcon(_T("CANCEL")), TRUE);
	cif.AddIcon(MP_PREVIEW, theApp.LoadIcon(_T("PREVIEW")), TRUE);
	cif.AddIcon(MP_METINFO, theApp.LoadIcon(_T("FILEINFO")), TRUE);

	cif.AddIcon(MP_VIEWFILECOMMENTS, theApp.LoadIcon(_T("FILECOMMENTS")), TRUE);
	cif.AddIcon(MP_CLEARCOMPLETED, theApp.LoadIcon(_T("CLEARCOMPLETE")), TRUE);
	cif.AddIcon(MP_GETED2KLINK, theApp.LoadIcon(_T("ED2KLINK")), TRUE);
	cif.AddIcon(MP_SHOWED2KLINK, theApp.LoadIcon(_T("ED2KLINK")), TRUE);
	cif.AddIcon(MP_PASTE, theApp.LoadIcon(_T("PASTELINK")), TRUE);
	cif.AddIcon(MP_FIND, theApp.LoadIcon(_T("Search")), TRUE);
	cif.AddIcon(MP_SEARCHRELATED, theApp.LoadIcon(_T("KadFileSearch")), TRUE);

	cif.AddIcon(MP_DETAIL, theApp.LoadIcon(_T("CLIENTDETAILS")), TRUE);
	cif.AddIcon(MP_ADDFRIEND, theApp.LoadIcon(_T("ADDFRIEND")), TRUE);
	cif.AddIcon(MP_MESSAGE, theApp.LoadIcon(_T("SENDMESSAGE")), TRUE);
	cif.AddIcon(MP_SHOWLIST, theApp.LoadIcon(_T("VIEWFILES")), TRUE);
	cif.AddIcon(MP_FIND, theApp.LoadIcon(_T("Search")), TRUE);

	cif.AddIcon(MP_PLAY, theApp.LoadIcon(_T("PLAY")), TRUE);

	//共享文件列表
	cif.AddIcon(MP_OPEN, theApp.LoadIcon(_T("OPENFILE")), TRUE);
	cif.AddIcon(MP_OPENFOLDER, theApp.LoadIcon(_T("OPENFOLDER")), TRUE);
	cif.AddIcon(MP_RENAME, theApp.LoadIcon(_T("RENAME")), TRUE);
	cif.AddIcon(MP_CANCEL, theApp.LoadIcon(_T("DELETE")), TRUE);
	cif.AddIcon(MP_REMOVE, theApp.LoadIcon(_T("DELETE")), TRUE);

	cif.AddIcon(MP_CREATECOLLECTION, theApp.LoadIcon(_T("COLLECTION_ADD")), TRUE);
	cif.AddIcon(MP_MODIFYCOLLECTION, theApp.LoadIcon(_T("COLLECTION_EDIT")), TRUE);
	cif.AddIcon(MP_VIEWCOLLECTION, theApp.LoadIcon(_T("COLLECTION_VIEW")), TRUE);
	cif.AddIcon(MP_SEARCHAUTHOR, theApp.LoadIcon(_T("COLLECTION_SEARCH")), TRUE);

	cif.AddIcon(MP_DETAIL, theApp.LoadIcon(_T("FILEINFO")), TRUE);
	cif.AddIcon(MP_CMT, theApp.LoadIcon(_T("FILECOMMENTS")), TRUE);
	cif.AddIcon(MP_GETED2KLINK, theApp.LoadIcon(_T("ED2KLINK")), TRUE);
	cif.AddIcon(MP_VIRUS, theApp.LoadIcon(_T("SHIELD")), TRUE);

	//服务器
	cif.AddIcon(MP_CONNECTTO, theApp.LoadIcon(_T("CONNECT")), TRUE);
	cif.AddIcon(MP_ADDTOSTATIC, theApp.LoadIcon(_T("ListAdd")), TRUE);
	cif.AddIcon(MP_REMOVEFROMSTATIC, theApp.LoadIcon(_T("ListRemove")), TRUE);
	cif.AddIcon(MP_REMOVEALL, theApp.LoadIcon(_T("DELETE")), TRUE);

	//搜索框
	cif.AddIcon(MP_SEARCHVERYCD,theApp.LoadIcon(IDI_ICON_VERYCD));
	cif.AddIcon(MP_SEARCHEMULE,theApp.LoadIcon(IDI_ICON_EMULE));
}

void CemuleDlg::RemoveOldUpdateFile(CString updatehash)
{
	uchar UpdateHash[16];
	CPartFile* pPartFile;

	try
	{
		if(!strmd4(updatehash,UpdateHash))
		{
			return;
		}

		CKnownFile* file = CGlobalVariable::sharedfiles->GetFileByID(UpdateHash);

		//共享列表中是否有
		if(file)
		{
			if(file->IsPartFile())
			{
				//共享列表有，但未下载完，移除
				pPartFile = DYNAMIC_DOWNCAST(CPartFile,file);
				if( pPartFile )
					pPartFile->DeleteFile();
			}
			else
			{
				//共享列表有，已经下载完成未安装，移除
				DeleteFile(file->GetFilePath());
				theApp.emuledlg->sharedfileswnd->sharedfilesctrl.RemoveFile(file);
				CGlobalVariable::sharedfiles->RemoveFile(file);
			}
		}
		else
		{
			if ((pPartFile = CGlobalVariable::downloadqueue->GetFileByID(UpdateHash)) != NULL)
			{
				//共享列表没有，但未下载完，移除
				pPartFile->DeleteFile();
			}
		}
	}
	catch (...)
	{

	}
}

// VC-kernel[2007-05-14]:hotkey
LRESULT CemuleDlg::OnHotKey(WPARAM/* wParam*/,LPARAM/* lParam*/)
{
	if(m_boss)
	{	
		//BossCome();
		TrayHide();
		
		ShowWindow(SW_HIDE);
	}
	else
	{	
		//BossLeave();
		TrayShow();
		ShowWindow(SW_SHOW);
	}
	m_boss = !m_boss;
	
	return NULL;
}

// VC-SearchDream[2007-05-24]: Player Request Begin
int CemuleDlg::ProcessPlayerRequest(PLAYER_DATA_REQ * req)
{
	CFile file;
	byte* filedata = 0;
	CString fullname;
	CSyncHelper lockFile;

	try
	{
		CKnownFile* srcfile = CGlobalVariable::sharedfiles->GetFileByID(req->filehash);

		if (!srcfile)
		{
			srcfile = CGlobalVariable::downloadqueue->GetFileByID(req->filehash);

			if (!srcfile)
			{
				return 0;
			}
		}

		if (srcfile->IsPartFile() && ((CPartFile*)srcfile)->GetStatus() != PS_COMPLETE)
		{
			// Do not access a part file, if it is currently moved into the incoming directory.
			// Because the moving of part file into the incoming directory may take a noticable
			// amount of time, we can not wait for 'm_FileCompleteMutex' and block the main thread.
			if (!((CPartFile*)srcfile)->m_FileCompleteMutex.Lock(0))
			{ 
				// just do a quick test of the mutex's state and return if it's locked.
				return 0;
			}

			lockFile.m_pObject = &((CPartFile*)srcfile)->m_FileCompleteMutex;
			// If it's a part file which we are uploading the file remains locked until we've read the
			// current block. This way the file completion thread can not (try to) "move" the file into
			// the incoming directory.

			fullname = RemoveFileExtension(((CPartFile*)srcfile)->GetFullName());
		}
		else
		{
			fullname.Format(_T("%s\\%s"),srcfile->GetPath(),srcfile->GetFileName());
		}

		uint32 togo = (uint32)req->len;
		uint64 end  = req->pos + req->len - 1;

#ifdef _SUPPORT_MEMPOOL
		if (srcfile->IsPartFile() && !((CPartFile*)srcfile)->IsCompleteforPlayer(req->pos, end, false))
#else
		if (srcfile->IsPartFile() && !((CPartFile*)srcfile)->IsCompleteforPlayer(req->pos, end, true))
#endif	
		{
			if (end > req->pos)
			{
				togo = (uint32)(end - req->pos + 1);
			}
			else
			{
				return 0;
			}
		}

		if (!srcfile->IsPartFile())
		{
			// This is not a part file...
			if (!file.Open(fullname,CFile::modeRead|CFile::osSequentialScan|CFile::shareDenyNone))
			{
				throw GetResString(IDS_ERR_OPEN);
			}

			file.Seek(req->pos, 0);

			filedata = new byte[togo];
			if (uint32 done = file.Read(filedata,togo) != togo)
			{
				throw CString(_T("can't get intact data."));
			}

			file.Close();
		}
		else
		{
			CPartFile* partfile = (CPartFile*)srcfile;
			
			filedata = new byte[togo];

#ifdef _SUPPORT_MEMPOOL
			if (!partfile->GetDataFromBufferThenDisk(filedata, req->pos, req->pos + togo - 1))
			{
				throw CString(_T("can't get intact data."));
			}
#else
			partfile->m_hpartfile.Seek(req->pos, 0);

			if (uint32 done = partfile->m_hpartfile.Read(filedata,togo) != togo)
			{
				throw CString(_T("can't get intact data."));
			}
#endif
		}

		if (lockFile.m_pObject)
		{
			lockFile.m_pObject->Unlock(); // Unlock the (part) file as soon as we are done with accessing it.
			lockFile.m_pObject = NULL;
		}

		CPlayerMgr::PlayerDataArrived(req, filedata, togo);

		delete[] filedata;
		filedata = 0;
		return togo;
	}
	catch (CString error)
	{
		if (filedata) 
		{
			delete[] filedata;
		}

		return -1;
	}
	catch (CFileException* e)
	{
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		e->GetErrorMessage(szError, ARRSIZE(szError));

		if (filedata) 
		{
			delete[] filedata;
		}

		e->Delete();
		return -1;
	}
}

LRESULT CemuleDlg::OnPlayerRequestData(WPARAM /*wParam*/, LPARAM lParam)
{
	PLAYER_DATA_REQ * req = (PLAYER_DATA_REQ *)lParam;

	if (ProcessPlayerRequest(req) <= 0)
	{
		CPlayerMgr::PlayerDataArrived(req, NULL, 0);
	}

	delete req;

	return 0;
}
// VC-SearchDream[2007-05-24]: Player Request End
void CemuleDlg::CloseToTray(void)
{
	if (TrayIsVisible())
	{
		ShowWindow(SW_HIDE);
	}
}

void CemuleDlg::CloseImmeditely(void)
{
	/*if (!CanClose())
		return;*/

	//Chocobo Start
	//eMule自动更新，added by Chocobo on 2006.07.31
	//关闭eMuel前取消正在进行的自动更新
	//if(m_pEmuleUpdater != NULL)
	//{
	//	m_pEmuleUpdater->CancelUpdater();
	//}
	//Chocobo End

	Log(_T("Closing eMule"));
	// {begin} Hide window right after close app by Soar Chin 11/07/2007
#ifndef _DEBUG
	ShowWindow(SW_HIDE);
	CTrayDialog::TrayHide();
#endif
	// {end}   Hide window right after close app by Soar Chin 11/07/2007
	CloseTTS();
	m_pDropTarget->Revoke();
	CGlobalVariable::m_app_state = APP_STATE_SHUTTINGDOWN;
	CGlobalVariable::serverconnect->Disconnect();
	theApp.OnlineSig(); // Added By Bouc7 

	// get main window placement
	WINDOWPLACEMENT wp;
	wp.length = sizeof(wp);
	GetWindowPlacement(&wp);
	ASSERT( wp.showCmd == SW_SHOWMAXIMIZED || wp.showCmd == SW_SHOWMINIMIZED || wp.showCmd == SW_SHOWNORMAL );
	if (wp.showCmd == SW_SHOWMINIMIZED && (wp.flags & WPF_RESTORETOMAXIMIZED))
		wp.showCmd = SW_SHOWMAXIMIZED;
	wp.flags = 0;
	thePrefs.SetWindowLayout(wp);

	// get active main window dialog
	if (activewnd){
		if (activewnd->IsKindOf(RUNTIME_CLASS(CServerWnd)))
			thePrefs.SetLastMainWndDlgID(IDD_SERVER);
		else if (activewnd->IsKindOf(RUNTIME_CLASS(CSharedFilesWnd)))
			thePrefs.SetLastMainWndDlgID(IDD_FILES);
		else if (activewnd->IsKindOf(RUNTIME_CLASS(CSearchDlg)))
			thePrefs.SetLastMainWndDlgID(IDD_SEARCH);
		else if (activewnd->IsKindOf(RUNTIME_CLASS(CTransferWnd)))
			thePrefs.SetLastMainWndDlgID(IDD_TRANSFER);
		else if (activewnd->IsKindOf(RUNTIME_CLASS(CStatisticsDlg)))
			thePrefs.SetLastMainWndDlgID(IDD_STATISTICS);
		else if (activewnd->IsKindOf(RUNTIME_CLASS(CKademliaWnd)))
			thePrefs.SetLastMainWndDlgID(IDD_KADEMLIAWND);
#if _ENABLE_NOUSE
		else if (activewnd->IsKindOf(RUNTIME_CLASS(CChatWnd)))
			thePrefs.SetLastMainWndDlgID(IDD_CHAT);
		else if (activewnd->IsKindOf(RUNTIME_CLASS(CIrcWnd)))
			thePrefs.SetLastMainWndDlgID(IDD_IRC);
#endif
		else if (activewnd->IsKindOf(RUNTIME_CLASS(CWebBrowserWnd))) // Added by thilon on 2006.08.01
			thePrefs.SetLastMainWndDlgID(IDD_WEBBROWSER);
		else{
			ASSERT(0);
			thePrefs.SetLastMainWndDlgID(0);
		}
	}

	CUPnpAsynThreads::CleanMapedPortQuickly(&(CGlobalVariable::m_upnpMgr)); //Added by thilon on 2006.11.01

	Kademlia::CKademlia::Stop();	// couple of data files are written

	// try to wait untill the hashing thread notices that we are shutting down
	CSingleLock sLock1(&theApp.hashing_mut); // only one filehash at a time
	sLock1.Lock(2000);

	// saving data & stuff
	// VC-kernel[2007-02-05]:to be delete
	//theApp.emuledlg->preferenceswnd->m_wndSecurity.DeleteDDB();

	CGlobalVariable::knownfiles->Save();										// CKnownFileList::Save
	//transferwnd->downloadlistctrl.SaveSettings();
	//transferwnd->downloadclientsctrl.SaveSettings();
	//transferwnd->uploadlistctrl.SaveSettings();
	//transferwnd->queuelistctrl.SaveSettings();
	//transferwnd->clientlistctrl.SaveSettings();
	//sharedfileswnd->sharedfilesctrl.SaveSettings();
	//chatwnd->m_FriendListCtrl.SaveSettings();
	searchwnd->SaveAllSettings();
	serverwnd->SaveAllSettings();
	kademliawnd->SaveAllSettings();
	CDlgAddTask::FreeInstance();

	CGlobalVariable::m_pPeerCache->Save();
	theApp.scheduler->RestoreOriginals();
	thePrefs.Save();
	thePerfLog.Shutdown();

	// explicitly delete all listview items which may hold ptrs to objects which will get deleted
	// by the dtors (some lines below) to avoid potential problems during application shutdown.
	transferwnd->downloadlistctrl.DeleteAllItems();
#if _ENABLE_NOUSE
	chatwnd->chatselector.DeleteAllItems();
#endif
	CGlobalVariable::clientlist->DeleteAll();
	searchwnd->DeleteAllSearchListCtrlItems();
	sharedfileswnd->sharedfilesctrl.DeleteAllItems();
	sharedfileswnd->m_UpLoading.uploadlistctrl.DeleteAllItems();
	transferwnd->queuelistctrl.DeleteAllItems();
	transferwnd->clientlistctrl.DeleteAllItems();
	transferwnd->uploadlistctrl.DeleteAllItems();
	serverwnd->serverlistctrl.DeleteAllItems();

	CPartFileConvert::CloseGUI();
	CPartFileConvert::RemoveAllJobs();

	CGlobalVariable::sharedfiles->DeletePartFileInstances();

	searchwnd->SendMessage(WM_CLOSE);

	theThreadsMgr.CleanAllThreads();

	CGlobalVariable::Clearup();
	// NOTE: Do not move those dtors into 'CemuleApp::InitInstance' (althought they should be there). The
	// dtors are indirectly calling functions which access several windows which would not be available 
	// after we have closed the main window -> crash!

#if _ENABLE_NOUSE
	delete theApp.friendlist;		theApp.friendlist = NULL;		// CFriendList::SaveList
#endif
	delete theApp.scheduler;		theApp.scheduler = NULL;
	delete theApp.m_pFirewallOpener;theApp.m_pFirewallOpener = NULL;

	//EastShare Start - added by AndCycle, IP to Country

	//EastShare End   - added by AndCycle, IP to Country

	//	delete theApp.uploadBandwidthThrottler; theApp.uploadBandwidthThrottler = NULL;
	//	delete theApp.lastCommonRouteFinder; theApp.lastCommonRouteFinder = NULL;

#ifdef _SUPPORT_MEMPOOL
	// Added by SearchDream@2006/01/05
	delete theApp.m_pMemoryPool;
	theApp.m_pMemoryPool = NULL;
#endif

	thePrefs.Uninit();
	CGlobalVariable::m_app_state = APP_STATE_DONE;
	CTrayDialog::OnCancel();
	AddDebugLogLine(DLP_VERYLOW, _T("Closed eMule"));
}

void CemuleDlg::TriggerConnectingTray(BOOL bStart)
{
	if (bStart)
	{
		if (0 == m_dwTimer_ConnectingTray)
			m_dwTimer_ConnectingTray = SetTimer(TIMER_CONNECTING_TRAY, 500, NULL);
	}
	else
	{
		if (0 != m_dwTimer_ConnectingTray)
		{
			KillTimer(m_dwTimer_ConnectingTray);
			m_dwTimer_ConnectingTray = 0;
		}
	}
}

void CemuleDlg::OnTimer(UINT uIDEvent)
{
	if (m_dwTimer_ConnectingTray == uIDEvent)
	{
		UpdateTrayIcon(0);
	}
	if (uIDEvent == TIMER_ID)
	{
		CGlobalVariable::filemgr.OnTimer(TIMER_ID);
	}
	
	return CTrayDialog::OnTimer(uIDEvent);
}

void CemuleDlg::InitShortcutManager()
{
	m_mgrShortcuts.AddShortcut(MP_HM_HELP, VK_F1, 0);		//F1 help
	m_mgrShortcuts.AddShortcut(MP_NEW, 'N', HOTKEYF_CONTROL);	// Ctrl + N 新建任务对话框
	m_mgrShortcuts.AddShortcut(MP_HM_PREFS, 'O', HOTKEYF_CONTROL);	//Ctrl + O 选项对话框

	m_mgrShortcuts.AddShortcut(MP_SEARCHTYPE, VK_F3, HOTKEYF_SHIFT);	//Shift + F3 SearchType
	m_mgrShortcuts.AddShortcut(MP_SEARCH, VK_F3, 0);	//F3
	
	m_mgrShortcuts.AddShortcut(MP_SEARCH +  1,'R',HOTKEYF_CONTROL);
	m_mgrShortcuts.AddShortcut(MP_SEARCH +  2,'D',HOTKEYF_CONTROL);
	m_mgrShortcuts.AddShortcut(MP_SEARCH +  3,'S',HOTKEYF_CONTROL);
	m_mgrShortcuts.AddShortcut(MP_SEARCH +  4,'G',HOTKEYF_CONTROL);

	m_mgrShortcuts.Initialize(this);
	/*if (m_mgrShortcuts.Initialize(this))
	{
		if (m_mgrShortcuts.GetShortcut(IDCLOSE) == VK_ESCAPE)
		{
			m_mgrShortcuts.DeleteShortcut(IDCLOSE);
		}

		if (m_mgrShortcuts.GetShortcut(ID_EDIT_PASTE))
		{
			m_mgrShortcuts.DeleteShortcut(ID_EDIT_PASTE);

			if (!m_mgrShortcuts.GetShortcut(ID_EDIT_PASTESUB))
			{
				m_mgrShortcuts.AddShortcut(ID_EDIT_PASTESUB, 'V', HOTKEYF_CONTROL);
			}
		}
	}*/
}

LRESULT CemuleDlg::OnNatCheckStrageties(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if (NULL != CGlobalVariable::natthread)
		CGlobalVariable::natthread->CheckStrategies();

	return 0;
}

LRESULT CemuleDlg::OnRemoveFromAllQ_In_Throttler(WPARAM /*wParam*/, LPARAM lParam)
{
	if (NULL != CGlobalVariable::uploadBandwidthThrottler)
		CGlobalVariable::uploadBandwidthThrottler->RemoveFromAllQueues((CClientReqSocket*)lParam);
	return 0;
}
