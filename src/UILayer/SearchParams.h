/*
 * $Id: SearchParams.h 9297 2008-12-24 09:55:04Z dgkang $
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

class CSafeMemFile;


///////////////////////////////////////////////////////////////////////////////
// ESearchType

enum ESearchType
{
	//NOTE: The numbers are *equal* to the entries in the comboxbox -> TODO: use item data
	SearchTypeAutomatic = 0,
	SearchTypeEd2kServer,
	SearchTypeEd2kGlobal,
	SearchTypeKademlia,
	SearchTypeVeryCD,	//Added by thilon on 2006.09.05
	SearchTypeFileDonkey
};


#define	MAX_SEARCH_EXPRESSION_LEN	512

///////////////////////////////////////////////////////////////////////////////
// SSearchParams

struct SSearchParams
{
	SSearchParams()
	{
		dwSearchID = (DWORD)-1;
		eType = SearchTypeEd2kServer;
		bClientSharedFiles = false;
		ullMinSize = 0;
		ullMaxSize = 0;
		uAvailability = 0;
		uComplete = 0;
		ulMinBitrate = 0;
		ulMinLength = 0;
		bMatchKeywords = false;
		bUnicode = true;
	}
	DWORD dwSearchID;
	bool bClientSharedFiles;
	CString strSearchTitle;
	CString strExpression;
	CString strKeyword;
	CString strBooleanExpr;
	ESearchType eType;
	CStringA strFileType;
	CString strMinSize;
	uint64 ullMinSize;
	CString strMaxSize;
	uint64 ullMaxSize;
	UINT uAvailability;
	CString strExtension;
	UINT uComplete;
	CString strCodec;
	ULONG ulMinBitrate;
	ULONG ulMinLength;
	CString strTitle;
	CString strAlbum;
	CString strArtist;
	CString strSpecialTitle;
	bool bMatchKeywords;
	bool bUnicode;
};

bool GetSearchPacket(CSafeMemFile* data, SSearchParams* pParams, bool bTargetSupports64Bit, bool* pbPacketUsing64Bit);
