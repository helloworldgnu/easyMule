/*
 * $Id: TraceEvent.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
// TraceEvent.cpp : 实现文件
//

#include "stdafx.h"
//#include "LogListDemo.h"
#include "TraceEvent.h"

IMPLEMENT_DYNAMIC(CTraceEvent, CObject)
IMPLEMENT_DYNAMIC(CTraceError, CTraceEvent)
IMPLEMENT_DYNAMIC(CTraceInformation, CTraceEvent)
IMPLEMENT_DYNAMIC(CTraceSendMessage, CTraceEvent)
IMPLEMENT_DYNAMIC(CTraceServerMessage, CTraceEvent)
IMPLEMENT_DYNAMIC(CTraceKadMessage, CTraceEvent)

// CTraceEvent

CTraceEvent::CTraceEvent()
{
	m_time = CTime::GetCurrentTime();
}

CTraceEvent::~CTraceEvent()
{
}

CString CTraceEvent::GetTime(void)
{
	CString time = m_time.Format(_T("%a %b %d %H:%M:%S %Y "));
	return time;
}


// CTraceError 成员函数
CTraceError::CTraceError(const CString & strError)
{
	m_strError = strError;
}

COLORREF CTraceError::GetTextColor()
{
	return RGB(139, 0, 0);
}

CString CTraceError::GetText()
{
	return m_strError;
}

COLORREF CTraceError::GetBkColor()
{
	return RGB(255, 192, 203);
}

//CTraceInfomation函数
CTraceInformation::CTraceInformation(const CString & strInfomation)
{
	m_strInfomation = strInfomation;
}

COLORREF CTraceInformation::GetTextColor()
{
	return RGB(0, 147, 0);
}

CString CTraceInformation::GetText()
{
	return m_strInfomation;
}

COLORREF CTraceInformation::GetBkColor()
{
	return RGB(152, 251, 152);
}

//CTraceSendMessage
CTraceSendMessage::CTraceSendMessage(const CString & strSendMessage)
{
	m_strSendMessage = strSendMessage;
}

COLORREF CTraceSendMessage::GetTextColor()
{
	return RGB(0, 0, 139);
}

CString CTraceSendMessage::GetText()
{
	return m_strSendMessage;
}

COLORREF CTraceSendMessage::GetBkColor()
{
	return RGB(175, 238, 238);
}

//CTraceServerMessage
CTraceServerMessage::CTraceServerMessage(const CString & strServerMessage)
{
	m_strServerMessage = strServerMessage;
}

COLORREF CTraceServerMessage::GetTextColor()
{
	return RGB(0, 0, 0);
}

CString CTraceServerMessage::GetText()
{
	return m_strServerMessage;
}

COLORREF CTraceServerMessage::GetBkColor()
{
	return RGB(230, 230, 230);
}

//CTraceKadMessage
CTraceKadMessage::CTraceKadMessage(const CString & strKadMessage)
{
	m_strKadMessage = strKadMessage;
}

COLORREF CTraceKadMessage::GetTextColor()
{
	return RGB(204, 0, 0);
}

CString CTraceKadMessage::GetText()
{
	return m_strKadMessage;
}

COLORREF CTraceKadMessage::GetBkColor()
{
	return RGB(255, 255, 136);
}
