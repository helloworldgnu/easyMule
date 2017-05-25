/*
 * $Id: GlobalVariable.cpp 11398 2009-03-17 11:00:27Z huby $
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
#include "StdAfx.h"
#include ".\globalvariable.h"

#include "kademlia/kademlia/Kademlia.h"
#include "kademlia/kademlia/Prefs.h"
#include "kademlia/kademlia/Error.h"
#include "kademlia/utils/UInt128.h"
#include "server.h"
#include "version.h"
#include "MessageLog.h"
#include "DNSManager.h"
#include "FileMgr.h"
#include "emule.h"
#include "emuleDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

AppState CGlobalVariable::m_app_state=APP_STATE_RUNNING;
CListenSocket* CGlobalVariable::listensocket = NULL;
CClientList* CGlobalVariable::clientlist = NULL;
CKnownFileList* CGlobalVariable::knownfiles = NULL;
CUploadQueue* CGlobalVariable::uploadqueue = NULL;
CDownloadQueue* CGlobalVariable::downloadqueue = NULL;
CClientUDPSocket* CGlobalVariable::clientudp = NULL;
CNatThread* CGlobalVariable::natthread = NULL;
CServerConnect* CGlobalVariable::serverconnect = NULL;
CSharedFileList* CGlobalVariable::sharedfiles = NULL;
CServerList*CGlobalVariable::serverlist = NULL;
CSearchList* CGlobalVariable::searchlist = NULL;
HWND CGlobalVariable::m_hListenWnd = NULL;
CIP2Country*CGlobalVariable::ip2country = NULL;
CPeerCacheFinder*CGlobalVariable::m_pPeerCache = NULL;
CMMServer*CGlobalVariable::mmserver = NULL;;
CWebServer*CGlobalVariable::webserver =NULL;
CIPFilter*CGlobalVariable::ipfilter=NULL;
#ifdef _ENABLE_LAN_TRANSFER
CInternalSocket*CGlobalVariable::internalsocket=NULL;    // VC-kernel[2007-01-11]:
#endif
CClientCreditsList*	CGlobalVariable::clientcredits = NULL;
UploadBandwidthThrottler* CGlobalVariable::uploadBandwidthThrottler = NULL;
LastCommonRouteFinder* CGlobalVariable::lastCommonRouteFinder = NULL;
CMutex CGlobalVariable::hashing_mut;
int CGlobalVariable::m_dwTCPIPValue =0;
CFileMgr CGlobalVariable::filemgr;
CUPnpMgr CGlobalVariable::m_upnpMgr;
CDNSManager* CGlobalVariable::m_DNSManager = NULL;

CTypedPtrList<CPtrList, SLogItem*> CGlobalVariable::m_QueueDebugLog;
CTypedPtrList<CPtrList, SLogItem*> CGlobalVariable::m_QueueLog;

//  private variant
uint32 CGlobalVariable::m_dwPublicIP = 0;
CString CGlobalVariable::m_strCurVersionLong=(_T(""));
CCriticalSection CGlobalVariable::m_queueLock;
UINT main_threadid=0;
const UINT CGlobalVariable::m_nVersionMjr = VERSION_MJR;
const UINT CGlobalVariable::m_nVersionMin = VERSION_MIN;
const UINT CGlobalVariable::m_nVersionUpd = VERSION_UPDATE;
const UINT CGlobalVariable::m_nVersionBld = VERSION_BUILD;
const UINT CGlobalVariable::m_nVCVersionBld = VC_VERSION_BUILD;	//Added by thilon on 2006.01.10, for VeryCD°æBuildºÅ

UINT CGlobalVariable::m_uCurVersionShort = 0;

const UINT	CGlobalVariable::m_nEasyMuleMjr = EASYMULE_MJR;
const UINT	CGlobalVariable::m_nEasyMuleMin = EASYMULE_MIN;
const UINT	CGlobalVariable::m_nEasyMuleUpd = EASYMULE_UPDATE;

CSeeFileManager CGlobalVariable::m_SeeFileManager;
const UINT CGlobalVariable::m_nMaxDownloadingSeeFiles = 3;
CDLP* CGlobalVariable::dlp = NULL;

UINT CGlobalVariable::m_uCurrentPublicConnectNum = 0;
UINT CGlobalVariable::m_uActiveTaskNumber = 0;
struct SLogItem
{
	UINT uFlags;
	CString line;
};

CGlobalVariable::CGlobalVariable(void)
{
}

CGlobalVariable::~CGlobalVariable(void)
{
}

bool CGlobalVariable::Initialize(HWND hWnd)
{
	m_hListenWnd = hWnd;
	main_threadid = GetCurrentThreadId();
	filemgr.LoadFiles( filemgr.GetDatabaseFile());
	ip2country->Load();

	return true;
}

bool CGlobalVariable::CreateGlobalObject()
{
	serverlist = new CServerList();
	listensocket = new CListenSocket();
	clientlist = new CClientList();
	knownfiles = new CKnownFileList();
	clientudp	= new CClientUDPSocket();
	downloadqueue = new CDownloadQueue();	// bugfix - do this before creating the uploadqueue
	uploadqueue = new CUploadQueue();
	serverconnect = new CServerConnect();
	
	sharedfiles = new CSharedFileList(serverconnect);

#ifdef _ENABLE_NATTRAVERSE
	natthread  = (CNatThread*) AfxBeginThread(RUNTIME_CLASS(CNatThread));
#endif

	searchlist = new CSearchList();
	ip2country = new CIP2Country();						//EastShare - added by AndCycle, IP to Country
	m_pPeerCache = new CPeerCacheFinder();
	ipfilter 	= new CIPFilter();
	webserver = new CWebServer(); // Webserver [kuchin]
	mmserver = new CMMServer();
#ifdef _ENABLE_LAN_TRANSFER
	internalsocket = new CInternalSocket(); // VC-kernel[2007-01-12]:
#endif
	clientcredits = new CClientCreditsList();
	uploadBandwidthThrottler = new UploadBandwidthThrottler();

#ifdef _ENABLE_USS
	lastCommonRouteFinder = new LastCommonRouteFinder();
#endif

	m_DNSManager = new CDNSManager();

	return true;
}

void CGlobalVariable::Clearup()
{
	delete clientlist;		clientlist = NULL;
	delete knownfiles;		knownfiles = NULL;
	delete downloadqueue;	downloadqueue = NULL;
	delete clientudp;		clientudp = NULL;
	delete uploadqueue;		uploadqueue = NULL;

	delete listensocket;	listensocket = NULL;
	delete sharedfiles;		sharedfiles = NULL;
	delete serverconnect;	serverconnect = NULL;
	delete serverlist;		serverlist = NULL;
	delete searchlist;		searchlist = NULL;
	delete ip2country;		ip2country = NULL;
	delete m_pPeerCache;	m_pPeerCache = NULL;
	delete webserver;		webserver = NULL;
	delete ipfilter;		ipfilter = NULL;			// CIPFilter::SaveToDefaultFile
	delete mmserver;		mmserver = NULL;
#ifdef _ENABLE_LAN_TRANSFER
	delete internalsocket;	internalsocket = NULL;
#endif
	delete clientcredits;	clientcredits = NULL;	// CClientCreditsList::SaveList

	uploadBandwidthThrottler->EndThread();

#ifdef _ENABLE_USS
	lastCommonRouteFinder->EndThread();
#endif

	delete uploadBandwidthThrottler; uploadBandwidthThrottler = NULL;
	//LastCommonRouteFinder->ResumeThread();
#ifdef _ENABLE_USS
	delete lastCommonRouteFinder; 
	lastCommonRouteFinder = NULL;
#endif

	delete m_DNSManager; m_DNSManager = NULL;
	// [10/26/2007 huby]: maybe it's the cause of crash in _AfxMsgFilterHook,solve the memleak by thilon on 2007.sep.14
	//delete natthread; natthread = NULL;
}

uint32 CGlobalVariable::GetPublicIP(bool bIgnoreKadIP)
{
	if (m_dwPublicIP == 0 && Kademlia::CKademlia::IsConnected() && Kademlia::CKademlia::GetIPAddress() && !bIgnoreKadIP)
		return ntohl(Kademlia::CKademlia::GetIPAddress());
	return m_dwPublicIP;
}

void CGlobalVariable::SetPublicIP(const uint32 dwIP)
{
	if (dwIP != 0){
		ASSERT ( !IsLowID(dwIP));
		ASSERT ( m_pPeerCache );
		if ( GetPublicIP() == 0)
			AddDebugLogLine(DLP_VERYLOW, false, _T("My public IP Address is: %s"),ipstr(dwIP));
		else if (Kademlia::CKademlia::IsConnected() && Kademlia::CKademlia::GetPrefs()->GetIPAddress())
			if(ntohl(Kademlia::CKademlia::GetIPAddress()) != dwIP)
				AddDebugLogLine(DLP_DEFAULT, false,  _T("Public IP Address reported from Kademlia (%s) differs from new found (%s)"),ipstr(ntohl(Kademlia::CKademlia::GetIPAddress())),ipstr(dwIP));
		m_pPeerCache->FoundMyPublicIPAddress(dwIP);	
	}
	else
		AddDebugLogLine(DLP_VERYLOW, false, _T("Deleted public IP"));

	if (dwIP != 0 && dwIP != m_dwPublicIP && CGlobalVariable::serverlist != NULL){
		m_dwPublicIP = dwIP;
		CGlobalVariable::serverlist->CheckForExpiredUDPKeys();
	}
	else
		m_dwPublicIP = dwIP;
}

void CGlobalVariable::SetCurVersionLong(const CString & strNew)
{
	m_strCurVersionLong = strNew;
}

void CGlobalVariable::QueueDebugLogLine(bool bAddToStatusbar, LPCTSTR line, ...)
{
	if (!thePrefs.GetVerbose())
		return;

	m_queueLock.Lock();

	TCHAR bufferline[1000];

	va_list argptr;
	va_start(argptr, line);
	_vsntprintf(bufferline, ARRSIZE(bufferline), line, argptr);
	va_end(argptr);

	SLogItem* newItem = new SLogItem;
	newItem->uFlags = LOG_DEBUG | (bAddToStatusbar ? LOG_STATUSBAR : 0);
	newItem->line = bufferline;
	m_QueueDebugLog.AddTail(newItem);

	m_queueLock.Unlock();
}

void CGlobalVariable::QueueLogLine(bool bAddToStatusbar, LPCTSTR line, ...)
{
	m_queueLock.Lock();

	TCHAR bufferline[1000];

	va_list argptr;
	va_start(argptr, line);
	_vsntprintf(bufferline, ARRSIZE(bufferline), line, argptr);
	va_end(argptr);

	SLogItem* newItem = new SLogItem;
	newItem->uFlags = bAddToStatusbar ? LOG_STATUSBAR : 0;
	newItem->line = bufferline;
	m_QueueLog.AddTail(newItem);

	m_queueLock.Unlock();
}

void CGlobalVariable::QueueDebugLogLineEx(UINT uFlags, LPCTSTR line, ...)
{
	if (!thePrefs.GetVerbose())
		return;

	m_queueLock.Lock();

	TCHAR bufferline[1000];

	va_list argptr;
	va_start(argptr, line);
	_vsntprintf(bufferline, ARRSIZE(bufferline), line, argptr);
	va_end(argptr);

	SLogItem* newItem = new SLogItem;
	newItem->uFlags = uFlags | LOG_DEBUG;
	newItem->line = bufferline;
	m_QueueDebugLog.AddTail(newItem);

	m_queueLock.Unlock();
}

void CGlobalVariable::QueueLogLineEx(UINT uFlags, LPCTSTR line, ...)
{
	m_queueLock.Lock();

	TCHAR bufferline[1000];

	va_list argptr;
	va_start(argptr, line);
	_vsntprintf(bufferline, ARRSIZE(bufferline), line, argptr);
	va_end(argptr);

	SLogItem* newItem = new SLogItem;
	newItem->uFlags = uFlags;
	newItem->line = bufferline;
	m_QueueLog.AddTail(newItem);

	m_queueLock.Unlock();
}

void CGlobalVariable::ClearDebugLogQueue(bool bDebugPendingMsgs)
{
	m_queueLock.Lock();
	while(!m_QueueDebugLog.IsEmpty())
	{
		if (bDebugPendingMsgs)
			TRACE(_T("Queued dbg log msg: %s\n"), m_QueueDebugLog.GetHead()->line);
		delete m_QueueDebugLog.RemoveHead();
	}
	m_queueLock.Unlock();
}

void CGlobalVariable::ClearLogQueue(bool bDebugPendingMsgs)
{
	m_queueLock.Lock();
	while(!m_QueueLog.IsEmpty())
	{
		if (bDebugPendingMsgs)
			TRACE(_T("Queued log msg: %s\n"), m_QueueLog.GetHead()->line);
		delete m_QueueLog.RemoveHead();
	}
	m_queueLock.Unlock();
}

void CGlobalVariable::HandleDebugLogQueue()
{
	m_queueLock.Lock();
	while (!m_QueueDebugLog.IsEmpty())
	{
		const SLogItem* newItem = m_QueueDebugLog.RemoveHead();
		if (thePrefs.GetVerbose())
			Log(newItem->uFlags, _T("%s"), newItem->line);
		delete newItem;
	}
	m_queueLock.Unlock();
}

void CGlobalVariable::HandleLogQueue()
{
	m_queueLock.Lock();
	while (!m_QueueLog.IsEmpty())
	{
		const SLogItem* newItem = m_QueueLog.RemoveHead();
		Log(newItem->uFlags, _T("%s"), newItem->line);
		delete newItem;
	}
	m_queueLock.Unlock();
}

bool CGlobalVariable::CanDoCallback(const CUpDownClient *client )
{
	if(Kademlia::CKademlia::IsConnected())
	{
		if(CGlobalVariable::serverconnect->IsConnected())
		{
			if(CGlobalVariable::serverconnect->IsLowID())
			{
				if(Kademlia::CKademlia::IsFirewalled())
				{
					//Both Connected - Both Firewalled
					return false;
				}
				else
				{
					if(client->GetServerIP() == CGlobalVariable::serverconnect->GetCurrentServer()->GetIP() && client->GetServerPort() == CGlobalVariable::serverconnect->GetCurrentServer()->GetPort())
					{
						//Both Connected - Server lowID, Kad Open - Client on same server
						//We prevent a callback to the server as this breaks the protocol and will get you banned.
						return false;
					}
					else
					{
						//Both Connected - Server lowID, Kad Open - Client on remote server
						return true;
					}
				}
			}
			else
			{
				//Both Connected - Server HighID, Kad don't care
				return true;
			}
		}
		else
		{
			if(Kademlia::CKademlia::IsFirewalled())
			{
				//Only Kad Connected - Kad Firewalled
				return false;
			}
			else
			{
				//Only Kad Conected - Kad Open
				return true;
			}
		}
	}
	else
	{
		if( CGlobalVariable::serverconnect->IsConnected() )
		{
			if( CGlobalVariable::serverconnect->IsLowID() )
			{
				//Only Server Connected - Server LowID
				return false;
			}
			else
			{
				//Only Server Connected - Server HighID
				return true;
			}
		}
		else
		{
			//We are not connected at all!
			return false;
		}
	}
}

bool CGlobalVariable::IsFirewalled()
{
	if (serverconnect->IsConnected() && !serverconnect->IsLowID())
		return false; // we have an eD2K HighID -> not firewalled

	if (Kademlia::CKademlia::IsConnected() && !Kademlia::CKademlia::IsFirewalled())
		return false; // we have an Kad HighID -> not firewalled

	return true; // firewalled
}

bool CGlobalVariable::IsConnected()
{
	return ( (CGlobalVariable::serverconnect && CGlobalVariable::serverconnect->IsConnected()) 
		|| Kademlia::CKademlia::IsConnected() );
}

uint32 CGlobalVariable::GetID()
{
	uint32 ID;
	if( Kademlia::CKademlia::IsConnected() && !Kademlia::CKademlia::IsFirewalled() )
		ID = ntohl(Kademlia::CKademlia::GetIPAddress());
	else if( CGlobalVariable::serverconnect->IsConnected() )
		ID = CGlobalVariable::serverconnect->GetClientID();
	else if ( Kademlia::CKademlia::IsConnected() && Kademlia::CKademlia::IsFirewalled() )
		ID = 1;
	else
		ID = 0;
	return ID;
}

void UINotify(UINT msg, WPARAM wParam, LPARAM lParam, void * pTag, bool bCredible)
{
	if(CGlobalVariable::m_hListenWnd)
	{
		UINT uIndex=CMessageLog::GetInstace()->SaveMessage(msg, wParam, lParam, pTag, bCredible);
		if(bCredible && main_threadid==GetCurrentThreadId())
		{
			SendMessage(CGlobalVariable::m_hListenWnd, msg, uIndex, 0);
		}
		else
		{
			PostMessage(CGlobalVariable::m_hListenWnd, msg, uIndex, 0);
		}
	}
}
void CGlobalVariable::ShowNotifier(LPCTSTR pszText, int iMsgType, LPCTSTR pszLink, bool bForceSoundOFF)
{
	theApp.emuledlg->ShowNotifier(pszText, iMsgType, pszLink, bForceSoundOFF);
}


void RemoveMessageTag(void * pTag)
{
	CMessageLog::GetInstace()->RemoveTag(pTag);
}

