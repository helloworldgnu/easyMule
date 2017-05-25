/*
 * $Id: SearchExpr.h 4483 2008-01-02 09:19:06Z soarchin $
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

#include "Opcodes.h"

enum ESearchOperators
{
	SEARCHOP_AND,
	SEARCHOP_OR,
	SEARCHOP_NOT
};

#define	SEARCHOPTOK_AND	"\255AND"
#define	SEARCHOPTOK_OR	"\255OR"
#define	SEARCHOPTOK_NOT	"\255NOT"

CString OptUtf8ToStr(const CStringA& rastr);


class CSearchAttr
{
public:
	CSearchAttr()
	{
		m_iTag = FT_FILENAME;
		(void)m_str;
		m_uIntegerOperator = ED2K_SEARCH_OP_EQUAL;
		m_nNum = 0;
	}

	CSearchAttr(LPCSTR pszString)
	{
		m_iTag = FT_FILENAME;
		m_str = pszString;
		m_uIntegerOperator = ED2K_SEARCH_OP_EQUAL;
		m_nNum = 0;
	}

	CSearchAttr(const CStringA* pstrString)
	{
		m_iTag = FT_FILENAME;
		m_str = *pstrString;
		m_uIntegerOperator = ED2K_SEARCH_OP_EQUAL;
		m_nNum = 0;
	}

	CSearchAttr(int iTag, UINT uIntegerOperator, uint64 nSize)
	{
		m_iTag = iTag;
		(void)m_str;
		m_uIntegerOperator = uIntegerOperator;
		m_nNum = nSize;
	}

	CSearchAttr(int iTag, LPCSTR pszString)
	{
		m_iTag = iTag;
		m_str = pszString;
		m_uIntegerOperator = ED2K_SEARCH_OP_EQUAL;
		m_nNum = 0;
	}

	CSearchAttr(int iTag, const CStringA* pstrString)
	{
		m_iTag = iTag;
		m_str = *pstrString;
		m_uIntegerOperator = ED2K_SEARCH_OP_EQUAL;
		m_nNum = 0;
	}

	CString DbgGetAttr() const
	{
		CString strDbg;
		switch (m_iTag)
		{
			case FT_FILESIZE:
			case FT_SOURCES:
			case FT_COMPLETE_SOURCES:
			case FT_FILERATING:
			case FT_MEDIA_BITRATE:
			case FT_MEDIA_LENGTH:
				strDbg.AppendFormat(_T("%s%s%I64u"), DbgGetFileMetaTagName(m_iTag), DbgGetSearchOperatorName(m_uIntegerOperator), m_nNum);
				break;
			case FT_FILETYPE:
			case FT_FILEFORMAT:
			case FT_MEDIA_CODEC:
			case FT_MEDIA_TITLE:
			case FT_MEDIA_ALBUM:
			case FT_MEDIA_ARTIST:
				ASSERT( m_uIntegerOperator == ED2K_SEARCH_OP_EQUAL );
				strDbg.AppendFormat(_T("%s=%s"), DbgGetFileMetaTagName(m_iTag), OptUtf8ToStr(m_str));
				break;
			default:
				ASSERT( m_iTag == FT_FILENAME );
				strDbg.AppendFormat(_T("\"%s\""), OptUtf8ToStr(m_str));
		}
		return strDbg;
	}

	int m_iTag;
	CStringA m_str;
	UINT m_uIntegerOperator;
	uint64 m_nNum;
};


class CSearchExpr
{
public:
	CSearchExpr()
	{
		(void)m_aExpr;
	}

	CSearchExpr(const CSearchAttr* pAttr)
	{
		m_aExpr.Add(*pAttr);
	}
	
	void Add(ESearchOperators eOperator)
	{
		if (eOperator == SEARCHOP_OR)
			m_aExpr.Add(SEARCHOPTOK_OR);
		else if (eOperator == SEARCHOP_NOT)
			m_aExpr.Add(SEARCHOPTOK_NOT);
		else {
			ASSERT( eOperator == SEARCHOP_AND );
			m_aExpr.Add(SEARCHOPTOK_AND);
		}
	}

	void Add(const CSearchAttr* pAttr)
	{
		m_aExpr.Add(*pAttr);
	}

	void Add(const CSearchExpr* pExpr)
	{
		m_aExpr.Append(pExpr->m_aExpr);
	}

	CArray<CSearchAttr, CSearchAttr> m_aExpr;
};
