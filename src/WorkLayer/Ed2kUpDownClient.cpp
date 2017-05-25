#include "StdAfx.h"
#include ".\ed2kupdownclient.h"

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

#include "Kademlia/Kademlia/Kademlia.h"
#include "Kademlia/Kademlia/Search.h"
#include "Kademlia/Kademlia/SearchManager.h"
#include "Kademlia/Kademlia/UDPFirewallTester.h"
#include "Kademlia/routing/RoutingZone.h"
#include "Kademlia/Utils/UInt128.h"
#include "Kademlia/Net/KademliaUDPListener.h"
#include "Kademlia/Kademlia/Prefs.h"

#include "vcconfig.h"

//Xman Anti-Leecher: simple Anti-Thief
const CString CEd2kUpDownClient::str_ANTAddOn=CUpDownClient::GetANTAddOn();

IMPLEMENT_DYNAMIC( CEd2kUpDownClient , CUpDownClient )

CEd2kUpDownClient::CEd2kUpDownClient(CClientReqSocket* sender ) : CUpDownClient(sender)
{
	init();
}

CEd2kUpDownClient::CEd2kUpDownClient(CPartFile* in_reqfile, uint16 in_port, uint32 in_userid, uint32 in_serverip, uint16 in_serverport, bool ed2kID )
 : CUpDownClient( in_reqfile , in_port , in_userid , in_serverip, in_serverport, ed2kID )
{
	init();
}

CEd2kUpDownClient::~CEd2kUpDownClient()
{
}

void CEd2kUpDownClient::init()
{
	m_pszFunnyNick = 0;
	SetLastBuddyPingPongTime();
}

void CEd2kUpDownClient::ClearHelloProperties()
{
	m_nUDPPort = 0;
	m_byUDPVer = 0;
	m_byDataCompVer = 0;
	m_byEmuleVersion = 0;
	m_bySourceExchange1Ver = 0;
	m_byAcceptCommentVer = 0;
	m_byExtendedRequestsVer = 0;
	m_byCompatibleClient = 0;
	m_nKadPort = 0;
	m_bySupportSecIdent = 0;
	m_fSupportsPreview = 0;
	m_nClientVersion = 0;
	m_fSharedDirectories = 0;
	m_bMultiPacket = 0;
	m_fPeerCache = 0;
	m_uPeerCacheDownloadPushId = 0;
	m_uPeerCacheUploadPushId = 0;
	m_byKadVersion = 0;
	m_fSupportsLargeFiles = 0;
	m_fExtMultiPacket = 0;
	m_fRequestsCryptLayer = 0;
	m_fSupportsCryptLayer = 0;
	m_fRequiresCryptLayer = 0;
}

bool CEd2kUpDownClient::ProcessHelloPacket(const uchar* pachPacket, uint32 nSize)
{
	CSafeMemFile data(pachPacket, nSize);
	//Xman Anti-Leecher
	//data.ReadUInt8(); // read size of userhash
	uhashsize=data.ReadUInt8();
	//Xman end
	// reset all client properties; a client may not send a particular emule tag any longer
	ClearHelloProperties();
	AddPeerLog(new CTraceServerMessage(GetResString(IDS_RECEIVE_HELLO)));
	return ProcessHelloTypePacket(&data,true); //Xman Anti-Leecher
}

bool CEd2kUpDownClient::ProcessHelloAnswer(const uchar* pachPacket, uint32 nSize)
{
	//Xman Anti-Leecher
	uhashsize=16;
	//Xman end
	AddPeerLog(new CTraceServerMessage(GetResString(IDS_RECEIVE_ANSWER)));
	CSafeMemFile data(pachPacket, nSize);
	bool bIsMule = ProcessHelloTypePacket(&data, false); //Xman Anti-Leecher
	m_bHelloAnswerPending = false;
	return bIsMule;
}

bool CEd2kUpDownClient::ProcessHelloTypePacket(CSafeMemFile* data, bool isHelloPacket) //Xman Anti-Leecher
{
	bool bDbgInfo = thePrefs.GetUseDebugDevice();
	m_strHelloInfo.Empty();
	// clear hello properties which can be changed _only_ on receiving OP_Hello/OP_HelloAnswer
	m_bIsHybrid = false;
	m_bIsML = false;	
	m_fNoViewSharedFiles = 0;
	m_bUnicodeSupport = false;

	data->ReadHash16(m_achUserHash);

	if (bDbgInfo)
		m_strHelloInfo.AppendFormat(_T("Hash=%s (%s)"), md4str(m_achUserHash), DbgGetHashTypeString(m_achUserHash));
	m_nUserIDHybrid = data->ReadUInt32();
	if (bDbgInfo)
		m_strHelloInfo.AppendFormat(_T("  UserID=%u (%s)"), m_nUserIDHybrid, ipstr(m_nUserIDHybrid));
	uint16 nUserPort = data->ReadUInt16(); // hmm clientport is sent twice - why?
	if (bDbgInfo)
		m_strHelloInfo.AppendFormat(_T("  Port=%u"), nUserPort);

	CString strBanReason; //Xman Anti-Leecher
	bool nonofficialopcodes = false; //Xman Anti-Leecher
	CString unknownopcode; //Xman Anti-Leecher
	bool wronghello = false; //Xman Anti-Leecher
	bool foundmd4string = false; //Xman Anti-Leecher
	
	DWORD dwEmuleTags = 0;
	bool bPrTag = false;
	uint32 tagcount = data->ReadUInt32();
	if (bDbgInfo)
		m_strHelloInfo.AppendFormat(_T("  Tags=%u"), tagcount);
	for (uint32 i = 0; i < tagcount; i++)
	{
		CTag temptag(data, true);
		switch (temptag.GetNameID())
		{
			// VC-yunchenn.chen[2006-12-30]
			case CT_SUPPORT_VCNT:
				if(temptag.IsInt())
				{
					if(temptag.GetInt()==0x1489e90c)
					{
						m_nSupportNatTraverse = SNT_SUPPORT;
					}
				}
				break;			
			case CT_NAME:
				if (temptag.IsStr()) {
					free(m_pszUsername);
					m_pszUsername = _tcsdup(temptag.GetStr());
					if (bDbgInfo) {
						if (m_pszUsername) {//filter username for bad chars
							TCHAR* psz = m_pszUsername;
							while (*psz != _T('\0')) {
								if (*psz == _T('\n') || *psz == _T('\r'))
									*psz = _T(' ');
								psz++;
							}
						}
						m_strHelloInfo.AppendFormat(_T("\n  Name='%s'"), m_pszUsername);
					}
				}
				else if (bDbgInfo)
					m_strHelloInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
				break;

			case CT_VERSION:
				if (temptag.IsInt()) {
					if (bDbgInfo)
						m_strHelloInfo.AppendFormat(_T("\n  Version=%u"), temptag.GetInt());
					m_nClientVersion = temptag.GetInt();
				}
				else if (bDbgInfo)
					m_strHelloInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
				break;

			case CT_PORT:
				if (temptag.IsInt()) {
					if (bDbgInfo)
						m_strHelloInfo.AppendFormat(_T("\n  Port=%u"), temptag.GetInt());
					nUserPort = (uint16)temptag.GetInt();
				}
				else if (bDbgInfo)
					m_strHelloInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
				break;

			case CT_MOD_VERSION:
				if (temptag.IsStr())
					m_strModVersion = temptag.GetStr();
				else if (temptag.IsInt())
					m_strModVersion.Format(_T("ModID=%u"), temptag.GetInt());
				else
					m_strModVersion = _T("ModID=<Unknown>");
				if (bDbgInfo)
					m_strHelloInfo.AppendFormat(_T("\n  ModID=%s"), m_strModVersion);
	
				AddPeerLog(new CTraceServerMessage(m_strModVersion));
				
				CheckForGPLEvilDoer();
				break;

			case CT_EMULE_UDPPORTS:
				// 16 KAD Port
				// 16 UDP Port
				if (temptag.IsInt()) {
					m_nKadPort = (uint16)(temptag.GetInt() >> 16);
					m_nUDPPort = (uint16)temptag.GetInt();

					// VC-SearchDream[2007-04-13]: For Low2Low2Low Begin
					if (socket && socket->m_bUseNat && CGlobalVariable::natthread)
					{
						uint16 nUDPPort = CGlobalVariable::natthread->GetUDPPort(socket);

						if (nUDPPort != 0)
						{
							m_nKadPort = ntohs(nUDPPort);
							m_nUDPPort = ntohs(nUDPPort);
						}

						m_bLowToLowClient = true; 
					}
					// VC-SearchDream[2007-04-13]: For Low2Low2Low End

					if (bDbgInfo)
						m_strHelloInfo.AppendFormat(_T("\n  KadPort=%u  UDPPort=%u"), m_nKadPort, m_nUDPPort);
					dwEmuleTags |= 1;
				}
				else if (bDbgInfo)
					m_strHelloInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
				break;

			case CT_EMULE_BUDDYUDP:
				// 16 --Reserved for future use--
				// 16 BUDDY Port
				if (temptag.IsInt()) {
					m_nBuddyPort = (uint16)temptag.GetInt();
					if (bDbgInfo)
						m_strHelloInfo.AppendFormat(_T("\n  BuddyPort=%u"), m_nBuddyPort);
				}
				else if (bDbgInfo)
					m_strHelloInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
				break;

			case CT_EMULE_BUDDYIP:
				// 32 BUDDY IP
				if (temptag.IsInt()) {
					m_nBuddyIP = temptag.GetInt();
					if (bDbgInfo)
						m_strHelloInfo.AppendFormat(_T("\n  BuddyIP=%s"), ipstr(m_nBuddyIP));
				}
				else if (bDbgInfo)
					m_strHelloInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
				break;

			case CT_EMULE_MISCOPTIONS1:
				//  3 AICH Version (0 = not supported)
				//  1 Unicode
				//  4 UDP version
				//  4 Data compression version
				//  4 Secure Ident
				//  4 Source Exchange
				//  4 Ext. Requests
				//  4 Comments
				//	1 PeerChache supported
				//	1 No 'View Shared Files' supported
				//	1 MultiPacket
				//  1 Preview
				if (temptag.IsInt()) {
					m_fSupportsAICH			= (temptag.GetInt() >> 29) & 0x07;
					m_bUnicodeSupport		= (temptag.GetInt() >> 28) & 0x01;
					m_byUDPVer				= (uint8)((temptag.GetInt() >> 24) & 0x0f);
					m_byDataCompVer			= (uint8)((temptag.GetInt() >> 20) & 0x0f);
					m_bySupportSecIdent		= (uint8)((temptag.GetInt() >> 16) & 0x0f);
					m_bySourceExchange1Ver	= (uint8)((temptag.GetInt() >> 12) & 0x0f);
					m_byExtendedRequestsVer	= (uint8)((temptag.GetInt() >>  8) & 0x0f);
					m_byAcceptCommentVer	= (uint8)((temptag.GetInt() >>  4) & 0x0f);
					m_fPeerCache			= (temptag.GetInt() >>  3) & 0x01;
					m_fNoViewSharedFiles	= (temptag.GetInt() >>  2) & 0x01;
					m_bMultiPacket			= (temptag.GetInt() >>  1) & 0x01;
					m_fSupportsPreview		= (temptag.GetInt() >>  0) & 0x01;
					dwEmuleTags |= 2;
					if (bDbgInfo) {
						m_strHelloInfo.AppendFormat(_T("\n  PeerCache=%u  UDPVer=%u  DataComp=%u  SecIdent=%u  SrcExchg=%u")
													_T("  ExtReq=%u  Commnt=%u  Preview=%u  NoViewFiles=%u  Unicode=%u"), 
													m_fPeerCache, m_byUDPVer, m_byDataCompVer, m_bySupportSecIdent, m_bySourceExchange1Ver, 
													m_byExtendedRequestsVer, m_byAcceptCommentVer, m_fSupportsPreview, m_fNoViewSharedFiles, m_bUnicodeSupport);
					}
				}
				else if (bDbgInfo)
					m_strHelloInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
				break;

			case CT_EMULE_MISCOPTIONS2:
				//	22 Reserved
				//	 1 Requires CryptLayer
				//	 1 Requests CryptLayer
				//	 1 Supports CryptLayer
				//	 1 Reserved (ModBit)
				//   1 Ext Multipacket (Hash+Size instead of Hash)
				//   1 Large Files (includes support for 64bit tags)
				//   4 Kad Version
				if (temptag.IsInt()) {
					m_fRequiresCryptLayer	= (temptag.GetInt() >>  9) & 0x01;
					m_fRequestsCryptLayer	= (temptag.GetInt() >>  8) & 0x01;
					m_fSupportsCryptLayer	= (temptag.GetInt() >>  7) & 0x01;
					// reserved 1
					m_fExtMultiPacket		= (temptag.GetInt() >>  5) & 0x01;
					m_fSupportsLargeFiles   = (temptag.GetInt() >>  4) & 0x01;
					m_byKadVersion			= (uint8)((temptag.GetInt() >>  0) & 0x0f);
					dwEmuleTags |= 8;
					if (bDbgInfo)
						m_strHelloInfo.AppendFormat(_T("\n  KadVersion=%u, LargeFiles=%u ExtMultiPacket=%u CryptLayerSupport=%u CryptLayerRequest=%u CryptLayerRequires=%u"), m_byKadVersion, m_fSupportsLargeFiles, m_fExtMultiPacket, m_fSupportsCryptLayer, m_fRequestsCryptLayer, m_fRequiresCryptLayer);
					m_fRequestsCryptLayer &= m_fSupportsCryptLayer;
					m_fRequiresCryptLayer &= m_fRequestsCryptLayer;

				}
				else if (bDbgInfo)
					m_strHelloInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
				break;

			case CT_EMULE_VERSION:
				//  8 Compatible Client ID
				//  7 Mjr Version (Doesn't really matter..)
				//  7 Min Version (Only need 0-99)
				//  3 Upd Version (Only need 0-5)
				//  7 Bld Version (Only need 0-99) -- currently not used
				if (temptag.IsInt()) {
					m_byCompatibleClient = (uint8)((temptag.GetInt() >> 24));
					m_nClientVersion = temptag.GetInt() & 0x00ffffff;
					m_byEmuleVersion = 0x99;
					m_fSharedDirectories = 1;
					dwEmuleTags |= 4;
					if (bDbgInfo)
						m_strHelloInfo.AppendFormat(_T("\n  ClientVer=%u.%u.%u.%u  Comptbl=%u"), (m_nClientVersion >> 17) & 0x7f, (m_nClientVersion >> 10) & 0x7f, (m_nClientVersion >> 7) & 0x07, m_nClientVersion & 0x7f, m_byCompatibleClient);
				}
				else if (bDbgInfo)
					m_strHelloInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
				break;
			case CT_LAN_PEER:
				m_nLanConnectIP = temptag.GetInt();
				m_nSourceFrom = SF_LAN;
				break;

			default:
				// {begin} VC-dgkang 2008年9月18日
				//Xman Anti-Leecher
				if(thePrefs.GetAntiLeecherSnafu())
					if(ProcessUnknownHelloTag(&temptag, strBanReason))
						foundmd4string=true;

				unknownopcode.AppendFormat(_T(",%s"),temptag.GetFullInfo());

				//-----------------
				if (thePrefs.GetVerbose())
				{
					TCHAR userhash[128];
					md4str(m_achUserHash,userhash);

					AddDebugLogLine(false, _T("********UserName: %s ModString: %s UnTag: %s IsShareFile: %d IsSupportVCNat: %d"),m_pszUsername,
						m_strModVersion,temptag.GetFullInfo(),m_fNoViewSharedFiles,m_nSupportNatTraverse);
				}
				//-----------------
				//{end}

				//Xman end
			

				// Since eDonkeyHybrid 1.3 is no longer sending the additional Int32 at the end of the Hello packet,
				// we use the "pr=1" tag to determine them.
				if (temptag.GetName() && temptag.GetName()[0]=='p' && temptag.GetName()[1]=='r') {
					bPrTag = true;
				}
				if (bDbgInfo)
					m_strHelloInfo.AppendFormat(_T("\n  ***UnkTag=%s"), temptag.GetFullInfo());
		}
	}
	
	if( m_nSupportNatTraverse != SNT_SUPPORT )
		m_nSupportNatTraverse = SNT_UNSUPPORT;

	m_nUserPort = nUserPort;
	m_dwServerIP = data->ReadUInt32();
	m_nServerPort = data->ReadUInt16();
	if (bDbgInfo)
		m_strHelloInfo.AppendFormat(_T("\n  Server=%s:%u"), ipstr(m_dwServerIP), m_nServerPort);

	// Check for additional data in Hello packet to determine client's software version.
	//
	// *) eDonkeyHybrid 0.40 - 1.2 sends an additional Int32. (Since 1.3 they don't send it any longer.)
	// *) MLdonkey sends an additional Int32
	//
	if (data->GetLength() - data->GetPosition() == sizeof(uint32)){
		uint32 test = data->ReadUInt32();
		if (test == 'KDLM'){
			m_bIsML = true;
			if (bDbgInfo)
				m_strHelloInfo += _T("\n  ***AddData: \"MLDK\"");
		}
		else{
			m_bIsHybrid = true;
			if (bDbgInfo)
				m_strHelloInfo.AppendFormat(_T("\n  ***AddData: uint32=%u (0x%08x)"), test, test);
		}
	}
	else if (bDbgInfo && data->GetPosition() < data->GetLength()){
		UINT uAddHelloDataSize = (UINT)(data->GetLength() - data->GetPosition());
		if (uAddHelloDataSize == sizeof(uint32)){
			DWORD dwAddHelloInt32 = data->ReadUInt32();
			m_strHelloInfo.AppendFormat(_T("\n  ***AddData: uint32=%u (0x%08x)"), dwAddHelloInt32, dwAddHelloInt32);
		}
		else if (uAddHelloDataSize == sizeof(uint32)+sizeof(uint16)){
			DWORD dwAddHelloInt32 = data->ReadUInt32();
			WORD w = data->ReadUInt16();
			m_strHelloInfo.AppendFormat(_T("\n  ***AddData: uint32=%u (0x%08x),  uint16=%u (0x%04x)"), dwAddHelloInt32, dwAddHelloInt32, w, w);
		}
		else
			m_strHelloInfo.AppendFormat(_T("\n  ***AddData: %u bytes"), uAddHelloDataSize);
	}

	SOCKADDR_IN sockAddr = {0};
	int nSockAddrLen = sizeof(sockAddr);
	socket->GetPeerName((SOCKADDR*)&sockAddr, &nSockAddrLen);
	SetIP(sockAddr.sin_addr.S_un.S_addr);

 	//EastShare Start - added by AndCycle, IP to Country
 	if(CGlobalVariable::ip2country->IsIP2Country()){
 		 // Superlexx
 		if (m_structUserCountry == CGlobalVariable::ip2country->GetDefaultIP2Country()){
 			m_structUserCountry = CGlobalVariable::ip2country->GetCountryFromIP(m_dwUserIP);
 		}
 	}
 	//EastShare End - added by AndCycle, IP to Country

	if (thePrefs.GetAddServersFromClients() && m_dwServerIP && m_nServerPort){
		CServer* addsrv = new CServer(m_nServerPort, ipstr(m_dwServerIP));
		addsrv->SetListName(addsrv->GetAddress());

		//  Comment UI begin
		if(! ::SendMessage(CGlobalVariable::m_hListenWnd, WM_SERVER_ADD_SVR, (WPARAM)addsrv, 0x0001))
		{
			delete addsrv;
		}
		//if (!theApp.emuledlg->serverwnd->serverlistctrl.AddServer(addsrv, true))
		//	delete addsrv;
		//  Comment UI end
	}

	//(a)If this is a highID user, store the ID in the Hybrid format.
	//(b)Some older clients will not send a ID, these client are HighID users that are not connected to a server.
	//(c)Kad users with a *.*.*.0 IPs will look like a lowID user they are actually a highID user.. They can be detected easily
	//because they will send a ID that is the same as their IP..
	if(!HasLowID() || m_nUserIDHybrid == 0 || m_nUserIDHybrid == m_dwUserIP ) 
		m_nUserIDHybrid = ntohl(m_dwUserIP);

	CClientCredits* pFoundCredits = NULL;
	//  Comment UI
	pFoundCredits=CGlobalVariable::clientcredits->GetCredit(m_achUserHash);
	if (credits == NULL){
		credits = pFoundCredits;
		if (!CGlobalVariable::clientlist->ComparePriorUserhash(m_dwUserIP, m_nUserPort, pFoundCredits)){
			if (thePrefs.GetLogBannedClients())
				AddDebugLogLine(false, _T("Clients: %s (%s), Banreason: Userhash changed (Found in TrackedClientsList)"), GetUserName(), ipstr(GetConnectIP()));
			Ban();
		}	
	}
	else if (credits != pFoundCredits){
		// userhash change ok, however two hours "waittime" before it can be used
		credits = pFoundCredits;
		if (thePrefs.GetLogBannedClients())
			AddDebugLogLine(false, _T("Clients: %s (%s), Banreason: Userhash changed"), GetUserName(), ipstr(GetConnectIP()));
		Ban();
	}

	//  Comment UI
	/*if ((m_Friend = theApp.friendlist->SearchFriend(m_achUserHash, m_dwUserIP, m_nUserPort)) != NULL){
		// Link the friend to that client
        m_Friend->SetLinkedClient(this);
	}
	else{
		// avoid that an unwanted client instance keeps a friend slot
		SetFriendSlot(false);
	}*/

	// check for known major gpl breaker
	
	// 修改和利用了原来的GPL Evildoer功能实现客户端和版本号的屏蔽
	/*
	if (strBuffer.Find(_T("EMULE-CLIENT")) != -1 || strBuffer.Find(_T("POWERMULE")) != -1 ){
		m_bGPLEvildoer = true;  
	}
	*/
	CString strBuffer = m_pszUsername;
	strBuffer.MakeUpper();
	strBuffer.Remove(_T(' '));
	//AddLogLine(false,_T("client nickname:%s"),strBuffer.GetBuffer());
	// 根据用户名屏蔽,使用VeryCD屏蔽列表 added by kernel1983 on 2006.08.02
	for(int i=0; i<CLIENT_FILTER_NAME_SIZE; i++)
	{
		if(filterNameList[i].method == CFM_INCLUDE //INCLUDE
			&& strBuffer.Find(filterNameList[i].keyStr) != -1)
		{
			m_bGPLEvildoer = true;
			//AddLogLine(false,_T("client name CFM_INCLUDE"));
			break;
		}
		else if(filterNameList[i].method == CFM_EQUAL //EQUAL
			&& strBuffer.Compare(filterNameList[i].keyStr) == 0)
		{
			m_bGPLEvildoer = true;
			//AddLogLine(false,_T("client name CFM_EQUAL"));
			break;
		}
	}

	m_byInfopacketsReceived |= IP_EDONKEYPROTPACK;
	// check if at least CT_EMULEVERSION was received, all other tags are optional
	bool bIsMule = (dwEmuleTags & 0x04) == 0x04;
	if (bIsMule){
		m_bEmuleProtocol = true;
		m_byInfopacketsReceived |= IP_EMULEPROTPACK;
	}
	else if (bPrTag){
		m_bIsHybrid = true;
	}

	InitClientSoftwareVersion(); // 初始化客户端版本号

	// 按版本号屏蔽,使用VeryCD屏蔽列表 added by kernel1983 on 2006.08.02
	strBuffer = m_strClientSoftware;
	strBuffer.MakeUpper();
	strBuffer.Remove(_T(' '));
	//AddLogLine(false,_T("client versionv is %s"),strBuffer);

	for(int i=0; i<CLIENT_FILTER_EDTION_SIZE; i++)
	{
		if(filterEditonList[i].method == CFM_INCLUDE
			&& strBuffer.Find(filterEditonList[i].keyStr) != -1)
		{
			m_bGPLEvildoer = true;
			//AddLogLine(false,_T("client version CFM_INCLUDE"));
			break;
		}
		else if(filterEditonList[i].method == CFM_EQUAL
			&& strBuffer.Compare(filterEditonList[i].keyStr) == 0)
		{
			m_bGPLEvildoer = true;
			//AddLogLine(false,_T("client version CFM_EQUAL"));
			break;
		}
	}


	//Xman Anti-Leecher
	if(thePrefs.GetAntiLeecher())
	{
		if (CGlobalVariable::GetID()!=m_nUserIDHybrid && memcmp(m_achUserHash, thePrefs.GetUserHash(), 16)==0)
		{
			strBanReason = GetResString(IDS_DLP_ANTICREDIT);
			BanLeecher(strBanReason,9);
			return bIsMule;
		}
		if(strBanReason.IsEmpty()==false && thePrefs.GetAntiLeecherSnafu())
		{
			BanLeecher(strBanReason,2); //snafu = old leecher = hard ban
			return bIsMule;
		}
		if(foundmd4string && thePrefs.GetAntiLeecherSnafu())
		{
			strBanReason = GetResString(IDS_DLP_MD4INOP);
			BanLeecher(strBanReason, 15);
			return bIsMule;
		}
		if(thePrefs.GetAntiLeecherBadHello() && (m_clientSoft==SO_EMULE || (m_clientSoft==SO_XMULE && m_byCompatibleClient!=SO_XMULE)))
		{
			if(wronghello)
			{
				strBanReason = GetResString(IDS_DLP_WRONGHELLOORDER);
				BanLeecher(strBanReason,1); //these are Leechers of a big german Leechercommunity
				return bIsMule;
			}
			if(data->GetPosition() < data->GetLength())
			{
				strBanReason = GetResString(IDS_DLP_EXTRABYTES);
				BanLeecher(strBanReason,13); // darkmule (or buggy)
				return bIsMule;
			}
			if(uhashsize!=16 && uhashsize!=205)
			{
				TRACE(_T("wrong uhashsize = %d\n"), uhashsize);
				strBanReason = GetResString(IDS_DLP_WRONGHASHSIZE);
				BanLeecher(strBanReason,14); //new united community
				return bIsMule;
			}
			if(m_fSupportsAICH > 1  && m_clientSoft == SO_EMULE && m_nClientVersion <= MAKE_CLIENT_VERSION(CGlobalVariable::m_nVersionMjr, CGlobalVariable::m_nVersionMin, CGlobalVariable::m_nVersionUpd))
			{
				strBanReason = GetResString(IDS_DLP_APPLEJUICE);
				BanLeecher(strBanReason,17); //Applejuice 
				return bIsMule;
			}
		}

		//if it is now a good mod, remove the reducing of score but do a second test
		if(IsLeecher()==1 || IsLeecher()==15) //category 2 is snafu and always a hard ban, need only to check 1
		{
			m_bLeecher=0; //it's a good mod now
			m_strBanMessage.Format(GetResString(IDS_DLP_UNBAN),DbgGetClientInfo());
			old_m_strClientSoftwareFULL.Empty();	//force recheck
			old_m_pszUsername.Empty();
		}

		if(IsLeecher()==14  && isHelloPacket ) //check if it is a Hello-Packet
		{
			m_bLeecher=0; //it's a good mod now
			m_strBanMessage.Format(GetResString(IDS_DLP_UNBAN),DbgGetClientInfo());
			old_m_strClientSoftwareFULL.Empty();	//force recheck
			old_m_pszUsername.Empty();
		}

		if(IsLeecher()==17)
		{
			m_bLeecher=0; //it's a good mod now
			m_strBanMessage.Format(GetResString(IDS_DLP_UNBAN),DbgGetClientInfo());
			old_m_strClientSoftwareFULL.Empty();	//force recheck
			old_m_pszUsername.Empty();
		}

		TestLeecher(); //test for modstring, nick and thiefs

		if(thePrefs.GetAntiLeecheremcrypt())
		{
			//Xman remark: I only check for 0.44d. 
			if(m_nClientVersion == MAKE_CLIENT_VERSION(0,44,3) && m_strModVersion.IsEmpty() && m_byCompatibleClient==0 && m_bUnicodeSupport==false && bIsMule)
			{
				if(IsLeecher()==0)
				{
					strBanReason = GetResString(IDS_DLP_EMCRYPT);
					BanLeecher(strBanReason,12); // emcrypt = no unicode for unicode version
				}
			}
			else if(IsLeecher()==12)
			{
				m_strBanMessage.Format(GetResString(IDS_DLP_UNBAN),DbgGetClientInfo());
				m_bLeecher=0; //unban, it isn't any longer a emcrypt
			}
		}
		else if(IsLeecher()==12) 
		{
			m_strBanMessage.Format(GetResString(IDS_DLP_UNBAN),DbgGetClientInfo());
			m_bLeecher=0; //unban, user doesn't want to ban it anymore
		}

		if(thePrefs.GetAntiGhost() )
		{
			if(m_strModVersion.IsEmpty() &&
				((nonofficialopcodes==true	&&	GetClientSoft()!=SO_LPHANT)
				|| ((unknownopcode.IsEmpty()==false || m_byAcceptCommentVer > 1) && m_clientSoft == SO_EMULE && m_nClientVersion <= MAKE_CLIENT_VERSION(CGlobalVariable::m_nVersionMjr, CGlobalVariable::m_nVersionMin, CGlobalVariable::m_nVersionUpd)))
				)
			{
				if(IsLeecher()==0)
				{
					strBanReason = GetResString(IDS_DLP_GHOSTMOD);
					if(unknownopcode.IsEmpty()==false)
						strBanReason += _T(" ") + unknownopcode;
					BanLeecher(strBanReason,3); // ghost mod = webcache tag without modstring
				}
			}
			else if(IsLeecher()==3) 
			{
				m_strBanMessage.Format(GetResString(IDS_DLP_UNBAN),DbgGetClientInfo());
				m_bLeecher=0; //unban, it isn't any longer a ghost mod
			}

		}
		else if(IsLeecher()==3)
		{
			m_strBanMessage.Format(GetResString(IDS_DLP_UNBAN),DbgGetClientInfo());
			m_bLeecher=0; //unban, user doesn't want to ban it anymore
		}
	}
	else if(IsLeecher()>0)
	{
		m_strBanMessage.Format(GetResString(IDS_DLP_UNBAN),DbgGetClientInfo());
		m_bLeecher=0; //unban, user doesn't want to ban it anymore
	}
	//Xman end

	if (m_bIsHybrid)
		m_fSharedDirectories = 1;

	if (thePrefs.GetVerbose() && GetServerIP() == INADDR_NONE)
		AddDebugLogLine(false, _T("Received invalid server IP %s from %s"), ipstr(GetServerIP()), DbgGetClientInfo());

	return bIsMule;
}

void CEd2kUpDownClient::SendMuleInfoPacket(bool bAnswer){
	if (socket == NULL){
		ASSERT(0);
		return;
	}

	CSafeMemFile data(128);
	//  Comment UI
	data.WriteUInt8((uint8)CGlobalVariable::m_uCurVersionShort);
	data.WriteUInt8(EMULE_PROTOCOL);
	
	uint32 tagcount = 7;
	bool bSendModVersion = (m_strModVersion.GetLength() || m_pszUsername==NULL) ;
	if(bSendModVersion) 
		tagcount+=1;
	data.WriteUInt32(tagcount); // nr. of tags

	CTag tag(ET_COMPRESSION,1);
	tag.WriteTagToFile(&data);
	CTag tag2(ET_UDPVER,4);
	tag2.WriteTagToFile(&data);
	CTag tag3(ET_UDPPORT,thePrefs.GetUDPPort());
	tag3.WriteTagToFile(&data);
	CTag tag4(ET_SOURCEEXCHANGE,3);
	tag4.WriteTagToFile(&data);
	CTag tag5(ET_COMMENTS,1);
	tag5.WriteTagToFile(&data);
	CTag tag6(ET_EXTENDEDREQUEST,2);
	tag6.WriteTagToFile(&data);

	uint32 dwTagValue = (CGlobalVariable::clientcredits->CryptoAvailable() ? 3 : 0);
	if (thePrefs.CanSeeShares() != vsfaNobody) // set 'Preview supported' only if 'View Shared Files' allowed
		dwTagValue |= 128;
	CTag tag7(ET_FEATURES, dwTagValue);
	tag7.WriteTagToFile(&data);

	// VC-Huby[2007-01-30]: send mod_version tag
	if(bSendModVersion) 
	{
		CString sMyModVersion;
		sMyModVersion.Format(_T("VeryCD 0%u"),VC_VERSION_BUILD);
		CTag tagMODVersion(ET_MOD_VERSION, sMyModVersion);
		tagMODVersion.WriteTagToFile(&data);
	}

	Packet* packet = new Packet(&data,OP_EMULEPROT);
	if (!bAnswer)
		packet->opcode = OP_EMULEINFO;
	else
		packet->opcode = OP_EMULEINFOANSWER;
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		DebugSend(!bAnswer ? "OP__EmuleInfo" : "OP__EmuleInfoAnswer", this);
	theStats.AddUpDataOverheadOther(packet->size);
	socket->SendPacket(packet,true,true);
}

void CEd2kUpDownClient::ProcessMuleInfoPacket(const uchar* pachPacket, uint32 nSize)
{
	CString strBanReason = NULL; //Xman Anti-Leecher
	bool nonofficialopcodes = false; //Xman Anti-Leecher

	bool bDbgInfo = thePrefs.GetUseDebugDevice();
	m_strMuleInfo.Empty();
	CSafeMemFile data(pachPacket, nSize);
	m_byCompatibleClient = 0;
	m_byEmuleVersion = data.ReadUInt8();
	if (bDbgInfo)
		m_strMuleInfo.AppendFormat(_T("EmuleVer=0x%x"), (UINT)m_byEmuleVersion);
	if (m_byEmuleVersion == 0x2B)
		m_byEmuleVersion = 0x22;
	uint8 protversion = data.ReadUInt8();
	if (bDbgInfo)
		m_strMuleInfo.AppendFormat(_T("  ProtVer=%u"), (UINT)protversion);

	//implicitly supported options by older clients
	if (protversion == EMULE_PROTOCOL) {
		//in the future do not use version to guess about new features

		if (m_byEmuleVersion < 0x25 && m_byEmuleVersion > 0x22)
			m_byUDPVer = 1;

		if (m_byEmuleVersion < 0x25 && m_byEmuleVersion > 0x21)
			m_bySourceExchange1Ver = 1;

		if (m_byEmuleVersion == 0x24)
			m_byAcceptCommentVer = 1;

		// Shared directories are requested from eMule 0.28+ because eMule 0.27 has a bug in 
		// the OP_ASKSHAREDFILESDIR handler, which does not return the shared files for a 
		// directory which has a trailing backslash.
		if (m_byEmuleVersion >= 0x28 && !m_bIsML) // MLdonkey currently does not support shared directories
			m_fSharedDirectories = 1;

	} else {
		return;
	}

	uint32 tagcount = data.ReadUInt32();
	if (bDbgInfo)
		m_strMuleInfo.AppendFormat(_T("  Tags=%u"), (UINT)tagcount);
	for (uint32 i = 0; i < tagcount; i++)
	{
		CTag temptag(&data, false);
		switch (temptag.GetNameID())
		{
			case ET_COMPRESSION:
				// Bits 31- 8: 0 - reserved
				// Bits  7- 0: data compression version
				if (temptag.IsInt()) {
					m_byDataCompVer = (uint8)temptag.GetInt();
					if (bDbgInfo)
						m_strMuleInfo.AppendFormat(_T("\n  Compr=%u"), (UINT)temptag.GetInt());
				}
				else if (bDbgInfo)
					m_strMuleInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
				break;
			
			case ET_UDPPORT:
				// Bits 31-16: 0 - reserved
				// Bits 15- 0: UDP port
				if (temptag.IsInt()) {
					m_nUDPPort = (uint16)temptag.GetInt();
					
					// VC-SearchDream[2007-04-13]: For Low2Low2Low Begin
					if (socket && socket->m_bUseNat && CGlobalVariable::natthread)
					{
						uint16 nUDPPort = CGlobalVariable::natthread->GetUDPPort(socket);

						if (nUDPPort != 0)
						{
							m_nUDPPort = ntohs(nUDPPort);
						}
					}
					// VC-SearchDream[2007-04-13]: For Low2Low2Low End

					if (bDbgInfo)
						m_strMuleInfo.AppendFormat(_T("\n  UDPPort=%u"), (UINT)temptag.GetInt());
				}
				else if (bDbgInfo)
					m_strMuleInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
				break;
			
			case ET_UDPVER:
				// Bits 31- 8: 0 - reserved
				// Bits  7- 0: UDP protocol version
				if (temptag.IsInt()) {
					m_byUDPVer = (uint8)temptag.GetInt();
					if (bDbgInfo)
						m_strMuleInfo.AppendFormat(_T("\n  UDPVer=%u"), (UINT)temptag.GetInt());
				}
				else if (bDbgInfo)
					m_strMuleInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
				break;
			
			case ET_SOURCEEXCHANGE:
				// Bits 31- 8: 0 - reserved
				// Bits  7- 0: source exchange protocol version
				if (temptag.IsInt()) {
					m_bySourceExchange1Ver = (uint8)temptag.GetInt();
					if (bDbgInfo)
						m_strMuleInfo.AppendFormat(_T("\n  SrcExch=%u"), (UINT)temptag.GetInt());
				}
				else if (bDbgInfo)
					m_strMuleInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
				break;
			
			case ET_COMMENTS:
				// Bits 31- 8: 0 - reserved
				// Bits  7- 0: comments version
				if (temptag.IsInt()) {
					m_byAcceptCommentVer = (uint8)temptag.GetInt();
					if (bDbgInfo)
						m_strMuleInfo.AppendFormat(_T("\n  Commnts=%u"), (UINT)temptag.GetInt());
				}
				else if (bDbgInfo)
					m_strMuleInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
				break;
			
			case ET_EXTENDEDREQUEST:
				// Bits 31- 8: 0 - reserved
				// Bits  7- 0: extended requests version
				if (temptag.IsInt()) {
					m_byExtendedRequestsVer = (uint8)temptag.GetInt();
					if (bDbgInfo)
						m_strMuleInfo.AppendFormat(_T("\n  ExtReq=%u"), (UINT)temptag.GetInt());
				}
				else if (bDbgInfo)
					m_strMuleInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
				break;
			
			case ET_COMPATIBLECLIENT:
				// Bits 31- 8: 0 - reserved
				// Bits  7- 0: compatible client ID
				if (temptag.IsInt()) {
					m_byCompatibleClient = (uint8)temptag.GetInt();
					if (bDbgInfo)
						m_strMuleInfo.AppendFormat(_T("\n  Comptbl=%u"), (UINT)temptag.GetInt());
				}
				else if (bDbgInfo)
					m_strMuleInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
				break;
			
			case ET_FEATURES:
				// Bits 31- 8: 0 - reserved
				// Bit	    7: Preview
				// Bit   6- 0: secure identification
				if (temptag.IsInt()) {
					m_bySupportSecIdent = (uint8)((temptag.GetInt()) & 3);
					m_fSupportsPreview  = (temptag.GetInt() >> 7) & 1;
					if (bDbgInfo)
						m_strMuleInfo.AppendFormat(_T("\n  SecIdent=%u  Preview=%u"), m_bySupportSecIdent, m_fSupportsPreview);
				}
				else if (bDbgInfo)
					m_strMuleInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
				break;
			
			case ET_MOD_VERSION:
				if (temptag.IsStr())
					m_strModVersion = temptag.GetStr();
				else if (temptag.IsInt())
					m_strModVersion.Format(_T("ModID=%u"), temptag.GetInt());
				else
					m_strModVersion = _T("ModID=<Unknwon>");
				if (bDbgInfo)
					m_strMuleInfo.AppendFormat(_T("\n  ModID=%s"), m_strModVersion);
				CheckForGPLEvilDoer();
				break;

			//Xman Anti-Leecher
			case 0x3D: //ICS
				nonofficialopcodes=true; //Xman Anti-Leecher
				break;
			//Xman end
			default:
				//Xman Anti-Leecher
				if(thePrefs.GetAntiLeecher())
					ProcessUnknownInfoTag(&temptag, strBanReason);
				//Xman end
				if (bDbgInfo)
					m_strMuleInfo.AppendFormat(_T("\n  ***UnkTag=%s"), temptag.GetFullInfo());

				if (thePrefs.GetVerbose())
				{
					TCHAR userhash[128];
					md4str(m_achUserHash,userhash);

					AddDebugLogLine(false, _T("-----------UserName: %s ModString: %s UnTag: %s IsShareFile: %d IsSupportVCNat: %d"),m_pszUsername,
						m_strModVersion,temptag.GetFullInfo(),m_fNoViewSharedFiles,m_nSupportNatTraverse);
				}
		}
	}
	if (m_byDataCompVer == 0) {
		m_bySourceExchange1Ver = 0;
		m_byExtendedRequestsVer = 0;
		m_byAcceptCommentVer = 0;
		m_nUDPPort = 0;
	}
	if (bDbgInfo && data.GetPosition() < data.GetLength()) {
		m_strMuleInfo.AppendFormat(_T("\n  ***AddData: %u bytes"), data.GetLength() - data.GetPosition());
	}

	m_bEmuleProtocol = true;
	m_byInfopacketsReceived |= IP_EMULEPROTPACK;
	InitClientSoftwareVersion();

	if (thePrefs.GetVerbose() && GetServerIP() == INADDR_NONE)
		AddDebugLogLine(false, _T("Received invalid server IP %s from %s"), ipstr(GetServerIP()), DbgGetClientInfo());

	//Xman Anti-Leecher
	if(thePrefs.GetAntiLeecher())
	{
		if(strBanReason.IsEmpty()==false && thePrefs.GetAntiLeecherSnafu())
		{
			BanLeecher(strBanReason,2); //snafu = old leecher = hard ban
			return;
		}

		TestLeecher(); //test for modstring (older clients send it with the MuleInfoPacket

		if(thePrefs.GetAntiGhost() )
		{
			if(nonofficialopcodes==true && m_strModVersion.IsEmpty() &&  GetClientSoft()!=SO_LPHANT)
			{
				if(IsLeecher()==0)
				{
					strBanReason = GetResString(IDS_DLP_GHOSTMOD);
					BanLeecher(strBanReason,3); // ghost mod = webcache tag without modstring
				}
			}
			else if(IsLeecher()==3)
			{
				m_strBanMessage.Format(GetResString(IDS_DLP_UNBAN),DbgGetClientInfo());
				m_bLeecher=0; //unban, it isn't any longer a ghost mod
			}

		}
		else if(IsLeecher()==3)
		{
			m_strBanMessage.Format(GetResString(IDS_DLP_UNBAN),DbgGetClientInfo());
			m_bLeecher=0; //unban, user doesn't want to ban it anymore
		}
	}
	else if(IsLeecher()>0)
	{
		m_strBanMessage.Format(GetResString(IDS_DLP_UNBAN),DbgGetClientInfo());
		m_bLeecher=0; //unban, user doesn't want to ban it anymore
	}
}

bool CEd2kUpDownClient::ProcessUnknownHelloTag(CTag *tag, CString &pszReason)
{
#ifndef LOGTAG
	//Xman DLP
	if(pszReason.IsEmpty()==false)
		return false;
	//Xman end
#endif

	//Xman DLP
	if(CGlobalVariable::dlp->IsDLPavailable()==false)
		return false;

	bool foundmd4=false;

	LPCTSTR strSnafuTag=CGlobalVariable::dlp->DLPCheckHelloTag(tag->GetNameID());
	if (strSnafuTag!=NULL)
	{
		pszReason.Format(_T("Suspect Hello-Tag: %s"),strSnafuTag);
	}
	//Xman end

	if (strSnafuTag==NULL && tag->IsStr() && tag->GetStr().GetLength() >= 32)
		foundmd4=true;

#ifdef LOGTAG
	if(m_byCompatibleClient==0 && GetHashType() == SO_EMULE )
	{
		AddDebugLogLine(false,_T("Unknown HelloTag: 0x%x, %s, client:%s"), tag->GetNameID(), tag->GetFullInfo(), DbgGetClientInfo());
	}
#endif

	return foundmd4;
}
void CEd2kUpDownClient::ProcessUnknownInfoTag(CTag *tag, CString &pszReason)
{
#ifndef LOGTAG
	//Xman DLP
	if(pszReason.IsEmpty()==false)
		return;
	//Xman end
#endif

	//Xman DLP
	if(CGlobalVariable::dlp->IsDLPavailable()==false)
		return;
	LPCTSTR strSnafuTag=CGlobalVariable::dlp->DLPCheckInfoTag(tag->GetNameID());
	if (strSnafuTag!=NULL)
	{
		pszReason.Format(_T("Suspect eMuleInfo-Tag: %s"),strSnafuTag);
	}
	//Xman end
#ifdef LOGTAG
		if(m_byCompatibleClient==0 && GetHashType() == SO_EMULE )
		{
			AddDebugLogLine(false,_T("Unknown InfoTag: 0x%x, %s, client:%s"), tag->GetNameID(), tag->GetFullInfo(), DbgGetClientInfo());
		}
#endif

}
//Xman end

void CEd2kUpDownClient::SendHelloAnswer(){
	if (socket == NULL){
		ASSERT(0);
		return;
	}

	CSafeMemFile data(133);
	SendHelloTypePacket(&data);
	Packet* packet = new Packet(&data);
	packet->opcode = OP_HELLOANSWER;
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		DebugSend("OP__HelloAnswer", this);
	theStats.AddUpDataOverheadOther(packet->size);

	// Servers send a FIN right in the data packet on check connection, so we need to force the response immediate
	bool bForceSend = CGlobalVariable::serverconnect->AwaitingTestFromIP(GetConnectIP());
	socket->SendPacket(packet, true, true, 0, bForceSend);

	m_bHelloAnswerPending = false;
}

// xman dlp
void CEd2kUpDownClient::TestLeecher()
{
	//Xman DLP
	if(CGlobalVariable::dlp->IsDLPavailable()==false)
		return;
	//Xman end

	if (thePrefs.GetAntiLeecherMod())
	{
		if(old_m_strClientSoftwareFULL.IsEmpty() || old_m_strClientSoftwareFULL!= DbgGetFullClientSoftVer() )
		{
		
			old_m_strClientSoftwareFULL = DbgGetFullClientSoftVer();
			LPCTSTR reason=CGlobalVariable::dlp->DLPCheckModstring_Hard(m_strModVersion,m_strClientSoftware);
			if(reason)
			{
				BanLeecher(reason,5); //hard ban
				return;
			}
			reason=CGlobalVariable::dlp->DLPCheckModstring_Soft(m_strModVersion,m_strClientSoftware);
			if(reason)
			{
				BanLeecher(reason,4); //soft ban
				return;
			}
			else if(IsLeecher()==4)
			{
				m_bLeecher=0;	//unban, because it is now a good mod
				m_strBanMessage.Format(GetResString(IDS_DLP_UNBAN),DbgGetClientInfo());
				old_m_pszUsername.Empty(); //force recheck
			}

		}
	}
	else if(IsLeecher()==4)
	{
		m_bLeecher=0;	//unban, because user doesn't want to check it anymore
		m_strBanMessage.Format(GetResString(IDS_DLP_UNBAN),DbgGetClientInfo());
		old_m_pszUsername.Empty(); //force recheck
		old_m_strClientSoftwareFULL.Empty(); //force recheck if user re enable function
	}


	if(thePrefs.GeTAntiLeecheruserhash() && HasValidHash())
	{
		PBYTE uhash=(PBYTE)GetUserHash();
		LPCTSTR reason=CGlobalVariable::dlp->DLPCheckUserhash(uhash);
		if(reason)
		{
			BanLeecher(_T("*AJ*"),18);
			return;
		}
	}

	if (thePrefs.GetAntiLeecherName())
	{

		//Xman Anti-Nick-Changer
		if(m_pszUsername!=NULL && old_m_pszUsername.IsEmpty()==false)
		{
			if(old_m_pszUsername!=m_pszUsername)
			{
				if(IsLeecher()==0 && m_strModVersion.IsEmpty() //check only if it isn't a known leecher and doesn't send modversion
					&& ::GetTickCount() - m_ulastNickChage < HR2MS(3)) //last nickchane was in less than 3 hours
				{
					m_uNickchanges++;
					if(m_uNickchanges >=3)
					{
						BanLeecher(_T("Nick-Changer"),5); //hard ban
						return;
					}
				}
			}
			else
			{
				//decrease the value if it's the same nick
				if(m_uNickchanges>0)
					m_uNickchanges--;
			}
		}
		//Xman end Anti-Nick-Changer
		
		if(m_bLeecher!=4 && m_pszUsername!=NULL && (old_m_pszUsername.IsEmpty() || old_m_pszUsername!=m_pszUsername)) //remark: because old_m_pszUsername is CString and there operator != is defined, it isn't a pointer comparison 
		{
			old_m_pszUsername = m_pszUsername;
			m_ulastNickChage=::GetTickCount(); //Xman Anti-Nick-Changer

			//find gamer snake 
			if (HasValidHash())
			{
				CString struserhash=md4str(GetUserHash());
				LPCTSTR reason=CGlobalVariable::dlp->DLPCheckNameAndHashAndMod(m_pszUsername,struserhash,m_strModVersion);
				if(reason)
				{
					BanLeecher(reason,10); //soft ban
					return;
				}
			}

			LPCTSTR reason=CGlobalVariable::dlp->DLPCheckUsername_Soft(m_pszUsername);
			if(reason)
			{
				BanLeecher(reason,10); //soft ban
				return;
			}

			reason=CGlobalVariable::dlp->DLPCheckUsername_Hard(m_pszUsername);
			if(reason)
			{
				BanLeecher(reason,5); //hard ban
				return;
			}

			if(IsLeecher()==10 && reason==NULL)
			{
				m_bLeecher=0; //unban it is a good mod now
				m_strBanMessage.Format(GetResString(IDS_DLP_UNBAN),DbgGetClientInfo());
			}
		}
	}
	else if(IsLeecher()==10)
	{
		m_bLeecher=0;	//unban, because user doesn't want to check it anymore
		m_strBanMessage.Format(GetResString(IDS_DLP_UNBAN),DbgGetClientInfo());
		old_m_pszUsername.Empty(); //force recheck if user re enable function
	}

	if (m_nClientVersion > MAKE_CLIENT_VERSION(0, 30, 0) && m_byEmuleVersion > 0 && m_byEmuleVersion != 0x99 && m_clientSoft == SO_EMULE)
	{
		BanLeecher(_T("Fake emuleVersion"),9);
		return;
	} else
	//Xman Anti-Leecher: simple Anti-Thief
	if(m_bLeecher==0 || m_bLeecher==6 || m_bLeecher==11 ) //only check if not banned by other criterion
	{
#if 0
		static 	const float MOD_FLOAT_VERSION= (float)_tstof(CString(MOD_VERSION).Mid(7)) ;
		const float xtremeversion=GetXtremeVersion(m_strModVersion);
		if(thePrefs.GetAntiLeecherThief())
		{
			if(xtremeversion==MOD_FLOAT_VERSION && !StrStrI(m_strClientSoftware,MOD_MAJOR_VERSION))
			{
				BanLeecher(_T("Mod-ID Faker"),6);
				return;
			}
			if(xtremeversion>=4.4f && CString(m_pszUsername).Right(m_strModVersion.GetLength()+1)!=m_strModVersion + _T("?))
			{
				BanLeecher(_T("MOD-ID Faker(advanced)"),6);
				return;
			}


			if(IsLeecher()==6)
			{
				m_strBanMessage.Format(GetResString(IDS_DLP_UNBAN),DbgGetClientInfo());
				m_bLeecher=0; //unban it isn't anymore a mod faker
			}

		}
		else
#endif
			if(IsLeecher()==6)
		{
			m_strBanMessage.Format(GetResString(IDS_DLP_UNBAN),DbgGetClientInfo());
			m_bLeecher=0; //unban, user doesn't want to ban it anymore
		}
		
		//Xman new Anti-Nick-Thief
		if(thePrefs.GetAntiLeecherThief() )
		{
			if(StrStrI(m_pszUsername, str_ANTAddOn)) 
			{
				BanLeecher(_T("Nick Thief"),11);
				return;
			}
			if(IsLeecher()==11)
			{
				m_strBanMessage.Format(GetResString(IDS_DLP_UNBAN),DbgGetClientInfo());
				m_bLeecher=0; //unban, it isn't a nickthief anymore
			}
		}
		else if(IsLeecher()==11)
		{
			m_strBanMessage.Format(GetResString(IDS_DLP_UNBAN),DbgGetClientInfo());
			m_bLeecher=0; //unban, user doesn't want to ban it anymore
		}
	}
	//Xman end simple Anti-Thief
}

void CEd2kUpDownClient::ProcessBanMessage()
{
	if(m_strBanMessage.IsEmpty()==false)
	{
		AddLeecherLogLine(false,m_strBanMessage);
		UpdateUI(UI_UPDATE_PEERLIST);
//		theApp.emuledlg->transferwnd->queuelistctrl.RefreshClient(this);
	}
	m_strBanMessage.Empty();
}

void CEd2kUpDownClient::ProcessMuleCommentPacket(const uchar* pachPacket, uint32 nSize)
{
	if (reqfile && reqfile->IsPartFile())
	{
		CSafeMemFile data(pachPacket, nSize);
		uint8 uRating = data.ReadUInt8();
		if (thePrefs.GetLogRatingDescReceived() && uRating > 0)
			AddDebugLogLine(false, GetResString(IDS_RATINGRECV), m_strClientFilename, uRating);
		CString strComment;
		UINT uLength = data.ReadUInt32();
		if (uLength > 0)
		{
			// we have to increase the raw max. allowed file comment len because of possible UTF8 encoding.
			if (uLength > MAXFILECOMMENTLEN*3)
				uLength = MAXFILECOMMENTLEN*3;
			strComment = data.ReadString(GetUnicodeSupport()!=utf8strNone, uLength);
			if (thePrefs.GetLogRatingDescReceived() && !strComment.IsEmpty())
				AddDebugLogLine(false, GetResString(IDS_DESCRIPTIONRECV), m_strClientFilename, strComment);

			// test if comment is filtered
			if (!thePrefs.GetCommentFilter().IsEmpty())
			{
				CString strCommentLower(strComment);
				strCommentLower.MakeLower();

				int iPos = 0;
				CString strFilter(thePrefs.GetCommentFilter().Tokenize(_T("|"), iPos));
				while (!strFilter.IsEmpty())
				{
					// comment filters are already in lowercase, compare with temp. lowercased received comment
					if (strCommentLower.Find(strFilter) >= 0)
					{
						strComment.Empty();
						uRating = 0;
						break;
					}
					strFilter = thePrefs.GetCommentFilter().Tokenize(_T("|"), iPos);
				}
			}
		}
		if (!strComment.IsEmpty() || uRating > 0)
		{
			m_strFileComment = strComment;
			m_uFileRating = uRating;
			reqfile->UpdateFileRatingCommentAvail();
		}
	}
}

void CEd2kUpDownClient::SendPublicKeyPacket()
{
	// send our public key to the client who requested it
	if (socket == NULL || credits == NULL || m_SecureIdentState != IS_KEYANDSIGNEEDED){
		ASSERT ( false );
		return;
	}
	//  Comment UI
	if (!CGlobalVariable::clientcredits->CryptoAvailable())
		return;

    Packet* packet = new Packet(OP_PUBLICKEY,CGlobalVariable::clientcredits->GetPubKeyLen() + 1,OP_EMULEPROT);
	theStats.AddUpDataOverheadOther(packet->size);
	memcpy(packet->pBuffer+1,CGlobalVariable::clientcredits->GetPublicKey(), CGlobalVariable::clientcredits->GetPubKeyLen());
	packet->pBuffer[0] = CGlobalVariable::clientcredits->GetPubKeyLen();
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		DebugSend("OP__PublicKey", this);
	socket->SendPacket(packet,true,true);
	m_SecureIdentState = IS_SIGNATURENEEDED;
}

void CEd2kUpDownClient::SendSignaturePacket()
{
	// signate the public key of this client and send it
	if (socket == NULL || credits == NULL || m_SecureIdentState == 0){
		ASSERT ( false );
		return;
	}

	//  Comment UI
	if (!CGlobalVariable::clientcredits->CryptoAvailable())
		return;
	if (credits==NULL || credits->GetSecIDKeyLen() == 0)
		return; // We don't have his public key yet, will be back here later
	// do we have a challenge value received (actually we should if we are in this function)
	if (credits->m_dwCryptRndChallengeFrom == 0){
		if (thePrefs.GetLogSecureIdent())
			AddDebugLogLine(false, _T("Want to send signature but challenge value is invalid ('%s')"), GetUserName());
		return;
	}
	// v2
	// we will use v1 as default, except if only v2 is supported
	bool bUseV2;
	if ( (m_bySupportSecIdent&1) == 1 )
		bUseV2 = false;
	else
		bUseV2 = true;

	uint8 byChaIPKind = 0;
	uint32 ChallengeIP = 0;
	if (bUseV2){
		if (CGlobalVariable::serverconnect->GetClientID() == 0 || CGlobalVariable::serverconnect->IsLowID()){
			// we cannot do not know for sure our public ip, so use the remote clients one
			ChallengeIP = GetIP();
			byChaIPKind = CRYPT_CIP_REMOTECLIENT;
		}
		else{
			ChallengeIP = CGlobalVariable::serverconnect->GetClientID();
			byChaIPKind  = CRYPT_CIP_LOCALCLIENT;
		}
	}
	//end v2
	uchar achBuffer[250];
	uint8 siglen = CGlobalVariable::clientcredits->CreateSignature(credits, achBuffer,  250, ChallengeIP, byChaIPKind );
	if (siglen == 0){
		ASSERT ( false );
		return;
	}
	Packet* packet = new Packet(OP_SIGNATURE,siglen + 1+ ( (bUseV2)? 1:0 ),OP_EMULEPROT);
	theStats.AddUpDataOverheadOther(packet->size);
	memcpy(packet->pBuffer+1,achBuffer, siglen);
	packet->pBuffer[0] = siglen;
	if (bUseV2)
		packet->pBuffer[1+siglen] = byChaIPKind;
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		DebugSend("OP__Signature", this);
	socket->SendPacket(packet,true,true);
	m_SecureIdentState = IS_ALLREQUESTSSEND;
}

void CEd2kUpDownClient::ProcessPublicKeyPacket(const uchar* pachPacket, uint32 nSize)
{
	CGlobalVariable::clientlist->AddTrackClient(this);

	if (socket == NULL || credits == NULL || pachPacket[0] != nSize-1
		|| nSize == 0 || nSize > 250){
		ASSERT ( false );
		return;
	}
	//  Comment UI
	if (!CGlobalVariable::clientcredits->CryptoAvailable())
		return;
	// the function will handle everything (mulitple key etc)
	if (credits->SetSecureIdent(pachPacket+1, pachPacket[0])){
		// if this client wants a signature, now we can send him one
		if (m_SecureIdentState == IS_SIGNATURENEEDED){
			SendSignaturePacket();
		}
		else if(m_SecureIdentState == IS_KEYANDSIGNEEDED)
		{
			// something is wrong
			if (thePrefs.GetLogSecureIdent())
				AddDebugLogLine(false, _T("Invalid State error: IS_KEYANDSIGNEEDED in ProcessPublicKeyPacket"));
		}
	}
	else
	{
		if (thePrefs.GetLogSecureIdent())
			AddDebugLogLine(false, _T("Failed to use new received public key"));
	}
}


void CEd2kUpDownClient::ProcessSignaturePacket(const uchar* pachPacket, uint32 nSize)
{
	// here we spread the good guys from the bad ones ;)

	if (socket == NULL || credits == NULL || nSize == 0 || nSize > 250){
		ASSERT ( false );
		return;
	}

	uint8 byChaIPKind;
	if (pachPacket[0] == nSize-1)
		byChaIPKind = 0;
	else if (pachPacket[0] == nSize-2 && (m_bySupportSecIdent & 2) > 0) //v2
		byChaIPKind = pachPacket[nSize-1];
	else{
		ASSERT ( false );
		return;
	}

	//  Comment UI
	if (!CGlobalVariable::clientcredits->CryptoAvailable())
		return;
	
	// we accept only one signature per IP, to avoid floods which need a lot cpu time for cryptfunctions
	if (m_dwLastSignatureIP == GetIP())
	{
		if (thePrefs.GetLogSecureIdent())
			AddDebugLogLine(false, _T("received multiple signatures from one client"));
		return;
	}
	
	// also make sure this client has a public key
	if (credits->GetSecIDKeyLen() == 0)
	{
		if (thePrefs.GetLogSecureIdent())
			AddDebugLogLine(false, _T("received signature for client without public key"));
		return;
	}
	
	// and one more check: did we ask for a signature and sent a challange packet?
	if (credits->m_dwCryptRndChallengeFor == 0)
	{
		if (thePrefs.GetLogSecureIdent())
			AddDebugLogLine(false, _T("received signature for client with invalid challenge value ('%s')"), GetUserName());
		return;
	}

	if (CGlobalVariable::clientcredits->VerifyIdent(credits, pachPacket+1, pachPacket[0], GetIP(), byChaIPKind ) ){
		// result is saved in function abouve
		//if (thePrefs.GetLogSecureIdent())
		//	AddDebugLogLine(false, _T("'%s' has passed the secure identification, V2 State: %i"), GetUserName(), byChaIPKind);
	}
	else
	{
		if (thePrefs.GetLogSecureIdent())
			AddDebugLogLine(false, _T("'%s' has failed the secure identification, V2 State: %i"), GetUserName(), byChaIPKind);
	}
	m_dwLastSignatureIP = GetIP(); 
}

void CEd2kUpDownClient::SendSecIdentStatePacket()
{
	// check if we need public key and signature
	uint8 nValue = 0;
	if (credits){
		//  Comment UI
		if (CGlobalVariable::clientcredits->CryptoAvailable()){
			if (credits->GetSecIDKeyLen() == 0)
				nValue = IS_KEYANDSIGNEEDED;
			else if (m_dwLastSignatureIP != GetIP())
				nValue = IS_SIGNATURENEEDED;
		}
		if (nValue == 0){
			//if (thePrefs.GetLogSecureIdent())
			//	AddDebugLogLine(false, _T("Not sending SecIdentState Packet, because State is Zero"));
			return;
		}
		// crypt: send random data to sign
		uint32 dwRandom = rand()+1;
		credits->m_dwCryptRndChallengeFor = dwRandom;
		Packet* packet = new Packet(OP_SECIDENTSTATE,5,OP_EMULEPROT);
		theStats.AddUpDataOverheadOther(packet->size);
		packet->pBuffer[0] = nValue;
		PokeUInt32(packet->pBuffer+1, dwRandom);
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend("OP__SecIdentState", this);
		socket->SendPacket(packet,true,true);
	}
	else
		ASSERT ( false );
}

void CEd2kUpDownClient::ProcessSecIdentStatePacket(const uchar* pachPacket, uint32 nSize)
{
	if (nSize != 5)
		return;
	if (!credits){
		ASSERT ( false );
		return;
	}
	switch(pachPacket[0]){
		case 0:
			m_SecureIdentState = IS_UNAVAILABLE;
			break;
		case 1:
			m_SecureIdentState = IS_SIGNATURENEEDED;
			break;
		case 2:
			m_SecureIdentState = IS_KEYANDSIGNEEDED;
			break;
	}
	credits->m_dwCryptRndChallengeFrom = PeekUInt32(pachPacket+1);
}

void CEd2kUpDownClient::InfoPacketsReceived()
{
	// indicates that both Information Packets has been received
	// needed for actions, which process data from both packets
	ASSERT ( m_byInfopacketsReceived == IP_BOTH );
	m_byInfopacketsReceived = IP_NONE;
	
	if (m_bySupportSecIdent){
		SendSecIdentStatePacket();
	}
}

void CEd2kUpDownClient::SendPreviewRequest(const CAbstractFile* pForFile)
{
	if (m_fPreviewReqPending == 0){
		m_fPreviewReqPending = 1;
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend("OP__RequestPreview", this, pForFile->GetFileHash());
		Packet* packet = new Packet(OP_REQUESTPREVIEW,16,OP_EMULEPROT);
		md4cpy(packet->pBuffer,pForFile->GetFileHash());
		theStats.AddUpDataOverheadOther(packet->size);
		SafeSendPacket(packet);
	}
	else{
		LogWarning(LOG_STATUSBAR, GetResString(IDS_ERR_PREVIEWALREADY));
	}
}

void CEd2kUpDownClient::SendPreviewAnswer(const CKnownFile* pForFile, CxImage** imgFrames, uint8 nCount)
{
	m_fPreviewAnsPending = 0;
	CSafeMemFile data(1024);
	if (pForFile){
		data.WriteHash16(pForFile->GetFileHash());
	}
	else{
		static const uchar _aucZeroHash[16] = {0};
		data.WriteHash16(_aucZeroHash);
	}
	data.WriteUInt8(nCount);
	for (int i = 0; i != nCount; i++){
		if (imgFrames == NULL){
			ASSERT ( false );
			return;
		}
		CxImage* cur_frame = imgFrames[i];
		if (cur_frame == NULL){
			ASSERT ( false );
			return;
		}
		BYTE* abyResultBuffer = NULL;
		long nResultSize = 0;
		//  Comment UI
		/*if (!cur_frame->Encode(abyResultBuffer, nResultSize, CXIMAGE_FORMAT_PNG)){
			ASSERT ( false );			
			return;
		}*/
		data.WriteUInt32(nResultSize);
		data.Write(abyResultBuffer, nResultSize);
		free(abyResultBuffer);
	}
	Packet* packet = new Packet(&data, OP_EMULEPROT);
	packet->opcode = OP_PREVIEWANSWER;
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		DebugSend("OP__PreviewAnswer", this, (uchar*)packet->pBuffer);
	theStats.AddUpDataOverheadOther(packet->size);
	SafeSendPacket(packet);
}

void CEd2kUpDownClient::ProcessPreviewReq(const uchar* pachPacket, uint32 nSize)
{
	if (nSize < 16)
		throw GetResString(IDS_ERR_WRONGPACKAGESIZE);
	
	if (m_fPreviewAnsPending || thePrefs.CanSeeShares()==vsfaNobody || (thePrefs.CanSeeShares()==vsfaFriends && !IsFriend()))
		return;
	
	m_fPreviewAnsPending = 1;
	CKnownFile* previewFile = CGlobalVariable::sharedfiles->GetFileByID(pachPacket);
	if (previewFile == NULL){
		SendPreviewAnswer(NULL, NULL, 0);
	}
	else{
		previewFile->GrabImage(4,0,true,450,this);
	}
}

void CEd2kUpDownClient::ProcessPreviewAnswer(const uchar* pachPacket, uint32 nSize)
{
	if (m_fPreviewReqPending == 0)
		return;
	m_fPreviewReqPending = 0;
	CSafeMemFile data(pachPacket, nSize);
	uchar Hash[16];
	data.ReadHash16(Hash);
	uint8 nCount = data.ReadUInt8();
	if (nCount == 0){
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_PREVIEWFAILED), GetUserName());
		return;
	}
	CSearchFile* sfile = CGlobalVariable::searchlist->GetSearchFileByHash(Hash);
	if (sfile == NULL){
		//already deleted
		return;
	}

	BYTE* pBuffer = NULL;
	try{
		for (int i = 0; i != nCount; i++){
			uint32 nImgSize = data.ReadUInt32();
			if (nImgSize > nSize)
				throw CString(_T("CUpDownClient::ProcessPreviewAnswer - Provided image size exceeds limit"));
			pBuffer = new BYTE[nImgSize];
			data.Read(pBuffer, nImgSize);
			//  Comment UI
			/*CxImage* image = new CxImage(pBuffer, nImgSize, CXIMAGE_FORMAT_PNG);
			delete[] pBuffer;
			pBuffer = NULL;
			if (image->IsValid()){
				sfile->AddPreviewImg(image);
			}*/
		}
	}
	catch(...){
		delete[] pBuffer;
		throw;
	}
	//  Comment UI
	//(new PreviewDlg())->SetFile(sfile);
}

void CEd2kUpDownClient::ProcessPublicIPAnswer(const BYTE* pbyData, UINT uSize){
	if (uSize != 4)
		throw GetResString(IDS_ERR_WRONGPACKAGESIZE);
	uint32 dwIP = PeekUInt32(pbyData);
	if (m_fNeedOurPublicIP == 1){ // did we?
		m_fNeedOurPublicIP = 0;
		if (CGlobalVariable::GetPublicIP() == 0 && !::IsLowID(dwIP) )
			CGlobalVariable::SetPublicIP(dwIP);
	}	
}

//Xman Anti-Leecher
void CEd2kUpDownClient::BanLeecher(LPCTSTR pszReason, uint8 leechercategory){
	//possible categories:
	//0 = no leecher
	//1 = bad hello + reduce score
	//2 = snafu
	//3 = ghost
	//4 = modstring soft
	//5 = modstring/username hard
	//6 = mod thief
	//7 = spammer
	//8 = XS-Exploiter
	//9 = other (fake emule version/ Credit Hack)
	//10 = username soft
	//11 = nick thief
	//12 = emcrypt
	//13 = bad hello + ban
	//14 = wrong HashSize + reduce score (=new united)
	//15 = snafu = m4 string
	//16 = wrong Startuploadrequest (bionic community)
	//17 = wrong m_fSupportsAICH (applejuice )
	//18 = detected by userhash (AJ) (ban)
	//19 = filefaker (in deadsourcelist but still requesting the file)

	m_strBanMessage.Empty();
	bool reducescore=false;
	switch(leechercategory) 
	{
	case 1:
	case 4:
	case 10:
	case 14:
	case 15:
	case 17:
		reducescore=thePrefs.GetAntiLeecherCommunity_Action();
		break;
	case 12: //emcrypt
		reducescore=true;
		break;
	case 3:
		//Xman always ban ghost mods
		//reducescore=thePrefs.GetAntiLeecherGhost_Action();
		break;
	case 6:
	case 11:
		reducescore=thePrefs.GetAntiLeecherThief_Action();
		break;
	}

	if (m_bLeecher!=leechercategory){
		theStats.leecherclients++;
		m_bLeecher = leechercategory;
		strBanReason_permament=pszReason;

		if(reducescore)
		{
			m_strBanMessage.Format(GetResString(IDS_DLP_REDUCEUSER),pszReason==NULL ? _T("No Reason") : pszReason, DbgGetClientInfo());
			// AddLeecherLogLine(false,_T("[%s](reduce score)- Client %s"),pszReason==NULL ? _T("No Reason") : pszReason, DbgGetClientInfo());
		}
		else {
			m_strBanMessage.Format(GetResString(IDS_DLP_BANUSER),pszReason==NULL ? _T("No Reason") : pszReason, DbgGetClientInfo());
			// AddLeecherLogLine(false,_T("[%s](ban)- Client %s"),pszReason==NULL ? _T("No Reason") : pszReason, DbgGetClientInfo());
		}
	}

	if(reducescore)
		return;

	SetChatState(MS_NONE);
	CGlobalVariable::clientlist->AddTrackClient(this);
	CGlobalVariable::clientlist->AddBannedClient( GetConnectIP() );
	SetUploadState(US_BANNED);
	UINotify(WM_FILE_UPDATE_UPLOADRANK,0,CGlobalVariable::uploadqueue->GetWaitingUserCount());
	UpdateUI(UI_UPDATE_QUEUELIST);
	if (socket != NULL && socket->IsConnected())
		socket->ShutDown(SD_RECEIVE); // let the socket timeout, since we dont want to risk to delete the client right now. This isnt acutally perfect, could be changed later
}
//Xman end

void CEd2kUpDownClient::UDPReaskForDownload()
{
	ASSERT ( reqfile );

	if (!reqfile || m_bUDPPending)
	{
		return;
	}

	//TODO: This should be changed to determine if the last 4 UDP packets failed, not the total one.
	if ( m_nTotalUDPPackets > 3 && ((float)(m_nFailedUDPPackets/m_nTotalUDPPackets) > .3))
	{
		return;
	}

	if (GetUDPPort() != 0 && GetUDPVersion() != 0 && thePrefs.GetUDPPort() != 0 &&
		!CGlobalVariable::IsFirewalled() && !(socket && socket->IsConnected()) && !thePrefs.GetProxySettings().UseProxy)
	{
		if ( !HasLowID()) 
		{
			//don't use UDP to ask for sources
			if (IsSourceRequestAllowed())
			{
				return;
			}

			if (SwapToAnotherFile(_T("A4AF check before OP__ReaskFilePing. CUpDownClient::UDPReaskForDownload()"), true, false, false, NULL, true, true))
			{
				return; // we swapped, so need to go to TCP
			}

			m_bUDPPending = true;
			CSafeMemFile data(128);
			data.WriteHash16(reqfile->GetFileHash());

			if (GetUDPVersion() > 3)
			{
				if (reqfile->IsPartFile())
				{
					((CPartFile*)reqfile)->WritePartStatus(&data);
				}
				else
				{
					data.WriteUInt16(0);
				}
			}

			if (GetUDPVersion() > 2)
			{
				data.WriteUInt16(reqfile->m_nCompleteSourcesCount);
			}

			if (thePrefs.GetDebugClientUDPLevel() > 0)
			{
				DebugSend("OP__ReaskFilePing", this, reqfile->GetFileHash());
			}

			Packet* response = new Packet(&data, OP_EMULEPROT);
			response->opcode = OP_REASKFILEPING;
			theStats.AddUpDataOverheadFileRequest(response->size);
			CGlobalVariable::downloadqueue->AddUDPFileReasks();
			CGlobalVariable::clientudp->SendPacket(response, GetIP(), GetUDPPort(), ShouldReceiveCryptUDPPackets(), GetUserHash(), false, 0);
			m_nTotalUDPPackets++;
		}
		else if (HasLowID() && GetBuddyIP() && GetBuddyPort() && HasValidBuddyID())
		{
			m_bUDPPending = true;
			CSafeMemFile data(128);
			data.WriteHash16(GetBuddyID());
			data.WriteHash16(reqfile->GetFileHash());
			if (GetUDPVersion() > 3)
			{
				if (reqfile->IsPartFile())
				{
					((CPartFile*)reqfile)->WritePartStatus(&data);
				}
				else
				{
					data.WriteUInt16(0);
				}
			}

			if (GetUDPVersion() > 2)
			{
				data.WriteUInt16(reqfile->m_nCompleteSourcesCount);
			}

			if (thePrefs.GetDebugClientUDPLevel() > 0)
			{
				DebugSend("OP__ReaskCallbackUDP", this, reqfile->GetFileHash());
			}

			Packet* response = new Packet(&data, OP_EMULEPROT);
			response->opcode = OP_REASKCALLBACKUDP;
			theStats.AddUpDataOverheadFileRequest(response->size);
			CGlobalVariable::downloadqueue->AddUDPFileReasks();
			CGlobalVariable::clientudp->SendPacket(response, GetBuddyIP(), GetBuddyPort(), false, NULL, true, 0);
			m_nTotalUDPPackets++;
		}
	}
	else if (GetUDPPort() != 0 && GetUDPVersion() != 0 && thePrefs.GetUDPPort() != 0 &&
		!(socket && socket->IsConnected()) && !thePrefs.GetProxySettings().UseProxy)
	{
		if (IsLowToLowClient()) // VC-SearchDream[2007-04-13]: Add IsLowToLowClient to Behave like Real TCP
		{
			//don't use UDP to ask for sources
			if (IsSourceRequestAllowed())
			{
				return;
			}

			if (SwapToAnotherFile(_T("A4AF check before OP__ReaskFilePing. CUpDownClient::UDPReaskForDownload()"), true, false, false, NULL, true, true))
			{
				return; // we swapped, so need to go to TCP
			}

			m_bUDPPending = true;
			CSafeMemFile data(128);
			data.WriteHash16(reqfile->GetFileHash());

			if (GetUDPVersion() > 3)
			{
				if (reqfile->IsPartFile())
				{
					((CPartFile*)reqfile)->WritePartStatus(&data);
				}
				else
				{
					data.WriteUInt16(0);
				}
			}

			if (GetUDPVersion() > 2)
			{
				data.WriteUInt16(reqfile->m_nCompleteSourcesCount);
			}

			if (thePrefs.GetDebugClientUDPLevel() > 0)
			{
				DebugSend("OP__ReaskFilePing", this, reqfile->GetFileHash());
			}

			TRACE(_T("\nCUpDownClient::UDPReaskForDownload\n"));

			Packet* response = new Packet(&data, OP_EMULEPROT);
			response->opcode = OP_REASKFILEPING;
			theStats.AddUpDataOverheadFileRequest(response->size);
			CGlobalVariable::downloadqueue->AddUDPFileReasks();
			CGlobalVariable::clientudp->SendPacket(response, GetIP(), GetUDPPort(), ShouldReceiveCryptUDPPackets(), GetUserHash(), false, 0);
			m_nTotalUDPPackets++;
		}
	}
}

void CEd2kUpDownClient::ProcessEdonkeyQueueRank(const uchar* packet, UINT size)
{
    CSafeMemFile data(packet, size);
    uint32 rank = data.ReadUInt32();
    if (thePrefs.GetDebugClientTCPLevel() > 0)
        Debug(_T("  QR=%u (prev. %d)\n"), rank, IsRemoteQueueFull() ? (UINT)-1 : (UINT)GetRemoteQueueRank());
    SetRemoteQueueRank(rank, GetDownloadState() == DS_ONQUEUE);
    CheckQueueRankFlood();
}

void CEd2kUpDownClient::ProcessEmuleQueueRank(const uchar* packet, UINT size)
{
    if (size != 12)
        throw GetResString(IDS_ERR_BADSIZE);
    uint16 rank = PeekUInt16(packet);
    if (thePrefs.GetDebugClientTCPLevel() > 0)
        Debug(_T("  QR=%u\n"), rank); // no prev. QR available for eMule clients
    SetRemoteQueueFull(false);
    SetRemoteQueueRank(rank, GetDownloadState() == DS_ONQUEUE);
    CheckQueueRankFlood();
}

void CEd2kUpDownClient::CheckQueueRankFlood()
{
    if (m_fQueueRankPending == 0)
    {
        if (GetDownloadState() != DS_DOWNLOADING)
        {
            if (m_fUnaskQueueRankRecv < 3) // NOTE: Do not increase this nr. without increasing the bits for 'm_fUnaskQueueRankRecv'
                m_fUnaskQueueRankRecv++;
            if (m_fUnaskQueueRankRecv == 3)
            {
                if (CGlobalVariable::clientlist->GetBadRequests(this) < 2)
                    CGlobalVariable::clientlist->TrackBadRequest(this, 1);
                if (CGlobalVariable::clientlist->GetBadRequests(this) == 2)
                {
                    CGlobalVariable::clientlist->TrackBadRequest(this, -2); // reset so the client will not be rebanned right after the ban is lifted
                    Ban(_T("QR flood"));
                }
                throw CString(thePrefs.GetLogBannedClients() ? _T("QR flood") : _T(""));
            }
        }
    }
    else
    {
        m_fQueueRankPending = 0;
        m_fUnaskQueueRankRecv = 0;
    }
}

void CEd2kUpDownClient::ProcessFirewallCheckUDPRequest(CSafeMemFile* data){
	if (!Kademlia::CKademlia::IsRunning() || Kademlia::CKademlia::GetUDPListener() == NULL){
		DebugLogWarning(_T("Ignored Kad Firewallrequest UDP because Kad is not running (%s)"), DbgGetClientInfo());
		return;
	}
	// first search if we know this IP already, if so the result might be biased and we need tell the requester 
	bool bErrorAlreadyKnown = false;
	if (GetUploadState() != US_NONE || GetDownloadState() != DS_NONE || GetChatState() != MS_NONE)
		bErrorAlreadyKnown = true;
	else if (Kademlia::CKademlia::GetRoutingZone()->GetContact(ntohl(GetConnectIP()), 0, false) != NULL)
		bErrorAlreadyKnown = true;

	uint16 nRemoteInternPort = data->ReadUInt16();
	uint16 nRemoteExternPort = data->ReadUInt16();
	uint32 dwSenderKey = data->ReadUInt32();
	if (nRemoteInternPort == 0){
		DebugLogError(_T("UDP Firewallcheck requested with Intern Port == 0 (%s)"), DbgGetClientInfo());
		return;
	}
	if (dwSenderKey == 0)
		DebugLogWarning(_T("UDP Firewallcheck requested with SenderKey == 0 (%s)"), DbgGetClientInfo());
	
	CSafeMemFile fileTestPacket1;
	fileTestPacket1.WriteUInt8(bErrorAlreadyKnown ? 1 : 0);
	fileTestPacket1.WriteUInt16(nRemoteInternPort);
	if (thePrefs.GetDebugClientKadUDPLevel() > 0)
		DebugSend("KADEMLIA2_FIREWALLUDP", ntohl(GetConnectIP()), nRemoteInternPort);
	Kademlia::CKademlia::GetUDPListener()->SendPacket(&fileTestPacket1, KADEMLIA2_FIREWALLUDP, ntohl(GetConnectIP())
		, nRemoteInternPort, Kademlia::CKadUDPKey(dwSenderKey, CGlobalVariable::GetPublicIP(false)), NULL);
	
	// if the client has a router with PAT (and therefore a different extern port than intern), test this port too
	if (nRemoteExternPort != 0 && nRemoteExternPort != nRemoteInternPort){
		CSafeMemFile fileTestPacket2;
		fileTestPacket2.WriteUInt8(bErrorAlreadyKnown ? 1 : 0);
		fileTestPacket2.WriteUInt16(nRemoteExternPort);
		if (thePrefs.GetDebugClientKadUDPLevel() > 0)
			DebugSend("KADEMLIA2_FIREWALLUDP", ntohl(GetConnectIP()), nRemoteExternPort);
		Kademlia::CKademlia::GetUDPListener()->SendPacket(&fileTestPacket2, KADEMLIA2_FIREWALLUDP, ntohl(GetConnectIP())
			, nRemoteExternPort, Kademlia::CKadUDPKey(dwSenderKey, CGlobalVariable::GetPublicIP(false)), NULL);
	}
	DebugLog(_T("Answered UDP Firewallcheck request (%s)"), DbgGetClientInfo());
}