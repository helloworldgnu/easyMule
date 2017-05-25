// IE2EM.cpp : DLL 导出的实现。

#include "stdafx.h"
#include "resource.h"
#include "IE2EM.h"
class CIE2EMModule : public CAtlDllModuleT< CIE2EMModule >
{
public :
	DECLARE_LIBID(LIBID_IE2EMLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_IE2EM, "{BCEADC2F-0C89-43F5-8DE9-068B5BC265E2}")
};

CIE2EMModule _AtlModule;


// DLL 入口点
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	hInstance;
	if(dwReason == DLL_PROCESS_ATTACH)
	{
		TCHAR pszLoader[MAX_PATH];
		GetModuleFileName(NULL, pszLoader, MAX_PATH);
		_tcslwr(pszLoader);
		if (_tcsstr(pszLoader, _T("explorer.exe")))
			return FALSE;
	}
	return _AtlModule.DllMain(dwReason, lpReserved);
}


// 用于确定 DLL 是否可由 OLE 卸载
STDAPI DllCanUnloadNow(void)
{
    return _AtlModule.DllCanUnloadNow();
}


// 返回一个类工厂以创建所请求类型的对象
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}


// DllRegisterServer - 将项添加到系统注册表
STDAPI DllRegisterServer(void)
{
	HRESULT hr = _AtlModule.DllRegisterServer();
	if ( FAILED(hr) )
		return hr;

	return S_OK;
}


// DllUnregisterServer - 将项从系统注册表中移除
STDAPI DllUnregisterServer(void)
{
	CRegKey key;
	key.Open (HKEY_CURRENT_USER, _T("Software\\Microsoft\\Internet Explorer"));
	key.DeleteValue (_T("DownloadUI"));

	HRESULT hr = _AtlModule.DllUnregisterServer();
	if ( FAILED(hr) )
		return hr;

	return hr;
}
