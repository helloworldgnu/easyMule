/* 
 * $Id: BaseClient.cpp 12458 2009-04-27 10:31:25Z huby $
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
#ifdef _DEBUG
#include "DebugHelpers.h"
#endif
//#include "emule.h"
#include "UpDownClient.h"
#include "FriendList.h"
#include "Clientlist.h"
#include "OtherFunctions.h"
#include "PartFile.h"
#include "ListenSocket.h"
#include "PeerCacheSocket.h"
#include "Friend.h"
#include <zlib/zlib.h>
#include "Packets.h"
#include "Opcodes.h"
#include "SafeFile.h"
#include "Preferences.h"
#include "Server.h"
#include "ClientCredits.h"
#include "IPFilter.h"
#include "Statistics.h"
#include "Sockets.h"
#include "DownloadQueue.h"
#include "UploadQueue.h"
#include "SearchFile.h"
#include "SearchList.h"
#include "SharedFileList.h"

#include "Kademlia/Kademlia/Kademlia.h"
#include "Kademlia/Kademlia/Search.h"
#include "Kademlia/Kademlia/SearchManager.h"
#include "Kademlia/Kademlia/UDPFirewallTester.h"
//#include "Kademlia/routing/RoutingZone.h"
#include "Kademlia/Utils/UInt128.h"
#include "Kademlia/Net/KademliaUDPListener.h"
#include "Kademlia/Kademlia/Prefs.h"

#include "Exceptions.h"
#include "Peercachefinder.h"
#include "ClientUDPSocket.h"
#include "shahashset.h"
#include "Log.h"
#include "DLP.h" //Xman DLP
#include "UIMessage.h"
#include "GlobalVariable.h"
#include "resource.h"
#include "Version.h"
#include "MessageLog.h"

// VC-yunchenn.chen[2007-01-04]
#include "NatTraversal/NatThread.h"

#include "IP2Country.h" //EastShare - added by AndCycle, IP to Country
#include "vcconfig.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

uint32 CUpDownClient::sm_iPeerIndex = 0; 

//EastShare Start - added by AndCycle, IP to Country
// Superlexx - client's location
CString	CUpDownClient::GetCountryName(bool longName) const {

	//display in client detail
	if(longName && CGlobalVariable::ip2country->IsIP2Country() == false)	return GetResString(IDS_DISABLED);

	if(CGlobalVariable::ip2country->IsIP2Country() == false) return _T("");

	if(longName) return m_structUserCountry->LongCountryName;

	CString tempStr;

	switch(thePrefs.GetIP2CountryNameMode()){
		case IP2CountryName_SHORT:
// EastShare - Modified by Pretender
//			tempStr.Format("<%s>",m_structUserCountry->ShortCountryName);
			tempStr.Format(_T("%s: "),m_structUserCountry->ShortCountryName);
			return tempStr;
		case IP2CountryName_MID:
//			tempStr.Format("<%s>",m_structUserCountry->MidCountryName);
			tempStr.Format(_T("%s: "),m_structUserCountry->MidCountryName);
			return tempStr;
		case IP2CountryName_LONG:
//			tempStr.Format("<%s>",m_structUserCountry->LongCountryName);
			tempStr.Format(_T("%s: "),m_structUserCountry->LongCountryName);
			return tempStr;
// EastShare - Modified by Pretender
	}
	return _T("");
}

int CUpDownClient::GetCountryFlagIndex() const {
	//ADDED by fengwen on 2007/01/17	<begin> :
	if (NULL == m_structUserCountry)
		return 0;
	//ADDED by fengwen on 2007/01/17	<end> :

	return m_structUserCountry->FlagIndex;
}
void CUpDownClient::ResetIP2Country(){
	m_structUserCountry = CGlobalVariable::ip2country->GetCountryFromIP(m_dwUserIP);
}
//EastShare End - added by AndCycle, IP to Country


IMPLEMENT_DYNAMIC(CClientException, CException)
IMPLEMENT_DYNAMIC(CUpDownClient, CObject)

CUpDownClient::CUpDownClient(CClientReqSocket* sender)
{
	m_iPeerIndex = ++sm_iPeerIndex;
	m_iPeerType = ptED2K;

	socket = sender;
	reqfile = NULL;
	bNeedProcess = true;
	Init();
}

CUpDownClient::CUpDownClient(CPartFile* in_reqfile, uint16 in_port, uint32 in_userid,uint32 in_serverip, uint16 in_serverport, bool ed2kID)
{
	m_iPeerIndex = ++sm_iPeerIndex;
	m_iPeerType = ptED2K;

	//Converting to the HybridID system.. The ED2K system didn't take into account of IP address ending in 0..
	//All IP addresses ending in 0 were assumed to be a lowID because of the calculations.
	socket = NULL;
	reqfile = in_reqfile;
	Init();
	m_nUserPort = in_port;
	//If this is a ED2K source, check if it's a lowID.. If not, convert it to a HyrbidID.
	//Else, it's already in hybrid form.
	if(ed2kID && !IsLowID(in_userid))
		m_nUserIDHybrid = ntohl(in_userid);
	else
		m_nUserIDHybrid = in_userid;

	//If highID and ED2K source, incoming ID and IP are equal..
	//If highID and Kad source, incoming IP needs ntohl for the IP
	if (!HasLowID() && ed2kID)
		m_nConnectIP = in_userid;
	else if(!HasLowID())
		m_nConnectIP = ntohl(in_userid);
	m_dwServerIP = in_serverip;
	m_nServerPort = in_serverport;
}

void CUpDownClient::Init()
{
	m_nChatstate = MS_NONE;
	m_nKadState = KS_NONE;
	//m_nChatCaptchaState = CA_NONE;
	m_nUploadState = US_NONE;
	m_nDownloadState = DS_NONE;
	m_SecureIdentState = IS_UNAVAILABLE;
	m_nConnectingState = CCS_NONE;
	m_ePeerCacheDownState = PCDS_NONE;
	m_ePeerCacheUpState = PCUS_NONE;

	m_structUserCountry = CGlobalVariable::ip2country->GetDefaultIP2Country(); //EastShare - added by AndCycle, IP to Country

	credits = 0;
	m_bLeecher = 0;
	m_nSumForAvgUpDataRate = 0;
	m_bAddNextConnect = false;
	m_cShowDR = 0;
	m_nUDPPort = 0;
	m_nKadPort = 0;
	m_nTransferredUp = 0;
	m_cAsked = 0;
	m_cDownAsked = 0;
	m_nUpDatarate = 0;
	m_pszUsername = 0;
	m_nUserIDHybrid = 0;
	m_dwServerIP = 0;
	m_nServerPort = 0;
    m_iFileListRequested = 0;
	m_dwLastUpRequest = 0;
	m_bEmuleProtocol = false;
	m_bCompleteSource = false;
	m_bFriendSlot = false;
	m_bCommentDirty = false;
	m_bReaskPending = false;
	m_bUDPPending = false;
	m_byEmuleVersion = 0;
	m_nUserPort = 0;
	m_nPartCount = 0;
	m_nUpPartCount = 0;
	m_abyPartStatus = 0;
	m_abyUpPartStatus = 0;
	m_dwUploadTime = 0;
	m_nTransferredDown = 0;
	m_nDownDatarate = 0;
	m_nDownDatarateOfPreTransfer = 0;
	m_nDownDataRateMS = 0;
	m_dwLastBlockReceived = 0;
	m_byDataCompVer = 0;
	m_byUDPVer = 0;
	m_bySourceExchange1Ver = 0;
	m_byAcceptCommentVer = 0;
	m_byExtendedRequestsVer = 0;
	m_nRemoteQueueRank = 0;
	m_dwLastSourceRequest = 0;
	m_dwLastSourceAnswer = 0;
	m_dwLastAskedForSources = 0;
	m_byCompatibleClient = 0;
	m_nSourceFrom = SF_SERVER;
	m_bIsHybrid = false;
	m_bIsML=false;
	m_Friend = NULL;
	m_uFileRating=0;
	(void)m_strFileComment;
	m_fMessageFiltered = 0;
	m_fIsSpammer = 0;
	m_cMessagesReceived = 0;
	m_cMessagesSent = 0;
	m_nCurSessionUp = 0;
	m_nCurSessionDown = 0;
    m_nCurSessionPayloadDown = 0;
	m_nSumForAvgDownDataRate = 0;
	m_clientSoft=SO_UNKNOWN;
	m_bRemoteQueueFull = false;
	md4clr(m_achUserHash);
	SetBuddyID(NULL);
	m_nBuddyIP = 0;
	m_nBuddyPort = 0;
	if (socket){
		SOCKADDR_IN sockAddr = {0};
		int nSockAddrLen = sizeof(sockAddr);
		socket->GetPeerName((SOCKADDR*)&sockAddr, &nSockAddrLen);
		SetIP(sockAddr.sin_addr.S_un.S_addr);

		//EastShare Start - added by AndCycle, IP to Country
		if(m_nConnectIP != m_dwUserIP){
			m_structUserCountry = CGlobalVariable::ip2country->GetCountryFromIP(m_dwUserIP);
			}
		//EastShare End - added by AndCycle, IP to Country

	}
	else{
		//SetIP(0);
		m_dwUserIP = 0;
		m_nConnectIP = 0;
	}
	m_nLanConnectIP = 0;
	m_fHashsetRequesting = 0;
	m_fSharedDirectories = 0;
	m_fSentCancelTransfer = 0;
	m_nClientVersion = 0;
	m_lastRefreshedDLDisplay = 0;
	m_dwDownStartTime = 0;
	m_LastGetBlockReqTime = 0;
	m_nLastBlockOffset = (uint64)-1;
	m_bUnicodeSupport = false;
	m_dwLastSignatureIP = 0;
	m_bySupportSecIdent = 0;
	m_byInfopacketsReceived = IP_NONE;
	m_lastPartAsked = (uint16)-1;
	m_nUpCompleteSourcesCount= 0;
	m_fSupportsPreview = 0;
	m_fPreviewReqPending = 0;
	m_fPreviewAnsPending = 0;
	m_bTransferredDownMini = false;
    m_addedPayloadQueueSession = 0;
    m_nCurQueueSessionPayloadUp = 0; // PENDING: Is this necessary? ResetSessionUp()...
    m_lastRefreshedULDisplay = ::GetTickCount();
	m_bGPLEvildoer = false;
	m_bHelloAnswerPending = false;
	m_fNoViewSharedFiles = 0;
	m_bMultiPacket = 0;
	md4clr(requpfileid);
	m_nTotalUDPPackets = 0;
	m_nFailedUDPPackets = 0;
	m_nUrlStartPos = (uint64)-1;
	m_iHttpSendState = 0;
	m_fPeerCache = 0;
	m_uPeerCacheDownloadPushId = 0;
	m_uPeerCacheUploadPushId = 0;
	m_pPCDownSocket = NULL;
	m_pPCUpSocket = NULL;
	m_uPeerCacheRemoteIP = 0;
	m_bPeerCacheDownHit = false;
	m_bPeerCacheUpHit = false;
	m_fNeedOurPublicIP = 0;
    m_random_update_wait = (uint32)(rand()/(RAND_MAX/1000));
    m_bSourceExchangeSwapped = false; // ZZ:DownloadManager
    m_dwLastTriedToConnect = ::GetTickCount()-20*60*1000; // ZZ:DownloadManager
	m_fQueueRankPending = 0;
	m_fUnaskQueueRankRecv = 0;
	m_fFailedFileIdReqs = 0;
    m_slotNumber = 0;
    lastSwapForSourceExchangeTick = 0;
	m_pReqFileAICHHash = NULL;
	m_fSupportsAICH = 0;
	m_fAICHRequested = 0;
	m_byKadVersion = 0;
	m_fSentOutOfPartReqs = 0;
	m_bCollectionUploadSlot = false;
	m_fSupportsLargeFiles = 0;
	m_fExtMultiPacket = 0;
	m_fRequestsCryptLayer = 0;
	m_fSupportsCryptLayer = 0;
	m_fRequiresCryptLayer = 0;
	m_fSupportsSourceEx2 = 0;
	//m_fSupportsCaptcha = 0;
	m_fDirectUDPCallback = 0;
	//m_cCaptchasSent = 0;

	m_iErrTimes = 0; 

	// VC-yunchenn.chen[2006-12-30]
	m_nSupportNatTraverse = SNT_UNKNOWN;
	m_nActiveConnectType  = ACT_UNKNOWN;

	m_pBlockRangeToDo = NULL;
	
	// VC-SearchDream[2007-04-06]: 
	m_pSourceExchangeClient = NULL;

	m_bLowToLowClient       = false;  // VC-SearchDream[2007-04-13]: Is LowToLow Client Or Not  

	m_dwSendKeepALiveTime   = time(NULL);   // VC-SearchDream[2007-04-18]: For SourceExchange NAT

	m_dwErrorCount = 0;
	m_dwLastErrorCount = 0;
	m_eLastError = erNoError;

	this->m_dwLastConnectInitTime = 0;
}

CUpDownClient::~CUpDownClient(){

#ifdef _DEBUG_PEER
	Debug( _T("Delete Peer(%d)-PeerType(%d)-ErrTimes(%d) \n"),m_iPeerIndex,m_iPeerType,m_iErrTimes);
#endif

	//CMessageLog::GetInstace()->RemoveTag(this);


	//ADDED by VC-fengwen on 2007/10/26 <begin> : 断开相互的关联
	SeverSrcExchangeWithHelper();
	SeverSrcExchangeWithAsker();
	//ADDED by VC-fengwen on 2007/10/26 <end> : 断开相互的关联

	if (IsAICHReqPending()){
		m_fAICHRequested = FALSE;
		CAICHHashSet::ClientAICHRequestFailed(this);
	}

	CGlobalVariable::clientlist->RemoveClient(this, _T("Destructing client object"));

/*	if (m_Friend)
        m_Friend->SetLinkedClient(NULL);*/

	if (socket){
		ASSERT( socket->client==this );
		//socket->client = 0;
		socket->SetDisconnetAll();
		socket->Safe_Delete();
	}
	else if( CGlobalVariable::natthread /*&& this peer is NatTrans Peer*/ )			
	{
		// VC-Huby[2006-12-21]: 由于emule的P2P之间 既有tcp,又有udp通信，因此low2low通信的时候(包括FakeTcp,Udp),
		// 不能在没有FakeTcp通信的时候就把底层打洞的Session删除，需要在完全不和该peer通信的时候,才可删除	
		// 因此为保证L2L-udp通信,Nat底层中 NatSocketPool有可能还有 natsocket 对象,现在可移除
		CGlobalVariable::natthread->RemoveNatSocket(GetUserHash());
	}

	if (m_pPCDownSocket){
		m_pPCDownSocket->client = NULL;
		m_pPCDownSocket->Safe_Delete();
	}
	if (m_pPCUpSocket){
		m_pPCUpSocket->client = NULL;
		m_pPCUpSocket->Safe_Delete();
	}
	
	free(m_pszUsername);
	
	delete[] m_abyPartStatus;
	m_abyPartStatus = NULL;
	
	delete[] m_abyUpPartStatus;
	m_abyUpPartStatus = NULL;
	
	ClearUploadBlockRequests();

	for (POSITION pos = m_DownloadBlocks_list.GetHeadPosition();pos != 0;)
		delete m_DownloadBlocks_list.GetNext(pos);
	
	for (POSITION pos = m_RequestedFiles_list.GetHeadPosition();pos != 0;)
		delete m_RequestedFiles_list.GetNext(pos);
	
	for (POSITION pos = m_PendingBlocks_list.GetHeadPosition();pos != 0;){
		Pending_Block_Struct *pending = m_PendingBlocks_list.GetNext(pos);
		delete pending->block;
		// Not always allocated
		if (pending->zStream){
			inflateEnd(pending->zStream);
			delete pending->zStream;
		}
		delete pending;
	}

	for (POSITION pos = m_WaitingPackets_list.GetHeadPosition();pos != 0;)
		delete m_WaitingPackets_list.GetNext(pos);
	
	DEBUG_ONLY (CGlobalVariable::listensocket->Debug_ClientDeleted(this));
	SetUploadFileID(NULL);

    m_fileReaskTimes.RemoveAll(); // ZZ:DownloadManager (one resk timestamp for each file)

	if(m_pReqFileAICHHash)
		delete m_pReqFileAICHHash;

	while(!m_EventList.IsEmpty())
	{
		delete m_EventList.GetHead();
		m_EventList.RemoveHead();
	}

	//Changed by thilon on 2008.01.19, for 调整CMessageLog中RemoveTag函数的调用顺序
	CMessageLog::GetInstace()->RemoveTag(this);
}

bool CUpDownClient::Disconnected(LPCTSTR pszReason, bool bFromSocket ,CClientReqSocket* /* pSocket */)
{
	ASSERT( CGlobalVariable::clientlist->IsValidClient(this) );

	// TODO LOGREMOVE
	//if (m_nConnectingState == CCS_DIRECTCALLBACK)
	//	DebugLog(_T("Direct Callback failed - %s"), DbgGetClientInfo());

	if (GetKadState() == KS_QUEUED_FWCHECK_UDP || GetKadState() == KS_CONNECTING_FWCHECK_UDP)
		Kademlia::CUDPFirewallTester::SetUDPFWCheckResult(false, true, ntohl(GetConnectIP()), 0); // inform the tester that this test was cancelled
	else if (GetKadState() == KS_FWCHECK_UDP)
		Kademlia::CUDPFirewallTester::SetUDPFWCheckResult(false, false, ntohl(GetConnectIP()), 0); // inform the tester that this test has failed
	else if (GetKadState() == KS_CONNECTED_BUDDY)
		DebugLogWarning(_T("Buddy client disconnected - %s, %s"), pszReason, DbgGetClientInfo());
	else if(GetKadState() == KS_CONNECTING_BUDDY)
		CGlobalVariable::clientlist->AddBadBuddy( m_nConnectIP,m_nUserPort,m_nUDPPort,m_achUserHash );

	//If this is a KAD client object, just delete it!
	SetKadState(KS_NONE);

	if (GetUploadState() == US_UPLOADING || GetUploadState() == US_CONNECTING)
	{
		//if (thePrefs.GetLogUlDlEvents() && GetUploadState()==US_UPLOADING && m_fSentOutOfPartReqs==0 && !CGlobalVariable::uploadqueue->IsOnUploadQueue(this))
		//	DebugLog(_T("Disconnected client removed from upload queue and waiting list: %s"), DbgGetClientInfo());
		CGlobalVariable::uploadqueue->RemoveFromUploadQueue(this, CString(_T("CUpDownClient::Disconnected: ")) + pszReason);
	}

	// 28-Jun-2004 [bc]: re-applied this patch which was in 0.30b-0.30e. it does not seem to solve the bug but
	// it does not hurt either...
	if (m_BlockRequests_queue.GetCount() > 0 || m_DoneBlocks_list.GetCount()){
		// Although this should not happen, it seems(?) to happens sometimes. The problem we may run into here is as follows:
		//
		// 1.) If we do not clear the block send requests for that client, we will send those blocks next time the client
		// gets an upload slot. But because we are starting to send any available block send requests right _before_ the
		// remote client had a chance to prepare to deal with them, the first sent blocks will get dropped by the client.
		// Worst thing here is, because the blocks are zipped and can therefore only be uncompressed when the first block
		// was received, all of those sent blocks will create a lot of uncompress errors at the remote client.
		//
		// 2.) The remote client may have already received those blocks from some other client when it gets the next
		// upload slot.
		DebugLogWarning(_T("Disconnected client with non empty block send queue; %s reqs: %s doneblocks: %s"), DbgGetClientInfo(), m_BlockRequests_queue.GetCount() > 0 ? _T("true") : _T("false"), m_DoneBlocks_list.GetCount() ? _T("true") : _T("false"));
		ClearUploadBlockRequests();
	}

	if (GetDownloadState() == DS_DOWNLOADING){
		ASSERT( m_nConnectingState == CCS_NONE );
		if (m_ePeerCacheDownState == PCDS_WAIT_CACHE_REPLY || m_ePeerCacheDownState == PCDS_DOWNLOADING)
			CGlobalVariable::m_pPeerCache->DownloadAttemptFailed();
		SetDownloadState(DS_ONQUEUE, CString(_T("Disconnected: ")) + pszReason);
		m_iErrTimes++;
	}
	else{
		// ensure that all possible block requests are removed from the partfile
		ClearDownloadBlockRequests();

		if (GetDownloadState() == DS_CONNECTED || GetDownloadState()==DS_CONNECTING )
		{
			m_iErrTimes++;
			if(m_iErrTimes>=2)
			{
				CGlobalVariable::clientlist->m_globDeadSourceList.AddDeadSource(this);
				CGlobalVariable::downloadqueue->RemoveSource(this);
			}		
			SetDownloadState(DS_ERROR);
		}
		else if( GetDownloadState()==DS_LOWTOLOWIP )
		{
			m_iErrTimes++;
		}
	}

	// we had still an AICH request pending, handle it
	if (IsAICHReqPending()){
		m_fAICHRequested = FALSE;
		CAICHHashSet::ClientAICHRequestFailed(this);
	}

	// The remote client does not have to answer with OP_HASHSETANSWER *immediatly* 
	// after we've sent OP_HASHSETREQUEST. It may occure that a (buggy) remote client 
	// is sending use another OP_FILESTATUS which would let us change to DL-state to DS_ONQUEUE.
	if (((GetDownloadState() == DS_REQHASHSET) || m_fHashsetRequesting) && (reqfile != NULL))
		reqfile->hashsetneeded = true;

    if (m_iFileListRequested){
		LogWarning(LOG_STATUSBAR, GetResString(IDS_SHAREDFILES_FAILED), GetUserName());
        m_iFileListRequested = 0;
	}

/*
	if (m_Friend)
		CGlobalVariable::friendlist->RefreshFriend(m_Friend);
*/

	ASSERT( CGlobalVariable::clientlist->IsValidClient(this) );

	//check if this client is needed in any way, if not delete it
	bool bDelete = true;
	switch(m_nUploadState){
		case US_ONUPLOADQUEUE:
			bDelete = false;
			break;
	}
	switch(m_nDownloadState){
		case DS_ONQUEUE:
		case DS_TOOMANYCONNS:
		case DS_NONEEDEDPARTS:
		case DS_LOWTOLOWIP:
		case DS_ERROR:
			m_nConnectingState = CCS_NONE;
			if( m_nDownloadState==DS_LOWTOLOWIP && m_nSupportNatTraverse==SNT_UNSUPPORT ) //[VC-Huby-081211]:不支持L2L穿越的下载源先删除,避免占了源限制
				bDelete = true;
			else if( m_nDownloadState==DS_ERROR && m_iErrTimes>=2 )
				bDelete = true;
			else
				bDelete = false;

	}

	// Dead Soure Handling
	//
	// If we failed to connect to that client, it is supposed to be 'dead'. Add the IP
	// to the 'dead sources' lists so we don't waste resources and bandwidth to connect
	// to that client again within the next hour.
	//
	// But, if we were just connecting to a proxy and failed to do so, that client IP
	// is supposed to be valid until the proxy itself tells us that the IP can not be
	// connected to (e.g. 504 Bad Gateway)
	//
	if ( (m_nConnectingState != CCS_NONE && !(socket && socket->GetProxyConnectFailed()))
		|| m_nDownloadState == DS_ERROR)
	{
		if (m_nDownloadState != DS_NONE) // Unable to connect = Remove any downloadstate
			CGlobalVariable::downloadqueue->RemoveSource(this);
		CGlobalVariable::clientlist->m_globDeadSourceList.AddDeadSource(this);
		bDelete = true;
	}

	// We keep chat partners in any case
/*
	if (GetChatState() != MS_NONE){
		bDelete = false;
		if (GetFriend() != NULL && GetFriend()->IsTryingToConnect())
			GetFriend()->UpdateFriendConnectionState(FCR_DISCONNECTED); // for friends any connectionupdate is handled in the friend class
		else
			theApp.emuledlg->chatwnd->chatselector.ConnectingResult(this,false); // other clients update directly
	}
*/


	// Delete Socket
	if (!bFromSocket && socket){
		ASSERT( CGlobalVariable::listensocket->IsValidSocket(socket) );
		socket->Safe_Delete();
	}
	socket = NULL;

	//  Comment UI
	//SendMessage(CGlobalVariable::m_hListenWnd,WM_FILE_UPDATE_PEER,0,(LPARAM)this);
	UpdateUI(UI_UPDATE_PEERLIST);
	//theApp.emuledlg->transferwnd->clientlistctrl.RefreshClient(this);

#ifdef _DEBUG
/*
	if( _tcscmp(pszReason,GetResString(IDS_TOOMANYCONNS))==0 )
		AddPeerLog( new CTraceError(GetResString(IDS_TOOMANYCONNS)) );
	else
*/
#endif

	if( GetDownloadState()!=DS_TOOMANYCONNS )
		AddPeerLog( new CTraceError(GetResString(IDS_DISCONNECTED)) );
	
	// finally, remove the client from the timeouttimer and reset the connecting state
	m_nConnectingState = CCS_NONE;
	CGlobalVariable::clientlist->RemoveConnectingClient(this);

#ifdef _DEBUG_PEER
	Debug( _T("Peer(%d) Disconnected(%s -%s- %d),SrcFrom=%d(%d),bDelete(%x),ErrTimes=%d \n"),m_iPeerIndex,m_pszUsername,pszReason,GetUserPort(),
		m_nSourceFrom,m_iPeerType,bDelete,m_iErrTimes );
#endif

	if (bDelete)
	{
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			Debug(_T("--- Deleted client            %s; Reason=%s\n"), DbgGetClientInfo(true), pszReason);
		return true;
	}
	else
	{
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			Debug(_T("--- Disconnected client       %s; Reason=%s\n"), DbgGetClientInfo(true), pszReason);
		m_fHashsetRequesting = 0;
		SetSentCancelTransfer(0);
		m_bHelloAnswerPending = false;
		m_fQueueRankPending = 0;
		m_fFailedFileIdReqs = 0;
		m_fUnaskQueueRankRecv = 0;
		m_uPeerCacheDownloadPushId = 0;
		m_uPeerCacheUploadPushId = 0;
		m_uPeerCacheRemoteIP = 0;
		SetPeerCacheDownState(PCDS_NONE);
		SetPeerCacheUpState(PCUS_NONE);
		if (m_pPCDownSocket){
			m_pPCDownSocket->client = NULL;
			m_pPCDownSocket->Safe_Delete();
		}
		if (m_pPCUpSocket){
			m_pPCUpSocket->client = NULL;
			m_pPCUpSocket->Safe_Delete();
		}
		m_fSentOutOfPartReqs = 0;
		return false;
	}
}

//Returned bool is not if the TryToConnect is successful or not..
//false means the client was deleted!
//true means the client was not deleted!
bool CUpDownClient::TryToConnect(bool bIgnoreMaxCon, bool bNoCallbacks, CRuntimeClass* pClassSocket)
{
	// There are 7 possible ways how we are going to connect in this function, sorted by priority:
	// 1) Already Connected/Connecting
	//		We are already connected or try to connect right now. Abort, no additional Disconnect() call will be done
	// 2) Immediate Fail
	//		Some precheck or precondition failed, or no other way is available, so we do not try to connect at all
	//		but fail right away, possibly deleting the client as it becomes useless
	// 3) Normal Outgoing TCP Connection
	//		Applies to all HighIDs/Open clients: We do a straight forward connection try to the TCP port of the client
	// 4) Direct Callback Connections
	//		Applies to TCP firewalled - UDP open clients: We sent a UDP packet to the client, requesting him to connect
	//		to us. This is pretty easy too and ressourcewise nearly on the same level as 3)
	// (* 5) Waiting/Abort
	//		This check is done outside this function.
	//		We want to connect for some download related thing (for example reasking), but the client has a LowID and
	//		is on our uploadqueue. So we are smart and safing ressources by just waiting untill he reasks us, so we don't
	//		have to do the ressource intensive options 6 or 7. *)
	// 6) Server Callback
	//		This client is firewalled, but connected to our server. We sent the server a callback request to forward to
	//		the client and hope for the best
	// 7) Kad Callback
	//		This client is firewalled, but has a Kad buddy. We sent the buddy a callback request to forward to the client
	//		and hope for the best

	// 8) VeryCD mod support SourceExchange callback
	// 9) VeryCD mod support Low2Low Connect...

	if( GetKadState() == KS_QUEUED_FWCHECK )
		SetKadState(KS_CONNECTING_FWCHECK);
	else if (GetKadState() == KS_QUEUED_FWCHECK_UDP)
		SetKadState(KS_CONNECTING_FWCHECK_UDP);

	////////////////////////////////////////////////////////////
	// Check for 1) Already Connected/Connecting
	if (m_nConnectingState != CCS_NONE) {
		DebugLog(_T("TryToConnect: Already Connecting (%s)"), DbgGetClientInfo());// TODO LogRemove
		return true;
	}
	else if (socket != NULL){
		if (socket->IsConnected())
	{
			if (CheckHandshakeFinished()){
				DEBUG_ONLY( DebugLog(_T("TryToConnect: Already Connected (%s)"), DbgGetClientInfo()) );// TODO LogRemove
				ConnectionEstablished();
			}
			else
				DebugLogWarning( _T("TryToConnect found connected socket, but without Handshake finished - %s"), DbgGetClientInfo());
			return true;
		}
		else
			socket->Safe_Delete();
	}
	m_nConnectingState = CCS_PRECONDITIONS; // We now officially try to connect :)

	////////////////////////////////////////////////////////////
	// Check for 2) Immediate Fail

	if (CGlobalVariable::listensocket->TooManySockets() && !bIgnoreMaxCon)
	{
		// This is a sanitize check and counts as a "hard failure", so this check should be also done before calling
		// TryToConnect if a special handling, like waiting till there are enough connection avaiable should be fone
		DebugLogWarning(_T("TryToConnect: Too many connections sanitize check (%s)"), DbgGetClientInfo());
		if(Disconnected(GetResString(IDS_TOOMANYCONNS)))
		{
			delete this;
			return false;
		}
		return true;
	}
	// do not try to connect to source which are incompatible with our encryption setting (one requires it, and the other one doesn't supports it)
	if ( (RequiresCryptLayer() && !thePrefs.IsClientCryptLayerSupported()) || (thePrefs.IsClientCryptLayerRequired() && !SupportsCryptLayer()) )
	{
		DEBUG_ONLY( AddDebugLogLine(DLP_DEFAULT, false, _T("Rejected outgoing connection because CryptLayer-Setting (Obfuscation) was incompatible %s"), DbgGetClientInfo()) );
		if(Disconnected(_T("CryptLayer-Settings (Obfuscation) incompatible"))){
			delete this;
			return false;
		}
		else
			return true;
	}

	this->m_dwLastConnectInitTime = GetTickCount();

	uint32 uClientIP = (GetIP() != 0) ? GetIP() : GetConnectIP();
	if (uClientIP == 0 && !HasLowID())
		uClientIP = ntohl(m_nUserIDHybrid);
	if (uClientIP)
	{
		// although we filter all received IPs (server sources, source exchange) and all incomming connection attempts,
		// we do have to filter outgoing connection attempts here too, because we may have updated the ip filter list
		if (CGlobalVariable::ipfilter->IsFiltered(uClientIP))
		{
			theStats.filteredclients++;
			if (thePrefs.GetLogFilteredIPs())
				AddDebugLogLine(true, GetResString(IDS_IPFILTERED), ipstr(uClientIP), CGlobalVariable::ipfilter->GetLastHit());
			if (Disconnected(_T("IPFilter")))
			{
				delete this;
				return false;
			}
			return true;
		}

		// for safety: check again whether that IP is banned
		if (CGlobalVariable::clientlist->IsBannedClient(uClientIP))
		{
			if (thePrefs.GetLogBannedClients())
				AddDebugLogLine(false, _T("Refused to connect to banned client %s"), DbgGetClientInfo());
			if (Disconnected(_T("Banned IP")))
			{
				delete this;
				return false;
			}
			return true;
		}
	}
	
	if( !PrepareActiveConnectType() ) 
		return false;

	if ( HasLowID() )	
	{
		if( ACT_FAKETCP==m_nActiveConnectType ) // 9) VeryCD mod support Low2Low Connect...
		{
			//  added by yunchenn, 2006/11/23
			//  try to connect by NAT traversal
			bool bLow2LowConn=false;

			//We cannot do a callback!
			if (GetDownloadState() == DS_CONNECTING)
			{
				bLow2LowConn = true;
			}
			else if (GetDownloadState() == DS_REQHASHSET)
			{
				SetDownloadState(DS_ONQUEUE);
				reqfile->hashsetneeded = true;
			}
			if (GetUploadState() == US_CONNECTING)
			{
				//  added by yunchenn, 2006/12/18
				bLow2LowConn = true;
				/*if(Disconnected(_T("LowID->LowID and US_CONNECTING")))
				{
					delete this;
					return false;
				}*/
			}

			//  added by yunchenn, 2006/12/18
			//  try to connect by NAT traversal
			if(bLow2LowConn)
			{
				if(socket && CGlobalVariable::natthread)
				{
					if(CGlobalVariable::natthread->IsSocketConnected(socket))
					{
						if (CheckHandshakeFinished())
							ConnectionEstablished();
						return true;
					}
				}
				
				if (!socket || !socket->IsConnected())
				{
					if (socket)
						socket->Safe_Delete();
					if (pClassSocket == NULL)
						pClassSocket = RUNTIME_CLASS(CClientReqSocket);
					socket = static_cast<CClientReqSocket*>(pClassSocket->CreateObject());
					socket->SetClient(this);
					socket->m_bUseNat = true; //[VC-Huby-080321]: 一定要在Create之前设置 m_bUseNat,否则会让L2L也创建没用的tcp handle(m_SocketData.hSocket) 
					if (!socket->Create())
					{
						socket->Safe_Delete();
						return true;
					}					
				}	
				else
				{
#ifdef _DEBUG_PEER					
					ASSERT( 0 ); //the AsyncSocketEx is connected,but natthread->IsSocketConnected return false
#endif
					socket->m_bUseNat = true;					
				}
				
				ASSERT(socket->m_bUseNat);
				ASSERT(!socket->IsValidSocket());

				CGlobalVariable::clientlist->AddConnectingClient(this);
				m_nConnectingState = CCS_FAKETCP;
				Connect();
			}
			return true;
		}


		// are callbacks disallowed?
		if (bNoCallbacks){
			DebugLogError(_T("TryToConnect: Would like to do callback on a no-callback client, %s"), DbgGetClientInfo());
			if(Disconnected(_T("LowID: No Callback Option allowed")))
			{
				delete this;
				return false;
			}
			return true;
		}

		// Is any callback available?
		if (!( (SupportsDirectUDPCallback() && thePrefs.GetUDPPort() != 0 && GetConnectIP() != 0) // Direct Callback
			|| (HasValidBuddyID() && Kademlia::CKademlia::IsConnected()) // Kad Callback
			|| CGlobalVariable::serverconnect->IsLocalServer(GetServerIP(), GetServerPort()) )) // Server Callback
		{
			// Nope
			if(Disconnected(_T("LowID: No Callback Option available")))
			{
				delete this;
				return false;
			}
			return true;
		}
	}

	// Prechecks finished, now for the real connecting
	////////////////////////////////////////////////////

	CGlobalVariable::clientlist->AddConnectingClient(this); // Starts and checks for the timeout, ensures following Disconnect() or ConnectionEstablished() call

	////////////////////////////////////////////////////////////
	// 3) Normal Outgoing TCP Connection(注意:VeryCD mod 支持局域网内直连)
	if ( !HasLowID() || ACT_DIRECTTCP==m_nActiveConnectType )
	{
		m_nConnectingState = CCS_DIRECTTCP;

		if (pClassSocket == NULL)
			pClassSocket = RUNTIME_CLASS(CClientReqSocket);
		socket = static_cast<CClientReqSocket*>(pClassSocket->CreateObject());
		socket->SetClient(this);
		if (!socket->Create())
		{
			socket->Safe_Delete();
			// we let the timeout handle the cleanup in this case
			DebugLogError(_T("TryToConnect: Failed to create socket for outgoing connection, %s"), DbgGetClientInfo());
		}
		else
			Connect();
		return true;
	}
	////////////////////////////////////////////////////////////
	// 4) Direct Callback Connections
	else if (SupportsDirectUDPCallback() && thePrefs.GetUDPPort() != 0 && GetConnectIP() != 0)
	{
		m_nConnectingState = CCS_DIRECTCALLBACK;
		// TODO LOGREMOVE
		DebugLog(_T("Direct Callback on port %u to client %s (%s) "), GetKadPort(), DbgGetClientInfo(), md4str(GetUserHash()));
		CSafeMemFile data;
		data.WriteUInt16(thePrefs.GetPort()); // needs to know our port
		data.WriteHash16(thePrefs.GetUserHash()); // and userhash
		// our connection settings
		data.WriteUInt8(GetMyConnectOptions(true, false));
		if (thePrefs.GetDebugClientUDPLevel() > 0)
			DebugSend("OP_DIRECTCALLBACKREQ", this);
		Packet* packet = new Packet(&data, OP_EMULEPROT);
		packet->opcode = OP_DIRECTCALLBACKREQ;
		theStats.AddUpDataOverheadOther(packet->size);
		CGlobalVariable::clientudp->SendPacket(packet, GetConnectIP(), GetKadPort(), ShouldReceiveCryptUDPPackets(), GetUserHash(), false, 0);
		return true;
	}

	////////////////////////////////////////////////////////////
	// 6) Server Callback + 7) Kad Callback +8) VeryCD SourceExchangeCallback
	if (GetDownloadState() == DS_CONNECTING)
		SetDownloadState(DS_WAITCALLBACK);

	if (GetUploadState() == US_CONNECTING){
		ASSERT( false ); // we should never try to connect in this case, but wait for the LowID to connect to us
		DebugLogError( _T("LowID and US_CONNECTING (%s)"), DbgGetClientInfo());
	}

	if( CGlobalVariable::serverconnect->IsLocalServer(m_dwServerIP, m_nServerPort))
	{
		m_nConnectingState = CCS_SERVERCALLBACK;
		Packet* packet = new Packet(OP_CALLBACKREQUEST,4);
		PokeUInt32(packet->pBuffer, m_nUserIDHybrid);
		if (thePrefs.GetDebugServerTCPLevel() > 0 || thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend("OP__CallbackRequest", this);
		theStats.AddUpDataOverheadServer(packet->size);
		CGlobalVariable::serverconnect->SendPacket(packet);
		return true;
	}
	// VC-SearchDream[2007-04-07]: For Source Exchange Call Back Begin
	else if (m_nSourceFrom == SF_SOURCE_EXCHANGE && m_pSourceExchangeClient != NULL
		&& NULL != m_pSourceExchangeClient->socket) //ADDED by VC-fengwen on 2007/10/26 : 
	{
		CSafeMemFile data_out;
		data_out.WriteHash16(GetUserHash());
		Packet* callback = new Packet(&data_out, OP_EMULEPROT, OP_TCPSECALLBACKREQ);
		m_pSourceExchangeClient->socket->SendPacket(callback, true, true, 0, true);
		return true;
	}
	// VC-SearchDream[2007-04-07]: For Source Exchange Call Back End
	else if (HasValidBuddyID() && Kademlia::CKademlia::IsConnected())
	{
		m_nConnectingState = CCS_KADCALLBACK;
		if( GetBuddyIP() && GetBuddyPort())
		{
			CSafeMemFile bio(34);
			bio.WriteUInt128(&Kademlia::CUInt128(GetBuddyID()));
			bio.WriteUInt128(&Kademlia::CUInt128(reqfile->GetFileHash()));
			bio.WriteUInt16(thePrefs.GetPort());
			if (thePrefs.GetDebugClientKadUDPLevel() > 0 || thePrefs.GetDebugClientUDPLevel() > 0)
				DebugSend("KadCallbackReq", this);
			Packet* packet = new Packet(&bio, OP_KADEMLIAHEADER);
			packet->opcode = KADEMLIA_CALLBACK_REQ;
			theStats.AddUpDataOverheadKad(packet->size);
			// FIXME: We dont know which kadversion the buddy has, so we need to send unencrypted
			CGlobalVariable::clientudp->SendPacket(packet, GetBuddyIP(), GetBuddyPort(), false, NULL, true, 0);
			SetDownloadState(DS_WAITCALLBACKKAD);
		}
		else
		{
			// I don't think we should ever have a buddy without its IP (anymore), but nevertheless let the functionality in
			//Create search to find buddy.
			Kademlia::CSearch *findSource = new Kademlia::CSearch;
			findSource->SetSearchTypes(Kademlia::CSearch::FINDSOURCE);
			findSource->SetTargetID(Kademlia::CUInt128(GetBuddyID()));
			findSource->AddFileID(Kademlia::CUInt128(reqfile->GetFileHash()));
			if( Kademlia::CKademlia::GetPrefs()->GetTotalSource() > 0 || Kademlia::CSearchManager::AlreadySearchingFor(Kademlia::CUInt128(GetBuddyID())))
			{
				//There are too many source lookups already or we are already searching this key.
				// bad luck, as lookups aren't supposed to hapen anyway, we just let it fail, if we want
				// to actually really use lookups (so buddies without known IPs), this should be reworked
				// for example by adding a queuesystem for queries
				DebugLogWarning(_T("TryToConnect: Buddy without knonw IP, Lookup crrently impossible"));
				return true;
			}
			if(Kademlia::CSearchManager::StartSearch(findSource))
			{
				//Started lookup..
				SetDownloadState(DS_WAITCALLBACKKAD);
			}
			else
			{
				//This should never happen..
				ASSERT(0);
			}
		}
		return true;
	}
	else {
		ASSERT( false );
		DebugLogError(_T("TryToConnect: Bug: No Callback available despite prechecks"));
		return true;
	}
}

bool CUpDownClient::Connect()
{
	if (!socket->m_bUseNat) // VC-SearchDream[2007-05-18]: NAT need not to do this
	{
		// enable or disable crypting based on our and the remote clients preference
		if (HasValidHash() && SupportsCryptLayer() && thePrefs.IsClientCryptLayerSupported() && (RequestsCryptLayer() || thePrefs.IsClientCryptLayerRequested()))
		{
			DebugLog(_T("Enabling CryptLayer on outgoing connection to client %s"), DbgGetClientInfo()); // to be removed later
			socket->SetConnectionEncryption(true, GetUserHash(), false);
		}
		else
		{
			socket->SetConnectionEncryption(false, NULL, false);
		}
	}

	//Try to always tell the socket to WaitForOnConnect before you call Connect.
	socket->WaitForOnConnect();
	SOCKADDR_IN sockAddr = {0};
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons(GetUserPort());
	if( m_nSourceFrom==SF_LAN )
		sockAddr.sin_addr.S_un.S_addr = m_nLanConnectIP;
	else
		sockAddr.sin_addr.S_un.S_addr = GetConnectIP();

	//  added by yunchenn
	if(socket->m_bUseNat)
	{
		socket->Connect((SOCKADDR*)m_achUserHash, 16);
	}
	else 
	{
		socket->Connect((SOCKADDR*)&sockAddr, sizeof sockAddr);
		if (!SendHelloPacket())
		{   
			return false; // client was deleted!
		}
	}
	CString temp;
	if( m_nSourceFrom==SF_LAN )
		temp.Format(GetResString(IDS_CONNECT_INFO),ipstr(m_nLanConnectIP),m_nUserPort);
	else
		temp.Format(GetResString(IDS_CONNECT_INFO),ipstr(m_nConnectIP),m_nUserPort);
#ifdef _DEBUG
    if(socket->m_bUseNat)
		temp += _T("-L2L");
	else if( m_nSourceFrom==SF_LAN )
		temp += _T("-LAN");
#endif
	AddPeerLog(new CTraceInformation(temp));

#ifdef _DEBUG_PEER
	Debug( _T("Peer(%d) Connect to %s(%d),ErrTimes=%d \n"),m_iPeerIndex,m_pszUsername,GetUserPort(),m_iErrTimes );
#endif

	return true;
}

void CUpDownClient::ConnectionEstablished()
{
	// ok we have a connection, lets see if we want anything from this client
	
	// was this a direct callback?
	if (m_nConnectingState == CCS_DIRECTCALLBACK) // TODO LOGREMOVE
		DebugLog(_T("Direct Callback succeeded, connection established - %s"), DbgGetClientInfo()); 

	// remove the connecting timer and state
	//if (m_nConnectingState == CCS_NONE) // TODO LOGREMOVE
	//	DEBUG_ONLY( DebugLog(_T("ConnectionEstablished with CCS_NONE (incoming, thats fine)")) );
	m_nConnectingState = CCS_NONE;
	CGlobalVariable::clientlist->RemoveConnectingClient(this);

	// check if we should use this client to retrieve our public IP
	//  Comment UI
	if (CGlobalVariable::GetPublicIP() == 0 && CGlobalVariable::IsConnected() && m_fPeerCache)
		SendPublicIPRequest();

	switch(GetKadState())
	{
		case KS_CONNECTING_FWCHECK:
            SetKadState(KS_CONNECTED_FWCHECK);
			break;
		case KS_CONNECTING_BUDDY:
		case KS_INCOMING_BUDDY:
			DEBUG_ONLY( DebugLog(_T("Set KS_CONNECTED_BUDDY for client %s"), DbgGetClientInfo()) );
			SetKadState(KS_CONNECTED_BUDDY);
			break;
		case KS_CONNECTING_FWCHECK_UDP:
			SetKadState(KS_FWCHECK_UDP);
			DEBUG_ONLY( DebugLog(_T("Set KS_FWCHECK_UDP for client %s"), DbgGetClientInfo()) );
			SendFirewallCheckUDPRequest();
			break;
	}
/*
	if (GetChatState() == MS_CONNECTING || GetChatState() == MS_CHATTING)
	{
		if (GetFriend() != NULL && GetFriend()->IsTryingToConnect()){
			GetFriend()->UpdateFriendConnectionState(FCR_ESTABLISHED); // for friends any connectionupdate is handled in the friend class
			if (credits != NULL && credits->GetCurrentIdentState(GetConnectIP()) == IS_IDFAILED)
				GetFriend()->UpdateFriendConnectionState(FCR_SECUREIDENTFAILED);
		}
		else
			theApp.emuledlg->chatwnd->chatselector.ConnectingResult(this, true); // other clients update directly
	}
*/
	switch(GetDownloadState())
	{
		case DS_CONNECTING:
		case DS_WAITCALLBACK:
		case DS_WAITCALLBACKKAD:
			m_bReaskPending = false;
			SetDownloadState(DS_CONNECTED);
			SendFileRequest();
			break;
	}

	if (m_bReaskPending)
	{
		m_bReaskPending = false;
		if (GetDownloadState() != DS_NONE && GetDownloadState() != DS_DOWNLOADING)
		{
			SetDownloadState(DS_CONNECTED);
			SendFileRequest();
		}
	}

	switch(GetUploadState())
	{
		case US_CONNECTING:
		//case US_WAITCALLBACK:
			if (CGlobalVariable::uploadqueue->IsDownloading(this))
			{
				SetUploadState(US_UPLOADING);
				if (thePrefs.GetDebugClientTCPLevel() > 0)
					DebugSend("OP__AcceptUploadReq", this);
				Packet* packet = new Packet(OP_ACCEPTUPLOADREQ,0);
				theStats.AddUpDataOverheadFileRequest(packet->size);
				socket->SendPacket(packet,true);
			}
	}

	if (m_iFileListRequested == 1)
	{
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend(m_fSharedDirectories ? "OP__AskSharedDirs" : "OP__AskSharedFiles", this);
        Packet* packet = new Packet(m_fSharedDirectories ? OP_ASKSHAREDDIRS : OP_ASKSHAREDFILES,0);
		theStats.AddUpDataOverheadOther(packet->size);
		socket->SendPacket(packet,true,true);
	}

	while (!m_WaitingPackets_list.IsEmpty())
	{
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend("Buffered Packet", this);
		socket->SendPacket(m_WaitingPackets_list.RemoveHead());
	}

}

void CUpDownClient::InitClientSoftwareVersion()
{
	if (m_pszUsername == NULL){
		m_clientSoft = SO_UNKNOWN;
		return;
	}

	int iHashType = GetHashType();
	if (m_bEmuleProtocol || iHashType == SO_EMULE){
		LPCTSTR pszSoftware;
		switch(m_byCompatibleClient){
			case SO_CDONKEY:
				m_clientSoft = SO_CDONKEY;
				pszSoftware = _T("cDonkey");
				break;
			case SO_XMULE:
				m_clientSoft = SO_XMULE;
				pszSoftware = _T("xMule");
				break;
			case SO_AMULE:
				m_clientSoft = SO_AMULE;
				pszSoftware = _T("aMule");
				break;
			case SO_SHAREAZA:
			case 40:
				m_clientSoft = SO_SHAREAZA;
				pszSoftware = _T("Shareaza");
				break;
			case SO_LPHANT:
				m_clientSoft = SO_LPHANT;
				pszSoftware = _T("lphant");
				break;
			default:
				if (m_bIsML || m_byCompatibleClient == SO_MLDONKEY){
					m_clientSoft = SO_MLDONKEY;
					pszSoftware = _T("MLdonkey");
				}
				else if (m_bIsHybrid){
					m_clientSoft = SO_EDONKEYHYBRID;
					pszSoftware = _T("eDonkeyHybrid");
				}
				else if (m_byCompatibleClient != 0){
					m_clientSoft = SO_XMULE; // means: 'eMule Compatible'
					pszSoftware = _T("eMule Compat");
				}
				else{
					m_clientSoft = SO_EMULE;
					pszSoftware = _T("eMule");
				}
		}

		int iLen;
		TCHAR szSoftware[128];
		if (m_byEmuleVersion == 0){
			m_nClientVersion = MAKE_CLIENT_VERSION(0, 0, 0);
			iLen = _sntprintf(szSoftware, ARRSIZE(szSoftware), _T("%s"), pszSoftware);
		}
		else if (m_byEmuleVersion != 0x99){
			UINT nClientMinVersion = (m_byEmuleVersion >> 4)*10 + (m_byEmuleVersion & 0x0f);
			m_nClientVersion = MAKE_CLIENT_VERSION(0, nClientMinVersion, 0);
			iLen = _sntprintf(szSoftware, ARRSIZE(szSoftware), _T("%s v0.%u"), pszSoftware, nClientMinVersion);
		}
		else{
			UINT nClientMajVersion = (m_nClientVersion >> 17) & 0x7f;
			UINT nClientMinVersion = (m_nClientVersion >> 10) & 0x7f;
			UINT nClientUpVersion  = (m_nClientVersion >>  7) & 0x07;
			m_nClientVersion = MAKE_CLIENT_VERSION(nClientMajVersion, nClientMinVersion, nClientUpVersion);
			if (m_clientSoft == SO_EMULE)
				iLen = _sntprintf(szSoftware, ARRSIZE(szSoftware), _T("%s v%u.%u%c"), pszSoftware, nClientMajVersion, nClientMinVersion, _T('a') + nClientUpVersion);
			else if (m_clientSoft == SO_AMULE || nClientUpVersion != 0)
				iLen = _sntprintf(szSoftware, ARRSIZE(szSoftware), _T("%s v%u.%u.%u"), pszSoftware, nClientMajVersion, nClientMinVersion, nClientUpVersion);
			else if (m_clientSoft == SO_LPHANT)
			{
				if (nClientMinVersion < 10)
				    iLen = _sntprintf(szSoftware, ARRSIZE(szSoftware), _T("%s v%u.0%u"), pszSoftware, (nClientMajVersion-1), nClientMinVersion);
				else
					iLen = _sntprintf(szSoftware, ARRSIZE(szSoftware), _T("%s v%u.%u"), pszSoftware, (nClientMajVersion-1), nClientMinVersion);
			}
			else
				iLen = _sntprintf(szSoftware, ARRSIZE(szSoftware), _T("%s v%u.%u"), pszSoftware, nClientMajVersion, nClientMinVersion);
		}
		if (iLen > 0){
			memcpy(m_strClientSoftware.GetBuffer(iLen), szSoftware, iLen*sizeof(TCHAR));
			m_strClientSoftware.ReleaseBuffer(iLen);
		}
		return;
	}

	if (m_bIsHybrid){
		m_clientSoft = SO_EDONKEYHYBRID;
		// seen:
		// 105010	0.50.10
		// 10501	0.50.1
		// 10300	1.3.0
		// 10201	1.2.1
		// 10103	1.1.3
		// 10102	1.1.2
		// 10100	1.1
		// 1051		0.51.0
		// 1002		1.0.2
		// 1000		1.0
		// 501		0.50.1

		UINT nClientMajVersion;
		UINT nClientMinVersion;
		UINT nClientUpVersion;
		if (m_nClientVersion > 100000){
			UINT uMaj = m_nClientVersion/100000;
			nClientMajVersion = uMaj - 1;
			nClientMinVersion = (m_nClientVersion - uMaj*100000) / 100;
			nClientUpVersion = m_nClientVersion % 100;
		}
		else if (m_nClientVersion >= 10100 && m_nClientVersion <= 10309){
			UINT uMaj = m_nClientVersion/10000;
			nClientMajVersion = uMaj;
			nClientMinVersion = (m_nClientVersion - uMaj*10000) / 100;
			nClientUpVersion = m_nClientVersion % 10;
		}
		else if (m_nClientVersion > 10000){
			UINT uMaj = m_nClientVersion/10000;
			nClientMajVersion = uMaj - 1;
			nClientMinVersion = (m_nClientVersion - uMaj*10000) / 10;
			nClientUpVersion = m_nClientVersion % 10;
		}
		else if (m_nClientVersion >= 1000 && m_nClientVersion < 1020){
			UINT uMaj = m_nClientVersion/1000;
			nClientMajVersion = uMaj;
			nClientMinVersion = (m_nClientVersion - uMaj*1000) / 10;
			nClientUpVersion = m_nClientVersion % 10;
		}
		else if (m_nClientVersion > 1000){
			UINT uMaj = m_nClientVersion/1000;
			nClientMajVersion = uMaj - 1;
			nClientMinVersion = m_nClientVersion - uMaj*1000;
			nClientUpVersion = 0;
		}
		else if (m_nClientVersion > 100){
			UINT uMin = m_nClientVersion/10;
			nClientMajVersion = 0;
			nClientMinVersion = uMin;
			nClientUpVersion = m_nClientVersion - uMin*10;
		}
		else{
			nClientMajVersion = 0;
			nClientMinVersion = m_nClientVersion;
			nClientUpVersion = 0;
		}
		m_nClientVersion = MAKE_CLIENT_VERSION(nClientMajVersion, nClientMinVersion, nClientUpVersion);

		int iLen;
		TCHAR szSoftware[128];
		if (nClientUpVersion)
			iLen = _sntprintf(szSoftware, ARRSIZE(szSoftware), _T("eDonkeyHybrid v%u.%u.%u"), nClientMajVersion, nClientMinVersion, nClientUpVersion);
		else
			iLen = _sntprintf(szSoftware, ARRSIZE(szSoftware), _T("eDonkeyHybrid v%u.%u"), nClientMajVersion, nClientMinVersion);
		if (iLen > 0){
			memcpy(m_strClientSoftware.GetBuffer(iLen), szSoftware, iLen*sizeof(TCHAR));
			m_strClientSoftware.ReleaseBuffer(iLen);
		}
		return;
	}

	if (m_bIsML || iHashType == SO_MLDONKEY){
		m_clientSoft = SO_MLDONKEY;
		UINT nClientMinVersion = m_nClientVersion;
		m_nClientVersion = MAKE_CLIENT_VERSION(0, nClientMinVersion, 0);
		TCHAR szSoftware[128];
		int iLen = _sntprintf(szSoftware, ARRSIZE(szSoftware), _T("MLdonkey v0.%u"), nClientMinVersion);
		if (iLen > 0){
			memcpy(m_strClientSoftware.GetBuffer(iLen), szSoftware, iLen*sizeof(TCHAR));
			m_strClientSoftware.ReleaseBuffer(iLen);
		}
		return;
	}

	if (iHashType == SO_OLDEMULE){
		m_clientSoft = SO_OLDEMULE;
		UINT nClientMinVersion = m_nClientVersion;
		m_nClientVersion = MAKE_CLIENT_VERSION(0, nClientMinVersion, 0);
		TCHAR szSoftware[128];
		int iLen = _sntprintf(szSoftware, ARRSIZE(szSoftware), _T("Old eMule v0.%u"), nClientMinVersion);
		if (iLen > 0){
			memcpy(m_strClientSoftware.GetBuffer(iLen), szSoftware, iLen*sizeof(TCHAR));
			m_strClientSoftware.ReleaseBuffer(iLen);
		}
		return;
	}

	m_clientSoft = SO_EDONKEY;
	UINT nClientMinVersion = m_nClientVersion;
	m_nClientVersion = MAKE_CLIENT_VERSION(0, nClientMinVersion, 0);
	TCHAR szSoftware[128];
	int iLen = _sntprintf(szSoftware, ARRSIZE(szSoftware), _T("eDonkey v0.%u"), nClientMinVersion);
	if (iLen > 0){
		memcpy(m_strClientSoftware.GetBuffer(iLen), szSoftware, iLen*sizeof(TCHAR));
		m_strClientSoftware.ReleaseBuffer(iLen);
	}
}

int CUpDownClient::GetHashType() const
{
	if (m_achUserHash[5] == 13 && m_achUserHash[14] == 110)
		return SO_OLDEMULE;
	else if (m_achUserHash[5] == 14 && m_achUserHash[14] == 111)
		return SO_EMULE;
 	else if (m_achUserHash[5] == 'M' && m_achUserHash[14] == 'L')
		return SO_MLDONKEY;
	else
		return SO_UNKNOWN;
}

void CUpDownClient::SetUserName(LPCTSTR pszNewName)
{
	free(m_pszUsername);
	if (pszNewName)
		m_pszUsername = _tcsdup(pszNewName);
	else
		m_pszUsername = NULL;
}

void CUpDownClient::RequestSharedFileList()
{
	if (m_iFileListRequested == 0){
		AddLogLine(true, GetResString(IDS_SHAREDFILES_REQUEST), GetUserName());
    	m_iFileListRequested = 1;
		TryToConnect(true);
	}
	else{
		LogWarning(LOG_STATUSBAR, _T("Requesting shared files from user %s (%u) is already in progress"), GetUserName(), GetUserIDHybrid());
	}
}

void CUpDownClient::ProcessSharedFileList(const uchar* pachPacket, uint32 nSize, LPCTSTR pszDirectory)
{
	if (m_iFileListRequested > 0)
	{
        m_iFileListRequested--;
		CGlobalVariable::searchlist->ProcessSearchAnswer(pachPacket,nSize,this,NULL,pszDirectory);
	}
}

void CUpDownClient::SetUserHash(const uchar* pucUserHash)
{
	if( pucUserHash == NULL ){
		md4clr(m_achUserHash);
		return;
	}
	md4cpy(m_achUserHash, pucUserHash);
}

void CUpDownClient::SetBuddyID(const uchar* pucBuddyID)
{
	if( pucBuddyID == NULL ){
		md4clr(m_achBuddyID);
		m_bBuddyIDValid = false;
		return;
	}
	m_bBuddyIDValid = true;
	md4cpy(m_achBuddyID, pucBuddyID);
}

// returns 'false', if client instance was deleted!
bool CUpDownClient::SendHelloPacket(){
	if (socket == NULL){
		ASSERT(0);
		return true;
	}

	CSafeMemFile data(128);
	data.WriteUInt8(16); // size of userhash
	SendHelloTypePacket(&data);
	Packet* packet = new Packet(&data);
	packet->opcode = OP_HELLO;
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		DebugSend("OP__Hello", this);
	theStats.AddUpDataOverheadOther(packet->size);
	socket->SendPacket(packet,true);

	AddPeerLog(new CTraceSendMessage(GetResString(IDS_SEND_REQUEST)));
	m_bHelloAnswerPending = true;
	return true;
}


void CUpDownClient::ResetFileStatusInfo()
{
	delete[] m_abyPartStatus;
	m_abyPartStatus = NULL;
	m_nRemoteQueueRank = 0;
	m_nPartCount = 0;
	m_strClientFilename.Empty();
	m_bCompleteSource = false;
	m_uFileRating = 0;
	m_strFileComment.Empty();
	delete m_pReqFileAICHHash;
	m_pReqFileAICHHash = NULL;
}

bool CUpDownClient::IsBanned() const
{
#if defined(_DEBUG) || defined(_VCALPHA)
	if( _tcsstr(m_pszUsername,thePrefs.m_sDebugUserName)==NULL )
		return true;
#endif

	return CGlobalVariable::clientlist->IsBannedClient(GetIP());
}

// sends a packet, if needed it will establish a connection before
// options used: ignore max connections, control packet, delete packet
// !if the functions returns false that client object was deleted because the connection try failed and the object wasn't needed anymore.
bool CUpDownClient::SafeSendPacket(Packet* packet){
	if (socket && socket->IsConnected()){
		socket->SendPacket(packet);
		return true;
	}
	else{
		m_WaitingPackets_list.AddTail(packet);
		return TryToConnect(true);
	}
}

#ifdef _DEBUG
void CUpDownClient::AssertValid() const
{
	CObject::AssertValid();

	CHECK_OBJ(socket);
	CHECK_PTR(credits);
	CHECK_PTR(m_Friend);
	CHECK_OBJ(reqfile);
	(void)m_abyUpPartStatus;
	m_OtherRequests_list.AssertValid();
	m_OtherNoNeeded_list.AssertValid();
	(void)m_lastPartAsked;
	(void)m_cMessagesReceived;
	(void)m_cMessagesSent;
	(void)m_dwUserIP;
	(void)m_dwServerIP;
	(void)m_nUserIDHybrid;
	(void)m_nUserPort;
	(void)m_nServerPort;
	(void)m_nClientVersion;
	(void)m_nUpDatarate;
	(void)m_byEmuleVersion;
	(void)m_byDataCompVer;
	CHECK_BOOL(m_bEmuleProtocol);
	CHECK_BOOL(m_bIsHybrid);
	(void)m_pszUsername;
	(void)m_achUserHash;
	(void)m_achBuddyID;
	(void)m_nBuddyIP;
	(void)m_nBuddyPort;
	(void)m_nUDPPort;
	(void)m_nKadPort;
	(void)m_byUDPVer;
	(void)m_bySourceExchange1Ver;
	(void)m_byAcceptCommentVer;
	(void)m_byExtendedRequestsVer;
	CHECK_BOOL(m_bFriendSlot);
	CHECK_BOOL(m_bCommentDirty);
	CHECK_BOOL(m_bIsML);
	//ASSERT( m_clientSoft >= SO_EMULE && m_clientSoft <= SO_SHAREAZA || m_clientSoft == SO_MLDONKEY || m_clientSoft >= SO_EDONKEYHYBRID && m_clientSoft <= SO_UNKNOWN );
	(void)m_strClientSoftware;
	(void)m_dwLastSourceRequest;
	(void)m_dwLastSourceAnswer;
	(void)m_dwLastAskedForSources;
    (void)m_iFileListRequested;
	(void)m_byCompatibleClient;
	m_WaitingPackets_list.AssertValid();
	m_DontSwap_list.AssertValid();
	(void)m_lastRefreshedDLDisplay;
	ASSERT( m_SecureIdentState >= IS_UNAVAILABLE && m_SecureIdentState <= IS_KEYANDSIGNEEDED );
	(void)m_dwLastSignatureIP;
	ASSERT( (m_byInfopacketsReceived & ~IP_BOTH) == 0 );
	(void)m_bySupportSecIdent;
	(void)m_nTransferredUp;
	ASSERT( m_nUploadState >= US_UPLOADING && m_nUploadState <= US_NONE );
	(void)m_dwUploadTime;
	(void)m_cAsked;
	(void)m_dwLastUpRequest;
	(void)m_nCurSessionUp;
    (void)m_nCurQueueSessionPayloadUp;
    (void)m_addedPayloadQueueSession;
	(void)m_nUpPartCount;
	(void)m_nUpCompleteSourcesCount;
	(void)s_UpStatusBar;
	(void)requpfileid;
    (void)m_lastRefreshedULDisplay;
	m_AvarageUDR_list.AssertValid();
	m_BlockRequests_queue.AssertValid();
	m_DoneBlocks_list.AssertValid();
	m_RequestedFiles_list.AssertValid();
	ASSERT( m_nDownloadState >= DS_DOWNLOADING && m_nDownloadState <= DS_NONE );
	(void)m_cDownAsked;
	(void)m_abyPartStatus;
	(void)m_strClientFilename;
	(void)m_nTransferredDown;
    (void)m_nCurSessionPayloadDown;
	(void)m_dwDownStartTime;
	(void)m_nLastBlockOffset;
	(void)m_nDownDatarate;
	(void)m_nDownDataRateMS;
	(void)m_nSumForAvgDownDataRate;
	(void)m_cShowDR;
	(void)m_nRemoteQueueRank;
	(void)m_dwLastBlockReceived;
	(void)m_nPartCount;
	ASSERT( m_nSourceFrom >= SF_SERVER && m_nSourceFrom <= SF_LINK );
	CHECK_BOOL(m_bRemoteQueueFull);
	CHECK_BOOL(m_bCompleteSource);
	CHECK_BOOL(m_bReaskPending);
	CHECK_BOOL(m_bUDPPending);
	CHECK_BOOL(m_bTransferredDownMini);
	CHECK_BOOL(m_bUnicodeSupport);
	ASSERT( m_nKadState >= KS_NONE && m_nKadState <= KS_CONNECTED_BUDDY );
	m_AvarageDDR_list.AssertValid();
	(void)m_nSumForAvgUpDataRate;
	m_PendingBlocks_list.AssertValid();
	m_DownloadBlocks_list.AssertValid();
	(void)s_StatusBar;
	ASSERT( m_nChatstate >= MS_NONE && m_nChatstate <= MS_UNABLETOCONNECT );
	(void)m_strFileComment;
	(void)m_uFileRating;
	CHECK_BOOL(m_bCollectionUploadSlot);
#undef CHECK_PTR
#undef CHECK_BOOL
}
#endif

#ifdef _DEBUG
void CUpDownClient::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
}
#endif

LPCTSTR CUpDownClient::DbgGetDownloadState() const
{
	const static LPCTSTR apszState[] =
	{
		_T("Downloading"),
		_T("OnQueue"),
		_T("Connected"),
		_T("Connecting"),
		_T("WaitCallback"),
		_T("WaitCallbackKad"),
		_T("ReqHashSet"),
		_T("NoNeededParts"),
		_T("TooManyConns"),
		_T("TooManyConnsKad"),
		_T("LowToLowIp"),
		_T("Banned"),
		_T("Error"),
		_T("None"),
		_T("RemoteQueueFull")
	};
	if (GetDownloadState() >= ARRSIZE(apszState))
		return _T("*Unknown*");
	return apszState[GetDownloadState()];
}

LPCTSTR CUpDownClient::DbgGetUploadState() const
{
	const static LPCTSTR apszState[] =
	{
		_T("Uploading"),
		_T("OnUploadQueue"),
		_T("WaitCallback"),
		_T("Connecting"),
		_T("Pending"),
		_T("LowToLowIp"),
		_T("Banned"),
		_T("Error"),
		_T("None")
	};
	if (GetUploadState() >= ARRSIZE(apszState))
		return _T("*Unknown*");
	return apszState[GetUploadState()];
}

LPCTSTR CUpDownClient::DbgGetKadState() const
{
	const static LPCTSTR apszState[] =
	{
		_T("None"),
		_T("FwCheckQueued"),
		_T("FwCheckConnecting"),
		_T("FwCheckConnected"),
		_T("BuddyQueued"),
		_T("BuddyIncoming"),
		_T("BuddyConnecting"),
		_T("BuddyConnected")
	};
	if (GetKadState() >= ARRSIZE(apszState))
		return _T("*Unknown*");
	return apszState[GetKadState()];
}

CString CUpDownClient::DbgGetFullClientSoftVer() const
{
	if (GetClientModVer().IsEmpty())
		return GetClientSoftVer();
	return GetClientSoftVer() + _T(" [") + GetClientModVer() + _T(']');
}

CString CUpDownClient::DbgGetClientInfo(bool bFormatIP) const
{
	CString str;
	if (this != NULL)
	{
		try{
			if (HasLowID())
			{
				if (GetConnectIP())
				{
					str.Format(_T("%u@%s (%s) '%s' (%s,%s/%s/%s)"),
						GetUserIDHybrid(), ipstr(GetServerIP()),
						ipstr(GetConnectIP()),
						GetUserName(),
						DbgGetFullClientSoftVer(),
						DbgGetDownloadState(), DbgGetUploadState(), DbgGetKadState());
				}
				else
				{
					str.Format(_T("%u@%s '%s' (%s,%s/%s/%s)"),
						GetUserIDHybrid(), ipstr(GetServerIP()),
						GetUserName(),
						DbgGetFullClientSoftVer(),
						DbgGetDownloadState(), DbgGetUploadState(), DbgGetKadState());
				}
			}
			else
			{
				str.Format(bFormatIP ? _T("%-15s '%s' (%s,%s/%s/%s)") : _T("%s '%s' (%s,%s/%s/%s)"),
					ipstr(GetConnectIP()),
					GetUserName(),
					DbgGetFullClientSoftVer(),
					DbgGetDownloadState(), DbgGetUploadState(), DbgGetKadState());
			}
		}
		catch(...){
			str.Format(_T("%08x - Invalid client instance"), this);
		}
	}
	return str;
}

bool CUpDownClient::CheckHandshakeFinished() const
{
	if (m_bHelloAnswerPending)
	{
		// 24-Nov-2004 [bc]: The reason for this is that 2 clients are connecting to each other at the same..
		//if (thePrefs.GetVerbose())
		//	AddDebugLogLine(DLP_VERYLOW, false, _T("Handshake not finished - while processing packet: %s; %s"), DbgGetClientTCPOpcode(protocol, opcode), DbgGetClientInfo());
		return false;
	}

	return true;
}

void CUpDownClient::CheckForGPLEvilDoer()
{
	if (!m_strModVersion.IsEmpty()){
		LPCTSTR pszModVersion = (LPCTSTR)m_strModVersion;

		// skip leading spaces
		while (*pszModVersion == _T(' '))
			pszModVersion++;

		// check for known major gpl breaker
		if (_tcsnicmp(pszModVersion, _T("LH"), 2)==0 || _tcsnicmp(pszModVersion, _T("LIO"), 3)==0 || _tcsnicmp(pszModVersion, _T("PLUS PLUS"), 9)==0)
			m_bGPLEvildoer = true;
	}
}

void CUpDownClient::OnSocketConnected(int /*nErrorCode*/)
{
}

CString CUpDownClient::GetDownloadStateDisplayString() const
{
	CString strState;
	switch (GetDownloadState())
	{
		case DS_CONNECTING:
			strState = GetResString(IDS_CONNECTING);
			break;
		case DS_CONNECTED:
			strState = GetResString(IDS_ASKING);
			break;
		case DS_WAITCALLBACK:
			strState = GetResString(IDS_CONNVIASERVER);
			break;
		case DS_ONQUEUE:
			if (IsRemoteQueueFull())
				strState = GetResString(IDS_QUEUEFULL);
			else
				strState = GetResString(IDS_ONQUEUE);
			break;
		case DS_DOWNLOADING:
            if(GetSourceFrom() == SF_LAN) strState = GetResString(IDS_LANTRANSFERRING);// VC-kernel[2007-01-19]:
			else if(socket && socket->m_bUseNat) strState = GetResString(IDS_LOWTOLOWTRANSFERRING);
			else strState = GetResString(IDS_TRANSFERRING);
			break;
		case DS_REQHASHSET:
			strState = GetResString(IDS_RECHASHSET);
			break;
		case DS_NONEEDEDPARTS:
			strState = GetResString(IDS_NONEEDEDPARTS);
			break;
		case DS_LOWTOLOWIP:
			strState = GetResString(IDS_NOCONNECTLOW2LOW);
			if( SNT_UNSUPPORT==m_nSupportNatTraverse )
			{
				strState=GetResString(IDS_NOT_SUPPORTED);	
			}			
			break;
		case DS_TOOMANYCONNS:
			strState = GetResString(IDS_TOOMANYCONNS);
			break;
		case DS_ERROR:
			strState = GetResString(IDS_ERROR);
			break;
		case DS_WAITCALLBACKKAD:
			strState = GetResString(IDS_KAD_WAITCBK);
			break;
		case DS_TOOMANYCONNSKAD:
			strState = GetResString(IDS_KAD_TOOMANDYKADLKPS);
			break;
#ifdef _DEBUG
		case DS_NONE:
			strState = _T("DS_NONE");
			break;
#endif
	}

	if (thePrefs.GetPeerCacheShow())
	{
		switch (m_ePeerCacheDownState)
		{
		case PCDS_WAIT_CLIENT_REPLY:
			strState += _T(" ")+GetResString(IDS_PCDS_CLIENTWAIT);
			break;
		case PCDS_WAIT_CACHE_REPLY:
			strState += _T(" ")+GetResString(IDS_PCDS_CACHEWAIT);
			break;
		case PCDS_DOWNLOADING:
			strState += _T(" ")+GetResString(IDS_CACHE);
			break;
		}
		if (m_ePeerCacheDownState != PCDS_NONE && m_bPeerCacheDownHit)
			strState += _T(" Hit");
	}

	return strState;
}

CString CUpDownClient::GetUploadStateDisplayString() const
{
	CString strState;
	switch (GetUploadState()){
		case US_ONUPLOADQUEUE:
			strState = GetResString(IDS_ONQUEUE);
			break;
/*		case US_PENDING:
			strState = GetResString(IDS_CL_PENDING);
			break;*/
		case US_LOWTOLOWIP:
			strState = GetResString(IDS_CL_LOW2LOW);
			break;
		case US_BANNED:
			strState = GetResString(IDS_BANNED);
			break;
/*		case US_ERROR:
			strState = GetResString(IDS_ERROR);
			break;*/
		case US_CONNECTING:
			strState = GetResString(IDS_CONNECTING);
			break;
/*		case US_WAITCALLBACK:
			strState = GetResString(IDS_CONNVIASERVER);
			break;*/
		case US_UPLOADING:
            if(GetPayloadInBuffer() == 0 && GetNumberOfRequestedBlocksInQueue() == 0 && thePrefs.IsExtControlsEnabled()) {
				strState = GetResString(IDS_US_STALLEDW4BR);
            } else if(GetPayloadInBuffer() == 0 && thePrefs.IsExtControlsEnabled()) {
				strState = GetResString(IDS_US_STALLEDREADINGFDISK);
			}
			else if(GetSlotNumber() <= CGlobalVariable::uploadqueue->GetActiveUploadsCount())
			{
                if(socket && socket->m_bUseNat) 
					strState = GetResString(IDS_LOWTOLOWTRANSFERRING);
				else
					strState = GetResString(IDS_TRANSFERRING);
            } else 
			{
                strState = GetResString(IDS_TRICKLING);
            }
			break;
	}

	if (thePrefs.GetPeerCacheShow())
	{
		switch (m_ePeerCacheUpState)
		{
		case PCUS_WAIT_CACHE_REPLY:
			strState += _T(" CacheWait");
			break;
		case PCUS_UPLOADING:
			strState += _T(" Cache");
			break;
		}
		if (m_ePeerCacheUpState != PCUS_NONE && m_bPeerCacheUpHit)
			strState += _T(" Hit");
	}
	
	if( m_nSourceFrom==SF_LAN )
		strState += _T("(LAN)");

	return strState;
}

void CUpDownClient::SendPublicIPRequest(){
	if (socket && socket->IsConnected()){
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend("OP__PublicIPReq", this);
		Packet* packet = new Packet(OP_PUBLICIP_REQ,0,OP_EMULEPROT);
		theStats.AddUpDataOverheadOther(packet->size);
		socket->SendPacket(packet,true);
		m_fNeedOurPublicIP = 1;
	}
}

void CUpDownClient::CheckFailedFileIdReqs(const uchar* aucFileHash)
{
	if ( aucFileHash != NULL && (CGlobalVariable::sharedfiles->IsUnsharedFile(aucFileHash) || CGlobalVariable::downloadqueue->GetFileByID(aucFileHash)) )
		return;
	//if (GetDownloadState() != DS_DOWNLOADING) // filereq floods are never allowed!
	{
		if (m_fFailedFileIdReqs < 6)// NOTE: Do not increase this nr. without increasing the bits for 'm_fFailedFileIdReqs'
			m_fFailedFileIdReqs++;
		if (m_fFailedFileIdReqs == 6)
		{
			if (CGlobalVariable::clientlist->GetBadRequests(this) < 2)
				CGlobalVariable::clientlist->TrackBadRequest(this, 1);
			if (CGlobalVariable::clientlist->GetBadRequests(this) == 2){
				CGlobalVariable::clientlist->TrackBadRequest(this, -2); // reset so the client will not be rebanned right after the ban is lifted
				Ban(_T("FileReq flood"));
			}
			throw CString(thePrefs.GetLogBannedClients() ? _T("FileReq flood") : _T(""));
		}
	}
}

EUtf8Str CUpDownClient::GetUnicodeSupport() const
{
	if (m_bUnicodeSupport)
		return utf8strRaw;
	return utf8strNone;
}

void CUpDownClient::SetSpammer(bool bVal){ 
	if (bVal)
		Ban(_T("Identified as Spammer"));
	else if (IsBanned() && m_fIsSpammer)
		UnBan();
	m_fIsSpammer = bVal ? 1 : 0;
}

void  CUpDownClient::SetMessageFiltered(bool bVal)	{
	m_fMessageFiltered = bVal ? 1 : 0;
}

bool  CUpDownClient::IsObfuscatedConnectionEstablished() const {
	if (socket != NULL && socket->IsConnected())
		return socket->IsObfusicating();
	else
		return false;
}

bool CUpDownClient::ShouldReceiveCryptUDPPackets() const {
	return (thePrefs.IsClientCryptLayerSupported() && SupportsCryptLayer() && CGlobalVariable::GetPublicIP() != 0
		&& HasValidHash() && (thePrefs.IsClientCryptLayerRequested() || RequestsCryptLayer()) );
}

void CUpDownClient::ProcessRawData(const BYTE * /*pucData*/, UINT /*uSize*/)
{
	ASSERT(false);
	//  for HTTP and FTP etc.
	//  so assert fail here
}

void CUpDownClient::Pause()
{
	Packet packet(OP_CANCELTRANSFER,0);
	if (GetDownloadState() == DS_DOWNLOADING)
	{
		SendCancelTransfer(&packet);
		if(IsEd2kClient())
			SetDownloadState(DS_ONQUEUE, _T("You cancelled the download. Sending OP_CANCELTRANSFER"));
		else
			SetDownloadState(DS_NONE, _T("You cancelled the download. Sending OP_CANCELTRANSFER"));
	}
}

void CUpDownClient::SetCompletePartStatus( bool bComplete )
{
	// VC-nightsuns[2007-11-16] start: 解决堆被程序损坏的BUG
	int oldCount = m_nPartCount;
	m_nPartCount = reqfile->GetPartCount();

	if( oldCount==m_nPartCount && m_bCompleteSource==bComplete )
	{
		return ;
	}
	else if( oldCount != m_nPartCount && m_abyPartStatus ) {
		// 如果前后不一致，那么删除掉原来的
		delete []m_abyPartStatus;
		m_abyPartStatus = NULL;
	}
	// VC-nightsuns[2007-11-16] end

	if( NULL==m_abyPartStatus )
		m_abyPartStatus = new uint8[m_nPartCount];
	if( bComplete )
		memset(m_abyPartStatus, 1, m_nPartCount);
	else 
		memset(m_abyPartStatus, 0, m_nPartCount);
	m_bCompleteSource = bComplete;
	if(NULL!=reqfile)
		reqfile->UpdatePartsInfo();
}

// VC-SearchDream[2007-04-18]: For SourceExchange NAT 
void CUpDownClient::CheckUDPTunnel()
{
	CSourceExchangeNAT::CheckUDPTunnel(this);
}

void CUpDownClient::AddPeerLog(CTraceEvent* Event)
{
	m_EventList.AddHead(Event);
	UINotify(WM_FILE_UPDATE_PEERLOG, (WPARAM)this, (LPARAM)Event, Event);
}

EventList* CUpDownClient::GetEventList(void)
{
	return &m_EventList;
}

void CUpDownClient::SeverSrcExchangeWithHelper()
{
	POSITION pos = NULL;
	if (NULL != m_pSourceExchangeClient)
	{
		pos = m_pSourceExchangeClient->m_listSrcExchangeFor.Find(this);
		if (NULL != pos)
			m_pSourceExchangeClient->m_listSrcExchangeFor.RemoveAt(pos);

		m_pSourceExchangeClient = NULL;
	}
}
void CUpDownClient::SeverSrcExchangeWithAsker()
{
	POSITION pos = NULL;
	CUpDownClient *pAsker = NULL;
	pos = m_listSrcExchangeFor.GetHeadPosition();
	while (NULL != pos)
	{
		pAsker = m_listSrcExchangeFor.GetNext(pos);
		if (NULL != pAsker)
			pAsker->m_pSourceExchangeClient = NULL;
	}
	m_listSrcExchangeFor.RemoveAll();
}

void CUpDownClient::SendHelloTypePacket(CSafeMemFile* data)
{
	//  Comment UI
	data->WriteHash16(thePrefs.GetUserHash());
	uint32 clientid;
	clientid = CGlobalVariable::GetID();

	data->WriteUInt32(clientid);
	data->WriteUInt16(thePrefs.GetPort());
#ifdef _ENABLE_NATTRAVERSE
	uint32 tagcount = 7;
#else
	uint32 tagcount = 6;
#endif

	if( CGlobalVariable::clientlist->GetBuddy() && CGlobalVariable::IsFirewalled() )
		tagcount += 2;

	bool bSendModVersion = true;//(m_strModVersion.GetLength() || m_pszUsername==NULL) ;
	if(bSendModVersion) 
		tagcount+=1;

	if( m_nSourceFrom==SF_LAN )
		tagcount+=1;
	
	data->WriteUInt32(tagcount);

	// eD2K Name

	// TODO implement multi language website which informs users of the effects of bad mods
	//CTag tagName(CT_NAME, (!m_bGPLEvildoer) ? thePrefs.GetUserNick() : _T("Please use a GPL-conform version of eMule") );

	//VeryCD added by kernel1983 2006.08.01
	CTag tagName(CT_NAME, thePrefs.GetUserNickVC());	//将带[CHN][VeryCD]的nick发送出去，有助于提高下载速度
	tagName.WriteTagToFile(data, utf8strRaw);

	// eD2K Version
	CTag tagVersion(CT_VERSION,EDONKEYVERSION);
	tagVersion.WriteTagToFile(data);

	// eMule UDP Ports
	uint32 kadUDPPort = 0;
	if(Kademlia::CKademlia::IsConnected())
	{
		kadUDPPort = thePrefs.GetUDPPort();
	}
	CTag tagUdpPorts(CT_EMULE_UDPPORTS, 
				((uint32)kadUDPPort			   << 16) |
				((uint32)thePrefs.GetUDPPort() <<  0)
				); 
	tagUdpPorts.WriteTagToFile(data);
	
	if( CGlobalVariable::clientlist->GetBuddy() && CGlobalVariable::IsFirewalled() )
	{
		CTag tagBuddyIP(CT_EMULE_BUDDYIP, CGlobalVariable::clientlist->GetBuddy()->GetIP() ); 
		tagBuddyIP.WriteTagToFile(data);
	
		CTag tagBuddyPort(CT_EMULE_BUDDYUDP, 
//					( RESERVED												)
					((uint32)CGlobalVariable::clientlist->GetBuddy()->GetUDPPort()  ) 
					);
		tagBuddyPort.WriteTagToFile(data);
	}

	// eMule Misc. Options #1
	const UINT uUdpVer				= 4;
	const UINT uDataCompVer			= 1;
	const UINT uSupportSecIdent		= CGlobalVariable::clientcredits->CryptoAvailable() ? 3 : 0;
	const UINT uSourceExchangeVer	= 4;
	const UINT uExtendedRequestsVer	= 2;
	const UINT uAcceptCommentVer	= 1;
	const UINT uNoViewSharedFiles	= (thePrefs.CanSeeShares() == vsfaNobody) ? 1 : 0; // for backward compatibility this has to be a 'negative' flag
	const UINT uMultiPacket			= 1;
	const UINT uSupportPreview		= (thePrefs.CanSeeShares() != vsfaNobody) ? 1 : 0; // set 'Preview supported' only if 'View Shared Files' allowed
	const UINT uPeerCache			= 1;
	const UINT uUnicodeSupport		= 1;
	const UINT nAICHVer				= 1;
	CTag tagMisOptions1(CT_EMULE_MISCOPTIONS1, 
				(nAICHVer				<< 29) |
				(uUnicodeSupport		<< 28) |
				(uUdpVer				<< 24) |
				(uDataCompVer			<< 20) |
				(uSupportSecIdent		<< 16) |
				(uSourceExchangeVer		<< 12) |
				(uExtendedRequestsVer	<<  8) |
				(uAcceptCommentVer		<<  4) |
				(uPeerCache				<<  3) |
				(uNoViewSharedFiles		<<  2) |
				(uMultiPacket			<<  1) |
				(uSupportPreview		<<  0)
				);
	tagMisOptions1.WriteTagToFile(data);

	// eMule Misc. Options #2
	const UINT uKadVersion			= KADEMLIA_VERSION;
	const UINT uSupportLargeFiles	= 1;
	const UINT uExtMultiPacket		= 1;
	const UINT uReserved			= 0; // mod bit
	const UINT uSupportsCryptLayer	= thePrefs.IsClientCryptLayerSupported() ? 1 : 0;
	const UINT uRequestsCryptLayer	= thePrefs.IsClientCryptLayerRequested() ? 1 : 0;
	const UINT uRequiresCryptLayer	= thePrefs.IsClientCryptLayerRequired() ? 1 : 0;

	CTag tagMisOptions2(CT_EMULE_MISCOPTIONS2, 
//				(RESERVED				     )
				(uRequiresCryptLayer	<<  9) |
				(uRequestsCryptLayer	<<  8) |
				(uSupportsCryptLayer	<<  7) |
				(uReserved				<<  6) |
				(uExtMultiPacket		<<  5) |
				(uSupportLargeFiles		<<  4) |
				(uKadVersion			<<  0) 
				);
	tagMisOptions2.WriteTagToFile(data);

	// eMule Version
	CTag tagMuleVersion(CT_EMULE_VERSION, 
				//(uCompatibleClientID		<< 24) |
				(CGlobalVariable::m_nVersionMjr	<< 17) |
				(CGlobalVariable::m_nVersionMin	<< 10) |
				(CGlobalVariable::m_nVersionUpd	<<  7) 
//				(RESERVED			     ) 
				);
	tagMuleVersion.WriteTagToFile(data);

	// VC-Huby[2007-01-30]: send mod_version tag
	if(bSendModVersion) 
	{
		CString sMyModVersion=_T("");
		sMyModVersion.Format(_T("VeryCD easyMule 0%u"),VC_VERSION_BUILD);
		CTag tagMODVersion(ET_MOD_VERSION, sMyModVersion);
		tagMODVersion.WriteTagToFile(data);
	}

#ifdef _ENABLE_NATTRAVERSE
	// VC-yunchenn.chen[2006-12-30]: add support nat traversal info
	CTag tagSupportVCNT(CT_SUPPORT_VCNT, 0x1489e90c, false);
	tagSupportVCNT.WriteTagToFile(data);
#endif

	if( m_nSourceFrom==SF_LAN )
	{
		CTag tagIsLanPeer( CT_LAN_PEER, CGlobalVariable::serverconnect->GetLocalIP() );
		tagIsLanPeer.WriteTagToFile(data);
	}	

	uint32 dwIP;
	uint16 nPort;
	if (CGlobalVariable::serverconnect->IsConnected()){
		dwIP = CGlobalVariable::serverconnect->GetCurrentServer()->GetIP();
		nPort = CGlobalVariable::serverconnect->GetCurrentServer()->GetPort();
#ifdef _DEBUG
		if (dwIP == CGlobalVariable::serverconnect->GetLocalIP()){
			dwIP = 0;
			nPort = 0;
		}
#endif
	}
	else{
		nPort = 0;
		dwIP = 0;
	}
	data->WriteUInt32(dwIP);
	data->WriteUInt16(nPort);
//	data->WriteUInt32(dwIP); //The Hybrid added some bits here, what ARE THEY FOR?
}

/**
 * Gets the current waiting score for this client, taking into consideration waiting
 * time, priority of requested file, and the client's credits.
 */
uint32 CUpDownClient::GetScore(bool sysvalue, bool isdownloading, bool onlybasevalue) const
{
    if (!m_pszUsername)
        return 0;

    if (credits == 0)
    {
        ASSERT ( IsKindOf(RUNTIME_CLASS(CHttpClient)) );
        return 0;
    }
    CKnownFile* currequpfile = NULL;
    //  Comment UI
    currequpfile = CGlobalVariable::sharedfiles->GetFileByID(requpfileid);
    if (!currequpfile)
        return 0;

    // bad clients (see note in function)
    if (credits->GetCurrentIdentState(GetIP()) == IS_IDBADGUY)
        return 0;
    // friend slot
    if (IsFriend() && GetFriendSlot() && !HasLowID())
        return 0x0FFFFFFF;

    if (IsBanned() || m_bGPLEvildoer)
        return 0;

    if (sysvalue && HasLowID() && !(socket && socket->IsConnected()))
    {
        return 0;
    }

    int filepriority = GetFilePrioAsNumber();

    // calculate score, based on waitingtime and other factors
    float fBaseValue;
    if (onlybasevalue)
        fBaseValue = 100;
    else if (!isdownloading)
        fBaseValue = (float)(::GetTickCount()-GetWaitStartTime())/1000;
    else
    {
        // we dont want one client to download forever
        // the first 15 min downloadtime counts as 15 min waitingtime and you get a 15 min bonus while you are in the first 15 min :)
        // (to avoid 20 sec downloads) after this the score won't raise anymore
        fBaseValue = (float)(m_dwUploadTime-GetWaitStartTime());
        ASSERT ( m_dwUploadTime-GetWaitStartTime() >= 0 ); //oct 28, 02: changed this from "> 0" to ">= 0"
        fBaseValue += (float)(::GetTickCount() - m_dwUploadTime > 900000)? 900000:1800000;
        fBaseValue /= 1000;
    }

    if (thePrefs.UseCreditSystem())
    {
        float modif = credits->GetScoreRatio(GetIP());
        fBaseValue *= modif;
    }
    if (!onlybasevalue)
        fBaseValue *= (float(filepriority)/10.0f);

    if ( (IsEmuleClient() || this->GetClientSoft() < 10) && m_byEmuleVersion <= 0x19 )
        fBaseValue *= 0.5f;

	//Xman Anti-Leecher
	if(IsLeecher()>0)
		fBaseValue *=0.33f;
	//Xman end

    return (uint32)fBaseValue;
}

bool CUpDownClient::SameErrorManage(ErrorReason error)
{
	bool bClose = false;
	if (error == m_eLastError)
	{
		m_dwLastErrorCount++;
		if (m_dwLastErrorCount >= 3)
		{
			bClose = true;
			m_dwLastErrorCount = 0;
		}
	}
	else
		m_dwLastErrorCount = 0;

	return bClose;
}

void CUpDownClient::SendFirewallCheckUDPRequest()
{
	ASSERT( GetKadState() == KS_FWCHECK_UDP );
	if (!Kademlia::CKademlia::IsRunning()){
		SetKadState(KS_NONE);
		return;
	}
	else if (GetUploadState() != US_NONE || GetDownloadState() != DS_NONE || GetChatState() != MS_NONE
		|| GetKadVersion() <= KADEMLIA_VERSION5_48a || GetKadPort() == 0)
	{
		Kademlia::CUDPFirewallTester::SetUDPFWCheckResult(false, true, ntohl(GetIP()), 0); // inform the tester that this test was cancelled
		SetKadState(KS_NONE);
		return;
	}
	ASSERT( Kademlia::CKademlia::GetPrefs()->GetExternalKadPort() != 0 );
	CSafeMemFile data;
	data.WriteUInt16(Kademlia::CKademlia::GetPrefs()->GetInternKadPort());
	data.WriteUInt16(Kademlia::CKademlia::GetPrefs()->GetExternalKadPort());
	data.WriteUInt32(Kademlia::CKademlia::GetPrefs()->GetUDPVerifyKey(GetConnectIP()));
	Packet* packet = new Packet(&data, OP_EMULEPROT, OP_FWCHECKUDPREQ);
	theStats.AddUpDataOverheadKad(packet->size);
	SafeSendPacket(packet);
}

void CUpDownClient::SetConnectOptions(uint8 byOptions, bool bEncryption, bool bCallback)
{
	SetCryptLayerSupport((byOptions & 0x01) != 0 && bEncryption);
	SetCryptLayerRequest((byOptions & 0x02) != 0 && bEncryption);
	SetCryptLayerRequires((byOptions & 0x04) != 0 && bEncryption);
	SetDirectUDPCallbackSupport((byOptions & 0x08) != 0 && bCallback);
}

bool CUpDownClient::PrepareActiveConnectType()
{
	if ( !HasLowID() || 
		( m_nSourceFrom==SF_LAN || m_nSourceFrom==SF_HTTP || m_nSourceFrom==SF_FTP) )
	{
		m_nActiveConnectType = ACT_DIRECTTCP;
		return true;
	}
	else if( HasLowID() )
	{		
		if(!CGlobalVariable::CanDoCallback(this)) /// VeryCD mod support Low2Low Connect
		{
			// VC-yunchenn.chen[2006-12-30]: to check whether supports nat traversal
			if( m_nUserPort!=0xffff 
				&& (m_nSourceFrom==SF_SOURCE_EXCHANGE || m_nSourceFrom==SF_KADEMLIA )
				&& m_nSupportNatTraverse==SNT_UNKNOWN )
			{
				if(Disconnected(_T("LowID->LowID(not support VeryCD Low2Low)")))
				{
					delete this;
				}				
				return false;
/*
				m_nSupportNatTraverse = SNT_UNSUPPORT;
				if (GetDownloadState() == DS_CONNECTING)
					SetDownloadState(DS_LOWTOLOWIP);
				else if (GetDownloadState() == DS_REQHASHSET)
				{
					SetDownloadState(DS_ONQUEUE);
					reqfile->hashsetneeded = true;
				}
				if (GetUploadState() == US_CONNECTING)
				{
					if(Disconnected(_T("LowID->LowID and US_CONNECTING")))
					{
						delete this;
						return false;
					}
				}
				return true;
*/
			}

			if(m_nUserPort==0xffff) 
				m_nSupportNatTraverse = SNT_SUPPORT;

			m_nActiveConnectType = ACT_FAKETCP;
			return true;
		}
		else
		{
			m_nActiveConnectType = ACT_CALLBACK;
			return true;
		}
	}

	return false;
}