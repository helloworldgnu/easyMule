/* 
 * $Id: PPgGeneral.cpp 7701 2008-10-15 07:34:41Z huby $
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
#include "langids.h"
#include "emule.h"
#include "SearchDlg.h"
#include "PreferencesDlg.h"
#include "ppggeneral.h"
#include "HttpDownloadDlg.h"
#include "Preferences.h"
#include "emuledlg.h"
#include "StatisticsDlg.h"
#include "ServerWnd.h"
#include "TransferWnd.h"
#include "ChatWnd.h"
#include "SharedFilesWnd.h"
#include "KademliaWnd.h"
#include "IrcWnd.h"
#include "WebServices.h"
#include "HelpIDs.h"
#include "StringConversion.h"
#include "Log.h"
#include "IEMonitor.h"
#include "PPgGeneral.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNAMIC(CPPgGeneral, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgGeneral, CPropertyPage)
	ON_BN_CLICKED(IDC_STARTMIN, OnSettingsChange)
	ON_BN_CLICKED(IDC_STARTWIN, OnSettingsChange)
	ON_EN_CHANGE(IDC_NICK, OnSettingsChange)
	ON_BN_CLICKED(IDC_BEEPER, OnSettingsChange)
	ON_BN_CLICKED(IDC_EXIT, OnSettingsChange)
	ON_BN_CLICKED(IDC_SPLASHON, OnSettingsChange)
	ON_BN_CLICKED(IDC_BRINGTOFOREGROUND, OnSettingsChange)
	ON_CBN_SELCHANGE(IDC_LANGS, OnLangChange)
	ON_BN_CLICKED(IDC_WEBSVEDIT , OnBnClickedEditWebservices)
	//ON_BN_CLICKED(IDC_ONLINESIG, OnSettingsChange)
	ON_BN_CLICKED(IDC_CHECK4UPDATE, OnBnClickedCheck4Update)
	ON_BN_CLICKED(IDC_WEBBROWSER, OnWebBroswerChange)  // Added by thilon on 2006.08.03
	//ON_BN_CLICKED(IDC_MINIMULE, OnSettingsChange)
	ON_WM_HSCROLL()
	ON_WM_HELPINFO()
	ON_NOTIFY(UDN_DELTAPOS, IDC_BUFSPIN, OnDeltaposBufspin)
	ON_CBN_SELCHANGE(IDC_CLOSE_MODE, OnCbnSelchangeCloseMode)
	ON_BN_CLICKED(IDC_NICK_FRM, OnBnClickedNickFrm)
	ON_CBN_SELCHANGE(IDC_COMBO_BUF, OnCbnSelchangeComboBuf)
END_MESSAGE_MAP()

CPPgGeneral::CPPgGeneral()
	: CPropertyPage(CPPgGeneral::IDD)
{
	m_iFileBufferSize = 0;
}

CPPgGeneral::~CPPgGeneral()
{
}

void CPPgGeneral::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LANGS, m_language);
	DDX_Control(pDX, IDC_CLOSE_MODE, m_CtrlCloseMode);
	DDX_Control(pDX, IDC_COMBO_BUF, m_DownloadBuffSizeCtrl);
}

void CPPgGeneral::LoadSettings(void)
{
	GetDlgItem(IDC_NICK)->SetWindowText(thePrefs.GetUserNick());

	for(int i = 0; i < m_language.GetCount(); i++)
		if(m_language.GetItemData(i) == thePrefs.GetLanguageID())
			m_language.SetCurSel(i);
	
	if(thePrefs.m_bAutoStart)
		CheckDlgButton(IDC_STARTWIN,1);
	else
		CheckDlgButton(IDC_STARTWIN,0);

	//Deleted by thilon  on 2008.03.20
	//if(thePrefs.startMinimized)
	//	CheckDlgButton(IDC_STARTMIN,1);
	//else
	//	CheckDlgButton(IDC_STARTMIN,0);

	if (thePrefs.onlineSig)
		CheckDlgButton(IDC_ONLINESIG,1);
	else
		CheckDlgButton(IDC_ONLINESIG,0);

	if (thePrefs.m_bShowBroswer)
		CheckDlgButton(IDC_WEBBROWSER,1); // Added by thilon on 2006.08.03, WebBroswer
	else
		CheckDlgButton(IDC_WEBBROWSER,0);
	
	if(thePrefs.beepOnError)
		CheckDlgButton(IDC_BEEPER,1);
	else
		CheckDlgButton(IDC_BEEPER,0);

	if(thePrefs.confirmExit)
		CheckDlgButton(IDC_EXIT,1);
	else
		CheckDlgButton(IDC_EXIT,0);

	if(thePrefs.splashscreen)
		CheckDlgButton(IDC_SPLASHON,1);
	else
		CheckDlgButton(IDC_SPLASHON,0);

	if(thePrefs.bringtoforeground)
		CheckDlgButton(IDC_BRINGTOFOREGROUND,1);
	else
		CheckDlgButton(IDC_BRINGTOFOREGROUND,0);

	if(thePrefs.updatenotify)
		CheckDlgButton(IDC_CHECK4UPDATE,1);
	else
		CheckDlgButton(IDC_CHECK4UPDATE,0);

	if(thePrefs.m_bEnableMiniMule)
		CheckDlgButton(IDC_MINIMULE,1);
	else
		CheckDlgButton(IDC_MINIMULE,0);

	CString strBuffer;
	strBuffer.Format(_T("%i %s"),thePrefs.versioncheckdays,GetResString(IDS_DAYS2));
	GetDlgItem(IDC_DAYS)->SetWindowText(strBuffer);

	switch(thePrefs.GetCloseMode())
	{
	case 0:
		m_CtrlCloseMode.SetCurSel(0);
		break;
	case 1:
		m_CtrlCloseMode.SetCurSel(1);
		break;
	case 2:
		m_CtrlCloseMode.SetCurSel(2);
		break;
	}

	//缓存模式
	switch(m_iFileBufferSize)
	{
	case 524288:
		m_DownloadBuffSizeCtrl.SetCurSel(0);
		break;
	case 1048576:
		m_DownloadBuffSizeCtrl.SetCurSel(1);
		break;
	case 2097152:
		m_DownloadBuffSizeCtrl.SetCurSel(2);
		break;
	case 4194304:
		m_DownloadBuffSizeCtrl.SetCurSel(3);
		break;
	case 8388608:
		m_DownloadBuffSizeCtrl.SetCurSel(4);
		break;
	case 16777216:
		m_DownloadBuffSizeCtrl.SetCurSel(5);
		break;
	}
}

BOOL CPPgGeneral::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);

	((CEdit*)GetDlgItem(IDC_NICK))->SetLimitText(thePrefs.GetMaxUserNickLength());

	CWordArray aLanguageIDs;
	thePrefs.GetLanguages(aLanguageIDs);
	for (int i = 0; i < aLanguageIDs.GetSize(); i++){
		TCHAR szLang[128];
		int ret=GetLocaleInfo(aLanguageIDs[i], LOCALE_SLANGUAGE, szLang, ARRSIZE(szLang));

		if (ret==0 && aLanguageIDs[i]== LANGID_GL_ES )
			_tcscpy(szLang,_T("Galician") );
		else if (ret==0 && aLanguageIDs[i]==LANGID_FR_BR )
			_tcscpy(szLang,_T("Breton (Brezhoneg)") );
		else if (ret==0 && aLanguageIDs[i]==LANGID_MT_MT )
			_tcscpy(szLang,_T("Maltese") );
		else if (ret==0 && aLanguageIDs[i]==LANGID_ES_AS )
			_tcscpy(szLang,_T("Asturian") );

		if (!_tcscmp(szLang, _T("中文(台湾)"))) //Added by thilon on 2006.09.06
			_tcscpy(szLang, _T("中文(繁体)"));
	
		if (!_tcscmp(szLang, _T("中文(中国)")))	//Added by thilon on 2006.09.06
			_tcscpy(szLang, _T("中文(简体)"));

		if (!_tcscmp(szLang, _T("Chinese (Taiwan)")))
			_tcscpy(szLang, _T("Chinese (Tradition)"));

		
		if (!_tcscmp(szLang, _T("Chinese (PRC)")))
			_tcscpy(szLang, _T("Chinese (Simplified)"));

		m_language.SetItemData(m_language.AddString(szLang), aLanguageIDs[i]);
	}

	//Chocobo Start
	//eMule自动更新，modified by Chocobo on 2006.08.01
	//屏蔽修改更新间隔
	CSliderCtrl *sliderUpdate = (CSliderCtrl*)GetDlgItem(IDC_CHECKDAYS);
	sliderUpdate->SetRange(1, 7, true);//自动更新时间1－7，modified by Chocobo on 2006.08.17
	sliderUpdate->SetPos(thePrefs.GetUpdateDays());
	//Chocobo End

	//Changed by thilon on 2007.11.01
	m_iFileBufferSize = thePrefs.m_iFileBufferSize;
	
	LoadSettings();
	//Localize();

	GetDlgItem(IDC_CHECKDAYS)->ShowWindow( IsDlgButtonChecked(IDC_CHECK4UPDATE) ? SW_SHOW : SW_HIDE );
	GetDlgItem(IDC_DAYS)->ShowWindow( IsDlgButtonChecked(IDC_CHECK4UPDATE) ? SW_SHOW : SW_HIDE );

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void ModifyAllWindowStyles(CWnd* pWnd, DWORD dwRemove, DWORD dwAdd)
{
	CWnd* pWndChild = pWnd->GetWindow(GW_CHILD);
	while (pWndChild)
	{
		ModifyAllWindowStyles(pWndChild, dwRemove, dwAdd);
		pWndChild = pWndChild->GetNextWindow();
	}

	if (pWnd->ModifyStyleEx(dwRemove, dwAdd, SWP_FRAMECHANGED))
	{
		pWnd->Invalidate();
//		pWnd->UpdateWindow();
	}
}

BOOL CPPgGeneral::OnApply()
{
	CString strNick;
	GetDlgItem(IDC_NICK)->GetWindowText(strNick);
	strNick.Trim();
	if (!IsValidEd2kString(strNick))
		strNick.Empty();
	if (strNick.IsEmpty())
	{
		strNick = DEFAULT_NICK;
		GetDlgItem(IDC_NICK)->SetWindowText(strNick);
	}
	thePrefs.SetUserNick(strNick);

	if (m_language.GetCurSel() != CB_ERR)
	{
		WORD wNewLang = (WORD)m_language.GetItemData(m_language.GetCurSel());
		if (thePrefs.GetLanguageID() != wNewLang)
		{
			thePrefs.SetLanguageID(wNewLang);
			thePrefs.SetLanguage();

#ifdef _DEBUG
			// Can't yet be switched on-the-fly, too much unresolved issues..
			if (thePrefs.GetRTLWindowsLayout())
			{
				ModifyAllWindowStyles(theApp.emuledlg, WS_EX_LAYOUTRTL | WS_EX_RTLREADING | WS_EX_RIGHT | WS_EX_LEFTSCROLLBAR, 0);
				ModifyAllWindowStyles(theApp.emuledlg->preferenceswnd, WS_EX_LAYOUTRTL | WS_EX_RTLREADING | WS_EX_RIGHT | WS_EX_LEFTSCROLLBAR, 0);
				theApp.DisableRTLWindowsLayout();
				thePrefs.m_bRTLWindowsLayout = false;
			}
#endif
			theApp.emuledlg->preferenceswnd->Localize();
			theApp.emuledlg->statisticswnd->CreateMyTree();
			theApp.emuledlg->statisticswnd->Localize();
			theApp.emuledlg->statisticswnd->ShowStatistics(true);
			theApp.emuledlg->serverwnd->Localize();
			theApp.emuledlg->transferwnd->Localize();
			theApp.emuledlg->transferwnd->UpdateCatTabTitles();
			theApp.emuledlg->searchwnd->Localize();
			theApp.emuledlg->sharedfileswnd->Localize();
			
			theApp.emuledlg->Localize();
#if _ENABLE_NOUSE
			theApp.emuledlg->chatwnd->Localize();
			theApp.emuledlg->ircwnd->Localize();
#endif
			theApp.emuledlg->kademliawnd->Localize();

			//{begin} VC-dgkang
			theApp.emuledlg->m_mainTabWnd.m_dlgResource.Localize();
			//{end}
		}
	}

	thePrefs.startMinimized = IsDlgButtonChecked(IDC_STARTMIN)!=0;
	thePrefs.m_bAutoStart = IsDlgButtonChecked(IDC_STARTWIN)!=0;
	if( thePrefs.m_bAutoStart )
		AddAutoStart();
	else
		RemAutoStart();
	thePrefs.beepOnError = IsDlgButtonChecked(IDC_BEEPER)!=0;
	thePrefs.confirmExit = IsDlgButtonChecked(IDC_EXIT)!=0;
	thePrefs.splashscreen = IsDlgButtonChecked(IDC_SPLASHON)!=0;
	thePrefs.bringtoforeground = IsDlgButtonChecked(IDC_BRINGTOFOREGROUND)!=0;
	thePrefs.updatenotify = IsDlgButtonChecked(IDC_CHECK4UPDATE)!=0;
	thePrefs.onlineSig = IsDlgButtonChecked(IDC_ONLINESIG)!=0;
	thePrefs.versioncheckdays = ((CSliderCtrl*)GetDlgItem(IDC_CHECKDAYS))->GetPos();
	thePrefs.m_bEnableMiniMule = IsDlgButtonChecked(IDC_MINIMULE) != 0;

	// Added by thilon on 2006.08.03, 显示隐藏浏览器
	if(	thePrefs.m_bShowBroswer != (IsDlgButtonChecked(IDC_WEBBROWSER)!=0))
	{
		thePrefs.m_bShowBroswer = IsDlgButtonChecked(IDC_WEBBROWSER)!=0;
		//MessageBox(GetResString(IDS_RESTARTFORBROWSER),_T("eMule"),MB_OK);
		if(!thePrefs.m_bShowBroswer)
		{
			thePrefs.m_bShowBroswer = 0;//隐藏浏览器按钮
		}
		else
		{
			thePrefs.m_bShowBroswer = 1;//显示浏览器按钮
		}
	}

	theApp.emuledlg->transferwnd->downloadlistctrl.SetStyle();

	// Changed by thilon on 2007.07.23
	thePrefs.m_iFileBufferSize = m_iFileBufferSize;

	LoadSettings();

	SetModified(FALSE);
	return CPropertyPage::OnApply();
}

BOOL CPPgGeneral::OnSetActive()
{
	return __super::OnSetActive();
}

void CPPgGeneral::Localize(void)
{
	if(m_hWnd)
	{
		SetWindowText(GetResString(IDS_PW_GENERAL));
		GetDlgItem(IDC_NICK_FRM)->SetWindowText(GetResString(IDS_QL_USERNAME));
		GetDlgItem(IDC_LANG_FRM)->SetWindowText(GetResString(IDS_PW_LANG));
		GetDlgItem(IDC_MISC_FRM)->SetWindowText(GetResString(IDS_PW_MISC));
		GetDlgItem(IDC_MISC_TRAY)->SetWindowText(GetResString(IDS_MISC_TRAY));
		GetDlgItem(IDC_BEEPER)->SetWindowText(GetResString(IDS_PW_BEEP));
		GetDlgItem(IDC_EXIT)->SetWindowText(GetResString(IDS_PW_PROMPT));
		//GetDlgItem(IDC_SPLASHON)->SetWindowText(GetResString(IDS_PW_SPLASH));
		GetDlgItem(IDC_BRINGTOFOREGROUND)->SetWindowText(GetResString(IDS_PW_FRONT));
		//GetDlgItem(IDC_ONLINESIG)->SetWindowText(GetResString(IDS_PREF_ONLINESIG));	
#ifdef _DISABLE_WEBBROWSER
		GetDlgItem(IDC_WEBBROWSER)->ShowWindow(SW_HIDE);
#else
		GetDlgItem(IDC_WEBBROWSER)->SetWindowText(GetResString(IDS_PREF_WEBBROWSER)); // Added by thilon on 2006.08.03
#endif		
		//GetDlgItem(IDC_STARTMIN)->SetWindowText(GetResString(IDS_PREF_STARTMIN));	
		//GetDlgItem(IDC_WEBSVEDIT)->SetWindowText(GetResString(IDS_WEBSVEDIT));
		GetDlgItem(IDC_CHECK4UPDATE)->SetWindowText(GetResString(IDS_CHECK4UPDATE));
		GetDlgItem(IDC_STARTUP)->SetWindowText(GetResString(IDS_STARTUP));
		GetDlgItem(IDC_STARTWIN)->SetWindowText(GetResString(IDS_STARTWITHWINDOWS));
		//GetDlgItem(IDC_MINIMULE)->SetWindowText(GetResString(IDS_ENABLEMINIMULE));

		CString temp;
		temp.Format(_T("%s:"), GetResString(IDS_DOWNLOADBUF));
		GetDlgItem(IDC_STATIC_DOWNLOADBUFFER)->SetWindowText(temp);

		GetDlgItem(IDC_STATIC_TRAY)->SetWindowText(GetResString(IDS_TRAY_CAPTION));
		
		m_CtrlCloseMode.ResetContent();
		m_CtrlCloseMode.AddString(GetResString(IDS_ASKME));
		m_CtrlCloseMode.AddString(GetResString(IDS_SHUTDOWN));
		m_CtrlCloseMode.AddString(GetResString(IDS_MINIMISE));

		switch(thePrefs.GetCloseMode())
		{
		case 0:
			m_CtrlCloseMode.SetCurSel(0);
			break;
		case 1:
			m_CtrlCloseMode.SetCurSel(1);
			break;
		case 2:
			m_CtrlCloseMode.SetCurSel(2);
			break;
		}

		//缓存模式
		m_DownloadBuffSizeCtrl.ResetContent();
		m_DownloadBuffSizeCtrl.AddString(GetResString(IDS_BUFFER0));	//512K
		m_DownloadBuffSizeCtrl.AddString(GetResString(IDS_BUFFER1));	//1M
		m_DownloadBuffSizeCtrl.AddString(GetResString(IDS_BUFFER2));	//2M
		m_DownloadBuffSizeCtrl.AddString(GetResString(IDS_BUFFER3));	//4M
		m_DownloadBuffSizeCtrl.AddString(GetResString(IDS_BUFFER4));	//8M
		m_DownloadBuffSizeCtrl.AddString(GetResString(IDS_BUFFER5));	//16M

		switch(m_iFileBufferSize)
		{
		case 524288:
			m_DownloadBuffSizeCtrl.SetCurSel(0);
			break;
		case 1048576:
			m_DownloadBuffSizeCtrl.SetCurSel(1);
			break;
		case 2097152:
			m_DownloadBuffSizeCtrl.SetCurSel(2);
			break;
		case 4194304:
			m_DownloadBuffSizeCtrl.SetCurSel(3);
			break;
		case 8388608:
			m_DownloadBuffSizeCtrl.SetCurSel(4);
			break;
		case 16777216:
			m_DownloadBuffSizeCtrl.SetCurSel(5);
			break;
		}
	}
}

void CPPgGeneral::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	SetModified(TRUE);

	if (pScrollBar==GetDlgItem(IDC_CHECKDAYS))
	{
		CSliderCtrl* slider =(CSliderCtrl*)pScrollBar;
		CString text;
		text.Format(_T("%i %s"),slider->GetPos(),GetResString(IDS_DAYS2));
		GetDlgItem(IDC_DAYS)->SetWindowText(text);
	}

	UpdateData(false); 
	CPropertyPage::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CPPgGeneral::OnBnClickedEditWebservices()
{
	theWebServices.Edit();
}

void CPPgGeneral::OnLangChange()
{
//MODIFIED by VC-fengwen on 2007/10/29 <begin> : 到verycd下语言包
//#define MIRRORS_URL	_T("http://langmirror%i.emule-project.org/lang/%i%i%i%i/")
#define MIRRORS_URL	_T("http://www.emule.org.cn/language/%06d/%s")
//MODIFIED by VC-fengwen on 2007/10/29 <end> : 到verycd下语言包

	WORD byNewLang = (WORD)m_language.GetItemData(m_language.GetCurSel());
	if (thePrefs.GetLanguageID() != byNewLang){
		if	(!thePrefs.IsLanguageSupported(byNewLang, false)){
			if (AfxMessageBox(GetResString(IDS_ASKDOWNLOADLANGCAP) + _T("\r\n\r\n") + GetResString(IDS_ASKDOWNLOADLANG), MB_ICONQUESTION | MB_YESNO) == IDYES){
				// download file
				// create url, use random mirror for load balancing

				//MODIFIED by VC-fengwen on 2007/10/29 <begin> : 到verycd下语言包
				//UINT nRand = (rand()/(RAND_MAX/3))+1;
				//CString strUrl;
				//strUrl.Format(MIRRORS_URL, nRand, CGlobalVariable::m_nVersionMjr, CGlobalVariable::m_nVersionMin, CGlobalVariable::m_nVersionUpd, CGlobalVariable::m_nVersionBld);
				//strUrl += thePrefs.GetLangDLLNameByID(byNewLang);
				
				CString strUrl;
				strUrl.Format(MIRRORS_URL, CGlobalVariable::m_nVCVersionBld, thePrefs.GetLangDLLNameByID(byNewLang));
				//MODIFIED by VC-fengwen on 2007/10/29 <end> : 到verycd下语言包

				// safeto
				CString strFilename = thePrefs.GetMuleDirectory(EMULE_ADDLANGDIR, true);

				strFilename.Append(thePrefs.GetLangDLLNameByID(byNewLang));
				// start
				CHttpDownloadDlg dlgDownload;
				dlgDownload.m_strTitle = GetResString(IDS_DOWNLOAD_LANGFILE);
				dlgDownload.m_sURLToDownload = strUrl;
				dlgDownload.m_sFileToDownloadInto = strFilename;
				if (dlgDownload.DoModal() == IDOK && thePrefs.IsLanguageSupported(byNewLang, true))
				{
					// everything ok, new language downloaded and working
					OnSettingsChange();
					return;
				}
				CString strErr;
				strErr.Format(GetResString(IDS_ERR_FAILEDDOWNLOADLANG), strUrl);
				LogError(LOG_STATUSBAR, _T("%s"), strErr);
				AfxMessageBox(strErr, MB_ICONERROR | MB_OK);
			}
			// undo change selection
			for(int i = 0; i < m_language.GetCount(); i++)
				if(m_language.GetItemData(i) == thePrefs.GetLanguageID())
					m_language.SetCurSel(i);
		}
		else
			OnSettingsChange();
	}
}

void CPPgGeneral::OnBnClickedCheck4Update()
{
	SetModified();
	GetDlgItem(IDC_CHECKDAYS)->ShowWindow( IsDlgButtonChecked(IDC_CHECK4UPDATE)?SW_SHOW:SW_HIDE );
	GetDlgItem(IDC_DAYS)->ShowWindow( IsDlgButtonChecked(IDC_CHECK4UPDATE)?SW_SHOW:SW_HIDE );
}

void CPPgGeneral::OnWebBroswerChange()
{
	SetModified();

	MessageBox(GetResString(IDS_WEBBROSWERS), GetResString(IDS_CAPTION));
}

void CPPgGeneral::OnHelp()
{
	theApp.ShowHelp(eMule_FAQ_Preferences_General);
}

BOOL CPPgGeneral::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_HELP)
	{
		OnHelp();
		return TRUE;
	}
	return __super::OnCommand(wParam, lParam);
}

BOOL CPPgGeneral::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	OnHelp();
	return TRUE;
}



void CPPgGeneral::OnDeltaposBufspin(NMHDR * /*pNMHDR*/, LRESULT *pResult)
{
	//LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	
	SetModified();// VC-kernel[2007-02-26]:

	*pResult = 0;
}

void CPPgGeneral::OnCbnSelchangeCloseMode()
{
	SetModified();

	switch(m_CtrlCloseMode.GetCurSel())
	{
	case 0:
		thePrefs.SetCloseMode(0);
		break;
	case 1:
		thePrefs.SetCloseMode(1);
		break;
	case 2:
		thePrefs.SetCloseMode(2);
		break;
	}
}

void CPPgGeneral::OnBnClickedNickFrm()
{
	// TODO: 在此添加控件通知处理程序代码
}

void CPPgGeneral::OnCbnSelchangeComboBuf()
{
	SetModified();

	switch(m_DownloadBuffSizeCtrl.GetCurSel())
	{
	case 0:
		m_iFileBufferSize = 524288; //512K
		break;
	case 1:
		m_iFileBufferSize = 1048576;	//1M
		break;
	case 2:
		m_iFileBufferSize = 2097152;	//2M
		break;
	case 3:
		m_iFileBufferSize = 4194304;	//4M
		break;
	case 4:
		m_iFileBufferSize = 8388608;	//8M
		break;
	case 5:
		m_iFileBufferSize = 16777216;	//16M
		break;
	}
}
