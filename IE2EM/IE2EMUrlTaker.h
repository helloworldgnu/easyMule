// IE2EMUrlTaker.h : CIE2EMUrlTaker µÄÉùÃ÷

#pragma once
#include "resource.h"       // Ö÷·ûºÅ

#include "IE2EM.h"
#include <ObjSafe.h>


// CIE2EMUrlTaker

class ATL_NO_VTABLE CIE2EMUrlTaker : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CIE2EMUrlTaker, &CLSID_IE2EMUrlTaker>,
	public IDispatchImpl<IIE2EMUrlTaker, &IID_IIE2EMUrlTaker, &LIBID_IE2EMLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
	CIE2EMUrlTaker()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_IE2EMURLTAKER)

BEGIN_CATEGORY_MAP(CIE2EMUrlTaker)
	IMPLEMENTED_CATEGORY(CATID_SafeForScripting)   
	IMPLEMENTED_CATEGORY(CATID_SafeForInitializing)   
END_CATEGORY_MAP()

BEGIN_COM_MAP(CIE2EMUrlTaker)
	COM_INTERFACE_ENTRY(IIE2EMUrlTaker)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

public:
	STDMETHOD(SendUrl)(BSTR strUrl, BSTR strInfo, BSTR strRef, BOOL * result);
	STDMETHOD(GetEmuleVersion)(BSTR* pstrVer);
	STDMETHOD(get_Version)(BSTR* pVal);
};

OBJECT_ENTRY_AUTO(__uuidof(IE2EMUrlTaker), CIE2EMUrlTaker)
