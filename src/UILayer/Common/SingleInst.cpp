/*
 * $Id: SingleInst.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
#include "stdafx.h"
#include ".\singleinst.h"

CSingleInst::CSingleInst(LPCTSTR lpszGuid)
{
	m_strGuid = lpszGuid;
	m_strGuidMapFile = m_strGuid;
	m_strGuidMapFile.append(_T("_MapFile"));

	m_hInstMutex	= NULL;
	m_hMapFile		= NULL;
}

CSingleInst::~CSingleInst(void)
{
	AppEnd();
}

BOOL CSingleInst::AppStart()
{
	m_hInstMutex = CreateMutex(NULL, TRUE, m_strGuid.c_str());
	if (::GetLastError() == ERROR_ALREADY_EXISTS || ::GetLastError() == ERROR_ACCESS_DENIED)
		return FALSE;
	else
		return TRUE;
}

void CSingleInst::AppEnd()
{
	if (NULL != m_hMapFile && INVALID_HANDLE_VALUE != m_hMapFile)
	{
		CloseHandle(m_hMapFile);
		m_hMapFile = NULL;
	}
	
	if (NULL != m_hInstMutex)
	{
		ReleaseMutex(m_hInstMutex);
		CloseHandle(m_hInstMutex);
		m_hInstMutex = NULL;
	}
}

void CSingleInst::InitCompleted(LPVOID pData, UINT uSize)
{
	if (NULL != pData && 0 != uSize)
	{
		LPVOID	pBuf;


		if (NULL == m_hMapFile)
		{
			m_hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, uSize, m_strGuidMapFile.c_str());
			if (NULL == m_hMapFile || INVALID_HANDLE_VALUE == m_hMapFile)
				goto end;
		}

		pBuf = MapViewOfFile(m_hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, uSize);
		CopyMemory(pBuf, pData, uSize);
		UnmapViewOfFile(pBuf);
	}

end:
	ReleaseMutex(m_hInstMutex);
}

void CSingleInst::OnUninit()
{
	CloseHandle(m_hMapFile);
	m_hMapFile = NULL;
}

BOOL CSingleInst::WaitForInitCompleted(LPVOID pData, UINT uSize, DWORD dwMillisecond)
{
	if (NULL == m_hInstMutex)
		return FALSE;

	DWORD dwResult;
	dwResult = WaitForSingleObject(m_hInstMutex, dwMillisecond);
	if (WAIT_OBJECT_0 != dwResult && WAIT_ABANDONED != dwResult)
		return FALSE;

	ReleaseMutex(m_hInstMutex);
	
	if (NULL == pData || 0 == uSize)
		return TRUE;

	HANDLE	hMapFile;
	LPVOID	pBuf;

	hMapFile = OpenFileMapping(FILE_MAP_READ, FALSE, m_strGuidMapFile.c_str());
	if (NULL == hMapFile || INVALID_HANDLE_VALUE == hMapFile)
		return FALSE;

	pBuf = MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, uSize);
	CopyMemory(pData, pBuf, uSize);
	UnmapViewOfFile(pBuf);

	CloseHandle(hMapFile);

	return TRUE;
}
