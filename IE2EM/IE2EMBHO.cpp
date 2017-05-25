// IE2EMBHO.cpp : CIE2EMBHO µÄÊµÏÖ

#include "stdafx.h"
#include "exdispid.h"
#include "IE2EMBHO.h"
#include "Utils.h"

_COM_SMARTPTR_TYPEDEF(IHTMLInputElement, __uuidof(IHTMLInputElement));      

// CIE2EMBHO

CComBSTR CIE2EMBHO::m_lastUrl = "";

STDMETHODIMP CIE2EMBHO::GetSite(REFIID riid, void **ppvSite)
{
	if (m_spWebBrowser2 == NULL || ppvSite == NULL)
		return E_INVALIDARG;

	*ppvSite = NULL;
	return m_spWebBrowser2.QueryInterface(riid, (IWebBrowser2**) ppvSite);
}

STDMETHODIMP CIE2EMBHO::SetSite(IUnknown *pSite)
{
	if (m_spWebBrowser2 != NULL)
		Disconnect();
	m_spWebBrowser2 = pSite;
	if (m_spWebBrowser2 == NULL)
		return pSite == NULL ? S_OK : E_INVALIDARG;

	m_spCPC = m_spWebBrowser2;
	if (m_spCPC == NULL)
		return E_INVALIDARG;

	return Connect ();
}

STDMETHODIMP CIE2EMBHO::Connect()
{
	HRESULT hr;

	hr = Disconnect();
	if (FAILED (hr))
		return hr;

	hr = m_spCPC->FindConnectionPoint(DIID_DWebBrowserEvents2, &m_spCP);
	if (FAILED (hr))
		return hr;

	IIE2EMBHO *pThis = (IIE2EMBHO*) this;

	hr = m_spCP->Advise (pThis, &m_dwCookie);

	return S_OK;
}

STDMETHODIMP CIE2EMBHO::Disconnect()
{
	if (m_spCP == NULL)
		return S_FALSE;

	HRESULT hr = m_spCP->Unadvise (m_dwCookie);
	if (FAILED (hr))
		return hr;

	m_spCP = NULL;
	m_spCPC = NULL;

	return S_OK;
}

STDMETHODIMP CIE2EMBHO::BeforeNavigate2(IDispatch *, VARIANT *url, VARIANT *flags, VARIANT *tfn, VARIANT *pd, VARIANT *headers, VARIANT_BOOL *bCancel)
{
	*bCancel = VARIANT_FALSE;

	USES_CONVERSION;

	LPCTSTR pszUrl = W2CT (url->bstrVal);

	if (TestUrl(pszUrl) == FALSE)
	{
		if(tfn->bstrVal == NULL || tfn->bstrVal[0] == 0)
			m_lastUrl = url->bstrVal;
		return S_OK;
	}

	CComBSTR strCookies, strPostData, strReferer;
	IDispatchPtr spdDoc;
	m_spWebBrowser2->get_Document (&spdDoc);
	IHTMLDocument2Ptr spDoc;
	if (spdDoc)
		spDoc = spdDoc;

	if (spDoc)
	{		
		BSTR bstr = NULL;
		spDoc->get_cookie (&bstr);
		if (bstr)
		{
			strCookies = bstr;
			SysFreeString (bstr);
		}

		bstr = NULL;
		spDoc->get_URL (&bstr);
		if (bstr)
		{
			strReferer = bstr;
			SysFreeString (bstr);
		}

		IHTMLElementCollectionPtr spForms;
		spDoc->get_forms (&spForms);

		long cForms = 0;
		if (spForms)
			spForms->get_length (&cForms);

		bool bFound = false;

		for (long i = 0; bFound == false && i < cForms; i++)
		{
			IDispatchPtr spd;
			spForms->item (CComVariant (i), CComVariant (i), &spd);

			IHTMLFormElementPtr spForm (spd);
			if (spForm == NULL)
				continue;

			BSTR bstr = NULL;
			spForm->get_action (&bstr);
			bFound = bstr != NULL && _tcscmp (pszUrl, bstr) == 0;
			SysFreeString (bstr);
			if (bFound == false)
				continue;

			bstr = NULL;
			spForm->get_method (&bstr);
			if (bstr == NULL || _tcsicmp (bstr, _T("post")))
				break;
			SysFreeString (bstr);

			IHTMLElementPtr spFormElem (spForm);
			if (spFormElem == NULL)
			{
				bFound = false;
				continue;
			}

			WalkThroughForm (spFormElem, strPostData);

			if (strPostData != "" && 
				strPostData [strPostData.Length() - 1] == '&')
				strPostData [strPostData.Length() - 1] = 0;
		}
	}
	else
		strReferer = m_lastUrl;

	if (SendUrlToEM(pszUrl, strReferer, strCookies != "" ? strCookies : NULL,
		strPostData != "" ? strPostData : NULL))
		*bCancel = VARIANT_TRUE;
	else
		SetLastFail(pszUrl);

	return S_OK;
}

void CIE2EMBHO::WalkThroughForm(IHTMLElement *pElement, CComBSTR& str)
{
	USES_CONVERSION;

	IDispatchPtr spd;
	pElement->get_children (&spd);
	IHTMLElementCollectionPtr spels (spd);

	long cElements = 0;
	if (spels != NULL)
		spels->get_length (&cElements);

	for (int j = 0; j < cElements; j++)
	{
		spd = NULL;
		spels->item (CComVariant (j), CComVariant (j), &spd);

		IHTMLInputElementPtr spInp (spd);
		if (spInp != NULL)
		{
			BSTR bstr = NULL, bstr2 = NULL;
			spInp->get_name (&bstr);				
			spInp->get_value (&bstr2);
			if (bstr)
			{
				str += W2A (bstr);
				str += "=";
				SysFreeString (bstr);
			}
			if (bstr2)
			{
				str += W2A (bstr2);
				SysFreeString (bstr2);
			}

			if (bstr || bstr2)
				str += "&";
		}

		IHTMLElementPtr spel (spd);
		if (spel != NULL)
			WalkThroughForm (spel, str);
	}
}
