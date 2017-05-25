/*
 * $Id: IEFilterMgr.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
#include ".\iefiltermgr.h"
#include "IEFilter.h"

CIEFilterMgr	g_ieFilterMgr;

CIEFilterMgr::CIEFilterMgr(void)
{
	m_pSession = NULL;
	m_pFactory = NULL;

	StartFilter();
}

CIEFilterMgr::~CIEFilterMgr(void)
{
	EndFilter();
}

void CIEFilterMgr::StartFilter()
{
	CLSID ciIEFilter = CIEFilter::factory.GetClassID();

	HRESULT		hr;
	if(NULL == m_pSession)
	{
		IClassFactory *pCF = &CIEFilter::factory.m_xClassFactory;
		hr = pCF->QueryInterface(IID_IClassFactory, (void **) &m_pFactory);
		
		if (SUCCEEDED(hr))
		{
			if (NULL != m_pFactory)
			{
				hr = CoInternetGetSession(0, &m_pSession, 0);
				if(SUCCEEDED(hr))
				{
					m_pSession->RegisterNameSpace(m_pFactory, ciIEFilter, L"http", 0, NULL, 0);
					m_pSession->RegisterNameSpace(m_pFactory, ciIEFilter, L"https", 0, NULL, 0);
				}
			}
		}
	}
}

void CIEFilterMgr::EndFilter()
{
	if(NULL != m_pSession)
	{
		m_pSession->UnregisterNameSpace(m_pFactory, L"http");
		m_pSession->UnregisterNameSpace(m_pFactory, L"https");
		m_pSession->Release();
		m_pSession = NULL;
	}
	if(NULL != m_pFactory)
	{
		m_pFactory->Release();
		m_pFactory = NULL;
	}
}
