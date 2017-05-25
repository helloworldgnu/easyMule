/*
 * $Id: CheckConflict.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
#include "StdAfx.h"
#include <strsafe.h>
#include ".\checkconflict.h"
#include "ini2.h"
#include "emule.h"

const CCheckConflict::CONFILCT_MOD_ENTRY	CCheckConflict::s_conflictModuleList[] = {

	_T("Fortress.dll"), _T("山丽网络堡垒"), 1, CCheckConflict::ThirdPartyError,
	_T("KB8964115.log"), _T("未知"), 1, CCheckConflict::ThirdPartyError,
	_T("ESPI11.dll"), _T("未知"), 1, CCheckConflict::ThirdPartyError,
	_T("asycfilt.dll"), _T("未知"), 1, CCheckConflict::ThirdPartyError,
//	_T("jscript.dll"), _T("未知"), 1, CCheckConflict::ThirdPartyError,
	_T("imon.dll"), _T("未知"), 1, CCheckConflict::ThirdPartyError,
//	_T("uxtheme.dll"), _T("未知"), 1, CCheckConflict::ThirdPartyError,
	_T("nvappfilter.dll"), _T("未知"), 1, CCheckConflict::ThirdPartyError,
	_T("dictnt.dll"), _T("未知"), 1, CCheckConflict::ThirdPartyError,
	_T("iKeeper.dll"), _T("网吧管理系统"), 1, CCheckConflict::ThirdPartyError,

	
	_T("unispim.ime"), _T("紫光输入法"), 1, CCheckConflict::ThirdPartyError,
	_T("jpwb.IME"), _T("极品五笔输入法"), 1, CCheckConflict::ThirdPartyError,
	_T("PINTLGNT.IME"), _T("输入法"), 1, CCheckConflict::ThirdPartyError,
	_T("winwb86.IME"), _T("Windows五笔输入法86"), 1, CCheckConflict::ThirdPartyError,
	_T("WINABC.IME"), _T("ABC输入法"), 1, CCheckConflict::ThirdPartyError,
	

	_T("mshtml.dll"), _T("WebBrowser"), 0, CCheckConflict::WebBrowserProblem,
	_T("urlmon.dll"), _T("WebBrowser"), 0, CCheckConflict::WebBrowserProblem,
	// VC-Huby[2007-02-10]: build 070206 的2007-02-08中发现350/680的比例是由flash*.ocx 引起Crash
	_T("flash.ocx"), _T("WebBrowser"), 0, CCheckConflict::FlashProblem,
	_T("flash9.ocx"), _T("WebBrowser"), 0, CCheckConflict::FlashProblem,
	_T("flash9b.ocx"), _T("WebBrowser"), 0, CCheckConflict::FlashProblem,
	_T("flash8.ocx"), _T("WebBrowser"), 0, CCheckConflict::FlashProblem,
	_T("flash8b.ocx"), _T("WebBrowser"), 0, CCheckConflict::FlashProblem,
	_T("flash8a.ocx"), _T("WebBrowser"), 0, CCheckConflict::FlashProblem,
	

	_T("sockspy.dll"), _T("未知"), 1, CCheckConflict::SimplyPrompt,
	_T("K7PSWSEn.dll"), _T("未知"), 1, CCheckConflict::SimplyPrompt,
	_T("Iefilter.dll"), _T("未知"), 1, CCheckConflict::SimplyPrompt,
	_T("tcpipdog.dll"), _T("未知"), 0, CCheckConflict::SimplyPrompt,
	_T("tcpipdog0.dll"), _T("未知"), 0, CCheckConflict::SimplyPrompt,
	_T("HintSock.dll"), _T("Hintsoft Pubwin 网吧管理系统"), 0, CCheckConflict::SimplyPrompt,
	_T("EagleFlt.dll"), _T("未知"), 1, CCheckConflict::SimplyPrompt,
	_T("BtFilter.dll"), _T("未知"), 1, CCheckConflict::SimplyPrompt,
	_T("MiFilter2.dll"), _T("未知"), 1, CCheckConflict::SimplyPrompt,
	_T("WinFilter.dll"), _T("未知"), 1, CCheckConflict::SimplyPrompt,
	_T("iaudit.dll"), _T("未知"), 1, CCheckConflict::SimplyPrompt
};

#define MODULE_LIST_COUNT ( sizeof(s_conflictModuleList) / (sizeof (CONFILCT_MOD_ENTRY)) )


BOOL CCheckConflict::SimplyPrompt(int iModuleIndex, HMODULE hMod)
{
	TCHAR	szModuleFileName[MAX_PATH];
	szModuleFileName[0] = _T('\0');
	GetModuleFileName(hMod, szModuleFileName, MAX_PATH);
	szModuleFileName[MAX_PATH - 1] = _T('\0');

	//*Warning* 修改显示方式时，注重下面szOutput申请的内存是否足够容纳要写入的字符。
	LPCTSTR lpcszOutputFormat = _T("eMule 与其他软件发生冲突，即将关闭。\r\n")
								_T("\r\n")
								_T("发生冲突的软件名称： [%s]\r\n")
								_T("发生冲突的模块： [%s]");

	size_t	nOutputMax = _tcslen(lpcszOutputFormat) + MAX_PATH + DESC_MAX + 1;
	TCHAR	*szOutput = new TCHAR[nOutputMax];

	StringCchPrintf(szOutput, nOutputMax, lpcszOutputFormat, s_conflictModuleList[iModuleIndex].szModuleDescription, szModuleFileName);
	MessageBox(NULL, szOutput, GetResString(IDS_CONFLICT), MB_OK);

	delete[] szOutput;
	szOutput = NULL;

	return TRUE;
}

BOOL CCheckConflict::ThirdPartyError(int iModuleIndex, HMODULE hMod)
{
	TCHAR	szModuleFileName[MAX_PATH];
	szModuleFileName[0] = _T('\0');
	GetModuleFileName(hMod, szModuleFileName, MAX_PATH);
	szModuleFileName[MAX_PATH - 1] = _T('\0');

	//*Warning* 修改显示方式时，注重下面szOutput申请的内存是否足够容纳要写入的字符。
	LPCTSTR lpcszOutputFormat = _T("第三方软件处理出错，导致eMule退出。\r\n")
								_T("\r\n")
								_T("出错的软件名称： [%s]\r\n")
								_T("出错的模块： [%s]");

	size_t	nOutputMax = _tcslen(lpcszOutputFormat) + MAX_PATH + DESC_MAX + 1;
	TCHAR	*szOutput = new TCHAR[nOutputMax];

	StringCchPrintf(szOutput, nOutputMax, lpcszOutputFormat, s_conflictModuleList[iModuleIndex].szModuleDescription, szModuleFileName);
	MessageBox(NULL, szOutput, _T("第三方软件出错导致eMule退出"), MB_OK);

	delete[] szOutput;
	szOutput = NULL;

	return TRUE;
}

BOOL CCheckConflict::WebBrowserProblem(int /*iModuleIndex*/, HMODULE /*hMod*/)
{
	BOOL	bShowBrowser;

	if (! GetIniBool(_T("eMule"), _T("Showbrowser"), &bShowBrowser)
		|| !bShowBrowser)		//WebBrowser has already been disabled.
		return FALSE;

	if (! WriteIniBool(_T("eMule"), _T("Showbrowser"), FALSE))
		return FALSE;

	LPCTSTR lpcszOutput	= _T("eMule使用网页浏览器时发生错误。\r\n")
							_T("\r\n")
							_T("我们已为您禁用了内置的网页浏览器，以尝试解决这个问题。\r\n")
							_T("您可以重启eMule，看是否已经可以正常使用。\r\n");
	MessageBox(NULL, lpcszOutput, _T("eMule发生错误"), MB_OK);

	return TRUE;
}

BOOL CCheckConflict::FlashProblem(int /*iModuleIndex*/, HMODULE hMod)
{
	BOOL	bShowBrowser;

	if (! GetIniBool(_T("eMule"), _T("Showbrowser"), &bShowBrowser)
		|| !bShowBrowser)		//WebBrowser has already been disabled.
		return FALSE;

	if (! WriteIniBool(_T("eMule"), _T("Showbrowser"), FALSE))
		return FALSE;


	TCHAR	szModuleFileName[MAX_PATH];
	szModuleFileName[0] = _T('\0');
	GetModuleFileName(hMod, szModuleFileName, MAX_PATH);
	szModuleFileName[MAX_PATH - 1] = _T('\0');

	//*Warning* 修改显示方式时，注重下面szOutput申请的内存是否足够容纳要写入的字符。
	LPCTSTR lpcszOutputFormat = _T("您目前使用的Flash模块为[%s].\r\n")
								_T("它可能存在一些问题导致eMule退出。\r\n")
								_T("我们已经帮你禁用了浏览器功能，您可以尝试再次打开eMule看是否正常。\r\n")
								_T("\r\n")
								_T("另外，在您把Flash更新到最新版本之后，\r\n")
								_T("可以尝试在eMule的“选项”里，打开浏览器功能，以继续正常使用eMule。\r\n");

	size_t	nOutputMax = _tcslen(lpcszOutputFormat) + MAX_PATH + 1;
	TCHAR	*szOutput = new TCHAR[nOutputMax];

	StringCchPrintf(szOutput, nOutputMax, lpcszOutputFormat, szModuleFileName);
	MessageBox(NULL, szOutput, _T("Flash出现问题"), MB_OK);

	delete[] szOutput;
	szOutput = NULL;

	return TRUE;
}


CCheckConflict::CCheckConflict(void)
{
	m_hPsapiDll						= NULL;
	m_pfnGetModuleInformation		= NULL;

	m_hDbgHelpDll					= NULL;
	m_pfnStackWalk64				= NULL;
	m_pfnSymFunctionTableAccess64	= NULL;
	m_pfnSymGetModuleBase64			= NULL;
	m_pfnEnumProcessModules			= NULL;


	m_hPsapiDll = LoadLibrary(_T("psapi.dll"));
	if (NULL != m_hPsapiDll)
	{
		m_pfnGetModuleInformation = (PFN_GetModuleInformation) GetProcAddress(m_hPsapiDll, "GetModuleInformation");
		m_pfnEnumProcessModules = (PFN_EnumProcessModules) GetProcAddress(m_hPsapiDll, "EnumProcessModules");
	}

	m_hDbgHelpDll = LoadLibrary(_T("dbghelp.dll"));
	if (NULL != m_hDbgHelpDll)
	{
		m_pfnStackWalk64 = (PFN_StackWalk64) GetProcAddress(m_hDbgHelpDll, "StackWalk64");
		m_pfnSymFunctionTableAccess64 = (PFUNCTION_TABLE_ACCESS_ROUTINE64) GetProcAddress(m_hDbgHelpDll, "SymFunctionTableAccess64");
		m_pfnSymGetModuleBase64 = (PGET_MODULE_BASE_ROUTINE64) GetProcAddress(m_hDbgHelpDll, "SymGetModuleBase64");
	}
}

CCheckConflict::~CCheckConflict(void)
{
	if (NULL != m_hPsapiDll)
	{
		FreeLibrary(m_hPsapiDll);
		m_hPsapiDll = NULL;
	}
	
	if (NULL != m_hDbgHelpDll)
	{
		FreeLibrary(m_hDbgHelpDll);
		m_hDbgHelpDll = NULL;
	}
}



BOOL CCheckConflict::CheckConflict(struct _EXCEPTION_POINTERS* pExceptionInfo)
{
	if (!IsFuncitonsReady())
		return FALSE;

	if (IsCrashInEmuleExe(pExceptionInfo))
		return FALSE;

	if (PreCheck(pExceptionInfo))
		return TRUE;

	if (CheckModules(pExceptionInfo))
		return TRUE;

	if (PostCheck(pExceptionInfo))
		return TRUE;

	//if (UserAnalyseDiy(pExceptionInfo))		//如果用户自己已经知道崩溃是怎么回事了，就不弹出“发送错误报告”窗口了。
	//	return TRUE;



	return FALSE;
}

BOOL CCheckConflict::IsFuncitonsReady()
{
	if (NULL == m_pfnGetModuleInformation)
		return FALSE;

	if (NULL == m_pfnStackWalk64)
		return FALSE;
	if (NULL == m_pfnSymFunctionTableAccess64)
		return FALSE;
	if (NULL == m_pfnSymGetModuleBase64)
		return FALSE;
	if (NULL == m_pfnEnumProcessModules)
		return FALSE;


	return TRUE;
}

BOOL CCheckConflict::IsAddressInModule(PVOID pvAddress, HMODULE hModule)
{
	if (NULL == m_pfnGetModuleInformation)
		return FALSE;

	MODULEINFO	mi;
	if ( m_pfnGetModuleInformation(GetCurrentProcess(), hModule, &mi, sizeof(mi)) )
	{
		if (pvAddress >= mi.lpBaseOfDll
			&& (size_t)pvAddress < (size_t)mi.lpBaseOfDll + (size_t)mi.SizeOfImage)
		{
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CCheckConflict::IsCrashInEmuleExe(struct _EXCEPTION_POINTERS* pExceptionInfo)
{
	HMODULE		hEmuleMod = GetModuleHandle(NULL);
	return IsAddressInModule((PVOID)pExceptionInfo->ExceptionRecord->ExceptionAddress, hEmuleMod);
}

BOOL CCheckConflict::PreCheck(struct _EXCEPTION_POINTERS* /*pExceptionInfo*/)
{

	return FALSE;
}

BOOL CCheckConflict::CheckModules(struct _EXCEPTION_POINTERS* pExceptionInfo)
{
	int				i, j, jCount;
	HMODULE			hMod;
	CONTEXT			context;
	STACKFRAME64	sf;

	for (i = 0; i < MODULE_LIST_COUNT; i++)
	{
		hMod = GetModuleHandle(s_conflictModuleList[i].szModuleName);
		if (NULL == hMod)
			continue;

		memcpy(&context, pExceptionInfo->ContextRecord, sizeof(context));

		ZeroMemory(&sf, sizeof(sf));
		sf.AddrPC.Offset = context.Eip;
		sf.AddrPC.Mode = AddrModeFlat;
		sf.AddrFrame.Offset = context.Ebp;
		sf.AddrFrame.Mode = AddrModeFlat;

		jCount = (s_conflictModuleList[i].iStackSearchLevel == 0) ? 100/*最多查找100层*/ : s_conflictModuleList[i].iStackSearchLevel;
		for (j = 0; j < jCount; j++)
		{
			if (! m_pfnStackWalk64(IMAGE_FILE_MACHINE_I386, GetCurrentProcess(), GetCurrentThread(),
				&sf, NULL, NULL, m_pfnSymFunctionTableAccess64, m_pfnSymGetModuleBase64, NULL))
			{
				break;
			}

			if (IsAddressInModule((PVOID)sf.AddrPC.Offset, hMod))
			{
				return s_conflictModuleList[i].conflictProc(i, hMod);
			}
		}
	}
	
	return FALSE;
}

BOOL CCheckConflict::PostCheck(struct _EXCEPTION_POINTERS* /*pExceptionInfo*/)
{
	HMODULE hMod = LoadLibrary(_T("SysWin64.Sys"));
	if (NULL != hMod)
	{
		TCHAR	szModuleFileName[MAX_PATH];
		szModuleFileName[0] = _T('\0');
		GetModuleFileName(hMod, szModuleFileName, MAX_PATH);
		szModuleFileName[MAX_PATH - 1] = _T('\0');
		
		LPCTSTR lpcszOutputFormat = _T("eMule 与 %s 发生冲突，即将关闭。\r\n");

		size_t	nOutputMax = _tcslen(lpcszOutputFormat) + MAX_PATH + 1;
		TCHAR	*szOutput = new TCHAR[nOutputMax];

		StringCchPrintf(szOutput, nOutputMax, lpcszOutputFormat, szModuleFileName);
		MessageBox(NULL, szOutput, GetResString(IDS_CONFLICT), MB_OK);
		return TRUE;
	}

	return FALSE;
}
BOOL CCheckConflict::UserAnalyseDiy(struct _EXCEPTION_POINTERS* pExceptionInfo)
{
	//让用户先自己分析一下。
	int		i;
	int		iModCount;
	DWORD	dwNeedBytes;
	TCHAR	szModName[MAX_PATH];
	HMODULE	*arrMods = NULL;
	int		iChoice = IDNO;


	m_pfnEnumProcessModules(GetCurrentProcess(), NULL, 0, &dwNeedBytes);
	iModCount = dwNeedBytes/sizeof(HMODULE);

	arrMods = new HMODULE[iModCount];
	m_pfnEnumProcessModules(GetCurrentProcess(), arrMods, dwNeedBytes, &dwNeedBytes);

	for (i = 0; i < iModCount; i++)
	{
		if (IsAddressInModule(pExceptionInfo->ExceptionRecord->ExceptionAddress, arrMods[i]))
		{
			GetModuleFileName(arrMods[i], szModName, MAX_PATH);

			//	询问用户	<begin>
			LPCTSTR lpcszOutputFormat = _T("程序在[%s]模块里出现异常情况。\r\n")
										_T("\r\n")
										_T("如果您已经知道这个模块本身存在问题，您可以选“是”来结束程序；\r\n")
										_T("或者，您可以选“否”，然后发送错误报告，让我们来为您作进一步分析。");
				
			size_t	nOutputMax = _tcslen(lpcszOutputFormat) + MAX_PATH + 1;
			TCHAR	*szOutput = new TCHAR[nOutputMax];

			StringCchPrintf(szOutput, nOutputMax, lpcszOutputFormat, szModName);
			iChoice = MessageBox(NULL, szOutput, _T("程序出错"), MB_YESNO | MB_DEFBUTTON2);

			delete[] szOutput;
			szOutput = NULL;
			//	询问用户	<end>

			break;
		}
	}


	delete[] arrMods;
	arrMods = NULL;

	return (iChoice == IDYES);
}

BOOL CCheckConflict::GetAppPath(LPTSTR lpszBuffer, DWORD dwBufferCch)
{
	TCHAR szModuleFileName[MAX_PATH];
	TCHAR drive[_MAX_DRIVE];
	TCHAR dir[_MAX_DIR];
	TCHAR fname[_MAX_FNAME];
	TCHAR ext[_MAX_EXT];

	if (NULL == GetModuleFileName(NULL, szModuleFileName, MAX_PATH))
		return FALSE;

	_tsplitpath( szModuleFileName, drive, dir, fname, ext );
	
	if ( FAILED(StringCchCopy(lpszBuffer, dwBufferCch, drive)))
		return FALSE;

	if ( FAILED(StringCchCat(lpszBuffer, dwBufferCch, dir)) )
		return FALSE;

	return TRUE;
}

BOOL CCheckConflict::GetIniPathName(LPTSTR lpszBuffer, DWORD dwBufferCch)
{
	if (!GetAppPath(lpszBuffer, dwBufferCch))
		return FALSE;

	if ( FAILED(StringCchCat(lpszBuffer, dwBufferCch, _T("config\\preferences.ini"))) )
		return FALSE;

	return TRUE;
}

BOOL CCheckConflict::GetIniBool(LPCTSTR lpszSection, LPCTSTR lpszEntry, BOOL *pbValue)
{
	if (NULL == pbValue)
		return FALSE;

	TCHAR	szIniFile[MAX_PATH];
	if (! GetIniPathName(szIniFile, MAX_PATH))
		return FALSE;

	TCHAR szValue[MAX_INI_BUFFER];
	if (0 == GetPrivateProfileString(lpszSection, lpszEntry, _T("0"), szValue, MAX_INI_BUFFER, szIniFile) )
		return FALSE;

	*pbValue = _tstoi(szValue);
	return TRUE;
}
BOOL CCheckConflict::WriteIniBool(LPCTSTR lpszSection, LPCTSTR lpszEntry, BOOL bValue)
{
	TCHAR	szIniFile[MAX_PATH];
	if (! GetIniPathName(szIniFile, MAX_PATH))
		return FALSE;

	TCHAR szBuffer[2];
	szBuffer[0] = bValue ? _T('1') : _T('0');
	szBuffer[1] = _T('\0');
	return WritePrivateProfileString(lpszSection, lpszEntry, szBuffer, szIniFile);
}
