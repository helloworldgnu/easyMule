/*
 * $Id: UrlSrcGetFromSvrSocket.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
// UrlSrcGetFromSvrSocket.cpp : 实现文件
//

#include "stdafx.h"
#include "emule.h"
#include "UrlSrcGetFromSvrSocket.h"
#include "UrlSrcFromSvrMgr.h"
#include "PartFile.h"
#include "StringConversion.h"
#include "Preferences.h"
#include "log.h"

// CUrlSrcGetFromSvrSocket

CUrlSrcGetFromSvrSocket::CUrlSrcGetFromSvrSocket(CUrlSrcFromSvrMgr *pMgr, bool bStart)
{
	m_pMgr = pMgr;
	m_bStart = bStart;
	m_strUrlPath = "";
}

CUrlSrcGetFromSvrSocket::~CUrlSrcGetFromSvrSocket()
{
}


// CUrlSrcGetFromSvrSocket 成员函数

CStringA CUrlSrcGetFromSvrSocket::GetServer()
{
	return "client.stat.verycd.com";
}

CStringA CUrlSrcGetFromSvrSocket::GetUrlPath()
{
	if(m_strUrlPath != "")
		return m_strUrlPath;

	if (::IsBadReadPtr(m_pMgr, sizeof(CUrlSrcFromSvrMgr)))
		return "";

	CStringA	m_strUrlPath;
	CStringA	strHash;
	CStringA	strSize;
	CStringA	strFileName;
	CStringA	strEncodedFileName;

	if( NULL==m_pMgr->m_pAssocPartFile )
		return "";

	try
	{
		//	Hash
		strHash = md4str(m_pMgr->m_pAssocPartFile->GetFileHash());
		strHash.MakeLower();		//必须都为小写。

		//	Size
		char szSize[1024];
		_i64toa(m_pMgr->m_pAssocPartFile->GetFileSize(), szSize,10);
		strSize = szSize;

		//	FileName
		strEncodedFileName = EncodeUrlUtf8(m_pMgr->m_pAssocPartFile->GetFileName());

		if (m_bStart)
			m_strUrlPath.Format("/dl/%s%s/%s/start", strHash, strSize, strEncodedFileName);
		else
			m_strUrlPath.Format("/dl/%s%s/%s/finished", strHash, strSize, strEncodedFileName);

		return m_strUrlPath;
	}
	catch ( ... ) 
	{
		return "";
	}
}

bool CUrlSrcGetFromSvrSocket::ProcessHttpResponse()
{
	if (m_bStart)
		return ProcessHttpResponse_Start();
	else
		return ProcessHttpResponse_Finished();
}

bool CUrlSrcGetFromSvrSocket::ProcessHttpResponseBody(const BYTE* pucData, UINT size)
{
	if (m_bStart)
		return ProcessHttpResponseBody_Start(pucData, size);
	else
		return ProcessHttpResponseBody_Finished(pucData, size);
}

bool CUrlSrcGetFromSvrSocket::ProcessHttpResponse_Start()
{
	int iMajorVer, iMinorVer;
	int iResponseCode;
	char szResponsePhrase[1024];
	sscanf(m_astrHttpHeaders[0], "HTTP/%d.%d %d %s", &iMajorVer, &iMinorVer, &iResponseCode, szResponsePhrase);

	if (thePrefs.GetVerbose())
		AddDebugLogLine(false, _T("Receive UrlSources from server (http response code = %d)"), iResponseCode);

	if (200 != iResponseCode)
		return false;

	return true;
}
bool CUrlSrcGetFromSvrSocket::ProcessHttpResponseBody_Start(const BYTE* pucData, UINT size)
{
	if (thePrefs.GetVerbose())
	{
		AddDebugLogLine(false, _T("Receive UrlSources from server (http response body)"));
		AddDebugLogLine(false, CString((const char*) pucData, size));
	}

	if (::IsBadReadPtr(m_pMgr, sizeof(CUrlSrcFromSvrMgr)))
		return true;

	CStringA	straLine;
	CString		strUrl;
	CString		strRefer;
	LPCSTR		p;
	LPCSTR		pcNl;
	int			iRemain;
	int			iLineLen;
	int			iIndexT;
	int			iSepLen;	//分隔符长度

	p = (LPCSTR) pucData;
	pcNl = NULL;
	iRemain = size;
	while (iRemain > 0)
	{
		pcNl = (LPCSTR) memchr(p, '\n', iRemain);
		if (NULL == pcNl)
		{
			iSepLen = 0;
			iLineLen = iRemain;
		}
		else
		{
			iSepLen = 1;

			if (pcNl > p							// 确保 (pcNl - 1) 可读
				&& *(pcNl - 1) == '\r')				//如果分隔符是“\r\n”，则分隔符长度为 2。
				iSepLen = 2;

			iLineLen = pcNl + 1 - p;
		}

		if (iLineLen <= 0)
			break;

		//	取出一行数据
		int iCopyStrLen = iLineLen - iSepLen;
		char *pCopy = new char[iCopyStrLen + 1];
		strncpy(pCopy, p, iCopyStrLen);
		pCopy[iCopyStrLen] = 0;

		straLine = pCopy;

		delete[] pCopy;
		pCopy = NULL;

		//	把指针移动到下一行
		p += iLineLen;
		iRemain -= iLineLen;

		//	把一行数据解析出来，并添加到列表中。
		iIndexT = straLine.Find('\t');
		if (-1 == iIndexT)
		{
			strUrl = straLine;
			strRefer.Empty();
		}
		else
		{
			strUrl = straLine.Left(iIndexT);
			strRefer = straLine.Mid(iIndexT + 1);
		}

		CStringW	strUrlW;
		strUrlW = strUrl;
		if (S_OK == ::IsValidURL(NULL, strUrlW, 0))
			m_pMgr->m_strlstFetchedUrlSources.AddTail(CString(straLine));	//把整行数据添加至列表，以保存Refer信息。
	}

	if (NULL != m_pMgr->m_pAssocPartFile)
		m_pMgr->m_pAssocPartFile->OnUrlSrcFromSvrFetched(&m_pMgr->m_strlstFetchedUrlSources);

	return true;
}

bool CUrlSrcGetFromSvrSocket::ProcessHttpResponse_Finished()
{
	return false;
}

bool CUrlSrcGetFromSvrSocket::ProcessHttpResponseBody_Finished(const BYTE* /*pucData*/, UINT /*size*/)
{
	return false;
}
