// EMDM.h : CEMDM µÄÉùÃ÷

#pragma once
#include "resource.h"       // Ö÷·ûºÅ

#include "IE2EM.h"
#include "downloadmgr.h"
#include <ObjSafe.h>

// CEMDM

class ATL_NO_VTABLE CEMDM : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CEMDM, &CLSID_EMDM>,
	public IDownloadManager
{
public:
	CEMDM()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_EMDM)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_CATEGORY_MAP(CEMDM)
	IMPLEMENTED_CATEGORY(CATID_SafeForScripting)   
	IMPLEMENTED_CATEGORY(CATID_SafeForInitializing)   
END_CATEGORY_MAP()

BEGIN_COM_MAP(CEMDM)
	COM_INTERFACE_ENTRY(IDownloadManager)
END_COM_MAP()  

public:
protected:
	STDMETHOD (Download) (IMoniker *pmk, IBindCtx *pbc, DWORD dwBindVerb, LONG grfBINDF, BINDINFO *pBindInfo, LPCOLESTR pszHeaders, LPCOLESTR pszRedir, UINT uiCP);
};

OBJECT_ENTRY_AUTO(__uuidof(EMDM), CEMDM)
