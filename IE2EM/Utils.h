#pragma once

BOOL TestUrl(LPCTSTR pszURL);
BOOL MonitorStatus();
BOOL OnNavUrl(LPCTSTR pszUrl);
BOOL SendUrlToEM(LPCTSTR pszUrl, LPCTSTR pszReferer, LPCTSTR pszCookies, LPCTSTR pszPostData);
CComBSTR GetLastSucc();
void SetLastSucc(CComBSTR strLast);
CComBSTR GetLastFail();
void SetLastFail(CComBSTR strLast);
