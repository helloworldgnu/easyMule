/* 
 * $Id: emule.h 9297 2008-12-24 09:55:04Z dgkang $
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
#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif
#include "resource.h"

#include "MemPool/MemPoolMgr.h"
using namespace MemPool;
#include "UPnpMgr.h"	//ADDED by fengwen on 2006/11/01 :

#include "SplashScreen.h"
#include "GlobalVariable.h"

#include "SingleInst.h"

#include "MenuXP.h"	//Added by thilnon on 2007.02.12
#include "BrowserToolbarInfo.h"

//#define	DEFAULT_NICK		thePrefs.GetHomepageBaseURL()

#define	DEFAULT_TCP_PORT	4662
#define	DEFAULT_UDP_PORT	(DEFAULT_TCP_PORT+10)

#define PORTTESTURL			_T("http://www.emule.org.cn/porttest/?tcpport=%i&udpport=%i")

class CIP2Country; //EastShare - added by AndCycle, IP to Country

class CSearchList;
class CUploadQueue;
class CListenSocket;
class CDownloadQueue;
class CScheduler;
class UploadBandwidthThrottler;
class LastCommonRouteFinder;
class CemuleDlg;
class CClientList;
class CKnownFileList;
class CServerConnect;
class CServerList;
class CSharedFileList;
class CClientCreditsList;
class CFriendList;
class CClientUDPSocket;
class CIPFilter;
class CWebServer;
class CMMServer;
class CAbstractFile;
class CUpDownClient;
class CPeerCacheFinder;
class CFirewallOpener;
class CInternalSocket;	// VC-kernel[2007-01-11]:


class CemuleApp : public CWinApp
{
public:
	CemuleApp(LPCTSTR lpszAppName = NULL);

	// ZZ:UploadSpeedSense -->
//    UploadBandwidthThrottler* uploadBandwidthThrottler;
  //  LastCommonRouteFinder* lastCommonRouteFinder;
	// ZZ:UploadSpeedSense <--
	CemuleDlg*			emuledlg;
#if _ENABLE_NOUSE
	CFriendList*		friendlist;
#endif
	CScheduler*			scheduler;
	CFirewallOpener*	m_pFirewallOpener;

#ifdef _SUPPORT_MEMPOOL
	CMemPoolMgr * m_pMemoryPool; // Added by SearchDream@2006/01/05
#endif

	//HANDLE				m_hMutexOneInstance;
	//HANDLE				m_hMutexDlgInit;
	int					m_iDfltImageListColorFlags;
	CFont				m_fontHyperText;
	CFont				m_fontDefaultBold;
	CFont				m_fontSymbol;
	CFont				m_fontLog;
	CBrush				m_brushBackwardDiagonal;
	
	DWORD				m_dwProductVersionMS;
	DWORD				m_dwProductVersionLS;

	CString				m_strCurVersionLongDbg;
	UINT				m_uCurVersionCheck;
	//Chocobo Start
	//eMule自动更新,added by Chocobo on 2006.07.31
	BOOL                m_bUpdateDownloaded; // 升级文件下载完成标志 --GGSoSo
	CString				m_strCurVersionCheck;
	//Chocobo End
	ULONGLONG			m_ullComCtrlVer;
	CMutex				hashing_mut;
	CString*			pstrPendingLink;
	COPYDATASTRUCT		sendstruct;

	CString				m_LocalBindAddress; // added by thilon on 2006.10.18, for BindToAdapter

// Implementierung
	virtual BOOL InitInstance();
	virtual int ExitInstance();

	// ed2k link functions
	//void		AddEd2kLinksToDownload(CString strLinks, int cat);
	void		SearchClipboard();
	void		IgnoreClipboardLinks(CString strLinks) {m_strLastClipboardContents = strLinks;}
	void		PasteClipboard(int cat = 0);
	bool		IsEd2kFileLinkInClipboard();
	bool		IsEd2kServerLinkInClipboard();
	bool		IsEd2kLinkInClipboard(LPCSTR pszLinkType, int iLinkTypeLen);
	LPCTSTR		GetProfileFile()		{ return m_pszProfileName; }

	CString		CreateED2kSourceLink(const CAbstractFile* f);
//	CString		CreateED2kHostnameSourceLink(const CAbstractFile* f);
	CString		CreateKadSourceLink(const CAbstractFile* f);

	// clipboard (text)
	bool		CopyTextToClipboard(CString strText);
	CString		CopyTextFromClipboard();

	void		OnlineSig();
	void		UpdateReceivedBytes(uint32 bytesToAdd);
	void		UpdateSentBytes(uint32 bytesToAdd, bool sentToFriend = false);
	int			GetFileTypeSystemImageIdx(LPCTSTR pszFilePath, int iLength = -1);
	HIMAGELIST	GetSystemImageList() { return m_hSystemImageList; }
	CSize		GetSmallSytemIconSize() { return m_sizSmallSystemIcon; }
	void		CreateBackwardDiagonalBrush();
	void		CreateAllFonts();
	bool		IsPortchangeAllowed();

	//void		SetTCPIPValue(DWORD dwValue);		//Added by thilon on 2006.08.07
	//DWORD		GetTCPIPVaule(void);				//Added by thilon on 2006.08.07

	LPCTSTR		GetBindAddress() { if (m_LocalBindAddress.IsEmpty()) { return NULL; } else { return m_LocalBindAddress; } }//Added by thilon on 2006.10.18
	void		BindToAddress(LPCTSTR LocalBindAddress = NULL);	//Added by thilon on 2006.10.18, for BindToAdapter

	// because nearly all icons we are loading are 16x16, the default size is specified as 16 and not as 32 nor LR_DEFAULTSIZE
	HICON		LoadIcon(LPCTSTR lpszResourceName, int cx = 16, int cy = 16, UINT uFlags = LR_DEFAULTCOLOR) const;
	HICON		LoadIcon(UINT nIDResource) const;
	HBITMAP		LoadImage(LPCTSTR lpszResourceName, LPCTSTR pszResourceType) const;
	HBITMAP		LoadImage(UINT nIDResource, LPCTSTR pszResourceType) const;
	bool		LoadSkinColor(LPCTSTR pszKey, COLORREF& crColor) const;
	bool		LoadSkinColorAlt(LPCTSTR pszKey, LPCTSTR pszAlternateKey, COLORREF& crColor) const;
	CString		GetSkinFileItem(LPCTSTR lpszResourceName, LPCTSTR pszResourceType) const;
	void		ApplySkin(LPCTSTR pszSkinProfile);
	void		EnableRTLWindowsLayout();
	void		DisableRTLWindowsLayout();
	void		UpdateDesktopColorDepth();

	bool		GetLangHelpFilePath(CString& strResult);
	void		SetHelpFilePath(LPCTSTR pszHelpFilePath);
	void		ShowHelp(UINT uTopic, UINT uCmd = HELP_CONTEXT);
	bool		ShowWebHelp(UINT uTopic);

	bool			DidWeAutoStart() { return m_bAutoStart; }

public:
		BOOL		LoadString(CString& str, UINT nStringID);

protected:
	bool ProcessCommandline();
	void SetTimeOnTransfer();
	static BOOL CALLBACK SearchEmuleWindow(HWND hWnd, LPARAM lParam);	

	HIMAGELIST m_hSystemImageList;
	CMapStringToPtr m_aExtToSysImgIdx;
	CSize m_sizSmallSystemIcon;

    BOOL RegisterFileType(LPCTSTR extension, LPCTSTR description, LPCTSTR iconpath, LPCTSTR iconnum);

	bool		m_bGuardClipboardPrompt;
	CString		m_strLastClipboardContents;

	bool m_bAutoStart;

	CMap<UINT, UINT, CString, CString&> m_pStrings;

public:
	CBrowserToolbarInfo	m_BrowserToolbarInfo;

private:
    UINT     m_wTimerRes;


//Addded by thilon on 2006.09.24, for UPnP
//upnp_start
public:
	BOOL   m_app_ready;
//upnp_end

//ADDED by fengwen on 2006/11/01 <begin> : 
public:
	CWinThread* m_pSplashThread;

//ADDED by fengwen on 2006/11/01 <end> : 
	
	void AddED2KUpdateToDownload(CString strLinks);

	CSingleInst		*m_pSingleInst;	//ADDED by fengwen on 2007/03/05 :	用于管理进程唯一实例。
	CSingleInst		*m_pSingleInst2Loader;	//ADDED by VC-fengwen on 2007/10/11 :	用于与ed2kLoader.exe通信。
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};

extern CemuleApp theApp;


//////////////////////////////////////////////////////////////////////////////
// CTempIconLoader

class CTempIconLoader
{
public:
	// because nearly all icons we are loading are 16x16, the default size is specified as 16 and not as 32 nor LR_DEFAULTSIZE
	CTempIconLoader(LPCTSTR pszResourceID, int cx = 16, int cy = 16, UINT uFlags = LR_DEFAULTCOLOR);
	CTempIconLoader(UINT uResourceID, int cx = 16, int cy = 16, UINT uFlags = LR_DEFAULTCOLOR);
	~CTempIconLoader();

	operator HICON() const{
		return this == NULL ? NULL : m_hIcon;
	}

protected:
	HICON m_hIcon;
};
