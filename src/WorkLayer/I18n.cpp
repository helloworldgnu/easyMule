/*
 * $Id: I18n.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
#include "stdafx.h"
#include <locale.h>
#include "emule.h"
#include "OtherFunctions.h"
#include "Preferences.h"
#include "langids.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static HINSTANCE _hLangDLL = NULL;

CString GetResString(UINT uStringID, WORD wLanguageID)
{
	
	CString resString;
	if (_hLangDLL)
		(void)resString.LoadString(_hLangDLL, uStringID, wLanguageID);
	if (resString.IsEmpty())
		(void)resString.LoadString(GetModuleHandle(NULL), uStringID, LANGID_EN_US);
	return resString;
}

CString GetResString(UINT uStringID)
{
	CString resString;
	if (_hLangDLL)
		resString.LoadString(_hLangDLL, uStringID);
	if (resString.IsEmpty())
		resString.LoadString(GetModuleHandle(NULL), uStringID);
	return resString;
}

struct SLanguage {
	LANGID	lid;
	LPCTSTR pszLocale;
	BOOL	bSupported;
	LPCTSTR	pszISOLocale;
	UINT	uCodepage;
	LPCTSTR	pszHtmlCharset;
};


// Codepages (Windows)
// ---------------------------------------------------------------------
// 1250		ANSI - Central European
// 1251		ANSI - Cyrillic
// 1252		ANSI - Latin I
// 1253		ANSI - Greek
// 1254		ANSI - Turkish
// 1255		ANSI - Hebrew
// 1256		ANSI - Arabic
// 1257		ANSI - Baltic
// 1258		ANSI/OEM - Vietnamese
//  932		ANSI/OEM - Japanese, Shift-JIS
//  936		ANSI/OEM - Simplified Chinese (PRC, Singapore)
//  949		ANSI/OEM - Korean (Unified Hangeul Code)
//  950		ANSI/OEM - Traditional Chinese (Taiwan; Hong Kong SAR, PRC)

// HTML charsets	CodePg
// -------------------------------------------
// windows-1250		1250	Central European (Windows)
// windows-1251		1251	Cyrillic (Windows)
// windows-1252		1252	Western European (Windows)
// windows-1253		1253	Greek (Windows)
// windows-1254		1254	Turkish (Windows)
// windows-1255		1255	Hebrew (Windows)
// windows-1256		1256	Arabic (Windows)
// windows-1257		1257	Baltic (Windows)
//
// NOTE: the 'iso-...' charsets are more backward compatible than the 'windows-...' charsets.
// NOTE-ALSO: some of the 'iso-...' charsets are by default *not* installed by IE6 (e.g. Arabic (ISO)) or show up
//	with wrong chars - so, better use the 'windows-' charsets..
//
// iso-8859-1		1252	Western European (ISO)
// iso-8859-2		1250	Central European (ISO)
// iso-8859-3		1254	Latin 3 (ISO)
// iso-8859-4		1257	Baltic (ISO)
// iso-8859-5		1251	Cyrillic (ISO)			does not show up correctly in IE6
// iso-8859-6		1256	Arabic (ISO)			not installed (by default) with IE6
// iso-8859-7		1253	Greek (ISO)
// iso-8859-8		1255	Hebrew (ISO-Visual)
// iso-8859-9		1254	Turkish (ISO)
// iso-8859-15		1252	Latin 9 (ISO)
// iso-2022-jp		 932	Japanese (JIS)
// iso-2022-kr		 949	Korean (ISO)

static SLanguage _aLanguages[] =
{
	//MODIFIED by VC-fengwen on 2007/07/18 <begin>	: 只留三种语言
	//{LANGID_AR_AE,	_T(""),				FALSE,	_T("ar_AE"),	1256,	_T("windows-1256")},	// Arabic (UAE)
	//{LANGID_BA_BA,	_T(""),				FALSE,	_T("ba_BA"),	1252,	_T("windows-1252")},	// Basque
	//{LANGID_BG_BG,	_T("hun"),			FALSE,	_T("bg_BG"),	1251,	_T("windows-1251")},	// Bulgarian
	//{LANGID_CA_ES,	_T(""),				FALSE,	_T("ca_ES"),	1252,	_T("windows-1252")},	// Catalan
	//{LANGID_CZ_CZ,	_T("czech"),		FALSE,	_T("cz_CZ"),	1250,	_T("windows-1250")},	// Czech
	//{LANGID_DA_DK,	_T("danish"),		FALSE,	_T("da_DK"),	1252,	_T("windows-1252")},	// Danish
	//{LANGID_DE_DE,	_T("german"),		FALSE,	_T("de_DE"),	1252,	_T("windows-1252")},	// German (Germany)
	//{LANGID_EL_GR,	_T("greek"),		FALSE,	_T("el_GR"),	1253,	_T("windows-1253")},	// Greek
	//{LANGID_EN_US,	_T("english"),		TRUE,	_T("en_US"),	1252,	_T("windows-1252")},	// English
	//{LANGID_ES_ES_T,_T("spanish"),		FALSE,	_T("es_ES_T"),	1252,	_T("windows-1252")},	// Spanish (Castilian)
	//{LANGID_ES_AS,  _T("spanish"),		FALSE,	_T("es_AS"),	1252,	_T("windows-1252")},	// Asturian
	//{LANGID_ET_EE,	_T(""),				FALSE,	_T("et_EE"),	1257,	_T("windows-1257")},	// Estonian
	//{LANGID_FI_FI,	_T("finnish"),		FALSE,	_T("fi_FI"),	1252,	_T("windows-1252")},	// Finnish
	//{LANGID_FR_FR,	_T("french"),		FALSE,	_T("fr_FR"),	1252,	_T("windows-1252")},	// French (France)
	//{LANGID_FR_BR,	_T("french"),		FALSE,	_T("fr_BR"),	1252,	_T("windows-1252")},	// French (Breton)
	//{LANGID_GL_ES,	_T(""),				FALSE,	_T("gl_ES"),	1252,	_T("windows-1252")},	// Galician
	//{LANGID_HE_IL,	_T(""),				FALSE,	_T("he_IL"),	1255,	_T("windows-1255")},	// Hebrew
	//{LANGID_HU_HU,	_T("hungarian"),	FALSE,	_T("hu_HU"),	1250,	_T("windows-1250")},	// Hungarian
	//{LANGID_IT_IT,	_T("italian"),		FALSE,	_T("it_IT"),	1252,	_T("windows-1252")},	// Italian (Italy)
	//{LANGID_JP_JP,	_T("japanese"),		FALSE,	_T("jp_JP"),	 932,	_T("shift_jis")},		// Japanese
	//{LANGID_KO_KR,	_T("korean"),		FALSE,	_T("ko_KR"),	 949,	_T("euc-kr")},			// Korean
	//{LANGID_LT_LT,	_T(""),				FALSE,	_T("lt_LT"),	1257,	_T("windows-1257")},	// Lithuanian
	//{LANGID_LV_LV,	_T(""),				FALSE,	_T("lv_LV"),	1257,	_T("windows-1257")},	// Latvian
	//{LANGID_MT_MT,	_T("mt"),			FALSE,	_T("mt_MT"),	1254,	_T("windows-1254")},	// Maltese
	//{LANGID_NB_NO,	_T("nor"),			FALSE,	_T("nb_NO"),	1252,	_T("windows-1252")},	// Norwegian (Bokmal)
	//{LANGID_NN_NO,	_T("non"),			FALSE,	_T("nn_NO"),	1252,	_T("windows-1252")},	// Norwegian (Nynorsk)
	//{LANGID_NL_NL,	_T("dutch"),		FALSE,	_T("nl_NL"),	1252,	_T("windows-1252")},	// Dutch (Netherlands)
	//{LANGID_PL_PL,	_T("polish"),		FALSE,	_T("pl_PL"),	1250,	_T("windows-1250")},	// Polish
	//{LANGID_PT_BR,	_T("ptb"),			FALSE,	_T("pt_BR"),	1252,	_T("windows-1252")},	// Portuguese (Brazil)
	//{LANGID_PT_PT,	_T("ptg"),			FALSE,	_T("pt_PT"),	1252,	_T("windows-1252")},	// Portuguese (Portugal)
	//{LANGID_RO_RO,	_T(""),				FALSE,	_T("ro_RO"),	1250,	_T("windows-1250")},	// Rum鋘isch
	//{LANGID_RU_RU,	_T("russian"),		FALSE,	_T("ru_RU"),	1251,	_T("windows-1251")},	// Russian
	//{LANGID_SL_SI,	_T("slovak"),		FALSE,	_T("sl_SI"),	1250,	_T("windows-1250")},	// Slovenian
	//{LANGID_SQ_AL,	_T(""),				FALSE,	_T("sq_AL"),	1252,	_T("windows-1252")},	// Albanian (Albania)
	//{LANGID_SV_SE,	_T("swedish"),		FALSE,	_T("sv_SE"),	1252,	_T("windows-1252")},	// Swedish
	//{LANGID_TR_TR,	_T("turkish"),		FALSE,	_T("tr_TR"),	1254,	_T("windows-1254")},	// Turkish
	//{LANGID_UA_UA,	_T(""),				FALSE,	_T("ua_UA"),	1251,	_T("windows-1251")},	// Ukrainian
	//{LANGID_ZH_CN,	_T("chs"),			FALSE,	_T("zh_CN"),	 936,	_T("gb2312")},			// Chinese (P.R.C.)
	//{LANGID_ZH_TW,	_T("cht"),			FALSE,	_T("zh_TW"),	 950,	_T("big5")}, 			// Chinese (Taiwan)
	//{LANGID_VI_VN,	_T(""),				FALSE,	_T("vi_VN"),	1258,	_T("windows-1258")},	// Vietnamese
	
	{LANGID_EN_US,	_T("english"),		TRUE,	_T("en_US"),	1252,	_T("windows-1252")},	// English
	{LANGID_ZH_CN,	_T("chs"),			FALSE,	_T("zh_CN"),	 936,	_T("gb2312")},			// Chinese (P.R.C.)
	{LANGID_ZH_TW,	_T("cht"),			FALSE,	_T("zh_TW"),	 950,	_T("big5")}, 			// Chinese (Taiwan)
	//MODIFIED by VC-fengwen on 2007/07/18 <end>	: 只留三种语言

	{0, NULL, 0, 0}
};

static void InitLanguages(const CString& rstrLangDir1, const CString& rstrLangDir2, bool bReInit = false)
{
	static BOOL _bInitialized = FALSE;
	if (_bInitialized && !bReInit)
		return;
	_bInitialized = TRUE;

	CFileFind ff;
	bool bEnd = !ff.FindFile(rstrLangDir1 + _T("*.dll"), 0);
	bool bFirstDir = rstrLangDir1.CompareNoCase(rstrLangDir2) != 0;
	while (!bEnd)
	{
		bEnd = !ff.FindNextFile();
		if (ff.IsDirectory())
			continue;
		TCHAR szLandDLLFileName[MAX_PATH];
		_tsplitpath(ff.GetFileName(), NULL, NULL, szLandDLLFileName, NULL);

		SLanguage* pLangs = _aLanguages;
		if (pLangs){
			while (pLangs->lid){
				if (_tcsicmp(pLangs->pszISOLocale, szLandDLLFileName) == 0){
					pLangs->bSupported = TRUE;
					break;
				}
				pLangs++;
			}
		}
		if (bEnd && bFirstDir){
			ff.Close();
			bEnd = !ff.FindFile(rstrLangDir2 + _T("*.dll"), 0);
			bFirstDir = false;
		}

	}
	ff.Close();
}

static void FreeLangDLL()
{
	if (_hLangDLL != NULL && _hLangDLL != GetModuleHandle(NULL)){
		VERIFY( FreeLibrary(_hLangDLL) );
		_hLangDLL = NULL;
	}
}

void CPreferences::GetLanguages(CWordArray& aLanguageIDs)
{
	const SLanguage* pLang = _aLanguages;
	while (pLang->lid){
		//if (pLang->bSupported)
		//show all languages, offer download if not supported ones later
		aLanguageIDs.Add(pLang->lid);
		pLang++;
	}
}

WORD CPreferences::GetLanguageID()
{
	return m_wLanguageID;
}

void CPreferences::SetLanguageID(WORD lid)
{
	m_wLanguageID = lid;
}

static bool CheckLangDLLVersion(const CString& rstrLangDLL)
{
	bool bResult = false;
	DWORD dwUnused;
	DWORD dwVerInfSize = GetFileVersionInfoSize(const_cast<LPTSTR>((LPCTSTR)rstrLangDLL), &dwUnused);
	if (dwVerInfSize != 0)
	{
		LPBYTE pucVerInf = (LPBYTE)calloc(dwVerInfSize, 1);
		if (pucVerInf)
		{
			if (GetFileVersionInfo(const_cast<LPTSTR>((LPCTSTR)rstrLangDLL), 0, dwVerInfSize, pucVerInf))
			{
				VS_FIXEDFILEINFO* pFileInf = NULL;
				UINT uLen = 0;
				if (VerQueryValue(pucVerInf, _T("\\"), (LPVOID*)&pFileInf, &uLen) && pFileInf && uLen)
				{
					bResult = (pFileInf->dwProductVersionMS == theApp.m_dwProductVersionMS && pFileInf->dwProductVersionLS == theApp.m_dwProductVersionLS);
				}
			}
			free(pucVerInf);
		}
	}

	return bResult;
}

static bool LoadLangLib(const CString& rstrLangDir1, const CString& rstrLangDir2, LANGID lid)
{
	const SLanguage* pLangs = _aLanguages;
	if (pLangs){
		while (pLangs->lid){
			if (pLangs->bSupported && pLangs->lid == lid){
				FreeLangDLL();

				bool bLoadedLib = false;
				if (pLangs->lid == LANGID_EN_US){
					_hLangDLL = NULL;
					bLoadedLib = true;
				}
				else{
					CString strLangDLL = rstrLangDir1;
					strLangDLL += pLangs->pszISOLocale;
					strLangDLL += _T(".dll");
					if (CheckLangDLLVersion(strLangDLL)){
						_hLangDLL = LoadLibrary(strLangDLL);
						if (_hLangDLL)
							bLoadedLib = true;
					}
					if (rstrLangDir1.CompareNoCase(rstrLangDir2) != 0){
						strLangDLL = rstrLangDir2;
						strLangDLL += pLangs->pszISOLocale;
						strLangDLL += _T(".dll");
						if (CheckLangDLLVersion(strLangDLL)){
							_hLangDLL = LoadLibrary(strLangDLL);
							if (_hLangDLL)
								bLoadedLib = true;
						}
					}
				}
				if (bLoadedLib)
					return true;
				break;
			}
			pLangs++;
		}
	}
	return false;
}

void CPreferences::SetLanguage()
{
	InitLanguages(thePrefs.GetMuleDirectory(EMULE_INSTLANGDIR), thePrefs.GetMuleDirectory(EMULE_ADDLANGDIR, false));

	bool bFoundLang = false;
	if (m_wLanguageID)
		bFoundLang = LoadLangLib(thePrefs.GetMuleDirectory(EMULE_INSTLANGDIR), thePrefs.GetMuleDirectory(EMULE_ADDLANGDIR, false), m_wLanguageID);

	if (!bFoundLang){
		LANGID lidLocale = (LANGID)::GetThreadLocale();
		//LANGID lidLocalePri = PRIMARYLANGID(::GetThreadLocale());
		//LANGID lidLocaleSub = SUBLANGID(::GetThreadLocale());

		bFoundLang = LoadLangLib(thePrefs.GetMuleDirectory(EMULE_INSTLANGDIR), thePrefs.GetMuleDirectory(EMULE_ADDLANGDIR, false), lidLocale);
		if (!bFoundLang){
			LoadLangLib(thePrefs.GetMuleDirectory(EMULE_INSTLANGDIR), thePrefs.GetMuleDirectory(EMULE_ADDLANGDIR, false), LANGID_EN_US);
			m_wLanguageID = LANGID_EN_US;
			CString strLngEnglish = GetResString(IDS_MB_LANGUAGEINFO);
			AfxMessageBox(strLngEnglish, MB_ICONASTERISK);
		}
		else
			m_wLanguageID = lidLocale;
	}

	// if loading a string fails, set language to English
	if (GetResString(IDS_MB_LANGUAGEINFO).IsEmpty()) {
		LoadLangLib(thePrefs.GetMuleDirectory(EMULE_INSTLANGDIR), thePrefs.GetMuleDirectory(EMULE_ADDLANGDIR, false), LANGID_EN_US);
		m_wLanguageID = LANGID_EN_US;
	}

	InitThreadLocale();
}

bool CPreferences::IsLanguageSupported(LANGID lidSelected, bool bUpdateBefore){
	InitLanguages(thePrefs.GetMuleDirectory(EMULE_INSTLANGDIR), thePrefs.GetMuleDirectory(EMULE_ADDLANGDIR, false), bUpdateBefore);
	if (lidSelected == LANGID_EN_US)
		return true;
	const SLanguage* pLang = _aLanguages;
	for (;pLang->lid;pLang++){
		if (pLang->lid == lidSelected && pLang->bSupported){
			bool bResult = CheckLangDLLVersion(thePrefs.GetMuleDirectory(EMULE_INSTLANGDIR) + CString(pLang->pszISOLocale) + _T(".dll"));
			return bResult || CheckLangDLLVersion(thePrefs.GetMuleDirectory(EMULE_ADDLANGDIR, false) + CString(pLang->pszISOLocale) + _T(".dll"));
		}
	}
	return false; 
}

CString CPreferences::GetLangDLLNameByID(LANGID lidSelected){
	const SLanguage* pLang = _aLanguages;
	for (;pLang->lid;pLang++){
		if (pLang->lid == lidSelected)
			return CString(pLang->pszISOLocale) + _T(".dll"); 
	}
	ASSERT ( false );
	return CString(_T(""));
}

void CPreferences::SetRtlLocale(LCID lcid)
{
	const SLanguage* pLangs = _aLanguages;
	while (pLangs->lid)
	{
		if (pLangs->lid == LANGIDFROMLCID(lcid))
		{
			if (pLangs->uCodepage)
			{
				CString strCodepage;
				strCodepage.Format(_T(".%u"), pLangs->uCodepage);
				_tsetlocale(LC_CTYPE, strCodepage);
			}
			break;
		}
		pLangs++;
	}
}

void CPreferences::InitThreadLocale()
{
	ASSERT( m_wLanguageID != 0 );

	// NOTE: This function is for testing multi language support only.
	// NOTE: This function is *NOT* to be enabled in release builds nor to be offered by any Mod!
	if (theApp.GetProfileInt(_T("eMule"), _T("SetLanguageACP"), 0) != 0)
	{
		//LCID lcidUser = GetUserDefaultLCID();		// Installation, or altered by user in control panel (WinXP)

		// get the ANSI code page which is to be used for all non-Unicode conversions.
		LANGID lidSystem = m_wLanguageID;

		// get user's sorting preferences
		//UINT uSortIdUser = SORTIDFROMLCID(lcidUser);
		//UINT uSortVerUser = SORTVERSIONFROMLCID(lcidUser);
		// we can't use the same sorting paramters for 2 different Languages..
		UINT uSortIdUser = SORT_DEFAULT;
		UINT uSortVerUser = 0;

		// set thread locale, this is used for:
		//	- MBCS->Unicode conversions (e.g. search results).
		//	- Unicode->MBCS conversions (e.g. publishing local files (names) in network, or savint text files on local disk)...
		LCID lcid = MAKESORTLCID(lidSystem, uSortIdUser, uSortVerUser);
		SetThreadLocale(lcid);

		// if we set the thread locale (see comments above) we also have to specify the proper
		// code page for the C-RTL, otherwise we may not be able to store some strings as MBCS
		// (Unicode->MBCS conversion may fail)
		SetRtlLocale(lcid);
	}
	else if (theApp.GetProfileInt(_T("eMule"), _T("SetSystemACP"), 0) != 0)
	{
		LCID lcidSystem = GetSystemDefaultLCID();	// Installation, or altered by user in control panel (WinXP)
		//LCID lcidUser = GetUserDefaultLCID();		// Installation, or altered by user in control panel (WinXP)

		// get the ANSI code page which is to be used for all non-Unicode conversions.
		LANGID lidSystem = LANGIDFROMLCID(lcidSystem);

		// get user's sorting preferences
		//UINT uSortIdUser = SORTIDFROMLCID(lcidUser);
		//UINT uSortVerUser = SORTVERSIONFROMLCID(lcidUser);
		// we can't use the same sorting paramters for 2 different Languages..
		UINT uSortIdUser = SORT_DEFAULT;
		UINT uSortVerUser = 0;

		// create a thread locale which gives full backward compability for users which had run ANSI emule on 
		// a system where the system's code page did not match the user's language..
		LCID lcid = MAKESORTLCID(lidSystem, uSortIdUser, uSortVerUser);
		LCID lcidThread = GetThreadLocale();
		if (lcidThread != lcid)
		{
			TRACE("+++ Setting thread locale: 0x%08x\n", lcid);
			SetThreadLocale(lcid);

			// if we set the thread locale (see comments above) we also have to specify the proper
			// code page for the C-RTL, otherwise we may not be able to store some strings as MBCS
			// (Unicode->MBCS conversion may fail)
			SetRtlLocale(lcid);
		}
	}
}

void InitThreadLocale()
{
	thePrefs.InitThreadLocale();
}

CString GetCodePageNameForLocale(LCID lcid)
{
	CString strCodePage;
	int iResult = GetLocaleInfo(lcid, LOCALE_IDEFAULTANSICODEPAGE, strCodePage.GetBuffer(6), 6);
	strCodePage.ReleaseBuffer();

	if (iResult > 0 && !strCodePage.IsEmpty())
	{
		UINT uCodePage = _tcstoul(strCodePage, NULL, 10);
		if (uCodePage != ULONG_MAX)
		{
			CPINFOEXW CPInfoEx = {0};
			BOOL (WINAPI *pfnGetCPInfoEx)(UINT, DWORD, LPCPINFOEXW);
			(FARPROC&)pfnGetCPInfoEx = GetProcAddress(GetModuleHandle(_T("kernel32")), "GetCPInfoExW");
			if (pfnGetCPInfoEx&& (*pfnGetCPInfoEx)(uCodePage, 0, &CPInfoEx))
				strCodePage = CPInfoEx.CodePageName;
		}
	}
	return strCodePage;
}

bool CheckThreadLocale()
{
	if (theApp.GetProfileInt(_T("eMule"), _T("SetLanguageACP"), 0) != 0)
		return true;
	int iSetSysACP = theApp.GetProfileInt(_T("eMule"), _T("SetSystemACP"), -1);
	if (iSetSysACP != -1)
		return true;
	iSetSysACP = 0;

	LCID lcidSystem = GetSystemDefaultLCID();	// Installation, or altered by user in control panel (WinXP)
	//LCID lcidUser = GetUserDefaultLCID();		// Installation, or altered by user in control panel (WinXP)

	// get the ANSI code page which is to be used for all non-Unicode conversions.
	LANGID lidSystem = LANGIDFROMLCID(lcidSystem);

	// get user's sorting preferences
	//UINT uSortIdUser = SORTIDFROMLCID(lcidUser);
	//UINT uSortVerUser = SORTVERSIONFROMLCID(lcidUser);
	// we can't use the same sorting paramters for 2 different Languages..
	UINT uSortIdUser = SORT_DEFAULT;
	UINT uSortVerUser = 0;

	// create a thread locale which gives full backward compability for users which had run ANSI emule on 
	// a system where the system's code page did not match the user's language..
	LCID lcidSys = MAKESORTLCID(lidSystem, uSortIdUser, uSortVerUser);
	LCID lcidUsr = GetThreadLocale();
	if (lcidUsr != lcidSys)
	{
		CString strUsrCP = GetCodePageNameForLocale(lcidUsr);
		if (!strUsrCP.IsEmpty())
			strUsrCP = _T(" \"") + strUsrCP + _T('\"');
		
		CString strSysCP = GetCodePageNameForLocale(lcidSys);
		if (!strSysCP.IsEmpty())
			strSysCP = _T(" \"") + strSysCP + _T('\"');

		static const TCHAR szMsg[] =
			_T("eMule has detected that your current code page%s is not the same as your system's code page%s. For converting non-Unicode data to Unicode, you need to specify which code page to use.\r\n")
			_T("\r\n")
			_T("If you want eMule to use your current code page for converting non-Unicode data, click 'Yes'. (If you are using eMule for the first time or if you don't care about this issue at all, choose this option. This is the recommended setting.)\r\n")
			_T("\r\n")
			_T("If you want eMule to use your system's code page for converting non-Unicode data, click 'No'. (This will give you more backward compatibility when reading older *.met files created with non-Unicode eMule versions.)\r\n")
			_T("\r\n")
			_T("If you want to cancel and create backup of all your configuration files or visit our forum to learn more about this issue, click 'Cancel'.\r\n")
			;
		CString strFullMsg;
		strFullMsg.Format(szMsg, strUsrCP, strSysCP);
		int iAnswer = AfxMessageBox(strFullMsg, MB_ICONSTOP | MB_YESNOCANCEL | MB_DEFBUTTON1);
		if (iAnswer == IDCANCEL)
			return false;
		if (iAnswer == IDNO)
			iSetSysACP = 1;
	}
	VERIFY( theApp.WriteProfileInt(_T("eMule"), _T("SetSystemACP"), iSetSysACP) );
	return true;
}

CString CPreferences::GetHtmlCharset()
{
	ASSERT( m_wLanguageID != 0 );

	LPCTSTR pszHtmlCharset = NULL;
	const SLanguage* pLangs = _aLanguages;
	while (pLangs->lid)
	{
		if (pLangs->lid == m_wLanguageID)
		{
			pszHtmlCharset = pLangs->pszHtmlCharset;
			break;
		}
		pLangs++;
	}

	if (pszHtmlCharset == NULL || pszHtmlCharset[0] == _T('\0'))
	{
		ASSERT(0); // should never come here

		// try to get charset from code page
		LPCTSTR pszLcLocale = _tsetlocale(LC_CTYPE, NULL);
		if (pszLcLocale)
		{
			TCHAR szLocaleID[128];
			UINT uCodepage = 0;
			if (_stscanf(pszLcLocale, _T("%[a-zA-Z_].%u"), szLocaleID, &uCodepage) == 2 && uCodepage != 0)
			{
				CString strHtmlCodepage;
				strHtmlCodepage.Format(_T("windows-%u"), uCodepage);
				return strHtmlCodepage;
			}
		}
	}

	return pszHtmlCharset;
}

static HHOOK s_hRTLWindowsLayoutOldCbtFilterHook = NULL;

LRESULT CALLBACK RTLWindowsLayoutCbtFilterHook(int code, WPARAM wParam, LPARAM lParam)
{
	if (code == HCBT_CREATEWND)
	{
		//LPCREATESTRUCT lpcs = ((LPCBT_CREATEWND)lParam)->lpcs;

		//if ((lpcs->style & WS_CHILD) == 0)
		//	lpcs->dwExStyle |= WS_EX_LAYOUTRTL;	// doesn't seem to have any effect, but shouldn't hurt

		HWND hWnd = (HWND)wParam;
		if ((GetWindowLong(hWnd, GWL_STYLE) & WS_CHILD) == 0) {
			SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYOUTRTL);
		}
	}
	return CallNextHookEx(s_hRTLWindowsLayoutOldCbtFilterHook, code, wParam, lParam);
}

void CemuleApp::EnableRTLWindowsLayout()
{
	BOOL (WINAPI *pfnSetProcessDefaultLayout)(DWORD dwFlags);
	(FARPROC&)pfnSetProcessDefaultLayout = GetProcAddress(GetModuleHandle(_T("user32")), "SetProcessDefaultLayout");
	if (pfnSetProcessDefaultLayout)
		(*pfnSetProcessDefaultLayout)(LAYOUT_RTL);

	s_hRTLWindowsLayoutOldCbtFilterHook = SetWindowsHookEx(WH_CBT, RTLWindowsLayoutCbtFilterHook, NULL, GetCurrentThreadId());
}

void CemuleApp::DisableRTLWindowsLayout()
{
	if (s_hRTLWindowsLayoutOldCbtFilterHook)
	{
		VERIFY( UnhookWindowsHookEx(s_hRTLWindowsLayoutOldCbtFilterHook) );
		s_hRTLWindowsLayoutOldCbtFilterHook = NULL;

		BOOL (WINAPI *pfnSetProcessDefaultLayout)(DWORD dwFlags);
		(FARPROC&)pfnSetProcessDefaultLayout = GetProcAddress(GetModuleHandle(_T("user32")), "SetProcessDefaultLayout");
		if (pfnSetProcessDefaultLayout)
			(*pfnSetProcessDefaultLayout)(0);
	}
}
