/*
 * $Id: StatForServer.h 9291 2008-12-24 09:19:05Z dgkang $
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

#include "opcodes.h"



#define PROTOCOL_TYPE_ED		0x01
#define PROTOCOL_TYPE_HTTP		0x02
#define PROTOCOL_TYPE_FTP		0x03
#define PROTOCOL_TYPE_BT		0x04

#pragma warning(disable:4200)




class CUDPSocket;

class CStatForServer
{
public:
	CStatForServer(void);
	~CStatForServer(void);

	enum{RECORD_INTERVAL_MS = 60000};

	void	RecordCurrentRate(float uploadrate, float downloadrate);

	void	GetAverageSpeeds(ULONG &ulUpload, ULONG &ulDownload);
	void	ResetAverageSpeeds();
	void	GetCurrentSpeedLimit(ULONG &ulUpload, ULONG &ulDownload);

protected:
	DWORD	m_dwLastRecordTime;
	ULONG	m_ulRecordTimes_Upload;
	float	m_fAverageUploadSpeed;
	ULONG	m_ulRecordTimes_Download;
	float	m_fAverageDownloadSpeed;

	
protected:
	BOOL	IsAnyTaskRunning();
};

extern CStatForServer	theStatForServer;
