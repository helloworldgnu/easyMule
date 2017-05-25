/*
 * $Id: HtmlCtrl.cpp 9780 2009-01-07 07:58:37Z dgkang $
 * 
 * this file is part of easyMule
 * Copyright (C)2002-2008 VeryCD Dev Team ( strEmail.Format("%s@%s", "emuledev", "verycd.com") / http: * www.easymule.org )
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
// HtmlCtrl.cpp : 实现文件
//

#include "stdafx.h"
#include "emule.h"
#include "emuleDlg.h"
#include "HtmlCtrl.h"
#include "WebbrowserWnd.h"
#include "UserMsgs.h"
#include "FindWnd.h"
#include <Version.h>
#include ".\htmlctrl.h"

// CHtmlCtrl

#define WM_NVTO		(WM_USER+1000)

class NvToParam
{
public:
	CString URL;
	DWORD Flags;
	CString TargetFrameName;
	CByteArray PostedData;
	CString Headers;
};

IMPLEMENT_DYNCREATE(CHtmlCtrl, CHtmlView)


void CHtmlCtrl::DoDataExchange(CDataExchange* pDX)
{
	CHtmlView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CHtmlCtrl, CHtmlView)
	ON_WM_DESTROY()
//	ON_WM_ERASEBKGND()
	ON_WM_MOUSEACTIVATE()
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_MESSAGE(WM_NVTO,OnNvTo)
END_MESSAGE_MAP()


// CHtmlCtrl 诊断

#ifdef _DEBUG
void CHtmlCtrl::AssertValid() const
{
	CHtmlView::AssertValid();
}

void CHtmlCtrl::Dump(CDumpContext& dc) const
{
	CHtmlView::Dump(dc);
}
#endif //_DEBUG


// CHtmlCtrl 消息处理程序
BOOL CHtmlCtrl::CreateFromStatic(UINT nID, CWnd* pParent)
{
	CStatic wndStatic;
	if (!wndStatic.SubclassDlgItem(nID, pParent))
		return FALSE;

	// Get static control rect, convert to parent's client coords.
	CRect rc;
	wndStatic.GetWindowRect(&rc);
	pParent->ScreenToClient(&rc);
	wndStatic.DestroyWindow();

	// create HTML control (CHtmlView)
	return Create(NULL,						 // class name
		NULL,										 // title
		(WS_CHILD | WS_VISIBLE ),			 // style
		rc,										 // rectangle
		pParent,									 // parent
		nID,										 // control ID
		NULL);									 // frame/doc context not used
}

//开始链接时会触发此事件
void CHtmlCtrl::OnBeforeNavigate2(LPCTSTR lpszURL, DWORD nFlags, LPCTSTR lpszTargetFrameName, CByteArray& baPostedData, LPCTSTR lpszHeaders, BOOL* pbCancel)
{

	m_bCancel = FALSE;
	if (NULL == lpszTargetFrameName || _T('\0') == lpszTargetFrameName[0])
	{
		GetParent()->SendMessage(UM_HC_BEFORE_NAVI, 0, (LPARAM) lpszURL);
	}
	//MODIFIED by fengwen on 2007/02/13	<end> :	其父窗口不一定是CWebBrowserWnd
	*pbCancel = m_bCancel;
	
}


BOOL CHtmlCtrl::OnAmbientProperty(COleControlSite* pSite,DISPID dispid, VARIANT* pvar)
{
	if (dispid == DISPID_AMBIENT_USERAGENT)
	{
		CStringW strUserAgent;
		strUserAgent.AppendFormat(L"eMule 0%u",VC_VERSION_BUILD);

		pvar->vt	  = VT_BSTR;
		pvar->bstrVal = ::SysAllocString((LPCWSTR)strUserAgent);
		return TRUE;
	}
	return CHtmlView::OnAmbientProperty(pSite, dispid, pvar);
}
/*

在修改Head 中的User-Agent 用BeforeNavigate2方法产生的副作用很大，尤其影响POST头。

void CHtmlCtrl::BeforeNavigate2(LPDISPATCH pDisp, VARIANT* URL, VARIANT* Flags, VARIANT* TargetFrameName, 
								VARIANT* PostData, VARIANT* Headers, VARIANT_BOOL* Cancel)
{	

	CComPtr<IWebBrowser2> pWeb;
	HRESULT hr = pDisp->QueryInterface(IID_IWebBrowser2,(void**)&pWeb);
	if (SUCCEEDED(hr))
	{
		CString strHeaders(V_BSTR(Headers));
		CString szUrl(V_BSTR(URL));

		if(strHeaders.Find(_T("User-Agent")) == -1 && StrStrI((LPCTSTR)szUrl,_T("http://www.verycd.com")) == LPCTSTR(szUrl))
		{		
				*Cancel = TRUE;

				strHeaders.AppendFormat(_T("User-Agent: eMule 0%u"),VC_VERSION_BUILD);
				strHeaders += _T("\r\n");
				COleVariant vHeaders(strHeaders);

				if( PostData && PostData->vt == (VT_VARIANT|VT_BYREF) && PostData->pvarVal->vt != VT_EMPTY)
				{ 
					//BeforeNavigate2 和 Navigate2中的PostData 格式是不一样的,在此需要做格式转换.

					VARIANT* vtPostedData = V_VARIANTREF(PostData);					
					if (V_VT(vtPostedData) & VT_ARRAY)
					{
						COleSafeArray vPostData;

						ASSERT(vtPostedData->parray->cDims == 1 && vtPostedData->parray->cbElements == 1);
						vtPostedData->vt |= VT_UI1;

						
						COleSafeArray safe(vtPostedData);
						DWORD dwSize = safe.GetOneDimSize();

						LPBYTE lpByte = NULL;
						safe.AccessData((void**)&lpByte);
						DWORD dwPostDataLen = lpByte[dwSize - 1] == '\0' ? dwSize - 1 : dwSize;
						vPostData.CreateOneDim(VT_UI1, dwPostDataLen, lpByte);
						safe.UnaccessData();						
						pWeb->Navigate2(URL,Flags,TargetFrameName,vtPostedData,vHeaders);
						return;
					}
#if 0
					SAFEARRAY* pData = PostData->pvarVal->parray;
					VARIANT vPostData;
					VariantInit(&vPostData);
					vPostData.vt = VT_ARRAY | VT_UI1;
					vPostData.parray = pData;					
					pWeb->Navigate2(URL,Flags,TargetFrameName,&vPostData,vHeaders);
					return;
#endif
				}  
				pWeb->Navigate2(URL,Flags,TargetFrameName,PostData,vHeaders);
		}
		else
			CHtmlView::BeforeNavigate2(pDisp,URL,Flags,TargetFrameName,PostData,Headers,Cancel);
	}

}
*/
LRESULT CHtmlCtrl::OnNvTo(WPARAM wParam, LPARAM lParam)
{
	//该代码废弃了
	NvToParam* pNvTo = (NvToParam*)wParam;
	Navigate((LPCTSTR)pNvTo->URL, 
		pNvTo->Flags, 
		(LPCTSTR)pNvTo->TargetFrameName, 
		(LPCTSTR)pNvTo->Headers,
		 pNvTo->PostedData.GetData());

	delete pNvTo;
	return 1;
}

void CHtmlCtrl::OnDestroy()
{
	// This is probably unnecessary since ~CHtmlView does it, but
	// safer to mimic CHtmlView::OnDestroy.
	if (m_pBrowserApp) 
	{
		m_pBrowserApp = NULL;
	}
	CWnd::OnDestroy(); // bypass CView doc/frame stuff
}

void CHtmlCtrl::OnDocumentComplete(LPCTSTR /*lpszURL*/)
{
	//MODIFIED by fengwen on 2007/02/13	<begin> :	其父窗口不一定是CWebBrowserWnd
	//CWebBrowserWnd* pBrowser = ((CWebBrowserWnd*)GetParent());
	//pBrowser->StopAnimation();
	if(theApp.m_BrowserToolbarInfo.Enable())
	{
		theApp.m_BrowserToolbarInfo.GetBrowserToolbarCtrl()->EnableButton(TB_STOP, FALSE);
	}
	
	GetParent()->SendMessage(UM_HC_DOC_CMPL, 0, 0);
	//MODIFIED by fengwen on 2007/02/13	<end> :	其父窗口不一定是CWebBrowserWnd
}

int CHtmlCtrl::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	return CHtmlView::OnMouseActivate(pDesktopWnd, nHitTest, message);
}

void CHtmlCtrl::OnNavigateComplete2(LPCTSTR /*strURL*/)
{
	//MODIFIED by fengwen on 2007/02/13	<begin> :	其父窗口不一定是CWebBrowserWnd
	//CWebBrowserWnd* pBrowser = ((CWebBrowserWnd*)GetParent());
	//pBrowser->SetAddress(GetLocationURL());
	if(theApp.m_BrowserToolbarInfo.Enable())
	{
		theApp.m_BrowserToolbarInfo.GetBrowserToolbarCtrl()->EnableButton(TB_STOP, TRUE);
	}
	
	GetParent()->SendMessage(UM_HC_NAVI_CMPL, 0, (LPARAM) (LPCTSTR) GetLocationURL());
	//MODIFIED by fengwen on 2007/02/13	<end> :	其父窗口不一定是CWebBrowserWnd

	HWND hwnd = GetBrowserHwnd();

	if(!m_IEManagerWnd.m_hWnd)
	{
		m_IEManagerWnd.SubclassWindow(hwnd);
	}
}

void CHtmlCtrl::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);			//Changed by thilon on 2006.08.10

	if (::IsWindow(m_wndBrowser.m_hWnd)) 
	{ 
		CRect rect; 
		GetClientRect(rect); 
		// 就这一句与CHtmlView的不同
		::AdjustWindowRectEx(rect, GetStyle(), FALSE, WS_EX_CLIENTEDGE);
		m_wndBrowser.SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
	} 
	if (NULL != m_sttcProgress.GetSafeHwnd())
	{
		m_sttcProgress.UpdatePos();
	}
}

void CHtmlCtrl::OnStatusTextChange(LPCTSTR lpszText)
{
	//MODIFIED by fengwen on 2007/02/13	<begin> :	其父窗口不一定是CWebBrowserWnd
	//((CWebBrowserWnd*)GetParent())->m_status.SetWindowText(lpszText);
	GetParent()->SendMessage(UM_HC_STATUS_TXT_CHANGE, 0, (LPARAM) lpszText);
	//MODIFIED by fengwen on 2007/02/13	<end> :	其父窗口不一定是CWebBrowserWnd
}

void CHtmlCtrl::PostNcDestroy()
{
	CHtmlView::PostNcDestroy();
}

void CHtmlCtrl::OnTitleChange(LPCTSTR lpszText)
{
	// TODO: 在此添加专用代码和/或调用基类
	GetParent()->SendMessage(UM_HC_TITLE_CHANGE, 0, (LPARAM) lpszText);

	CHtmlView::OnTitleChange(lpszText);
}

void CHtmlCtrl::OnProgressChange(long nProgress, long nProgressMax)
{
	m_sttcProgress.UpdateProgress(nProgress, nProgressMax);

	CHtmlView::OnProgressChange(nProgress, nProgressMax);
}

int CHtmlCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CHtmlView::OnCreate(lpCreateStruct) == -1)
		return -1;
	m_sttcProgress.CreatePS(this);
    bForwordEnable = false;
	bBackEnable = false;
	return 0;
}


void CHtmlCtrl::OnCommandStateChange(long nCommand, BOOL bEnable)
{

	if(theApp.m_BrowserToolbarInfo.Enable())
	{
		switch(nCommand)
		{
		case CSC_NAVIGATEFORWARD:
			theApp.m_BrowserToolbarInfo.GetBrowserToolbarCtrl()->EnableButton(TB_FORWARD, bEnable);
			bForwordEnable = bEnable;
			break;
		case CSC_NAVIGATEBACK:
			theApp.m_BrowserToolbarInfo.GetBrowserToolbarCtrl()->EnableButton(TB_BACK, bEnable);
			bBackEnable = bEnable;
			break;
		default:
			CHtmlView::OnCommandStateChange(nCommand, bEnable);
		}
	}
}
HWND CHtmlCtrl::GetBrowserHwnd(void)
{
	CFindWnd ies(this->GetSafeHwnd(), L"Internet Explorer_Server");
	return ies.m_hWnd;
}

//{begin} VC-dgkang 2008年11月18日
void CHtmlCtrl::OnNewWindow2(LPDISPATCH* ppDisp, BOOL* Cancel)
{
	CString sServer, sObject;
	INTERNET_PORT nPort;
	DWORD dwServiceType;
	AfxParseURL(m_NewWindowsURL, dwServiceType, sServer, sObject, nPort);

	if (StrStrI(sServer,_T("verycd.com")))
	{	
		if (theApp.emuledlg && ::IsWindow(theApp.emuledlg->m_mainTabWnd.m_dlgResource.m_hWnd))
		{
			CWebBrowserWnd * pWnd = theApp.emuledlg->m_mainTabWnd.m_dlgResource.OpenNewUrl(_T("about:blank"),NULL,NULL);
			if (pWnd)
			{
				*ppDisp = pWnd->m_pExplorer->GetApplication();
				*Cancel = FALSE;
			}		
			else
				*Cancel = TRUE;
		}	
	}
}

HRESULT CHtmlCtrl::OnTranslateUrl(DWORD dwTranslate,OLECHAR* pchURLIn, OLECHAR** ppchURLOut)
{
	m_NewWindowsURL = pchURLIn;
	return S_FALSE;
}


//{end}
