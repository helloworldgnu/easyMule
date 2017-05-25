/*
 * $Id: DlgMainTabResource.cpp 9073 2008-12-18 04:38:51Z dgkang $
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
// DlgMainTabResource.cpp : 实现文件
//

#include "stdafx.h"
#include "DlgMainTabResource.h"
#include "TabItem_Normal.h"
#include "TabItem_NormalCloseable.h"
#include "emule.h"
#include "emuleDlg.h"
#include "WebBrowserWnd.h"
#include "SearchListCtrl.h"
#include "SearchDlg.h"
#include "SearchResultsWnd.h"
#include "HelpIDs.h"
#include "UserMsgs.h"
#include "CmdFuncs.h"
#include "TabItem_Wnd.h"
#include "PageTabBkDraw.h"
#include "TabItem_WebBrowser.h"
#include "kademlia/kademlia/kademlia.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define  UM_RESTAB_EM_DESTROY UM_END  + 1
#define  MAX_OPENED_TABS 19

SSearchParams* GetParameters(CString expression)
{
	CString strExpression = expression;

	CString strExtension;

	UINT uAvailability = 0;

	UINT uComplete = 0;

	CString strCodec;

	ULONG ulMinBitrate = 0;

	ULONG ulMinLength = 0;
	
	SSearchParams* pParams = new SSearchParams;
	pParams->strExpression = strExpression;
	pParams->eType         = SearchTypeEd2kGlobal;
	pParams->strFileType   = _T("");
	pParams->strMinSize    = _T("");
	pParams->ullMinSize    = 0;
	pParams->strMaxSize    = _T("");
	pParams->ullMaxSize    = 0;
	pParams->uAvailability = uAvailability;
	pParams->strExtension  = strExtension;
	pParams->uComplete     = uComplete;
	pParams->strCodec      = strCodec;
	pParams->ulMinBitrate  = ulMinBitrate;
	pParams->ulMinLength   = ulMinLength;

	return pParams;
}

// CDlgMainTabResource 对话框

IMPLEMENT_DYNAMIC(CDlgMainTabResource, CDialog)
CDlgMainTabResource::CDlgMainTabResource(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CDlgMainTabResource::IDD, pParent)
{
	m_dwCounter = 0;
	m_SearchMap.InitHashTable(1031);
	m_dwTotalCount = 0;
}

CDlgMainTabResource::~CDlgMainTabResource()
{
	// {begin} 09/27/2007 Added by Soar Chin to resolve memory leaking
	POSITION pos = m_SearchMap.GetStartPosition();
	while (pos != NULL) 
	{
		int key;
		SSearchParams * params;
		m_SearchMap.GetNextAssoc(pos, key, params);
		if(params != NULL)
			delete params;
	}
	m_SearchMap.RemoveAll();
	// {end}   09/27/2007 Added by Soar Chin to resolve memory leaking
}

void CDlgMainTabResource::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDlgMainTabResource, CResizableDialog)
	ON_NOTIFY(NMC_TW_ACTIVE_TAB_CHANDED, IDC_RESOURCE_TAB_WND, OnNMActiveTabChanged)
	ON_NOTIFY(NMC_TW_TAB_DESTROY, IDC_RESOURCE_TAB_WND, OnNMTabDestroy)
	ON_NOTIFY(NMC_TW_TAB_CREATE, IDC_RESOURCE_TAB_WND, OnNMTabCreate)
	ON_MESSAGE(UM_RESTAB_WB_DESTROY, OnWbTabDestroy)
	ON_MESSAGE(UM_RESTAB_EM_DESTROY,OnEMTabDestroy)
END_MESSAGE_MAP()


CWebBrowserWnd* CDlgMainTabResource::OpenNewUrl(LPCTSTR lpszUrl, LPCTSTR lpszCaption, BOOL bSetActive, BOOL bClosable)
{
	if (m_dwTotalCount > MAX_OPENED_TABS)
	{
		MessageBox(GetResString(IDS_ERR_MAXTABS),GetResString(IDS_CAPTION),MB_ICONWARNING);
		return NULL;
	}

	CmdFuncs::SetMainActiveTab(CMainTabWnd::TI_RESOURCE);
	CWebBrowserWnd	*pwbw = new CWebBrowserWnd;
	if (pwbw == NULL)
		return NULL;

	if (NULL != lpszUrl)
	{
		pwbw->SetOpenUrl(lpszUrl);
		// VC-dgkang 2008年7月10日
		m_strSearchUrl.AddTail(lpszUrl);
	}
	pwbw->Create(IDD_WEBBROWSER);

	//CmdFuncs::TabWnd_AddCloseTab(&m_tabWnd, lpszCaption, pwbw->GetSafeHwnd(), TRUE, pwbw, bSetActive);
	POSITION			pos;
	CTabItem_WebBrowser	*pWbItem = NULL;

	pWbItem = new CTabItem_WebBrowser;
	pWbItem->SetCaption(lpszCaption);
	pWbItem->SetRelativeWnd(pwbw->GetSafeHwnd(), NULL, TRUE, pwbw);
	pWbItem->SetWbWnd(pwbw);
	pWbItem->SetCustomData(TCD_WEB_BROWSER);
	pWbItem->EnableClose(bClosable);
	pwbw->SetAssocItem(pWbItem);

	POSITION Pos = m_tabWnd.GetActiveTab();
	pos = m_tabWnd.AddTab(pWbItem,TRUE,Pos);

	pWbItem = NULL;
	
	if (bSetActive)
		m_tabWnd.SetActiveTab(pos);

	m_dwTotalCount++;
	return pwbw;
}

//{begin}VC-dgkang 2008年7月11日

BOOL CDlgMainTabResource::CreateNewShareFileTab(SSearchParams *pSS)
{
	if (!pSS)
		return FALSE;

	if (m_dwTotalCount > MAX_OPENED_TABS)
	{
		MessageBox(GetResString(IDS_ERR_MAXTABS),GetResString(IDS_CAPTION),MB_ICONWARNING);
		return FALSE;
	}

	POSITION Pos = m_SearchMap.GetStartPosition();
	SSearchParams * pNextSS;
	int i = 0;
	while(Pos != NULL)
	{
		m_SearchMap.GetNextAssoc(Pos,i,pNextSS);
		if (pNextSS->dwSearchID == pSS->dwSearchID)
		{
			return FALSE;
		}
	}

	CmdFuncs::SetMainActiveTab(CMainTabWnd::TI_RESOURCE);

	DWORD	iCounter = 0;
	DWORD	CustomData = 0xF0F00000;

	CustomData	+= m_dwCounter;

	theApp.emuledlg->searchwnd->m_pwndResults->m_iCurSearchIndexInRes = m_dwCounter;

	iCounter = m_dwCounter;
	m_dwCounter++;

	m_SearchMap.SetAt(iCounter,pSS);

	pSS->strSearchTitle = (pSS->strSpecialTitle.IsEmpty() ? pSS->strExpression : pSS->strSpecialTitle);
	CString	strDisplayText = pSS->strSearchTitle;

	CTabItem_NormalCloseable	*pTi = NULL;
	pTi = new CTabItem_NormalCloseable;
	pTi->SetCaption(strDisplayText);
	pTi->SetRelativeWnd(theApp.emuledlg->searchwnd->m_pwndResults->GetSafeHwnd());
	pTi->SetCustomData(CustomData);
	pTi->SetDesireLength(150);

	Pos = m_tabWnd.GetActiveTab();
	m_tabWnd.AddTab(pTi,TRUE,Pos);

	m_dwTotalCount++;
	return TRUE;
}

void CDlgMainTabResource::OpenNewSearchResult(LPCTSTR lpszCaption,ESearchType * pSearchType/* = NULL */)
{
	if (m_dwTotalCount > MAX_OPENED_TABS)
	{
		MessageBox(GetResString(IDS_ERR_MAXTABS),GetResString(IDS_CAPTION),MB_ICONWARNING);
		return;
	}

	CmdFuncs::SetMainActiveTab(CMainTabWnd::TI_RESOURCE);
	DWORD	iCounter = 0;

	if ( (CGlobalVariable::serverconnect->IsConnected() && !pSearchType) || 
		 (CGlobalVariable::serverconnect->IsConnected() && *pSearchType == SearchTypeEd2kGlobal))
	{
		DWORD CustomData = 0xF0F00000;

		SSearchParams * pSearchParams = GetParameters(lpszCaption);

		CustomData          += m_dwCounter;
		theApp.emuledlg->searchwnd->m_pwndResults->m_iCurSearchIndexInRes = m_dwCounter;
		iCounter = m_dwCounter;
		m_dwCounter++;
		m_SearchMap.SetAt(iCounter, pSearchParams);

		CString	strDisplayText;
		strDisplayText = GetResString(IDS_TABTITLE_SEARCH_RESULT);
		strDisplayText += lpszCaption;


		CTabItem_NormalCloseable	*pTi = NULL;
		pTi = new CTabItem_NormalCloseable;
		pTi->SetCaption(strDisplayText);
		pTi->SetRelativeWnd(theApp.emuledlg->searchwnd->m_pwndResults->GetSafeHwnd());
		pTi->SetCustomData(CustomData);
		pTi->SetDesireLength(150);
		POSITION Pos = m_tabWnd.GetActiveTab();
		m_tabWnd.AddTab(pTi,TRUE,Pos);

		if (!theApp.emuledlg->searchwnd->m_pwndResults->StartSearch(pSearchParams))
			m_SearchMap.RemoveKey(iCounter);

		m_dwTotalCount++;
	}
	else
	{
		if( (Kademlia::CKademlia::IsConnected() && !pSearchType) || 
			(Kademlia::CKademlia::IsConnected() && *pSearchType == SearchTypeKademlia) )
		{
			DWORD CustomData = 0xF0F00000;

			if (m_dwCounter > 0x20)
			{
				return;
			}

			SSearchParams * pSearchParams = GetParameters(lpszCaption);
			pSearchParams->eType = SearchTypeKademlia;

			CustomData          += m_dwCounter;

			theApp.emuledlg->searchwnd->m_pwndResults->m_iCurSearchIndexInRes = m_dwCounter;
			iCounter = m_dwCounter;
			m_dwCounter++;
			m_SearchMap.SetAt(iCounter, pSearchParams);

			CString	strDisplayText;
			strDisplayText = GetResString(IDS_TABTITLE_SEARCH_RESULT);
			strDisplayText += lpszCaption;


			CTabItem_NormalCloseable	*pTi = NULL;
			pTi = new CTabItem_NormalCloseable;
			pTi->SetCaption(strDisplayText);
			pTi->SetRelativeWnd(theApp.emuledlg->searchwnd->m_pwndResults->GetSafeHwnd());
			pTi->SetCustomData(CustomData);
			pTi->SetDesireLength(150);
			POSITION Pos = m_tabWnd.GetActiveTab();
			m_tabWnd.AddTab(pTi,TRUE,Pos);

			/* VC-dgkang 2008年7月17日
			//Kademlia 返回错误，但不Delete  pParams也不RemoveKey从m_SearchMap中，而是在删除标签页一起删除.
			//这是为多标签页操作修改的。
			if (!theApp.emuledlg->searchwnd->m_pwndResults->StartSearch(pSearchParams))
				m_SearchMap.RemoveKey(iCounter);
			*/
			theApp.emuledlg->searchwnd->m_pwndResults->StartSearch(pSearchParams);
			m_dwTotalCount++;
		}
		else
		{
			MessageBox(GetResString(IDS_ERR_NOTCONNECTED),GetResString(IDS_CAPTION),MB_ICONWARNING);
		}
	}
}

void CDlgMainTabResource::ShowVerycdPage()
{
	OpenNewUrl(NULL, GetResString(IDS_VERYCD), TRUE, FALSE);
}


void CDlgMainTabResource::ShowEmuleSearch()
{
	DWORD CustomData = 0xF0F00000;

	CString	strDisplayText;
	strDisplayText = GetResString(IDS_SEARCHEMULE);

	SSearchParams * pSearchParams = GetParameters(_T(""));

	CustomData          += m_dwCounter;

	theApp.emuledlg->searchwnd->m_pwndResults->m_iCurSearchIndexInRes = m_dwCounter;
	int iCounter = m_dwCounter;

	m_dwCounter++;

	m_SearchMap.SetAt(iCounter, pSearchParams);

	CTabItem_NormalCloseable	*pTi = NULL;
	pTi = new CTabItem_NormalCloseable;
	pTi->SetCaption(strDisplayText);
	pTi->SetRelativeWnd(theApp.emuledlg->searchwnd->m_pwndResults->GetSafeHwnd());
	pTi->SetCustomData(CustomData);
	pTi->EnableClose(FALSE);
	pTi->SetDesireLength(150);

	m_tabWnd.AddTab(pTi,TRUE);
	m_dwTotalCount++;
}


// CDlgMainTabResource 消息处理程序

BOOL CDlgMainTabResource::OnInitDialog()
{
	CResizableDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	ModifyStyle(0, WS_CLIPCHILDREN, 0);

	CRect	rect;
	GetDlgItem(IDC_TAB1)->GetWindowRect(&rect);
	ScreenToClient(&rect);
	m_tabWnd.Create(WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN, rect, this, IDC_RESOURCE_TAB_WND);
	//m_tabWnd.SetBkColor(GetSysColor(COLOR_3DFACE), FALSE);
	//m_tabWnd.SetJointColor(RGB(121, 138, 169), RGB(255, 255, 255));
	CPageTabBkDraw	*pBarBkDraw = new CPageTabBkDraw;
	m_tabWnd.SetBarBkDraw(pBarBkDraw);


	//	add WebBrowser Toolbar	<begin>

	m_browserToolbar.Create(WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, IDC_CUSTOM_BAR);
	m_browserToolbar.Init();
	//m_browserToolbar.SetOwner(theApp.emuledlg->webbrowser);
	m_browserToolbar.SetIndent(8);

	theApp.m_BrowserToolbarInfo.SetBrowserToolbarCtrl(&m_browserToolbar);


	//CSize	size;
	//m_browserToolbar.GetMaxSize(&size);
	
	CTabItem_Wnd	*pTabItemWnd = new CTabItem_Wnd;
	pTabItemWnd->SetItemWnd(&m_browserToolbar, FALSE);
	//	pTabItemWnd->SetWindowLength(size.cx + 8);
	pTabItemWnd->SetDynDesireLength(TRUE);
	m_tabWnd.AddTab(pTabItemWnd);
	pTabItemWnd = NULL;

	//	add WebBrowser Toolbar	<end>


	// {begin} VC-dgkang 2008-7-10
	if(thePrefs.m_bStartShowHomePage)
		ShowVerycdPage();

	ShowEmuleSearch();

	//Set Active Tab for verycd.com
	POSITION pos =m_tabWnd.GetFirstTab();
	if (pos)
	{
		m_tabWnd.GetNextTab(pos);
		if (pos)
			m_tabWnd.SetActiveTab(pos);
	}
	//{end}

	AddAnchor(m_tabWnd,TOP_LEFT,BOTTOM_RIGHT);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

BOOL CDlgMainTabResource::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (pMsg->message == WM_KEYDOWN)
	{
		// Don't handle Ctrl+Tab in this window. It will be handled by main window.
		if (pMsg->wParam == VK_TAB && GetAsyncKeyState(VK_CONTROL) < 0)
			return FALSE;

		if (VK_RETURN == pMsg->wParam || VK_ESCAPE == pMsg->wParam)
			return FALSE;
	}

	return CResizableDialog::PreTranslateMessage(pMsg);
}

void CDlgMainTabResource::OnNMActiveTabChanged(NMHDR* pNMHDR, LRESULT *pResult)
{
	NMTW_TABOP *pTabOp = reinterpret_cast<NMTW_TABOP*>(pNMHDR);

	//pTabOp->posOld;
	//pTabOp->posTab;
	DWORD CustomData = m_tabWnd.GetTabData(pTabOp->posTab);

	DWORD LowData  = CustomData & 0x0000FFFF;
	DWORD HighData = CustomData & 0xFFFF0000;

	if (HighData ^ 0x0F0FFFFF)
	{
		SSearchParams *pSearchParams=NULL;
		BOOL bSearch = m_SearchMap.Lookup(LowData, pSearchParams);

		if ( bSearch && pSearchParams )
		{
			theApp.emuledlg->searchwnd->m_pwndResults->searchlistctrl.ShowResults(pSearchParams->dwSearchID);
			theApp.emuledlg->searchwnd->m_pwndResults->UpdateParamDisplay(pSearchParams);
			theApp.emuledlg->searchwnd->m_pwndResults->m_iCurSearchIndexInRes = LowData;
		}
	}

	
	if (TCD_WEB_BROWSER == CustomData)
	{
		//m_browserToolbar.EnableAllButtons(TRUE);

		CTabItem_WebBrowser *pItem = (CTabItem_WebBrowser*) m_tabWnd.GetTabItem(pTabOp->posTab);
		if (NULL != pItem)
			m_browserToolbar.SetOwner(pItem->GetAssocWbw());
		BOOL bForwordEnable = pItem->GetAssocWbw()->m_pExplorer->bForwordEnable;
		BOOL bBackEnable = pItem->GetAssocWbw()->m_pExplorer->bBackEnable;
		theApp.m_BrowserToolbarInfo.GetBrowserToolbarCtrl()->EnableButton(TB_FORWARD,bForwordEnable);
		theApp.m_BrowserToolbarInfo.GetBrowserToolbarCtrl()->EnableButton(TB_BACK, bBackEnable); 
		theApp.m_BrowserToolbarInfo.GetBrowserToolbarCtrl()->EnableButton(TB_REFRESH, TRUE);
        theApp.m_BrowserToolbarInfo.GetBrowserToolbarCtrl()->EnableButton(TB_HOME, TRUE);
	}
	else
		m_browserToolbar.EnableAllButtons(FALSE);

	*pResult = 0;
}

void CDlgMainTabResource::OnNMTabDestroy(NMHDR* pNMHDR, LRESULT *pResult)
{
	theApp.emuledlg->searchwnd->m_pwndResults->SetPos(0);
	NMTW_TABOP *pTabOp = reinterpret_cast<NMTW_TABOP*>(pNMHDR);

	//pTabOp->posTab;

	DWORD CustomData = m_tabWnd.GetTabData(pTabOp->posTab);

	DWORD LowData  = CustomData & 0x0000FFFF;
	DWORD HighData = CustomData & 0xFFFF0000;

	if (HighData ^ 0x0F0FFFFF)
	{
		SSearchParams *pSearchParams=NULL;
		BOOL bSearch = m_SearchMap.Lookup(LowData, pSearchParams);

		if ( bSearch && pSearchParams)
		{
			theApp.emuledlg->searchwnd->m_pwndResults->searchlistctrl.RemoveResults(pSearchParams->dwSearchID);
			m_SearchMap.RemoveKey(LowData);
		}

		delete pSearchParams;
	}

	if (TCD_WEB_BROWSER == CustomData)
	{
		PostMessage(UM_RESTAB_WB_DESTROY);
	}
	else if (CustomData >= 0xF0F00000)
	{
		PostMessage(UM_RESTAB_EM_DESTROY);
	}

	if (m_dwTotalCount > 0)
		m_dwTotalCount--;

	*pResult = 0;
}

void CDlgMainTabResource::OnNMTabCreate(NMHDR* pNMHDR, LRESULT *pResult)
{
	NMTW_TABOP *pTabOp = reinterpret_cast<NMTW_TABOP*>(pNMHDR);

	DWORD dwCustomData = m_tabWnd.GetTabData(pTabOp->posTab);
	if (TCD_WEB_BROWSER == dwCustomData)
		UpdateWbsClosableStatus();

	else if(dwCustomData >= 0xF0F00000)
		UpdateEMsClosableStatus();

	*pResult = 0;
}

//{begin} VC-dgkang 2008年7月8日
CString GetNoParamUrl(LPCTSTR lpszText)
{
	CString tcs = lpszText;
	int n = tcs.Find(_T("#"));

	if (n == -1)
		n = tcs.Find(_T("?"));

	if (n != -1)
		tcs = tcs.Left(n);
	return tcs;
}

BOOL CDlgMainTabResource::CreateNewWbTabSearch(WPARAM wParam, LPCTSTR lpszText)
{
	CString tcs = lpszText;
	BOOL bValid = FALSE,bHome = FALSE;
	CTabItem_WebBrowser *pWbItem = NULL;
	POSITION Pos = m_tabWnd.GetActiveTab();
	if(Pos)
	{
		CTabItem *pItem = m_tabWnd.GetTabItem(Pos);
		if (pItem->m_dwCustomData == TCD_WEB_BROWSER)
			pWbItem = (CTabItem_WebBrowser *)pItem;
	}

	if (tcs.Find(_T("#")) != -1 || tcs.Find(_T("?")) != -1)
	{
		if (pWbItem)
		{
			if (GetNoParamUrl(lpszText) == GetNoParamUrl(pWbItem->GetRealUrl()))
				return FALSE;
		}	
	}

	if (wParam == 1)
	{
		OpenNewUrl(lpszText,NULL,TRUE,FALSE); 
		return TRUE; //这个返回无任何意义
	}

#ifdef _FOREIGN_VERSION
	bHome = tcs.CompareNoCase(thePrefs.m_strHomePage) == 0;
	if (!bHome)
	{
		bValid = tcs.Find(thePrefs.m_strSearchPage) != -1;
	}
#else
	bHome = tcs.CompareNoCase(_T("http://www.verycd.com/start/")) == 0;
	if (!bHome)
	{
		bValid = tcs.Find(_T("http://www.verycd.com/search/folders/")) != -1;
	}
#endif


	if (!bHome && bValid)
	{

#ifdef _FOREIGN_VERSION
		if (pWbItem && !pWbItem->GetRealUrl().CompareNoCase(thePrefs.m_strHomePage))
			return FALSE;
#else
		if (pWbItem && !pWbItem->GetRealUrl().CompareNoCase(_T("http://www.verycd.com/start/")))
			return FALSE;
#endif
		//m_strSerchUrl 只是简单作为一个状态变量来使用。
		if (m_strSearchUrl.IsEmpty())
		{
			OpenNewUrl(lpszText,NULL,TRUE,FALSE); 
			return TRUE; //将不在本标签页显示.
		}
		else
		{
			m_strSearchUrl.RemoveAll();
			return FALSE;
		}
	}
	return FALSE;
}

//{begin} VC-dgkang  2008年7月17日
void CDlgMainTabResource::Localize()
{
	const CString tcsEN = _T("easyMule Search"),tcsTW = _T("搜索電驢網路"), tcsCN = _T("搜索电驴网络");
	const CString szEN = _T("Search:"),szTW = _T("搜索:"), szCN = _T("搜索:");

	CString tcs;
	CTabItem_NormalCloseable * pEMItem  = NULL;
	int nLen = -1;

	POSITION	pos = m_tabWnd.GetFirstTab();
	CTabItem	*pItem = NULL;
	while (NULL != pos)
	{
		pItem = m_tabWnd.GetNextTab(pos);
		if (NULL != pItem)
		{
			if (pItem->m_dwCustomData >= 0xF0F00000)
			{
				pEMItem = (CTabItem_NormalCloseable *)pItem;
				CString tcs = pEMItem->GetCaption();
				if (tcs == tcsEN || tcs == tcsTW || tcs == tcsCN)
					pEMItem->SetCaption(GetResString(IDS_SEARCHEMULE));
				else
				{
					if (tcs.Find(szEN) == 0) nLen = szEN.GetLength();
					else if(tcs.Find(szTW) == 0) nLen = szTW.GetLength();
					else if(tcs.Find(szCN) == 0) nLen = szCN.GetLength();
					else nLen = -1;
					if (nLen != -1)
					{
						tcs.Replace(tcs.Left(nLen),GetResString(IDS_TABTITLE_SEARCH_RESULT));
						pEMItem->SetCaption(tcs);
					}

				}
			}
		}
	}
}

//{end}
void CDlgMainTabResource::UpdateEMsClosableStatus()
{
	int     iEMCount = 0;

	CTabItem_NormalCloseable * pEMItem  = NULL;

	POSITION	pos =m_tabWnd.GetFirstTab();
	CTabItem	*pItem = NULL;

	while (NULL != pos)
	{
		pItem = m_tabWnd.GetNextTab(pos);
		if (NULL != pItem)
		{
			if (pItem->m_dwCustomData >= 0xF0F00000)
			{
				pEMItem = (CTabItem_NormalCloseable *)pItem;
				pEMItem->EnableClose(TRUE);
				iEMCount ++;
			}
		}
	}

	if (1 == iEMCount && NULL != pEMItem && 
		pEMItem->GetCaption() == GetResString(IDS_SEARCHEMULE))
	{
		pEMItem->EnableClose(FALSE);
	}

	if ( 0 == iEMCount)
		ShowEmuleSearch();
}
//{end}
void CDlgMainTabResource::UpdateWbsClosableStatus()
{
	int		iWbCount = 0;

	CTabItem_WebBrowser *pWbItem		= NULL;

	POSITION	pos =m_tabWnd.GetFirstTab();
	CTabItem	*pItem;
	while (NULL != pos)
	{
		pItem = m_tabWnd.GetNextTab(pos);
		if (NULL != pItem)
		{
			if (TCD_WEB_BROWSER == pItem->m_dwCustomData)
			{
				pWbItem = (CTabItem_WebBrowser*)pItem;
				pWbItem->EnableClose(TRUE);
				iWbCount++;
			}
		}
	}

	// VC-Dgkang 2008年7月7日	
	if (1 == iWbCount && NULL != pWbItem &&
#ifdef  _FOREIGN_VERSION
		thePrefs.m_strHomePage == pWbItem->GetRealUrl())
#else
		_T("http://www.verycd.com/start/")== pWbItem->GetRealUrl())
#endif
	{
		pWbItem->EnableClose(FALSE);
	}

	if (0 == iWbCount)
		ShowVerycdPage();

	//{end}
}

LRESULT CDlgMainTabResource::OnWbTabDestroy(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	UpdateWbsClosableStatus();	
	return 0;
}

LRESULT CDlgMainTabResource::OnEMTabDestroy(WPARAM /*wParam */, LPARAM /*lParam */)
{
	UpdateEMsClosableStatus();
	return 0;
}
void CDlgMainTabResource::UpdateSearchParam(int iIndex, SSearchParams *pSearchParams)
{
	SSearchParams	*psp;
	if (m_SearchMap.Lookup(iIndex, psp))
	{
		delete psp;
		m_SearchMap.SetAt(iIndex, pSearchParams);
	}
}
