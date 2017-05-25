/*
 * $Id: PerfLog.h 4483 2008-01-02 09:19:06Z soarchin $
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

class CPerfLog
{
public:
	CPerfLog();

	void Startup();
	void Shutdown();
	void LogSamples();

protected:
	// those values have to be specified in 'preferences.ini' -> hardcode them
	enum ELogMode {
		None		= 0,
		OneSample	= 1,
		AllSamples	= 2
	} m_eMode;
	// those values have to be specified in 'preferences.ini' -> hardcode them
	enum ELogFileFormat {
		CSV			= 0,
		MRTG		= 1
	} m_eFileFormat;
	DWORD m_dwInterval;
	bool m_bInitialized;
	CString m_strFilePath;
	CString m_strMRTGDataFilePath;
	CString m_strMRTGOverheadFilePath;
	DWORD m_dwLastSampled;
	uint64 m_nLastSessionSentBytes;
	uint64 m_nLastSessionRecvBytes;
	uint64 m_nLastDnOH;
	uint64 m_nLastUpOH;

	void WriteSamples(UINT nCurDn, UINT nCurUp, UINT nCurDnOH, UINT uCurUpOH);
};

extern CPerfLog thePerfLog;
