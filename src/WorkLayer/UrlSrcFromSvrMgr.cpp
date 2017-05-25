/*
 * $Id: UrlSrcFromSvrMgr.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
// UrlSrcFromSvrMgr.cpp : 实现文件
//

#include "stdafx.h"
#include "emule.h"
#include "UrlSrcFromSvrMgr.h"
#include "UrlSrcGetFromSvrSocket.h"
#include "MetaLink/MetaLinkQuerySocket.h"
#include "PartFile.h"


// CUrlSrcFromSvrMgr

CUrlSrcFromSvrMgr::CUrlSrcFromSvrMgr()
{
	m_pAssocPartFile	= NULL;
}

CUrlSrcFromSvrMgr::~CUrlSrcFromSvrMgr()
{
}


// CUrlSrcFromSvrMgr 成员函数
void CUrlSrcFromSvrMgr::SetAssocPartFile(CPartFile *pAssocPartFile)
{
	m_pAssocPartFile = pAssocPartFile;
}

bool CUrlSrcFromSvrMgr::GetSrcFromServerAsyn(void)
{
	try
	{
		if (NULL == m_pAssocPartFile)
			return false;
		
		CMetaLinkQuerySocket * pQSocket = new CMetaLinkQuerySocket(this, true);
		if (NULL == pQSocket)
			return false;

		if( !m_pAssocPartFile->HasNullHash() )
		{
			CUrlSrcGetFromSvrSocket	*pSocket = new CUrlSrcGetFromSvrSocket(this, true);		//由于其基类 CClientReqSocket 在连接断开后会 delete 自己。所以new一个对象来建立连接。
			if (NULL != pSocket)
				return pSocket->SendRequest() && pQSocket->SendRequest();
		}
		
		return pQSocket->SendRequest();
	}
	catch( ... )
	{
		return false;
	}
}

bool CUrlSrcFromSvrMgr::SendReq_FileDownloaded(void)
{
	if (NULL == m_pAssocPartFile)
		return false;

	CMetaLinkQuerySocket * pQSocket = new CMetaLinkQuerySocket(this, false);
	if (NULL == pQSocket)
		return false;

	CUrlSrcGetFromSvrSocket	*pSocket = new CUrlSrcGetFromSvrSocket(this, false);		//由于其基类 CClientReqSocket 在连接断开后会 delete 自己。所以new一个对象来建立连接。
	if (NULL == pSocket)
		return pQSocket->SendRequest();

	return pQSocket->SendRequest() && pSocket->SendRequest();
}

bool CUrlSrcFromSvrMgr::AddSrcToServer(LPCTSTR lpszUrl)
{
	if (!IsExistInFetchedList(lpszUrl))
	{
		//todo: add to server
		m_strlstFetchedUrlSources.AddTail(lpszUrl);
	}

	return true;
}

bool CUrlSrcFromSvrMgr::IsExistInFetchedList(LPCTSTR lpszUrl)
{
	POSITION	pos;
	CString		str;

	pos = m_strlstFetchedUrlSources.GetHeadPosition();
	while (NULL != pos)
	{
		str = m_strlstFetchedUrlSources.GetNext(pos);
		if (0 == str.CompareNoCase(lpszUrl))
			return true;
	}

	return false;
}
