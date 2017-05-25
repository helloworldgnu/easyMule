/*
 * $Id: UPnpNat.cpp 7686 2008-10-14 10:53:57Z dgkang $
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
#include ".\upnpnat.h"
#include "IpHlpApi.h"
#include "GlobalVariable.h"
#include "http_chunk.h"

#pragma comment (lib, "IpHlpApi.lib")

//#define NUMBEROFDEVICES	2
//static const CString s_devices[][2] = {{_T("WANIPConnection"), _T("service")},
//										{_T("WANPPPConnection"), _T("service")}};

#define UPNPPORTMAP0   _T("WANIPConnection")
#define UPNPPORTMAP1   _T("WANPPPConnection")
static const ULONG	UPNPADDR = 0xFAFFFFEF;
static const int	UPNPPORT = 1900;
static const CString	URNPREFIX = _T("urn:schemas-upnp-org:");

//////////////////////////////////////////////////////////////////////////
//

extern const CString getString(int i);
extern const CString GetArgString(const CString& name, const CString& value);
extern const CString GetArgString(const CString& name, int value);
extern bool parseHTTPResponse(const CString& response, CString& result);
extern const CString getProperty(const CString& all, const CString& name);
extern bool  IsHttpChunked(char * buffer,int len);
extern int DecodeHttpChunked(char *buffer,int len,size_t size);


const CString getString(int i)
{
	CString s;

	s.Format(_T("%d"), i);

	return s;
}

const CString GetArgString(const CString& name, const CString& value)
{
	return _T("<") + name + _T(">") + value + _T("</") + name + _T(">");
}

const CString GetArgString(const CString& name, int value)
{
	return _T("<") + name + _T(">") + getString(value) + _T("</") + name + _T(">");
}

bool parseHTTPResponse(const CString& response, CString& result)
{
	int pos = 0;

	CString status = response.Tokenize(_T("\r\n"), pos);

	result = response;
	result.Delete(0, pos);

	pos = 0;
	status.Tokenize(_T(" "), pos);
	status = status.Tokenize(_T(" "), pos);
	if (status.IsEmpty() || status[0]!='2') return false;
	return true;
}

const CString getProperty(const CString& all, const CString& name)
{
	CString startTag = '<' + name + '>';
	CString endTag = _T("</") + name + '>';
	CString property;

	int posStart = all.Find(startTag);
	if (posStart<0) return CString();

	int posEnd = all.Find(endTag, posStart);
	if (posStart>=posEnd) return CString();

	return all.Mid(posStart + startTag.GetLength(), posEnd - posStart - startTag.GetLength());
}

static CString NGetAddressFromUrl(const CString& str, CString& post, CString& host, int& port)
{
	CString s = str;

	post = _T("");
	host = post;
	port = 0;
	int pos = s.Find(_T("://"));
	//if (!pos) return CString();
	if (-1 == pos) return CString();	//MODIFIED by fengwen on 2006/11/29	: Find返回-1表示没找到。
	s.Delete(0, pos + 3);

	pos = s.Find('/');
	//if (!pos) {
	if (-1 == pos) {		//MODIFIED by fengwen on 2006/11/29	: Find返回-1表示没找到。
		host = s;
		s = _T("");
	} else {
		host = s.Mid(0, pos);
		s.Delete(0, pos);
	}

	if (s.IsEmpty()) {
		post = _T("");
	} else {
		post = s;
	}

	pos = 0;
	CString addr = host.Tokenize(_T(":"), pos);
	
	//MODIFIED by fengwen on 2006/11/29	<begin> :	如果用pos==-1，调用Tokenize会抛出异常。
	//s = host.Tokenize(_T(":"), pos);
	if (addr.IsEmpty() || -1 == pos)
		s = _T("");
	else
		s = host.Tokenize(_T(":"), pos);
	//MODIFIED by fengwen on 2006/11/29	<end> :	如果用pos==-1，调用Tokenize会抛出异常。

	if (s.IsEmpty()) {
		port = 80;
	} else {
		port = _tstoi(s);
	}

	return addr;
}

BOOL GetDefaultGateway(UINT32 &ip)
{
	PIP_ADAPTER_INFO pAdapterInfo = NULL, pCurAdapterInfo;
	ULONG ulOutBufLen = 0;
	DWORD dwRetVal = 0;
	BOOL bRet;

	bRet = FALSE;

	pAdapterInfo = (IP_ADAPTER_INFO *) malloc( sizeof(IP_ADAPTER_INFO) );
	ulOutBufLen = sizeof(IP_ADAPTER_INFO);

	dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen);
	
	if (ERROR_BUFFER_OVERFLOW == dwRetVal)
	{
		free(pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO *) malloc (ulOutBufLen);
		dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen);
	}

	if (ERROR_SUCCESS == dwRetVal)
	{
		pCurAdapterInfo = pAdapterInfo;
		do {
			ip = inet_addr(pCurAdapterInfo->GatewayList.IpAddress.String);
		} while(ip == 0 && (pCurAdapterInfo = pCurAdapterInfo->Next) != NULL);
		bRet = TRUE;
	}

	free(pAdapterInfo);
	pAdapterInfo = NULL;

	return bRet;
}

//////////////////////////////////////////////////////////////////////////
//	member functions 

CUPnpNat::CUPnpNat(void)
{
	m_strBindAddress.Empty();
	m_iActionTimeoutMs = DEF_ACTION_TIMEOUT_MS;

	m_dwLastErrorCode = 0;

	m_version		= 1;
	m_isSearched	= FALSE;
}

CUPnpNat::~CUPnpNat(void)
{
}

HRESULT CUPnpNat::SearchDevice(BOOL bUseDefaultGateway)
{
	if (m_isSearched)
	{
		if (isComplete())
			return S_OK;
		else
			return E_UNAT_SEARCH_PENDING;
	}

	m_isSearched = TRUE;

	if (InternalSearch(1, bUseDefaultGateway))
		return S_OK;
	else
		return E_UNAT_UNKNOWN_ERROR;
}

HRESULT	CUPnpNat::AddPortMapping(LPCTSTR lpszRemoteHost,
					   USHORT usExternalPort, LPCTSTR lpszPortMappingProtocol,
					   USHORT usInternalPort, LPCTSTR lpszInternalClient,
					   LPCTSTR lpszPortMappingDescription,
					   BOOL bPortMappingEnabled,
					   ULONG ulPortMappingLeaseDuration)
{
	CString		args;
	CString		strResponse;
	
	args.Empty();
	args.Append(GetArgString(_T("NewRemoteHost"), CString(lpszRemoteHost)));
	args.Append(GetArgString(_T("NewExternalPort"), usExternalPort));
	args.Append(GetArgString(_T("NewProtocol"), CString(lpszPortMappingProtocol)));
	args.Append(GetArgString(_T("NewInternalPort"), usInternalPort));
	args.Append(GetArgString(_T("NewInternalClient"), CString(lpszInternalClient)));
	args.Append(GetArgString(_T("NewEnabled"), bPortMappingEnabled));
	args.Append(GetArgString(_T("NewPortMappingDescription"), CString(lpszPortMappingDescription)));
	args.Append(GetArgString(_T("NewLeaseDuration"), ulPortMappingLeaseDuration));

	return InvokeCommand(_T("AddPortMapping"), args, strResponse);

}

HRESULT	CUPnpNat::DeletePortMapping(LPCTSTR lpszRemoteHost,
						  USHORT usExternalPort,
						  LPCTSTR lpszPortMappingProtocol)
{
	CString		args;
	CString		strResponse;

	args.Empty();
	args.Append(GetArgString(_T("NewRemoteHost"), CString(lpszRemoteHost)));
	args.Append(GetArgString(_T("NewExternalPort"), usExternalPort));
	args.Append(GetArgString(_T("NewProtocol"), CString(lpszPortMappingProtocol)));

	return InvokeCommand(_T("DeletePortMapping"), args, strResponse);
}

HRESULT	CUPnpNat::GetSpecificPortMappingEntry(LPCTSTR lpszRemoteHost,					//[in]
									USHORT usExternalPort,								//[in]
									LPCTSTR lpszPortMappingProtocol,					//[in]
									USHORT *pusInternalPort,							//[out]
									CString *pstrInternalClient,						//[out]
									bool *pbEnable,										//[out]
									CString *pstrDescription,							//[out]
									ULONG *pulPortMappingLeaseDuration)					//[out]
{
	CString		args;
	CString		strResponse;

	args.Empty();
	args.Append(GetArgString(_T("NewRemoteHost"), CString(lpszRemoteHost)));
	args.Append(GetArgString(_T("NewExternalPort"), usExternalPort));
	args.Append(GetArgString(_T("NewProtocol"), CString(lpszPortMappingProtocol)));

	HRESULT		hr;
	hr = InvokeCommand(_T("GetSpecificPortMappingEntry"), args, strResponse);
	if (FAILED(hr))
		return hr;

	if (NULL != pusInternalPort)
		*pusInternalPort = (USHORT) atoi(CStringA(getProperty(strResponse, _T("NewInternalPort"))));
	if (NULL != pstrInternalClient)
		*pstrInternalClient = getProperty(strResponse, _T("NewInternalClient"));
	if (NULL != pbEnable)
		*pbEnable = (bool) (atoi(CStringA(getProperty(strResponse, _T("NewEnabled")))) == 0 ? false : true);
	if (NULL != pstrDescription)
		*pstrDescription = getProperty(strResponse, _T("NewPortMappingDescription"));
	if (NULL != pulPortMappingLeaseDuration)
		*pulPortMappingLeaseDuration = (ULONG) atoi(CStringA(getProperty(strResponse, _T("NewLeaseDuration"))));

	return S_OK;
}

HRESULT	CUPnpNat::GetGenericPortMappingEntry(USHORT usIndex,					//[in]
								   CString *pstrRemoteHost,						//[out]
								   USHORT *pusExternalPort,						//[out]
								   CString *pstrProtocol,						//[out]
								   USHORT *pusInternalPort,						//[out]
								   CString *pstrInternalClient,					//[out]
								   bool *pbEnable,								//[out]
								   CString *pstrDescription)					//[out]
{
	CString		args;
	CString		strResponse;

	args.Empty();
	args.Append(GetArgString(_T("NewPortMappingIndex"), usIndex));

	HRESULT		hr;
	hr = InvokeCommand(_T("GetGenericPortMappingEntry"), args, strResponse);
	if (FAILED(hr))
		return hr;


	if (NULL != pstrRemoteHost)
		*pstrRemoteHost = getProperty(strResponse, _T("NewRemoteHost"));
	if (NULL != pusExternalPort)
		*pusExternalPort = (USHORT) atoi(CStringA(getProperty(strResponse, _T("NewExternalPort"))));
	if (NULL != pstrProtocol)
		*pstrProtocol = getProperty(strResponse, _T("NewProtocol"));
	if (NULL != pusInternalPort)
		*pusInternalPort = (USHORT) atoi(CStringA(getProperty(strResponse, _T("NewInternalPort"))));
	if (NULL != pstrInternalClient)
		*pstrInternalClient = getProperty(strResponse, _T("NewInternalClient"));
	if (NULL != pbEnable)
		*pbEnable = (bool) (atoi(CStringA(getProperty(strResponse, _T("NewEnabled")))) == 0 ? false : true);
	if (NULL != pstrDescription)
		*pstrDescription = getProperty(strResponse, _T("NewPortMappingDescription"));

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
//	Protected functions

HRESULT CUPnpNat::SOAP_action(CString addr, UINT16 port, const CString request, CString &response)
{
	char buffer[10240];

	const CStringA sa(request);
	int length = sa.GetLength();
	strcpy(buffer, (const char*)sa);

	UINT32 ip = inet_addr(CStringA(addr));
	struct sockaddr_in sockaddr;
	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(port);
	sockaddr.sin_addr.S_un.S_addr = ip;
	SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == s)
		return E_UNAT_CREATE_SOCKET_FAILED;

	if (! m_strBindAddress.IsEmpty())
	{
		sockaddr_in sa;
		sa.sin_family = AF_INET;
		sa.sin_addr.s_addr = inet_addr(CStringA(m_strBindAddress));
		sa.sin_port = 0;

		int nRet = bind(s, (SOCKADDR*)&sa, sizeof(sa));
		if (nRet == SOCKET_ERROR)
		{
			closesocket(s); // ignore return value - error close anyway
			return E_UNAT_SOCKET_BIND_FAILED;
		} 
	}

	BOOL nl = TRUE;
	setsockopt(s, SOL_SOCKET, SO_DONTLINGER, (const char *)&nl, sizeof(nl));

	if(connect(s, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) != 0)
		return E_UNAT_CREATE_SOCKET_FAILED;
	//ADDED by VC-fengwen 2007/08/24 <begin> : 仍通过异步处理，使处理回复内容时不用去处理http头里的content-length。
	u_long lv = 1;
	ioctlsocket(s, FIONBIO, &lv);
	//ADDED by VC-fengwen 2007/08/24 <end> : 仍通过异步处理，使处理回复内容时不用去处理http头里的content-length。

	//COMMENTED by VC-fengwen 2007/08/23 <begin> : 使用Sleep的方法收集tcp数据
	////ADDED by VC-fengwen 2007/08/23 <begin> : 为防止卡死，需要设置Timeout
	//setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char *)&m_iActionTimeoutMs, sizeof(int));
	//setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (const char *)&m_iActionTimeoutMs, sizeof(int));
	////ADDED by VC-fengwen 2007/08/23 <end> : 为防止卡死，需要设置Timeout
	//COMMENTED by VC-fengwen 2007/08/23 <end> : 使用Sleep的方法收集tcp数据

	send(s, buffer, length, 0);

	//MODIFIED by VC-fengwen 2007/08/23 <begin> : 接收流数据直到socket断开或超时。
		////MODIFIED by VC-fengwen 2007/08/23 <begin> : 根据同步Timeout来实现延时
		////int rlen = 0;
		////int	iSleepMs = 500;
		////for (int i = 0; rlen <= 0 && i < m_iActionTimeoutMs; i += iSleepMs)
		////{
		////	Sleep(iSleepMs);

		////	rlen = recv(s, buffer, sizeof(buffer), 0);
		////}

		//Sleep(m_iActionTimeoutMs);//ADDED by VC-fengwen 2007/08/23 : 由于是TCP连接，等待一段时间使数据收集完整

		//int rlen = 0;
		//rlen = recv(s, buffer, sizeof(buffer), 0);
		////MODIFIED by VC-fengwen 2007/08/23 <end> : 根据同步Timeout来实现延时

	int rlen = 0;
	int	iSleepMs = 50;
	char *pCurWritePos = buffer;
	int iWritedCount = 0;

	for (int i = 0; i < m_iActionTimeoutMs; i += iSleepMs)
	{
		// MODIFIED by by VC-nightsuns <begin> on 2007/11/08 : 解决程序退出的缓慢
		DWORD dwNow = ::GetTickCount();
		while( ::GetTickCount() - dwNow < (DWORD)iSleepMs ) 
		{
			if( CGlobalVariable::m_app_state == APP_STATE_RUNNING ) 
			{
				Sleep(10);
			} 
			else 
			{
				// 程序已经在退出了
				closesocket(s);

				return E_FAIL;
			}
		}

		//Sleep(iSleepMs);
	// MODIFIED by by VC-nightsuns <end> on 2007/11/08 : 解决程序退出的缓慢

		rlen = recv(s, pCurWritePos, sizeof(buffer) - iWritedCount, 0);
		
		if (0 == rlen)
			break;
		else if (rlen > 0)
		{
			pCurWritePos += rlen;
			iWritedCount += rlen;
		}

		if (IsLengthedHttpPacketComplete(buffer, iWritedCount))
			break;
	}
	rlen = iWritedCount;
	//MODIFIED by VC-fengwen 2007/08/23 <end> : 接收流数据直到socket断开或超时。



	closesocket(s);
	if (rlen <= 0)
		return E_UNAT_TIMEOUT;


	closesocket(s);
	if (rlen == SOCKET_ERROR) return E_UNAT_TIMEOUT;
	if (!rlen) return E_UNAT_TIMEOUT;

	if (IsHttpChunked(buffer,rlen))
		rlen = DecodeHttpChunked(buffer,rlen,10240);

	response = CString(CStringA(buffer, rlen));

	CString result;
	if (!parseHTTPResponse(response, result))
	{
		int		iResponseCode = -1;
		CString strErrorCode = getProperty(response, _T("errorCode"));
		if (!strErrorCode.IsEmpty())
			iResponseCode = atoi(CStringA(strErrorCode));

		SetLastActionErrorCode(iResponseCode);
		return E_UNAT_ACTION_HTTP_ERRORCODE;
	}

	return S_OK;
}

int CUPnpNat::SSDP_sendRequest(SOCKET s, UINT32 ip, UINT16 port, const CString& request)
{
	char buffer[10240];

	const CStringA sa(request);
	int length = sa.GetLength();
	strcpy(buffer, (const char*)sa);

	struct sockaddr_in sockaddr;
	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(port);
	sockaddr.sin_addr.S_un.S_addr = ip;

	return sendto(s, buffer, length, 0, (struct sockaddr *)&sockaddr, sizeof(sockaddr));
}


bool CUPnpNat::InternalSearch(int version, BOOL bUseDefaultGateway)
{
	if(version<=0)version = 1;
	m_version = version;

#define NUMBEROFDEVICES	3
	CString devices[][2] = {
		{UPNPPORTMAP1, _T("service")},
		{UPNPPORTMAP0, _T("service")},
		{_T("InternetGatewayDevice"), _T("device")},
	};

	SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);
	if (INVALID_SOCKET == s)
		return false;
	
	BOOL nl = TRUE;
	setsockopt(s, SOL_SOCKET, SO_DONTLINGER, (const char *)&nl, sizeof(nl));

	if (! m_strBindAddress.IsEmpty())
	{
		sockaddr_in sa;
		sa.sin_family = AF_INET;
		sa.sin_addr.s_addr = inet_addr(CStringA(m_strBindAddress));
		sa.sin_port = 0;
		
		int nRet = bind(s, (SOCKADDR*)&sa, sizeof(sa));
		if (nRet == SOCKET_ERROR)
		{
			closesocket(s); // ignore return value - error close anyway
			return false;
		} 
	}

	UINT32	uReqIp = 0;
	UINT32	uDefGW = 0;
	if (bUseDefaultGateway && GetDefaultGateway(uDefGW))
		uReqIp = uDefGW;
	else
		uReqIp = UPNPADDR;


	int	iSleepTime = 1000;

	//MODIFIED by VC-fengwen 2007/08/24 <begin> : udp设置timeout似乎无效，仍使用sleep来控置timeout
		////ADDED by VC-fengwen 2007/08/23 <begin> : 为防止卡死，需要设置Timeout
		//setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char *)&iSleepTime, sizeof(int));
		//setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (const char *)&iSleepTime, sizeof(int));
		////ADDED by VC-fengwen 2007/08/23 <end> : 为防止卡死，需要设置Timeout
	u_long lv = 1;
	ioctlsocket(s, FIONBIO, &lv);
	//MODIFIED by VC-fengwen 2007/08/24 <end> : udp设置timeout似乎无效，仍使用sleep来控置timeout


	int rlen = 0;
	char buffer[10240];
	for (int i = 0; rlen <= 0 && i < SEARCH_TIMEOUT_MS; i += iSleepTime) {
		if (i % 3000 == 0) {		//在没收到回复的情况下，每3秒广播一次请求。
			for (int i=0; i<NUMBEROFDEVICES; i++) {
				m_name.Format(_T("%s%s:%s:%d"), URNPREFIX, devices[i][1], devices[i][0], version);
				CString request;
				request.Format(_T("M-SEARCH * HTTP/1.1\r\nHOST: 239.255.255.250:1900\r\nMAN: \"ssdp:discover\"\r\nMX: %d\r\nST: %s\r\n\r\n"),
					6, m_name);

				SSDP_sendRequest(s, uReqIp, UPNPPORT, request);
			}
		}

		// MODIFIED by by VC-nightsuns <begin> on 2007/11/08 : 解决程序退出的缓慢
		DWORD dwNow = ::GetTickCount();
		while( ::GetTickCount() - dwNow < (DWORD)iSleepTime ) 
		{
			if( CGlobalVariable::m_app_state == APP_STATE_RUNNING ) 
			{
				Sleep(10);
			}
			else 
			{
				// 程序已经在退出了
				closesocket(s);

				return false;
			}
		}

//		Sleep(iSleepTime);

		// MODIFIED by by VC-nightsuns <end> on 2007/11/08 : 解决程序退出的缓慢

		rlen = recv(s, buffer, sizeof(buffer), 0);
	}
	closesocket(s);

	if(rlen <= 0)
		return false;

	if (IsHttpChunked(buffer,rlen))
		rlen = DecodeHttpChunked(buffer,rlen,10240);

	CString response = CString(CStringA(buffer, rlen));
	CString result;
	if (!parseHTTPResponse(response, result)) return false;

	for (int d=0; d<NUMBEROFDEVICES; d++) {
		m_name.Format(_T("%s%s:%s:%d"), URNPREFIX, devices[d][1], devices[d][0], version);
		if (result.Find(m_name) >= 0) {
			for (int pos = 0;;) {
				CString line = result.Tokenize(_T("\r\n"), pos);
				if (line.IsEmpty()) return false;
				CString name = line.Mid(0, 9);
				name.MakeUpper();
				if (name == _T("LOCATION:")) {
					line.Delete(0, 9);
					m_description = line;
					m_description.Trim();
					return GetDescription();
				}
			}
		}
	}

	return false;
}

bool CUPnpNat::GetDescription()
{
	if(!Valid())return false;
	CString post, host, addr;
	int port = 0;
	addr = NGetAddressFromUrl(m_description, post, host, port);
	if(addr.IsEmpty())return false;
	CString request = CString(_T("GET ")) + post + _T(" HTTP/1.1\r\nHOST: ") + host + _T("\r\nACCEPT-LANGUAGE: en\r\n\r\n");
	CString response;
	if (FAILED(SOAP_action(addr, (UINT16)port, request, response))) return false;
	CString result;
	if (!parseHTTPResponse(response, result)) return false;

	m_friendlyname = getProperty(result, _T("friendlyName"));
	m_modelname = getProperty(result, _T("modelName"));
	m_baseurl = getProperty(result, _T("URLBase"));
	if(m_baseurl.IsEmpty())m_baseurl = CString(_T("http://")) + host + _T("/");
	if(m_baseurl[m_baseurl.GetLength() - 1]!='/')m_baseurl += _T("/");

	CString serviceType = _T("<serviceType>") + m_name + _T("</serviceType>");
	int pos = result.Find(serviceType);
	if (pos >= 0) {
		result.Delete(0, pos + serviceType.GetLength());
		pos = result.Find(_T("</service>"));
		if (pos >= 0) {
			result = result.Mid(0, pos);
			m_controlurl = getProperty(result, _T("controlURL"));
			if (!m_controlurl.IsEmpty() && m_controlurl[0] == '/') {
				m_controlurl = m_baseurl + m_controlurl.Mid(1);
			}
		}
	}

	return isComplete();
}

CString CUPnpNat::GetProperty(const CString& name, CString& response)
{
	if (!isComplete())return CString();
	CString post, host, addr;
	int port = 0;
	addr = NGetAddressFromUrl(m_controlurl, post, host, port);
	if(addr.IsEmpty())return CString();
	CString cnt;
	CString psr;
	cnt.Append(_T("<s:Envelope\r\n    xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\"\r\n    "));
	cnt.Append(_T("s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">\r\n  <s:Body>\r\n    <u:"));
	cnt.Append(name);
	cnt.Append(_T(" xmlns:u=\""));
	cnt.Append(m_name);
	cnt.Append(_T("\">\r\n    </u:"));
	cnt.Append(name);
	cnt.Append(_T(">\r\n  </s:Body>\r\n</s:Envelope>\r\n\r\n"));
	psr.Append(_T("POST "));
	psr.Append(post);
	psr.Append(_T(" HTTP/1.1\r\nHOST: "));
	psr.Append(host);
	psr.Append(_T("\r\nContent-Length: "));
	psr.Append(getString(CStringA(cnt).GetLength()));
	psr.Append(_T("\r\nContent-Type: text/xml; charset=\"utf-8\"\r\nSOAPAction: \""));
	psr.Append(m_name);
	psr.Append(_T("#"));
	psr.Append(name);
	psr.Append(_T("\"\r\n\r\n"));
	psr.Append(cnt);

	CString request = psr;
	if (FAILED(SOAP_action(addr, (UINT16)port, request, response))) return CString();
	CString result;
	if (!parseHTTPResponse(response, result)) return CString();

	return getProperty(result, response);
}

HRESULT CUPnpNat::InvokeCommand(const CString& name, const CString& args, CString &strResponse)
{
	if(!isComplete())return false;
	CString post, host, addr;
	int port = 0;
	addr = NGetAddressFromUrl(m_controlurl, post, host, port);
	if(addr.IsEmpty())return false;
	CString cnt;
	CString psr;
	cnt.Append(_T("<?xml version=\"1.0\"?><s:Envelope\r\n    xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\"\r\n    "));
	cnt.Append(_T("s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">\r\n  <s:Body>\r\n    <u:"));
	cnt.Append(name);
	cnt.Append(_T(" xmlns:u=\""));
	cnt.Append(m_name);
	cnt.Append(_T("\">\r\n"));
	cnt.Append(args);
	cnt.Append(_T("    </u:"));
	cnt.Append(name);
	cnt.Append(_T(">\r\n  </s:Body>\r\n</s:Envelope>\r\n\r\n"));
	psr.Append(_T("POST "));
	psr.Append(post);
	psr.Append(_T(" HTTP/1.1\r\nHOST: "));
	psr.Append(host);
	psr.Append(_T("\r\nContent-Length: "));
	psr.Append(getString(CStringA(cnt).GetLength()));
	psr.Append(_T("\r\nContent-Type: text/xml; charset=\"utf-8\"\r\nSOAPAction: \""));
	psr.Append(m_name);
	psr.Append(_T("#"));
	psr.Append(name);
	psr.Append(_T("\"\r\n\r\n"));
	psr.Append(cnt);

	strResponse.Empty();

	HRESULT			hr;
	CString response;
	CString request = psr;
	hr = SOAP_action(addr, (UINT16)port, request, response);
	if (FAILED(hr))
		return hr;

	strResponse = response;

	CString result;
	if (!parseHTTPResponse(response, result))
		return E_UNAT_ACTION_HTTP_ERRORCODE;


	return S_OK;
}

struct chunked_arg{
	char * buffer;
	size_t size;
	int pos;
}ch_arg;

bool IsHttpChunked(char * buffer,int len)
{
	const char STR_HTTP_CHUNKED[] = "Transfer-Encoding: chunked";
	const int  STRLEN_CHUNKED = sizeof(STR_HTTP_CHUNKED) / sizeof(char);
	char * FindPos = NULL;
	if (len  > STRLEN_CHUNKED)
	{
		FindPos = StrStrIA(buffer,STR_HTTP_CHUNKED);
		if (FindPos)
			return true;
	}
	return false;
}

int write_body_data(char * buf,size_t len,void * arg)
{
	chunked_arg * ch = (chunked_arg *) arg;
	if (!ch)
		return CHUNKE_WRITE_ERROR;

	if (len < (ch->size - ch->pos))
	{
		memcpy(ch->buffer + ch->pos,buf,len);
		ch->pos += len;
	}

	return CHUNKE_OK;
}

int DecodeHttpChunked(char *buffer,int len,size_t size)
{
	const char STR_DOUBLE_NEWLINE[] = "\r\n\r\n";
	const int STRLEN_DOUBLE_NEWLINE = sizeof(STR_DOUBLE_NEWLINE) / sizeof(char) - 1;

	char * FindPos1 = NULL,*FindPos2;
	FindPos1 = StrStrIA(buffer,STR_DOUBLE_NEWLINE);
	if (!FindPos1)
		return false;

/*
	FindPos2 = StrStrIA(FindPos1 + STRLEN_DOUBLE_NEWLINE,STR_DOUBLE_NEWLINE);
	if (!FindPos2)
		return false;
*/
	int chunked_len = len - (FindPos1 +  STRLEN_DOUBLE_NEWLINE - buffer);
	char * pChunked = (char * ) malloc(chunked_len);
	if (!pChunked)
		return false;

	memcpy(pChunked,FindPos1 +  STRLEN_DOUBLE_NEWLINE,chunked_len);

	ch_arg.buffer = buffer;
	ch_arg.size = size;
	ch_arg.pos = 0;

	decode_chunked_init(write_body_data,&ch_arg,0,0,0);
	size_t wrote = 0;

	CHUNK_CODE cc = decode_chunked(pChunked,chunked_len,&wrote);

	decode_chunked_cleanup();
	free(pChunked);

	if (cc != CHUNKE_STOP )
		return 0;

	ASSERT( wrote == ch_arg.pos);

	return wrote;
}

BOOL CUPnpNat::IsLengthedHttpPacketComplete(const char *packet, int len)
{
	const char STR_CONTENT_LENGTH[] = "Content-Length:";
	const int STRLEN_CONTENT_LENGTH = sizeof(STR_CONTENT_LENGTH) / sizeof(char) - 1;
	const char STR_DOUBLE_NEWLINE[] = "\r\n\r\n";
	const int STRLEN_DOUBLE_NEWLINE = sizeof(STR_DOUBLE_NEWLINE) / sizeof(char) - 1;

	char *pContLenPos = NULL;
	pContLenPos = StrStrIA(packet, STR_CONTENT_LENGTH);
	if (NULL == pContLenPos)
		return FALSE;
	
	char *pNewLinePos = NULL;
	pNewLinePos = strstr(pContLenPos, "\r\n");
	if (NULL == pNewLinePos)
		return FALSE;

	// Get number of content length <begin>
	int	iContentLen = 0;
	char *pLenStartPos = pContLenPos + STRLEN_CONTENT_LENGTH;
	char *pLenEndPos = pNewLinePos;
	size_t nBufSize = pLenEndPos - pLenStartPos + 1;
	char *szLength = new char[nBufSize];
	
	memcpy(szLength, pLenStartPos, nBufSize - 1);
	szLength[nBufSize - 1] = 0;
	iContentLen = atoi(szLength);
	
	delete[] szLength;
	szLength = NULL;
	// Get number of content length <end>
	
	int iHeadLen = 0;
	char *pDoubleNewLinePos = NULL;
	pDoubleNewLinePos = (char*) strstr(packet, STR_DOUBLE_NEWLINE);
	if (NULL == pDoubleNewLinePos)
		return FALSE;

	iHeadLen = pDoubleNewLinePos - packet + STRLEN_DOUBLE_NEWLINE;

	if (len >= iHeadLen + iContentLen)
		return TRUE;
	else
		return FALSE;
}
