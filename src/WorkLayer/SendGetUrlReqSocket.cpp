/*
 * $Id: SendGetUrlReqSocket.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
// SendGetUrlReqSocket.cpp : 实现文件
//

#include "stdafx.h"
#include "SendGetUrlReqSocket.h"
#include "packets.h"
#include "Preferences.h"
#include "log.h"


// CSendGetUrlReqSocket

CSendGetUrlReqSocket::CSendGetUrlReqSocket()
{
	m_bIsPost = false;
	m_strPost = "";
}

CSendGetUrlReqSocket::~CSendGetUrlReqSocket()
{
}


// CSendGetUrlReqSocket 成员函数

void CSendGetUrlReqSocket::SetPost(bool bIsPost)
{
	m_bIsPost = bIsPost;
}

void CSendGetUrlReqSocket::SetPostData(CStringA strPost)
{
	m_strPost = strPost;
}

bool CSendGetUrlReqSocket::SendRequest(void)
{
	CStringA strUrlPath = GetUrlPath();
	if (strUrlPath.IsEmpty())
		return false;

	if (!Create())
		return false;

	Connect(GetServer(), GetPort());

	return true;
}

void CSendGetUrlReqSocket::OnConnect(int nErrorCode)
{
	if (0 != nErrorCode)
	{
		if (thePrefs.GetVerbose())
		{
			CString		strUrlPath(GetUrlPath());
			CString		strServer(GetServer());

			strUrlPath.Replace(_T("%"), _T("%%"));
			AddDebugLogLine(false, _T("将要取%s，但连接%s返回失败。"), strUrlPath, strServer);
		}

		return;
	}

	CStringA strHttpRequest;
	strHttpRequest.AppendFormat("%s %s HTTP/1.0\r\n", m_bIsPost ? "POST" : "GET", GetUrlPath());
	strHttpRequest.AppendFormat("Host: %s\r\n", GetServer());
	strHttpRequest.AppendFormat("Accept: */*\r\n");
	if(m_bIsPost)
	{
		strHttpRequest.AppendFormat("Accept-Encoding: none\r\n");
		strHttpRequest.AppendFormat("Content-Type: application/x-www-form-urlencoded\r\n");
		strHttpRequest.AppendFormat("Content-Length: %d\r\n", m_strPost.GetLength());
	}
	//strHttpRequest.AppendFormat("Connection: Keep-Alive\r\n");
	strHttpRequest.Append("\r\n");
	if(m_bIsPost)
	{
		strHttpRequest.Append(m_strPost);
	}

	if (thePrefs.GetVerbose())
	{
		CString		strRequest(strHttpRequest);
		strRequest.Replace(_T("%"), _T("%%"));

		AddDebugLogLine(false, _T("与服务器 %s 连接成功，准备发送:"), CString(GetServer()));
		AddDebugLogLine(false, strRequest);
	}


	CRawPacket* pHttpPacket = new CRawPacket(strHttpRequest);
	SendPacket(pHttpPacket);
	SetHttpState(HttpStateRecvExpected);
}

void CSendGetUrlReqSocket::DataReceived(const BYTE* pucData, UINT uSize)
{
	if (thePrefs.GetVerbose())
	{
		CString		strUrlPath(GetUrlPath());
		strUrlPath.Replace(_T("%"), _T("%%"));

		CStringA	straRecv((char *)pucData, uSize);
		AddDebugLogLine(false, _T("取%s时，返回结果："), strUrlPath);
		AddDebugLogLine(false, CString(straRecv));
	}

	CHttpClientReqSocket::DataReceived(pucData, uSize);
}


bool CSendGetUrlReqSocket::ProcessHttpResponse()
{
	return CHttpClientReqSocket::ProcessHttpResponse();
}

bool CSendGetUrlReqSocket::ProcessHttpResponseBody(const BYTE* pucData, UINT size)
{
	return CHttpClientReqSocket::ProcessHttpResponseBody(pucData, size);
}

