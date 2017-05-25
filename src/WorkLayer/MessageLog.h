/*
 * $Id: MessageLog.h 4483 2008-01-02 09:19:06Z soarchin $
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

class CMessageParam
{
public:
	WPARAM wParam;
	LPARAM lParam;
	CMessageParam()
	{
		wParam = 0;
		lParam = 0;
	}
};

class CUIMessageParam : public CMessageParam
{
public:
	CSingleLock * m_pLock;
	CUIMessageParam(const CMessageParam & param)
	{
		m_pLock = NULL;

		wParam = param.wParam;
		lParam = param.lParam;
	}
	~CUIMessageParam();
};

typedef CAutoPtr<CUIMessageParam> aMessageParam;

class CLogMessageParam : public CMessageParam
{
public:
	void * pTag;
	UINT msg;
};
class CMessageLog
{
	friend class CMessageParam;
public:
	~CMessageLog(void);
	static CMessageLog * GetInstace();
	UINT SaveMessage(UINT msg, WPARAM wParam, LPARAM lParam, void * pTag, bool bCredible);
	aMessageParam GetMessage(UINT uIndex);
	void RemoveTag(void * pTag);
private:
	CMessageLog();
	const CMessageLog & operator=(const CMessageLog &);
	CMessageLog(const CMessageLog &);
	CMutex m_Mutex;
	UINT m_uIndex;
	CRBMap<UINT, CLogMessageParam> m_MsgParams;
	CRBMap<void *, CList<UINT, UINT&>* > m_MsgTags;
#ifdef _DEBUG
	CList<void * > m_RemovedTags;
#endif
};
