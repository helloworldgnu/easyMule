/*
 * $Id: UPnpNat.h 4483 2008-01-02 09:19:06Z soarchin $
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


#include <WinSock2.h>
#include "UPnpError.h"

class CUPnpNat
{
public:
	CUPnpNat(void);
	~CUPnpNat(void);

	enum {SEARCH_TIMEOUT_MS = 15000,
			DEF_ACTION_TIMEOUT_MS = 9000};

	void SetBindAddress(LPCTSTR lpszBindAddress){m_strBindAddress = lpszBindAddress;}
	void SetActionTimeout(int iActionTimeoutMs = DEF_ACTION_TIMEOUT_MS){m_iActionTimeoutMs = iActionTimeoutMs;}
	int	 GetActionTimeout(){return m_iActionTimeoutMs;}


	HRESULT	SearchDevice(BOOL bUseDefaultGateway = TRUE);


	HRESULT	AddPortMapping(LPCTSTR lpszRemoteHost,
							USHORT usExternalPort, LPCTSTR lpszPortMappingProtocol,
							USHORT usInternalPort, LPCTSTR lpszInternalClient,
							LPCTSTR lpszPortMappingDescription = NULL,
							BOOL bPortMappingEnabled = TRUE,
							ULONG ulPortMappingLeaseDuration = 0);

	HRESULT	DeletePortMapping(LPCTSTR lpszRemoteHost,
							USHORT usExternalPort,
							LPCTSTR lpszPortMappingProtocol);


	HRESULT	GetSpecificPortMappingEntry(LPCTSTR lpszRemoteHost,					//[in]
										USHORT usExternalPort,					//[in]
										LPCTSTR lpszPortMappingProtocol,		//[in]
										USHORT *pusInternalPort,				//[out]
										CString *pstrInternalClient,			//[out]
										bool *pbEnable,							//[out]
										CString *pstrDescription,				//[out]
										ULONG *pulPortMappingLeaseDuration);	//[out]

	
	HRESULT	GetGenericPortMappingEntry(USHORT usIndex,					//[in]
										CString *pstrRemoteHost,		//[out]
										USHORT *pusExternalPort,		//[out]
										CString *pstrProtocol,			//[out]
										USHORT *pusInternalPort,		//[out]
										CString *pstrInternalClient,	//[out]
										bool *pbEnable,					//[out]
										CString *pstrDescription);		//[out]

	DWORD	GetLastActionErrorCode(){return m_dwLastErrorCode;}

protected:
	void	SetLastActionErrorCode(DWORD dwLastErrorCode){m_dwLastErrorCode = dwLastErrorCode;}

	HRESULT SOAP_action(CString addr, UINT16 port, const CString request, CString &response);
	int SSDP_sendRequest(SOCKET s, UINT32 ip, UINT16 port, const CString& request);

	bool		InternalSearch(int version, BOOL bUseDefaultGateway = FALSE);
	bool		GetDescription();
	CString		GetProperty(const CString& name, CString& response);
	bool		isComplete() const { return !m_controlurl.IsEmpty(); }
	bool		Valid()const{return (/*!m_name.IsEmpty()&&*/!m_description.IsEmpty());}

	HRESULT		InvokeCommand(const CString& name, const CString& args, CString &strResponse);

	static BOOL IsLengthedHttpPacketComplete(const char *packet, int len);
protected:
	CString		m_strBindAddress;
	int			m_iActionTimeoutMs;

	DWORD		m_dwLastErrorCode;

	BOOL		m_isSearched;

	int			m_version;
	CString		m_devicename;
	CString		m_name;
	CString		m_description;
	CString		m_baseurl;
	CString		m_controlurl;
	CString		m_friendlyname;
	CString		m_modelname;

};
