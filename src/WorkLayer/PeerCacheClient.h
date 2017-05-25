/* 
 * $Id: PeerCacheClient.h 5041 2008-03-19 04:35:57Z huby $
 * 
 * this file is part of eMule
 * Copyright (C)2002-2006 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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

#include "UpDownClient.h"

///////////////////////////////////////////////////////////////////////////////
// CUrlClient

/*
class CUrlClient : public CUpDownClient
{
	DECLARE_DYNAMIC(CUrlClient)

public:
	CUrlClient(LPCTSTR pszUrl, CPartFile* pPartFile, uint32 nIP = 0);
	virtual ~CUrlClient();

	virtual bool TryToConnect(bool bIgnoreMaxCon, CRuntimeClass* pClassSocket = NULL);
	virtual bool Disconnected(CString strReason, bool bFromSocket = false);
	virtual bool SendHelloPacket();
};
*/
