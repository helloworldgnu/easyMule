/*
 * $Id: GlobalVariable.h 9297 2008-12-24 09:55:04Z dgkang $
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
#pragma once

#include "ServerList.h"
#include "ListenSocket.h"
#include "ClientList.h"
#include "KnownFileList.h"
#include "UploadQueue.h"
#include "DownloadQueue.h"
#include "ClientUDPSocket.h"
#include "NatTraversal/NatThread.h"
#include "sockets.h"
#include "SharedFileList.h"
#include "SearchList.h"
#include "ip2country.h"
#include "PeerCacheFinder.h"
#include "WebServer.h"
#include "IPFilter.h"
#include "Internal/InternalSocket.h"
#include "MMServer.h"
#include "ClientCredits.h"
#include "LastCommonRouteFinder.h"
#include "SeeFileManager.h"
#include "UPnpMgr.h"
#include "DLP.h"

enum AppState
{
	APP_STATE_RUNNING = 0,
	APP_STATE_SHUTTINGDOWN,
	APP_STATE_DONE
};

class CPeerCacheFinder;
class CInternalSocket;
class CDNSManager;
class CFileMgr;

struct SLogItem;

class CGlobalVariable
{
public:
	CGlobalVariable(void);
	~CGlobalVariable(void);

	static CClientList*			clientlist;
	static CKnownFileList*		knownfiles;
	static CUploadQueue*		uploadqueue;
	static CDownloadQueue*		downloadqueue;
	static CServerConnect*		serverconnect;
	static CSharedFileList*		sharedfiles;
	static CServerList*			serverlist;
	static CSearchList*			searchlist;
	static CClientCreditsList*	clientcredits;
	static CDNSManager*         m_DNSManager;

	static HWND					m_hListenWnd;
	static AppState				m_app_state; // defines application state for shutdown 
	static CListenSocket*		listensocket;
	static CClientUDPSocket*	clientudp;
	static CNatThread*			natthread;
	static CIP2Country*			ip2country; //EastShare - added by AndCycle, IP to Country
	static CPeerCacheFinder*	m_pPeerCache;
	static CWebServer*			webserver;
	static CIPFilter*			ipfilter;
#ifdef _ENABLE_LAN_TRANSFER
	static CInternalSocket*		internalsocket;    // VC-kernel[2007-01-11]:
#endif
	static CMMServer*			mmserver;
	static CFileMgr				filemgr;
	static CMutex				hashing_mut;
	static CSeeFileManager     m_SeeFileManager; // VC-SearchDream[2007-07-12]:

	static CUPnpMgr				m_upnpMgr;

	static UploadBandwidthThrottler* uploadBandwidthThrottler;
	static LastCommonRouteFinder* lastCommonRouteFinder;
	static CTypedPtrList<CPtrList, SLogItem*> m_QueueDebugLog;
	static CTypedPtrList<CPtrList, SLogItem*> m_QueueLog;

	static const UINT	m_nVersionMjr;
	static const UINT	m_nVersionMin;
	static const UINT	m_nVersionUpd;
	static const UINT	m_nVersionBld;
	static const UINT	m_nVCVersionBld;	//Added by thilon on 2006.01.10, for VeryCD∞ÊBuild∫≈
	static UINT			m_uCurVersionShort;

	static const UINT	m_nEasyMuleMjr;
	static const UINT	m_nEasyMuleMin;
	static const UINT	m_nEasyMuleUpd;

	static const UINT   m_nMaxDownloadingSeeFiles;

	//Xman DLP
	static CDLP *		dlp;

public:
	static bool IsRunning()
	{
		return (CGlobalVariable::m_app_state == APP_STATE_RUNNING);
	}
	static bool CreateGlobalObject();
	static bool Initialize(HWND hWnd);
	static void Clearup();

	//  global functions
	static void SetPublicIP(const uint32 dwIP);
	static uint32 GetPublicIP(bool bIgnoreKadIP = false);

	static CString GetCurVersionLong()
	{
		return m_strCurVersionLong;
	}
	static DWORD GetTCPIPVaule(void)
	{
		return m_dwTCPIPValue;
	}
	static void SetTCPIPValue(DWORD dwValue)
	{
		m_dwTCPIPValue = dwValue;
	}

	static void SetCurVersionLong(const CString & strNew);

	static void QueueDebugLogLine(bool bAddToStatusBar, LPCTSTR line,...);
	static void	QueueDebugLogLineEx(UINT uFlags, LPCTSTR line,...);
	static void QueueLogLine(bool bAddToStatusBar, LPCTSTR line,...);
	static void	QueueLogLineEx(UINT uFlags, LPCTSTR line,...);
	static void	ClearLogQueue(bool bDebugPendingMsgs = false);
	static void ClearDebugLogQueue(bool bDebugPendingMsgs = false);
	static void HandleDebugLogQueue();
	static void HandleLogQueue();
	static bool CanDoCallback(const CUpDownClient *client );
	static bool IsFirewalled();
	static bool IsConnected();
	static uint32 GetID();
	static void ShowNotifier(LPCTSTR pszText, int iMsgType, LPCTSTR pszLink = NULL, bool bForceSoundOFF = false);

private:
	static uint32		m_dwPublicIP;
	static CString		m_strCurVersionLong;
	static CCriticalSection m_queueLock;
	static int		m_dwTCPIPValue;		//Added by thilon on 2006.08.07
public:
	static UINT m_uCurrentPublicConnectNum;
	static UINT m_uActiveTaskNumber;
};

extern void UINotify(UINT msg, WPARAM wParam, LPARAM lParam, void * pTag = NULL, bool bCredible=false);
extern void RemoveMessageTag(void * pTag);
