// ProtMon.h : CProtMon µÄÉùÃ÷

#pragma once
#include "resource.h"       // Ö÷·ûºÅ

#include "IE2EM.h"
#include <urlmon.h>
#include <ObjSafe.h>


// CProtMon

class ATL_NO_VTABLE CProtMon : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CProtMon, &CLSID_ProtMon>,
	public IInternetProtocol
{
public:
	CProtMon()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_PROTMON)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_CATEGORY_MAP(CProtMon)
	IMPLEMENTED_CATEGORY(CATID_SafeForScripting)   
	IMPLEMENTED_CATEGORY(CATID_SafeForInitializing)   
END_CATEGORY_MAP()

BEGIN_COM_MAP(CProtMon)
	COM_INTERFACE_ENTRY(IInternetProtocol)
	COM_INTERFACE_ENTRY(IInternetProtocolRoot)
END_COM_MAP()

public:
protected:
	STDMETHOD(UnlockRequest)();
	STDMETHOD(Seek )(LARGE_INTEGER, DWORD, ULARGE_INTEGER*);
	STDMETHOD(Read )(LPVOID, ULONG, ULONG*);
	STDMETHOD(LockRequest )(DWORD);
	STDMETHOD(Terminate )(DWORD);
	STDMETHOD(Suspend)();
	STDMETHOD(Start )(LPCWSTR pszwUrl, IInternetProtocolSink*, IInternetBindInfo*, DWORD, DWORD);
	STDMETHOD(Resume)();
	STDMETHOD(Continue )(PROTOCOLDATA*);
	STDMETHODIMP Abort (HRESULT, DWORD);
};

OBJECT_ENTRY_AUTO(__uuidof(ProtMon), CProtMon)
