/*
 * $Id: Exceptions.h 4483 2008-01-02 09:19:06Z soarchin $
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

#ifdef _DEBUG
#define	CATCH_DFLT_ALL(fname)
#else
#define	CATCH_DFLT_ALL(fname) \
	catch(...){ \
		if (thePrefs.GetVerbose()) \
			DebugLogError(LOG_STATUSBAR, _T("Unknown exception in ") fname); \
		ASSERT(0); \
	}
#endif

// This type of "last chance" exception handling is to be used at least in several callback functions to avoid memory leaks.
// It is *not* thought as a proper handling of exceptions in general! 
// -> Use explicit exception handlers where needed!
#define CATCH_DFLT_EXCEPTIONS(fname) \
	catch(CException* e){ \
		TCHAR szError[1024]; \
		e->GetErrorMessage(szError, ARRSIZE(szError)); \
		const CRuntimeClass* pRuntimeClass = e->GetRuntimeClass(); \
		LPCSTR pszClassName = (pRuntimeClass) ? pRuntimeClass->m_lpszClassName : NULL; \
		if (!pszClassName) \
			pszClassName = "CException"; \
		if (thePrefs.GetVerbose()) \
			DebugLogError(LOG_STATUSBAR, _T("Unknown %hs exception in ") fname _T(" - %s"), pszClassName, szError); \
		e->Delete(); \
	} \
	catch(CString strError){ \
		if (thePrefs.GetVerbose()) \
			DebugLogError(LOG_STATUSBAR, _T("Unknown CString exception in ") fname _T(" - %s"), strError); \
	}


class CMsgBoxException : public CException
{
	DECLARE_DYNAMIC(CMsgBoxException)
public:
	explicit CMsgBoxException(LPCTSTR pszMsg, UINT uType = MB_ICONWARNING, UINT uHelpID = 0)
	{
		m_strMsg = pszMsg;
		m_uType = uType;
		m_uHelpID = uHelpID;
	}

	CString m_strMsg;
	UINT m_uType;
	UINT m_uHelpID;
};

class CClientException : public CException
{
	DECLARE_DYNAMIC(CClientException)
public:
	CClientException(LPCTSTR pszMsg, bool bDelete)
	{
		m_strMsg = pszMsg;
		m_bDelete = bDelete;
	}

	CString m_strMsg;
	bool m_bDelete;
};
