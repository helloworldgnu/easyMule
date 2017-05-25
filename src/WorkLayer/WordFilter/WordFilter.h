/*
 * $Id: WordFilter.h 4483 2008-01-02 09:19:06Z soarchin $
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
#include <tchar.h>

#ifndef WORDFILTER_H
#define WORDFILTER_H

/////////////////////////////////////////////////////////////////////////////
//
// WordFilter.h : interface of the CWordFilter, used to filter limited words
//                Added by Soar Chin
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define FLITER_FILE _T("wordfilter.txt")

class CWordFilter
{
private:
	int		m_count;			// Count of limited words
	TCHAR **	m_filterwords;	// Limited words for filter
	int **	m_kmpvalue;			// KMP Next Value
	bool	m_filterall;
	void Free();
public:
	CWordFilter():
		m_count(0), m_filterwords(NULL), m_kmpvalue(NULL), m_filterall(false) {} // Init values
	~CWordFilter();
	void	Init();				// Init class and read from data file
	bool	VerifyString(const CString & sString);	// Verify if the string has limited words
};

extern CWordFilter WordFilter;

#endif // WORDFILTER_H
