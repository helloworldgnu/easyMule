/*
 * $Id: StatForServer.cpp 9291 2008-12-24 09:19:05Z dgkang $
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
#include ".\statforserver.h"
#include "opcodes.h"
#include "Preferences.h"
#include "eMule.h"
#include "DownloadQueue.h"
#include "UDPSocket.h"



CStatForServer	theStatForServer;

CStatForServer::CStatForServer(void)
{
	ResetAverageSpeeds();


}

CStatForServer::~CStatForServer(void)
{
}

void CStatForServer::RecordCurrentRate(float uploadrate, float downloadrate)
{
	//	if reach record interval
	DWORD	dwCurTime = GetTickCount();
	if (dwCurTime - m_dwLastRecordTime < RECORD_INTERVAL_MS)
		return;
	m_dwLastRecordTime = dwCurTime;


	
	if (ULONG_MAX != m_ulRecordTimes_Download
		&& IsAnyTaskRunning())
	{
		m_ulRecordTimes_Download++;
		//MODIFIED by fengwen on 2007/06/19 <begin>	: 原来的算法错了。
		//m_fAverageDownloadSpeed = (ULONG) ( ((float)m_fAverageDownloadSpeed + downloadrate) / m_ulRecordTimes_Download );
		m_fAverageDownloadSpeed = m_fAverageDownloadSpeed + ((downloadrate - m_fAverageDownloadSpeed) / m_ulRecordTimes_Download );
		//MODIFIED by fengwen on 2007/06/19 <end>	: 原来的算法错了。
	}

	if (ULONG_MAX != m_ulRecordTimes_Upload)
	{
		m_ulRecordTimes_Upload++;
		//MODIFIED by fengwen on 2007/06/19 <begin>	: 原来的算法错了。
		//m_fAverageUploadSpeed = (ULONG) ( ((float)m_fAverageUploadSpeed + uploadrate) / m_ulRecordTimes_Upload );
		m_fAverageUploadSpeed = m_fAverageUploadSpeed + ( (uploadrate - m_fAverageUploadSpeed) / m_ulRecordTimes_Upload );
		//MODIFIED by fengwen on 2007/06/19 <end>	: 原来的算法错了。
	}

}

void CStatForServer::GetAverageSpeeds(ULONG &ulUpload, ULONG &ulDownload)
{
	ulUpload	= (ULONG) m_fAverageUploadSpeed;
	ulDownload	= (ULONG) m_fAverageDownloadSpeed;
}

void CStatForServer::ResetAverageSpeeds()
{
	m_dwLastRecordTime			= 0;
	m_ulRecordTimes_Upload		= 0;
	m_fAverageUploadSpeed		= 0;
	m_ulRecordTimes_Download	= 0;
	m_fAverageDownloadSpeed	= 0;
}

void CStatForServer::GetCurrentSpeedLimit(ULONG &ulUpload, ULONG &ulDownload)
{
	ulDownload = (thePrefs.maxdownload != UNLIMITED) ? thePrefs.maxdownload : 0;
	ulUpload = (thePrefs.maxupload != UNLIMITED) ? thePrefs.maxupload : 0;
}

BOOL CStatForServer::IsAnyTaskRunning()
{
	if (NULL == CGlobalVariable::downloadqueue)
		return FALSE;
	else
		return CGlobalVariable::downloadqueue->IsAnyTaskRunning();
}

