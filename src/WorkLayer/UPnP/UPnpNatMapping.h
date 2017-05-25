/*
 * $Id: UPnpNatMapping.h 4483 2008-01-02 09:19:06Z soarchin $
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

class CUPnpNatMapping
{
public:
	CUPnpNatMapping(void);
	~CUPnpNatMapping(void);

	CUPnpNatMapping(const CUPnpNatMapping	&entry);
	CUPnpNatMapping&		operator=(const CUPnpNatMapping &entry);

	void	Empty();
public:
	WORD		m_wInternalPort;				// Port mapping internal port
	WORD		m_wExternalPort;				// Port mapping external port
	CString		m_strProtocol;					// Protocol-> TCP (UPNPNAT_PROTOCOL:UNAT_TCP) || UDP (UPNPNAT_PROTOCOL:UNAT_UDP)
	CString		m_strDescription;				// Port mapping description
	CString		m_strInternalClient;

};
