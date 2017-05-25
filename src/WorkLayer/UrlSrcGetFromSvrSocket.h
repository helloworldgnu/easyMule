/*
 * $Id: UrlSrcGetFromSvrSocket.h 4483 2008-01-02 09:19:06Z soarchin $
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


#include "SendGetUrlReqSocket.h"
#include "UrlSrcFromSvrMgr.h"
// CUrlSrcGetFromSvrSocket 命令目标

class CUrlSrcGetFromSvrSocket : public CSendGetUrlReqSocket
{
public:
	CUrlSrcGetFromSvrSocket(CUrlSrcFromSvrMgr *pMgr, bool bStart);
	virtual ~CUrlSrcGetFromSvrSocket();

protected:
	virtual CStringA GetServer();
	virtual CStringA GetUrlPath();

	virtual bool	ProcessHttpResponse();
	virtual bool	ProcessHttpResponseBody(const BYTE* pucData, UINT size);

	bool	ProcessHttpResponse_Start();
	bool	ProcessHttpResponseBody_Start(const BYTE* pucData, UINT size);
	bool	ProcessHttpResponse_Finished();
	bool	ProcessHttpResponseBody_Finished(const BYTE* pucData, UINT size);
protected:
	CUrlSrcFromSvrMgr		*m_pMgr;
	
	bool					m_bStart;	//true:		表示文件开始下载。
										//false:	表示文件完成下载。
	CStringA				m_strUrlPath;
};


