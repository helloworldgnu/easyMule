/*
 * $Id: UPnpAsynThreads.h 4483 2008-01-02 09:19:06Z soarchin $
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

#include "UPnpMgr.h"
#include "UPnpNatMapping.h"

class CUPnpAsynThreadsParam
{
public:
	CUPnpMgr			*pUpnpMgr;
	CUPnpNatMapping		mapping;
	BOOL				bRetryRand;
	MSG					notifyParam;
	DWORD				dwCustomParam;
	BOOL				bHasCleanedFillupBug;
};

class CUPnpAsynThreadsResult
{
public:
	DWORD				dwCustomParam;
	WORD				wInternalPort;
	WORD				wExternalPort;
	BOOL				bCleanedFillupBug;
	DWORD				dwActionErrorCode;
};

class CUPnpAsynThreads
{
public:
	CUPnpAsynThreads(void);
	~CUPnpAsynThreads(void);


	static void AddNatPortMappingAsyn(CUPnpMgr	*pMgr,
										const CUPnpNatMapping &mapping,
										HWND hNotifyWnd,
										UINT uNotifyMessage,
										BOOL bRetryRand = FALSE,
										DWORD dwCustomParam = 0,
										BOOL bHasCleanedFillupBug = TRUE);
	static UINT AFX_CDECL AddNATPortMappingAsynProc(LPVOID lpParam);

	static void CleanMapedPortQuickly(CUPnpMgr *pMgr);
};
