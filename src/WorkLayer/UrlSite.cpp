/*
 * $Id: UrlSite.cpp 5702 2008-05-30 09:00:18Z huby $
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
#include ".\UrlSite.h"

CUrlSite::CUrlSite(void) :
m_dwFromWhere(sfUnknwon) , m_dwConnectionCount(0) , m_dwRetryCount(0)
, m_dwDataTransferedWithoutPayload(0) , m_dwDataTransferedWithPayload(0),m_dwOldDataTransferedWithoutPayload(0),m_dwOldDataTransferedWithPayload(0)
, m_bBadSite(false),m_bNeedCommitted(true),m_pRedirectFrom(NULL)
{
}

CUrlSite::~CUrlSite(void)
{
	for (size_t i = 0;i < this->m_IPSiteList.size();i++)
	{
        delete this->m_IPSiteList[i];
	}
	this->m_IPSiteList.clear();
}

bool CUrlSite::IsBadUrlSite()
{   
	if( m_bBadSite )
		return true;

	bool bBadSite = false;
	for (size_t i = 0; i< this->m_IPSiteList.size(); i++)
	{
		if (!this->m_IPSiteList[i]->m_bBadSite)
		{
			bBadSite = false;
			break;
		}
		else
			bBadSite = true;
	}
	return bBadSite;
}

DWORD CUrlSite::GetUrlSiteConnectNum()
{
	m_dwConnectionCount = 0;
	for (size_t i = 0; i< this->m_IPSiteList.size();i++)
	{
		m_dwConnectionCount += this->m_IPSiteList[i]->m_dwConnectionCount;
	}
	return m_dwConnectionCount;
}

bool CUrlSite::IsMyIP( DWORD dwIP,DWORD dwFrom)
{
	if( dwFrom!=m_dwFromWhere )
		return false;

	for (size_t i = 0; i< this->m_IPSiteList.size();i++)
	{
		if( dwIP==m_IPSiteList[i]->m_dwIpAddress )
			return true;
	}	

	return false;
}