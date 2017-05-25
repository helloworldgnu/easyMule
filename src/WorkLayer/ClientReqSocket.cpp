/* 
 * $Id: ClientReqSocket.cpp 11398 2009-03-17 11:00:27Z huby $
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

#include "Ed2kUpDownClient.h"

//#include "FirewallOpener.h" // Added by thilon for IFWS - [ICSFirewall]

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// CClientReqSocket

IMPLEMENT_DYNCREATE(CClientReqSocket, CEMSocket)

CClientReqSocket::CClientReqSocket(CUpDownClient* in_client)
{
	SetClient(in_client);

	ResetTimeOutTimer();
	m_bDeleteThis = false;
	m_bDisconnectAll = false;
	deltimer = 0;
	m_bPortTestCon=false;
	m_nOnConnect=SS_Other;

	CGlobalVariable::listensocket->AddSocket(this);
}

void CClientReqSocket::SetConState( SocketState val )
{
	//If no change, do nothing..
	if ( (UINT)val == m_nOnConnect )
		return;
	//Decrease count of old state..
	switch ( m_nOnConnect )
	{
	case SS_Half:
		{
			if(m_bUseNat)
				CGlobalVariable::listensocket->m_nHalfOpenOfL2L--;
			else
				CGlobalVariable::listensocket->m_nHalfOpen--;
			break;
		}
	case SS_Complete:
		CGlobalVariable::listensocket->m_nComp--;
	}
	//Set state to new state..
	m_nOnConnect = val;
	//Increase count of new state..
	switch ( m_nOnConnect )
	{
	case SS_Half:
		{
			if(m_bUseNat)
				CGlobalVariable::listensocket->m_nHalfOpenOfL2L++;
			else
				CGlobalVariable::listensocket->m_nHalfOpen++;
			break;
		}
	case SS_Complete:
		CGlobalVariable::listensocket->m_nComp++;
	}
}

void CClientReqSocket::WaitForOnConnect()
{
	SetConState(SS_Half);
}

CClientReqSocket::~CClientReqSocket()
{
	//This will update our statistics.
	SetConState(SS_Other);
	if (client && (client->m_iPeerType&ptFtp)==0 )
		client->socket = 0;
	client = 0;

	CGlobalVariable::uploadBandwidthThrottler->RemoveFromAllQueues(this);
	CGlobalVariable::listensocket->RemoveSocket(this);
	if (CGlobalVariable::clientlist)
	{
		DEBUG_ONLY (CGlobalVariable::clientlist->Debug_SocketDeleted(this));
	}
}

void CClientReqSocket::SetClient(CUpDownClient* pClient)
{
	client = pClient;
	if (client)
		client->socket = this;
}

void CClientReqSocket::ResetTimeOutTimer()
{
	timeout_timer = ::GetTickCount();
}

UINT CClientReqSocket::GetTimeOut()
{
	// PC-TODO
	// the PC socket may even already be disconnected and deleted and we still need to keep the
	// ed2k socket open because remote client may still be downloading from cache.
	if (client && client->IsUploadingToPeerCache() && (client->m_pPCUpSocket == NULL || !client->m_pPCUpSocket->IsConnected()))
	{
		// we are uploading (or at least allow uploading) but currently no socket
		return max(CEMSocket::GetTimeOut(), GetPeerCacheSocketUploadTimeout());
	}
	else if (client && client->m_pPCUpSocket && client->m_pPCUpSocket->IsConnected())
	{
		// we have an uploading PC socket, but that socket is not used (nor can it be closed)
		return max(CEMSocket::GetTimeOut(), client->m_pPCUpSocket->GetTimeOut());
	}
	else if (client && client->m_pPCDownSocket && client->m_pPCDownSocket->IsConnected())
	{
		// we have a downloading PC socket
		return max(CEMSocket::GetTimeOut(), client->m_pPCDownSocket->GetTimeOut());
	}
	else
		return CEMSocket::GetTimeOut();
}

bool CClientReqSocket::CheckTimeOut()
{
	if (m_nOnConnect == SS_Half)
	{
		// VC-yunchenn.chen[2007-01-15]: if the socket is doing NAT-Traverse, don't disconnect it.
		//  If the socket is not used by natthread, GetUsedTime() returns 0
/*
		if (CGlobalVariable::natthread && CGlobalVariable::natthread->GetUsedTime(this))
		{
			timeout_timer = ::GetTickCount();
			return false;
		}
*/
		//This socket is still in a half connection state.. Because of SP2, we don't know
		//if this socket is actually failing, or if this socket is just queued in SP2's new
		//protection queue. Therefore we give the socket a chance to either finally report
		//the connection error, or finally make it through SP2's new queued socket system..
		if (::GetTickCount() - timeout_timer > CEMSocket::GetTimeOut()*4)
		{
			timeout_timer = ::GetTickCount();
			CString str;
			str.Format(_T("Timeout: State:%u = SS_Half"), m_nOnConnect);
			Disconnect(str);
			return true;
		}
		return false;
	}
	UINT uTimeout = GetTimeOut();
	if (client)
	{
		if (client->GetKadState() == KS_CONNECTED_BUDDY)
		{
			uTimeout += MIN2MS(15);
		}
		if (client->GetChatState()!=MS_NONE)
		{
			//We extend the timeout time here to avoid people chatting from disconnecting to fast.
			uTimeout += CONNECTION_TIMEOUT;
		}
	}

	if (::GetTickCount() - timeout_timer > uTimeout)
	{
		// VC-yunchenn.chen[2007-03-28]: check timeout for natsock
		if(CGlobalVariable::natthread)
		{
			//MODIFIED by VC-fengwen on 2008/01/16 <begin> : NatSocket的Timeout很短，容易误删好的连接
			//int timeout_nat=CGlobalVariable::natthread->GetTimeout(this)*4;
			int timeout_nat=CONNECTION_TIMEOUT;
			//MODIFIED by VC-fengwen on 2008/01/16 <end> : NatSocket的Timeout很短，容易误删好的连接

			if((int)(::GetTickCount() - timeout_timer) < timeout_nat)
				return false;
			TRACE("timeout_nat = %d\n", timeout_nat);
		}

		timeout_timer = ::GetTickCount();
		CString str;
		str.Format(_T("Timeout: State:%u (0 = SS_Other, 1 = SS_Half, 2 = SS_Complete"), m_nOnConnect);
		Disconnect(str);
		//  added by yunchenn 2006.12.18
		//if (CGlobalVariable::natthread && m_bUseNat) CGlobalVariable::natthread->RemoveSocket(this);
		return true;
	}
	return false;
}

void CClientReqSocket::OnClose(int nErrorCode)
{
	ASSERT (CGlobalVariable::listensocket->IsValidSocket(this));
	CEMSocket::OnClose(nErrorCode);

	LPCTSTR pszReason;
	CString* pstrReason = NULL;
	if (nErrorCode == 0)
		pszReason = _T("Close");
	else if (thePrefs.GetVerbose())
	{
		pstrReason = new CString;
		*pstrReason = GetErrorMessage(nErrorCode, 1);
		pszReason = *pstrReason;
	}
	else
		pszReason = NULL;
	Disconnect(pszReason);
	delete pstrReason;
}

void CClientReqSocket::Disconnect(LPCTSTR pszReason)
{
	AsyncSelect(0);
	byConnected = ES_DISCONNECTED;
	if (!client)
	{
		Safe_Delete();
	}
	else
	{
		if( client->Disconnected(CString(_T("CClientReqSocket::Disconnect(): ")) + pszReason, true,this) )
		{			
			CUpDownClient* temp = client;
			//client->socket = NULL;
			//client = NULL;
			SetDisconnetAll();
			Safe_Delete();
			delete temp;
		}
		else
		{
			Safe_Delete();
			client = NULL;
		}
	}
};

void CClientReqSocket::Delete_Timed()
{
	// it seems that MFC Sockets call socketfunctions after they are deleted, even if the socket is closed
	// and select(0) is set. So we need to wait some time to make sure this doesn't happens


	//MODIFIED by VC-fengwen on 2007/09/20 <begin> : 尝试解决EMSocket的一些Crash，可能是由于Socket还在处理时，对象被删除。
	////  added by yunchenn
	//if(m_bUseNat && theApp.natthread)
	//{
	//	if(time(NULL) - theApp.natthread->GetUsedTime(this) > TM_KEEPALIVE*4)
	//	{
	//		delete this;
	//	}
	//}
	//else
	//{
	//	if (::GetTickCount() - deltimer > 10000)
	//	{
	//		delete this;
	//	}
	//}

	//  added by yunchenn
	if (::GetTickCount() - deltimer > 10000)
	{
		if(m_bUseNat && CGlobalVariable::natthread)
		{
			if(time(NULL) - CGlobalVariable::natthread->GetUsedTime(this) > TM_KEEPALIVE*4)
			{
				delete this;
			}
#ifdef _DEBUG_PEER
			else
			{
				ASSERT( 0 );
			}
#endif
		}
		else
		{
			delete this;
		}
	}
	//MODIFIED by VC-fengwen on 2007/09/20 <end> : 尝试解决EMSocket的一些Crash，可能是由于Socket还在处理时，对象被删除。

}

void CClientReqSocket::Safe_Delete()
{
	ASSERT (CGlobalVariable::listensocket->IsValidSocket(this));
	AsyncSelect(0);
	deltimer = ::GetTickCount();
	
	if (m_SocketData.hSocket != INVALID_SOCKET) // deadlake PROXYSUPPORT - changed to AsyncSocketEx
		ShutDown(SD_BOTH);

	if( CGlobalVariable::natthread && m_bUseNat )
	{
		if( client && client->GetUserHash() )
			CGlobalVariable::natthread->RemoveStrategy(client->GetUserHash());
		CGlobalVariable::natthread->RemoveSocket(this,m_bDisconnectAll,client? client->GetUserHash() : NULL);
	}

	if (client) {
        // 避免FTP的数据通道会关闭命令通道的SOCKET
		if( client->socket == this ) {
			client->socket = 0;
		}
#ifdef _DEBUG_PEER
		else if( client->socket )
			AfxDebugBreak( );
#endif

		client = 0;
	}

	DEBUG_ONLY (CGlobalVariable::clientlist->Debug_SocketDeleted(this));

	byConnected = ES_DISCONNECTED;
	m_bDeleteThis = true;
	
	CGlobalVariable::uploadBandwidthThrottler->RemoveFromAllQueues(this);//ADDED by VC-fengwen on 2007/09/24 : 尝试解决EMSocket的一些Crash：此时应清空待发数据，再等上几秒，之后再delete，应该不会有错了。
}

bool CClientReqSocket::ProcessPacket(const BYTE* packet, uint32 size, UINT opcode)
{
	try
	{
		try
		{
			if (!client && opcode != OP_HELLO)
			{
				theStats.AddDownDataOverheadOther(size);
				throw GetResString(IDS_ERR_NOHELLO);
			}
			else if (client && opcode != OP_HELLO && opcode != OP_HELLOANSWER)
				client->CheckHandshakeFinished();
			CEd2kUpDownClient * ed2k_client = STATIC_DOWNCAST( CEd2kUpDownClient , client );
			if( client && 0 == ed2k_client )
				throw "error";

			switch (opcode)
			{
			case OP_HELLOANSWER:
				{
					theStats.AddDownDataOverheadOther(size);
					ed2k_client->ProcessHelloAnswer(packet,size);
					ed2k_client->ProcessBanMessage(); //Xman Anti-Leecher

					if (thePrefs.GetDebugClientTCPLevel() > 0){
						DebugRecv("OP_HelloAnswer", client);
						Debug(_T("  %s\n"), client->DbgGetHelloInfo());
					}

					// start secure identification, if
					//  - we have received OP_EMULEINFO and OP_HELLOANSWER (old eMule)
					//	- we have received eMule-OP_HELLOANSWER (new eMule)
					if (ed2k_client->GetInfoPacketsReceived() == IP_BOTH)
						ed2k_client->InfoPacketsReceived();

					if (client)
					{
						client->ConnectionEstablished();

						//  Comment UI
						//SendMessage(CGlobalVariable::m_hListenWnd,WM_FILE_UPDATE_PEER,0,(LPARAM)client);
						client->UpdateUI(UI_UPDATE_PEERLIST);
						//theApp.emuledlg->transferwnd->clientlistctrl.RefreshClient(client);
					}
					break;
				}
			case OP_HELLO:
				{
					theStats.AddDownDataOverheadOther(size);

					bool bNewClient = !client;
					if (bNewClient)
					{
						// create new client to save standart informations
						ed2k_client = new CEd2kUpDownClient(this);
						client = ed2k_client;
					}

					bool bIsMuleHello = false;
					try
					{
						bIsMuleHello = ed2k_client->ProcessHelloPacket(packet,size);
					}
					catch (...)
					{
						if (bNewClient)
						{
							// Don't let CUpDownClient::Disconnected be processed for a client which is not in the list of clients.
							delete client;
							client = NULL;
						}
						throw;
					}

					if (thePrefs.GetDebugClientTCPLevel() > 0)
					{
						DebugRecv("OP_Hello", client);
						Debug(_T("  %s\n"), client->DbgGetHelloInfo());
					}

					// now we check if we know this client already. if yes this socket will
					// be attached to the known client, the new client will be deleted
					// and the var. "client" will point to the known client.
					// if not we keep our new-constructed client ;)
					if (CGlobalVariable::clientlist->AttachToAlreadyKnown(&client,this))
					{
						ed2k_client = STATIC_DOWNCAST( CEd2kUpDownClient , client );
						if( 0 == ed2k_client )
							throw "error";

						// update the old client informations
						bIsMuleHello = ed2k_client->ProcessHelloPacket(packet,size);
						ed2k_client->ProcessBanMessage(); //Xman Anti-Leecher

					}
					else
					{
						CGlobalVariable::clientlist->AddClient(client);
						ed2k_client->SetCommentDirty();
						ed2k_client->ProcessBanMessage(); //Xman Anti-Leecher
					}

					//  Comment UI
					//SendMessage(CGlobalVariable::m_hListenWnd,WM_FILE_UPDATE_PEER, ,(LPARAM)client);
					client->UpdateUI(UI_UPDATE_PEERLIST);
					//theApp.emuledlg->transferwnd->clientlistctrl.RefreshClient(client);

					//Xman don't continue sending after banning
					if(client && client->GetUploadState()==US_BANNED)
						break;
					//Xman end

					// send a response packet with standart informations
					if (client->GetHashType() == SO_EMULE && !bIsMuleHello)
						ed2k_client->SendMuleInfoPacket(false);

					ed2k_client->SendHelloAnswer();

					if (client)
						client->ConnectionEstablished();

					ASSERT( client );
					if (client)
					{
						// start secure identification, if
						//	- we have received eMule-OP_HELLO (new eMule)
						if (ed2k_client->GetInfoPacketsReceived() == IP_BOTH)
							ed2k_client->InfoPacketsReceived();

						if ( client->GetKadPort() )
							Kademlia::CKademlia::Bootstrap(ntohl(client->GetIP()), client->GetKadPort(), (client->GetKadVersion() > 1));
					}
					break;
				}
			case OP_REQUESTFILENAME:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_FileRequest", client, (size >= 16) ? packet : NULL);
					theStats.AddDownDataOverheadFileRequest(size);

					if (size >= 16)
					{
						if (!client->GetWaitStartTime())
							client->SetWaitStartTime();

						CSafeMemFile data_in(packet, size);
						uchar reqfilehash[16];
						data_in.ReadHash16(reqfilehash);

						CKnownFile* reqfile;
						if ( (reqfile = CGlobalVariable::sharedfiles->GetFileByID(reqfilehash)) == NULL )
						{
							if ( !((reqfile = CGlobalVariable::downloadqueue->GetFileByID(reqfilehash)) != NULL
								&& reqfile->GetFileSize() > (uint64)PARTSIZE) )
							{
								client->CheckFailedFileIdReqs(reqfilehash);
								break;
							}
						}

						if (reqfile->IsLargeFile() && !client->SupportsLargeFiles())
						{
							DebugLogWarning(_T("Client without 64bit file support requested large file; %s, File=\"%s\""), client->DbgGetClientInfo(), reqfile->GetFileName());
							break;
						}

						// check to see if this is a new file they are asking for
						if (md4cmp(client->GetUploadFileID(), reqfilehash) != 0)
							ed2k_client->SetCommentDirty();
						client->SetUploadFileID(reqfile);

						//Xman FileFaker detection
						if(reqfile->IsPartFile() && ((CPartFile*)reqfile)->m_DeadSourceList.IsDeadSource(client))
						{
							ed2k_client->BanLeecher(_T("FileFaker"),19);
							ed2k_client->ProcessBanMessage();
							break;
						}

						if (!client->ProcessExtendedInfo(&data_in, reqfile)){
							if (thePrefs.GetDebugClientTCPLevel() > 0)
								DebugSend("OP__FileReqAnsNoFil", client, packet);
							Packet* replypacket = new Packet(OP_FILEREQANSNOFIL, 16);
							md4cpy(replypacket->pBuffer, reqfile->GetFileHash());
							theStats.AddUpDataOverheadFileRequest(replypacket->size);
							SendPacket(replypacket, true);
							DebugLogWarning(_T("Partcount mismatch on requested file, sending FNF; %s, File=\"%s\""), client->DbgGetClientInfo(), reqfile->GetFileName());
							break;
						}

						// if we are downloading this file, this could be a new source
						// no passive adding of files with only one part
						if (reqfile->IsPartFile() && reqfile->GetFileSize() > (uint64)PARTSIZE)
						{
							if (((CPartFile*)reqfile)->GetMaxSources() > ((CPartFile*)reqfile)->GetSourceCount())
								CGlobalVariable::downloadqueue->CheckAndAddKnownSource((CPartFile*)reqfile, client, true);
						}

						// send filename etc
						CSafeMemFile data_out(128);
						data_out.WriteHash16(reqfile->GetFileHash());
						data_out.WriteString(reqfile->GetFileName(), client->GetUnicodeSupport());
						Packet* packet = new Packet(&data_out);
						packet->opcode = OP_REQFILENAMEANSWER;
						if (thePrefs.GetDebugClientTCPLevel() > 0)
							DebugSend("OP__FileReqAnswer", client, reqfile->GetFileHash());
						theStats.AddUpDataOverheadFileRequest(packet->size);
						SendPacket(packet, true);

						client->SendCommentInfo(reqfile);
						break;
					}
					throw GetResString(IDS_ERR_WRONGPACKAGESIZE);
					break;
				}
			case OP_SETREQFILEID:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_SetReqFileID", client, (size >= 16) ? packet : NULL);
					theStats.AddDownDataOverheadFileRequest(size);

					if (size == 16)
					{
						if (!client->GetWaitStartTime())
							client->SetWaitStartTime();

						CKnownFile* reqfile;
						if ( (reqfile = CGlobalVariable::sharedfiles->GetFileByID(packet)) == NULL )
						{
							if ( !((reqfile = CGlobalVariable::downloadqueue->GetFileByID(packet)) != NULL
								&& reqfile->GetFileSize() > (uint64)PARTSIZE) )
							{
								// send file request no such file packet (0x48)
								if (thePrefs.GetDebugClientTCPLevel() > 0)
									DebugSend("OP__FileReqAnsNoFil", client, packet);
								Packet* replypacket = new Packet(OP_FILEREQANSNOFIL, 16);
								md4cpy(replypacket->pBuffer, packet);
								theStats.AddUpDataOverheadFileRequest(replypacket->size);
								SendPacket(replypacket, true);
								client->CheckFailedFileIdReqs(packet);
								break;
							}
						}
						if (reqfile->IsLargeFile() && !client->SupportsLargeFiles())
						{
							if (thePrefs.GetDebugClientTCPLevel() > 0)
								DebugSend("OP__FileReqAnsNoFil", client, packet);
							Packet* replypacket = new Packet(OP_FILEREQANSNOFIL, 16);
							md4cpy(replypacket->pBuffer, packet);
							theStats.AddUpDataOverheadFileRequest(replypacket->size);
							SendPacket(replypacket, true);
							DebugLogWarning(_T("Client without 64bit file support requested large file; %s, File=\"%s\""), client->DbgGetClientInfo(), reqfile->GetFileName());
							break;
						}

						// check to see if this is a new file they are asking for
						if (md4cmp(client->GetUploadFileID(), packet) != 0)
							ed2k_client->SetCommentDirty();

						client->SetUploadFileID(reqfile);

						// send filestatus
						CSafeMemFile data(16+16);
						data.WriteHash16(reqfile->GetFileHash());
						if (reqfile->IsPartFile())
							((CPartFile*)reqfile)->WritePartStatus(&data);
						else
							data.WriteUInt16(0);
						Packet* packet = new Packet(&data);
						packet->opcode = OP_FILESTATUS;
						if (thePrefs.GetDebugClientTCPLevel() > 0)
							DebugSend("OP__FileStatus", client, reqfile->GetFileHash());
						theStats.AddUpDataOverheadFileRequest(packet->size);
						SendPacket(packet, true);
						break;
					}
					throw GetResString(IDS_ERR_WRONGPACKAGESIZE);
					break;
				}
			case OP_FILEREQANSNOFIL:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_FileReqAnsNoFil", client, (size >= 16) ? packet : NULL);
					theStats.AddDownDataOverheadFileRequest(size);
					if (size == 16)
					{
						CPartFile* reqfile = CGlobalVariable::downloadqueue->GetFileByID(packet);
						if (!reqfile)
						{
							client->CheckFailedFileIdReqs(packet);
							break;
						}
						else
							reqfile->m_DeadSourceList.AddDeadSource(client);
						// if that client does not have my file maybe has another different
						// we try to swap to another file ignoring no needed parts files
						switch (client->GetDownloadState())
						{
						case DS_CONNECTED:
						case DS_ONQUEUE:
						case DS_NONEEDEDPARTS:
							client->DontSwapTo(client->GetRequestFile()); // ZZ:DownloadManager
							if (!client->SwapToAnotherFile(_T("Source says it doesn't have the file. CClientReqSocket::ProcessPacket()"), true, true, true, NULL, false, false))
							{ // ZZ:DownloadManager
								CGlobalVariable::downloadqueue->RemoveSource(client);
							}
							break;
						}
						break;
					}
					throw GetResString(IDS_ERR_WRONGPACKAGESIZE);
					break;
				}
			case OP_REQFILENAMEANSWER:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_FileReqAnswer", client, (size >= 16) ? packet : NULL);
					theStats.AddDownDataOverheadFileRequest(size);

					CSafeMemFile data(packet, size);
					uchar cfilehash[16];
					data.ReadHash16(cfilehash);
					CPartFile* file = CGlobalVariable::downloadqueue->GetFileByID(cfilehash);
					if (file == NULL)
						client->CheckFailedFileIdReqs(cfilehash);
					client->ProcessFileInfo(&data, file);
					break;
				}
			case OP_FILESTATUS:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_FileStatus", client, (size >= 16) ? packet : NULL);
					theStats.AddDownDataOverheadFileRequest(size);

					CSafeMemFile data(packet, size);
					uchar cfilehash[16];
					data.ReadHash16(cfilehash);
					CPartFile* file = CGlobalVariable::downloadqueue->GetFileByID(cfilehash);
					if (file == NULL)
						client->CheckFailedFileIdReqs(cfilehash);
					client->ProcessFileStatus(false, &data, file);
					break;
				}
			case OP_STARTUPLOADREQ:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_StartUpLoadReq", client, (size >= 16) ? packet : NULL);
					theStats.AddDownDataOverheadFileRequest(size);

					if (!client->CheckHandshakeFinished())
						break;
					if (size == 16)
					{
						CKnownFile* reqfile = CGlobalVariable::sharedfiles->GetFileByID(packet);
						if (reqfile)
						{
							if (md4cmp(client->GetUploadFileID(), packet) != 0)
								ed2k_client->SetCommentDirty();
							client->SetUploadFileID(reqfile);
							client->SendCommentInfo(reqfile);
							CGlobalVariable::uploadqueue->AddClientToQueue(client);
						}
						else
							client->CheckFailedFileIdReqs(packet);
					}
					else
						//Xman Anti-Leecher
						if(thePrefs.GetAntiLeecher())
						{
							ed2k_client->BanLeecher(_T("wrong OPSTARTUPLOADREQ"),16); 
							ed2k_client->ProcessBanMessage();
						}
						//Xman end
						break;
				}
			case OP_QUEUERANK:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_QueueRank", client);
					theStats.AddDownDataOverheadFileRequest(size);
					ed2k_client->ProcessEdonkeyQueueRank(packet, size);
					break;
				}
			case OP_ACCEPTUPLOADREQ:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
					{
						DebugRecv("OP_AcceptUploadReq", client, (size >= 16) ? packet : NULL);
						if (size > 0)
							Debug(_T("  ***NOTE: Packet contains %u additional bytes\n"), size);
						Debug(_T("  QR=%d\n"), client->IsRemoteQueueFull() ? (UINT)-1 : (UINT)client->GetRemoteQueueRank());
					}
					theStats.AddDownDataOverheadFileRequest(size);
					client->ProcessAcceptUpload();
					break;
				}
			case OP_REQUESTPARTS:
				{
					// see also OP_REQUESTPARTS_I64
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_RequestParts", client, (size >= 16) ? packet : NULL);
					theStats.AddDownDataOverheadFileRequest(size);

					CSafeMemFile data(packet, size);
					uchar reqfilehash[16];
					data.ReadHash16(reqfilehash);

					uint32 auStartOffsets[3];
					auStartOffsets[0] = data.ReadUInt32();
					auStartOffsets[1] = data.ReadUInt32();
					auStartOffsets[2] = data.ReadUInt32();

					uint32 auEndOffsets[3];
					auEndOffsets[0] = data.ReadUInt32();
					auEndOffsets[1] = data.ReadUInt32();
					auEndOffsets[2] = data.ReadUInt32();

					if (thePrefs.GetDebugClientTCPLevel() > 0)
					{
						Debug(_T("  Start1=%u  End1=%u  Size=%u\n"), auStartOffsets[0], auEndOffsets[0], auEndOffsets[0] - auStartOffsets[0]);
						Debug(_T("  Start2=%u  End2=%u  Size=%u\n"), auStartOffsets[1], auEndOffsets[1], auEndOffsets[1] - auStartOffsets[1]);
						Debug(_T("  Start3=%u  End3=%u  Size=%u\n"), auStartOffsets[2], auEndOffsets[2], auEndOffsets[2] - auStartOffsets[2]);
					}

					//Xman Anti-Leecher
					if(client->GetUploadState()==US_BANNED) //just to be sure
					{
						CGlobalVariable::uploadqueue->RemoveFromUploadQueue(client,_T("banned client detected during upload")); 
						client->SetUploadFileID(NULL); 
						AddLeecherLogLine(false,_T("banned client was in upload: %s"),client->DbgGetClientInfo());
					}
					//Xman end

					for (int i = 0; i < ARRSIZE(auStartOffsets); i++)
					{
						if (auEndOffsets[i] > auStartOffsets[i])
						{
							Requested_Block_Struct* reqblock = new Requested_Block_Struct;
							reqblock->StartOffset = auStartOffsets[i];
							reqblock->EndOffset = auEndOffsets[i];
							md4cpy(reqblock->FileID, reqfilehash);
							reqblock->transferred = 0;
							client->AddReqBlock(reqblock);
						}
						else
						{
							if (thePrefs.GetVerbose())
							{
								if (auEndOffsets[i] != 0 || auStartOffsets[i] != 0)
									DebugLogWarning(_T("Client requests invalid %u. file block %u-%u (%d bytes): %s"), i, auStartOffsets[i], auEndOffsets[i], auEndOffsets[i] - auStartOffsets[i], client->DbgGetClientInfo());
							}
						}
					}
					break;
				}
			case OP_CANCELTRANSFER:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_CancelTransfer", client);
					theStats.AddDownDataOverheadFileRequest(size);
					CGlobalVariable::uploadqueue->RemoveFromUploadQueue(client, _T("Remote client canceled transfer."));
					break;
				}
			case OP_END_OF_DOWNLOAD:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_EndOfDownload", client, (size >= 16) ? packet : NULL);
					theStats.AddDownDataOverheadFileRequest(size);
					if (size>=16 && !md4cmp(client->GetUploadFileID(),packet))
						CGlobalVariable::uploadqueue->RemoveFromUploadQueue(client, _T("Remote client ended transfer."));
					else
						client->CheckFailedFileIdReqs(packet);
					break;
				}
			case OP_HASHSETREQUEST:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_HashSetReq", client, (size >= 16) ? packet : NULL);
					theStats.AddDownDataOverheadFileRequest(size);

					if (size != 16)
						throw GetResString(IDS_ERR_WRONGHPACKAGESIZE);
					client->SendHashsetPacket(packet);
					break;
				}
			case OP_HASHSETANSWER:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_HashSetAnswer", client, (size >= 16) ? packet : NULL);
					theStats.AddDownDataOverheadFileRequest(size);
					client->ProcessHashSet(packet,size);
					break;
				}
			case OP_SENDINGPART:
				{
					// see also OP_SENDINGPART_I64
					if (thePrefs.GetDebugClientTCPLevel() > 1)
						DebugRecv("OP_SendingPart", client, (size >= 16) ? packet : NULL);
					theStats.AddDownDataOverheadFileRequest(24);
					if (client->GetRequestFile() && !client->GetRequestFile()->IsStopped() && (client->GetRequestFile()->GetStatus()==PS_READY || client->GetRequestFile()->GetStatus()==PS_EMPTY))
					{
						client->ProcessBlockPacket(packet, size, false, false);
						if (client->GetRequestFile()->IsStopped() || client->GetRequestFile()->GetStatus()==PS_PAUSED || client->GetRequestFile()->GetStatus()==PS_ERROR)
						{
							client->SendCancelTransfer();
							client->SetDownloadState(client->GetRequestFile()->IsStopped() ? DS_NONE : DS_ONQUEUE);
						}
					}
					else
					{
						client->SendCancelTransfer();
						client->SetDownloadState((client->GetRequestFile()==NULL || client->GetRequestFile()->IsStopped()) ? DS_NONE : DS_ONQUEUE);
					}
					break;
				}
			case OP_OUTOFPARTREQS:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_OutOfPartReqs", client);
					theStats.AddDownDataOverheadFileRequest(size);
					if (client->GetDownloadState() == DS_DOWNLOADING)
					{
						client->SetDownloadState(DS_ONQUEUE, _T("The remote client decided to stop/complete the transfer (got OP_OutOfPartReqs)."));
					}
					break;
				}
			case OP_CHANGE_CLIENT_ID:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_ChangedClientID", client);
					theStats.AddDownDataOverheadOther(size);

					CSafeMemFile data(packet, size);
					uint32 nNewUserID = data.ReadUInt32();
					uint32 nNewServerIP = data.ReadUInt32();
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						Debug(_T("  NewUserID=%u (%08x, %s)  NewServerIP=%u (%08x, %s)\n"), nNewUserID, nNewUserID, ipstr(nNewUserID), nNewServerIP, nNewServerIP, ipstr(nNewServerIP));
					if (IsLowID(nNewUserID))
					{	// client changed server and has a LowID
						CServer* pNewServer = CGlobalVariable::serverlist->GetServerByIP(nNewServerIP);
						if (pNewServer != NULL)
						{
							client->SetUserIDHybrid(nNewUserID); // update UserID only if we know the server
							client->SetServerIP(nNewServerIP);
							client->SetServerPort(pNewServer->GetPort());
						}
					}
					else if (nNewUserID == client->GetIP())
					{	// client changed server and has a HighID(IP)
						client->SetUserIDHybrid(ntohl(nNewUserID));
						CServer* pNewServer = CGlobalVariable::serverlist->GetServerByIP(nNewServerIP);
						if (pNewServer != NULL)
						{
							client->SetServerIP(nNewServerIP);
							client->SetServerPort(pNewServer->GetPort());
						}
					}
					else
					{
						if (thePrefs.GetDebugClientTCPLevel() > 0)
							Debug(_T("***NOTE: OP_ChangedClientID unknown contents\n"));
					}
					UINT uAddData = (UINT)(data.GetLength() - data.GetPosition());
					if (uAddData > 0)
					{
						if (thePrefs.GetDebugClientTCPLevel() > 0)
							Debug(_T("***NOTE: OP_ChangedClientID contains add. data %s\n"), DbgGetHexDump(packet + data.GetPosition(), uAddData));
					}
					break;
				}
			case OP_CHANGE_SLOT:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_ChangeSlot", client, (size >= 16) ? packet : NULL);
					theStats.AddDownDataOverheadFileRequest(size);

					// sometimes sent by Hybrid
					break;
				}
			case OP_MESSAGE:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_Message", client);
					theStats.AddDownDataOverheadOther(size);

					if (size < 2)
						throw CString(_T("invalid message packet"));
					CSafeMemFile data(packet, size);
					UINT length = data.ReadUInt16();
					if (length+2 != size)
						throw CString(_T("invalid message packet"));

					//filter me?
					if ( (thePrefs.MsgOnlyFriends() && !client->IsFriend()) || (thePrefs.MsgOnlySecure() && client->GetUserName()==NULL) )
					{
						if (!client->GetMessageFiltered())
						{
							if (thePrefs.GetVerbose())
								AddDebugLogLine(false,_T("Filtered Message from '%s' (IP:%s)"), client->GetUserName(), ipstr(client->GetConnectIP()));
						}
						client->SetMessageFiltered(true);
						break;
					}

					if (length > MAX_CLIENT_MSG_LEN)
					{
						if (thePrefs.GetVerbose())
							AddDebugLogLine(false, _T("Message from '%s' (IP:%s) exceeds limit by %u chars, truncated."), client->GetUserName(), ipstr(client->GetConnectIP()), length - MAX_CLIENT_MSG_LEN);
						length = MAX_CLIENT_MSG_LEN;
					}

					CString strMessage(data.ReadString(client->GetUnicodeSupport()!=utf8strNone, length));
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						Debug(_T("  %s\n"), strMessage);

					//  Comment UI
					//theApp.emuledlg->chatwnd->chatselector.ProcessMessage(client, strMessage);
					break;
				}
			case OP_ASKSHAREDFILES:
				{
					// client wants to know what we have in share, let's see if we allow him to know that
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_AskSharedFiles", client);
					theStats.AddDownDataOverheadOther(size);


					CPtrList list;
					if (thePrefs.CanSeeShares()==vsfaEverybody || (thePrefs.CanSeeShares()==vsfaFriends && client->IsFriend()))
					{
						CCKey bufKey;
						CKnownFile* cur_file;
						for (POSITION pos = CGlobalVariable::sharedfiles->m_Files_map.GetStartPosition();pos != 0;)
						{
							CGlobalVariable::sharedfiles->m_Files_map.GetNextAssoc(pos,bufKey,cur_file);
							if (!cur_file->IsLargeFile() || client->SupportsLargeFiles())
								list.AddTail((void*&)cur_file);
						}
//						AddLogLine(true, GetResString(IDS_REQ_SHAREDFILES), client->GetUserName(), client->GetUserIDHybrid(), GetResString(IDS_ACCEPTED));
					}
					else
					{
//						AddLogLine(true, GetResString(IDS_REQ_SHAREDFILES), client->GetUserName(), client->GetUserIDHybrid(), GetResString(IDS_DENIED));
					}

					// now create the memfile for the packet
					uint32 iTotalCount = list.GetCount();
					CSafeMemFile tempfile(80);
					tempfile.WriteUInt32(iTotalCount);
					while (list.GetCount())
					{
						CGlobalVariable::sharedfiles->CreateOfferedFilePacket((CKnownFile*)list.GetHead(), &tempfile, NULL, client);
						list.RemoveHead();
					}

					// create a packet and send it
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugSend("OP__AskSharedFilesAnswer", client);
					Packet* replypacket = new Packet(&tempfile);
					replypacket->opcode = OP_ASKSHAREDFILESANSWER;
					theStats.AddUpDataOverheadOther(replypacket->size);
					SendPacket(replypacket, true, true);
					break;
				}
			case OP_ASKSHAREDFILESANSWER:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_AskSharedFilesAnswer", client);
					theStats.AddDownDataOverheadOther(size);
					client->ProcessSharedFileList(packet,size);
					break;
				}
			case OP_ASKSHAREDDIRS:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_AskSharedDirectories", client);
					theStats.AddDownDataOverheadOther(size);

					if (thePrefs.CanSeeShares()==vsfaEverybody || (thePrefs.CanSeeShares()==vsfaFriends && client->IsFriend()))
					{
//						AddLogLine(true, GetResString(IDS_SHAREDREQ1), client->GetUserName(), client->GetUserIDHybrid(), ipstr(client->GetIP()), GetResString(IDS_ACCEPTED));

						//TODO: Don't send shared directories which do not contain any files
						// add shared directories
						CString strDir;
						CStringArray arFolders;
						POSITION pos = thePrefs.shareddir_list.GetHeadPosition();
						while (pos)
						{
							strDir = thePrefs.shareddir_list.GetNext(pos);
							PathRemoveBackslash(strDir.GetBuffer());
							strDir.ReleaseBuffer();
							bool bFoundFolder = false;
							for (int i = 0; i < arFolders.GetCount(); i++)
							{
								if (strDir.CompareNoCase(arFolders.GetAt(i)) == 0)
								{
									bFoundFolder = true;
									break;
								}
							}
							if (!bFoundFolder)
								arFolders.Add(strDir);
						}

						// add incoming folders
						for (int iCat = 0; iCat < thePrefs.GetCatCount(); iCat++)
						{
							strDir = thePrefs.GetCategory(iCat)->strIncomingPath;
							PathRemoveBackslash(strDir.GetBuffer());
							strDir.ReleaseBuffer();
							bool bFoundFolder = false;
							for (int i = 0; i < arFolders.GetCount(); i++)
							{
								if (strDir.CompareNoCase(arFolders.GetAt(i)) == 0)
								{
									bFoundFolder = true;
									break;
								}
							}
							if (!bFoundFolder)
								arFolders.Add(strDir);
						}

						// add temporary folder
						strDir = OP_INCOMPLETE_SHARED_FILES;
						bool bFoundFolder = false;
						for (int i = 0; i < arFolders.GetCount(); i++)
						{
							if (strDir.CompareNoCase(arFolders.GetAt(i)) == 0)
							{
								bFoundFolder = true;
								break;
							}
						}
						if (!bFoundFolder)
							arFolders.Add(strDir);

						// build packet
						CSafeMemFile tempfile(80);
						tempfile.WriteUInt32(arFolders.GetCount());
						for (int i = 0; i < arFolders.GetCount(); i++)
							tempfile.WriteString(arFolders.GetAt(i), client->GetUnicodeSupport());

						if (thePrefs.GetDebugClientTCPLevel() > 0)
							DebugSend("OP__AskSharedDirsAnswer", client);
						Packet* replypacket = new Packet(&tempfile);
						replypacket->opcode = OP_ASKSHAREDDIRSANS;
						theStats.AddUpDataOverheadOther(replypacket->size);
						SendPacket(replypacket, true, true);
					}
					else
					{
//						AddLogLine(true, GetResString(IDS_SHAREDREQ1), client->GetUserName(), client->GetUserIDHybrid(), ipstr(client->GetIP()), GetResString(IDS_DENIED));

						if (thePrefs.GetDebugClientTCPLevel() > 0)
							DebugSend("OP__AskSharedDeniedAnswer", client);
						Packet* replypacket = new Packet(OP_ASKSHAREDDENIEDANS, 0);
						theStats.AddUpDataOverheadOther(replypacket->size);
						SendPacket(replypacket, true, true);
					}
					break;
				}
			case OP_ASKSHAREDFILESDIR:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_AskSharedFilesInDirectory", client);
					theStats.AddDownDataOverheadOther(size);

					CSafeMemFile data(packet, size);
					CString strReqDir = data.ReadString(client->GetUnicodeSupport()!=utf8strNone);
					PathRemoveBackslash(strReqDir.GetBuffer());
					strReqDir.ReleaseBuffer();
					if (thePrefs.CanSeeShares()==vsfaEverybody || (thePrefs.CanSeeShares()==vsfaFriends && client->IsFriend()))
					{
//						AddLogLine(true, GetResString(IDS_SHAREDREQ2), client->GetUserName(), client->GetUserIDHybrid(), strReqDir, GetResString(IDS_ACCEPTED));
						ASSERT( data.GetPosition() == data.GetLength() );
						CTypedPtrList<CPtrList, CKnownFile*> list;
						if (strReqDir == OP_INCOMPLETE_SHARED_FILES)
						{
							// get all shared files from download queue
							int iQueuedFiles = CGlobalVariable::downloadqueue->GetFileCount();
							for (int i = 0; i < iQueuedFiles; i++)
							{
								CPartFile* pFile = CGlobalVariable::downloadqueue->GetFileByIndex(i);
								if (pFile == NULL || pFile->GetStatus(true) != PS_READY || (pFile->IsLargeFile() && !client->SupportsLargeFiles()))
									continue;
								list.AddTail(pFile);
							}
						}
						else
						{
							// get all shared files from requested directory
							for (POSITION pos = CGlobalVariable::sharedfiles->m_Files_map.GetStartPosition();pos != 0;)
							{
								CCKey bufKey;
								CKnownFile* cur_file;
								CGlobalVariable::sharedfiles->m_Files_map.GetNextAssoc(pos, bufKey, cur_file);
								CString strSharedFileDir(cur_file->GetPath());
								PathRemoveBackslash(strSharedFileDir.GetBuffer());
								strSharedFileDir.ReleaseBuffer();
								if (strReqDir.CompareNoCase(strSharedFileDir) == 0 && (!cur_file->IsLargeFile() || client->SupportsLargeFiles()))
									list.AddTail(cur_file);
							}
						}

						// Currently we are sending each shared directory, even if it does not contain any files.
						// Because of this we also have to send an empty shared files list..
						CSafeMemFile tempfile(80);
						tempfile.WriteString(strReqDir, client->GetUnicodeSupport());
						tempfile.WriteUInt32(list.GetCount());
						while (list.GetCount())
						{
							CGlobalVariable::sharedfiles->CreateOfferedFilePacket(list.GetHead(), &tempfile, NULL, client);
							list.RemoveHead();
						}

						if (thePrefs.GetDebugClientTCPLevel() > 0)
							DebugSend("OP__AskSharedFilesInDirectoryAnswer", client);
						Packet* replypacket = new Packet(&tempfile);
						replypacket->opcode = OP_ASKSHAREDFILESDIRANS;
						theStats.AddUpDataOverheadOther(replypacket->size);
						SendPacket(replypacket, true, true);
					}
					else
					{
//						AddLogLine(true, GetResString(IDS_SHAREDREQ2), client->GetUserName(), client->GetUserIDHybrid(), strReqDir, GetResString(IDS_DENIED));
						if (thePrefs.GetDebugClientTCPLevel() > 0)
							DebugSend("OP__AskSharedDeniedAnswer", client);
						Packet* replypacket = new Packet(OP_ASKSHAREDDENIEDANS, 0);
						theStats.AddUpDataOverheadOther(replypacket->size);
						SendPacket(replypacket, true, true);
					}
					break;
				}
			case OP_ASKSHAREDDIRSANS:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_AskSharedDirectoriesAnswer", client);
					theStats.AddDownDataOverheadOther(size);
					if (client->GetFileListRequested() == 1)
					{
						CSafeMemFile data(packet, size);
						UINT uDirs = data.ReadUInt32();
						for (UINT i = 0; i < uDirs; i++)
						{
							CString strDir = data.ReadString(client->GetUnicodeSupport()!=utf8strNone);
							// Better send the received and untouched directory string back to that client
							//PathRemoveBackslash(strDir.GetBuffer());
							//strDir.ReleaseBuffer();
							AddLogLine(true, GetResString(IDS_SHAREDANSW), client->GetUserName(), client->GetUserIDHybrid(), strDir);

							if (thePrefs.GetDebugClientTCPLevel() > 0)
								DebugSend("OP__AskSharedFilesInDirectory", client);
							CSafeMemFile tempfile(80);
							tempfile.WriteString(strDir, client->GetUnicodeSupport());
							Packet* replypacket = new Packet(&tempfile);
							replypacket->opcode = OP_ASKSHAREDFILESDIR;
							theStats.AddUpDataOverheadOther(replypacket->size);
							SendPacket(replypacket, true, true);
						}
						ASSERT( data.GetPosition() == data.GetLength() );
						client->SetFileListRequested(uDirs);
					}
					else
						AddLogLine(true, GetResString(IDS_SHAREDANSW2), client->GetUserName(), client->GetUserIDHybrid());
					break;
				}
			case OP_ASKSHAREDFILESDIRANS:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_AskSharedFilesInDirectoryAnswer", client);
					theStats.AddDownDataOverheadOther(size);

					CSafeMemFile data(packet, size);
					CString strDir = data.ReadString(client->GetUnicodeSupport()!=utf8strNone);
					PathRemoveBackslash(strDir.GetBuffer());
					strDir.ReleaseBuffer();
					if (client->GetFileListRequested() > 0)
					{
						AddLogLine(true, GetResString(IDS_SHAREDINFO1), client->GetUserName(), client->GetUserIDHybrid(), strDir);
						client->ProcessSharedFileList(packet + (UINT)data.GetPosition(), (UINT)(size - data.GetPosition()), strDir);
						if (client->GetFileListRequested() == 0)
							AddLogLine(true, GetResString(IDS_SHAREDINFO2), client->GetUserName(), client->GetUserIDHybrid());
					}
					else
						AddLogLine(true, GetResString(IDS_SHAREDANSW3), client->GetUserName(), client->GetUserIDHybrid(), strDir);
					break;
				}
			case OP_ASKSHAREDDENIEDANS:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_AskSharedDeniedAnswer", client);
					theStats.AddDownDataOverheadOther(size);

					AddLogLine(true, GetResString(IDS_SHAREDREQDENIED), client->GetUserName(), client->GetUserIDHybrid());
					client->SetFileListRequested(0);
					break;
				}
			default:
				theStats.AddDownDataOverheadOther(size);
				PacketToDebugLogLine(_T("eDonkey"), packet, size, opcode);
				break;
			}
		}
		catch (CFileException* error)
		{
			error->Delete();
			throw GetResString(IDS_ERR_INVALIDPACKAGE);
		}
		catch (CMemoryException* error)
		{
			error->Delete();
			throw CString(_T("Memory exception"));
		}
	}
	catch (CClientException* ex) // nearly same as the 'CString' exception but with optional deleting of the client
	{
		if (thePrefs.GetVerbose() && !ex->m_strMsg.IsEmpty())
			DebugLogWarning(_T("Error: %s - while processing eDonkey packet: opcode=%s  size=%u; %s"), ex->m_strMsg, DbgGetDonkeyClientTCPOpcode(opcode), size, DbgGetClientInfo());
		if (client && ex->m_bDelete)
			client->SetDownloadState(DS_ERROR, _T("Error while processing eDonkey packet (CClientException): ") + ex->m_strMsg);
		Disconnect(ex->m_strMsg);
		ex->Delete();
		return false;
	}
	catch (CString error)
	{
		if (thePrefs.GetVerbose() && !error.IsEmpty())
		{
			if (opcode == OP_REQUESTFILENAME /*low priority for OP_REQUESTFILENAME*/)
				DebugLogWarning(_T("Error: %s - while processing eDonkey packet: opcode=%s  size=%u; %s"), error, DbgGetDonkeyClientTCPOpcode(opcode), size, DbgGetClientInfo());
			else
				DebugLogWarning(_T("Error: %s - while processing eDonkey packet: opcode=%s  size=%u; %s"), error, DbgGetDonkeyClientTCPOpcode(opcode), size, DbgGetClientInfo());
		}
		if (client)
			client->SetDownloadState(DS_ERROR, _T("Error while processing eDonkey packet (CString exception): ") + error);
		Disconnect(_T("Error when processing packet.") + error);
		return false;
	}
	return true;
}

bool CClientReqSocket::ProcessExtPacket(const BYTE* packet, uint32 size, UINT opcode, UINT uRawSize)
{
	try
	{
		try
		{
			if (!client && opcode!=OP_PORTTEST)
			{
				theStats.AddDownDataOverheadOther(uRawSize);
				throw GetResString(IDS_ERR_UNKNOWNCLIENTACTION);
			}
			if (thePrefs.m_iDbgHeap >= 2 && opcode!=OP_PORTTEST)
				ASSERT_VALID(client);

			// VC-SearchDream[2007-04-18]: For SourceExchange NAT
			if (!CSourceExchangeNAT::ProcessTCPPacket(packet, size, opcode, uRawSize, this))
			{
				return true;
			}

			CEd2kUpDownClient * ed2k_client = STATIC_DOWNCAST( CEd2kUpDownClient , client );
			if( client && 0 == ed2k_client )
				throw "error";

			switch(opcode)
			{
			case OP_MULTIPACKET:
			case OP_MULTIPACKET_EXT:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
					{
						if (opcode == OP_MULTIPACKET_EXT)
							DebugRecv("OP_MultiPacket_Ext", client, (size >= 24) ? packet : NULL);
						else
							DebugRecv("OP_MultiPacket", client, (size >= 16) ? packet : NULL);
					}
					theStats.AddDownDataOverheadFileRequest(uRawSize);
					client->CheckHandshakeFinished();

					if ( client->GetKadPort() )
						Kademlia::CKademlia::Bootstrap(ntohl(client->GetIP()), client->GetKadPort(), (client->GetKadVersion() > 1));

					CSafeMemFile data_in(packet, size);
					uchar reqfilehash[16];
					data_in.ReadHash16(reqfilehash);
					uint64 nSize = 0;
					if (opcode == OP_MULTIPACKET_EXT)
					{
						nSize = data_in.ReadUInt64();
					}
					CKnownFile* reqfile;
					if ( (reqfile = CGlobalVariable::sharedfiles->GetFileByID(reqfilehash)) == NULL )
					{
						if ( !((reqfile = CGlobalVariable::downloadqueue->GetFileByID(reqfilehash)) != NULL
							&& reqfile->GetFileSize() > (uint64)PARTSIZE) )
						{
							// send file request no such file packet (0x48)
							if (thePrefs.GetDebugClientTCPLevel() > 0)
								DebugSend("OP__FileReqAnsNoFil", client, packet);
							Packet* replypacket = new Packet(OP_FILEREQANSNOFIL, 16);
							md4cpy(replypacket->pBuffer, packet);
							theStats.AddUpDataOverheadFileRequest(replypacket->size);
							SendPacket(replypacket, true);
							client->CheckFailedFileIdReqs(reqfilehash);
							break;
						}
					}
					if (reqfile->IsLargeFile() && !client->SupportsLargeFiles())
					{
						if (thePrefs.GetDebugClientTCPLevel() > 0)
							DebugSend("OP__FileReqAnsNoFil", client, packet);
						Packet* replypacket = new Packet(OP_FILEREQANSNOFIL, 16);
						md4cpy(replypacket->pBuffer, packet);
						theStats.AddUpDataOverheadFileRequest(replypacket->size);
						SendPacket(replypacket, true);
						DebugLogWarning(_T("Client without 64bit file support requested large file; %s, File=\"%s\""), client->DbgGetClientInfo(), reqfile->GetFileName());
						break;
					}
					if (nSize != 0 && nSize != reqfile->GetFileSize())
					{
						if (thePrefs.GetDebugClientTCPLevel() > 0)
							DebugSend("OP__FileReqAnsNoFil", client, packet);
						Packet* replypacket = new Packet(OP_FILEREQANSNOFIL, 16);
						md4cpy(replypacket->pBuffer, packet);
						theStats.AddUpDataOverheadFileRequest(replypacket->size);
						SendPacket(replypacket, true);
						DebugLogWarning(_T("Size Mismatch on requested file, sending FNF; %s, File=\"%s\""), client->DbgGetClientInfo(), reqfile->GetFileName());
						break;
					}

					if (!client) break;
					if (!client->GetWaitStartTime())
						client->SetWaitStartTime();

					// if we are downloading this file, this could be a new source
					// no passive adding of files with only one part
					if (reqfile->IsPartFile() && reqfile->GetFileSize() > (uint64)PARTSIZE)
					{
						if (((CPartFile*)reqfile)->GetMaxSources() > ((CPartFile*)reqfile)->GetSourceCount())
							CGlobalVariable::downloadqueue->CheckAndAddKnownSource((CPartFile*)reqfile, client, true);
					}

					// check to see if this is a new file they are asking for
					if (md4cmp(client->GetUploadFileID(), reqfilehash) != 0)
						ed2k_client->SetCommentDirty();

					client->SetUploadFileID(reqfile);

					uint8 opcode_in;
					CSafeMemFile data_out(128);
					data_out.WriteHash16(reqfile->GetFileHash());
					bool bAnswerFNF = false;
					while (data_in.GetLength()-data_in.GetPosition() && !bAnswerFNF)
					{
						opcode_in = data_in.ReadUInt8();
						switch (opcode_in)
						{
						case OP_REQUESTFILENAME:
							{
								if (thePrefs.GetDebugClientTCPLevel() > 0)
									DebugRecv("OP_MPReqFileName", client, packet);

								if (!client->ProcessExtendedInfo(&data_in, reqfile))
								{
									if (thePrefs.GetDebugClientTCPLevel() > 0)
										DebugSend("OP__FileReqAnsNoFil", client, packet);
									Packet* replypacket = new Packet(OP_FILEREQANSNOFIL, 16);
									md4cpy(replypacket->pBuffer, reqfile->GetFileHash());
									theStats.AddUpDataOverheadFileRequest(replypacket->size);
									SendPacket(replypacket, true);
									DebugLogWarning(_T("Partcount mismatch on requested file, sending FNF; %s, File=\"%s\""), client->DbgGetClientInfo(), reqfile->GetFileName());
									bAnswerFNF = true;
									break;
								}
								data_out.WriteUInt8(OP_REQFILENAMEANSWER);
								data_out.WriteString(reqfile->GetFileName(), client->GetUnicodeSupport());
								break;
							}
						case OP_AICHFILEHASHREQ:
							{
								if (thePrefs.GetDebugClientTCPLevel() > 0)
									DebugRecv("OP_MPAichFileHashReq", client, packet);

								if (client->IsSupportingAICH() && reqfile->GetAICHHashset()->GetStatus() == AICH_HASHSETCOMPLETE
									&& reqfile->GetAICHHashset()->HasValidMasterHash())
								{
									data_out.WriteUInt8(OP_AICHFILEHASHANS);
									reqfile->GetAICHHashset()->GetMasterHash().Write(&data_out);
								}
								break;
							}
						case OP_SETREQFILEID:
							{
								if (thePrefs.GetDebugClientTCPLevel() > 0)
									DebugRecv("OP_MPSetReqFileID", client, packet);

								//Xman FileFaker detection
								if(reqfile->IsPartFile() && ((CPartFile*)reqfile)->m_DeadSourceList.IsDeadSource(client))
								{
									ed2k_client->BanLeecher(_T("FileFaker"),19);
									ed2k_client->ProcessBanMessage();
									bAnswerFNF=true; //will skip to answer
									break;
								}

								data_out.WriteUInt8(OP_FILESTATUS);
								if (reqfile->IsPartFile())
									((CPartFile*)reqfile)->WritePartStatus(&data_out);
								else
									data_out.WriteUInt16(0);
								break;
							}
							//We still send the source packet seperately..
							case OP_REQUESTSOURCES2:
						case OP_REQUESTSOURCES:
							{
								if (thePrefs.GetDebugClientTCPLevel() > 0)
									DebugRecv(opcode_in == OP_REQUESTSOURCES2 ? "OP_MPReqSources2" : "OP_MPReqSources", client, packet);

								if (thePrefs.GetDebugSourceExchange())
									AddDebugLogLine(false, _T("SXRecv: Client source request; %s, File=\"%s\""), client->DbgGetClientInfo(), reqfile->GetFileName());

								//Xman Anti-Leecher
								//>>> Anti-XS-Exploit (Xman)
								if(thePrefs.GetAntiLeecherXSExploiter() && ed2k_client->IsXSExploiter())
									break; //no answer
								//Xman end

								uint8 byRequestedVersion = 0;
								uint16 byRequestedOptions = 0;
								if (opcode_in == OP_REQUESTSOURCES2){ // SX2 requests contains additional data
									byRequestedVersion = data_in.ReadUInt8();
									byRequestedOptions = data_in.ReadUInt16();
								}
								//Although this shouldn't happen, it's a just in case to any Mods that mess with version numbers.
								if (byRequestedVersion > 0 || client->GetSourceExchange1Version() > 1)
								{
									DWORD dwTimePassed = ::GetTickCount() - client->GetLastSrcReqTime() + CONNECTION_LATENCY;
									bool bNeverAskedBefore = client->GetLastSrcReqTime() == 0;
									if (
										//if not complete and file is rare
										(    reqfile->IsPartFile()
										&& (bNeverAskedBefore || dwTimePassed > SOURCECLIENTREASKS)
										&& ((CPartFile*)reqfile)->GetSourceCount() <= RARE_FILE
										) ||
										//OR if file is not rare or if file is complete
										(bNeverAskedBefore || dwTimePassed > SOURCECLIENTREASKS * MINCOMMONPENALTY)
										)
									{
										client->SetLastSrcReqTime();
										Packet* tosend = reqfile->CreateSrcInfoPacket(client, byRequestedVersion, byRequestedOptions);
										if (tosend)
										{
											if (thePrefs.GetDebugClientTCPLevel() > 0)
												DebugSend("OP__AnswerSources", client, reqfile->GetFileHash());
											theStats.AddUpDataOverheadSourceExchange(tosend->size);
											SendPacket(tosend, true);
										}
									}
									/*else
									{
									if (thePrefs.GetVerbose())
									AddDebugLogLine(false, _T("RCV: Source Request to fast. (This is testing the new timers to see how much older client will not receive this)"));
									}*/
								}
								break;
							}
						default:
							{
								CString strError;
								strError.Format(_T("Invalid sub opcode 0x%02x received"), opcode_in);
								throw strError;
							}
						}
					}
					if (data_out.GetLength() > 16 && !bAnswerFNF)
					{
						if (thePrefs.GetDebugClientTCPLevel() > 0)
							DebugSend("OP__MultiPacketAns", client, reqfile->GetFileHash());
						Packet* reply = new Packet(&data_out, OP_EMULEPROT);
						reply->opcode = OP_MULTIPACKETANSWER;
						theStats.AddUpDataOverheadFileRequest(reply->size);
						SendPacket(reply, true);
					}
					break;
				}
			case OP_MULTIPACKETANSWER:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_MultiPacketAns", client, (size >= 16) ? packet : NULL);
					theStats.AddDownDataOverheadFileRequest(uRawSize);
					client->CheckHandshakeFinished();

					if ( client->GetKadPort() )
						Kademlia::CKademlia::Bootstrap(ntohl(client->GetIP()), client->GetKadPort(), (client->GetKadVersion() > 1));

					CSafeMemFile data_in(packet, size);
					uchar reqfilehash[16];
					data_in.ReadHash16(reqfilehash);
					CPartFile* reqfile = CGlobalVariable::downloadqueue->GetFileByID(reqfilehash);
					//Make sure we are downloading this file.
					if (reqfile==NULL){
						client->CheckFailedFileIdReqs(reqfilehash);
						throw GetResString(IDS_ERR_WRONGFILEID) + _T(" (OP_MULTIPACKETANSWER; reqfile==NULL)");
					}
					if (client->GetRequestFile()==NULL)
						throw GetResString(IDS_ERR_WRONGFILEID) + _T(" (OP_MULTIPACKETANSWER; client->GetRequestFile()==NULL)");
					if (reqfile != client->GetRequestFile())
						throw GetResString(IDS_ERR_WRONGFILEID) + _T(" (OP_MULTIPACKETANSWER; reqfile!=client->GetRequestFile())");
					uint8 opcode_in;
					while (data_in.GetLength()-data_in.GetPosition())
					{
						opcode_in = data_in.ReadUInt8();
						switch (opcode_in)
						{
						case OP_REQFILENAMEANSWER:
							{
								if (thePrefs.GetDebugClientTCPLevel() > 0)
									DebugRecv("OP_MPReqFileNameAns", client, packet);

								client->ProcessFileInfo(&data_in, reqfile);
								break;
							}
						case OP_FILESTATUS:
							{
								if (thePrefs.GetDebugClientTCPLevel() > 0)
									DebugRecv("OP_MPFileStatus", client, packet);

								client->ProcessFileStatus(false, &data_in, reqfile);
								break;
							}
						case OP_AICHFILEHASHANS:
							{
								if (thePrefs.GetDebugClientTCPLevel() > 0)
									DebugRecv("OP_MPAichFileHashAns", client);

								client->ProcessAICHFileHash(&data_in, reqfile);
								break;
							}
						default:
							{
								CString strError;
								strError.Format(_T("Invalid sub opcode 0x%02x received"), opcode_in);
								throw strError;
							}
						}
					}
					break;
				}
			case OP_EMULEINFO:
				{
					theStats.AddDownDataOverheadOther(uRawSize);
					ed2k_client->ProcessMuleInfoPacket(packet,size);
					ed2k_client->ProcessBanMessage(); //Xman Anti-Leecher
					if (thePrefs.GetDebugClientTCPLevel() > 0){
						DebugRecv("OP_EmuleInfo", client);
						Debug(_T("  %s\n"), client->DbgGetMuleInfo());
					}

					// start secure identification, if
					//  - we have received eD2K and eMule info (old eMule)
					if (ed2k_client->GetInfoPacketsReceived() == IP_BOTH)
						ed2k_client->InfoPacketsReceived();

					ed2k_client->SendMuleInfoPacket(true);
					break;
				}
			case OP_EMULEINFOANSWER:
				{
					theStats.AddDownDataOverheadOther(uRawSize);
					ed2k_client->ProcessMuleInfoPacket(packet,size);
					ed2k_client->ProcessBanMessage(); //Xman Anti-Leecher
					if (thePrefs.GetDebugClientTCPLevel() > 0){
						DebugRecv("OP_EmuleInfoAnswer", client);
						Debug(_T("  %s\n"), client->DbgGetMuleInfo());
					}

					// start secure identification, if
					//  - we have received eD2K and eMule info (old eMule)
					if (ed2k_client->GetInfoPacketsReceived() == IP_BOTH)
						ed2k_client->InfoPacketsReceived();
					break;
				}
			case OP_SECIDENTSTATE:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_SecIdentState", client);
					theStats.AddDownDataOverheadOther(uRawSize);

					ed2k_client->ProcessSecIdentStatePacket(packet, size);
					if (ed2k_client->GetSecureIdentState() == IS_SIGNATURENEEDED)
						ed2k_client->SendSignaturePacket();
					else if (ed2k_client->GetSecureIdentState() == IS_KEYANDSIGNEEDED)
					{
						ed2k_client->SendPublicKeyPacket();
						ed2k_client->SendSignaturePacket();
					}
					break;
				}
			case OP_PUBLICKEY:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_PublicKey", client);
					theStats.AddDownDataOverheadOther(uRawSize);

					ed2k_client->ProcessPublicKeyPacket(packet, size);
					break;
				}
			case OP_SIGNATURE:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_Signature", client);
					theStats.AddDownDataOverheadOther(uRawSize);

					ed2k_client->ProcessSignaturePacket(packet, size);
					break;
				}
			case OP_QUEUERANKING:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_QueueRanking", client);
					theStats.AddDownDataOverheadFileRequest(uRawSize);
					client->CheckHandshakeFinished();

					ed2k_client->ProcessEmuleQueueRank(packet, size);
					break;
				}
			case OP_REQUESTSOURCES:
			case OP_REQUESTSOURCES2:
				{
					CSafeMemFile data(packet, size);
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv(opcode == OP_REQUESTSOURCES2 ? "OP_MPReqSources2" : "OP_MPReqSources", client, (size >= 16) ? packet : NULL);
					
					theStats.AddDownDataOverheadSourceExchange(uRawSize);
					client->CheckHandshakeFinished();

					uint8 byRequestedVersion = 0;
					uint16 byRequestedOptions = 0;
					if (opcode == OP_REQUESTSOURCES2){ // SX2 requests contains additional data
						byRequestedVersion = data.ReadUInt8();
						byRequestedOptions = data.ReadUInt16();
					}
					//Although this shouldn't happen, it's a just in case to any Mods that mess with version numbers.
					if (byRequestedVersion > 0 || client->GetSourceExchange1Version() > 1)
					{
						if (size < 16)
							throw GetResString(IDS_ERR_BADSIZE);

						if (thePrefs.GetDebugSourceExchange())
							AddDebugLogLine(false, _T("SXRecv: Client source request; %s, %s"), client->DbgGetClientInfo(), DbgGetFileInfo(packet));

						//first check shared file list, then download list
						uchar ucHash[16];
						data.ReadHash16(ucHash);
						CKnownFile* reqfile;
						if ((reqfile = CGlobalVariable::sharedfiles->GetFileByID(ucHash)) != NULL ||
							(reqfile = CGlobalVariable::downloadqueue->GetFileByID(ucHash)) != NULL)
						{
							// There are some clients which do not follow the correct protocol procedure of sending
							// the sequence OP_REQUESTFILENAME, OP_SETREQFILEID, OP_REQUESTSOURCES. If those clients
							// are doing this, they will not get the optimal set of sources which we could offer if
							// the would follow the above noted protocol sequence. They better to it the right way
							// or they will get just a random set of sources because we do not know their download
							// part status which may get cleared with the call of 'SetUploadFileID'.
							client->SetUploadFileID(reqfile);

							DWORD dwTimePassed = ::GetTickCount() - client->GetLastSrcReqTime() + CONNECTION_LATENCY;
							bool bNeverAskedBefore = client->GetLastSrcReqTime() == 0;
							if (
								//if not complete and file is rare
								(    reqfile->IsPartFile()
								&& (bNeverAskedBefore || dwTimePassed > SOURCECLIENTREASKS)
								&& ((CPartFile*)reqfile)->GetSourceCount() <= RARE_FILE
								) ||
								//OR if file is not rare or if file is complete
								(bNeverAskedBefore || dwTimePassed > SOURCECLIENTREASKS * MINCOMMONPENALTY)
								)
							{
								client->SetLastSrcReqTime();
								Packet* tosend = reqfile->CreateSrcInfoPacket(client, byRequestedVersion, byRequestedOptions);
								if (tosend)
								{
									if (thePrefs.GetDebugClientTCPLevel() > 0)
										DebugSend("OP__AnswerSources", client, reqfile->GetFileHash());
									theStats.AddUpDataOverheadSourceExchange(tosend->size);
									SendPacket(tosend, true, true);
								}
							}
						}
						else
							client->CheckFailedFileIdReqs(ucHash);
					}
					break;
				}
			case OP_ANSWERSOURCES:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_AnswerSources", client, (size >= 16) ? packet : NULL);
					theStats.AddDownDataOverheadSourceExchange(uRawSize);
					client->CheckHandshakeFinished();

					CSafeMemFile data(packet, size);
					uchar hash[16];
					data.ReadHash16(hash);
					CKnownFile* file = CGlobalVariable::downloadqueue->GetFileByID(hash);
					if (file){
						if (file->IsPartFile()){
							//set the client's answer time
							client->SetLastSrcAnswerTime();
							//and set the file's last answer time
							((CPartFile*)file)->SetLastAnsweredTime();
							((CPartFile*)file)->AddClientSources(&data, client->GetSourceExchange1Version(), false, client);
						}
					}
					else
						client->CheckFailedFileIdReqs(hash);
					break;
				}
 				case OP_ANSWERSOURCES2:
						{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_AnswerSources2", client, (size >= 17) ? packet : NULL);
					theStats.AddDownDataOverheadSourceExchange(uRawSize);
					client->CheckHandshakeFinished();

					CSafeMemFile data(packet, size);
					uint8 byVersion = data.ReadUInt8();
					uchar hash[16];
					data.ReadHash16(hash);
					CKnownFile* file = CGlobalVariable::downloadqueue->GetFileByID(hash);
					if (file){
						if (file->IsPartFile()){
							//set the client's answer time
							client->SetLastSrcAnswerTime();
							//and set the file's last answer time
							((CPartFile*)file)->SetLastAnsweredTime();
							((CPartFile*)file)->AddClientSources(&data, byVersion, true, client);
						}
					}
					else
						client->CheckFailedFileIdReqs(hash);
					break;
				}
			case OP_FILEDESC:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_FileDesc", client);
					theStats.AddDownDataOverheadFileRequest(uRawSize);
					client->CheckHandshakeFinished();

					ed2k_client->ProcessMuleCommentPacket(packet,size);
					break;
				}
			case OP_REQUESTPREVIEW:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_RequestPreView", client, (size >= 16) ? packet : NULL);
					theStats.AddDownDataOverheadOther(uRawSize);
					client->CheckHandshakeFinished();

					if (thePrefs.CanSeeShares()==vsfaEverybody || (thePrefs.CanSeeShares()==vsfaFriends && client->IsFriend()))
					{
						ed2k_client->ProcessPreviewReq(packet,size);
						if (thePrefs.GetVerbose())
							AddDebugLogLine(true,_T("Client '%s' (%s) requested Preview - accepted"), client->GetUserName(), ipstr(client->GetConnectIP()));
					}
					else
					{
						// we don't send any answer here, because the client should know that he was not allowed to ask
						if (thePrefs.GetVerbose())
							AddDebugLogLine(true,_T("Client '%s' (%s) requested Preview - denied"), client->GetUserName(), ipstr(client->GetConnectIP()));
					}
					break;
				}
			case OP_PREVIEWANSWER:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_PreviewAnswer", client, (size >= 16) ? packet : NULL);
					theStats.AddDownDataOverheadOther(uRawSize);
					client->CheckHandshakeFinished();

					ed2k_client->ProcessPreviewAnswer(packet, size);
					break;
				}
			case OP_PEERCACHE_QUERY:
				{
					theStats.AddDownDataOverheadFileRequest(uRawSize);
					if (!client->ProcessPeerCacheQuery(packet, size))
					{
						CSafeMemFile dataSend(128);
						dataSend.WriteUInt8(PCPCK_VERSION);
						dataSend.WriteUInt8(PCOP_NONE);
						if (thePrefs.GetDebugClientTCPLevel() > 0){
							DebugSend("OP__PeerCacheAnswer", client);
							Debug(_T("  %s\n"), _T("Not supported"));
						}
						Packet* pEd2kPacket = new Packet(&dataSend, OP_EMULEPROT, OP_PEERCACHE_ANSWER);
						theStats.AddUpDataOverheadFileRequest(pEd2kPacket->size);
						SendPacket(pEd2kPacket);
					}
					break;
				}
			case OP_PEERCACHE_ANSWER:
				{
					theStats.AddDownDataOverheadFileRequest(uRawSize);
					if ( (!client->ProcessPeerCacheAnswer(packet, size)) && client->GetDownloadState() != DS_NONEEDEDPARTS)
					{
						// We have sent a PeerCache Query to the remote client, for any reason the remote client
						// can not process it -> fall back to ed2k download.
						client->SetPeerCacheDownState(PCDS_NONE);
						ASSERT( client->m_pPCDownSocket == NULL );

						// PC-TODO: Check client state.
						ASSERT( client->GetDownloadState() == DS_DOWNLOADING );
						client->SetDownloadState(DS_ONQUEUE, _T("Peer cache query trouble")); // clear block requests
						if (client)
							client->StartDownload();
					}
					break;
				}
			case OP_PEERCACHE_ACK:
				{
					theStats.AddDownDataOverheadFileRequest(uRawSize);
					client->ProcessPeerCacheAcknowledge(packet, size);
					break;
				}
			case OP_PUBLICIP_ANSWER:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_PublicIPAns", client);
					theStats.AddDownDataOverheadOther(uRawSize);

					ed2k_client->ProcessPublicIPAnswer(packet, size);
					break;
				}
			case OP_PUBLICIP_REQ:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_PublicIPReq", client);
					theStats.AddDownDataOverheadOther(uRawSize);

					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugSend("OP__PublicIPAns", client);
					Packet* pPacket = new Packet(OP_PUBLICIP_ANSWER, 4, OP_EMULEPROT);
					PokeUInt32(pPacket->pBuffer, client->GetIP());
					theStats.AddUpDataOverheadOther(pPacket->size);
					SendPacket(pPacket);
					break;
				}
			case OP_PORTTEST:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_PortTest", client);
					theStats.AddDownDataOverheadOther(uRawSize);

					m_bPortTestCon=true;
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugSend("OP__PortTest", client);
					Packet* replypacket = new Packet(OP_PORTTEST, 1);
					replypacket->pBuffer[0]=0x12;
					theStats.AddUpDataOverheadOther(replypacket->size);
					SendPacket(replypacket);
					break;
				}
			case OP_CALLBACK:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_Callback", client);
					theStats.AddDownDataOverheadFileRequest(uRawSize);

					if (!Kademlia::CKademlia::IsRunning())
						break;
					CSafeMemFile data(packet, size);
					Kademlia::CUInt128 check;
					data.ReadUInt128(&check);
					check.Xor(Kademlia::CUInt128(true));
					if (check == Kademlia::CKademlia::GetPrefs()->GetKadID())
					{
						Kademlia::CUInt128 fileid;
						data.ReadUInt128(&fileid);
						uchar fileid2[16];
						fileid.ToByteArray(fileid2);
						CKnownFile* reqfile;
						if ( (reqfile = CGlobalVariable::sharedfiles->GetFileByID(fileid2)) == NULL )
						{
							if ( (reqfile = CGlobalVariable::downloadqueue->GetFileByID(fileid2)) == NULL)
							{
								client->CheckFailedFileIdReqs(fileid2);
								break;
							}
						}

						uint32 ip = data.ReadUInt32();
						uint16 tcp = data.ReadUInt16();
						CUpDownClient* callback;
						callback = CGlobalVariable::clientlist->FindClientByIP(ntohl(ip), tcp);
						if ( callback == NULL )
						{
							callback = new CEd2kUpDownClient(NULL,tcp,ip,0,0);
							CGlobalVariable::clientlist->AddClient(callback);
						}
						callback->TryToConnect(true);
					}
					break;
				}
			case OP_BUDDYPING:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_BuddyPing", client);
					theStats.AddDownDataOverheadOther(uRawSize);

					CUpDownClient* buddy = CGlobalVariable::clientlist->GetBuddy();
					if ( buddy != client || client->GetKadVersion() == 0 || !ed2k_client->AllowIncomeingBuddyPingPong() )
						//This ping was not from our buddy or wrong version or packet sent to fast. Ignore
						break;
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugSend("OP__BuddyPong", client);
					Packet* replypacket = new Packet(OP_BUDDYPONG, 0, OP_EMULEPROT);
					theStats.AddDownDataOverheadOther(replypacket->size);
					SendPacket(replypacket);
					ed2k_client->SetLastBuddyPingPongTime();
					break;
				}
			case OP_BUDDYPONG:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_BuddyPong", client);
					theStats.AddDownDataOverheadOther(uRawSize);

					CUpDownClient* buddy = CGlobalVariable::clientlist->GetBuddy();
					if ( buddy != client || client->GetKadVersion() == 0 )
						//This pong was not from our buddy or wrong version. Ignore
						break;
					ed2k_client->SetLastBuddyPingPongTime();
					//All this is for is to reset our socket timeout.
					break;
				}
			case OP_REASKCALLBACKTCP:
				{
					theStats.AddDownDataOverheadFileRequest(uRawSize);
					CUpDownClient* buddy = CGlobalVariable::clientlist->GetBuddy();
					if (buddy != client)
					{
						if (thePrefs.GetDebugClientTCPLevel() > 0)
							DebugRecv("OP_ReaskCallbackTCP", client, NULL);
						//This callback was not from our buddy.. Ignore.
						break;
					}
					CSafeMemFile data_in(packet, size);
					uint32 destip = data_in.ReadUInt32();
					uint16 destport = data_in.ReadUInt16();

					//  added by yunchenn, 2006/12/13
					if (size==54)
					{
						if (CGlobalVariable::natthread && packet[22]==OP_VC_NAT_HEADER)
						{
							int nSize= *((DWORD *)(packet+23)) + 5;
							memcpy(((BYTE*)packet)+28, &destip, 4);
							uint16 port = htons(destport);
							memcpy(((BYTE*)packet)+32, &port, 2);
							CGlobalVariable::natthread->ProcessPacket(packet+23, nSize, destip, port);
							break;
						}
					}

					uchar reqfilehash[16];
					data_in.ReadHash16(reqfilehash);
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_ReaskCallbackTCP", client, reqfilehash);
					CKnownFile* reqfile = CGlobalVariable::sharedfiles->GetFileByID(reqfilehash);

					bool bSenderMultipleIpUnknown = false;
					CUpDownClient* sender = CGlobalVariable::uploadqueue->GetWaitingClientByIP_UDP(destip, destport, true, &bSenderMultipleIpUnknown);
					if (!reqfile)
					{
						if (thePrefs.GetDebugClientUDPLevel() > 0)
							DebugSend("OP__FileNotFound", NULL);
						Packet* response = new Packet(OP_FILENOTFOUND,0,OP_EMULEPROT);
						theStats.AddUpDataOverheadFileRequest(response->size);
						if (sender != NULL)
							CGlobalVariable::clientudp->SendPacket(response, destip, destport, sender->ShouldReceiveCryptUDPPackets(), sender->GetUserHash(), false, 0);
						else
							CGlobalVariable::clientudp->SendPacket(response, destip, destport, false, NULL, false, 0);
						break;
					}

					if (sender)
					{
						//Make sure we are still thinking about the same file
						if (md4cmp(reqfilehash, sender->GetUploadFileID()) == 0)
						{
							sender->AddAskedCount();
							sender->SetLastUpRequest();
							//I messed up when I first added extended info to UDP
							//I should have originally used the entire ProcessExtenedInfo the first time.
							//So now I am forced to check UDPVersion to see if we are sending all the extended info.
							//For now on, we should not have to change anything here if we change
							//anything to the extended info data as this will be taken care of in ProcessExtendedInfo()
							//Update extended info.
							if (sender->GetUDPVersion() > 3)
							{
								sender->ProcessExtendedInfo(&data_in, reqfile);
							}
							//Update our complete source counts.
							else if (sender->GetUDPVersion() > 2)
							{
								uint16 nCompleteCountLast= sender->GetUpCompleteSourcesCount();
								uint16 nCompleteCountNew = data_in.ReadUInt16();
								sender->SetUpCompleteSourcesCount(nCompleteCountNew);
								if (nCompleteCountLast != nCompleteCountNew)
								{
									reqfile->UpdatePartsInfo();
								}
							}
							CSafeMemFile data_out(128);
							if (sender->GetUDPVersion() > 3)
							{
								if (reqfile->IsPartFile())
									((CPartFile*)reqfile)->WritePartStatus(&data_out);
								else
									data_out.WriteUInt16(0);
							}
							data_out.WriteUInt16((uint16)CGlobalVariable::uploadqueue->GetWaitingPosition(sender));
							if (thePrefs.GetDebugClientUDPLevel() > 0)
								DebugSend("OP__ReaskAck", sender);
							Packet* response = new Packet(&data_out, OP_EMULEPROT);
							response->opcode = OP_REASKACK;
							theStats.AddUpDataOverheadFileRequest(response->size);
							CGlobalVariable::clientudp->SendPacket(response, destip, destport, sender->ShouldReceiveCryptUDPPackets(), sender->GetUserHash(), false, 0);
						}
						else
						{
							DebugLogWarning(_T("Client UDP socket; OP_REASKCALLBACKTCP; reqfile does not match"));
							TRACE(_T("reqfile:         %s\n"), DbgGetFileInfo(reqfile->GetFileHash()));
							TRACE(_T("sender->GetRequestFile(): %s\n"), sender->GetRequestFile() ? DbgGetFileInfo(sender->GetRequestFile()->GetFileHash()) : _T("(null)"));
						}
					}
					else
					{
						if (!bSenderMultipleIpUnknown)
						{
							if (((uint32)CGlobalVariable::uploadqueue->GetWaitingUserCount() + 50) > thePrefs.GetQueueSize())
							{
								if (thePrefs.GetDebugClientUDPLevel() > 0)
									DebugSend("OP__QueueFull", NULL);
								Packet* response = new Packet(OP_QUEUEFULL,0,OP_EMULEPROT);
								theStats.AddUpDataOverheadFileRequest(response->size);
								CGlobalVariable::clientudp->SendPacket(response, destip, destport, false, NULL, false, 0);
							}
						}
						else
						{
							DebugLogWarning(_T("OP_REASKCALLBACKTCP Packet received - multiple clients with the same IP but different UDP port found. Possible UDP Portmapping problem, enforcing TCP connection. IP: %s, Port: %u"), ipstr(destip), destport);
						}
					}
					break;
				}
			case OP_AICHANSWER:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_AichAnswer", client, (size >= 16) ? packet : NULL);
					theStats.AddDownDataOverheadFileRequest(uRawSize);

					client->ProcessAICHAnswer(packet,size);
					break;
				}
			case OP_AICHREQUEST:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_AichRequest", client, (size >= 16) ? packet : NULL);
					theStats.AddDownDataOverheadFileRequest(uRawSize);

					client->ProcessAICHRequest(packet,size);
					break;
				}
			case OP_AICHFILEHASHANS:
				{
					// those should not be received normally, since we should only get those in MULTIPACKET
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_AichFileHashAns", client, (size >= 16) ? packet : NULL);
					theStats.AddDownDataOverheadFileRequest(uRawSize);

					CSafeMemFile data(packet, size);
					client->ProcessAICHFileHash(&data, NULL);
					break;
				}
			case OP_AICHFILEHASHREQ:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_AichFileHashReq", client, (size >= 16) ? packet : NULL);
					theStats.AddDownDataOverheadFileRequest(uRawSize);

					// those should not be received normally, since we should only get those in MULTIPACKET
					CSafeMemFile data(packet, size);
					uchar abyHash[16];
					data.ReadHash16(abyHash);
					CKnownFile* pPartFile = CGlobalVariable::sharedfiles->GetFileByID(abyHash);
					if (pPartFile == NULL)
					{
						client->CheckFailedFileIdReqs(abyHash);
						break;
					}
					if (client->IsSupportingAICH() && pPartFile->GetAICHHashset()->GetStatus() == AICH_HASHSETCOMPLETE
						&& pPartFile->GetAICHHashset()->HasValidMasterHash())
					{
						if (thePrefs.GetDebugClientTCPLevel() > 0)
							DebugSend("OP__AichFileHashAns", client, abyHash);
						CSafeMemFile data_out;
						data_out.WriteHash16(abyHash);
						pPartFile->GetAICHHashset()->GetMasterHash().Write(&data_out);
						Packet* response = new Packet(&data_out, OP_EMULEPROT, OP_AICHFILEHASHANS);
						theStats.AddUpDataOverheadFileRequest(response->size);
						SendPacket(response);
					}
					break;
				}
			case OP_REQUESTPARTS_I64:
				{
					// see also OP_REQUESTPARTS
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_RequestParts_I64", client, (size >= 16) ? packet : NULL);
					theStats.AddDownDataOverheadFileRequest(size);

					CSafeMemFile data(packet, size);
					uchar reqfilehash[16];
					data.ReadHash16(reqfilehash);

					uint64 auStartOffsets[3];
					auStartOffsets[0] = data.ReadUInt64();
					auStartOffsets[1] = data.ReadUInt64();
					auStartOffsets[2] = data.ReadUInt64();

					uint64 auEndOffsets[3];
					auEndOffsets[0] = data.ReadUInt64();
					auEndOffsets[1] = data.ReadUInt64();
					auEndOffsets[2] = data.ReadUInt64();

					if (thePrefs.GetDebugClientTCPLevel() > 0)
					{
						Debug(_T("  Start1=%I64u  End1=%I64u  Size=%I64u\n"), auStartOffsets[0], auEndOffsets[0], auEndOffsets[0] - auStartOffsets[0]);
						Debug(_T("  Start2=%I64u  End2=%I64u  Size=%I64u\n"), auStartOffsets[1], auEndOffsets[1], auEndOffsets[1] - auStartOffsets[1]);
						Debug(_T("  Start3=%I64u  End3=%I64u  Size=%I64u\n"), auStartOffsets[2], auEndOffsets[2], auEndOffsets[2] - auStartOffsets[2]);
					}

					//Xman Anti-Leecher
					if(client->GetUploadState()==US_BANNED) //just to be sure
					{
						CGlobalVariable::uploadqueue->RemoveFromUploadQueue(client,_T("banned client detected during upload")); 
						client->SetUploadFileID(NULL); 
						AddLeecherLogLine(false,_T("banned client was in upload: %s"),client->DbgGetClientInfo());
					}
					//Xman end

					for (int i = 0; i < ARRSIZE(auStartOffsets); i++)
					{
						if (auEndOffsets[i] > auStartOffsets[i])
						{
							Requested_Block_Struct* reqblock = new Requested_Block_Struct;
							reqblock->StartOffset = auStartOffsets[i];
							reqblock->EndOffset = auEndOffsets[i];
							md4cpy(reqblock->FileID, reqfilehash);
							reqblock->transferred = 0;
							client->AddReqBlock(reqblock);
						}
						else
						{
							if (thePrefs.GetVerbose())
							{
								if (auEndOffsets[i] != 0 || auStartOffsets[i] != 0)
									DebugLogWarning(_T("Client requests invalid %u. file block %I64u-%I64u (%I64d bytes): %s"), i, auStartOffsets[i], auEndOffsets[i], auEndOffsets[i] - auStartOffsets[i], client->DbgGetClientInfo());
							}
						}
					}
					break;
				}
			case OP_COMPRESSEDPART:
			case OP_SENDINGPART_I64:
			case OP_COMPRESSEDPART_I64:
				{
					// see also OP_SENDINGPART
					if (thePrefs.GetDebugClientTCPLevel() > 1){
						if (opcode == OP_COMPRESSEDPART)
							DebugRecv("OP_CompressedPart", client, (size >= 16) ? packet : NULL);
						else if (opcode == OP_SENDINGPART_I64)
							DebugRecv("OP_SendingPart_I64", client, (size >= 16) ? packet : NULL);
						else
							DebugRecv("OP_CompressedPart_I64", client, (size >= 16) ? packet : NULL);
					}

					theStats.AddDownDataOverheadFileRequest(16 + 2*(opcode == OP_COMPRESSEDPART ? 4 : 8));
					client->CheckHandshakeFinished();

					if (client->GetRequestFile() && !client->GetRequestFile()->IsStopped() && (client->GetRequestFile()->GetStatus()==PS_READY || client->GetRequestFile()->GetStatus()==PS_EMPTY))
					{
						client->ProcessBlockPacket(packet, size, (opcode == OP_COMPRESSEDPART || opcode == OP_COMPRESSEDPART_I64), (opcode == OP_SENDINGPART_I64 || opcode == OP_COMPRESSEDPART_I64) );
						if (client->GetRequestFile()->IsStopped() || client->GetRequestFile()->GetStatus()==PS_PAUSED || client->GetRequestFile()->GetStatus()==PS_ERROR)
						{
							client->SendCancelTransfer();
							client->SetDownloadState(client->GetRequestFile()->IsStopped() ? DS_NONE : DS_ONQUEUE);
						}
					}
					else
					{
						client->SendCancelTransfer();
						client->SetDownloadState((client->GetRequestFile()==NULL || client->GetRequestFile()->IsStopped()) ? DS_NONE : DS_ONQUEUE);
					}
					break;
				}
/*
				case OP_CHATCAPTCHAREQ:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_CHATCAPTCHAREQ", client);
					theStats.AddDownDataOverheadOther(uRawSize);
					CSafeMemFile data(packet, size);
					client->ProcessCaptchaRequest(&data);
					break;
				}
				case OP_CHATCAPTCHARES:
				{
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_CHATCAPTCHARES", client);
					theStats.AddDownDataOverheadOther(uRawSize);
					if (size < 1)
						throw GetResString(IDS_ERR_BADSIZE);
					client->ProcessCaptchaReqRes(packet[0]);
					break;
				}
*/
				case OP_FWCHECKUDPREQ: //*Support required for Kadversion >= 6
				{
					// Kad related packet
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_FWCHECKUDPREQ", client);
					theStats.AddDownDataOverheadOther(uRawSize);
					CSafeMemFile data(packet, size);
					ed2k_client->ProcessFirewallCheckUDPRequest(&data);
					break;
				}
				case OP_KAD_FWTCPCHECK_ACK: //*Support required for Kadversion >= 7
				{
					// Kad related packet, replaces KADEMLIA_FIREWALLED_ACK_RES
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_KAD_FWTCPCHECK_ACK", client);
					if (CGlobalVariable::clientlist->IsKadFirewallCheckIP(client->GetIP())){
						if (Kademlia::CKademlia::IsRunning())
							Kademlia::CKademlia::GetPrefs()->IncFirewalled();
					}
					else
						DebugLogWarning(_T("Unrequested OP_KAD_FWTCPCHECK_ACK packet from client %s"), client->DbgGetClientInfo());
					break;
				}
			default:
				theStats.AddDownDataOverheadOther(uRawSize);
				PacketToDebugLogLine(_T("eMule"), packet, size, opcode);
				break;
			}
		}
		catch (CFileException* error)
		{
			error->Delete();
			throw GetResString(IDS_ERR_INVALIDPACKAGE);
		}
		catch (CMemoryException* error)
		{
			error->Delete();
			throw CString(_T("Memory exception"));
		}
	}
	catch (CClientException* ex) // nearly same as the 'CString' exception but with optional deleting of the client
	{
		if (thePrefs.GetVerbose() && !ex->m_strMsg.IsEmpty())
			DebugLogWarning(_T("Error: %s - while processing eMule packet: opcode=%s  size=%u; %s"), ex->m_strMsg, DbgGetMuleClientTCPOpcode(opcode), size, DbgGetClientInfo());
		if (client && ex->m_bDelete)
			client->SetDownloadState(DS_ERROR, _T("Error while processing eMule packet: ") + ex->m_strMsg);
		Disconnect(ex->m_strMsg);
		ex->Delete();
		return false;
	}
	catch (CString error)
	{
		if (thePrefs.GetVerbose() && !error.IsEmpty())
			DebugLogWarning(_T("Error: %s - while processing eMule packet: opcode=%s  size=%u; %s"), error, DbgGetMuleClientTCPOpcode(opcode), size, DbgGetClientInfo());
		if (client)
			client->SetDownloadState(DS_ERROR, _T("ProcessExtPacket error. ") + error);
		Disconnect(_T("ProcessExtPacket error. ") + error);
		return false;
	}
	return true;
}

void CClientReqSocket::PacketToDebugLogLine(LPCTSTR protocol, const uchar* packet, uint32 size, UINT opcode)
{
	if (thePrefs.GetVerbose())
	{
		CString buffer;
		buffer.Format(_T("Unknown %s Protocol Opcode: 0x%02x, Size=%u, Data=["), protocol, opcode, size);
		UINT i;
		for (i = 0; i < size && i < 50; i++)
		{
			if (i > 0)
				buffer += _T(' ');
			TCHAR temp[3];
			_stprintf(temp, _T("%02x"), packet[i]);
			buffer += temp;
		}
		buffer += (i == size) ? _T("]") : _T("..]");
		DbgAppendClientInfo(buffer);
		DebugLogWarning(_T("%s"), buffer);
	}
}

CString CClientReqSocket::DbgGetClientInfo()
{
	CString str;
	SOCKADDR_IN sockAddr = {0};
	int nSockAddrLen = sizeof(sockAddr);
	GetPeerName((SOCKADDR*)&sockAddr, &nSockAddrLen);
	if (sockAddr.sin_addr.S_un.S_addr != 0 && (client == NULL || sockAddr.sin_addr.S_un.S_addr != client->GetIP()))
		str.AppendFormat(_T("IP=%s"), ipstr(sockAddr.sin_addr));
	if (client)
	{
		if (!str.IsEmpty())
			str += _T("; ");
		str += _T("Client=") + client->DbgGetClientInfo();
	}
	return str;
}

void CClientReqSocket::DbgAppendClientInfo(CString& str)
{
	CString strClientInfo(DbgGetClientInfo());
	if (!strClientInfo.IsEmpty())
	{
		if (!str.IsEmpty())
			str += _T("; ");
		str += strClientInfo;
	}
}

void CClientReqSocket::OnConnect(int nErrorCode)
{
	TRACE(_T("%s: client=%08x\n\n"), __FUNCTION__, client);
	/*if(GetConState()==SS_Complete) return;*/

	SetConState(SS_Complete);
	CEMSocket::OnConnect(nErrorCode);
	if (nErrorCode)
	{
		CString strTCPError;
		if (thePrefs.GetVerbose())
		{
			strTCPError = GetFullErrorMessage(nErrorCode);
			if ((nErrorCode != WSAECONNREFUSED && nErrorCode != WSAETIMEDOUT) || !GetLastProxyError().IsEmpty())
				DebugLogError(_T("Client TCP socket (OnConnect): %s; %s"), strTCPError, DbgGetClientInfo());
		}
		if(client)
			client->m_eLastError = erConnectionRefused;
		Disconnect(strTCPError);
	}
	else
	{
		//This socket may have been delayed by SP2 protection, lets make sure it doesn't time out instantly.
		ResetTimeOutTimer();
	}

	//  added by yunchenn, 2006/11/24
	if (client && m_bUseNat)
	{
		if (!client->SendHelloPacket())
		{
			// client was deleted!
		}
	}

	if(m_pTraverseFac)
	{
		delete m_pTraverseFac;
		m_pTraverseFac = NULL;
	}
}

void CClientReqSocket::OnSend(int nErrorCode)
{
	ResetTimeOutTimer();
	CEMSocket::OnSend(nErrorCode);
}

void CClientReqSocket::OnError(int nErrorCode)
{
	CString strTCPError;
	if (thePrefs.GetVerbose())
	{
		if (nErrorCode == ERR_WRONGHEADER)
			strTCPError = _T("Error: Wrong header");
		else if (nErrorCode == ERR_TOOBIG)
			strTCPError = _T("Error: Too much data sent");
		else if (nErrorCode == ERR_ENCRYPTION)
			strTCPError = _T("Error: Encryption layer error");
		else if (nErrorCode == ERR_ENCRYPTION_NOTALLOWED)
			strTCPError = _T("Error: Unencrypted Connection when Encryption was required");
		else
			strTCPError = GetErrorMessage(nErrorCode);
		DebugLogWarning(_T("Client TCP socket: %s; %s"), strTCPError, DbgGetClientInfo());
	}

	Disconnect(strTCPError);
}

bool CClientReqSocket::PacketReceivedCppEH(Packet* packet)
{
	bool bResult;
	UINT uRawSize = packet->size;
	switch (packet->prot)
	{
	case OP_EDONKEYPROT:
		bResult = ProcessPacket((const BYTE*)packet->pBuffer, packet->size, packet->opcode);
		break;
	case OP_PACKEDPROT:
		if (!packet->UnPackPacket())
		{
			if (thePrefs.GetVerbose())
				DebugLogError(_T("Failed to decompress client TCP packet; %s; %s"), DbgGetClientTCPPacket(packet->prot, packet->opcode, packet->size), DbgGetClientInfo());
			bResult = false;
			break;
		}
	case OP_EMULEPROT:
		bResult = ProcessExtPacket((const BYTE*)packet->pBuffer, packet->size, packet->opcode, uRawSize);
		break;
	default:
		{
			theStats.AddDownDataOverheadOther(uRawSize);
			if (thePrefs.GetVerbose())
				DebugLogWarning(_T("Received unknown client TCP packet; %s; %s"), DbgGetClientTCPPacket(packet->prot, packet->opcode, packet->size), DbgGetClientInfo());

			if (client)
				client->SetDownloadState(DS_ERROR, _T("Unknown protocol"));
			Disconnect(_T("Unknown protocol"));
			bResult = false;
		}
	}
	return bResult;
}

#if !NO_USE_CLIENT_TCP_CATCH_ALL_HANDLER
int FilterSE(DWORD dwExCode, LPEXCEPTION_POINTERS pExPtrs, CClientReqSocket* reqsock, Packet* packet)
{
	if (thePrefs.GetVerbose())
	{
		CString strExError;
		if (pExPtrs)
		{
			const EXCEPTION_RECORD* er = pExPtrs->ExceptionRecord;
			strExError.Format(_T("Error: Unknown exception %08x in CClientReqSocket::PacketReceived at 0x%08x"), er->ExceptionCode, er->ExceptionAddress);
		}
		else
			strExError.Format(_T("Error: Unknown exception %08x in CClientReqSocket::PacketReceived"), dwExCode);

		// we already had an unknown exception, better be prepared for dealing with invalid data -> use another exception handler
		try
		{
			CString strError = strExError;
			strError.AppendFormat(_T("; %s"), DbgGetClientTCPPacket(packet?packet->prot:0, packet?packet->opcode:0, packet?packet->size:0));
			reqsock->DbgAppendClientInfo(strError);
			DebugLogError(_T("%s"), strError);
		}
		catch (...)
		{
			ASSERT(0);
			DebugLogError(_T("%s"), strExError);
		}
	}

	// this searches the next exception handler -> catch(...) in 'CAsyncSocketExHelperWindow::WindowProc'
	// as long as I do not know where and why we are crashing, I prefere to have it handled that way which
	// worked fine in 28a/b.
	//
	// 03-J鋘-2004 [bc]: Returning the execution to the catch-all handler in 'CAsyncSocketExHelperWindow::WindowProc'
	// can make things even worse, because in some situations, the socket will continue fireing received events. And
	// because the processed packet (which has thrown the exception) was not removed from the EMSocket buffers, it would
	// be processed again and again.
	//return EXCEPTION_CONTINUE_SEARCH;

	// this would continue the program "as usual" -> return execution to the '__except' handler
	return EXCEPTION_EXECUTE_HANDLER;
}
#endif//!NO_USE_CLIENT_TCP_CATCH_ALL_HANDLER

#if !NO_USE_CLIENT_TCP_CATCH_ALL_HANDLER
int CClientReqSocket::PacketReceivedSEH(Packet* packet)
{
	int iResult;
	// this function is only here to get a chance of determining the crash address via SEH
	__try{
		iResult = PacketReceivedCppEH(packet);
	}
	__except(FilterSE(GetExceptionCode(), GetExceptionInformation(), this, packet))
	{
		iResult = -1;
	}
	return iResult;
}
#endif//!NO_USE_CLIENT_TCP_CATCH_ALL_HANDLER

bool CClientReqSocket::PacketReceived(Packet* packet)
{
	bool bResult;
#if !NO_USE_CLIENT_TCP_CATCH_ALL_HANDLER
	int iResult = PacketReceivedSEH(packet);
	if (iResult < 0)
	{
		if (client)
			client->SetDownloadState(DS_ERROR, _T("Unknown Exception"));
		Disconnect(_T("Unknown Exception"));
		bResult = false;
	}
	else
		bResult = iResult!=0;
#else//!NO_USE_CLIENT_TCP_CATCH_ALL_HANDLER
	bResult = PacketReceivedCppEH(packet);
#endif//!NO_USE_CLIENT_TCP_CATCH_ALL_HANDLER
	return bResult;
}

void CClientReqSocket::OnReceive(int nErrorCode)
{
	ResetTimeOutTimer();
	CEMSocket::OnReceive(nErrorCode);
}

bool CClientReqSocket::Create()
{
	CGlobalVariable::listensocket->AddConnection();
	return (CAsyncSocketEx::Create(0, SOCK_STREAM, FD_WRITE | FD_READ | FD_CLOSE | FD_CONNECT, thePrefs.GetBindAddrA()) != FALSE);
}

SocketSentBytes CClientReqSocket::SendControlData(uint32 maxNumberOfBytesToSend, uint32 overchargeMaxBytesToSend)
{
	SocketSentBytes returnStatus = CEMSocket::SendControlData(maxNumberOfBytesToSend, overchargeMaxBytesToSend);
	if (returnStatus.success && (returnStatus.sentBytesControlPackets > 0 || returnStatus.sentBytesStandardPackets > 0))
		ResetTimeOutTimer();
	return returnStatus;
}

SocketSentBytes CClientReqSocket::SendFileAndControlData(uint32 maxNumberOfBytesToSend, uint32 overchargeMaxBytesToSend)
{
	SocketSentBytes returnStatus = CEMSocket::SendFileAndControlData(maxNumberOfBytesToSend, overchargeMaxBytesToSend);
	if (returnStatus.success && (returnStatus.sentBytesControlPackets > 0 || returnStatus.sentBytesStandardPackets > 0))
		ResetTimeOutTimer();
	return returnStatus;
}

void CClientReqSocket::SendPacket(Packet* packet, bool delpacket, bool controlpacket, uint32 actualPayloadSize, bool bForceImmediateSend)
{
	ResetTimeOutTimer();
	CEMSocket::SendPacket(packet, delpacket, controlpacket, actualPayloadSize, bForceImmediateSend);
}


void CClientReqSocket::OnFailConnect(int /*nErrorCode*/)
{
	ASSERT( m_bUseNat );
	timeout_timer = ::GetTickCount();
	if (client) 
	{
#ifdef _DEBUG
		client->AddPeerLog(new CTraceError(_T("L2L connect failed!")));
#endif
		client->SetDownloadState(DS_LOWTOLOWIP);
	}

	Disconnect( _T("L2L connect failed!") );
}

// VC-kernel[2007-01-19]:
bool CClientReqSocket::IsLan()
{
	if ( NULL!=client )
		return client->GetSourceFrom() == SF_LAN;
	else
		return false;
}

