/*
 * $Id: HttpClient.h 9297 2008-12-24 09:55:04Z dgkang $
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

#include "INetClient.h"
#include "TimerOp_UrlClientRetry.h"


//////////////////////////////////////////////////////////////////////////
enum URLEncodeType
{
	UET_NONE,
	UET_NOENCODE, //对url不做编码处理，直接发送..
	UET_UTF8,	
	UET_UTF16
};


///////////////////////////////////////////////////////////////////////////////
// CHttpClient

class CHttpClient : public CINetClient
{
	DECLARE_DYNAMIC(CHttpClient)

public:
	CHttpClient(IPSite *pIPSite);
	virtual ~CHttpClient();

	bool SetUrl(LPCTSTR pszUrl, uint32 nIP = 0);

	virtual void SetRequestFile(CPartFile* pReqFile);

	virtual bool TryToConnect(bool bIgnoreMaxCon = false, bool bNoCallbacks = false, CRuntimeClass* pClassSocket = NULL);
	virtual bool Connect();
	virtual void OnSocketConnected(int nErrorCode);
	virtual bool Disconnected(LPCTSTR pszReason, bool bFromSocket = false, CClientReqSocket* pSocket=NULL);

	virtual void ProcessRawData(const BYTE * pucData, UINT uSize);
	virtual void ProcessNoSizeRawData(const BYTE * pucData, UINT uSize);

	virtual bool SendHttpBlockRequests();
	virtual void SendCancelTransfer(Packet* packet);

	bool ProcessHttpDownResponse(const CStringAArray& astrHeaders);

	void OnNoSizeFileComplete();

	virtual void	Pause();	//ADDED by VC-fengwen 2007/08/03 : 

	CString						m_strRefer;	

private:
	URLEncodeType				m_iUrlEncodeTypeToUse;
	URLEncodeType				m_iUrlEncodeTypeSucced;

	/// 确保PendingBlockList 中是连续的Block请求
	void						EnsureContinueBlockRequest( );
	
	/// 处理重定向后逻辑
	void						OnRedirect( CString& strLocation );	
	int							m_iRedirected;

	CStringA					m_strHostA;
	// VC-SearchDream[2007-07-23]: Add URL Record
	CString						m_strURL;

	// VC-SearchDream[2007-06-19]: for no size file downloading
	uint64						m_nRangeOffset;		
	BOOL						m_bDataTransfering;
	BOOL						m_bKnownSize;
	BOOL						m_bFirstHeader;

	void	FormatRequestHeader( CStringA& strHttpRequest,char *pCookie,char *pReferer,uint64 uiFrom,uint64 uiTo ); //< 拼出 http request header stringlist	
};
