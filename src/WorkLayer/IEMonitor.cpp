/* 
 * $Id: IEMonitor.cpp 9073 2008-12-18 04:38:51Z dgkang $
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

// IEMonitor.cpp Added by Soar Chin (8/31/2007)

#include "StdAfx.h"
#include ".\iemonitor.h"
#include "Preferences.h"
#include "otherfunctions.h"
#include "resource.h"

bool CIEMonitor::m_bFirstRun = true;
bool CIEMonitor::m_bIEMenu = true;
bool CIEMonitor::m_bMonitor = false;
bool CIEMonitor::m_bEd2k = true;
bool CIEMonitor::m_bAlt = false;
WORD CIEMonitor::m_wLangID = 0;

CIEMonitor::CIEMonitor(void)
{
}

CIEMonitor::~CIEMonitor(void)
{
}

BOOL CIEMonitor::RegisterLibrary(LPCTSTR szName)
{
	typedef HRESULT (_stdcall *FNDLLRS)(void);

	HMODULE hLib = ::LoadLibrary(szName);
	if(hLib == NULL)
		return FALSE;
	FNDLLRS m_pfnRegServer = (FNDLLRS)GetProcAddress(hLib, "DllRegisterServer");
	if(m_pfnRegServer == NULL)
	{
		FreeLibrary(hLib);
		return FALSE;
	}
	m_pfnRegServer();

	FreeLibrary(hLib);
	return TRUE;
}

BOOL CIEMonitor::UnregisterLibrary(LPCTSTR szName)
{
	typedef HRESULT (_stdcall *FNDLLRS)(void);

	HMODULE hLib = ::LoadLibrary(szName);
	if(hLib == NULL)
		return FALSE;
	FNDLLRS m_pfnUnregServer = (FNDLLRS)GetProcAddress(hLib, "DllUnregisterServer");
	if(m_pfnUnregServer == NULL)
	{
		FreeLibrary(hLib);
		return FALSE;
	}
	m_pfnUnregServer();

	FreeLibrary(hLib);
	return TRUE;
}

void CIEMonitor::ApplyChanges( void )
{
	BOOL bNeedReg = FALSE;
	if(m_bFirstRun)
	{
		bNeedReg = CheckForUpdate(thePrefs.GetMuleDirectory(EMULE_MODULEDIR) + _T("IE2EM.dll"));
	}
	if(bNeedReg || !IsRegistered())
		RegisterAll();
	if(m_bFirstRun || m_bIEMenu != thePrefs.GetAddToIEMenu() || m_bMonitor != thePrefs.GetMonitorLinks() || m_bEd2k != thePrefs.GetMonitorEd2k() || m_wLangID != thePrefs.GetLanguageID())
	{
		m_bIEMenu = thePrefs.GetAddToIEMenu();
		m_bMonitor = thePrefs.GetMonitorLinks();
		m_bEd2k = thePrefs.GetMonitorEd2k();
		m_wLangID = thePrefs.GetLanguageID();
		CRegKey regkey;
		CRegKey subkey;
		regkey.Create(HKEY_CURRENT_USER, _T("Software\\easyMule"));
		TCHAR szPath[512];
		DWORD count = 512;
		if(regkey.QueryStringValue(_T("InstallPath"), szPath, &count) != ERROR_SUCCESS)
		{
			_tcscpy(szPath, thePrefs.GetMuleDirectory(EMULE_EXECUTEABLEDIR));
			INT len = _tcslen(szPath);
			if(len > 3 && szPath[len - 1] == '\\')
				szPath[len - 1] = 0;
			regkey.SetStringValue(_T("InstallPath"), szPath);
		}
		regkey.Close();

		CString strPath = thePrefs.GetMuleDirectory(EMULE_EXECUTEABLEDIR) + _T("IE2EM.htm");
		regkey.Create(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Internet Explorer\\MenuExt"));
		TCHAR szName[1024];
		DWORD dwLen = 1024;
		BOOL bFound = FALSE;
		for(INT i = 0; regkey.EnumKey(i, szName, &dwLen) != ERROR_NO_MORE_ITEMS; i ++)
		{
			subkey.Open(regkey, szName);
			TCHAR szValue[1024];
			ULONG uLen = 1024;
			subkey.QueryStringValue(NULL, szValue, &uLen);
			if(strPath == szValue)
			{
				bFound = TRUE;
				break;
			}
			subkey.Close();
			dwLen = 1024;
		}
		if(bFound)
		{
			subkey.Close();
			regkey.RecurseDeleteKey(szName);
		}
		if(m_bIEMenu)
		{
			subkey.Create(regkey, GetResString(IDS_IEMENUEXT));
			subkey.SetStringValue(NULL, strPath);
			subkey.SetDWORDValue(_T("Contexts"), 0x22);
			subkey.Close();
		}
		regkey.Close();
		regkey.Create(HKEY_CURRENT_USER, _T("Software\\easyMule"));
		if(m_bMonitor)
			regkey.SetDWORDValue(_T("Monitor"), 1);
		else
			regkey.SetDWORDValue(_T("Monitor"), 0);
		regkey.Close();
	}
	m_bFirstRun = false;
}

BOOL CIEMonitor::CheckForUpdate( CString realpath )
{
	CString oldpath = realpath + _T(".old");
	CString newpath = realpath + _T(".new");
	if(!PathFileExists(newpath))
		return FALSE;
	if(PathFileExists(realpath))
	{
		if(PathFileExists(oldpath))
		{
			if(_tremove(oldpath) != 0)
				return FALSE;
		}
		if(_trename(realpath, oldpath) != 0)
		{
			return FALSE;
		}
		UnregisterLibrary(realpath);
	}
	_trename(newpath, realpath);
	return TRUE;
}

void CIEMonitor::RegisterAll( void )
{
	RegisterLibrary(thePrefs.GetMuleDirectory(EMULE_MODULEDIR) + _T("IE2EM.dll"));
}

BOOL CIEMonitor::IsRegistered( void )
{
	CRegKey checkkey;
	return checkkey.Open(HKEY_CLASSES_ROOT, _T("CLSID\\{A0867FD1-79E7-456C-8B41-165A2504FD86}")) == ERROR_SUCCESS &&
		   checkkey.Open(HKEY_CLASSES_ROOT, _T("CLSID\\{48618374-565F-4CA0-B8CD-6F496C997FAF}")) == ERROR_SUCCESS &&
		   checkkey.Open(HKEY_CLASSES_ROOT, _T("CLSID\\{0A0DDBD3-6641-40B9-873F-BBDD26D6C14E}")) == ERROR_SUCCESS &&
		   checkkey.Open(HKEY_CLASSES_ROOT, _T("IE2EM.IE2EMUrlTaker")) == ERROR_SUCCESS &&
		   checkkey.Open(HKEY_LOCAL_MACHINE, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Browser Helper Objects\\{0A0DDBD3-6641-40B9-873F-BBDD26D6C14E}")) == ERROR_SUCCESS &&
		   IsPathOk();
}

BOOL CIEMonitor::IsPathOk( void )
{
	CRegKey checkkey;
	if(checkkey.Open(HKEY_CLASSES_ROOT, _T("CLSID\\{48618374-565F-4CA0-B8CD-6F496C997FAF}\\InprocServer32")) != ERROR_SUCCESS)
		return FALSE;
	TCHAR szVal[MAX_PATH];
	ULONG len = MAX_PATH;
	checkkey.QueryStringValue(NULL, szVal, &len);

	//VC-dgkang 2008年6月11日
	//应该忽略字符串大小写敏感比较

	CString tcs;
	tcs = thePrefs.GetMuleDirectory(EMULE_MODULEDIR) + _T("IE2EM.dll");
	return !tcs.CompareNoCase(szVal);

	//return thePrefs.GetMuleDirectory(EMULE_MODULEDIR) + _T("IE2EM.dll") == szVal;
}
