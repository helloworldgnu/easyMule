/*
 * $Id: UPnpNatMapping.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
#include ".\upnpnatmapping.h"

CUPnpNatMapping::CUPnpNatMapping(void)
{
	Empty();
}

CUPnpNatMapping::~CUPnpNatMapping(void)
{
}

CUPnpNatMapping::CUPnpNatMapping(const CUPnpNatMapping	&entry)
{
	*this = entry;
}

CUPnpNatMapping& CUPnpNatMapping::operator=(const CUPnpNatMapping &entry)
{
	if (this == &entry)
		return *this;

	m_wInternalPort		= entry.m_wInternalPort;
	m_wExternalPort		= entry.m_wExternalPort;
	m_strProtocol		= entry.m_strProtocol;
	m_strDescription	= entry.m_strDescription;
	m_strInternalClient	= entry.m_strInternalClient;

	return *this;
}

void CUPnpNatMapping::Empty()
{
	m_wInternalPort		= 0;
	m_wExternalPort		= 0;
	m_strProtocol.Empty();
	m_strDescription.Empty();
	m_strInternalClient.Empty();
}
