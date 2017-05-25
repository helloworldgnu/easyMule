// IE2EMBHO.h : CIE2EMBHO µÄÉùÃ÷

#pragma once
#include "resource.h"       // Ö÷·ûºÅ

#include "IE2EM.h"
#include <Exdisp.h>
#include <mshtml.h>
#include <comdef.h>
#include <ObjSafe.h>

_COM_SMARTPTR_TYPEDEF(IWebBrowser2, __uuidof(IWebBrowser2));
// CIE2EMBHO
class ATL_NO_VTABLE CIE2EMBHO : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CIE2EMBHO, &CLSID_IE2EMBHO>,
	public IObjectWithSiteImpl<CIE2EMBHO>,
	public IDispatchImpl<IIE2EMBHO, &IID_IIE2EMBHO, &LIBID_IE2EMLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
	CIE2EMBHO()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_IE2EMBHO)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_CATEGORY_MAP(CIE2EMBHO)
	IMPLEMENTED_CATEGORY(CATID_SafeForScripting)   
	IMPLEMENTED_CATEGORY(CATID_SafeForInitializing)   
END_CATEGORY_MAP()

BEGIN_COM_MAP(CIE2EMBHO)
	COM_INTERFACE_ENTRY(IIE2EMBHO)
	COM_INTERFACE_ENTRY(IObjectWithSite)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

public:
	STDMETHOD(BeforeNavigate2)(IDispatch*, VARIANT* url, VARIANT* flags, VARIANT* tfn, VARIANT* pd, VARIANT* headers, VARIANT_BOOL* bCancel);
protected:
	DWORD m_dwCookie;
	IWebBrowser2Ptr m_spWebBrowser2; 
	IConnectionPointContainerPtr m_spCPC; 
	IConnectionPointPtr m_spCP; 
	static CComBSTR m_lastUrl;
	void WalkThroughForm(IHTMLElement *pElement, CComBSTR& str);
	STDMETHOD(Connect)();
	STDMETHOD(Disconnect)();
	STDMETHOD(SetSite)(IUnknown* pSite);
	STDMETHOD(GetSite)(REFIID riid, void** ppvSite);
};

OBJECT_ENTRY_AUTO(__uuidof(IE2EMBHO), CIE2EMBHO)
