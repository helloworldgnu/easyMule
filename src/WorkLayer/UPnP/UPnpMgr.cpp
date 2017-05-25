/*
 * $Id: UPnpMgr.cpp 4789 2008-02-14 08:59:23Z fengwen $
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
#include ".\upnpmgr.h"

#include <WinSock2.h>
#include "Preferences.h"
#include "IpHlpApi.h"
#include "emule.h"




CUPnpMgr::CUPnpMgr(void)
{
	m_slocalIP.Empty();
	m_uLocalIP = 0;
}

CUPnpMgr::~CUPnpMgr(void)
{
	CleanupMappingArr();
}

void CUPnpMgr::SetBindAddress(LPCTSTR lpszBindAddress)
{
	CSingleLock		localLock(&m_cs, TRUE);
	m_nat.SetBindAddress(lpszBindAddress);
}

void CUPnpMgr::SetActionTimeout(int iActionTimeoutMs)
{
	CSingleLock		localLock(&m_cs, TRUE);
	m_nat.SetActionTimeout(iActionTimeoutMs);
}

int CUPnpMgr::GetActionTimeout()
{
	CSingleLock		localLock(&m_cs, TRUE);
	return m_nat.GetActionTimeout();
}

HRESULT CUPnpMgr::AddNATPortMapping(CUPnpNatMapping &mapping, BOOL bTryRandom)
{
	CSingleLock		localLock(&m_cs, TRUE);
	
	if(!IsLANIP(GetLocalIP()))
		return E_UNAT_NOT_IN_LAN;

	if (FAILED(m_nat.SearchDevice()))
		return E_UNAT_CANNOT_FIND_ROUTER;

	
	HRESULT		hr;
	CString		strDesc;
	int			i;

	if(0 == mapping.m_wExternalPort)
		mapping.m_wExternalPort = mapping.m_wInternalPort;

	mapping.m_strInternalClient = GetLocalIPStr();

	for (i = 0; i < RANDOM_RETRY_TIMES; i++)
	{
		strDesc.Format(_T("%s [%s: %u]"), mapping.m_strDescription, mapping.m_strProtocol, mapping.m_wExternalPort);

		hr = m_nat.AddPortMapping(CString(_T("")),
									mapping.m_wExternalPort, mapping.m_strProtocol,
									mapping.m_wInternalPort, mapping.m_strInternalClient,
									strDesc);


		switch(hr)
		{
		case S_OK:
			{
				CString		strInternalClient;
				hr = m_nat.GetSpecificPortMappingEntry(CString(_T("")),
														mapping.m_wExternalPort, mapping.m_strProtocol,
														NULL, &strInternalClient, NULL, NULL, NULL);
				if (SUCCEEDED(hr)
					&& strInternalClient == mapping.m_strInternalClient)			//	看PortMapping是否真的加进去了。
				{
					CUPnpNatMappingKey key;
					key.m_strRemoteHost.Empty();
					key.m_usExternalPort = mapping.m_wExternalPort;
					key.m_strProtocol = mapping.m_strProtocol;
					m_setAddedMapping.insert(key);

					WriteAddedMappingToFile();

					return S_OK;
				}
				else											//	如果添加请求成功返回而又不在列表中，则有可能是列表已满。
					return E_UNAT_ENTRY_MAYBE_FULL;

				break;
			}
		case E_UNAT_ACTION_HTTP_ERRORCODE:
			{
				if (718 == m_nat.GetLastActionErrorCode() && bTryRandom)
					mapping.m_wExternalPort = (WORD)(2049 + (65535 - 2049) * 1 *rand() / (RAND_MAX + 1));
				else
					return E_UNAT_ACTION_HTTP_ERRORCODE;
				break;
			}
		case E_UNAT_TIMEOUT:
			{
				//	为防止可能出现“实际加上了”但等待回复超时的情况，凡是执行了操作的都记下来，以便之后尝试清除一下。
				
				CUPnpNatMappingKey key;
				key.m_strRemoteHost.Empty();
				key.m_usExternalPort = mapping.m_wExternalPort;
				key.m_strProtocol = mapping.m_strProtocol;
				m_setAddedMapping.insert(key);

				WriteAddedMappingToFile();

				return E_UNAT_TIMEOUT;		//	超时未返回则直接返回，而不去重试。（因为此刻可能实际上添加成功，而不停去添加，可能造成路由器表项充满。）
				break;
			}
		default:
			return hr;
		}
	}

	return E_UNAT_REACH_RAND_PORT_RETRY_TIMES;
}

void CUPnpMgr::RemoveNATPortMapping(const CUPnpNatMappingKey &mappingKey)
{
	CSingleLock		localLock(&m_cs, TRUE);
	
	if (FAILED(m_nat.SearchDevice()))
		return;

	HRESULT		hr;
	hr = m_nat.DeletePortMapping(mappingKey.m_strRemoteHost, mappingKey.m_usExternalPort, mappingKey.m_strProtocol);

	if (SUCCEEDED(hr)
		|| 714 == GetLastActionErrorCode())	// No such entry in array
	{
		m_setAddedMapping.erase(mappingKey);
		WriteAddedMappingToFile();
	}
}

void CUPnpMgr::CleanupAllEverMapping(void)
{
	CSingleLock		localLock(&m_cs, TRUE);

	//ADDED by VC-fengwen 2007/08/23 <begin> : 没有映射项的话则不用执行下去。
	if (0 == m_setAddedMapping.size())
		return;
	//ADDED by VC-fengwen 2007/08/23 <end> : 没有映射项的话则不用执行下去。

	if (FAILED(m_nat.SearchDevice()))
		return;

	HRESULT				hr;
	set<CUPnpNatMappingKey>::iterator	it;
	set<CUPnpNatMappingKey>::iterator	itNext;
	int					iOpCount;

	iOpCount = 0;
	it = m_setAddedMapping.begin();
	while (it != m_setAddedMapping.end())
	{
		itNext = it;
		itNext++;

		hr = m_nat.DeletePortMapping(it->m_strRemoteHost, it->m_usExternalPort, it->m_strProtocol);
		if (SUCCEEDED(hr)
			|| 714 == GetLastActionErrorCode()	// No such entry in array
			|| 402 == GetLastActionErrorCode()) // Some router will return 402 for some reason. Delete it as well anyway.
		{
			m_setAddedMapping.erase(it);
		}

		iOpCount++;
		if (iOpCount >= 10)		//	防止一些路由器不能一次处理太多请求，删除操作最多不大于10次，由于一般启动eMule只会开2个端口，这个数值应该是够的。
			break;

		it = itNext;
	}

	WriteAddedMappingToFile();
}


BOOL CUPnpMgr::CleanedFillupBug()
{
	CSingleLock		localLock(&m_cs, TRUE);
	
	if (FAILED(m_nat.SearchDevice()))
		return FALSE;


	HRESULT		hr;
	USHORT		usIndex;
	USHORT		usExternalPort;
	CString		strProtocol;
	CString		strDesc;
	CString		strInternalClient;
	CString		strLocalIP;
	
	CArray<CUPnpNatMapping, const CUPnpNatMapping&>		arrMapping;
	CUPnpNatMapping										mapping;

	strLocalIP = GetLocalIPStr();

	for (usIndex = 0; usIndex < 128; usIndex++)		//	最多查询128项，以防止有些路由器可能会一直返回成功。
	{
		hr = m_nat.GetGenericPortMappingEntry(usIndex, NULL, &usExternalPort, &strProtocol,
												NULL, &strInternalClient, NULL, &strDesc);

		if (-1 != strDesc.Find(_T("eMule (")))			//	描述里包含指定字样
		{
			if (strInternalClient == strLocalIP)	//	是本机添加的映射
				m_nat.DeletePortMapping(CString(_T("")),usExternalPort, strProtocol);
			else
			{
				mapping.m_strInternalClient = strInternalClient;
				mapping.m_wExternalPort = usExternalPort;
				mapping.m_strProtocol = strProtocol;	
				arrMapping.Add(mapping);
			}
		}

		if (FAILED(hr))		//	返回Failed结果，表示已经全部取完了。
			break;
	}

	
	//	如果同一ip做的eMule映射超过3项，则很有可能是以前的bug所致。则清除。<begin>
	int			i;
	int			iMappingCount;
	CString		strTestIp;

	while (! arrMapping.IsEmpty())
	{
		strTestIp = arrMapping[0].m_strInternalClient;
		iMappingCount = 0;
		for (i = 0; i < arrMapping.GetCount(); i++)
		{
			if (strTestIp == arrMapping[i].m_strInternalClient)
				iMappingCount++;
		}

		for (i = arrMapping.GetCount() - 1; i >= 0; i--)
		{
			if (strTestIp == arrMapping[i].m_strInternalClient)
			{
				if (iMappingCount > 3)			
				{
					m_nat.DeletePortMapping(CString(_T("")),
											arrMapping[i].m_wExternalPort,
											arrMapping[i].m_strProtocol);
				}

				arrMapping.RemoveAt(i);
			}

		}
	}
	//	如果同一ip做的eMule映射超过3项，则很有可能是以前的bug所致。则清除。<end>


	return TRUE;


}


/////////////////////////////////////////////////////////////////////////////////
// Returns a CString with the local IP in format xxx.xxx.xxx.xxx
/////////////////////////////////////////////////////////////////////////////////
CString CUPnpMgr::GetLocalIPStr()
{
	CSingleLock		localLock(&m_cs, TRUE);

	if(m_slocalIP.IsEmpty())
		InitLocalIP();
	
	return m_slocalIP;
}

/////////////////////////////////////////////////////////////////////////////////
// Returns the Local IP
/////////////////////////////////////////////////////////////////////////////////
DWORD CUPnpMgr::GetLocalIP()
{
	CSingleLock		localLock(&m_cs, TRUE);

	if(m_uLocalIP == 0)
		InitLocalIP();

	return m_uLocalIP;
}

CString CUPnpMgr::ResultCode2String(HRESULT hr)
{
	CString		str;

	if (FAILED(hr))
		str = GetResString(IDS_FAILED);
	else
		str.Empty();

	switch(hr)
	{
	case S_OK:
		str = str + GetResString(IDS_SUCCEED);
		break;
	case E_UNAT_BAD_ARGUMENTS:
		str = str + GetResString(IDS_PARAMETER_ERROR);
		break;
	case E_UNAT_NOT_IN_LAN:
		str = str + GetResString(IDS_PUBLIC_IP);
		break;
	case E_UNAT_CANNOT_FIND_ROUTER:
		str = str + GetResString(IDS_CANNOT_FIND_ROUTER);
		break;
	case E_UNAT_TIMEOUT:
		str = str + GetResString(IDS_TIME_OUT);
		break;
	case E_UNAT_ENTRY_MAYBE_FULL:
		str = str + GetResString(IDS_FULL_MAPPING);
		break;
	case E_UNAT_REACH_RAND_PORT_RETRY_TIMES:
		str = str + GetResString(IDS_REACH_RETRY_TIMES);
		break;
	case E_UNAT_CREATE_SOCKET_FAILED:
		str = str + GetResString(IDS_CREATE_SOCKET_FAILED);
		break;
	case E_UNAT_SOCKET_BIND_FAILED:
		str = str + GetResString(IDS_SOCKET_BIND_FAILED);
		break;
	case E_UNAT_ACTION_HTTP_ERRORCODE:
		str = str + GetResString(IDS_ACTION_ERROR);
		break;
	case E_UNAT_UNKNOWN_ERROR:
	default:
		str = str + GetResString(IDS_UNKNOWN_ERROR);
		break;
	}

	return str;
}

CString	CUPnpMgr::Result2String(HRESULT hr, DWORD dwActionErrorCode)
{
	CString	strOut;
	strOut = ResultCode2String(hr);

	CString strErrorCode;
	if (E_UNAT_ACTION_HTTP_ERRORCODE == hr)
	{
		switch(dwActionErrorCode)
		{
		case 718:
			strErrorCode = GetResString(IDS_EXTERNAL_MAPPING);
			break;
		default:
			strErrorCode.Empty();
			break;
		}

		CString	str;
		str.Format(_T("(#%d : %s)"), dwActionErrorCode, strErrorCode);
		strOut += str;
	}

	return strOut;
}

/////////////////////////////////////////////////////////////////////////////////
// Returns true if nIP is a LAN ip, false otherwise
/////////////////////////////////////////////////////////////////////////////////
bool CUPnpMgr::IsLANIP(DWORD nIP){
	// filter LAN IP's
	// -------------------------------------------
	// 0.*
	// 10.0.0.0 - 10.255.255.255  class A
	// 172.16.0.0 - 172.31.255.255  class B
	// 192.168.0.0 - 192.168.255.255 class C

	unsigned char nFirst = (unsigned char)nIP;
	unsigned char nSecond = (unsigned char)(nIP >> 8);

	if (nFirst==192 && nSecond==168) // check this 1st, because those LANs IPs are mostly spreaded
		return true;

	if (nFirst==172 && nSecond>=16 && nSecond<=31)
		return true;

	if (nFirst==0 || nFirst==10)
		return true;

	return false; 
}

/////////////////////////////////////////////////////////////////////////////////
// Initializes m_localIP variable, for future access to GetLocalIP()
/////////////////////////////////////////////////////////////////////////////////
void CUPnpMgr::InitLocalIP()
{
	CSingleLock		localLock(&m_cs, TRUE);

#ifndef _DEBUG
	try
#endif
	{
		PIP_ADAPTER_INFO pAdapterInfo = NULL, pCurAdapterInfo;
		ULONG ulOutBufLen = 0;
		DWORD dwRetVal = 0;

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
			UINT32 ip;
			do {
				ip = inet_addr(pCurAdapterInfo->GatewayList.IpAddress.String);
				m_slocalIP = pCurAdapterInfo->IpAddressList.IpAddress.String;
				m_uLocalIP = inet_addr(pCurAdapterInfo->IpAddressList.IpAddress.String);
			} while(ip == 0 && (pCurAdapterInfo = pCurAdapterInfo->Next) != NULL);
		}

		free(pAdapterInfo);
		pAdapterInfo = NULL;
	}
#ifndef _DEBUG
	catch(...){
		m_slocalIP = _T("");
		m_uLocalIP = 0;
	}
#endif
}

void CUPnpMgr::CleanupMappingArr()
{
	CSingleLock		localLock(&m_cs, TRUE);

	m_setAddedMapping.clear();
}

void CUPnpMgr::WriteAddedMappingToFile(void)
{
	CSingleLock		localLock(&m_cs, TRUE);

	CString		strDatFile;
	strDatFile.Format(_T("%sUPnp.dat"), thePrefs.GetMuleDirectory(EMULE_CONFIGDIR));

	CFile		file;
	if (!file.Open(strDatFile, CFile::modeCreate | CFile::modeReadWrite))
		return;

	try
	{
		CArchive	ar(&file, CArchive::store);

		int						iCount;

		iCount = m_setAddedMapping.size();
		ar << iCount;

		set<CUPnpNatMappingKey>::iterator	it;
		for (it = m_setAddedMapping.begin(); it != m_setAddedMapping.end(); it++)
		{
            CUPnpNatMappingKey      key(*it);
            key.Serialize(ar);
            //it->Serialize(ar);
		}
		ar.Close();
	}
	catch (CArchiveException *e)
	{
		e->Delete();
		e = NULL;
	}

	file.Close();
}

void CUPnpMgr::ReadAddedMappingFromFile(void)
{
	CSingleLock		localLock(&m_cs, TRUE);

	CleanupMappingArr();
	
	CString		strDatFile;
	strDatFile.Format(_T("%sUPnp.dat"), thePrefs.GetMuleDirectory(EMULE_CONFIGDIR));

	CFile		file;
	try
	{
		if (!file.Open(strDatFile, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeReadWrite))
			return;

		if (0 == file.GetLength())
		{
			file.Close();
			return;
		}
	}
	catch (CFileException *e)
	{
		e->Delete();
		e = NULL;
		
		file.Close();
		return;
	}


	try
	{
		CArchive	ar(&file, CArchive::load);

		int		iCount;
		ar >> iCount;

		int						i;
		CUPnpNatMappingKey		key;
		for (i = 0; i < iCount; i++)
		{
			key.Serialize(ar);
			m_setAddedMapping.insert(key);
		}
		ar.Close();
	}
	catch (CArchiveException *e)
	{
		e->Delete();
		e = NULL;
	}
	
	file.Close();

}
