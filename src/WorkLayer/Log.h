/*
 * $Id: Log.h 4483 2008-01-02 09:19:06Z soarchin $
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

enum EDebugLogPriority{
	DLP_VERYLOW = 0,
	DLP_LOW,
	DLP_DEFAULT,
	DLP_HIGH,
	DLP_VERYHIGH
};

// Log message type enumeration
#define	LOG_INFO		0
#define	LOG_WARNING		1
#define	LOG_ERROR		2
#define	LOG_SUCCESS		3
#define	LOGMSGTYPEMASK	0x03

// Log message targets flags
#define	LOG_DEFAULT		0x00
#define	LOG_DEBUG		0x10
#define	LOG_STATUSBAR	0x20
#define	LOG_DONTNOTIFY	0x40
#define LOG_LEECHER		0x80 //Xman Anti-Leecher-Log


void Log(LPCTSTR pszLine, ...);
void LogError(LPCTSTR pszLine, ...);
void LogWarning(LPCTSTR pszLine, ...);

void Log(UINT uFlags, LPCTSTR pszLine, ...);
void LogError(UINT uFlags, LPCTSTR pszLine, ...);
void LogWarning(UINT uFlags, LPCTSTR pszLine, ...);

void DebugLog(LPCTSTR pszLine, ...);
void DebugLogError(LPCTSTR pszLine, ...);
void DebugLogWarning(LPCTSTR pszLine, ...);

void DebugLog(UINT uFlags, LPCTSTR pszLine, ...);
void DebugLogError(UINT uFlags, LPCTSTR pszLine, ...);
void DebugLogWarning(UINT uFlags, LPCTSTR pszLine, ...);

void LogV(UINT uFlags, LPCTSTR pszFmt, va_list argp);

void AddLogLine(bool bAddToStatusBar, LPCTSTR pszLine, ...);
void AddDebugLogLine(bool bAddToStatusBar, LPCTSTR pszLine, ...);
void AddDebugLogLine(EDebugLogPriority Priority, bool bAddToStatusBar, LPCTSTR pszLine, ...);

void AddLogTextV(UINT uFlags, EDebugLogPriority dlpPriority, LPCTSTR pszLine, va_list argp);

//Xman Anti-Leecher-Log
void AddLeecherLogLine(bool bAddToStatusBar, LPCTSTR pszLine, ...);

///////////////////////////////////////////////////////////////////////////////
// CLogFile

enum ELogFileFormat
{
	Unicode = 0,
	Utf8
};

class CLogFile
{
public:
	CLogFile();
	~CLogFile();

	bool IsOpen() const;
	const CString& GetFilePath() const;
	bool SetFilePath(LPCTSTR pszFilePath);
	void SetMaxFileSize(UINT uMaxFileSize);
	bool SetFileFormat(ELogFileFormat eFileFormat);

	bool Create(LPCTSTR pszFilePath, UINT uMaxFileSize = 1024*1024, ELogFileFormat eFileFormat = Unicode);
	bool Open();
	bool Close();
	bool Log(LPCTSTR pszMsg, int iLen = -1);
	bool Logf(LPCTSTR pszFmt, ...);
	void StartNewLogFile();

protected:
	FILE* m_fp;
	time_t m_tStarted;
	CString m_strFilePath;
	UINT m_uBytesWritten;
	UINT m_uMaxFileSize;
	bool m_bInOpenCall;
	ELogFileFormat m_eFileFormat;
};

extern CLogFile theLog;
extern CLogFile theVerboseLog;
