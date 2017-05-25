/*
 * $Id: FtpClient.cpp 10311 2009-02-04 10:43:59Z huby $
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
#include "ftpclient.h"

#include <wininet.h>
#include "PartFile.h"
#include "Preferences.h"
#include "ClientCredits.h"
#include "globalvariable.h"
#include "emule.h"

#include "WndMgr.h"
#include "UserMsgs.h"

IMPLEMENT_DYNAMIC(CFtpClient, CINetClient)

CFtpClient::CFtpClient(IPSite *pIPSite) : CINetClient(pIPSite)
{
	m_clientSoft		= SO_URL;
	m_strClientSoftware = _T("ftp server");
	m_pFtpDataSocket	= NULL;
    
	m_iPeerType = ptFtp;
}

CFtpClient::~CFtpClient(void)
{
	//CMDSocket 会在CUpDownClient中Delete并CloseSocket

	if( m_pFtpDataSocket )
	{
		m_pFtpDataSocket->client = NULL; 
		delete m_pFtpDataSocket;
	}
}

void CFtpClient::SetRequestFile(CPartFile* pReqFile)
{
	CUpDownClient::SetRequestFile(pReqFile);
	if (reqfile)
	{
		m_nPartCount    = reqfile->GetPartCount();
		if(NULL==m_abyPartStatus)
			m_abyPartStatus = new uint8[m_nPartCount];
		memset(m_abyPartStatus, 0, m_nPartCount);
		m_bCompleteSource = false;
	}
}

bool CFtpClient::SetUrl(LPCTSTR pszUrl, uint32 nIP)
{
	if ( !m_SourceURL.ParseFTP( pszUrl ) )
	{
		return false;
	}

	m_nUrlStartPos = (uint64)-1;

	//SetUserName(m_SourceURL.m_sAddress);

	if (nIP)
	{
		m_nConnectIP = nIP;
		m_dwUserIP	 = nIP;
		ResetIP2Country();
	}
	else
	{
		return false; // We do not do DNS here
	}

	m_nUserIDHybrid = htonl(m_nConnectIP);
	ASSERT( m_nUserIDHybrid != 0 );
	m_nUserPort     = m_SourceURL.m_nPort;

	CString sUserName;
#ifdef _DEBUG_PEER
	sUserName.Format( _T("%s(%d)"),ipstr(nIP),m_iPeerIndex);
#else
	sUserName = ipstr( nIP );
#endif
	SetUserName( sUserName );

	return true;
}

bool CFtpClient::SendFtpBlockRequests()
{
	USES_CONVERSION;
	
	m_dwLastBlockReceived = ::GetTickCount();
	
	if (reqfile == NULL)
	{
		throw CString(_T("Failed to send block requests - No 'reqfile' attached"));
	}

	if( !TryToGetBlockRequests(INetPeerBlockReqCount) )
		return false;	

	POSITION pos = m_PendingBlocks_list.GetHeadPosition();
	Pending_Block_Struct* pending = m_PendingBlocks_list.GetNext(pos);

	m_uReqStart = pending->block->StartOffset;
	m_uReqEnd   = pending->block->EndOffset;
	
	bool bMergeBlocks = true;
	
	while (pos)
	{
		POSITION posLast = pos;
		pending = m_PendingBlocks_list.GetNext(pos);
		if (bMergeBlocks && pending->block->StartOffset == m_uReqEnd + 1)
		{
			m_uReqEnd = pending->block->EndOffset;
		}
		else
		{
			bMergeBlocks = false;
			reqfile->RemoveBlockFromList(pending->block->StartOffset, pending->block->EndOffset);
			delete pending->block;
			delete pending;
			m_PendingBlocks_list.RemoveAt(posLast);
		}
	}

	m_nUrlStartPos = m_uReqStart;

	Debug( _T("BlockRequest(Ftp):BlockCount=%d,start=%I64u,end=%I64u \n"),m_PendingBlocks_list.GetCount(),m_uReqStart,m_uReqEnd );

	return true;
}

bool CFtpClient::TryToConnect(bool bIgnoreMaxCon, bool bNoCallbacks, CRuntimeClass* pClassSocket)
{
	if( m_pFtpDataSocket )
	{
		delete m_pFtpDataSocket;
		m_pFtpDataSocket = NULL;
	}

	if( reqfile->GetPartFileSizeStatus()==FS_UNKNOWN )
	{
		reqfile->m_dwTickGetFileSize = GetTickCount();
	}

	return CUpDownClient::TryToConnect(bIgnoreMaxCon,bNoCallbacks,RUNTIME_CLASS(CFtpClientReqSocket));	
}

bool CFtpClient::Connect()
{
	if (GetConnectIP() != 0 && GetConnectIP() != INADDR_NONE)
	{
		CString temp;
		temp.Format(GetResString(IDS_CONNECT_INFOMATION),m_SourceURL.m_sAddress,m_nUserPort);
		AddPeerLog(new CTraceInformation(temp));

		return CUpDownClient::Connect();
	}

	//Try to always tell the socket to WaitForOnConnect before you call Connect.
	socket->WaitForOnConnect();
	//socket->Connect(m_strHostA, m_nUserPort);

	return true;
}

void CFtpClient::OnSocketConnected(int nErrorCode)
{
	if (nErrorCode == 0)
	{
		AddPeerLog(new CTraceInformation(GetResString(IDS_CONNECTED)));
		SetDownloadState( DS_CONNECTED );
	}

	m_nConnectingState = CCS_NONE;
	CGlobalVariable::clientlist->RemoveConnectingClient(this);
}

bool CFtpClient::Disconnected(LPCTSTR pszReason, bool bFromSocket ,CClientReqSocket* pSocket)
{
 	if( pSocket && pSocket->IsKindOf(RUNTIME_CLASS(CFtpClientDataSocket)) )
	{   //只是ftpDataSocket Disconnect
		CloseDataSocket();
		if( socket==NULL )
			return false;
		CFtpClientReqSocket* reqSocket =  DYNAMIC_DOWNCAST(CFtpClientReqSocket,socket);
		reqSocket->SetFtpState(ftpClose); 		
		return false;
	}

#ifdef _DEBUG_PEER
	Debug( _T("Peer(%d)-Ftp Disconnected because-%s \n"),m_iPeerIndex,pszReason );
#endif	

	//return CUpDownClient::Disconnected(CString(_T("CFtpClient::Disconnected")) + pszReason, bFromSocket);
	ASSERT( CGlobalVariable::clientlist->IsValidClient(this) );

	m_iErrTimes++;
	m_dwErrorCount++;
	getIpSite()->m_dwRetryCount++;

	SetDownloadState( DS_ERROR, CString(_T("Disconnected: ")) + pszReason);		

	if ( GetDownloadState() != DS_DOWNLOADING )
	{		
		// ensure that all possible block requests are removed from the partfile
		ClearDownloadBlockRequests();
	}

	if (!bFromSocket && socket)
	{
		ASSERT( CGlobalVariable::listensocket->IsValidSocket(socket) );
		socket->Safe_Delete();
	}

	socket = NULL;

	CloseDataSocket();

	if( m_pBlockRangeToDo!=NULL )
	{
		m_pBlockRangeToDo->m_pClient = NULL;
		m_pBlockRangeToDo->m_dwTakeOverTime = 0;
		m_pBlockRangeToDo = NULL;
	}

/*
	if( GetDownloadState()==DS_ERROR )
		SetCompletePartStatus(false);
*/

	m_nConnectingState = CCS_NONE;
	CGlobalVariable::clientlist->RemoveConnectingClient(this);

	UpdateUI(UI_UPDATE_PEERLIST);

	if( reqfile->GetPartFileSizeStatus()!=FS_UNKNOWN && this->GetFileSize()>0 
		&& reqfile->GetFileSize()!=this->GetFileSize() )
		return false;

	CString sTemp;
	sTemp.Format(GetResString(IDS_AFTER_RECONNECT),m_iErrTimes*thePrefs.GetRetryDelay());
	AddPeerLog(new CTraceError(sTemp));
 
	return false;
}

/// 下载的文件数据来了,在这里处理
void CFtpClient::ProcessRawData(const BYTE * pucData, UINT uSize)
{
	if( m_nDownloadState!=DS_DOWNLOADING )
		return;
/*
	if( socket )
	{
		CFtpClientReqSocket* reqSocket =  DYNAMIC_DOWNCAST(CFtpClientReqSocket,socket);
		if( reqSocket->GetFtpState()!=ftpDownloading )
			return;
	}

	CFtpClientReqSocket* reqSocket =  DYNAMIC_DOWNCAST(CFtpClientReqSocket,socket);
	ASSERT( reqSocket->GetFtpState()==ftpDownloading );
*/
	// VC-wangna[2007-11-26]: Add other connect
	if(m_bAddOtherSources && (GetTickCount()-m_dwLastAddOtherSources)>3000)//判断是否需要加入其它几个连接
	{
		m_dwLastAddOtherSources = GetTickCount();
		int i = 0;
		int iMax = reqfile->GetOtherConnectNum(getIpSite()->m_dwConnectionCount);
		if( getIpSite() )
		{
			iMax = min(iMax,int(getIpSite()->m_dwMaxAllowConnCount-getIpSite()->m_dwConnectionCount));
		}
		if( i <iMax )
		{
			// 还需要再发起连接,这里需要查看同一个IP上是否还有连接
			if( IsNeedAvoidInitalizeConnection() ) {
				i = iMax;
			}
		}

		while(i< iMax)
		{
 			CFtpClient * client = new CFtpClient(getIpSite());
			client->m_bAddOtherSources = false;
			if (!client->SetUrl(getIpSite()->m_pUrlSite->m_strUrl, m_pIpSite->m_dwIpAddress))
			{
				LogError(LOG_STATUSBAR, _T("Failed to process URL source \"%s\""), getIpSite()->m_pUrlSite->m_strUrl);
				delete client;
				return;
			}
			client->SetRequestFile(reqfile);
			client->SetSourceFrom(SF_FTP);
			if (CGlobalVariable::downloadqueue->CheckAndAddSource(reqfile, client))
			{
				reqfile->UpdatePartsInfo(); //TODO: 对于INet Peer，这里可能不需要了
			}
			i++;
		}
		//m_bAddOtherSources = false;
	}
	// VC-wangna[2007-11-26]: Add other connect

	getIpSite()->m_pUrlSite->m_dwOldDataTransferedWithoutPayload = getIpSite()->m_pUrlSite->m_dwDataTransferedWithoutPayload;
	getIpSite()->m_pUrlSite->m_dwDataTransferedWithoutPayload += uSize;
	UpdateTransData(getIpSite()->m_pUrlSite->m_strUrl);

#ifdef _DEBUG_PEER_DATA
	Debug( _T("Receive Data from FTP size : %d \n"), uSize);
#endif
	if (reqfile == NULL)
	{
		throw CString(_T("Failed to process FTP data block - No 'reqfile' attached"));
	}

	if (reqfile->IsStopped() || (reqfile->GetStatus() != PS_READY && reqfile->GetStatus() != PS_EMPTY))
	{
		return; //throw CString(_T("Failed to process FTP data block - File not ready for receiving data"));
	}

	if (m_nUrlStartPos == (uint64)-1)
	{
		throw CString(_T("Failed to process FTP data block - Unexpected file data"));
	}

	if( socket!=NULL )
	{
		socket->ResetTimeOutTimer(); /// 避免命令通道Socket超时检测中断.
	}

	uint64 nStartPos = m_nUrlStartPos;
	uint64 nEndPos   = m_nUrlStartPos + uSize;

	m_nUrlStartPos  += uSize;

	m_dwLastBlockReceived = ::GetTickCount();

	if( nEndPos == nStartPos || uSize != nEndPos - nStartPos )
	{
		throw CString(_T("Failed to process FTP data block - Invalid block start/end offsets"));
	}

	thePrefs.Add2SessionTransferData(GetClientSoft(), (GetClientSoft()==SO_URL) ? (UINT)-2 : (UINT)-1, false, false, uSize);

	m_nDownDataRateMS += uSize;

	if (credits)
	{
		credits->AddDownloaded(uSize, GetIP());
	}

	nEndPos--;

	UINT iBlockRequestToGet = max(min(GetDownloadDatarate()*10,EMBLOCKSIZE*10),uSize)/EMBLOCKSIZE +1;
	if( m_pBlockRangeToDo )
		iBlockRequestToGet = min(iBlockRequestToGet,m_pBlockRangeToDo->m_iBlockIndexE-m_pBlockRangeToDo->m_iBlockCurrentDoing+1);
	if( (UINT)m_PendingBlocks_list.GetCount()<iBlockRequestToGet ) //网络数据来了,但此Peer还没领取Block任务或是没领够
	{
		while( GetBlockRange() )
		{
			CreateBlockRequests(iBlockRequestToGet);	 //PARTSIZE / EMBLOCKSIZE + 1
			if( !m_PendingBlocks_list.IsEmpty() )
				break;
		}		
	}

	if( m_PendingBlocks_list.IsEmpty() )
	{
		Disconnected( _T( "NONEEDEDPARTS from this Peer.") );
		SetDownloadState( DS_NONEEDEDPARTS );
		TRACE("%s - Dropping packet(%d)-Peer(%d),because DS_NONEEDEDPARTS \n", __FUNCTION__,uSize,m_iPeerIndex);
		return;
	}

	uint64 nTempEndPos;
	UINT   uTempSize;
	uint32 lenWrittenTotal =0;
	BYTE * pucDataForWrite = (BYTE *)pucData;

	Pending_Block_Struct *cur_block;
	POSITION posLast;
	POSITION pos = m_PendingBlocks_list.GetHeadPosition();
	while ( pos != NULL )
	{
		posLast = pos;
		cur_block = m_PendingBlocks_list.GetNext(pos);
		if (cur_block->block->StartOffset <= nStartPos && nStartPos <= cur_block->block->EndOffset)
		{
			if (thePrefs.GetDebugClientTCPLevel() > 0)
			{
				// NOTE: 'Left' is only accurate in case we have one(!) request block!
				void* p = m_pPCDownSocket ? (void*)m_pPCDownSocket : (void*)socket;
				Debug(_T("%08x  Start=%I64u  End=%I64u  Size=%u  Left=%I64u  %s\n"), p, nStartPos, nEndPos, uSize, cur_block->block->EndOffset - (nStartPos + uSize) + 1, DbgGetFileInfo(reqfile->GetFileHash()));
			}

			m_nLastBlockOffset = nStartPos;
			nTempEndPos = min(nEndPos,cur_block->block->EndOffset); /// 考虑可能会跨Block写
			uTempSize   = (UINT)(nTempEndPos - nStartPos + 1);

			// nightsuns: 防止文件被 metalink 这一类重定向了
			CPartFile* tempfile = this->reqfile;
			uint32 lenWritten = reqfile->WriteToBuffer(uTempSize, pucDataForWrite, nStartPos, nTempEndPos, cur_block->block, this);
			if( m_PendingBlocks_list.GetCount()==0 || !socket )
				return;
			nStartPos = nTempEndPos + 1;

			if (lenWritten > 0 && tempfile->GetPartFileSizeStatus() != FS_UNKNOWN )
			{
				m_nTransferredDown += uSize;
				m_nCurSessionPayloadDown += lenWritten;
				SetTransferredDownMini();

				lenWrittenTotal +=lenWritten;

				if (nTempEndPos >= cur_block->block->EndOffset)
				{
					reqfile->RemoveBlockFromList(cur_block->block->StartOffset, cur_block->block->EndOffset);
#ifdef _DEBUG_PEER
					Debug( _T("Peer(%d)-Ftp Finished BlockJob(%d):%I64u-%I64u \n"),m_iPeerIndex,cur_block->block->BlockIdx,cur_block->block->StartOffset,cur_block->block->EndOffset );
#endif

					if(m_pBlockRangeToDo && cur_block->block->BlockIdx!=(uint32)-1 
						&& cur_block->block->BlockIdx>=m_pBlockRangeToDo->m_iBlockIndexS
						&& cur_block->block->BlockIdx<=m_pBlockRangeToDo->m_iBlockIndexE )
					{
						if( (cur_block->block->BlockIdx+1)<=m_pBlockRangeToDo->m_iBlockIndexE )
						{
							m_pBlockRangeToDo->m_iBlockCurrentDoing=cur_block->block->BlockIdx+1;
							ASSERT( m_pBlockRangeToDo->m_iBlockCurrentDoing>=m_pBlockRangeToDo->m_iBlockIndexS 
								&& m_pBlockRangeToDo->m_iBlockCurrentDoing<=m_pBlockRangeToDo->m_iBlockIndexE);							
						}
						else
						{
#ifdef _DEBUG_PEER
							Debug( _T("Peer(%d)-Ftp Finished BlockRange(%d-%d-%d)\n"),m_iPeerIndex,m_pBlockRangeToDo->m_iBlockIndexS,m_pBlockRangeToDo->m_iBlockCurrentDoing,m_pBlockRangeToDo->m_iBlockIndexE );
#endif
							CString sLogOut;
							sLogOut.Format(GetResString(IDS_FINISHED_BLOCK),m_pBlockRangeToDo->m_iBlockIndexS,m_pBlockRangeToDo->m_iBlockIndexE);
							AddPeerLog(new CTraceInformation(sLogOut));
							m_pBlockRangeToDo->m_bRangeIsFinished = true;
							if( reqfile->GetTotalGapSizeInBlockRange(m_pBlockRangeToDo->m_iBlockIndexS,m_pBlockRangeToDo->m_iBlockIndexE)==0 )
								m_pBlockRangeToDo->m_bDataFinished = true;    
							m_pBlockRangeToDo = NULL;
						}
					}

					delete cur_block->block;
					delete cur_block;
					m_PendingBlocks_list.RemoveAt(posLast);
					
					if( m_pBlockRangeToDo && m_pBlockRangeToDo->m_bRangeIsFinished )
					{
						ASSERT( m_PendingBlocks_list.IsEmpty() );
						ClearDownloadBlockRequests();
					}

					if (m_PendingBlocks_list.IsEmpty())
					{
						if (thePrefs.GetDebugClientTCPLevel() > 0)
						{
							DebugSend("More block requests", this);
						}

						int iBlockRequestToGet = max(min(GetDownloadDatarate()*10,EMBLOCKSIZE*10),uSize-lenWrittenTotal)/EMBLOCKSIZE +1;
						CreateBlockRequests(iBlockRequestToGet);	//PARTSIZE / EMBLOCKSIZE+1
					}

					if( !m_PendingBlocks_list.IsEmpty() )
					{
						cur_block = m_PendingBlocks_list.GetHead();
						if( cur_block )
						{
							if( cur_block->block->EndOffset < nStartPos || nStartPos < cur_block->block->StartOffset ) //跳点
							{								
								if( socket==NULL )
									return;
								CFtpClientReqSocket* reqSocket =  DYNAMIC_DOWNCAST(CFtpClientReqSocket,socket);
								reqSocket->Safe_Delete();
								reqSocket = NULL;
								//reqSocket->SetFtpState(ftpABOR); 
								//reqSocket->SendCommand(); 		
								CloseDataSocket();
								TryToConnect();
								return;
							}
						}
						pos = m_PendingBlocks_list.GetHeadPosition();
					}
					
				}//if 填充block完成
			}
			
			pucDataForWrite += uTempSize;
			if( nStartPos>nEndPos )
				return; /// 本次收到的数据全部写完
		}
		else //跳点,和http类似,但ftp只需要断开DataSocket并重连,然后reqSocket重发Rest
		{			
			if( socket==NULL )
				return;
			CFtpClientReqSocket* reqSocket =  DYNAMIC_DOWNCAST(CFtpClientReqSocket,socket);
			reqSocket->Safe_Delete();
			reqSocket = NULL;
			CloseDataSocket();
			TryToConnect();
			return;						
		}
	}//end for


#ifdef _DEBUG_PEER
	TRACE("%s - Dropping packet,lenwritenTotal/uSize:(%d/%d)-Peer(%d)\n", __FUNCTION__,lenWrittenTotal,uSize,m_iPeerIndex);
#endif
	
}

void CFtpClient::SendCancelTransfer(Packet* /*packet*/)
{
	if (socket)
	{
		socket->Safe_Delete();
	}

	if( m_pFtpDataSocket )
	{
		delete m_pFtpDataSocket;
		m_pFtpDataSocket = NULL;
	}
}
bool CFtpClient::OnFileSizeKnown( uint64 uiFileSize )
{
	if (reqfile == NULL)
	{
		throw CString(_T("Failed to process received FTP data block - No 'reqfile' attached"));
	}

	m_uiFileSize = uiFileSize;

	CString sLogOut;
	sLogOut.Format(GetResString(IDS_GET_FILESIZE),m_uiFileSize);
	AddPeerLog(new CTraceInformation(sLogOut));

	if ( reqfile->GetPartFileSizeStatus()==FS_KNOWN || reqfile->GetPartFileSizeStatus()==FS_KNOWN_FROM_ORIGINAL ) 
	{ 
		if( reqfile->GetFileSize()>(uint64)0 && m_uiFileSize!=reqfile->GetFileSize() )
		{
			if( !IsOriginalUrlSite() || reqfile->GetPartFileSizeStatus()==FS_KNOWN_FROM_ORIGINAL )
			{
				SetSiteBadOrGood(); //和下载任务已知大小不一致,该Peer无效
				
				SetCompletePartStatus(false);
				CString strError;
				strError.Format(_T("This peer fileSize is not OK: =%i64"), m_uiFileSize);
				AddPeerLog(new CTraceInformation(sLogOut));
				throw strError;
			}
			else
			{
				reqfile->SetFileSize(EMFileSize(m_uiFileSize));
				::SendMessage(theWndMgr.GetWndHandle(CWndMgr::WI_DOWNLOADING_LISTCTRL), UM_RECREATE_PARTFILE, 0, (LPARAM)reqfile);		//Added by thilon on 2008.04.29
				return false;
			}
		}	
		else // this peer filesize is ok :)
		{
			if( getIpSite() && !m_bAddOtherSources && !m_bAllowedAddMoreConn )
			{
				getIpSite()->m_dwMaxAllowConnCount = min(getIpSite()->m_dwMaxAllowConnCount++,thePrefs.GetMaxSourceConnect());
				m_bAllowedAddMoreConn = true;
			}
			SetCompletePartStatus();
			if( reqfile->m_BlockRangeList.IsEmpty() )
				reqfile->SplitFileToBlockRange();
		}
	}
	else if( reqfile->GetPartFileSizeStatus()==FS_UNKNOWN ) //reqfile->GetFileSize()==(uint64)0
	{						
		if( m_uiFileSize>0 )
		{			
			reqfile->OnGetFileSizeFromInetPeer( m_uiFileSize,IsOriginalUrlSite() );// VC-Huby[2007-07-30]:通知任务层
			// VC-Huby[2007-07-30]:set part property of this ftp peer directly
			SetCompletePartStatus();		
		}
		else //虽然知道大小,但确实为零
		{			
			return false;
		}			
	}
	
	reqfile->UpdatePartsInfo();

	return true;
}

CFtpClientDataSocket* CFtpClient::GetClientDataSocket( bool bCreateIfNull )
{
	if( m_pFtpDataSocket==NULL && bCreateIfNull )
	{		
		m_pFtpDataSocket = new CFtpClientDataSocket(this);		
	}

	return m_pFtpDataSocket;
}

CFtpClientReqSocket*  CFtpClient::GetClientReqSocket( )
{
	if( socket )
		return DYNAMIC_DOWNCAST(CFtpClientReqSocket,socket);
	else
		return NULL;
}

bool CFtpClient::GetUrlStartPosForReq( uint64& uiUrlStartPos )
{
	//对于ftp peer,一定是已经商量好了 filesize,然后获取BlockJob才之知道准确的数据请求位置..
	ASSERT( (reqfile->GetPartFileSizeStatus()==FS_KNOWN || reqfile->GetPartFileSizeStatus()==FS_KNOWN_FROM_ORIGINAL ) 
		&& reqfile->GetFileSize()!=(uint64)0 && reqfile->GetPartCount()>0 );
	
	SendFtpBlockRequests( ); ///领取要下载的Block任务

	uiUrlStartPos = m_nUrlStartPos;

	return true;
}

void CFtpClient::Pause( )
{
	CUpDownClient::Pause( );

	//socket->Safe_Delete(); //先shutdown,再closesocket(这个在其它地方已经做了)
	if( m_pFtpDataSocket )
	{
		m_pFtpDataSocket->Safe_Delete(); 
		m_pFtpDataSocket = NULL;
	}
}

void CFtpClient::CloseDataSocket( )
{
	if(m_pFtpDataSocket)
	{
		delete m_pFtpDataSocket;
		m_pFtpDataSocket = NULL;
	}
}

void CFtpClient::OnCmdSocketErr(CStringA strError)
{ 
	int error = atoi(strError);
	if (error == 550)
		m_eLastError = erFileNotExisted;
	else if (error == 530)
		m_eLastError = erUsernameOrPasswdNotMatched;
	else
		m_eLastError = erUnknown;
 	bool bClose = false;
 	bClose = SameErrorManage(m_eLastError);
 	if (bClose && getIpSite()->m_dwConnectionCount > 1)
 	{
       if(socket)
	   {
		   CFtpClientReqSocket* reqSocket =  DYNAMIC_DOWNCAST(CFtpClientReqSocket,socket);
		   reqSocket->Close();			
	   }
	   
 	   this->bNeedProcess = false;
	}

	if( getIpSite() && !m_bAddOtherSources )
		getIpSite()->m_dwMaxAllowConnCount = max(getIpSite()->m_dwMaxAllowConnCount--,2);

	SetCompletePartStatus(false);

	reqfile->RetryManage(m_iErrTimes);
}
