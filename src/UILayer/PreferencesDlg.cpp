/* 
 * $Id: PreferencesDlg.cpp 5178 2008-03-28 09:41:48Z soarchin $
 * 
 * this file is part of eMule
 * Copyright (C)2002-2006 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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

#include "stdafx.h"
#include "emule.h"
#include "PreferencesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNAMIC(CPreferencesDlg, CTreePropSheet)

BEGIN_MESSAGE_MAP(CPreferencesDlg, CTreePropSheet)
	ON_WM_DESTROY()
	ON_WM_HELPINFO()
	ON_BN_CLICKED(s_unPageAdvId,OnPageAdvanceChanged)// VC-kernel[2007-02-09]:
	ON_MESSAGE(PSM_SETCURSEL, OnSetCurSel)
END_MESSAGE_MAP()

CPreferencesDlg::CPreferencesDlg()
{
	m_psh.dwFlags &= ~PSH_HASHELP;
	m_wndGeneral.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndDisplay.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndConnection.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndDirectories.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndFiles.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndDownloadSecurity.m_psp.dwFlags &= ~PSH_HASHELP;   //Changed by thilon on 2008.03.21
	// VC-kernel[2007-02-05]:to be delete
	//m_wndStats.m_psp.dwFlags &= ~PSH_HASHELP;
	//m_wndIRC.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndTweaks.m_psp.dwFlags &= ~PSH_HASHELP;
	//m_wndSecurity.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndServer.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndScheduler.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndWebServer.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndProxy.m_psp.dwFlags &= ~PSH_HASHELP;
	//m_wndMessages.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndAntiLeech.m_psp.dwFlags &= ~PSH_HASHELP;
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
	m_wndDebug.m_psp.dwFlags &= ~PSH_HASHELP;
#endif

	CTreePropSheet::SetPageIcon(&m_wndGeneral, _T("Preferences"));
	CTreePropSheet::SetPageIcon(&m_wndDisplay, _T("DISPLAY"));
	CTreePropSheet::SetPageIcon(&m_wndConnection, _T("CONNECTION"));
	CTreePropSheet::SetPageIcon(&m_wndDirectories, _T("FOLDERS"));
	CTreePropSheet::SetPageIcon(&m_wndFiles, _T("FILES"));
	CTreePropSheet::SetPageIcon(&m_wndDownloadSecurity, _T("SHIELD"));	//Changed by thilon on 2008.03.21
	// VC-kernel[2007-02-05]:to be delete
	CTreePropSheet::SetPageIcon(&m_wndProxy, _T("PROXY"));
	CTreePropSheet::SetPageIcon(&m_wndServer, _T("SERVER"));
	//CTreePropSheet::SetPageIcon(&m_wndNotify, _T("NOTIFICATIONS"));
	//CTreePropSheet::SetPageIcon(&m_wndStats, _T("STATISTICS"));
	//CTreePropSheet::SetPageIcon(&m_wndIRC, _T("IRC"));
	//CTreePropSheet::SetPageIcon(&m_wndSecurity, _T("SECURITY"));
	CTreePropSheet::SetPageIcon(&m_wndScheduler, _T("SCHEDULER"));
	CTreePropSheet::SetPageIcon(&m_wndWebServer, _T("WEB"));
	CTreePropSheet::SetPageIcon(&m_wndTweaks, _T("TWEAK"));
	//CTreePropSheet::SetPageIcon(&m_wndMessages, _T("MESSAGES"));
	CTreePropSheet::SetPageIcon(&m_wndAntiLeech, _T("LEECHER"));
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
	CTreePropSheet::SetPageIcon(&m_wndDebug, _T("Preferences"));
#endif

	AddPage(&m_wndGeneral);
	AddPage(&m_wndDisplay);
	AddPage(&m_wndConnection);
	AddPage(&m_wndDirectories);
	AddPage(&m_wndFiles);
	AddPage(&m_wndDownloadSecurity);				//Changed by thilon on 2008.03.21
	AddPage(&m_wndServer);
	AddPage(&m_wndScheduler);
	AddPage(&m_wndWebServer);
	// VC-kernel[2007-02-05]:to be delete
	AddPage(&m_wndProxy);
	//AddPage(&m_wndNotify);
	//AddPage(&m_wndStats);
	//AddPage(&m_wndIRC);
	//AddPage(&m_wndMessages);
	//AddPage(&m_wndSecurity);
	AddPage(&m_wndTweaks);
	AddPage(&m_wndAntiLeech);

#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
	AddPage(&m_wndDebug);
#endif

	SetTreeViewMode(TRUE, TRUE, TRUE);
	SetTreeWidth(170);

	m_pPshStartPage = NULL;
	m_bSaveIniFile = false;

	m_bTipAdded = FALSE;
}

CPreferencesDlg::~CPreferencesDlg()
{
}

void CPreferencesDlg::OnDestroy()
{
	CTreePropSheet::OnDestroy();
	if (m_bSaveIniFile)
	{
		thePrefs.Save();
		m_bSaveIniFile = false;
	}
	m_pPshStartPage = GetPage(GetActiveIndex())->m_psp.pszTemplate;
}

BOOL CPreferencesDlg::OnInitDialog()
{
	ASSERT( !m_bSaveIniFile );
	BOOL bResult = CTreePropSheet::OnInitDialog();
	InitWindowStyles(this);

	for (int i = 0; i < m_pages.GetSize(); i++)
	{
		if (GetPage(i)->m_psp.pszTemplate == m_pPshStartPage)
		{
			if(i<5)
				SetActivePage(i);
			else
				SetActivePage(0);
			break;
		}
	}

	m_ttc.Create(this);

	Localize();	
	OnPageAdvanceChanged();
	return bResult;
}

void CPreferencesDlg::Localize()
{
	SetTitle(RemoveAmbersand(GetResString(IDS_EM_PREFS)));

	// VC-kernel[2007-03-02]:
	GetDlgItem(s_unPageAdvId)->SetWindowText(GetResString(IDS_MORE_CONFIG));

	m_wndGeneral.Localize();
	m_wndDisplay.Localize();
	m_wndConnection.Localize();
	m_wndDirectories.Localize();
	m_wndFiles.Localize();
	m_wndDownloadSecurity.Localize();			//Changed by thilon on 2008.03.21
	// VC-kernel[2007-02-05]:to be deleted start
	m_wndServer.Localize();
	//m_wndStats.Localize();
	//m_wndNotify.Localize();
	//m_wndIRC.Localize();
	//m_wndSecurity.Localize();	
	m_wndWebServer.Localize();
	m_wndScheduler.Localize();
	m_wndProxy.Localize();
	//m_wndMessages.Localize();
	// VC-kernel[2007-02-05]:to be deleted end
	m_wndTweaks.Localize();
	m_wndAntiLeech.Localize();

	int c = 0;

	CTreeCtrl* pTree = GetPageTreeControl();
	if (pTree)
	{
		pTree->SetItemText(GetPageTreeItem(c++), RemoveAmbersand(GetResString(IDS_PW_GENERAL)));
		pTree->SetItemText(GetPageTreeItem(c++), RemoveAmbersand(GetResString(IDS_PW_DISPLAY))); 
		pTree->SetItemText(GetPageTreeItem(c++), RemoveAmbersand(GetResString(IDS_PW_CONNECTION))); 
		pTree->SetItemText(GetPageTreeItem(c++), RemoveAmbersand(GetResString(IDS_PW_DIR))); 
		pTree->SetItemText(GetPageTreeItem(c++), RemoveAmbersand(GetResString(IDS_PW_FILES))); 
		pTree->SetItemText(GetPageTreeItem(c++), RemoveAmbersand(GetResString(IDS_DOWNLOADSECURITY)));		//Changed by thilon on 2008.03.21
		//pTree->SetItemText(GetPageTreeItem(c++), RemoveAmbersand(GetResString(IDS_PW_EKDEV_OPTIONS))); 
		//pTree->SetItemText(GetPageTreeItem(c++), RemoveAmbersand(GetResString(IDS_STATSSETUPINFO))); 
		//pTree->SetItemText(GetPageTreeItem(c++), RemoveAmbersand(GetResString(IDS_IRC)));
		//pTree->SetItemText(GetPageTreeItem(c++), RemoveAmbersand(GetResString(IDS_MESSAGESCOMMENTS)));
		//pTree->SetItemText(GetPageTreeItem(c++), RemoveAmbersand(GetResString(IDS_SECURITY))); 
		pTree->SetItemText(GetPageTreeItem(c++), RemoveAmbersand(GetResString(IDS_PW_SERVER)));
		pTree->SetItemText(GetPageTreeItem(c++), RemoveAmbersand(GetResString(IDS_SCHEDULER)));
		pTree->SetItemText(GetPageTreeItem(c++), RemoveAmbersand(GetResString(IDS_PW_WS)));
		pTree->SetItemText(GetPageTreeItem(c++), RemoveAmbersand(GetResString(IDS_PW_PROXY)));
		pTree->SetItemText(GetPageTreeItem(c++), RemoveAmbersand(GetResString(IDS_PW_TWEAK)));
		pTree->SetItemText(GetPageTreeItem(c++), RemoveAmbersand(GetResString(IDS_ANTILEECH)));
	#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
		pTree->SetItemText(GetPageTreeItem(c++), _T("Debug"));
	#endif

		if (m_bTipAdded)
		{
			m_ttc.UpdateTipText(GetResString(IDS_TIP_SCHEDULER), pTree, 7);
			m_ttc.UpdateTipText(GetResString(IDS_TIP_WEB_INTERFACE), pTree, 8);

		}
	}

	UpdateCaption();
}

void CPreferencesDlg::OnHelp()
{
	int iCurSel = GetActiveIndex();
	if (iCurSel >= 0)
	{
		CPropertyPage* pPage = GetPage(iCurSel);
		if (pPage)
		{
			HELPINFO hi = {0};
			hi.cbSize = sizeof hi;
			hi.iContextType = HELPINFO_WINDOW;
			hi.iCtrlId = 0;
			hi.hItemHandle = pPage->m_hWnd;
			hi.dwContextId = 0;
			pPage->SendMessage(WM_HELP, 0, (LPARAM)&hi);
			return;
		}
	}

	theApp.ShowHelp(0, HELP_CONTENTS);
}

BOOL CPreferencesDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_HELP)
	{
		OnHelp();
		return TRUE;
	}
	if (wParam == IDOK || wParam == ID_APPLY_NOW)
	{
		m_bSaveIniFile = true;
		if(IsDlgButtonChecked(s_unPageAdvId))
			thePrefs.m_moreOptions = 1;
		else
			thePrefs.m_moreOptions = 0;
	}
	return __super::OnCommand(wParam, lParam);
}

BOOL CPreferencesDlg::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	OnHelp();
	return TRUE;
}

void CPreferencesDlg::SetStartPage(UINT uStartPageID)
{
	m_pPshStartPage = MAKEINTRESOURCE(uStartPageID);
}

// VC-kernel[2007-02-09]:
void CPreferencesDlg::OnPageAdvanceChanged()
{
	if(IsDlgButtonChecked(s_unPageAdvId))
	{
		//m_iDispPageCount = 5;
		//CreatePageTreeItem(_T("Server"));
		ShowTreeItem(6);
		ShowTreeItem(7);
		ShowTreeItem(8);
		ShowTreeItem(9);
		ShowTreeItem(10);
		ShowTreeItem(11);
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
		ShowTreeItem(12);
#endif

		if (!m_bTipAdded)
		{
			AddTreeItemTip(7, GetResString(IDS_TIP_SCHEDULER), 7);
			AddTreeItemTip(8, GetResString(IDS_TIP_WEB_INTERFACE), 8);
			m_bTipAdded = TRUE;
		}

		Localize();
	}
	else
	{
		HTREEITEM pSelectitem;
		pSelectitem = GetPageTreeControl()->GetSelectedItem();
		if(pSelectitem)
			if(GetPageTreeControl()->GetItemData(pSelectitem) > DEFAULT_PAGE_COUNT - 1)
				GetPageTreeControl()->SelectItem(GetPageTreeItem(DEFAULT_PAGE_COUNT - 1));

		if(GetPageTreeControl()->GetCount() > DEFAULT_PAGE_COUNT)
		{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
			HideTreeItem(12);
#endif
			HideTreeItem(11);
			HideTreeItem(10);
			HideTreeItem(9);
			HideTreeItem(8);
			HideTreeItem(7);
			HideTreeItem(6);
		}

		if (m_bTipAdded)
		{
			m_ttc.DelTool(GetPageTreeControl(), 7);
			m_ttc.DelTool(GetPageTreeControl(), 8);
			m_bTipAdded = FALSE;
		}

	}

	if(m_wndGeneral)
	{
		m_wndGeneral.GetDlgItem(IDC_MISC_FRM)->ShowWindow(IsDlgButtonChecked(s_unPageAdvId)?SW_SHOW:SW_HIDE);
		m_wndGeneral.GetDlgItem(IDC_BEEPER)->ShowWindow(IsDlgButtonChecked(s_unPageAdvId)?SW_SHOW:SW_HIDE);
		/*m_wndGeneral.GetDlgItem(IDC_EXIT)->ShowWindow(IsDlgButtonChecked(s_unPageAdvId)?SW_SHOW:SW_HIDE);*/
		
		//COMMENTED by fengwen on 2007/06/05 <begin>	不显示WebBrowser选项
		//m_wndGeneral.GetDlgItem(IDC_WEBBROWSER)->ShowWindow(IsDlgButtonChecked(s_unPageAdvId)?SW_SHOW:SW_HIDE);
		//COMMENTED by fengwen on 2007/06/05 <end>	不显示WebBrowser选项
		//m_wndGeneral.GetDlgItem(IDC_USEPROXY)->ShowWindow(IsDlgButtonChecked(s_unPageAdvId)?SW_SHOW:SW_HIDE);
		m_wndGeneral.GetDlgItem(IDC_BRINGTOFOREGROUND)->ShowWindow(IsDlgButtonChecked(s_unPageAdvId)?SW_SHOW:SW_HIDE);
	}

	if(m_wndDirectories)
	{
		m_wndDirectories.GetDlgItem(IDC_SHARED_FRM)->ShowWindow(IsDlgButtonChecked(s_unPageAdvId)?SW_SHOW:SW_HIDE);
		m_wndDirectories.GetDlgItem(IDC_SHARESELECTOR)->ShowWindow(IsDlgButtonChecked(s_unPageAdvId)?SW_SHOW:SW_HIDE);
		m_wndDirectories.GetDlgItem(IDC_UNCLIST)->ShowWindow(IsDlgButtonChecked(s_unPageAdvId)?SW_SHOW:SW_HIDE);
		m_wndDirectories.GetDlgItem(IDC_UNCADD)->ShowWindow(IsDlgButtonChecked(s_unPageAdvId)?SW_SHOW:SW_HIDE);
		m_wndDirectories.GetDlgItem(IDC_UNCREM)->ShowWindow(IsDlgButtonChecked(s_unPageAdvId)?SW_SHOW:SW_HIDE);
	}

	if(m_wndFiles)
	{
		m_wndFiles.GetDlgItem(IDC_STATICVIDEOPLAYER)->ShowWindow(IsDlgButtonChecked(s_unPageAdvId)?SW_SHOW:SW_HIDE);
		m_wndFiles.GetDlgItem(IDC_VIDEOPLAYER_CMD_LBL)->ShowWindow(IsDlgButtonChecked(s_unPageAdvId)?SW_SHOW:SW_HIDE);
		m_wndFiles.GetDlgItem(IDC_VIDEOPLAYER)->ShowWindow(IsDlgButtonChecked(s_unPageAdvId)?SW_SHOW:SW_HIDE);
		m_wndFiles.GetDlgItem(IDC_BROWSEV)->ShowWindow(IsDlgButtonChecked(s_unPageAdvId)?SW_SHOW:SW_HIDE);
		m_wndFiles.GetDlgItem(IDC_VIDEOPLAYER_ARGS_LBL)->ShowWindow(IsDlgButtonChecked(s_unPageAdvId)?SW_SHOW:SW_HIDE);
		m_wndFiles.GetDlgItem(IDC_VIDEOPLAYER_ARGS)->ShowWindow(IsDlgButtonChecked(s_unPageAdvId)?SW_SHOW:SW_HIDE);
		m_wndFiles.GetDlgItem(IDC_VIDEOBACKUP)->ShowWindow(IsDlgButtonChecked(s_unPageAdvId)?SW_SHOW:SW_HIDE);
	}

	if(m_wndConnection)
	{
		m_wndConnection.GetDlgItem(IDC_CAPACITIES_FRM)->ShowWindow(IsDlgButtonChecked(s_unPageAdvId)?SW_SHOW:SW_HIDE);
		m_wndConnection.GetDlgItem(IDC_DCAP_LBL)->ShowWindow(IsDlgButtonChecked(s_unPageAdvId)?SW_SHOW:SW_HIDE);
		m_wndConnection.GetDlgItem(IDC_DOWNLOAD_CAP)->ShowWindow(IsDlgButtonChecked(s_unPageAdvId)?SW_SHOW:SW_HIDE);
		m_wndConnection.GetDlgItem(IDC_KBS2)->ShowWindow(IsDlgButtonChecked(s_unPageAdvId)?SW_SHOW:SW_HIDE);
		m_wndConnection.GetDlgItem(IDC_UCAP_LBL)->ShowWindow(IsDlgButtonChecked(s_unPageAdvId)?SW_SHOW:SW_HIDE);
		m_wndConnection.GetDlgItem(IDC_UPLOAD_CAP)->ShowWindow(IsDlgButtonChecked(s_unPageAdvId)?SW_SHOW:SW_HIDE);
		m_wndConnection.GetDlgItem(IDC_KBS3)->ShowWindow(IsDlgButtonChecked(s_unPageAdvId)?SW_SHOW:SW_HIDE);
		m_wndConnection.GetDlgItem(IDC_CONNECTIONTYPE)->ShowWindow(IsDlgButtonChecked(s_unPageAdvId)?SW_SHOW:SW_HIDE);

		m_wndConnection.GetDlgItem(IDC_MISC)->ShowWindow(IsDlgButtonChecked(s_unPageAdvId)?SW_SHOW:SW_HIDE);
		m_wndConnection.GetDlgItem(IDC_MAXSRCHARD_LBL)->ShowWindow(IsDlgButtonChecked(s_unPageAdvId)?SW_SHOW:SW_HIDE);
		m_wndConnection.GetDlgItem(IDC_MAXCONLABEL)->ShowWindow(IsDlgButtonChecked(s_unPageAdvId)?SW_SHOW:SW_HIDE);
		m_wndConnection.GetDlgItem(IDC_MAXHALFCONLABEL)->ShowWindow(IsDlgButtonChecked(s_unPageAdvId)?SW_SHOW:SW_HIDE);
		m_wndConnection.GetDlgItem(IDC_MAXSOURCEPERFILE)->ShowWindow(IsDlgButtonChecked(s_unPageAdvId)?SW_SHOW:SW_HIDE);
		m_wndConnection.GetDlgItem(IDC_MAXCON)->ShowWindow(IsDlgButtonChecked(s_unPageAdvId)?SW_SHOW:SW_HIDE);
		m_wndConnection.GetDlgItem(IDC_MAXHALFCON)->ShowWindow(IsDlgButtonChecked(s_unPageAdvId)?SW_SHOW:SW_HIDE);
		m_wndConnection.GetDlgItem(IDC_MAXSOURCEPERFILESPIN)->ShowWindow(IsDlgButtonChecked(s_unPageAdvId)?SW_SHOW:SW_HIDE);
		m_wndConnection.GetDlgItem(IDC_MAXCONSPIN)->ShowWindow(IsDlgButtonChecked(s_unPageAdvId)?SW_SHOW:SW_HIDE);
		m_wndConnection.GetDlgItem(IDC_MAXHALFCONSPIN)->ShowWindow(IsDlgButtonChecked(s_unPageAdvId)?SW_SHOW:SW_HIDE);

		m_wndConnection.GetDlgItem(IDC_MAXCONN_FRM)->ShowWindow(IsDlgButtonChecked(s_unPageAdvId)?SW_SHOW:SW_HIDE);
		m_wndConnection.GetDlgItem(IDC_UPNP)->ShowWindow(IsDlgButtonChecked(s_unPageAdvId)?SW_SHOW:SW_HIDE);
		m_wndConnection.GetDlgItem(IDC_UPNPRPORT)->ShowWindow(IsDlgButtonChecked(s_unPageAdvId)?SW_SHOW:SW_HIDE);
	}

	return;
}

LRESULT CPreferencesDlg::OnSetCurSel(WPARAM wParam, LPARAM lParam)
{
	//OnPageAdvanceChanged();// VC-kernel[2007-03-01]:
	return CTreePropSheet::OnSetCurSel(wParam,lParam);
}

BOOL CPreferencesDlg::PreTranslateMessage(MSG* pMsg)
{
	m_ttc.RelayEvent(pMsg);
	return CTreePropSheet::PreTranslateMessage(pMsg);
}

void CPreferencesDlg::AddTreeItemTip(int iPage, LPCTSTR lpszTipText, UINT uIdTool)
{
	CTreeCtrl	*ptc = GetPageTreeControl();
	if (NULL == ptc)
		return;

	HTREEITEM hti;
	CRect	rtItem;
	hti = GetPageTreeItem(iPage);
	ptc->GetItemRect(hti, &rtItem, FALSE);
	m_ttc.AddTool(ptc, lpszTipText, &rtItem, uIdTool);
}
