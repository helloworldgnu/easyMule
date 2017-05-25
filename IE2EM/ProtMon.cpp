// ProtMon.cpp : CProtMon µÄÊµÏÖ

#include "stdafx.h"
#include "ProtMon.h"
#include "Utils.h"
#include <objbase.h>
#include <comdef.h>        

// CProtMon

STDMETHODIMP CProtMon::Abort(HRESULT, DWORD)
{
	return E_NOTIMPL;
}

STDMETHODIMP CProtMon::Continue(PROTOCOLDATA *)
{
	return E_NOTIMPL;
}

STDMETHODIMP CProtMon::Resume()
{
	return E_NOTIMPL;
}

STDMETHODIMP CProtMon::Start(LPCWSTR pszwUrl, IInternetProtocolSink *, IInternetBindInfo *, DWORD, DWORD)
{
	USES_CONVERSION;

	LPCTSTR pszUrl = W2CT(pszwUrl);

	if (TestUrl(pszUrl) == FALSE)
		return INET_E_USE_DEFAULT_PROTOCOLHANDLER;

	if (SendUrlToEM(pszUrl, NULL, NULL, NULL) == FALSE)
		return INET_E_USE_DEFAULT_PROTOCOLHANDLER;

	return INET_E_DATA_NOT_AVAILABLE;
}

STDMETHODIMP CProtMon::Suspend()
{
	return E_NOTIMPL;
}

STDMETHODIMP CProtMon::Terminate(DWORD)
{
	return E_NOTIMPL;
}

STDMETHODIMP CProtMon::LockRequest(DWORD)
{
	return E_NOTIMPL;
}

STDMETHODIMP CProtMon::Read(LPVOID, ULONG, ULONG *)
{
	return E_NOTIMPL;
}

STDMETHODIMP CProtMon::Seek(LARGE_INTEGER, DWORD, ULARGE_INTEGER*)
{
	return E_NOTIMPL;
}

STDMETHODIMP CProtMon::UnlockRequest()
{
	return E_NOTIMPL;
}

