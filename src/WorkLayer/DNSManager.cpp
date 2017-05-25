/*
 * $Id: DNSManager.cpp 6769 2008-08-21 08:54:29Z huby $
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
#include "DNSManager.h"
#include "GlobalVariable.h"
#include "DownloadQueue.h"
#include "PartFile.h"

/////////////////////////////////////////////////////////////////////////////////////////////

#define WM_DNSRESOLVED		(WM_USER + 0x101)	// does not need to be placed in "UserMsgs.h"

/////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CDNSResolveWnd, CWnd)
	ON_MESSAGE(WM_DNSRESOLVED, OnDNSResolved)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////////////////////

CDNSResolveWnd::CDNSResolveWnd()
{

}

CDNSResolveWnd::~CDNSResolveWnd()
{
	while (!m_HostEntryList.IsEmpty())
	{
		delete m_HostEntryList.RemoveHead();
	}
}

void CDNSResolveWnd::AddToResolve(const uchar* fileid, LPCSTR pszHostname, uint16 port, LPCTSTR pszURL, LPCTSTR lpszRefer/* = NULL*/,CPartFile* pPartFile )
{
	bool bResolving = !m_HostEntryList.IsEmpty();

	if (fileid != NULL) // VC-SearchDream[2007-03-19]: for HTTP and FTP Direct DownLoad
	{
		// double checking
		//带http的ed2k的情况没有考虑，为不影响大体的架构，先这样改
// 		if (!CGlobalVariable::downloadqueue->GetFileByID(fileid))
// 		{	
// 			return;
// 		}
	}

	Hostname_Entry* entry = new Hostname_Entry;

	if (fileid != NULL)
	{
		md4cpy(entry->fileid, fileid);
	}
	else // VC-SearchDream[2007-03-19]: for HTTP and FTP Direct DownLoad
	{
		md4clr(entry->fileid);
	}

	entry->strHostname	= pszHostname;
	entry->port			= port;
	entry->strURL		= pszURL;
	entry->strRefer		= lpszRefer;		
	entry->pPartFile	= pPartFile;
	m_HostEntryList.AddTail(entry);

	if (bResolving)
	{
		return;
	}

	memset(m_aucHostnameBuffer, 0, sizeof(m_aucHostnameBuffer));
	
	if (WSAAsyncGetHostByName(m_hWnd, WM_DNSRESOLVED, entry->strHostname, m_aucHostnameBuffer, sizeof m_aucHostnameBuffer) != 0)
	{
		return;
	}

	m_HostEntryList.RemoveHead();
	
	delete entry;
}
LRESULT CDNSResolveWnd::OnDNSResolved(WPARAM /*wParam*/, LPARAM lParam)
{
	try
	{
		if (m_HostEntryList.IsEmpty())
		{
			return TRUE;
		}

		Hostname_Entry* resolved = m_HostEntryList.RemoveHead();

		if (WSAGETASYNCERROR(lParam) == 0)
		{
			int iBufLen = WSAGETASYNCBUFLEN(lParam);

			if (iBufLen >= sizeof(HOSTENT))
			{
				LPHOSTENT pHost = (LPHOSTENT)m_aucHostnameBuffer;
				if (pHost->h_length == 4 && pHost->h_addr_list && pHost->h_addr_list[0])
				{

					uint32 nIP;
					CPartFile* pPartfile;

					for (int i = 0; pHost->h_addr_list[i] != NULL; i++)
					{
						nIP= ((LPIN_ADDR)(pHost->h_addr_list[i]))->s_addr;
#ifdef _DEBUG_PEER
						TRACE ( _T("URL(%s) Resolved to:%s \n"),resolved->strURL,ipstr(nIP) );
#endif
						if ( isnulmd4(resolved->fileid) )// for HTTP and FTP DownLoad
						{	
							pPartfile = resolved->pPartFile;
							if( CGlobalVariable::downloadqueue->IsPartFile(pPartfile) ) 
								pPartfile->AddSource(resolved->strURL, nIP, resolved->strRefer);
						}
						else
						{
							pPartfile = CGlobalVariable::downloadqueue->GetFileByID(resolved->fileid);						
							if (pPartfile)
							{
								if (resolved->strURL.IsEmpty())
								{
									CSafeMemFile sources(1+4+2);
									sources.WriteUInt8(1);
									sources.WriteUInt32(nIP);
									sources.WriteUInt16(resolved->port);
									sources.SeekToBegin();
									pPartfile->AddSources(&sources,0,0, false);
								}
								else
								{
									pPartfile->AddSource(resolved->strURL, nIP, resolved->strRefer);
								}
							}
						}//else
					}
				}
			}
		}
		else
		{
			// VC-SearchDream[2007-05-30]: Catch the errors here 
		}

		delete resolved;

		while (!m_HostEntryList.IsEmpty())
		{
			Hostname_Entry* entry = m_HostEntryList.GetHead();
			memset(m_aucHostnameBuffer, 0, sizeof(m_aucHostnameBuffer));

			if (WSAAsyncGetHostByName(m_hWnd, WM_DNSRESOLVED, entry->strHostname, m_aucHostnameBuffer, sizeof m_aucHostnameBuffer) != 0)
			{
				return TRUE;
			}

			m_HostEntryList.RemoveHead();

			delete entry;
		}
	}
	catch (...)
	{

	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////

CDNSManager::CDNSManager()
{
	VERIFY(m_DNSWnd.CreateEx(0, AfxRegisterWndClass(0), _T("eMule Async DNS Resolve Socket Wnd #2"), WS_OVERLAPPED, 0, 0, 0, 0, NULL, NULL));
}

CDNSManager::~CDNSManager()
{
	m_DNSWnd.DestroyWindow(); // just to avoid a MFC warning
}

void CDNSManager::AddToResolved(CPartFile* pFile, SUnresolvedHostname* pUH, LPCTSTR lpszRefer)
{
	if( pFile && pUH ) 
	{
		if (!pFile->HasNullHash())
		{
			AddToResolved(pFile->GetFileHash(), pUH->strHostname, pUH->nPort, pUH->strURL, lpszRefer);
		}
		else // VC-SearchDream[2007-07-23] : for HTTP and FTP Direct DownLoad
		{
			m_DNSMap.SetAt(pUH->strURL, pFile); // Add to DNS Map
			AddToResolved(NULL, pUH->strHostname, pUH->nPort, pUH->strURL, lpszRefer,pFile);
		}
	}
	else if (pFile == NULL) // VC-SearchDream[2007-03-19]: for HTTP and FTP Direct DownLoad
	{
		AddToResolved(NULL, pUH->strHostname, pUH->nPort, pUH->strURL, lpszRefer);
	}
}

void CDNSManager::AddToResolved(const uchar* fileid, LPCSTR pszHostname, uint16 port, LPCTSTR pszURL, LPCTSTR lpszRefer,CPartFile* pPartFile )
{
	m_DNSWnd.AddToResolve(fileid, pszHostname, port, pszURL, lpszRefer,pPartFile);
}

void CDNSManager::DisPatch(Hostname_Entry * resolved, CList<uint32> & iplist)
{
	CPartFile * pPartFile = NULL;

	if (!m_DNSMap.IsEmpty() && m_DNSMap.Lookup(resolved->strURL, pPartFile))
	{
		if (iplist.GetCount() > 1)
		{
			for (POSITION pos = iplist.GetHeadPosition(); pos != NULL; iplist.GetNext(pos))
			{
				pPartFile->AddSource(resolved->strURL, iplist.GetAt(pos));
			}
		}
		else
		{
			pPartFile->AddSource(resolved->strURL, iplist.GetHead());
		}
	}
	else
	{
	}
}

void CDNSManager::AddUrlToDNS(const CString &strUrl, CPartFile * pPartFile)
{
	POSITION pos = pPartFile->m_UrlSiteList.GetHeadPosition();
	while (pos)
	{
		CUrlSite *pSite = pPartFile->m_UrlSiteList.GetNext(pos);
        if (pSite->m_strUrl == strUrl)
        {
			return;
        }
	}
	
	if ((strUrl.Left(7).CompareNoCase(_T("http://")) == 0) || (strUrl.Left(6).CompareNoCase(_T("ftp://")) == 0))
	{
		TCHAR szScheme[INTERNET_MAX_SCHEME_LENGTH];
		TCHAR szHostName[INTERNET_MAX_HOST_NAME_LENGTH];
		TCHAR szUrlPath[INTERNET_MAX_PATH_LENGTH];
		TCHAR szUserName[INTERNET_MAX_USER_NAME_LENGTH];
		TCHAR szPassword[INTERNET_MAX_PASSWORD_LENGTH];
		TCHAR szExtraInfo[INTERNET_MAX_URL_LENGTH];

		// URL Split
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

		if (InternetCrackUrl(strUrl, 0, 0, &Url) && Url.dwHostNameLength > 0)
		{
			SUnresolvedHostname* hostname = new SUnresolvedHostname;
			hostname->strURL = strUrl;
			hostname->strHostname = szHostName;
			AddToResolved(pPartFile, hostname);
			delete hostname;
		}
	}
}
