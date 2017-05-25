/* 
 * $Id: emuleDlg.h 11398 2009-03-17 11:00:27Z huby $
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

#pragma once
#include "TrayDialog.h"
#include "MeterIcon.h"
#include "TitleMenu.h"

#include "PlayerMgr.h" // VC-SearchDream[2007-05-24]: PlayerMgr
#include "MenuXP.h"	//Added by thilon on 2007.02.12, for XP Style
#include "ShortcutManager.h"

//Chocobo Start
//eMule自动更新，added by Chocobo on 2006.07.31
#include "EmuleUpdater.h" // added by thilon 2006.02.23
//Chocobo End
#include "UpdaterThread.h"
#include "MainTabWnd.h"

namespace Kademlia {
	class CSearch;
	class CContact;
	class CEntry;
	class CUInt128;
};

class CChatWnd;
class CIrcWnd;
class CKademliaWnd;
class CKnownFileList; 
class CMainFrameDropTarget;
class CMuleStatusBarCtrl;
//class CMuleToolbarCtrl;
class CPreferencesDlg;
class CSearchDlg;
class CServerWnd;
class CSharedFilesWnd;
class CStatisticsDlg;
class CTaskbarNotifier;
class CTransferWnd;
struct Status;
class CSplashScreen;
class CMuleSystrayDlg;
class CMiniMule;
class CWebBrowserWnd; // Added by thilon on 2006.08.01
class CPopMule; // VC-SearchDream[2007-05-18]: for see the movie while downloading
class CPartFile; // VC-SearchDream[2007-05-25]:

// emuleapp <-> emuleapp
#define OP_ED2KLINK				12000
#define OP_CLCOMMAND			12001
#define OP_COLLECTION			12002
#define OP_QUERYSTATUS			12003
#define OP_COMEOUT				12004

#define	EMULE_HOTMENU_ACCEL		'x'
#define	EMULSKIN_BASEEXT		_T("eMuleSkin")


typedef struct TagTHREADPARMS{
	HWND hWnd;
}THREADPARMS;

class CMappingValue
{
public:
	WPARAM m_wParam;
	LPARAM m_lParam;
};


class CemuleDlg : public CTrayDialog
{
	//friend class CMuleToolbarCtrl;
	friend class CMiniMule;
	friend class CPopMule; // VC-SearchDream[2007-05-18]: Pop Mule

public:
	CemuleDlg(CWnd* pParent = NULL);
	~CemuleDlg();

	enum { IDD = IDD_EMULE_DIALOG };

	bool IsRunning();
	void ShowConnectionState();
	void ShowNotifier(LPCTSTR pszText, int iMsgType, LPCTSTR pszLink = NULL, bool bForceSoundOFF = false);
	void SendNotificationMail(int iMsgType, LPCTSTR pszText);
	void ShowUserCount();
	void ShowMessageState(UINT iconnr);
	void SetActiveDialog(CWnd* dlg);
	void ShowTransferRate(bool forceAll=false);
    void ShowPing();
	void Localize();

	//Chocobo Start
	//eMule自动更新，added by Chocobo on 2006.07.31
	//检查更新后的响应
	void OnUpdateAvailable();
	void OnUpdateNotAvailable(void);
	void OnUpdateNoCheck(void);
	//Chocobo End

	// Logging
	void AddLogText(UINT uFlags, LPCTSTR pszText);
	void AddServerMessageLine(UINT uFlags, LPCTSTR pszText);
	void ResetLog();
	void ResetDebugLog();
	void ResetServerInfo();
	void ResetLeecherLog();		//Xman Anti-Leecher-Log
	CString GetLastLogEntry();
	CString	GetLastDebugLogEntry();
	CString	GetAllLogEntries();
	CString	GetAllDebugLogEntries();
	CString GetServerInfoText();

	CString	GetConnectionStateString();
	UINT GetConnectionStateIconIndex() const;
	CString	GetTransferRateString();
	CString	GetUpDatarateString(UINT uUpDatarate = -1);
	CString	GetDownDatarateString(UINT uDownDatarate = -1);

	void StopTimer();
	void DoVersioncheck(bool manual);
	void ApplyHyperTextFont(LPLOGFONT pFont);
	void ApplyLogFont(LPLOGFONT pFont);
	void ProcessFileLink(LPCTSTR pszData, BOOL bTestUrl = FALSE);
	void SetStatusBarPartsSize();
	int ShowPreferences(UINT uStartPageID = (UINT)-1);
	bool IsPreferencesDlgOpen() const;
	bool IsTrayIconToFlash()	{ return m_iMsgIcon!=0; }

	virtual void TrayMinimizeToTrayChange();
	virtual void RestoreWindow();
	virtual void HtmlHelp(DWORD_PTR dwData, UINT nCmd = 0x000F);

	void InitShortcutManager(void);

	void SetUpdaterCreated( bool bCreated ){ m_bCreated = bCreated; }

	// VC-SearchDream[2007-05-18]: Pop Mule Begin 
	void ShowPopMule(CPartFile* pPartFile);
	// VC-SearchDream[2007-05-18]: Pop Mule End

	CTransferWnd*	transferwnd;
	CServerWnd*		serverwnd;
	CPreferencesDlg* preferenceswnd;
	CSharedFilesWnd* sharedfileswnd;
	CSearchDlg*		searchwnd;
	
	CMuleStatusBarCtrl* statusbar;
	CStatisticsDlg*  statisticswnd;
#if _ENABLE_NOUSE
	CChatWnd*		chatwnd;
	CIrcWnd*		ircwnd;
#endif
	CTaskbarNotifier* m_wndTaskbarNotifier;

#if _ENABLE_NOUSE
	CReBarCtrl			m_ctlMainTopReBar;
	CMuleToolbarCtrl*	toolbar;
#endif 

	CKademliaWnd*	kademliawnd;
	//CWebBrowserWnd*	webbrowser; //Added by thilon on 2006.08.01

	CMenuXP MenuXP;		//Added by thilon on 2007.02.12, for XP Style

//#ifdef EASY_UI
	CMainTabWnd		m_mainTabWnd;	//ADDED by fengwen on 2007/01/25
//#endif


	CWnd*			activewnd;
	uint8			status;

protected:
	HICON			m_hIcon;
	bool			ready;
	bool			m_bStartMinimizedChecked;
	bool			m_bStartMinimized;
	WINDOWPLACEMENT m_wpFirstRestore;
	HICON			connicons[9];
	HICON			transicons[4];
	HICON			imicons[3];
	HICON			m_icoSysTrayCurrent;
	HICON			usericon;
	CMeterIcon		m_TrayIcon;
	HICON			m_icoSysTrayConnected;		// do not use those icons for anything else than the traybar!!!
	HICON			m_icoSysTrayDisconnected;	// do not use those icons for anything else than the traybar!!!
	HICON			m_icoSysTrayLowID;	// do not use those icons for anything else than the traybar!!!
	int				m_iMsgIcon;
	UINT			m_uLastSysTrayIconCookie;
	uint32			m_uUpDatarate;
	uint32			m_uDownDatarate;
	CImageList		imagelist;
	CTitleMenu		trayPopup;
	CMuleSystrayDlg* m_pSystrayDlg;
	CMainFrameDropTarget* m_pDropTarget;
	CMenu			m_SysMenuOptions;
	CMenu			m_menuUploadCtrl;
	CMenu			m_menuDownloadCtrl;
	char			m_acVCDNSBuffer[MAXGETHOSTSTRUCT];
	bool			m_iMsgBlinkState;

	CShortcutManager m_mgrShortcuts;

	HANDLE			m_hMapping;
	LPSTR			m_lpData;
	//Chocobo Start
	//eMule自动更新, added by Chocobo on 2006.07.31
	//CEmuleUpdater*  m_pEmuleUpdater;//added by thilon on 2006.02.23
	bool			m_bCreated;
	//Chocobo End
	CUpdaterThread	*m_pUpdaterThread;
	bool			 m_bManualUpdte;

	// Mini Mule
	CMiniMule* m_pMiniMule;
	void DestroyMiniMule();

	// VC-SearchDream[2007-05-18]: Pop Mule Begin 
	// Pop Mule
	CPopMule* m_pPopMule;
	void DestroyPopMule();
	// VC-SearchDream[2007-05-18]: Pop Mule End 

#if _ENABLE_NOUSE
	CMap<UINT, UINT, LPCTSTR, LPCTSTR> m_mapTbarCmdToIcon;
	void CreateToolbarCmdIconMap();
	LPCTSTR GetIconFromCmdId(UINT uId);
#endif

	// Startup Timer
	UINT_PTR m_hTimer;
	static void CALLBACK StartupTimer(HWND hwnd, UINT uiMsg, UINT idEvent, DWORD dwTime);

	void StartConnection();
	void CloseConnection();
	void MinimizeWindow();
	void PostStartupMinimized();
	void UpdateTrayIcon(int iPercent);
	void ShowConnectionStateIcon();
	void ShowTransferStateIcon();
	void ShowUserStateIcon();
	void AddSpeedSelectorMenus(CMenu* addToMenu);
	int  GetRecMaxUpload();
	void LoadNotifier(CString configuration);
	bool notifierenabled;
	void ShowToolPopup(bool toolsonly = false);
	void SetAllIcons();
	bool CanClose();
#if _ENABLE_NOUSE
	int MapWindowToToolbarButton(CWnd* pWnd) const;
	CWnd* MapToolbarButtonToWindow(int iButtonID) const;
	int GetNextWindowToolbarButton(int iButtonID, int iDirection = 1) const;
	bool IsWindowToolbarButton(int iButtonID) const;
#endif
	//ADDED by fengwen on 2006/11/01 <begin> : 
	void	AddNatPortMappingAsyn();
	//ADDED by fengwen on 2006/11/01 <end> : 
	
	// VC-yunchenn.chen[2007-01-15]:
	virtual bool ProcessMessage(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT & lResult);

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	virtual void OnOK();
	virtual void OnTrayRButtonUp(CPoint pt);
	virtual void OnTrayLButtonUp(CPoint pt);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);


	afx_msg LRESULT OnMappingFinished(WPARAM wParam,LPARAM lParam);	//Added by thilon on 2006.10.27, for UPnP
	afx_msg LRESULT OnPortChanged(WPARAM wParam,LPARAM lParam);

	DECLARE_MESSAGE_MAP()
//	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType,int cx,int cy);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBnClickedButton2();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnBnClickedHotmenu();
	afx_msg LRESULT OnMenuChar(UINT nChar, UINT nFlags, CMenu* pMenu);
	afx_msg void OnSysColorChange();
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg BOOL OnQueryEndSession();
	afx_msg void OnEndSession(BOOL bEnding);
	afx_msg LRESULT OnUserChanged(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnKickIdle(UINT nWhy, long lIdleCount);
	afx_msg void OnShowWindow( BOOL bShow, UINT nStatus );
	afx_msg BOOL OnChevronPushed(UINT id, NMHDR *pnm, LRESULT *pResult);

	// quick-speed changer -- based on xrmb
	afx_msg void QuickSpeedUpload(UINT nID);
	afx_msg void QuickSpeedDownload(UINT nID);
	afx_msg void QuickSpeedOther(UINT nID);
	// end of quick-speed changer
	
	afx_msg LRESULT OnTaskbarNotifierClicked(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnWMData(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnFileHashed(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnHashFailed(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnFileAllocExc(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnFileCompleted(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnFileAddCompleted(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnFileOpProgress(WPARAM wParam,LPARAM lParam);

	// VC-SearchDream[2007-05-24]: For Player Request
	afx_msg LRESULT OnPlayerRequestData(WPARAM wParam,LPARAM lParam); 

	//Framegrabbing
	afx_msg LRESULT OnFrameGrabFinished(WPARAM wParam,LPARAM lParam);

	afx_msg LRESULT OnAreYouEmule(WPARAM, LPARAM);

	//Webinterface
	afx_msg LRESULT OnWebGUIInteraction(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWebServerClearCompleted(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWebServerFileRename(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWebAddDownloads(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWebSetCatPrio(WPARAM wParam, LPARAM lParam);
#if _ENABLE_NOUSE
	afx_msg LRESULT OnAddRemoveFriend(WPARAM wParam, LPARAM lParam);
#endif
	afx_msg LRESULT OnNatDisconnect(WPARAM wParam, LPARAM lParam);

	// VersionCheck DNS
	afx_msg LRESULT OnVersionCheckResponse(WPARAM wParam, LPARAM lParam);

	// Peercache DNS
	afx_msg LRESULT OnPeerCacheResponse(WPARAM wParam, LPARAM lParam);

	// Mini Mule
	afx_msg LRESULT OnCloseMiniMule(WPARAM wParam, LPARAM lParam);

	// Terminal Services
	afx_msg LRESULT OnConsoleThreadEvent(WPARAM wParam, LPARAM lParam);

	afx_msg LRESULT OnStartED2KUpdate(WPARAM wParam, LPARAM lParam); 
	afx_msg LRESULT OnCheckUpdateFinished(WPARAM wParam, LPARAM lParam); 
	afx_msg LRESULT OnED2KUpdateComplete(WPARAM wParam, LPARAM lParam);

	afx_msg LRESULT OnHotKey(WPARAM,LPARAM);// VC-kernel[2007-05-14]:hotkey
	afx_msg LRESULT OnNatCheckStrageties(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnRemoveFromAllQ_In_Throttler(WPARAM wParam, LPARAM lParam);
	
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);


protected:
	void InitMenuIcon(void);
	void RemoveOldUpdateFile(CString updatehash);

	// VC-SearchDream[2007-05-28]: Process Player Request 
	int ProcessPlayerRequest(PLAYER_DATA_REQ * req);

private:
	bool m_boss;	// VC-kernel[2007-05-14]:hotkey

	enum {TIMER_CONNECTING_TRAY = 1039};
	DWORD	m_dwTimer_ConnectingTray;
	BOOL	m_bConnectingIconSide;
	void	TriggerConnectingTray(BOOL bStart);
	afx_msg void OnTimer(UINT uIDEvent);
protected:
	void CloseToTray(void);
	void CloseImmeditely(void);
public:
	afx_msg void OnClose();
public:
 	bool m_called_OnMappingFinished;
	CList<CMappingValue> m_MappingValueList;
};


enum EEMuleAppMsgs
{
	//thread messages
	TM_FINISHEDHASHING = WM_APP + 10,
	TM_HASHFAILED,
	TM_FRAMEGRABFINISHED,
	TM_FILEALLOCEXC,
	TM_FILECOMPLETED,
	TM_FILEOPPROGRESS,
	TM_CONSOLETHREADEVENT
};

enum EWebinterfaceOrders
{
	WEBGUIIA_UPDATEMYINFO = 1,
	WEBGUIIA_WINFUNC,
	WEBGUIIA_UPD_CATTABS,
	WEBGUIIA_UPD_SFUPDATE,
	WEBGUIIA_UPDATESERVER,
	WEBGUIIA_STOPCONNECTING,
	WEBGUIIA_CONNECTTOSERVER,
	WEBGUIIA_DISCONNECT,
	WEBGUIIA_SERVER_REMOVE,
	WEBGUIIA_SHARED_FILES_RELOAD,
	WEBGUIIA_ADD_TO_STATIC,
	WEBGUIIA_REMOVE_FROM_STATIC,
	WEBGUIIA_UPDATESERVERMETFROMURL,
	WEBGUIIA_SHOWSTATISTICS,
	WEBGUIIA_DELETEALLSEARCHES,
	WEBGUIIA_KAD_BOOTSTRAP,
	WEBGUIIA_KAD_START,
	WEBGUIIA_KAD_STOP,
	WEBGUIIA_KAD_RCFW
};
