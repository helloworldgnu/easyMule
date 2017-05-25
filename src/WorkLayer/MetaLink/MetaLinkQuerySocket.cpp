/*
 * $Id: MetaLinkQuerySocket.cpp 7501 2008-09-27 07:44:03Z huby $
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
// MetaLinkQuerySocket.cpp : 实现文件
//

#include "stdafx.h"
#include "emule.h"
#include "MetaLinkQuerySocket.h"
#include "UrlSrcFromSvrMgr.h"
#include "PartFile.h"
#include "StringConversion.h"
#include "Preferences.h"
#include "MD4.h"
#include "MetaLinkParser.h"
#include "MetaLinkPacker.h"
#include "log.h"
#include "version.h"


// CMetaLinkQuerySocket

CMetaLinkQuerySocket::CMetaLinkQuerySocket(CUrlSrcFromSvrMgr *pMgr, bool bStart)
{
	m_pMgr = pMgr;
	m_bStart = bStart;
	m_strUrlPath = "";
}

CMetaLinkQuerySocket::~CMetaLinkQuerySocket()
{
}


// CMetaLinkQuerySocket 成员函数

CStringA CMetaLinkQuerySocket::GetServer()
{
	return "meta.verycd.com";
}

CStringA CMetaLinkQuerySocket::GetUrlPath()
{
	if(m_strUrlPath != "")
		return m_strUrlPath;

	if (::IsBadReadPtr(m_pMgr, sizeof(CUrlSrcFromSvrMgr)))
		return "";

	CStringA	strHash;
	CStringA	strSize;
	CStringA	strFileName;
	CStringA	strEncodedFileName;

	if( NULL==m_pMgr->m_pAssocPartFile )
		return "";

	try
	{
		if(m_bStart)
		{
			SetPost(false);
			if(m_pMgr->m_pAssocPartFile->HasNullHash())
			{
				strFileName = m_pMgr->m_pAssocPartFile->GetPartFileURL();
				ParseRef(strFileName);
				if(strFileName == "")
					throw;

				CMD4 md4;
				md4.Add((const CHAR *)strFileName, strFileName.GetLength());
				md4.Finish();
				strHash = md4str(md4.GetHash());
				strHash.MakeLower();
				m_strUrlPath.Format("/app/emule/http/%s/%s", strHash, CStringA(EncodeUrlUtf8(CString(strFileName))));
			}
			else
			{
				//	Hash
				strHash = md4str(m_pMgr->m_pAssocPartFile->GetFileHash());
				strHash.MakeLower();		//必须都为小写。

				//	Size
				char szSize[1024];
				_i64toa(m_pMgr->m_pAssocPartFile->GetFileSize(), szSize,10);
				strSize = szSize;

				//	FileName
				strEncodedFileName = URLEncode(EncodeUrlUtf8(m_pMgr->m_pAssocPartFile->GetFileName()));

				//	construct UrlPath
				m_strUrlPath.Format("/app/emule/metalink/%s%s/%s.metalink", strHash, strSize, strEncodedFileName);

			}
		}
		else
		{			
			uint64 uSize = m_pMgr->m_pAssocPartFile->GetFileSize();
			if(uSize == 0)
				return "";
			CStringA strFileName = (CStringA)m_pMgr->m_pAssocPartFile->GetPartFileURL();
			ParseRef(strFileName);

			if( strFileName.GetLength() ) // http,ftp任务
			{				
				CMD4 md4;
				md4.Add((const CHAR *)strFileName, strFileName.GetLength());
				md4.Finish();
				strHash = md4str(md4.GetHash());
				strHash.MakeLower();
				m_strUrlPath.Format("/app/emule/metalink/finish/http/%s/%s", strHash, CStringA(EncodeUrlUtf8(CString(strFileName))));
			} 
			else // ed2k任务
			{
				char szSize[1024];
				_i64toa(m_pMgr->m_pAssocPartFile->GetFileSize(), szSize,10);
				strSize = szSize;				
				m_strUrlPath.Format("/app/emule/metalink/finish/%s%s", (const char*)md4strA( m_pMgr->m_pAssocPartFile->GetFileHash() ) , (const char*)strSize );
			}

			SetPost( m_pMgr->m_pAssocPartFile->NeedPostUrlSiteToMetaServer() );

			if( m_bIsPost )
			{
				CMetaLinkPacker packer;
				packer.SetFileName(CStringA(strFileName));
				packer.SetFileSize(uSize);
				packer.AddResource("ed2k", "", CStringA(CreateED2kLink(m_pMgr->m_pAssocPartFile)), -2);

				CList <CUrlSite*>* slSrc = &m_pMgr->m_pAssocPartFile->m_UrlSiteList;
				POSITION	pos;
				CStringA	url, ref = "";
				pos = slSrc->GetHeadPosition();
				CString strMetaUrlInfo;
				int iUrlPref;
				while (NULL != pos)
				{
					CUrlSite* urlSite = slSrc->GetNext(pos);

					if(!urlSite->m_bNeedCommitted)
						continue;

					iUrlPref = urlSite->m_bBadSite ? -1 : min(100,(int)(urlSite->m_dwDataTransferedWithoutPayload * 100 / uSize + 0.9)); 
					url = urlSite->m_strUrl;					
					ref = ParseRef(url);					
					packer.AddResource("http", ref, url, iUrlPref);
					
					strMetaUrlInfo.Format( _T("Send Url to MetaServer : %s [pref=%d] "), CString(url), iUrlPref );
					m_pMgr->m_pAssocPartFile->AddFileLog( new CTraceServerMessage(strMetaUrlInfo) );
				}
				if( !m_pMgr->m_pAssocPartFile->m_bIsSafeDrmFile )
				{
					CStringA sReporterName;
					sReporterName.Format("easyMule.%u.%u.%u",EASYMULE_MJR,EASYMULE_MIN,EASYMULE_UPDATE);
					packer.AddReport("DRM",CStringA(sReporterName),NULL);
				}

				const CHAR * szXml = packer.GetXml();
				SetPostData(CStringA("metalink=") + CStringA(URLEncode(EncodeUrlUtf8(CString(szXml)))));
				delete szXml;
			}
		}
		return m_strUrlPath;
	}
	catch ( ... ) 
	{
		return "";
	}
}

bool CMetaLinkQuerySocket::ProcessHttpResponse()
{
	if (m_bStart)
		return ProcessHttpResponse_Start();
	else
		return ProcessHttpResponse_Finished();
}

bool CMetaLinkQuerySocket::ProcessHttpResponseBody(const BYTE* pucData, UINT size)
{
	if (m_bStart)
		return ProcessHttpResponseBody_Start(pucData, size);
	else
		return ProcessHttpResponseBody_Finished(pucData, size);
}

bool CMetaLinkQuerySocket::ProcessHttpResponse_Start()
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

static int GzipDecompress(Bytef *&dest, uLongf *destLen, const Bytef *source, uLong sourceLen)
{
	int err;
	z_stream stream = {0};
	stream.zalloc = (alloc_func)0;
	stream.zfree = (free_func)0;
	stream.opaque = (voidpf)0;

	err = inflateInit2(&stream, -MAX_WBITS);
	if (err != Z_OK)
		return err;
	*destLen = *(uLong *)&source[sourceLen - 4];
	dest = new Bytef[*destLen + 1];

	stream.next_in = (Bytef*) source + 10 ;
	stream.avail_in = (uInt) sourceLen - 18;
	stream.next_out = (Bytef*) dest;
	stream.avail_out = *destLen;
	// doit
	err = inflate(&stream, Z_FINISH);
	if (err != Z_STREAM_END)
	{
		inflateEnd(&stream);
		return err;
	}
	err = inflateEnd(&stream);
	dest[*destLen] = 0;

	return err;
}

bool CMetaLinkQuerySocket::ProcessHttpResponseBody_Start(const BYTE* pucData, UINT size)
{
	if (thePrefs.GetVerbose())
	{
		AddDebugLogLine(false, _T("Receive UrlSources from server (http response body)"));
		AddDebugLogLine(false, CString((const char*) pucData, size));
	}

	if (::IsBadReadPtr(m_pMgr, sizeof(CUrlSrcFromSvrMgr)))
		return true;

	CMetaLinkParser * parser = NULL;
	if(pucData[0] == 0x1f && pucData[1] == 0x8b)
	{
		Bytef * dest = NULL;
		uLong dlen;
		GzipDecompress(dest, &dlen, pucData, size);
		if(dest != NULL)
		{
			parser = new CMetaLinkParser((const CHAR *)dest);
			delete []dest;
		}
	}
	else
		parser = new CMetaLinkParser((const CHAR *)pucData);

	if (parser != NULL && NULL != m_pMgr->m_pAssocPartFile && parser->GetFileCount() > 0)
		m_pMgr->m_pAssocPartFile->AddMetalinkSource(parser);

	delete parser;

	return true;
}

bool CMetaLinkQuerySocket::ProcessHttpResponse_Finished()
{
	return false;
}

bool CMetaLinkQuerySocket::ProcessHttpResponseBody_Finished(const BYTE* /*pucData*/, UINT /*size*/)
{
	return false;
}
