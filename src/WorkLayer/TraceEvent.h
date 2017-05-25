/*
 * $Id: TraceEvent.h 4483 2008-01-02 09:19:06Z soarchin $
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



// CTraceEvent ÃüÁîÄ¿±ê

class CTraceEvent : public CObject
{
	DECLARE_DYNAMIC(CTraceEvent)
public:
	CTraceEvent();
	virtual ~CTraceEvent();

public:
	virtual COLORREF GetTextColor() = 0;
	virtual COLORREF GetBkColor() = 0;

	virtual CString GetText(void) = NULL;
	virtual CString GetTime(void);

public:
	enum { None = -1 };
protected:
	CTime	m_time;
};

class CTraceError : public CTraceEvent
{
	DECLARE_DYNAMIC(CTraceError)
public:
	CTraceError(const CString& strError);
	virtual ~CTraceError() {}

public:
	virtual COLORREF GetTextColor();
	virtual COLORREF GetBkColor();

	virtual CString GetText(void);

protected:
	CString m_strError;

};

//CTraceInformation
class CTraceInformation : public CTraceEvent
{
	DECLARE_DYNAMIC(CTraceInformation)
public:
	CTraceInformation(const CString& strInfomation);
	virtual ~CTraceInformation() {}

public:
	virtual COLORREF GetTextColor();
	virtual COLORREF GetBkColor();

	virtual CString GetText(void);

protected:
	CString m_strInfomation;
};

//CTraceSendMessage
class CTraceSendMessage : public CTraceEvent
{
	DECLARE_DYNAMIC(CTraceSendMessage)
public:
	CTraceSendMessage(const CString& strSendMessage);
	virtual ~CTraceSendMessage() {}

public:
	virtual COLORREF GetTextColor();
	virtual COLORREF GetBkColor();

	virtual CString GetText(void);

protected:
	CString m_strSendMessage;
};

//CTraceServerMessage
class CTraceServerMessage : public CTraceEvent
{
	DECLARE_DYNAMIC(CTraceServerMessage)
public:
	CTraceServerMessage(const CString& strServerMessage);
	virtual ~CTraceServerMessage() {}

public:
	virtual COLORREF GetTextColor();
	virtual COLORREF GetBkColor();

	virtual CString GetText(void);

protected:
	CString m_strServerMessage;
};

//CTraceKadMessage
class CTraceKadMessage : public CTraceEvent
{
	DECLARE_DYNAMIC(CTraceKadMessage)
public:
	CTraceKadMessage(const CString& strKadMessage);
	virtual ~CTraceKadMessage() {}

public:
	virtual COLORREF GetTextColor();
	virtual COLORREF GetBkColor();

	virtual CString GetText(void);

protected:
	CString m_strKadMessage;
};
