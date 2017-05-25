/*
 * $Id: DNSManager.h 4483 2008-01-02 09:19:06Z soarchin $
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

#include "ED2KLink.h"

class CPartFile;
///////////////////////////////////////////////////////////////////////////////////////////////////

struct Hostname_Entry 
{
	uchar		fileid[16];
	CStringA	strHostname;
	uint16		port;
	CString		strURL;
	CString		strRefer;	
	CPartFile*  pPartFile;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class CDNSResolveWnd : public CWnd
{
public:
	CDNSResolveWnd();
	virtual ~CDNSResolveWnd();

	void AddToResolve(const uchar* fileid, LPCSTR pszHostname, uint16 port, LPCTSTR pszURL = NULL, LPCTSTR lpszRefer = NULL,CPartFile* pPartFile =NULL );	

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnDNSResolved(WPARAM wParam, LPARAM lParam);

private:
	CTypedPtrList<CPtrList, Hostname_Entry*> m_HostEntryList;
	char m_aucHostnameBuffer[MAXGETHOSTSTRUCT];
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class CPartFile;
class CDNSManager
{
public:
	CDNSManager();
	virtual ~CDNSManager();

	void AddUrlToDNS(const CString &strUrl, CPartFile * pPartFile);

	void AddToResolved(CPartFile* pFile, SUnresolvedHostname* pUH, LPCTSTR lpszRefer = NULL);
	void AddToResolved(const uchar* fileid, LPCSTR pszHostname, uint16 port, LPCTSTR pszURL = NULL, LPCTSTR lpszRefer = NULL,CPartFile* pPartFile =NULL );

protected:

	void DisPatch(Hostname_Entry * resolved, CList<uint32> & iplist);

protected:
	CDNSResolveWnd m_DNSWnd;
	friend class CDNSResolveWnd;

	CRBMap<CString, CPartFile*> m_DNSMap; // VC-SearchDream[2007-07-23]: This is for HTTP and FTP Direct DownLoad
};

///////////////////////////////////////////////////////////////////////////////////////////////////
