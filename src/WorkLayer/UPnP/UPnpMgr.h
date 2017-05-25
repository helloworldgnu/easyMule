/*
 * $Id: UPnpMgr.h 4789 2008-02-14 08:59:23Z fengwen $
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

#include <afxtempl.h>
#include <AfxMt.h>
#include <set>
#include "UPnpNat.h"
#include "UPnpNatMapping.h"
#include "UPnpNatMappingKey.h"

using namespace std;

class CUPnpMgr
{
public:
	CUPnpMgr(void);
	~CUPnpMgr(void);

	enum {RANDOM_RETRY_TIMES = 5};

	CSyncObject*	GetSyncObject(){return &m_cs;}
	BOOL			Lock(){return m_cs.Lock();}
	BOOL			Unlock(){return m_cs.Unlock();}
	BOOL			TryLock(){return TryEnterCriticalSection(&(m_cs.m_sect));}

	void			SetBindAddress(LPCTSTR lpszBindAddress);
	void			SetActionTimeout(int iActionTimeoutMs = 5000);
	int				GetActionTimeout();

	HRESULT			AddNATPortMapping(CUPnpNatMapping &mapping, BOOL bTryRandom = FALSE);
	void			RemoveNATPortMapping(const CUPnpNatMappingKey &mappingKey);
	void			CleanupAllEverMapping(void);

	void			WriteAddedMappingToFile(void);
	void			ReadAddedMappingFromFile(void);


	BOOL			CleanedFillupBug();
	
	CString			GetLocalIPStr();
	DWORD			GetLocalIP();
	
	static CString	ResultCode2String(HRESULT hr);
	static CString	Result2String(HRESULT hr, DWORD dwActionErrorCode);
	static bool		IsLANIP(DWORD nIP);

	DWORD			GetLastActionErrorCode(){return m_nat.GetLastActionErrorCode();}
protected:
	void			InitLocalIP();
	void			CleanupMappingArr();


protected:
	CCriticalSection	m_cs;
	CUPnpNat			m_nat;

	set<CUPnpNatMappingKey>									m_setAddedMapping;

	CString		m_slocalIP;
	DWORD		m_uLocalIP;

};
