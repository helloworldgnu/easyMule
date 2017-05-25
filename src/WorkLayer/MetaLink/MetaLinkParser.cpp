/* 
 * $Id: MetaLinkParser.cpp 4490 2008-01-03 05:58:24Z soarchin $
 * 
 * this file is part of easyMule
 * Copyright (C)2002-2007 VeryCD Dev Team ( strEmail.Format("%s@%s", "devteam", "easymule.org") / http://www.easymule.org )
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

// MetaLinkParser.cpp Added by Soar Chin (8/17/2007)
#include "stdafx.h"
#include "MetaLinkParser.h"
#include <string.h>
#include "StringConversion.h"

CMetaLinkParser::CMetaLinkParser(CString strFilename)
{
	m_fileList.RemoveAll();
	m_doc = NULL;
	m_data = NULL;
	CFile file;
	if(!file.Open(strFilename, CFile::shareDenyNone))
		return;
	UINT len = (UINT)file.GetLength();
	try
	{
		m_data = new CHAR[len + 1];
		file.Read(m_data, len);
		m_data[len] = 0;
	}
	catch(...)
	{
		m_data = NULL;
	}
	if(m_data != NULL && !parseXml())
	{
		delete m_data;
		this->m_data = 0;
	}
}

CMetaLinkParser::CMetaLinkParser(const CHAR * data)
{
	m_doc = NULL;
	INT len = strlen(data);
	try
	{
		m_data = new CHAR[len + 1];
		strcpy(m_data, data);
	}
	catch(...)
	{
		m_data = NULL;
	}
	if(m_data != NULL && !parseXml())
	{
		delete[] m_data;
		this->m_data = 0;
	}
}

CMetaLinkParser::~CMetaLinkParser()
{
	if(m_doc != NULL)
		delete m_doc;
	if(m_data != NULL)
		delete[] m_data;
	for(INT i = 0; i < m_fileList.GetCount(); i ++)
	{
		delete m_fileList.GetAt(i);
	}
	m_fileList.RemoveAll();
}

__inline UINT64 StrToInt(const CHAR * str)
{
	if(str == NULL)
		return 0;
	return _atoi64(str);
}


BOOL CMetaLinkParser::parseXml()
{
	m_errorMsg = "";
	m_errorCode = METALINK_OK;
	m_fileList.RemoveAll();
	try
	{
		m_doc = new TiXmlDocument;
		m_doc->Parse(m_data, 0, TIXML_ENCODING_UTF8);
	}
	catch (...)
	{
		m_errorMsg = "Failed to create TiXmlDocument!";
		m_errorCode = METALINK_MEM_ERROR;
		return FALSE;
	}

	TiXmlNode* node = m_doc->FirstChild("metalink");
	if(node == NULL)
	{
		m_errorCode = METALINK_PARSE_ERROR;
		m_errorMsg = "metalink element is not found!\n";
		return FALSE;
	}
	node = node->FirstChild("files");
	if(node == NULL)
	{
		m_errorCode = METALINK_PARSE_ERROR;
		m_errorMsg = "files element is not found!\n";
		return FALSE;
	}
	node = node->FirstChild("file");
	if(node == NULL)
	{
		m_errorCode = METALINK_PARSE_ERROR;
		m_errorMsg = "file element is not found!\n";
		return FALSE;
	}
	do
	{
		CMetaLinkFile * mlfile;
		try
		{
			mlfile = new CMetaLinkFile;
		}
		catch (...)
		{
			m_errorMsg = "Failed to create CMetaLinkFile!";
			m_errorCode = METALINK_MEM_ERROR;
			return FALSE;
		}
		TiXmlElement * elem = node->ToElement();

		if(elem == NULL)
			continue;
		const CHAR * name;
		if((name = elem->Attribute("name")) != NULL)
		{
			mlfile->m_strFilename = OptUtf8ToStr(name);
		}
		TiXmlNode * node2 = node->FirstChild("size");
		if(node2 == NULL || (node2 = node2->FirstChild()) == NULL)
			mlfile->m_uFilesize = 0;
		else
			mlfile->m_uFilesize = StrToInt(node2->Value());

		// Parse verifications
		node2 = node->FirstChild("verification");
		if(node2 != NULL)
		{
			// Parse hash
			TiXmlNode * node3 = node2->FirstChild("hash");
			while(node3 != NULL)
			{
				SMetaLinkHash * hash;
				try
				{
					hash = new SMetaLinkHash;
				}
				catch (...)
				{
					m_errorMsg = "Failed to create SMetaLinkHash!";
					m_errorCode = METALINK_MEM_ERROR;
					return FALSE;
				}
				elem = node3->ToElement();
				if(elem == NULL)
					continue;
				if(node3->FirstChild() == NULL)
					continue;
				if(node3->FirstChild() == NULL)
				{
					delete hash;
				}
				else
				{
					if((name = elem->Attribute("type")) != NULL)
						hash->strType = OptUtf8ToStr(name);
					hash->strHash = OptUtf8ToStr(node3->FirstChild()->Value());
					mlfile->m_hashList.Add(hash);
				}
				node3 = node3->NextSibling("hash");
			}
			// Parse piece hash
			node3 = node2->FirstChild("pieces");
			while(node3 != NULL)
			{
				elem = node3->ToElement();
				if(elem == NULL)
					continue;
				if(elem->Attribute("type") == NULL || strcmp(elem->Attribute("type"), "sha1") != 0)
					continue;
				mlfile->m_pieceSize = (INT)StrToInt(elem->Attribute("length"));
				TiXmlNode * node4 = node3->FirstChild("hash");
				while(node4 != NULL)
				{
					elem = node4->ToElement();
					if(elem == NULL)
						continue;
					if(node4->FirstChild() == NULL)
						continue;
					name = elem->Attribute("piece");
					if(name == NULL || node4->FirstChild() == NULL)
						continue;
					mlfile->m_piecehashList.SetAtGrow((INT)StrToInt(name), OptUtf8ToStr(node4->FirstChild()->Value()));
					node4 = node4->NextSibling("hash");
				}
				node3 = node3->NextSibling("pieces");
			}
		}

		// Parse resources
		node2 = node->FirstChild("resources");
		if(node2 != NULL)
		{
			elem = node2->ToElement();
			if(elem == NULL)
				continue;
			const CHAR * s;
			if((s = elem->Attribute("maxconnections")) != NULL)
				mlfile->m_nMaxConn = (INT)StrToInt(s);
			else
				mlfile->m_nMaxConn = 0;
			TiXmlNode * node3;
			for(node3 = node2->FirstChild("url"); node3 != NULL; node3 = node3->NextSibling("url"))
			{
				if(node3->FirstChild() == NULL)
					continue;
				elem = node3->ToElement();
				if(elem == NULL)
					continue;
				SMetaLinkURL * url;
				try
				{
					url = new SMetaLinkURL;
				}
				catch (...)
				{
					m_errorMsg = "Failed to create CMetaLinkURL!";
					m_errorCode = METALINK_MEM_ERROR;
					return FALSE;
				}
				url->strUrl = OptUtf8ToStr(node3->FirstChild()->Value());
				if((s = elem->Attribute("type")) == NULL)
				{
					INT nPos = url->strUrl.Find(':');
					if(nPos >= 0)
						url->strType = url->strUrl.Left(nPos);
					else
						url->strType = "unknown";
				}
				else
				{
					url->strType = OptUtf8ToStr(s);
				}
				if((s = elem->Attribute("referrer")) != NULL)
				{
					url->strUrl = url->strUrl + _T("<referer=") + CString(s) + _T(">");
				}
				if((s = elem->Attribute("maxconnections")) != NULL)
					url->nMaxConn = (INT)StrToInt(s);
				else
					url->nMaxConn = 0;
				if((s = elem->Attribute("preference")) != NULL)
					url->nPreference = (INT)StrToInt(s);
				else
					url->nPreference = 50;
				if((s = elem->Attribute("location")) != NULL)
					url->strLocation = OptUtf8ToStr(s);
				else
					url->strLocation = "unknown";
				mlfile->m_urlList.Add(url);
			}
		}
		m_fileList.Add(mlfile);
	}
	while((node = node->NextSibling("file")) != NULL);

	return TRUE;
}

INT CMetaLinkParser::GetFileCount()
{
	return m_fileList.GetCount();
}

CMetaLinkFile * CMetaLinkParser::GetFile( INT nIndex )
{
	ASSERT(nIndex >=0 && nIndex < m_fileList.GetCount());
	return m_fileList.GetAt(nIndex);
}

CString CMetaLinkParser::GetErrorMsg()
{
	return m_errorMsg;
}

INT CMetaLinkParser::GetErrorCode()
{
	return m_errorCode;
}

CMetaLinkFile::CMetaLinkFile()
{
	m_strFilename = "";
	m_hashList.RemoveAll();
	m_pieceSize = 0;
	m_nMaxConn = 0;
	m_uFilesize = 0;
	m_piecehashList.RemoveAll();
	m_urlList.RemoveAll();
}

CMetaLinkFile::~CMetaLinkFile()
{
	INT i;
	for(i = 0; i < m_hashList.GetCount(); i ++)
	{
		SMetaLinkHash * hash = m_hashList.GetAt(i);
		if(hash != NULL)
			delete hash;
	}
	m_hashList.RemoveAll();
	for(i = 0; i < m_urlList.GetCount(); i ++)
	{
		delete m_urlList.GetAt(i);
	}
	m_urlList.RemoveAll();
	m_piecehashList.RemoveAll();
}

INT CMetaLinkFile::GetHashCount()
{
	return m_hashList.GetCount();
}

INT CMetaLinkFile::GetSHA1PieceCount()
{
	return m_piecehashList.GetCount();
}

INT CMetaLinkFile::GetURLCount()
{
	return m_urlList.GetCount();
}

SMetaLinkHash * CMetaLinkFile::GetHash( INT nIndex )
{
	ASSERT(nIndex >= 0 && nIndex < GetHashCount());
	return m_hashList.GetAt(nIndex);
}

INT CMetaLinkFile::GetSHA1PieceSize()
{
	return m_pieceSize;
}

CString CMetaLinkFile::GetSHA1Piece( INT nIndex )
{
	ASSERT(nIndex >= 0 && nIndex < GetSHA1PieceCount());
	return m_piecehashList.GetAt(nIndex);
}

SMetaLinkURL * CMetaLinkFile::GetURL( INT nIndex )
{
	ASSERT(nIndex >= 0 && nIndex < GetURLCount());
	return m_urlList.GetAt(nIndex);
}

CString CMetaLinkFile::GetFileName()
{
	return m_strFilename;
}

UINT64 CMetaLinkFile::GetFileSize()
{
	return m_uFilesize;
}

INT CMetaLinkFile::GetMaxConn()
{
	return m_nMaxConn;
}
