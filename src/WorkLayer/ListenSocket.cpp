/* 
 * $Id: ListenSocket.cpp 5137 2008-03-26 06:45:40Z huby $
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
#include "DebugHelpers.h"
//#include "emule.h"
#include "ListenSocket.h"
#include "PeerCacheSocket.h"
#include "opcodes.h"
#include "UpDownClient.h"
#include "ClientList.h"
#include "OtherFunctions.h"
#include "DownloadQueue.h"
#include "Statistics.h"
#include "IPFilter.h"
#include "SharedFileList.h"
#include "PartFile.h"
#include "SafeFile.h"
#include "Packets.h"
#include "UploadQueue.h"
#include "ServerList.h"
#include "Server.h"
#include "Sockets.h"
//#include "emuledlg.h"
//#include "TransferWnd.h"
//#include "ClientListCtrl.h"
//#include "ChatWnd.h"
#include "PeerCacheFinder.h"
#include "Exceptions.h"
#include "Kademlia/Utils/uint128.h"
#include "Kademlia/Kademlia/kademlia.h"
#include "Kademlia/Kademlia/prefs.h"
#include "ClientUDPSocket.h"
#include "SHAHashSet.h"
#include "Log.h"
#include "NatTraversal/NatThread.h"
#include "UIMessage.h"
#include "GlobalVariable.h"
#include "resource.h"

//#include "FirewallOpener.h" // Added by thilon for IFWS - [ICSFirewall]

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CListenSocket::~CListenSocket()
{
	Close();

	KillAllSockets();
}

bool CListenSocket::Rebind()
{
	if (thePrefs.GetPort() == m_port)
		return false;

	Close();
	KillAllSockets();

	return StartListening();
}

//Added by thilon on 2006.09.24, for UPnP
//upnp_start
//bool CListenSocket::StartListening(){
//	//Xman Info about binding
//	if(thePrefs.GetBindAddrW()!=NULL)
//	{
//		AddLogLine(false,_T("You specified an ip-address to bind. Try to bind to: %s"), thePrefs.GetBindAddrW());
//	}
//	//Xman Info about binding
//
//	bool ret=Create(thePrefs.GetPort(), SOCK_STREAM, FD_ACCEPT, thePrefs.GetBindAddrA(), FALSE/*bReuseAddr*/) && Listen();
//
//	//Xman Info about binding
//	if(thePrefs.GetBindAddrW()!=NULL && ret)
//	{
//		AddLogLine(false,_T("binding successful"));
//	}
//	//Xman Info about binding
//
//	if(ret)
//	{	
//		if(mapping)
//		{
//			theApp.RemoveUPnPNatPort(mapping);
//		}
//
//		if(thePrefs.GetUPnPNat())
//		{
//			mapping = new CUPnP::UPNPNAT_MAPPING;
//			mapping->ref = &mapping;
//
//			mapping->internalPort = mapping->externalPort = thePrefs.GetPort();
//			mapping->protocol = CUPnP::UNAT_TCP;
//			mapping->description = "TCP Port";
//			if(theApp.AddUPnPNatPort(mapping, thePrefs.GetUPnPNatTryRandom()))
//				thePrefs.SetUPnPTCPExternal(mapping->externalPort);
//		}
//		else
//		{
//			thePrefs.SetUPnPTCPExternal(thePrefs.GetPort());
//		}
//	}
//
//	if (ret)
//		m_port=thePrefs.GetPort();
//
//	return ret;
//}

//bool CListenSocket::StartListening()
//{
//	bListening = true;
//
//	// Creating the socket with SO_REUSEADDR may solve LowID issues if emule was restarted
//	// quickly or started after a crash, but(!) it will also create another problem. If the
//	// socket is already used by some other application (e.g. a 2nd emule), we though bind
//	// to that socket leading to the situation that 2 applications are listening at the same
//	// port!
//	if (!Create(thePrefs.GetPort(), SOCK_STREAM, FD_ACCEPT, CT2CA(theApp.GetBindAddress()), FALSE/*bReuseAddr*/)) // Added by thilon on 2006.10.18, for [BindToAdapter]
//		return false;
//
//	// Rejecting a connection with conditional WSAAccept and not using SO_CONDITIONAL_ACCEPT
//	// -------------------------------------------------------------------------------------
//	// recv: SYN
//	// send: SYN ACK (!)
//	// recv: ACK
//	// send: ACK RST
//	// recv: PSH ACK + OP_HELLO packet
//	// send: RST
//	// --- 455 total bytes (depending on OP_HELLO packet)
//	// In case SO_CONDITIONAL_ACCEPT is not used, the TCP/IP stack establishes the connection
//	// before WSAAccept has a chance to reject it. That's why the remote peer starts to send
//	// it's first data packet.
//	// ---
//	// Not using SO_CONDITIONAL_ACCEPT gives us 6 TCP packets and the OP_HELLO data. We
//	// have to lookup the IP only 1 time. This is still way less traffic than rejecting the
//	// connection by closing it after the 'Accept'.
//
//	// Rejecting a connection with conditional WSAAccept and using SO_CONDITIONAL_ACCEPT
//	// ---------------------------------------------------------------------------------
//	// recv: SYN
//	// send: ACK RST
//	// recv: SYN
//	// send: ACK RST
//	// recv: SYN
//	// send: ACK RST
//	// --- 348 total bytes
//	// The TCP/IP stack tries to establish the connection 3 times until it gives up. 
//	// Furthermore the remote peer experiences a total timeout of ~ 1 minute which is
//	// supposed to be the default TCP/IP connection timeout (as noted in MSDN).
//	// ---
//	// Although we get a total of 6 TCP packets in case of using SO_CONDITIONAL_ACCEPT,
//	// it's still less than not using SO_CONDITIONAL_ACCEPT. But, we have to lookup
//	// the IP 3 times instead of 1 time.
//
//	//if (thePrefs.GetConditionalTCPAccept() && !thePrefs.GetProxySettings().UseProxy) {
//	//	int iOptVal = 1;
//	//	VERIFY( SetSockOpt(SO_CONDITIONAL_ACCEPT, &iOptVal, sizeof iOptVal) );
//	//}
//
//	if (!Listen())
//		return false;
//
//	m_port = thePrefs.GetPort();
//
//	//// Added by thilon on 2006.10.19, for IFWS - [ICSFirewall]
//	//if(thePrefs.GetICFSupport())
//	//{
//	//	bool bResult = (theApp.m_pFirewallOpener->OpenPort(m_port, NAT_PROTOCOL_TCP, EMULE_DEFAULTRULENAME_TCP, thePrefs.GetICFClearOnClose() /*|| thePrefs.GetUseRandomPorts()*/));
//	//	theApp.QueueLogLine(false, GetResString(bResult ? IDS_FO_TEMPTCP_S : IDS_FO_TEMPTCP_F), m_port);
//	//}
//
//	if(mapping)
//	{
//		theApp.RemoveUPnPNatPort(mapping);
//	}
//
//	if(thePrefs.GetUPnPNat())
//	{
//		mapping = new CUPnP::UPNPNAT_MAPPING;
//		mapping->ref = &mapping;
//
//		mapping->internalPort = mapping->externalPort = thePrefs.GetPort();
//		mapping->protocol = CUPnP::UNAT_TCP;
//		mapping->description = "TCP Port";
//		if(theApp.AddUPnPNatPort(mapping, thePrefs.GetUPnPNatTryRandom()))
//			thePrefs.SetUPnPTCPExternal(mapping->externalPort);
//	}
//	/*else
//	{
//		thePrefs.SetUPnPTCPExternal(thePrefs.GetPort());
//	}*/
//
//	bListening = true;
//	return true;
//}
//upnp_end
bool CListenSocket::StartListening()
{
	//ADDED by fengwen on 2007/03/21	<begin> :	∑¿÷π÷ÿ∏¥Create
	if (bPortListening)
		return true;
	//ADDED by fengwen on 2007/03/21	<end> :		∑¿÷π÷ÿ∏¥Create	

	// Creating the socket with SO_REUSEADDR may solve LowID issues if emule was restarted
	// quickly or started after a crash, but(!) it will also create another problem. If the
	// socket is already used by some other application (e.g. a 2nd emule), we though bind
	// to that socket leading to the situation that 2 applications are listening at the same
	// port!
	if (!Create(thePrefs.GetPort(), SOCK_STREAM, FD_ACCEPT, thePrefs.GetBindAddrA(), FALSE/*bReuseAddr*/))
		return false;

	// Rejecting a connection with conditional WSAAccept and not using SO_CONDITIONAL_ACCEPT
	// -------------------------------------------------------------------------------------
	// recv: SYN
	// send: SYN ACK (!)
	// recv: ACK
	// send: ACK RST
	// recv: PSH ACK + OP_HELLO packet
	// send: RST
	// --- 455 total bytes (depending on OP_HELLO packet)
	// In case SO_CONDITIONAL_ACCEPT is not used, the TCP/IP stack establishes the connection
	// before WSAAccept has a chance to reject it. That's why the remote peer starts to send
	// it's first data packet.
	// ---
	// Not using SO_CONDITIONAL_ACCEPT gives us 6 TCP packets and the OP_HELLO data. We
	// have to lookup the IP only 1 time. This is still way less traffic than rejecting the
	// connection by closing it after the 'Accept'.

	// Rejecting a connection with conditional WSAAccept and using SO_CONDITIONAL_ACCEPT
	// ---------------------------------------------------------------------------------
	// recv: SYN
	// send: ACK RST
	// recv: SYN
	// send: ACK RST
	// recv: SYN
	// send: ACK RST
	// --- 348 total bytes
	// The TCP/IP stack tries to establish the connection 3 times until it gives up. 
	// Furthermore the remote peer experiences a total timeout of ~ 1 minute which is
	// supposed to be the default TCP/IP connection timeout (as noted in MSDN).
	// ---
	// Although we get a total of 6 TCP packets in case of using SO_CONDITIONAL_ACCEPT,
	// it's still less than not using SO_CONDITIONAL_ACCEPT. But, we have to lookup
	// the IP 3 times instead of 1 time.

	//if (thePrefs.GetConditionalTCPAccept() && !thePrefs.GetProxySettings().UseProxy) {
	//	int iOptVal = 1;
	//	VERIFY( SetSockOpt(SO_CONDITIONAL_ACCEPT, &iOptVal, sizeof iOptVal) );
	//}

	if (!Listen())
		return false;
	
	m_port = thePrefs.GetPort();
	bPortListening = true;
	bListening = true;

	return true;
}

void CListenSocket::ReStartListening()
{
	bListening = true;

	ASSERT( m_nPendingConnections >= 0 );
	if (m_nPendingConnections > 0)
	{
		m_nPendingConnections--;
		OnAccept(0);
	}
}

void CListenSocket::StopListening()
{
	bListening = false;
	maxconnectionreached++;
}

static int _iAcceptConnectionCondRejected;

int CALLBACK AcceptConnectionCond(LPWSABUF lpCallerId, LPWSABUF /*lpCallerData*/, LPQOS /*lpSQOS*/, LPQOS /*lpGQOS*/,
								  LPWSABUF /*lpCalleeId*/, LPWSABUF /*lpCalleeData*/, GROUP FAR* /*g*/, DWORD /*dwCallbackData*/)
{
	if (lpCallerId && lpCallerId->buf && lpCallerId->len >= sizeof SOCKADDR_IN)
	{
		LPSOCKADDR_IN pSockAddr = (LPSOCKADDR_IN)lpCallerId->buf;
		ASSERT( pSockAddr->sin_addr.S_un.S_addr != 0 && pSockAddr->sin_addr.S_un.S_addr != INADDR_NONE );

		if (CGlobalVariable::ipfilter->IsFiltered(pSockAddr->sin_addr.S_un.S_addr)){
			if (thePrefs.GetLogFilteredIPs())
				AddDebugLogLine(false, _T("Rejecting connection attempt (IP=%s) - IP filter (%s)"), ipstr(pSockAddr->sin_addr.S_un.S_addr), CGlobalVariable::ipfilter->GetLastHit());
			_iAcceptConnectionCondRejected = 1;
			return CF_REJECT;
		}

		if (CGlobalVariable::clientlist->IsBannedClient(pSockAddr->sin_addr.S_un.S_addr)){
			if (thePrefs.GetLogBannedClients()){
				CUpDownClient* pClient = CGlobalVariable::clientlist->FindClientByIP(pSockAddr->sin_addr.S_un.S_addr);
				AddDebugLogLine(false, _T("Rejecting connection attempt of banned client %s %s"), ipstr(pSockAddr->sin_addr.S_un.S_addr), pClient->DbgGetClientInfo());
			}
			_iAcceptConnectionCondRejected = 2;
			return CF_REJECT;
		}
	}
	else {
		if (thePrefs.GetVerbose())
			DebugLogError(_T("Client TCP socket: AcceptConnectionCond unexpected lpCallerId"));
	}

	return CF_ACCEPT;
}

void CListenSocket::OnAccept(int nErrorCode)
{
	if (!nErrorCode)
	{
		m_nPendingConnections++;
		if (m_nPendingConnections < 1){
			ASSERT(0);
			m_nPendingConnections = 1;
		}

		if (TooManySockets(true) && !CGlobalVariable::serverconnect->IsConnecting()){
			StopListening();
			return;
		}
		else if (!bListening)
			ReStartListening(); //If the client is still at maxconnections, this will allow it to go above it.. But if you don't, you will get a lowID on all servers.

		uint32 nFataErrors = 0;
		while (m_nPendingConnections > 0)
		{
			m_nPendingConnections--;

			CClientReqSocket* newclient;
			SOCKADDR_IN SockAddr = {0};
			int iSockAddrLen = sizeof SockAddr;
			if (thePrefs.GetConditionalTCPAccept() && !thePrefs.GetProxySettings().UseProxy)
			{
				_iAcceptConnectionCondRejected = 0;
				SOCKET sNew = WSAAccept(m_SocketData.hSocket, (SOCKADDR*)&SockAddr, &iSockAddrLen, AcceptConnectionCond, 0);
				if (sNew == INVALID_SOCKET){
					DWORD nError = GetLastError();
					if (nError == WSAEWOULDBLOCK){
						DebugLogError(LOG_STATUSBAR, _T("%hs: Backlogcounter says %u connections waiting, Accept() says WSAEWOULDBLOCK - setting counter to zero!"), __FUNCTION__, m_nPendingConnections);
						m_nPendingConnections = 0;
						break;
					}
					else{
						if (nError != WSAECONNREFUSED || _iAcceptConnectionCondRejected == 0){
							DebugLogError(LOG_STATUSBAR, _T("%hs: Backlogcounter says %u connections waiting, Accept() says %s - setting counter to zero!"), __FUNCTION__, m_nPendingConnections, GetErrorMessage(nError, 1));
							nFataErrors++;
						}
						else if (_iAcceptConnectionCondRejected == 1)
							theStats.filteredclients++;
					}
					if (nFataErrors > 10){
						// the question is what todo on a error. We cant just ignore it because then the backlog will fill up
						// and lock everything. We can also just endlos try to repeat it because this will lock up eMule
						// this should basically never happen anyway
						// however if we are in such a position, try to reinitalize the socket.
						DebugLogError(LOG_STATUSBAR, _T("%hs: Accept() Error Loop, recreating socket"), __FUNCTION__);
						Close();
						StartListening();
						m_nPendingConnections = 0;
						break;
					}
					continue;
				}
				newclient = new CClientReqSocket;
				VERIFY( newclient->InitAsyncSocketExInstance() );
				newclient->m_SocketData.hSocket = sNew;
				newclient->AttachHandle(sNew);

				AddConnection();
			}
			else
			{
				newclient = new CClientReqSocket;
				if (!Accept(*newclient, (SOCKADDR*)&SockAddr, &iSockAddrLen)){
					newclient->Safe_Delete();
					DWORD nError = GetLastError();
					if (nError == WSAEWOULDBLOCK){
						DebugLogError(LOG_STATUSBAR, _T("%hs: Backlogcounter says %u connections waiting, Accept() says WSAEWOULDBLOCK - setting counter to zero!"), __FUNCTION__, m_nPendingConnections);
						m_nPendingConnections = 0;
						break;
					}
					else{
						DebugLogError(LOG_STATUSBAR, _T("%hs: Backlogcounter says %u connections waiting, Accept() says %s - setting counter to zero!"), __FUNCTION__, m_nPendingConnections, GetErrorMessage(nError, 1));
						nFataErrors++;
					}
					if (nFataErrors > 10){
						// the question is what todo on a error. We cant just ignore it because then the backlog will fill up
						// and lock everything. We can also just endlos try to repeat it because this will lock up eMule
						// this should basically never happen anyway
						// however if we are in such a position, try to reinitalize the socket.
						DebugLogError(LOG_STATUSBAR, _T("%hs: Accept() Error Loop, recreating socket"), __FUNCTION__);
						Close();
						StartListening();
						m_nPendingConnections = 0;
						break;
					}
					continue;
				}

				AddConnection();

				if (SockAddr.sin_addr.S_un.S_addr == 0) // for safety..
				{
					iSockAddrLen = sizeof SockAddr;
					newclient->GetPeerName((SOCKADDR*)&SockAddr, &iSockAddrLen);
					DebugLogWarning(_T("SockAddr.sin_addr.S_un.S_addr == 0;  GetPeerName returned %s"), ipstr(SockAddr.sin_addr.S_un.S_addr));
				}

				ASSERT( SockAddr.sin_addr.S_un.S_addr != 0 && SockAddr.sin_addr.S_un.S_addr != INADDR_NONE );

				if (CGlobalVariable::ipfilter->IsFiltered(SockAddr.sin_addr.S_un.S_addr)){
					if (thePrefs.GetLogFilteredIPs())
						AddDebugLogLine(false, _T("Rejecting connection attempt (IP=%s) - IP filter (%s)"), ipstr(SockAddr.sin_addr.S_un.S_addr), CGlobalVariable::ipfilter->GetLastHit());
					newclient->Safe_Delete();
					theStats.filteredclients++;
					continue;
				}

				if (CGlobalVariable::clientlist->IsBannedClient(SockAddr.sin_addr.S_un.S_addr)){
					if (thePrefs.GetLogBannedClients()){
						CUpDownClient* pClient = CGlobalVariable::clientlist->FindClientByIP(SockAddr.sin_addr.S_un.S_addr);
						AddDebugLogLine(false, _T("Rejecting connection attempt of banned client %s %s"), ipstr(SockAddr.sin_addr.S_un.S_addr), pClient->DbgGetClientInfo());
					}
					newclient->Safe_Delete();
					continue;
				}
			}
			newclient->AsyncSelect(FD_WRITE | FD_READ | FD_CLOSE);
		}

		ASSERT( m_nPendingConnections >= 0 );
	}
}

CAsyncSocketEx * CListenSocket::OnAcceptEx(int /*nErrorCode*/)
{
	/*	m_nPendingConnections++;
	if (m_nPendingConnections < 1){
	ASSERT(0);
	m_nPendingConnections = 1;
	}

	if (TooManySockets(true) && !CGlobalVariable::serverconnect->IsConnecting()){
	StopListening();
	return NULL;
	}
	else if (!bListening)
	ReStartListening(); //If the client is still at maxconnections, this will allow it to go above it.. But if you don't, you will get a lowID on all servers.
	*/
	//uint32 nFataErrors = 0;
	//while (m_nPendingConnections > 0)
	//{
	if(m_nPendingConnections>0) m_nPendingConnections--;

	CClientReqSocket* newclient;
	//SOCKADDR_IN SockAddr = {0};
	//int iSockAddrLen = sizeof SockAddr;
	if (thePrefs.GetConditionalTCPAccept() && !thePrefs.GetProxySettings().UseProxy)
	{
		_iAcceptConnectionCondRejected = 0;
		//SOCKET sNew = 100;
		/*SOCKET sNew = WSAAccept(m_SocketData.hSocket, (SOCKADDR*)&SockAddr, &iSockAddrLen, AcceptConnectionCond, 0);
		if (sNew == INVALID_SOCKET)
		{
		DWORD nError = GetLastError();
		if (nError == WSAEWOULDBLOCK)
		{
		DebugLogError(LOG_STATUSBAR, _T("%hs: Backlogcounter says %u connections waiting, Accept() says WSAEWOULDBLOCK - setting counter to zero!"), __FUNCTION__, m_nPendingConnections);
		m_nPendingConnections = 0;
		break;
		}
		else{
		if (nError != WSAECONNREFUSED || _iAcceptConnectionCondRejected == 0){
		DebugLogError(LOG_STATUSBAR, _T("%hs: Backlogcounter says %u connections waiting, Accept() says %s - setting counter to zero!"), __FUNCTION__, m_nPendingConnections, GetErrorMessage(nError, 1));
		nFataErrors++;
		}
		else if (_iAcceptConnectionCondRejected == 1)
		theStats.filteredclients++;
		}
		if (nFataErrors > 10){
		// the question is what todo on a error. We cant just ignore it because then the backlog will fill up
		// and lock everything. We can also just endlos try to repeat it because this will lock up eMule
		// this should basically never happen anyway
		// however if we are in such a position, try to reinitalize the socket.
		DebugLogError(LOG_STATUSBAR, _T("%hs: Accept() Error Loop, recreating socket"), __FUNCTION__);
		Close();
		StartListening();
		m_nPendingConnections = 0;
		break;
		}
		continue;
		}*/
		newclient = new CClientReqSocket;
		newclient->m_bUseNat = true;

		VERIFY( newclient->InitAsyncSocketExInstance() );
		//newclient->m_SocketData.hSocket = sNew;
		//newclient->AttachHandle(sNew);

		AddConnection();
	}
	else
	{
		newclient = new CClientReqSocket;
		newclient->m_bUseNat = true;

		//SOCKET sNew = 100;
		//newclient->m_SocketData.hSocket = sNew;

		/*if (!Accept(*newclient, (SOCKADDR*)&SockAddr, &iSockAddrLen))
		{
		newclient->Safe_Delete();
		DWORD nError = GetLastError();
		if (nError == WSAEWOULDBLOCK){
		DebugLogError(LOG_STATUSBAR, _T("%hs: Backlogcounter says %u connections waiting, Accept() says WSAEWOULDBLOCK - setting counter to zero!"), __FUNCTION__, m_nPendingConnections);
		m_nPendingConnections = 0;
		break;
		}
		else{
		DebugLogError(LOG_STATUSBAR, _T("%hs: Backlogcounter says %u connections waiting, Accept() says %s - setting counter to zero!"), __FUNCTION__, m_nPendingConnections, GetErrorMessage(nError, 1));
		nFataErrors++;
		}
		if (nFataErrors > 10){
		// the question is what todo on a error. We cant just ignore it because then the backlog will fill up
		// and lock everything. We can also just endlos try to repeat it because this will lock up eMule
		// this should basically never happen anyway
		// however if we are in such a position, try to reinitalize the socket.
		DebugLogError(LOG_STATUSBAR, _T("%hs: Accept() Error Loop, recreating socket"), __FUNCTION__);
		Close();
		StartListening();
		m_nPendingConnections = 0;
		break;
		}
		continue;
		}

		AddConnection();

		if (SockAddr.sin_addr.S_un.S_addr == 0) // for safety..
		{
		iSockAddrLen = sizeof SockAddr;
		newclient->GetPeerName((SOCKADDR*)&SockAddr, &iSockAddrLen);
		DebugLogWarning(_T("SockAddr.sin_addr.S_un.S_addr == 0;  GetPeerName returned %s"), ipstr(SockAddr.sin_addr.S_un.S_addr));
		}

		ASSERT( SockAddr.sin_addr.S_un.S_addr != 0 && SockAddr.sin_addr.S_un.S_addr != INADDR_NONE );

		if (CGlobalVariable::ipfilter->IsFiltered(SockAddr.sin_addr.S_un.S_addr)){
		if (thePrefs.GetLogFilteredIPs())
		AddDebugLogLine(false, _T("Rejecting connection attempt (IP=%s) - IP filter (%s)"), ipstr(SockAddr.sin_addr.S_un.S_addr), CGlobalVariable::ipfilter->GetLastHit());
		newclient->Safe_Delete();
		theStats.filteredclients++;
		continue;
		}

		if (CGlobalVariable::clientlist->IsBannedClient(SockAddr.sin_addr.S_un.S_addr)){
		if (thePrefs.GetLogBannedClients()){
		CUpDownClient* pClient = CGlobalVariable::clientlist->FindClientByIP(SockAddr.sin_addr.S_un.S_addr);
		AddDebugLogLine(false, _T("Rejecting connection attempt of banned client %s %s"), ipstr(SockAddr.sin_addr.S_un.S_addr), pClient->DbgGetClientInfo());
		}
		newclient->Safe_Delete();
		continue;
		}*/
	}
	
	// newclient->AsyncSelect(FD_WRITE | FD_READ | FD_CLOSE);

	return newclient;
	//}

	//ASSERT( m_nPendingConnections >= 0 );
}

void CListenSocket::Process()
{
	m_OpenSocketsInterval = 0;
	POSITION pos2;
	for (POSITION pos1 = socket_list.GetHeadPosition(); (pos2 = pos1) != NULL; )
	{
		socket_list.GetNext(pos1);
		CClientReqSocket* cur_sock = socket_list.GetAt(pos2);
		if (cur_sock->IsDeleteThis())
		{
			if (cur_sock->IsValidSocket()){
				cur_sock->Close();			// calls 'closesocket'
			}
			else{
				cur_sock->Delete_Timed();	// may delete 'cur_sock'
			}
		}
		else{
			cur_sock->CheckTimeOut();		// may call 'shutdown'
		}
	}

	if ((GetOpenSockets() + 5 < thePrefs.GetMaxConnections() || CGlobalVariable::serverconnect->IsConnecting()) && !bListening)
		ReStartListening();
}

void CListenSocket::RecalculateStats()
{
	memset(m_ConnectionStates, 0, sizeof m_ConnectionStates);
	for (POSITION pos = socket_list.GetHeadPosition(); pos != NULL; )
	{
		switch (socket_list.GetNext(pos)->GetConState())
		{
		case ES_DISCONNECTED:
			m_ConnectionStates[0]++;
			break;
		case ES_NOTCONNECTED:
			m_ConnectionStates[1]++;
			break;
		case ES_CONNECTED:
			m_ConnectionStates[2]++;
			break;
		}
	}
}

void CListenSocket::AddSocket(CClientReqSocket* toadd)
{
	socket_list.AddTail(toadd);
}

void CListenSocket::RemoveSocket(CClientReqSocket* todel)
{
	for (POSITION pos = socket_list.GetHeadPosition(); pos != NULL; )
	{
		POSITION posLast = pos;
		if (socket_list.GetNext(pos) == todel)
			socket_list.RemoveAt(posLast);
	}
}

void CListenSocket::KillAllSockets()
{
	for (POSITION pos = socket_list.GetHeadPosition(); pos != 0; pos = socket_list.GetHeadPosition())
	{
		CClientReqSocket* cur_socket = socket_list.GetAt(pos);
		if (cur_socket->client)
			delete cur_socket->client;
		else
			delete cur_socket;
	}
}

void CListenSocket::AddConnection()
{
	m_OpenSocketsInterval++;
}

bool CListenSocket::TooManySockets( bool bIgnoreInterval, bool bUseTcp/*=false*/, bool bUseNat/*=false*/ )
{
	if (   GetOpenSockets() > thePrefs.GetMaxConnections()
		|| (m_OpenSocketsInterval > (thePrefs.GetMaxConperFive() * GetMaxConperFiveModifier()) && !bIgnoreInterval) )
		return true;

	if( bUseTcp && m_nHalfOpen >= thePrefs.GetMaxHalfConnections() && !bIgnoreInterval )
		return true;

	if(	bUseNat && m_nHalfOpenOfL2L >= thePrefs.GetMaxL2LHalfConnections() && !bIgnoreInterval ) 
		return true;

	return false;
}

bool CListenSocket::IsValidSocket(CClientReqSocket* totest)
{
	return socket_list.Find(totest) != NULL;
}

#ifdef _DEBUG
void CListenSocket::Debug_ClientDeleted(CUpDownClient* deleted)
{
	for (POSITION pos = socket_list.GetHeadPosition(); pos != NULL;)
	{
		CClientReqSocket* cur_sock = socket_list.GetNext(pos);
		if (!AfxIsValidAddress(cur_sock, sizeof(CClientReqSocket)))
			AfxDebugBreak();
		if (thePrefs.m_iDbgHeap >= 2)
			ASSERT_VALID(cur_sock);
		if (cur_sock->client == deleted)
			AfxDebugBreak();
	}
}
#endif

void CListenSocket::UpdateConnectionsStatus()
{
	activeconnections = GetOpenSockets();

	// Update statistics for 'peak connections'
	if (peakconnections < activeconnections)
		peakconnections = activeconnections;
	if (peakconnections > thePrefs.GetConnPeakConnections())
		thePrefs.SetConnPeakConnections(peakconnections);

	//  Comment UI
	if (CGlobalVariable::IsConnected())
	{
		totalconnectionchecks++;
		if (totalconnectionchecks == 0) {
			// wrap around occured, avoid division by zero
			totalconnectionchecks = 100;
		}

		// Get a weight for the 'avg. connections' value. The longer we run the higher 
		// gets the weight (the percent of 'avg. connections' we use).
		float fPercent = (float)(totalconnectionchecks - 1) / (float)totalconnectionchecks;
		if (fPercent > 0.99F)
			fPercent = 0.99F;

		// The longer we run the more we use the 'avg. connections' value and the less we
		// use the 'active connections' value. However, if we are running quite some time
		// without any connections (except the server connection) we will eventually create 
		// a floating point underflow exception.
		averageconnections = averageconnections * fPercent + activeconnections * (1.0F - fPercent);
		if (averageconnections < 0.001F)
			averageconnections = 0.001F;	// avoid floating point underflow
	}
}

float CListenSocket::GetMaxConperFiveModifier()
{
	float SpikeSize = GetOpenSockets() - averageconnections;
	if (SpikeSize < 1.0F)
		return 1.0F;

	float SpikeTolerance = 25.0F * (float)thePrefs.GetMaxConperFive() / 10.0F;
	if (SpikeSize > SpikeTolerance)
		return 0;

	float Modifier = 1.0F - SpikeSize / SpikeTolerance;
	return Modifier;
}

bool CListenSocket::SendPortTestReply(char result, bool disconnect)
{
	POSITION pos2;
	for(POSITION pos1 = socket_list.GetHeadPosition(); ( pos2 = pos1 ) != NULL; )
	{
		socket_list.GetNext(pos1);
		CClientReqSocket* cur_sock = socket_list.GetAt(pos2);
		if (cur_sock->m_bPortTestCon)
		{
			if (thePrefs.GetDebugClientTCPLevel() > 0)
				DebugSend("OP__PortTest", cur_sock->client);
			Packet* replypacket = new Packet(OP_PORTTEST, 1);
			replypacket->pBuffer[0]=result;
			theStats.AddUpDataOverheadOther(replypacket->size);
			cur_sock->SendPacket(replypacket);
			if (disconnect)
				cur_sock->m_bPortTestCon = false;
			return true;
		}
	}
	return false;
}

CListenSocket::CListenSocket()
{
	bPortListening = false;
	bListening = false;
	maxconnectionreached = 0;
	m_OpenSocketsInterval = 0;
	m_nPendingConnections = 0;
	memset(m_ConnectionStates, 0, sizeof m_ConnectionStates);
	peakconnections = 0;
	totalconnectionchecks = 0;
	averageconnections = 0.0;
	activeconnections = 0;
	m_port=0;
	m_nHalfOpen = 0;
	m_nHalfOpenOfL2L = 0;
	m_nComp = 0;
}
