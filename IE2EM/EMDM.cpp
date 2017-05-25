// EMDM.cpp : CEMDM µÄÊµÏÖ

#include "stdafx.h"
#include "EMDM.h"
#include "Utils.h"
#include <shlguid.h>

// CEMDM

STDMETHODIMP CEMDM::Download(IMoniker *pmk, IBindCtx *pbc, DWORD dwBindVerb, LONG grfBINDF, BINDINFO *pBindInfo, LPCOLESTR pszHeaders, LPCOLESTR pszRedir, UINT uiCP)
{
	USES_CONVERSION;
	LPOLESTR pwszUrl;
	LPMALLOC pMalloc;
	HRESULT hr;

	if (MonitorStatus() == FALSE)
		return E_FAIL;

	if (FAILED (hr = pmk->GetDisplayName (pbc, NULL, &pwszUrl)))
		return hr;

	if (FAILED (hr = CoGetMalloc (1, &pMalloc)))
		return hr;



	hr = E_FAIL;

	LPCTSTR strUrl = OLE2CT (pwszUrl);
	if(GetLastFail() == strUrl)
	{
		SetLastFail(_T(""));
		return E_FAIL;
	}

	SetLastFail(_T(""));
	if (OnNavUrl(strUrl))
	{
		if (SendUrlToEM(strUrl, NULL, NULL, NULL))
			hr = S_OK;
	}

	pMalloc->Free (pwszUrl);
	pMalloc->Release ();

	return hr;
}
