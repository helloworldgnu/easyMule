/* 
 * $Id: HttpClient.cpp 10311 2009-02-04 10:43:59Z huby $
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
#include <wininet.h>
#include "HttpClient.h"
#include "PartFile.h"
#include "Packets.h"
#include "ListenSocket.h"
#include "HttpClientReqSocket.h"
#include "Preferences.h"
#include "OtherFunctions.h"
#include "Statistics.h"
#include "ClientCredits.h"
#include "StringConversion.h"

#include "DNSManager.h"
#include "GlobalVariable.h"
#include "emule.h"

#include "WndMgr.h"
#include "UserMsgs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


///////////////////////////////////////////////////////////////////////////////
// CHttpClient

IMPLEMENT_DYNAMIC(CHttpClient, CINetClient)

CHttpClient::CHttpClient(IPSite *pIPSite) : CINetClient(pIPSite)
{
	m_nRangeOffset = 0;
	m_iRedirected  = 0;
	m_clientSoft   = SO_URL;

	m_uReqStart		=0; 
	m_uReqEnd		=0;

	m_bDataTransfering = FALSE;
	m_bKnownSize	= FALSE;
	m_bFirstHeader	= TRUE;

	m_iPeerType = ptHttp;
	m_iUrlEncodeTypeToUse = UET_NOENCODE;
	m_iUrlEncodeTypeSucced = UET_NONE;
}

void CHttpClient::SetRequestFile(CPartFile* pReqFile)
{
	CUpDownClient::SetRequestFile(pReqFile);
	
	if ( reqfile && (reqfile->GetPartFileSizeStatus()==FS_KNOWN || reqfile->GetPartFileSizeStatus()==FS_KNOWN_FROM_ORIGINAL) )
	{
		m_nPartCount = reqfile->GetPartCount();
		if(NULL==m_abyPartStatus)
			m_abyPartStatus = new uint8[m_nPartCount];
		memset(m_abyPartStatus, 0, m_nPartCount);
		m_bCompleteSource = false;
	}
}

// pszUrl - 不一定已经做了utf-8编码的http url
bool CHttpClient::SetUrl(LPCTSTR pszUrl, uint32 nIP)
{
	m_strURL = CString(pszUrl); // Add URL Record
	if( _tcsrchr( pszUrl+7,_T('/') )==NULL )
		m_strURL += _T("/");
	
	/// 必须保持用户初始给定的Url信息！这样发送 GET的时候才能正确获取
	//m_strURL = URLDecode( m_strURL ); 

	m_strRefer = ParseRef( m_strURL );

	USES_CONVERSION;

	TCHAR szScheme[INTERNET_MAX_SCHEME_LENGTH];
	TCHAR szHostName[INTERNET_MAX_HOST_NAME_LENGTH];
	TCHAR szUrlPath[INTERNET_MAX_PATH_LENGTH];
	TCHAR szUserName[INTERNET_MAX_USER_NAME_LENGTH];
	TCHAR szPassword[INTERNET_MAX_PASSWORD_LENGTH];
	TCHAR szExtraInfo[INTERNET_MAX_URL_LENGTH];
	URL_COMPONENTS Url = {0};
	Url.dwStructSize = sizeof(Url);
	Url.lpszScheme = szScheme;
	Url.dwSchemeLength = ARRSIZE(szScheme);
	Url.lpszHostName = szHostName;
	Url.dwHostNameLength = ARRSIZE(szHostName);
	Url.lpszUserName = szUserName;
	Url.dwUserNameLength = ARRSIZE(szUserName);
	Url.lpszPassword = szPassword;
	Url.dwPasswordLength = ARRSIZE(szPassword);
	Url.lpszUrlPath = szUrlPath;
	Url.dwUrlPathLength = ARRSIZE(szUrlPath);
	Url.lpszExtraInfo = szExtraInfo;
	Url.dwExtraInfoLength = ARRSIZE(szExtraInfo);
	
	if (!InternetCrackUrl(m_strURL, 0, 0, &Url))
	{
		return false;
	}

	if (Url.dwSchemeLength == 0 || Url.nScheme != INTERNET_SCHEME_HTTP)		// we only support "http://"
		return false;
	if (Url.dwHostNameLength == 0)			// we must know the hostname
		return false;
	if (Url.dwUserNameLength != 0)			// no support for user/password
		return false;
	if (Url.dwPasswordLength != 0)			// no support for user/password
		return false;
	if (Url.dwUrlPathLength == 0)			// we must know the URL path on that host
		return false;

	m_strHostA = szHostName;

/*
	TCHAR szEncodedUrl[INTERNET_MAX_URL_LENGTH];
	DWORD dwEncodedUrl = ARRSIZE(szEncodedUrl);
	if (!InternetCanonicalizeUrl(pszUrl, szEncodedUrl, &dwEncodedUrl, ICU_ENCODE_PERCENT))
	{
		return false;
	}
*/

	//CString strUrlPathEncoded = EncodeUrlUtf8( CString(Url.lpszUrlPath) + CString(Url.lpszExtraInfo) );
	m_strUrlPath = CString(Url.lpszUrlPath) + CString(Url.lpszExtraInfo);//CStringA(strUrlPathEncoded);	
	if( m_strUrlPath.Find(_T('#'))>0 )
		m_strUrlPath = m_strUrlPath.Left( m_strUrlPath.Find(_T('#')) );
	m_strUrlPath.Trim();

	m_nUrlStartPos = (uint64)-1;

	//NOTE: be very careful with what is stored in the following IP/ID/Port members!
	if (nIP)
	{
		m_nConnectIP = nIP;
		m_dwUserIP	 = nIP;
		ResetIP2Country();
	}
	else
	{
		m_nConnectIP = inet_addr(T2A(szHostName));
	}

	m_nUserIDHybrid = htonl(m_nConnectIP);
	ASSERT( m_nUserIDHybrid != 0 );
	m_nUserPort = Url.nPort;

	CString sUserName;
#ifdef _DEBUG_PEER
	sUserName.Format( _T("%s(%d)"),ipstr(nIP),m_iPeerIndex);
#else
	sUserName = ipstr( nIP );
#endif
	SetUserName( sUserName );

	return true;
}

CHttpClient::~CHttpClient()
{
	
}

bool CHttpClient::SendHttpBlockRequests()
{	
	USES_CONVERSION;	
	m_dwLastBlockReceived = ::GetTickCount();	
	if (reqfile == NULL)
	{
		throw CString(_T("Failed to send block requests - No 'reqfile' attached"));
	}

	CStringA strHttpRequest;

	if( (reqfile->GetPartFileSizeStatus()==FS_KNOWN || reqfile->GetPartFileSizeStatus()==FS_KNOWN_FROM_ORIGINAL ) && reqfile->GetFileSize()!=(uint64)0 && reqfile->GetPartCount()>0 )
	{
		if( m_PendingBlocks_list.IsEmpty() && !TryToGetBlockRequests(INetPeerBlockReqCount) )
		{
			return false;	
		}
		else
		{	
			EnsureContinueBlockRequest();
			m_nUrlStartPos = m_uReqStart;
		}
		
		ASSERT( (m_uReqEnd-m_uReqStart)>=0 );

		FormatRequestHeader( strHttpRequest,NULL,NULL,m_uReqStart,0); //ToDo-HUBY
	}
	else
	{
		FormatRequestHeader( strHttpRequest,NULL,NULL,0,0 ); //第一次,从头位置发请求,因为还不知道大小
		m_nUrlStartPos = 0;
	}
 
	CRawPacket* pHttpPacket = new CRawPacket(strHttpRequest);
	
	theStats.AddUpDataOverheadFileRequest(pHttpPacket->size);
	
	if(socket)
		socket->SendPacket(pHttpPacket);
#ifdef _DEBUG
	else
		ASSERT( 0 );
#endif

	STATIC_DOWNCAST(CHttpClientDownSocket, socket)->SetHttpState(HttpStateRecvExpected);

#ifdef _DEBUG_PEER
	Debug( _T("Peer(%d)-pt:(%d),PendingBlocks(%d),FormatRequestHeader:%-24hs \n"),m_iPeerIndex,m_iPeerType,m_PendingBlocks_list.GetCount(),(LPCSTR)strHttpRequest );
#endif

	return true;
}

bool CHttpClient::TryToConnect(bool bIgnoreMaxCon, bool bNoCallbacks, CRuntimeClass* pClassSocket)
{
	if( reqfile==NULL )
		return false;

	// 后加入的Peer可以预先领取一遍干活任务,领不到活就不连接
	if( (reqfile->GetPartFileSizeStatus()==FS_KNOWN || reqfile->GetPartFileSizeStatus()==FS_KNOWN_FROM_ORIGINAL ) && reqfile->GetFileSize()!=(uint64)0 && reqfile->GetPartCount()>0 )
	{
		if( !TryToGetBlockRequests(INetPeerBlockReqCount) )
			return false;
	}
	else
	{
		reqfile->m_dwTickGetFileSize = GetTickCount();
	}
	
	//////////////////////////////////////////////////////////////////////////

	if (!socket || !socket->IsConnected())
	{
		if (socket)
			socket->Safe_Delete();

		socket = static_cast<CClientReqSocket*>(RUNTIME_CLASS(CHttpClientDownSocket)->CreateObject());
		socket->SetClient(this);

		if (!socket->Create())
		{
			socket->Safe_Delete();
			return true;
		}
	}
	else
	{
		return true;
	}

	SetDownloadState( DS_CONNECTING );

	return Connect();		
}

bool CHttpClient::Connect()
{
	if (GetConnectIP() != 0 && GetConnectIP() != INADDR_NONE)
	{

		CString temp;
		temp.Format(GetResString(IDS_CONNECT_INFOMATION),(CString)m_strHostA, m_nUserPort);
		AddPeerLog(new CTraceInformation(temp));
/*
		temp.Format(_T("Connecting %s [IP=%s:%d]"), (CString)m_strHostA, ipstr(GetConnectIP()), m_nUserPort);
		AddPeerLog(new CTraceInformation(temp));
*/

		return CUpDownClient::Connect();
	}

	//Try to always tell the socket to WaitForOnConnect before you call Connect.
	socket->WaitForOnConnect();
	socket->Connect(m_strHostA, m_nUserPort);
	return true;
}

void CHttpClient::OnSocketConnected(int nErrorCode)
{
	m_bFirstHeader = TRUE;

	if (nErrorCode == 0)
	{
		AddPeerLog(new CTraceInformation(GetResString(IDS_CONNECTED)));
		SetDownloadState( DS_CONNECTED );
		SendHttpBlockRequests();
	}
	else 
	{
		SetDownloadState(DS_ERROR);
	}
}
bool CHttpClient::Disconnected(LPCTSTR pszReason, bool bFromSocket, CClientReqSocket* /*pSocket*/)
{
#ifdef _DEBUG_PEER
	Debug( _T("Peer(%d)-Http Disconnected because-%s \n"),m_iPeerIndex,pszReason );
#endif	

	CHttpClientDownSocket* s = STATIC_DOWNCAST(CHttpClientDownSocket, socket);

	//TRACE(_T("%hs: HttpState=%u, Reason=%s\n"), __FUNCTION__, s==NULL ? -1 : s->GetHttpState(), pszReason);
	// TODO: This is a mess..
	if (s && (s->GetHttpState() == HttpStateRecvExpected || s->GetHttpState() == HttpStateRecvBody))
	{
		m_fileReaskTimes.RemoveKey(reqfile); // ZZ:DownloadManager (one resk timestamp for each file)
	}

	ASSERT( CGlobalVariable::clientlist->IsValidClient(this) );	

	if ( GetDownloadState() == DS_DOWNLOADING )
	{	
		SetDownloadState( DS_ERROR, CString(_T("Disconnected: ")) + pszReason);		
		m_iErrTimes++;
		m_dwErrorCount++;
		getIpSite()->m_dwRetryCount++;
	}
	else if( GetDownloadState() == DS_REDIRECTED )
	{
	}
	else
	{
		if( m_PendingBlocks_list.GetCount()==0 && 
			(reqfile->GetPartFileSizeStatus()==FS_KNOWN || reqfile->GetPartFileSizeStatus()==FS_KNOWN_FROM_ORIGINAL) )
		{		
			SetDownloadState( DS_NONEEDEDPARTS );
		}
		else
		{
			SetDownloadState( DS_ERROR );
			m_iErrTimes++;
			m_dwErrorCount++;
			getIpSite()->m_dwRetryCount++;
			// ensure that all possible block requests are removed from the partfile
			ClearDownloadBlockRequests();
		}		
	}

	if (!bFromSocket && socket)
	{
		ASSERT( CGlobalVariable::listensocket->IsValidSocket(socket) );
		socket->Safe_Delete();
	}

	socket = NULL;

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

	UpdateUI(UI_UPDATE_PEERLIST);


	if( reqfile->GetPartFileSizeStatus()!=FS_UNKNOWN && this->GetFileSize()>0 
		&& reqfile->GetFileSize()!=this->GetFileSize() )
		return false;

	CString sTemp;
	sTemp.Format(GetResString(IDS_AFTER_RECONNECT),m_iErrTimes*thePrefs.GetRetryDelay());
	AddPeerLog(new CTraceError(sTemp));
    
	reqfile->RetryManage(m_iErrTimes);
	return false;
}

bool CHttpClient::ProcessHttpDownResponse(const CStringAArray& astrHeaders)
{
	if (reqfile == NULL)
	{   
		AddPeerLog(new CTraceError(_T("Failed to process received HTTP data block - No 'reqfile' attached")));
		throw CString(_T("Failed to process received HTTP data block - No 'reqfile' attached"));
	}
	
	try
	{
		if (astrHeaders.GetCount() == 0)
		{
			AddPeerLog(new CTraceError(_T("Unexpected HTTP response - No headers available")));
			throw CString(_T("Unexpected HTTP response - No headers available"));
		}

		const CStringA& rstrHdr = astrHeaders.GetAt(0);
		AddPeerLog(new CTraceServerMessage((CString)rstrHdr));

#ifdef _DEBUG_PEER
		TRACE( "Peer(%d) responsed:%s \n",m_iPeerIndex,rstrHdr );
#endif

		UINT uHttpMajVer, uHttpMinVer, uHttpStatusCode;
		if (sscanf(rstrHdr, "HTTP/%u.%u %u", &uHttpMajVer, &uHttpMinVer, &uHttpStatusCode) != 3)
		{
			CString strError;
			strError.Format(_T("Unexpected HTTP response: \"%hs\""), rstrHdr);
			AddPeerLog(new CTraceError((CString)strError));

			throw strError;
		}

		if (uHttpMajVer != 1 || (uHttpMinVer != 0 && uHttpMinVer != 1))
		{
			CString strError;
			strError.Format(_T("Unexpected HTTP version: \"%hs\""), rstrHdr);
			AddPeerLog(new CTraceError((CString)strError));

			throw strError;
		}
        if(uHttpStatusCode >= 400)
		{
			if( getIpSite() && !m_bAddOtherSources )
				getIpSite()->m_dwMaxAllowConnCount = max(getIpSite()->m_dwMaxAllowConnCount--,2);
			if (uHttpStatusCode == 404)
			      m_eLastError =erFileNotExisted;
			else if (uHttpStatusCode == 401)
			      m_eLastError = erUsernameOrPasswdNotMatched;
			else
				 m_eLastError = erUnknown;
			bool bClose = SameErrorManage(m_eLastError);
			if (bClose && getIpSite()->m_dwConnectionCount > 1)
			{ 
			  this->bNeedProcess = false;
               CHttpClientDownSocket *s = STATIC_DOWNCAST(CHttpClientDownSocket, socket);
			   s->Close();
			}
		}
		if( uHttpStatusCode>=HTTP_STATUS_BAD_REQUEST && uHttpStatusCode<=HTTP_STATUS_NOT_FOUND )
		{
			if( m_iUrlEncodeTypeToUse==UET_NOENCODE )
				m_iUrlEncodeTypeToUse = UET_UTF8;
			else if( m_iUrlEncodeTypeToUse==UET_UTF8 )
				m_iUrlEncodeTypeToUse=UET_UTF16;
			else if( m_iUrlEncodeTypeToUse==UET_UTF16 )
				m_iUrlEncodeTypeToUse = UET_NOENCODE;
		}
		else if( m_iUrlEncodeTypeSucced==UET_NONE )
		{
 			m_iUrlEncodeTypeSucced = m_iUrlEncodeTypeToUse;
			if( m_iUrlEncodeTypeToUse!=UET_NOENCODE )
				reqfile->m_UrlEncodeTypeMap.SetAt(m_strURL,m_iUrlEncodeTypeSucced); /// 方便其它同Url的Peer
		}

		bool bExpectData  = uHttpStatusCode == HTTP_STATUS_OK || uHttpStatusCode == HTTP_STATUS_PARTIAL_CONTENT;
		bool bRedirection = uHttpStatusCode == HTTP_STATUS_MOVED || uHttpStatusCode == HTTP_STATUS_REDIRECT;
		if (!bExpectData && !bRedirection)
		{
			CString strError;
			strError.Format(_T("Unexpected HTTP status code \"%u\""), uHttpStatusCode);
			//AddPeerLog(new CTraceError((CString)strError));
			SetCompletePartStatus( false );
			throw strError;
		}
		else if (bExpectData && reqfile->m_iRename!=1 && reqfile->HasNullHash() )
		{
			CString strFileName;
			strFileName = GetFileNameFromUrlStr( m_strUrlPath );
			reqfile->SetFileName( strFileName );
			reqfile->m_iRename = 1;
		}

		bool bNewLocation = false;
		bool bValidContentRange = false;
		uint64 ui64ContentLength = 0;

		for (int i = 1; i < astrHeaders.GetCount(); i++)
		{
			const CStringA& rstrHdr = astrHeaders.GetAt(i);
			AddPeerLog(new CTraceServerMessage((CString)rstrHdr));
#ifdef _DEBUG_PEER
			TRACE( rstrHdr );
			TRACE( "\n" );
#endif
			if (bExpectData && strnicmp(rstrHdr, "Content-Range:", 14) == 0)//非0开始的请求回应
			{
				uint64 ui64Start = 0, ui64End = 0;

				if (sscanf((LPCSTR)rstrHdr + 14," bytes %I64u - %I64u / %I64u", &ui64Start, &ui64End, &m_uiFileSize) != 3)
				{
					CString strError;
					strError.Format(_T("Unexpected HTTP header field \"%hs\""), rstrHdr);
					AddPeerLog(new CTraceError((CString)strError));

					throw strError;
				}
				if ( ( ui64Start!=m_uReqStart ) /*|| ui64End!=m_uReqEnd || ( m_uiFileSize!=reqfile->GetFileSize() && FS_KNOWN==reqfile->GetPartFileSizeStatus()) ) 
					&& (uHttpStatusCode!=HTTP_STATUS_PARTIAL_CONTENT)*/ )				
				{
					CString strError;
					strError.Format(_T("Unexpected HTTP header field \"%hs\""), rstrHdr);
					AddPeerLog(new CTraceError((CString)strError));

					throw strError;
				}
				bValidContentRange = true;
			}
			else if (bExpectData && strnicmp(rstrHdr, "Content-Length:", 15) == 0)//从0开始的请求回应
			{
				m_bKnownSize = TRUE;
				ui64ContentLength = _atoi64((LPCSTR)rstrHdr + 15);			
			} 
			else if (strnicmp(rstrHdr, "Server:", 7) == 0)
			{
				if (m_strClientSoftware.IsEmpty())
				{
					m_strClientSoftware = rstrHdr.Mid(7).Trim();
				}
			}
			else if ( strnicmp(rstrHdr, "Content-Disposition:", 20) == 0 )
			{
				CString sDispParam = CString(rstrHdr.Mid(20).Trim()) + _T(";");
				CString sParamItem;				
				while( sDispParam.Find(_T(';'))>0 ) //遍历查找服务端给的 filename
				{
					sParamItem = sDispParam.Left( sDispParam.Find(_T(';')) ).Trim();
					if( sParamItem.Find(_T('='))>0 )
					{
						if( sParamItem.Left(sParamItem.Find(_T('=')))==_T("filename") )
						{
							CString sFileName = sParamItem.Mid(sParamItem.Find(_T('='))+1);
							if( sFileName.Left(1)==_T("\"") || sFileName.Left(1)==_T("\'") )
							{
								sFileName = sFileName.Left( sFileName.GetLength()-1 );
								sFileName = sFileName.Right( sFileName.GetLength()-1 );
							}
							ParseUrlString( sFileName );
							reqfile->SetFileName( sFileName );
						}
						else if( sParamItem.Left(sParamItem.Find(_T('=')))==_T("size") ) 
						{
							CStringA sFileSize = (CStringA)sParamItem.Right(sParamItem.GetLength()-5);
							m_uiFileSize = _atoi64((LPCSTR)sFileSize);
							m_bKnownSize = TRUE;
						}
					}
					sDispParam = sDispParam.Mid( sDispParam.Find(_T(';'))+1 );
					sDispParam.Trim();
				}
			}
			else if (bRedirection && strnicmp(rstrHdr, "Location:", 9) == 0)
			{
				CString strLocation(rstrHdr.Mid(9).Trim());
#ifdef _DEBUG_PEER
				TRACE( _T("Peer(%d) redirected to %s,now addToDNS.\n"),m_iPeerIndex,strLocation);
#endif
				CString sLogOut;
				sLogOut.Format(GetResString(IDS_REDIRECT),strLocation);				
				AddPeerLog(new CTraceInformation((CString)sLogOut));

				OnRedirect( strLocation );			
				bNewLocation = true;
			}
		}//for

		if (bNewLocation)
		{
			m_bDataTransfering = FALSE;

			m_iRedirected++;
			if (m_iRedirected >= 3)
			{
				AddPeerLog(new CTraceError(_T("Max. HTTP redirection count exceeded")));
				throw CString(_T("Max. HTTP redirection count exceeded"));
			}

			return false;				// tell our old parent object (which was marked as to get deleted 
			// and which is no longer attached to us) to disconnect.
		}
		else
		{
			m_bDataTransfering = TRUE;

			if( m_uiFileSize==0 )
				m_uiFileSize = ui64ContentLength; //"Content-Range:"中如果没有拿到FileSize,那么就靠"Content-Length:"决定

			CString sLogOut;	
			if ( (reqfile->GetPartFileSizeStatus()==FS_KNOWN || reqfile->GetPartFileSizeStatus()==FS_KNOWN_FROM_ORIGINAL ) 
				&& reqfile->GetFileSize()>(uint64)0  ) 
			{ 				
				if( m_uiFileSize!=reqfile->GetFileSize() ) //和下载任务已知大小不一致,该Peer无效
				{
					if( !IsOriginalUrlSite() || reqfile->GetPartFileSizeStatus()==FS_KNOWN_FROM_ORIGINAL )
					{
						SetSiteBadOrGood();
						CString sLogOut;
						sLogOut.Format( _T("This peer FileSize is not OK: =%i64, Get Url from original link."),m_uiFileSize );
						AddPeerLog(new CTraceError((CString)sLogOut));
						throw sLogOut;
					}
					else 
					{
						reqfile->SetFileSize(EMFileSize(m_uiFileSize));
						::SendMessage(theWndMgr.GetWndHandle(CWndMgr::WI_DOWNLOADING_LISTCTRL), UM_RECREATE_PARTFILE, 0, (LPARAM)reqfile);		//Added by thilon on 2008.04.29
						return false;
					}
				}				
				else
				{
					if( getIpSite() && !m_bAddOtherSources && !m_bAllowedAddMoreConn )
					{
						getIpSite()->m_dwMaxAllowConnCount = min(getIpSite()->m_dwMaxAllowConnCount++,thePrefs.GetMaxSourceConnect());
						m_bAllowedAddMoreConn = true;
					}
					SetCompletePartStatus( );
					if( reqfile->m_BlockRangeList.IsEmpty() )
						reqfile->SplitFileToBlockRange();
				}
			}
			else if( reqfile->GetPartFileSizeStatus()==FS_UNKNOWN ) //reqfile->GetFileSize()==(uint64)0
			{				
				//ADDED by VC-fengwen 2007/08/01 <bein> : 当此时还不知道文件大小时，则以“无大小”初始PartFile
				if( !m_bKnownSize )
				{
				    AddPeerLog(new CTraceError(GetResString(IDS_UNKNOWN_FILE)));
					reqfile->InitNosizePartFile();
				}			
				//ADDED by VC-fengwen 2007/08/01 <end> : 当此时还不知道文件大小时，则以“无大小”初始PartFile
				else if( m_uiFileSize>0 )
				{
					sLogOut.Format(GetResString(IDS_GET_FILESIZE),m_uiFileSize);
					AddPeerLog(new CTraceInformation(sLogOut));
					reqfile->OnGetFileSizeFromInetPeer( m_uiFileSize,IsOriginalUrlSite() );// VC-Huby[2007-07-30]:通知任务层
					// VC-Huby[2007-07-30]:set part property of this http peer directly
					SetCompletePartStatus( );
					CreateBlockRequests(1);	//PARTSIZE / EMBLOCKSIZE+1
				}
				else //虽然知道大小,但确实为零
				{	
					AddPeerLog(new CTraceInformation(GetResString(IDS_ZERO_INFO)));
					Disconnected(_T("Filesize of this httpClient url is zero!"));					
					SetDownloadState(DS_NONEEDEDPARTS);
					reqfile->ZeroSize_CompleteDownLoad();
					return false;
				}			
			}

			if (m_bFirstHeader && !m_bKnownSize)
			{
				m_nRangeOffset = 0;
				m_nTransferredDown = 0;
			}

		}

		if (!bExpectData
			&& !bValidContentRange
			&& uHttpStatusCode != HTTP_STATUS_NOT_FOUND)
		{
			return true;
		}

		SetDownloadState(DS_DOWNLOADING);	
		reqfile->UpdatePartsInfo();

		m_bFirstHeader = FALSE;

		return true;
	}
	catch ( ... )
	{
		//m_iErrTimes++;
		//SetDownloadState( DS_ERROR );
		//ClearDownloadBlockRequests();
		Disconnected( _T(" HttpDown Repsponse Err!") );
		//AddPeerLog(new CTraceError(_T("HttpDown Response Err!")));
		return false;
	}	
}

void CHttpClient::ProcessNoSizeRawData(const BYTE * pucData, UINT uSize)
{
	if (reqfile->IsStopped() || (reqfile->GetStatus() != PS_READY && reqfile->GetStatus() != PS_EMPTY))
	{
		throw CString(_T("Failed to process HTTP data block - File not ready for receiving data"));
	}

	if (!(GetDownloadState() == DS_DOWNLOADING || GetDownloadState() == DS_NONEEDEDPARTS))
	{
		throw CString(_T("Failed to process HTTP data block - Invalid download state"));
	}

	m_dwLastBlockReceived = ::GetTickCount();

	thePrefs.Add2SessionTransferData(GetClientSoft(), (GetClientSoft()==SO_URL) ? (UINT)-2 : (UINT)-1, false, false, uSize);
	m_nDownDataRateMS += uSize;


	uint32 lenWritten = reqfile->NoSize_WriteToBuffer(uSize, pucData, m_nRangeOffset, m_nRangeOffset + uSize - 1, NULL, this);
	m_nRangeOffset += uSize;

	if (lenWritten > 0)
	{
		m_nTransferredDown += uSize;
		m_nCurSessionPayloadDown += lenWritten;
		SetTransferredDownMini();
	}
}

void CHttpClient::ProcessRawData(const BYTE * pucData, UINT uSize)
{   
	if( m_nDownloadState!=DS_DOWNLOADING )
		return;

	// VC-wangna[2007-11-26]: Add other connect
	if( m_bAddOtherSources && (GetTickCount()-m_dwLastAddOtherSources)>3000 )//判断是否需要加入其它几个连接
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

		while(i <iMax)
		{
 			CHttpClient* client = new CHttpClient(getIpSite());
			client->m_bAddOtherSources = false;
			if (!client->SetUrl(m_strURL, getIpSite()->m_dwIpAddress))
			{
				LogError(LOG_STATUSBAR, _T("Failed to process URL source \"%s\""), m_strURL);
				delete client;
				return;
			}
			if( _tcslen(m_strRefer)>0 )
				client->m_strRefer = m_strRefer;
			client->SetRequestFile(reqfile);
			client->SetSourceFrom(SF_HTTP);
			if (CGlobalVariable::downloadqueue->CheckAndAddSource(reqfile, client))
			{
				reqfile->UpdatePartsInfo();
			}
			i++;
		}
		//m_bAddOtherSources = false;
	}
   // VC-wangna[2007-11-26]: Add other connect

	getIpSite()->m_pUrlSite->m_dwOldDataTransferedWithoutPayload = getIpSite()->m_pUrlSite->m_dwDataTransferedWithoutPayload;
    getIpSite()->m_pUrlSite->m_dwDataTransferedWithoutPayload += uSize;
    UpdateTransData(m_strURL);

	if (!m_bKnownSize)
	{
		ProcessNoSizeRawData(pucData, uSize);
		return;
	}

	if (reqfile == NULL)
	{
		throw CString(_T("Failed to process HTTP data block - No 'reqfile' attached"));
	}

	if (reqfile->IsStopped() || (reqfile->GetStatus() != PS_READY && reqfile->GetStatus() != PS_EMPTY))
	{
		throw CString(_T("Failed to process HTTP data block - File not ready for receiving data"));
	}

	if (m_nUrlStartPos == (uint64)-1)
	{
		throw CString(_T("Failed to process HTTP data block - Unexpected file data"));
	}

	if( m_PendingBlocks_list.GetCount()==0 )
	{
		throw CString(_T("Failed to process HTTP data block,PendingBlocks_list is empty"));
	}

	uint64 nStartPos = m_nUrlStartPos;
	uint64 nEndPos = m_nUrlStartPos + uSize;

	m_nUrlStartPos += uSize;

	if (!(GetDownloadState() == DS_DOWNLOADING || GetDownloadState() == DS_NONEEDEDPARTS))
		throw CString(_T("Failed to process HTTP data block - Invalid download state"));

	m_dwLastBlockReceived = ::GetTickCount();

	if (nEndPos == nStartPos || uSize != nEndPos - nStartPos)
		throw CString(_T("Failed to process HTTP data block - Invalid block start/end offsets"));

	thePrefs.Add2SessionTransferData(GetClientSoft(), (GetClientSoft()==SO_URL) ? (UINT)-2 : (UINT)-1, false, false, uSize);
	m_nDownDataRateMS += uSize;
	if (credits)
		credits->AddDownloaded(uSize, GetIP());
	nEndPos--;
	
/*
	if( m_PendingBlocks_list.IsEmpty() ) //网络数据来了,但此Peer还没领取Block任务
	{	
		int iBlockRequestToGet = max(min(GetDownloadDatarate()*10,EMBLOCKSIZE*10),uSize)/EMBLOCKSIZE +1; //VC-Huby[2007-08-29]: 领取Peer 10s内就可以完成的Block,避免其它Peer打劫后不连续		
		while( GetBlockRange() )
		{
			CreateBlockRequests(iBlockRequestToGet);	 //PARTSIZE / EMBLOCKSIZE + 1
			if( !m_PendingBlocks_list.IsEmpty() )
				break;
		}		
	}*/

	/// 优化为每次数据来了后预留一定的BlockRequest给此Peer
	UINT iBlockRequestToGet = max(min(GetDownloadDatarate()*10,EMBLOCKSIZE*10),uSize)/EMBLOCKSIZE +1;
	if( m_pBlockRangeToDo )
		iBlockRequestToGet = min(iBlockRequestToGet,m_pBlockRangeToDo->m_iBlockIndexE-m_pBlockRangeToDo->m_iBlockCurrentDoing+1);
	if( (UINT)m_PendingBlocks_list.GetCount()<iBlockRequestToGet )
	{
		//iBlockRequestToGet = iBlockRequestToGet-m_PendingBlocks_list.GetCount();
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
			
	ASSERT( m_PendingBlocks_list.GetCount()>0 );

	uint64 nTempEndPos;
	UINT   uTempSize;
	uint32 lenWrittenTotal =0;
	BYTE * pucDataForWrite = (BYTE *)pucData;

	for (POSITION pos = m_PendingBlocks_list.GetHeadPosition(); pos != NULL; )
	{
		POSITION posLast = pos;
		Pending_Block_Struct *cur_block = m_PendingBlocks_list.GetNext(pos);

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

			if (lenWritten >=0  && tempfile->GetPartFileSizeStatus() != FS_UNKNOWN )
			{
				m_nTransferredDown += uSize;
				m_nCurSessionPayloadDown += lenWritten;
				SetTransferredDownMini();

				lenWrittenTotal +=lenWritten;

				if (nTempEndPos >= cur_block->block->EndOffset)//网络数据把cur_block gap填充完毕
				{
					reqfile->RemoveBlockFromList(cur_block->block->StartOffset, cur_block->block->EndOffset);
#ifdef _DEBUG_PEER
					Debug( _T("Peer(%d)-Http Finished BlockJob(%d):%I64u-%I64u \n"),m_iPeerIndex,cur_block->block->BlockIdx,cur_block->block->StartOffset,cur_block->block->EndOffset );
#endif
					if(m_pBlockRangeToDo && cur_block->block->BlockIdx!=(uint32)-1 
						&& cur_block->block->BlockIdx>=m_pBlockRangeToDo->m_iBlockIndexS
						&& cur_block->block->BlockIdx<=m_pBlockRangeToDo->m_iBlockIndexE )
					{
						if( (cur_block->block->BlockIdx+1)<=m_pBlockRangeToDo->m_iBlockIndexE )
						{
							m_pBlockRangeToDo->m_iBlockCurrentDoing = cur_block->block->BlockIdx+1;
							ASSERT( m_pBlockRangeToDo->m_iBlockCurrentDoing>=m_pBlockRangeToDo->m_iBlockIndexS 
								&& m_pBlockRangeToDo->m_iBlockCurrentDoing<=m_pBlockRangeToDo->m_iBlockIndexE);							
						}
						else
						{
#ifdef _DEBUG_PEER
							Debug( _T("Peer(%d)-Http Finished BlockRange(%d-%d-%d)\n"),m_iPeerIndex,m_pBlockRangeToDo->m_iBlockIndexS,m_pBlockRangeToDo->m_iBlockCurrentDoing,m_pBlockRangeToDo->m_iBlockIndexE );
#endif							
							CString sLogOut;
							sLogOut.Format(GetResString(IDS_FINISHED_BLOCK),m_pBlockRangeToDo->m_iBlockIndexS,m_pBlockRangeToDo->m_iBlockIndexE);
							AddPeerLog(new CTraceInformation(sLogOut));
							m_pBlockRangeToDo->m_bRangeIsFinished = true;
							if( reqfile->GetTotalGapSizeInBlockRange(m_pBlockRangeToDo->m_iBlockIndexS,m_pBlockRangeToDo->m_iBlockIndexE)==0 )
								m_pBlockRangeToDo->m_bDataFinished = true;							
						}
					}
					delete cur_block->block;
					delete cur_block;
					m_PendingBlocks_list.RemoveAt(posLast);

					if( m_pBlockRangeToDo && m_pBlockRangeToDo->m_bRangeIsFinished )
					{
						ASSERT( m_PendingBlocks_list.IsEmpty() );
						ClearDownloadBlockRequests();
						m_pBlockRangeToDo = NULL;
					}

					if (m_PendingBlocks_list.IsEmpty())
					{
						if (thePrefs.GetDebugClientTCPLevel() > 0)
							DebugSend("More block requests", this);
						
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
								socket->Safe_Delete();
								socket = NULL;
								TryToConnect(); /// 直接立刻在新位置重连
								return;
							}
						}
						pos = m_PendingBlocks_list.GetHeadPosition();
					}
				}
			}
			
			pucDataForWrite += uTempSize;
			if( nStartPos>nEndPos )
				return; /// 本次收到的数据全部写完
		}
		else
		{
			/// 需断开重连(可能是往前跳了,或是往后跳太大了),跳点重新发http请求
#ifdef _DEBUG_PEER
			Debug( _T("Peer(%d) Need ReConnect,req cur_Block[%d](%I64u-%I64u),but urlPos(%I64u,%I64u) \n"),m_iPeerIndex,
				cur_block->block->BlockIdx,cur_block->block->StartOffset,cur_block->block->EndOffset,nStartPos,nEndPos );
#endif			
			socket->Safe_Delete();
			socket = NULL;
			TryToConnect(); /// 直接立刻在新位置重连
			return;
		}
	}//for

#ifdef _DEBUG_PEER
	TRACE("%s - Dropping packet,lenwritenTotal/uSize:(%d/%d)-Peer(%d)\n", __FUNCTION__,lenWrittenTotal,uSize,m_iPeerIndex);
#endif
}

void CHttpClient::SendCancelTransfer(Packet* /*packet*/)
{
	if (socket)
	{
		STATIC_DOWNCAST(CHttpClientDownSocket, socket)->SetHttpState(HttpStateUnknown);
		socket->Safe_Delete();
	}
}

void CHttpClient::OnNoSizeFileComplete()
{
	if (reqfile != NULL)
	{
		if (m_bDataTransfering && !m_bKnownSize)
		{
			SetDownloadState(DS_NONEEDEDPARTS);
			reqfile->NoSize_CompleteDownLoad();
			CGlobalVariable::filemgr.ModifyURLState(reqfile->GetPartFileURL(), _T(""),FILESTATE_COMPLETED);
		}	
	}
}

void CHttpClient::FormatRequestHeader( CStringA& strHttpRequest,char * /*pCookie*/,char * /*pReferer*/,uint64 uiFrom,uint64 uiTo )
{
	CStringA temp;
	
	URLEncodeType urlEncodeTypeToFormat;
	if( m_iUrlEncodeTypeSucced!=UET_NONE )
	{
		urlEncodeTypeToFormat = m_iUrlEncodeTypeSucced;
	}
	else if(  reqfile->m_UrlEncodeTypeMap.Lookup( m_strURL,urlEncodeTypeToFormat ) ) 
	{
		m_iUrlEncodeTypeSucced = urlEncodeTypeToFormat;
	}
	else
	{
		urlEncodeTypeToFormat = m_iUrlEncodeTypeToUse;
	}

	CStringA strUrlPathToGet;
	if( urlEncodeTypeToFormat==UET_UTF8 )
	{
		strUrlPathToGet = EncodeUrlUtf8(m_strUrlPath);
	}
	else if( urlEncodeTypeToFormat==UET_NOENCODE )
	{
		strUrlPathToGet = m_strUrlPath;
	}
	else if( urlEncodeTypeToFormat==UET_UTF16 )
	{
		strUrlPathToGet = m_strUrlPath; //TODO
	}

	strHttpRequest.AppendFormat("GET %s HTTP/1.1\r\n", strUrlPathToGet);//TODO-0727
	AddPeerLog(new CTraceSendMessage((CString)strHttpRequest));

	strHttpRequest.AppendFormat("Accept: */*\r\n");
	AddPeerLog(new CTraceSendMessage(_T("Accept: */*")));

	strHttpRequest.AppendFormat("Cache-Control: no-cache\r\n");
	AddPeerLog(new CTraceSendMessage(_T("Cache-Control: no-cache")));

	if(uiTo>uiFrom)
	{
		strHttpRequest.AppendFormat("Range: bytes=%I64u-%I64u\r\n", uiFrom, uiTo);

		temp.Format("Range: bytes=%I64u-%I64u", uiFrom, uiTo);
		AddPeerLog(new CTraceSendMessage((CString)temp));
	}
	else if (uiFrom != 0) //有些Web可能不支持Range
	{
		strHttpRequest.AppendFormat("Range: bytes=%I64u-\r\n", uiFrom);

		temp.Format("Range: bytes=%I64u-", uiFrom);
		AddPeerLog(new CTraceSendMessage((CString)temp));
	}

	strHttpRequest.AppendFormat("Connection: close\r\n"); //Connection: Keep-Alive
	AddPeerLog(new CTraceSendMessage(_T("Connection: close")));

	
	DWORD dwCookieSize;

	if( InternetGetCookie(m_strURL,NULL,NULL,&dwCookieSize) )
	{
		TCHAR *pCookieData = new TCHAR[dwCookieSize];
		InternetGetCookie(m_strURL,NULL,pCookieData,&dwCookieSize);
		CStringA sCookieDataA = CStringA(pCookieData);
		strHttpRequest.AppendFormat("Cookie: %s\r\n",sCookieDataA);		
		temp.Format("Cookie: %s",sCookieDataA);
		AddPeerLog(new CTraceSendMessage((CString)temp));
		delete[] pCookieData;
	}

	if( 80==m_nUserPort )
		strHttpRequest.AppendFormat("Host: %s\r\n", m_strHostA);
	else
		strHttpRequest.AppendFormat("Host: %s:%d\r\n", m_strHostA ,m_nUserPort);

	temp.Format("Host: %s\r\n", m_strHostA);
	AddPeerLog(new CTraceSendMessage((CString)temp));

	strHttpRequest.AppendFormat("Pragma: no-cache\r\n");
	AddPeerLog(new CTraceSendMessage(_T("Pragma: no-cache")));

	if ( 0==m_strHostA.CompareNoCase("verycd.com") || m_strHostA.MakeLower().Find(".verycd.com")>=0 )
	{
		temp.Format( "User-Agent: VeryCD %s v%s\r\n",CStringA(GetResString(IDS_CAPTION)),CStringA(CGlobalVariable::GetCurVersionLong()) );//浏览器客户端标示，告诉我们自己的服务器这是eMule，另外可以附上版本号，用户操作系统等信息
		strHttpRequest.AppendFormat(temp);
		AddPeerLog(new CTraceSendMessage((CString)temp));
	}
	else
	{
		strHttpRequest.AppendFormat("User-Agent: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.0; .NET CLR 1.1.4322)\r\n");	//浏览器客户端标示，伪装成IE浏览器
		AddPeerLog(new CTraceSendMessage(_T("User-Agent: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.0; .NET CLR 1.1.4322)")));
	}

	if( m_strRefer.IsEmpty() ) //url中没有给定默认referer时候取默认域名为 referer
	{
		char buffer[1024];
		CString strRefer = m_strURL.Left(m_strURL.ReverseFind('/') + 1);
		int nMulti = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)strRefer.GetBuffer(),strRefer.GetLength(), NULL, 0 , NULL, NULL);
		WideCharToMultiByte(CP_ACP, 0, strRefer.GetBuffer(), strRefer.GetLength(), buffer, nMulti, NULL, NULL);
		m_strRefer = CString(buffer, nMulti);
	}	

	temp.Format("Referer: %s", CStringA(m_strRefer));
	AddPeerLog(new CTraceSendMessage((CString)temp));

	strHttpRequest.AppendFormat("Referer: %s\r\n", CStringA(m_strRefer));

	strHttpRequest.AppendFormat("\r\n");

	if (thePrefs.GetDebugClientTCPLevel() > 0)
	{
		Debug(_T("Sending HTTP request:\n%hs"), strHttpRequest);
	}
}

void CHttpClient::Pause()
{
	CUpDownClient::Pause();
	AddPeerLog(new CTraceError(_T("User cancel")));
	
	if ( GetDownloadState() != DS_DOWNLOADING && GetDownloadState() != DS_NONE )
	{
		SetDownloadState( DS_NONE,_T("Paused") );
	}

	if (!m_bKnownSize)
	{
		m_bDataTransfering = FALSE;
	}
}

void CHttpClient::EnsureContinueBlockRequest()
{
	//以下计算发送请求数据的起始和结束位置
	POSITION pos = m_PendingBlocks_list.GetHeadPosition();
	Pending_Block_Struct* pending = m_PendingBlocks_list.GetNext(pos);
	m_uReqStart = pending->block->StartOffset;
	m_uReqEnd = pending->block->EndOffset;

	bool bMergeBlocks = true;
	while (pos)
	{
		POSITION posLast = pos;
		pending = m_PendingBlocks_list.GetNext(pos);
		if (bMergeBlocks && pending->block->StartOffset == m_uReqEnd + 1) //连续的Block请求
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
}

void CHttpClient::OnRedirect( CString& strLocation )
{		
	bool bRedirectInSameSite = false;
	CString sUrlShort;
	if ((strLocation.Left(7).CompareNoCase(_T("http://")) == 0) || (strLocation.Left(6).CompareNoCase(_T("ftp://")) == 0))
	{	// VC-SearchDream[2007-07-23]: the following condition need DNS	
		
		CUrlSite *pUrlSiteAdded=NULL;
		if( reqfile && m_pIpSite && m_pIpSite->m_pUrlSite )
		{
			pUrlSiteAdded = reqfile->RecordUrlSource( strLocation,!reqfile->IsStopped(),0,(ESiteFrom)m_pIpSite->m_pUrlSite->m_dwFromWhere );
			if(pUrlSiteAdded)
				pUrlSiteAdded->m_pRedirectFrom = m_pIpSite->m_pUrlSite;
		}
		
		sUrlShort = strLocation.Right( strLocation.GetLength() - strLocation.Find(_T("://")) -3 );
		//reqfile->SetPartFileURL( strLocation );	//COMMENTED by VC-fengwen 2007/08/16 : 保留源下载地址作唯一标识					
	}
	else
	{
		bRedirectInSameSite = true;
		if( strLocation.Left(1)!=_T("/") )
			strLocation = _T("/") + strLocation;
		sUrlShort = strLocation;
	}

	CString sNewPath;
	if( reqfile->HasNullHash() || reqfile->GetFileName()==_T("") )
	{					
		int iIndex = sUrlShort.ReverseFind(_T('/'));
		if( iIndex==-1 || 
			sUrlShort.GetLength() - 1 == iIndex )
		{
			sNewPath		= sUrlShort;
		}
		else
		{
			sNewPath = sUrlShort.Left(iIndex);
		}	
	}

	if( bRedirectInSameSite )
	{
		m_strUrlPath = strLocation;
		m_strRefer.Format(_T("http://%s/%s/"),CString(m_strHostA),sNewPath);
		m_iUrlEncodeTypeSucced = UET_NONE;
	}
	else
	{
		SetDownloadState( DS_REDIRECTED );
		Disconnected(_T("This url of this httpClient has been redirected!"));					
	}
}


////////////////////////////////////////////////////////////////////////////
bool CUpDownClient::ProcessHttpDownResponse(const CStringAArray& )
{
	ASSERT(0);
	return false;
}
