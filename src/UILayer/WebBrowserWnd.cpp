/*
 * $Id: WebBrowserWnd.cpp 9073 2008-12-18 04:38:51Z dgkang $
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
// WebBrowser.cpp : 实现文件
//

#include "stdafx.h"
#include "emule.h"
#include "emuleDlg.h"
#include "WebBrowserWnd.h"
#include "otherfunctions.h"
#include "Preferences.h"
#include "UserMsgs.h"
#include "CmdFuncs.h"

#define VERYCD_HOMEPAGE _T("http://www.verycd.com/start/")
//#define VERYCD_HOMEPAGE _T("about:blank")
#define POST_FORM_HEADER _T("Content-Type: application/x-www-form-urlencoded\r\n")

// CWebBrowserWnd 对话框

IMPLEMENT_DYNAMIC(CWebBrowserWnd, CDialog)
CWebBrowserWnd::CWebBrowserWnd(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CWebBrowserWnd::IDD, pParent)
{
	m_pExplorer = NULL;
	//m_pExplorer = new CHtmlCtrl(); // VC-SearchDream[2006-12-25]: Move to OnInitDialog 

//VC-dgkang 2008年7月8日
#ifdef _FOREIGN_VERSION
	m_strOpenUrl = thePrefs.m_strHomePage;
#else
	m_strOpenUrl = VERYCD_HOMEPAGE;
#endif

}

CWebBrowserWnd::~CWebBrowserWnd()
{
	 if (m_pExplorer)
	 {
		 m_pExplorer = NULL, delete m_pExplorer;
	 }
}

void CWebBrowserWnd::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
//	DDX_Control(pDX, IDC_CUSTOM_BAR, m_toolbar);
//	DDX_Control(pDX, IDC_VERYEARTH, m_animate);
	DDX_Control(pDX, IDC_STATIC_WEBINFO, m_status);
//	DDX_Control(pDX, IDC_ADDRESSBOX, m_addressbox);
}


BEGIN_MESSAGE_MAP(CWebBrowserWnd, CResizableDialog)
	ON_CBN_SELENDOK(IDC_COMBO_ADDRESS,OnNewAddress)
	ON_WM_KEYDOWN()

	ON_COMMAND( TB_BACK, OnBack )
	ON_COMMAND( TB_FORWARD, OnForward )
	ON_COMMAND( TB_STOP, OnStop )
	ON_COMMAND( TB_HOME, OnHomePage )
	ON_COMMAND( TB_REFRESH, OnRefresh )

	ON_MESSAGE(UM_HC_BEFORE_NAVI, OnHcBeforeNavi)
	ON_MESSAGE(UM_HC_NAVI_CMPL, OnHcNaviCmpl)
	ON_MESSAGE(UM_HC_DOC_CMPL, OnDocCmpl)
	ON_MESSAGE(UM_HC_STATUS_TXT_CHANGE, OnStatusTxtChange)
	ON_MESSAGE(UM_HC_TITLE_CHANGE, OnTitleChange)

//	ON_WM_ERASEBKGND()
//ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

// CWebBrowserWnd 消息处理程序
void CWebBrowserWnd::Localize(void)
{
	if (!::IsWindow(m_hWnd))
		return;

	GetDlgItem(IDC_STATIC_BROWSER)->SetWindowText(GetResString(IDS_WEBBROWSER));  
//	GetDlgItem(IDC_STATIC_ADDRESS)->SetWindowText(GetResString(IDS_ADDRESS));
//	m_toolbar.Localize();

	if (::IsWindow(m_staticError.GetSafeHwnd()))
		m_staticError.SetWindowText(GetResString(m_uStridDisableReason));
}

void CWebBrowserWnd::SetAddress(LPCTSTR /*lpszURL*/)
{
//	m_addressbox.SetWindowText(lpszURL);
}

void CWebBrowserWnd::StartAnimation()
{
//	m_animate.Play(0,(UINT)-1,(UINT)-1);
}

void CWebBrowserWnd::StopAnimation()
{
//	m_animate.Stop();
}

void CWebBrowserWnd::OnNewAddress()
{
	CString str;
	//m_addressbox.GetLBText(m_addressbox.GetCurSel(), str);
	m_pExplorer->Navigate2(str, 0, NULL);
}
void CWebBrowserWnd::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if(nChar == '\r')
	{
		OnNewAddressEnter();
	}

	CResizableDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}

//Chocobo Start
//浏览器访问指定页面，added by Chocobo on 2006.08.07
//方便帮助链接在内置浏览器中显示
void CWebBrowserWnd::Navigate(LPCTSTR lpszURL)
{
	if (NULL == m_pExplorer)	return;
	m_pExplorer->Navigate2(lpszURL,0,0,0,0,0);
}
//Chocobo End

void CWebBrowserWnd::OnNewAddressEnter()
{

	//CString str;
	//m_addressbox.GetWindowText(str);
	//
	//m_pExplorer->Navigate2(str, 0, NULL);
	//if(!m_addressbox.FindString(0,str))
	//{
 //       m_addressbox.InsertString(0,str);
	//}
}

CString CWebBrowserWnd::ResourceToURL(LPCTSTR lpszURL)
{
	// This functions shows how to convert an URL to point
	// within the application
	// I only use it once to get the startup page

	CString m_strURL;
	HINSTANCE hInstance = AfxGetResourceHandle();
	ASSERT(hInstance != NULL);
	
	LPTSTR lpszModule = new TCHAR[_MAX_PATH];
	
	if (GetModuleFileName(hInstance, lpszModule, _MAX_PATH))
	{
		m_strURL.Format(_T("res://%s/%s"), lpszModule, lpszURL);
	}
	
	delete []lpszModule;

	return m_strURL;
}

void CWebBrowserWnd::OnBack()
{
	m_pExplorer->GoBack();
}

void CWebBrowserWnd::OnForward()
{
	m_pExplorer->GoForward();
}

void CWebBrowserWnd::OnStop()
{
	m_pExplorer->Stop();
}

void CWebBrowserWnd::OnHomePage()
{
//VC-dgkang 2008年7月8日
#ifdef _FOREIGN_VERSION
	m_pExplorer->Navigate2(thePrefs.m_strHomePage, NULL,NULL,NULL,NULL);
#else
	m_pExplorer->Navigate2(VERYCD_HOMEPAGE, NULL,NULL,NULL,NULL);
#endif 

}

void CWebBrowserWnd::OnRefresh()
{
	StartAnimation();
    m_pExplorer->Refresh();
}

void CWebBrowserWnd::DisableBrowser(UINT uStridReason)
{
	GetDlgItem(IDC_STATIC_BROWSER)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_CUSTOM_BAR)->ShowWindow(SW_HIDE);

	m_uStridDisableReason = uStridReason;

	CRect rect;
	GetClientRect(&rect);
	m_staticError.Create(NULL, WS_CHILD|WS_VISIBLE, rect, this, IDC_RUNTIMEERROR);
	m_staticError.SetWindowText(GetResString(m_uStridDisableReason));
	m_staticError.SetFont(&(theApp.m_fontLog));
}
CString CWebBrowserWnd::GetRealUrl()
{

	if(m_pExplorer)
		return m_pExplorer->GetLocationURL();

	return _T("");
}
BOOL CWebBrowserWnd::OnInitDialog()
{
	CResizableDialog::OnInitDialog();

	// VC-SearchDream[2006-12-25]: Move the following code from the tail of m_animate.Open(IDR_VERYEARTH)
	// Add Browser
	CRect toolRect,frameRect,addressRect,staticRect, statusRect, loaderRect;
	GetDlgItem(IDC_BROWSERFRAME)->GetWindowRect(&frameRect);
	GetDlgItem(IDC_BROWSERFRAME)->DestroyWindow();	
	::MapWindowPoints(NULL, m_hWnd, (LPPOINT)&frameRect, 2);

	//ADDED by fengwen on 2006/12/29	<begin> :
	if (! thePrefs.m_bShowBroswer)
	{
		DisableBrowser(IDS_WEBBROWSER_DISABLED);
		return TRUE;
	}

	//ADDED by fengwen on 2006/12/29	<end> :
	// VC-SearchDream[2006-12-25]: Add try-catch
	try
	{
		m_pExplorer = new CHtmlCtrl(); // VC-SearchDream[2006-12-25]: Move from CWebBrowserWnd
		m_pExplorer->Create(NULL, NULL ,WS_CHILD | WS_VISIBLE &~WS_BORDER,frameRect,this, IDC_BROWSERFRAME,NULL);
		m_pExplorer->SetSilent(true);
	}
	catch(...)
	{
		if (m_pExplorer)
		{
			delete m_pExplorer;
			m_pExplorer = NULL;
		}

		//CString strError;
		//strError.Format(GetResString(IDS_CREATE_WEBBROWSER_FAIL), VERYCD_HOMEPAGE);
		DisableBrowser(IDS_CREATE_WEBBROWSER_FAIL);
		//ShellExecute(this->m_hWnd, _T("open"), CString(VERYCD_HOMEPAGE), _T(""), _T(""), SW_SHOW);

		return TRUE; // Directly Return
	}
	// VC-SearchDream[2006-12-25]: Move the up code from the tail of m_animate.Open(IDR_VERYEARTH)
	theApp.m_BrowserToolbarInfo.SetHtmlCtrl(m_pExplorer);

	// Add Toolbar
//	m_toolbar.Init();

	// Add Title
	GetDlgItem(IDC_STATIC_BROWSER)->SetWindowText(GetResString(IDS_WEBBROWSER));  
//	GetDlgItem(IDC_STATIC_ADDRESS)->SetWindowText(GetResString(IDS_ADDRESS));

	m_browsericon = theApp.LoadIcon(_T("WEBBROWSER"));

	// Add VeryCD Earth
//	m_animate.Open(IDR_VERYEARTH);

	//工具条位置
	//CSize size;
	//m_toolbar.GetMaxSize(&size);
	//m_toolbar.GetWindowRect(&toolRect);
	//ScreenToClient(&toolRect);
	//m_toolbar.MoveWindow(toolRect.left,toolRect.top,size.cx,toolRect.Height());

	// Set Static_Address Postion
//	m_toolbar.GetWindowRect(&toolRect);
	//GetDlgItem(IDC_STATIC_ADDRESS)->GetWindowRect(&staticRect);
	//staticRect.left = toolRect.right;
	//staticRect.right = staticRect.left + 50 ;
	//ScreenToClient(&staticRect);
	//GetDlgItem(IDC_STATIC_ADDRESS)->MoveWindow(&staticRect);

	// Set Address Box Position
	//m_addressbox.GetWindowRect(&addressRect);
	//addressRect.left = toolRect.right + 55;
	//ScreenToClient(&addressRect);
	//m_addressbox.MoveWindow(&addressRect);
//	m_addressbox.SetItemHeight(-1,160);

	// Set Window Style
	UINT uStyle = ::GetWindowLong(m_pExplorer->m_hWnd,GWL_EXSTYLE);
	uStyle |= ~WS_EX_STATICEDGE;
	::SetWindowLong(m_pExplorer->m_hWnd,GWL_EXSTYLE,uStyle);

	CRect	rtClient;
	GetClientRect(rtClient);
	m_pExplorer->MoveWindow(&rtClient, FALSE);

//	AddAnchor(GetDlgItem(IDC_STATIC_ADDRESS)->m_hWnd,TOP_LEFT,NOANCHOR);
//	AddAnchor(m_addressbox.m_hWnd,TOP_LEFT,TOP_RIGHT);
	AddAnchor(m_pExplorer->m_hWnd,TOP_LEFT,BOTTOM_RIGHT);
//	AddAnchor(GetDlgItem(IDC_VERYEARTH)->m_hWnd,TOP_RIGHT,NOANCHOR);
	AddAnchor(m_status.m_hWnd,BOTTOM_LEFT, BOTTOM_RIGHT);

	// Navigate Homepage
	m_pExplorer->SetSilent(true);
	m_pExplorer->Navigate2(m_strOpenUrl, NULL,NULL,NULL,NULL);

	InitControlContainer();
	return TRUE;
}

LRESULT CWebBrowserWnd::OnHcBeforeNavi(WPARAM wParam, LPARAM lParam)
{	
	if (theApp.emuledlg && ::IsWindow(theApp.emuledlg->m_mainTabWnd.m_dlgResource.m_hWnd))
	{
		if ( theApp.emuledlg->m_mainTabWnd.m_dlgResource.CreateNewWbTabSearch(wParam,(LPCTSTR) lParam) )
			m_pExplorer->m_bCancel = TRUE;
	}

/*		
		撤销该代码，verycd.com ED2K连接不调用。
		if (StrStrI((LPCTSTR)lParam,_T("ed2k://|file|")))
		{
			CmdFuncs::AddMultiLinksTask((LPCTSTR)lParam);
			m_pExplorer->m_bCancel = TRUE;
		}
*/

	SetAddress((LPCTSTR) lParam);
	StartAnimation();
	return 0;
}
LRESULT CWebBrowserWnd::OnHcNaviCmpl(WPARAM /*wParam*/, LPARAM lParam)
{
	if (theApp.emuledlg && ::IsWindow(theApp.emuledlg->m_mainTabWnd.m_dlgResource.m_hWnd))
		theApp.emuledlg->m_mainTabWnd.m_dlgResource.UpdateWbsClosableStatus();

	SetAddress((LPCTSTR) lParam);
	return 0;
}
LRESULT CWebBrowserWnd::OnDocCmpl(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	StopAnimation();
	return 0;
}	

LRESULT CWebBrowserWnd::OnStatusTxtChange(WPARAM /*wParam*/, LPARAM lParam)
{
	m_status.SetWindowText((LPCTSTR) lParam);
	return 0;
}

LRESULT CWebBrowserWnd::OnTitleChange(WPARAM /*wParam*/, LPARAM lParam)
{
	//VC-dgkang 不在标签上显示about:blank标识

	if (lParam && StrCmp((LPCTSTR)lParam,_T("about:blank")))
		SetItemCaption((LPCTSTR) lParam);
	return 0;
}

//防止按下回车键，浏览器窗口消失
void CWebBrowserWnd::OnOK()
{
	return;
}

void CWebBrowserWnd::OnCancel()
{
	return;
}
