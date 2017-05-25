#include "stdafx.h"
#include "Utils.h"
#include <comutil.h>
#include <time.h>
#include "IE2EM_i.c"
#include "IE2EM.h"

LPCTSTR tcscmp_m(LPCTSTR pszWhere, LPCTSTR pszWhat)
{
	if (*pszWhere == 0)
		return *pszWhat == 0 ? pszWhere : NULL;

	if (*pszWhat == 0)
		return NULL;

	if (*pszWhat == '*')
	{
		if (pszWhat [1] == 0)
			return pszWhere;



		LPCTSTR psz = tcscmp_m (pszWhere, pszWhat+1);
		if (psz)
			return psz;


		return tcscmp_m (pszWhere+1, pszWhat);
	}

	if (*pszWhat != '?')
	{
		if (*pszWhere != *pszWhat)
			return NULL;
	}

	return tcscmp_m (pszWhere+1, pszWhat+1) ? pszWhere : NULL;
}

BOOL IsSuitableExt(LPCTSTR pszExt)
{
	static const TCHAR * pszExts[] = {_T("EXE"),_T("ZIP"),_T("ARJ"),_T("RAR"),_T("LZH"),_T("Z"),_T("GZ"),_T("TGZ"),_T("GZIP"),_T("TAR"),
		_T("BIN"),_T("MP3"),_T("M4A"),_T("WAV"),_T("RA"),_T("RAM"),_T("AAC"),_T("AIF"),_T("AVI"),_T("MPG"),_T("MPEG"),_T("QT"),
		_T("PLJ"),_T("ASF"),_T("MOV"),_T("RM"),_T("MP4"),_T("WMA"),_T("WMV"),_T("MPE"),_T("MPA"),_T("R0*"),_T("R1*"),_T("A0*"),
		_T("A1*"),_T("TIF"),_T("TIFF"),_T("PDF"),_T("MSI"),_T("ACE"),_T("ISO"),_T("OGG"),_T("7Z"),_T("SEA"),_T("SIT"),_T("SITX"),
		_T("PPT"),_T("PPS"), NULL};
	TCHAR * strExt = new TCHAR[_tcslen(pszExt) + 1];
	_tcscpy(strExt, pszExt);
	_tcsupr(strExt);
	for(const TCHAR ** pszCur = pszExts; *pszCur != NULL; pszCur ++)
	{
		if(tcscmp_m(strExt, *pszCur) != NULL)
		{
			delete[] strExt;
			return TRUE;
		}
	}
	delete[] strExt;
	return FALSE;
}

BOOL TestUrl(LPCTSTR pszURL)
{
	if(_tcsnicmp(pszURL, _T("ed2k://"), 7) == 0)
		return TRUE;
	if(!MonitorStatus())
		return FALSE;
	if(_tcsnicmp(pszURL, _T("http://"), 7) == 0 ||
		_tcsnicmp (pszURL, _T("ftp://"), 6) == 0)
	{
		LPCTSTR pszDot = _tcsrchr(pszURL, '.');
		if(pszDot == NULL || !IsSuitableExt(pszDot + 1))
			return FALSE;
		return TRUE;
	}
	return FALSE;
}

BOOL MonitorStatus()
{
	CRegKey hkey;
	if(hkey.Open(HKEY_CURRENT_USER, _T("Software\\easyMule"), KEY_READ) != ERROR_SUCCESS)
		return TRUE;

	DWORD dwMonitor;
	if(hkey.QueryDWORDValue(_T("Monitor"), dwMonitor) != ERROR_SUCCESS)
		return TRUE;

	BOOL bALT = GetKeyState (VK_MENU) & 0x8000;
	BOOL bCTRL = GetKeyState (VK_CONTROL) & 0x8000;

	if (bCTRL)
		return FALSE;

	DWORD bALTShouldPressed = FALSE;
	if(hkey.QueryDWORDValue(_T("MonitorOnAlt"), bALTShouldPressed) != ERROR_SUCCESS)
		return dwMonitor;

	if (bALT == FALSE && bALTShouldPressed)
		return FALSE; 

	return dwMonitor;
}

BOOL OnNavUrl(LPCTSTR pszUrl)
{
	return TRUE;
}

BOOL SendUrlToEM(LPCTSTR pszUrl, LPCTSTR pszReferer, LPCTSTR pszCookies, LPCTSTR pszPostData)
{
	IIE2EMUrlTaker * taker;
	HRESULT hr;
	if (FAILED(hr = CoCreateInstance (CLSID_IE2EMUrlTaker, NULL, CLSCTX_ALL, IID_IIE2EMUrlTaker, (void**) &taker)))
	{
		TCHAR szMsg[1000];
		_tcscpy(szMsg, _T("easyMule might have not been installed!\nPlease confirm you easyMule installation.\n\nError code: 0x"));
		TCHAR sz[100];
		_itot((UINT)hr, sz, 16);
		_tcscat(szMsg, sz);
		MessageBox(NULL, szMsg, _T("Error"), MB_ICONERROR);
		return FALSE;
	}

	USES_CONVERSION;
	CComBSTR url(pszUrl);
	CComBSTR refer(pszReferer);
	BOOL res;
	if(taker->SendUrl(url.Copy(), _T("\1"), refer.Copy(), &res) != S_OK)
		return FALSE;
	return res;
}

static CComBSTR strLastSucc = _T(""), strLastFail = _T("");

static time_t tLastSucc = 0;

CComBSTR GetLastSucc()
{
	if(time(NULL) - tLastSucc >= 20)
		strLastSucc = _T("");
	return strLastSucc;
}

void SetLastSucc(CComBSTR strLast)
{
	strLastSucc = strLast;
	tLastSucc = time(NULL);
}

CComBSTR GetLastFail()
{
	return strLastFail;
}

void SetLastFail(CComBSTR strLast)
{
	strLastFail = strLast;
}
