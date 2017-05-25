/* 
 * $Id: MetaLinkParser.h 4490 2008-01-03 05:58:24Z soarchin $
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

// MetaLinkParser.h Added by Soar Chin (8/17/2007)
#pragma once

#include "TinyXml.h"

#define METALINK_OK				0
#define METALINK_PARSE_ERROR	-1
#define METALINK_MEM_ERROR		-2

struct SMetaLinkURL
{
	CString strUrl;		// URL address
	CString strType;	// URL type (http/ftp/bittorrent/ed2k/gnutella)
	INT nMaxConn;		// Max connections allowed for this download, 0 for unlimited
	INT nPreference;	// Preference of source
	CString strLocation;// Location of source
};

struct SMetaLinkHash
{
	CString strType;	// Hash type, support list in MetaLink protocol:  md4 md5 sha1 sha256 sha384 sha512 rmd160 tiger crc32 ed2k gnunet
	CString strHash;	// Base16 encoded hash string
};

class CMetaLinkFile
{
private:
	friend class CMetaLinkParser;
	CString m_strFilename;									// Filename
	UINT64 m_uFilesize;										// Filesize
	INT m_nMaxConn;											// Max connections allowed counted for all resources, 0 for unlimited
	CTypedPtrArray<CPtrArray, SMetaLinkHash *> m_hashList;	// Hash list
	INT m_pieceSize;										// Size of each SHA-1 piece, zero for no SHA-1 piece hash found
	CStringArray m_piecehashList;							// Base16 encoded SHA-1 piece hash list
	CTypedPtrArray<CPtrArray, SMetaLinkURL *> m_urlList;	// URL list
public:
	CMetaLinkFile();						// Constructor
	~CMetaLinkFile();						// Destructor
	CString GetFileName();					// Get filename
	UINT64 GetFileSize();					// Get size of file
	INT GetMaxConn();						// Get max connections allowed
	INT GetHashCount();						// Get size of hash list
	INT GetSHA1PieceCount();				// Get size of SHA-1 piece hash list
	INT GetURLCount();						// Get size of URL list
	SMetaLinkHash * GetHash(INT nIndex);	// Query indexed SMetaLinkHash
	INT GetSHA1PieceSize();					// Get size of each SHA-1 piece
	CString GetSHA1Piece(INT nIndex);		// Query indexed SHA-1 piece hash
	SMetaLinkURL * GetURL(INT nIndex);		// Query indexed SMetaLinkURL
};

class CMetaLinkParser
{
private:
	CHAR * m_data;											// data for parsing
	TiXmlDocument * m_doc;									// TinyXml document object
	INT m_errorCode;										// Error code
	CString m_errorMsg;										// Error message
	BOOL parseXml();										// inline Xml parse function
	CTypedPtrArray<CPtrArray, CMetaLinkFile *> m_fileList;	// File list parsed out
public:
	CMetaLinkParser(CString strFilename);	// Constructor
	CMetaLinkParser(const CHAR * data);		// Constructor
	~CMetaLinkParser();						// Destructor
	INT GetErrorCode();						// Query error code
	CString GetErrorMsg();					// Query error message
	INT GetFileCount();						// Get count of files
	CMetaLinkFile * GetFile(INT nIndex);	// Query indexed CMetaLinkFile
};
