/*
 * $Id: MessageLog.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
#include ".\MessageLog.h"
#include "GlobalVariable.h"
#include "UIMessage.h"


CUIMessageParam::~CUIMessageParam()
{
	if(m_pLock)
	{
		delete m_pLock;
		m_pLock = NULL;
	}
}

////////////////////////////////////////////////////////////////////////////

CMessageLog::CMessageLog(void)
{
	m_uIndex = 0;
}

CMessageLog::~CMessageLog(void)
{
	POSITION pos = m_MsgTags.GetHeadPosition();
	while(pos)
	{
		delete m_MsgTags.GetNextValue(pos);
	}
	m_MsgTags.RemoveAll();

	pos = m_MsgParams.GetHeadPosition();

	while (pos)
	{
		CLogMessageParam param;
		UINT uIndex;
		m_MsgParams.GetAt(pos, uIndex, param);

		if (param.msg == WM_ADD_LOGTEXT)
		{
			delete (CString*)param.lParam;
		}

		m_MsgParams.GetNextValue(pos);
	}

	m_MsgParams.RemoveAll();

	//while (pos)
	//{
	//	if(((CPair*)m_MsgParams.GetAt(pos))->msg == WM_ADD_LOGTEXT)
	//	{
	//		//delete (CString*)((CLogMessageParam*)m_MsgParams.GetAt(pos))->lParam;
	//	}
	//	
	//	m_MsgParams.GetNextValue(pos);
	//}
}

CMessageLog * CMessageLog::GetInstace()
{
	static CAutoPtr<CMessageLog> a;
	if(a==NULL)
	{
		a.Attach(new CMessageLog);
	}

	return a;
}

aMessageParam CMessageLog::GetMessage(UINT uIndex)
{
	aMessageParam aMsg;
	CSingleLock * pLock=new CSingleLock(&m_Mutex, true);

	CLogMessageParam param;
	if(m_MsgParams.Lookup(uIndex, param))
	{
		aMsg.Attach(new CUIMessageParam(param) );
		aMsg->m_pLock = pLock;
		m_MsgParams.RemoveKey(uIndex);
		//TRACE(_T("GetMessage %d \n"),uIndex);
		if(param.pTag)
		{
			CList<UINT, UINT&> * pLst;
			if(m_MsgTags.Lookup(param.pTag, pLst) && pLst)
			{
				POSITION pos=pLst->Find(uIndex);
				if(pos) pLst->RemoveAt(pos);
			}
		}
	}
	else
	{
		delete pLock;
	}

	return aMsg;
}

UINT CMessageLog::SaveMessage(UINT msg, WPARAM wParam, LPARAM lParam, void * pTag, bool bCredible)
{
	CSingleLock slock(&m_Mutex, true);
	CLogMessageParam param;
	param.lParam = lParam;
	param.wParam = wParam;
	param.msg = msg;
	param.pTag = pTag;
	if(++m_uIndex> ((UINT(-1))>>1) ) m_uIndex =1;

	m_MsgParams.SetAt(m_uIndex, param);
	//TRACE(_T("SaveMessage %d,%d \n"),m_uIndex,lParam);
	if(pTag)
	{
		CList<UINT, UINT&> * pIndexLst=NULL;
		if(!m_MsgTags.Lookup(pTag, pIndexLst))
		{
			pIndexLst = new CList<UINT, UINT&>;
			m_MsgTags.SetAt(pTag, pIndexLst);
		}
		UINT ele=bCredible? (m_uIndex|0x80000000) : m_uIndex;
		pIndexLst->AddTail(ele);
	}

	return m_uIndex;
}

void CMessageLog::RemoveTag(void * pTag)
{
	CSingleLock slock(&m_Mutex, true);

#ifdef _DEBUG
	m_RemovedTags.AddTail(pTag);
	if(m_RemovedTags.GetSize()>5) m_RemovedTags.RemoveHead();
#endif

	CList<UINT, UINT&> * pLst;
	if(m_MsgTags.Lookup(pTag, pLst))
	{
		POSITION pos=pLst->GetHeadPosition();
		UINT uOldIndex=0;
		UINT uCount=0;
		while(pos && IsWindow(CGlobalVariable::m_hListenWnd))
		{
			POSITION posBak = pos;
			UINT uIndex=pLst->GetNext(pos);

			//  The message must be sent out to UI layer,
			//  or else will cause the UI crash
			if(uIndex> ((UINT(-1))>>1) )
			{
				uIndex-=(UINT)(1<<31);

				if(uIndex!=uOldIndex)
				{
					uCount = 0;
					uOldIndex = uIndex;
				}
				else uCount++;

				//  timeout
				if(uCount>6)
				{
					pLst->RemoveAt(posBak);
					m_MsgParams.RemoveKey(uIndex);
					//TRACE(_T("RemoveMessage %d \n"),uIndex);
					ASSERT(false);
					continue;
				}

				CLogMessageParam param;
				if(! m_MsgParams.Lookup(uIndex, param))
				{
					continue;
				}

				//  try to send message to UI layer again
				slock.Unlock();
				PostMessage(CGlobalVariable::m_hListenWnd, param.msg, uIndex, 0);
				Sleep(500);
				slock.Lock();

				//  Maybe the next thread finish the message
				if(! m_MsgTags.Lookup(pTag, pLst)) return;

				//  restart check
				pos=pLst->GetHeadPosition();
			}
			//  remove the params, so the UI layer cannot get the invalid param
			else
			{
				pLst->RemoveAt(posBak);
				m_MsgParams.RemoveKey(uIndex);
				//TRACE(_T("RemoveMessage %d \n"),uIndex);
			}
		}

		m_MsgTags.RemoveKey(pTag);
		delete pLst;
	}
}
