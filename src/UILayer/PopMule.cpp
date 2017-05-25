/*
 * $Id: PopMule.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
// PopMule.cpp : 实现文件
//

#include "stdafx.h"
#include "emule.h"
#include "PopMule.h"
#include "IESecurity.h"
#include "emuleDlg.h"
#include "PlayerMgr.h"

#if (WINVER < 0x0500)
/* AnimateWindow() Commands */
#define AW_HOR_POSITIVE             0x00000001
#define AW_HOR_NEGATIVE             0x00000002
#define AW_VER_POSITIVE             0x00000004
#define AW_VER_NEGATIVE             0x00000008
#define AW_CENTER                   0x00000010
#define AW_HIDE                     0x00010000
#define AW_ACTIVATE                 0x00020000
#define AW_SLIDE                    0x00040000
#define AW_BLEND                    0x00080000
#endif

extern UINT _uMainThreadId;

// CPopMule 对话框

IMPLEMENT_DYNCREATE(CPopMule, CDHtmlDialog)

BEGIN_MESSAGE_MAP(CPopMule, CDHtmlDialog)
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_TIMER()
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CPopMule, CDHtmlDialog)
	ON_EVENT(CDHtmlDialog, AFX_IDC_BROWSER, 250 /* BeforeNavigate2 */, _OnBeforeNavigate2, VTS_DISPATCH VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PBOOL)
END_EVENTSINK_MAP()

BEGIN_DHTML_EVENT_MAP(CPopMule)
	DHTML_EVENT_ONCLICK(_T("PlayLaterImg"), OnPlayLater)
	DHTML_EVENT_ONKEYPRESS(_T("PlayLaterImg"), OnPlayLater)
	DHTML_EVENT_ONCLICK(_T("PlayNowImg"), OnPlayNow)
	DHTML_EVENT_ONKEYPRESS(_T("PlayNowImg"), OnPlayNow)
END_DHTML_EVENT_MAP()

CPopMule::CPopMule(CWnd* pParent /*=NULL*/)
	: CDHtmlDialog(CPopMule::IDD, CPopMule::IDH, pParent)
{
	m_pPartFile = NULL;
	ASSERT( GetCurrentThreadId() == _uMainThreadId );
	m_iInCallback = 0;
	m_bResolveImages = true;
	m_uAutoCloseTimer = 0;
	m_bAutoClose = theApp.GetProfileInt(_T("eMule"), _T("PopMuleAutoClose"), 0)!=0;
	m_uWndTransparency = theApp.GetProfileInt(_T("eMule"), _T("PopMuleTransparency"), 0);
	SetHostFlags(m_dwHostFlags
		| DOCHOSTUIFLAG_DIALOG					// MSHTML does not enable selection of the text in the form
		| DOCHOSTUIFLAG_DISABLE_HELP_MENU		// MSHTML does not add the Help menu item to the container's menu.
		);
}

CPopMule::~CPopMule()
{
}

void CPopMule::DoDataExchange(CDataExchange* pDX)
{
	CDHtmlDialog::DoDataExchange(pDX);
}

BOOL CPopMule::OnInitDialog()
{
	ASSERT( GetCurrentThreadId() == _uMainThreadId );
	ASSERT( m_iInCallback == 0 );
	CString strHtmlFile = theApp.GetSkinFileItem(_T("PopMule"), _T("HTML"));
	if (!strHtmlFile.IsEmpty())
	{
		if (_taccess(strHtmlFile, 0) == 0)
		{
			m_strCurrentUrl = CreateFilePathUrl(strHtmlFile, INTERNET_SCHEME_FILE);
			m_nHtmlResID = 0;
			m_szHtmlResID = NULL;
			m_bResolveImages = false;
		}
	}

	if (m_strCurrentUrl.IsEmpty())
	{
		TCHAR szModulePath[MAX_PATH];
		if (GetModuleFileName(AfxGetResourceHandle(), szModulePath, ARRSIZE(szModulePath)))
		{
			m_strCurrentUrl = CreateFilePathUrl(szModulePath, INTERNET_SCHEME_RES);
			m_strCurrentUrl.AppendFormat(_T("/%d"), m_nHtmlResID);
			m_nHtmlResID = 0;
			m_szHtmlResID = NULL;
			m_bResolveImages = true;
		}
	}

	// TODO: Only in debug build: Check the size of the dialog resource right before 'OnInitDialog'
	// to ensure the window is small enough!
	CDHtmlDialog::OnInitDialog();

	if (m_uWndTransparency)
	{
		m_layeredWnd.AddLayeredStyle(m_hWnd);
		m_layeredWnd.SetTransparentPercentage(m_hWnd, m_uWndTransparency);
	}

	SetWindowText(GetResString(IDS_CAPTION) + _T(" v")+CGlobalVariable::GetCurVersionLong());

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}


STDMETHODIMP CPopMule::GetOptionKeyPath(LPOLESTR* /*pchKey*/, DWORD /*dw*/)
{
	ASSERT( GetCurrentThreadId() == _uMainThreadId );
	TRACE(_T("%hs\n"), __FUNCTION__);

	return E_NOTIMPL;
}

BOOL CPopMule::CreateControlSite(COleControlContainer* pContainer, COleControlSite** ppSite, UINT /*nID*/, REFCLSID /*clsid*/)
{
	ASSERT( GetCurrentThreadId() == _uMainThreadId );
	
	CMuleBrowserControlSite *pBrowserSite = new CMuleBrowserControlSite(pContainer, this);
	
	if (!pBrowserSite)
	{
		return FALSE;
	}

	*ppSite = pBrowserSite;

	return TRUE;
}

// CPopMule 消息处理程序

void CPopMule::SetPartFile(CPartFile* pPartFile)
{
	m_pPartFile = pPartFile;
}

HRESULT CPopMule::OnPlayLater(IHTMLElement* /*pElement*/)
{
	ASSERT( GetCurrentThreadId() == _uMainThreadId );
	
	return S_OK;
}

HRESULT CPopMule::OnPlayNow(IHTMLElement* /*pElement*/)
{
	ASSERT( GetCurrentThreadId() == _uMainThreadId );

	if (m_pPartFile)
	{
		int iDot = m_pPartFile->GetFileName().ReverseFind('.');

		if (iDot != -1)
		{
			CString strFileExt = m_pPartFile->GetFileName().Right(m_pPartFile->GetFileName().GetLength() - iDot - 1);		
		
#ifdef _DEBUG
			CPlayerMgr::StartPlayer(m_pPartFile->GetFileHash(), m_pPartFile->GetFileName(), (DWORD)(UINT64)m_pPartFile->GetFileSize(), strFileExt);
#else
			CPlayerMgr::StartPlayer(m_pPartFile->GetFileHash(), m_pPartFile->GetFileName(), (DWORD)m_pPartFile->GetFileSize(), strFileExt);
#endif
		}
	}

	OnClose();
	
	return S_OK;
}

void CPopMule::OnClose()
{
	TRACE("%s\n", __FUNCTION__);
	ASSERT( GetCurrentThreadId() == _uMainThreadId );
	ASSERT( m_iInCallback == 0 );
	KillAutoCloseTimer();

	if (GetAutoClose())
	{
		BOOL (WINAPI *pfnAnimateWindow)(HWND hWnd, DWORD dwTime, DWORD dwFlags);
		(FARPROC&)pfnAnimateWindow = GetProcAddress(GetModuleHandle(_T("user32")), "AnimateWindow");
		
		if (pfnAnimateWindow)
		{
			(*pfnAnimateWindow)(m_hWnd, 200, AW_HIDE | AW_BLEND | AW_CENTER);
		}
	}

	CDHtmlDialog::OnClose();

	theApp.emuledlg->m_pPopMule = NULL;
	DestroyWindow();
}

void CPopMule::OnDestroy()
{
	TRACE("%s\n", __FUNCTION__);
	ASSERT( GetCurrentThreadId() == _uMainThreadId );
	ASSERT( m_iInCallback == 0 );
	KillAutoCloseTimer();
	CDHtmlDialog::OnDestroy();
}

void CPopMule::OnBeforeNavigate(LPDISPATCH pDisp, LPCTSTR pszUrl)
{
	ASSERT( GetCurrentThreadId() == _uMainThreadId );
	TRACE(_T("%hs: %s\n"), __FUNCTION__, pszUrl);
	CDHtmlDialog::OnBeforeNavigate(pDisp, pszUrl);
}

void CPopMule::OnNavigateComplete(LPDISPATCH pDisp, LPCTSTR pszUrl)
{
	ASSERT( GetCurrentThreadId() == _uMainThreadId );
	TRACE(_T("%hs: %s\n"), __FUNCTION__, pszUrl);
	// If the HTML file contains 'OnLoad' scripts, the HTML DOM is fully accessible
	// only after 'DocumentComplete', but not after 'OnNavigateComplete'
	CDHtmlDialog::OnNavigateComplete(pDisp, pszUrl);
}

CString CPopMule::CreateFilePathUrl(LPCTSTR pszFilePath, int nProtocol)
{
	CString strEncodedFilePath;
	if (nProtocol == INTERNET_SCHEME_RES)
	{
		// "res://" protocol has to be specified with 2 slashes ("res:///" does not work)
		strEncodedFilePath = _T("res://");
		strEncodedFilePath += pszFilePath;
	}
	else
	{
		ASSERT( nProtocol == INTERNET_SCHEME_FILE );
		// "file://" protocol has to be specified with 3 slashes
		strEncodedFilePath = _T("file:///");
		strEncodedFilePath += pszFilePath;
	}
	return strEncodedFilePath;
}

void CPopMule::Localize()
{
	CComPtr<IHTMLElement> a;
	GetElementInterface(_T("PlayNow"), &a);
	if (a)
	{
		a->put_title(CComBSTR(RemoveAmbersand(GetResString(IDS_OPENINC))));
		a.Release();
	}
}

void CPopMule::OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR pszUrl)
{
	ASSERT( GetCurrentThreadId() == _uMainThreadId );
	
	if (theApp.emuledlg->m_pPopMule == NULL)
	{
		// FIX ME
		// apperently in some rare cases (high cpu load, fast double clicks) this function is called when the object is destroyed already
		ASSERT(0);
		return;
	}

	/*CCounter cc(m_iInCallback);*/

	TRACE(_T("%hs: %s\n"), __FUNCTION__, pszUrl);
	// If the HTML file contains 'OnLoad' scripts, the HTML DOM is fully accessible
	// only after 'DocumentComplete', but not after 'OnNavigateComplete'
	CDHtmlDialog::OnDocumentComplete(pDisp, pszUrl);

	if (m_bResolveImages)
	{
		TCHAR szModulePath[_MAX_PATH];
		if (GetModuleFileName(AfxGetResourceHandle(), szModulePath, ARRSIZE(szModulePath)))
		{
			CString strFilePathUrl(CreateFilePathUrl(szModulePath, INTERNET_SCHEME_RES));

			static const struct
			{
				LPCTSTR pszImgId;
				LPCTSTR pszResourceId;
			}
			
			_aImg[] = 
			{
				{ _T("connectedImg"),	_T("CONNECTED.GIF") },
			};

			for (int i = 0; i < ARRSIZE(_aImg); i++)
			{
				CComPtr<IHTMLImgElement> elm;
				GetElementInterface(_aImg[i].pszImgId, &elm);
				if (elm)
				{
					CString strResourceURL;
					strResourceURL.Format(_T("%s/%s"), strFilePathUrl, _aImg[i].pszResourceId);
					elm->put_src(CComBSTR(strResourceURL));
				}
			}

			CComPtr<IHTMLTable> elm;
			GetElementInterface(_T("table"), &elm);
			if (elm)
			{
				CString strResourceURL;
				strResourceURL.Format(_T("%s/%s"), strFilePathUrl, _T("TABLEBACKGND.GIF"));
				elm->put_background(CComBSTR(strResourceURL));
				elm.Release();
			}
		}
	}

	if (m_spHtmlDoc)
	{
		CComQIPtr<IHTMLElement> body;
		if (m_spHtmlDoc->get_body(&body) == S_OK && body)
		{
			// NOTE: The IE control will always use the size of the associated dialog resource (IDD_MINIMULE)
			// as the minium window size. 'scrollWidth' and 'scrollHeight' will therefore never return values
			// smaller than the size of that window. To have the auto-size working correctly even for
			// very small window sizes, the size of the dialog resource should therefore be kept very small!
			// TODO: Only in debug build: Check the size of the dialog resource right before 'OnInitDialog'.
			CComQIPtr<IHTMLElement2> body2 = body;
			long lScrollWidth = 0;
			long lScrollHeight = 0;
			if (body2->get_scrollWidth(&lScrollWidth) == S_OK && lScrollWidth > 0 && body2->get_scrollHeight(&lScrollHeight) == S_OK && lScrollHeight > 0)
			{
				AutoSizeAndPosition(CSize(lScrollWidth, lScrollHeight));
			}
		}
	}

	Localize();
	//UpdateContent();

	if (m_bAutoClose)
	{
		CreateAutoCloseTimer();
	}
}

UINT CPopMule::GetTaskbarPos(HWND hwndTaskbar)
{
	if (hwndTaskbar != NULL)
	{
		// See also: Q179908
		APPBARDATA abd = {0};
		abd.cbSize = sizeof abd;
		abd.hWnd = hwndTaskbar;
		SHAppBarMessage(ABM_GETTASKBARPOS, &abd);

		// SHAppBarMessage may fail to get the rectangle...
		CRect rcAppBar(abd.rc);
		if (rcAppBar.IsRectEmpty() || rcAppBar.IsRectNull())
			::GetWindowRect(hwndTaskbar, &abd.rc);

		if (abd.rc.top == abd.rc.left && abd.rc.bottom > abd.rc.right)
			return ABE_LEFT;
		else if (abd.rc.top == abd.rc.left && abd.rc.bottom < abd.rc.right)
			return ABE_TOP;
		else if (abd.rc.top > abd.rc.left)
			return ABE_BOTTOM;
		return ABE_RIGHT;
	}
	return ABE_BOTTOM;
}

void CPopMule::AutoSizeAndPosition(CSize sizClient)
{
	CSize sizDesktop(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
	if (sizClient.cx > sizDesktop.cx/2)
		sizClient.cx = sizDesktop.cx/2;
	if (sizClient.cy > sizDesktop.cy/2)
		sizClient.cy = sizDesktop.cy/2;

	CRect rcWnd;
	GetWindowRect(&rcWnd);
	if (sizClient.cx > 0 && sizClient.cy > 0)
	{
		CRect rcClient(0, 0, sizClient.cx, sizClient.cy);
		AdjustWindowRectEx(&rcClient, GetStyle(), FALSE, GetExStyle());
		rcClient.OffsetRect(-rcClient.left, -rcClient.top);
		rcWnd = rcClient;
	}

	CRect rcTaskbar(0, sizDesktop.cy - 34, sizDesktop.cx, sizDesktop.cy);
	HWND hWndTaskbar = ::FindWindow(_T("Shell_TrayWnd"), 0);
	if (hWndTaskbar)
		::GetWindowRect(hWndTaskbar, &rcTaskbar);
	CPoint ptWnd;
	UINT uTaskbarPos = GetTaskbarPos(hWndTaskbar);
	switch (uTaskbarPos)
	{
	case ABE_TOP:
		ptWnd.x = sizDesktop.cx - 8 - rcWnd.Width();
		ptWnd.y = rcTaskbar.Height() + 8;
		break;
	case ABE_LEFT:
		ptWnd.x = rcTaskbar.Width() + 8;
		ptWnd.y = sizDesktop.cy - 8 - rcWnd.Height();
		break;
	case ABE_RIGHT:
		ptWnd.x = sizDesktop.cx - rcTaskbar.Width() - 8 - rcWnd.Width();
		ptWnd.y = sizDesktop.cy - 8 - rcWnd.Height();
		break;
	default:
		ASSERT( uTaskbarPos == ABE_BOTTOM );
		ptWnd.x = sizDesktop.cx - 8 - rcWnd.Width();
		ptWnd.y = sizDesktop.cy - rcTaskbar.Height() - 8 - rcWnd.Height();
	}

	SetWindowPos(NULL, ptWnd.x, ptWnd.y, rcWnd.Width(), rcWnd.Height(), SWP_NOZORDER | SWP_SHOWWINDOW);
}

void CPopMule::CreateAutoCloseTimer()
{
	if (m_uAutoCloseTimer == 0)
		m_uAutoCloseTimer = SetTimer(IDT_AUTO_CLOSE_TIMER, 3000, NULL);
}

void CPopMule::KillAutoCloseTimer()
{
	if (m_uAutoCloseTimer != 0)
	{
		VERIFY( KillTimer(m_uAutoCloseTimer) );
		m_uAutoCloseTimer = 0;
	}
}

void CPopMule::OnTimer(UINT nIDEvent)
{
	ASSERT( GetCurrentThreadId() == _uMainThreadId );
	if (m_bAutoClose && nIDEvent == m_uAutoCloseTimer)
	{
		KillAutoCloseTimer();

		CPoint pt;
		GetCursorPos(&pt);
		CRect rcWnd;
		GetWindowRect(&rcWnd);
		if (!rcWnd.PtInRect(pt))
			PostMessage(WM_CLOSE);
		else
			CreateAutoCloseTimer();
	}
	CDHtmlDialog::OnTimer(nIDEvent);
}

STDMETHODIMP CPopMule::TranslateUrl(DWORD /*dwTranslate*/, OLECHAR* pchURLIn, OLECHAR** ppchURLOut)
{
	ASSERT( GetCurrentThreadId() == _uMainThreadId );
	UNREFERENCED_PARAMETER(pchURLIn);
	TRACE(_T("%hs: %ls\n"), __FUNCTION__, pchURLIn);
	*ppchURLOut = NULL;
	return S_FALSE;
}

void CPopMule::_OnBeforeNavigate2(LPDISPATCH pDisp, VARIANT* URL, VARIANT* /*Flags*/, VARIANT* /*TargetFrameName*/, VARIANT* /*PostData*/, VARIANT* /*Headers*/, BOOL* Cancel)
{
	ASSERT( GetCurrentThreadId() == _uMainThreadId );
	CString strURL(V_BSTR(URL));
	TRACE(_T("%hs: %s\n"), __FUNCTION__, strURL);

	// No external links allowed!
	TCHAR szScheme[INTERNET_MAX_SCHEME_LENGTH];
	URL_COMPONENTS Url = {0};
	Url.dwStructSize = sizeof(Url);
	Url.lpszScheme = szScheme;
	Url.dwSchemeLength = ARRSIZE(szScheme);
	if (InternetCrackUrl(strURL, 0, 0, &Url) && Url.dwSchemeLength)
	{
		if (Url.nScheme != INTERNET_SCHEME_UNKNOWN  // <absolute local file path>
			&& Url.nScheme != INTERNET_SCHEME_RES	// res://...
			&& Url.nScheme != INTERNET_SCHEME_FILE)	// file://...
		{
			*Cancel = TRUE;
			return;
		}
	}

	OnBeforeNavigate(pDisp, strURL);
}

STDMETHODIMP CPopMule::ShowContextMenu(DWORD /*dwID*/, POINT* /*ppt*/, IUnknown* /*pcmdtReserved*/, IDispatch* /*pdispReserved*/)
{
	ASSERT( GetCurrentThreadId() == _uMainThreadId );
	/*CCounter cc(m_iInCallback);*/
	// Avoid IE context menu
	return S_OK;	// S_OK = Host displayed its own user interface (UI). MSHTML will not attempt to display its UI.
}

STDMETHODIMP CPopMule::TranslateAccelerator(LPMSG lpMsg, const GUID* /*pguidCmdGroup*/, DWORD /*nCmdID*/)
{
	ASSERT( GetCurrentThreadId() == _uMainThreadId );
	/*CCounter cc(m_iInCallback);*/
	// Allow only some basic keys
	//
	//TODO: Allow the ESC key (for closing the window); does currently not work properly because
	// we don't get a callback that the window was just hidden(!) by MSHTML.
	switch (lpMsg->message)
	{
	case WM_CHAR:
		switch (lpMsg->wParam)
		{
		case ' ':			// SPACE - Activate a link
			return S_FALSE;	// S_FALSE = Let the control process the key stroke.
		}
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
		switch (lpMsg->wParam)
		{
		case VK_TAB:		// Cycling through controls which can get the focus
		case VK_SPACE:		// Activate a link
			return S_FALSE; // S_FALSE = Let the control process the key stroke.
		case VK_ESCAPE:
			//TODO: Small problem here.. If the options dialog was open and was closed with ESC,
			//we still get an ESC here too and the HTML window would be closed too..
			//PostMessage(WM_CLOSE);
			break;
		}
		break;
	}

	// Avoid any IE shortcuts (especially F5 (Refresh) which messes up the content)
	return S_OK;	// S_OK = Don't let the control process the key stroke.
}
