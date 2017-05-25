/*
 * $Id: PPgAntiLeech.cpp 6876 2008-08-27 09:36:20Z dgkang $
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
// PPgAntiLeech.cpp : implementation file
//

#include "stdafx.h"
#include "emule.h"
#include "PPgAntiLeech.h"
#include "emuleDlg.h"
#include "ServerWnd.h"
#include "Preferences.h"
#include "OtherFunctions.h"
#include "opcodes.h"
#include "DLP.h" //Xman DLP

#ifdef PRINT_STATISTIC
#include "UploadBandwidthThrottler.h"
#include "ClientCredits.h"
#include "ClientList.h"
#include "TransferWnd.h"
#include "DownloadQueue.h"
#include "BandWidthControl.h"
#endif


// CPPgXtreme dialog
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CPPgAntiLeech, CPropertyPage)
CPPgAntiLeech::CPPgAntiLeech()
	: CPropertyPage(CPPgAntiLeech::IDD)
{
}

CPPgAntiLeech::~CPPgAntiLeech()
{
}

void CPPgAntiLeech::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CPPgAntiLeech, CPropertyPage)
	ON_BN_CLICKED(IDC_ANTILEECHER_CHECK, OnBnClickedAntiLeecher) //Xman Anti-Leecher
	ON_BN_CLICKED(IDC_ANTILEECHERLOG_CHECK, OnSettingsChange) //Xman Anti-Leecher
	ON_BN_CLICKED(IDC_ANTILEECHERNAME_CHECK, OnSettingsChange) //Xman Anti-Leecher
	ON_BN_CLICKED(IDC_ANTIGHOST_CHECK, OnSettingsChange) //Xman Anti-Leecher
	ON_BN_CLICKED(IDC_ANTILEECHERBADHELLO_CHECK, OnSettingsChange) //Xman Anti-Leecher
	ON_BN_CLICKED(IDC_ANTILEECHERSNAFU_CHECK, OnSettingsChange) //Xman Anti-Leecher
	ON_BN_CLICKED(IDC_ANTILEECHERMOD_CHECK, OnSettingsChange) //Xman Anti-Leecher
	ON_BN_CLICKED(IDC_ANTILEECHERTHIEF_CHECK, OnSettingsChange) //Xman Anti-Leecher
	ON_BN_CLICKED(IDC_ANTILEECHERSPAMMER_CHECK, OnSettingsChange) //Xman Anti-Leecher
	ON_BN_CLICKED(IDC_ANTILEECHERXSEXPLOITER_CHECK, OnSettingsChange) //Xman Anti-Leecher
	ON_BN_CLICKED(IDC_ANTILEECHEREMCRYPT_CHECK, OnSettingsChange) //Xman Anti-Leecher
	ON_BN_CLICKED(IDC_ANTILEECHERUSERHASH_CHECK, OnSettingsChange) //Xman Anti-Leecher
	ON_BN_CLICKED(IDC_RADIO_LEECHERCOMMUNITY_1, OnSettingsChange) //Xman Anti-Leecher
	ON_BN_CLICKED(IDC_RADIO_LEECHERCOMMUNITY_2, OnSettingsChange) //Xman Anti-Leecher
	ON_BN_CLICKED(IDC_RADIO_LEECHERGHOST_1, OnSettingsChange) //Xman Anti-Leecher
	ON_BN_CLICKED(IDC_RADIO_LEECHERGHOST_2, OnSettingsChange) //Xman Anti-Leecher
	ON_BN_CLICKED(IDC_RADIO_LEECHERTHIEF_1, OnSettingsChange) //Xman Anti-Leecher
	ON_BN_CLICKED(IDC_RADIO_LEECHERTHIEF_2, OnSettingsChange) //Xman Anti-Leecher
	ON_BN_CLICKED(IDC_DLPRELOAD, OnBnClickedDlpreload) //Xman dlp
	ON_BN_CLICKED(IDC_ANTILEECHER_UPDATE_CHECK, &CPPgAntiLeech::OnBnClickedAntileecherUpdateCheck)
END_MESSAGE_MAP()

// CPPgXtreme message handlers

BOOL CPPgAntiLeech::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);


	LoadSettings();
	Localize();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CPPgAntiLeech::LoadSettings(void)
{
		CString buffer;
		//Xman Anti-Leecher

		CheckDlgButton(IDC_ANTILEECHER_CHECK, thePrefs.GetAntiLeecher());
		CheckDlgButton(IDC_ANTILEECHERLOG_CHECK, thePrefs.GetAntiLeecherLog());
		//remark don't use the Getfunction at this point!
		CheckDlgButton(IDC_ANTILEECHERBADHELLO_CHECK, thePrefs.m_antileecherbadhello);
		CheckDlgButton(IDC_ANTILEECHERSNAFU_CHECK, thePrefs.m_antileechersnafu);
		CheckDlgButton(IDC_ANTIGHOST_CHECK, thePrefs.m_antighost);
		CheckDlgButton(IDC_ANTILEECHERMOD_CHECK, thePrefs.m_antileechermod);
		CheckDlgButton(IDC_ANTILEECHERNAME_CHECK, thePrefs.m_antileechername);
		CheckDlgButton(IDC_ANTILEECHERTHIEF_CHECK, thePrefs.m_antileecherthief);
		CheckDlgButton(IDC_ANTILEECHERSPAMMER_CHECK, thePrefs.m_antileecherspammer);
		CheckDlgButton(IDC_ANTILEECHERXSEXPLOITER_CHECK, thePrefs.m_antileecherxsexploiter);
		CheckDlgButton(IDC_ANTILEECHEREMCRYPT_CHECK, thePrefs.m_antileecheremcrypt);
		CheckDlgButton(IDC_ANTILEECHERUSERHASH_CHECK, thePrefs.m_antileecheruserhash);

		//{begin}VC-dgkang 
		CheckDlgButton(IDC_ANTILEECHER_UPDATE_CHECK,thePrefs.m_bUpdateAntiLeecher);
		//{end}

		//{begin}VC-dgkang 2008年7月1日
		//修改其逻辑混乱
		if(thePrefs.m_antileechercommunity_action)
		{
			CheckDlgButton(IDC_RADIO_LEECHERCOMMUNITY_2, TRUE);
			CheckDlgButton(IDC_RADIO_LEECHERCOMMUNITY_1, FALSE);			
		}
		else
		{
			CheckDlgButton(IDC_RADIO_LEECHERCOMMUNITY_1, TRUE);
			CheckDlgButton(IDC_RADIO_LEECHERCOMMUNITY_2, FALSE);
		}
		if(thePrefs.m_antileecherghost_action)
		{
			CheckDlgButton(IDC_RADIO_LEECHERGHOST_2, TRUE);
			CheckDlgButton(IDC_RADIO_LEECHERGHOST_1, FALSE);
		}
		else
		{
			CheckDlgButton(IDC_RADIO_LEECHERGHOST_1, TRUE);
			CheckDlgButton(IDC_RADIO_LEECHERGHOST_2, FALSE);
		}
		if(thePrefs.m_antileecherthief_action)
		{
			CheckDlgButton(IDC_RADIO_LEECHERTHIEF_2, TRUE);
			CheckDlgButton(IDC_RADIO_LEECHERTHIEF_1, FALSE);
		}
		else
		{
			CheckDlgButton(IDC_RADIO_LEECHERTHIEF_1, TRUE);
			CheckDlgButton(IDC_RADIO_LEECHERTHIEF_2, FALSE);
		}
		//{end}VC-dgkang 2008年7月1日
		OnBnClickedAntiLeecher();
		//Xman end

		//Xman DLP
		if(CGlobalVariable::dlp->IsDLPavailable())
		{
			buffer.Format(_T("antiLeech.dll (DLP v%u)"), CGlobalVariable::dlp->GetDLPVersion());
			GetDlgItem(IDC_DLP_STATIC)->SetWindowText(buffer);
			GetDlgItem(IDC_ANTILEECHER_CHECK)->EnableWindow(true);
		}
		else
		{
			GetDlgItem(IDC_DLP_STATIC)->SetWindowText(_T("failed"));
			GetDlgItem(IDC_ANTILEECHER_CHECK)->EnableWindow(false);
			GetDlgItem(IDC_ANTILEECHERNAME_CHECK)->EnableWindow(false);
			GetDlgItem(IDC_ANTIGHOST_CHECK)->EnableWindow(false);
			GetDlgItem(IDC_ANTILEECHERBADHELLO_CHECK)->EnableWindow(false);
			GetDlgItem(IDC_ANTILEECHERSNAFU_CHECK)->EnableWindow(false);
			GetDlgItem(IDC_ANTILEECHERMOD_CHECK)->EnableWindow(false);
			GetDlgItem(IDC_ANTILEECHERTHIEF_CHECK)->EnableWindow(false);
			GetDlgItem(IDC_ANTILEECHERSPAMMER_CHECK)->EnableWindow(false);
			GetDlgItem(IDC_ANTILEECHERXSEXPLOITER_CHECK)->EnableWindow(false);
			GetDlgItem(IDC_ANTILEECHEREMCRYPT_CHECK)->EnableWindow(false);
			GetDlgItem(IDC_ANTILEECHERUSERHASH_CHECK)->EnableWindow(false);
			GetDlgItem(IDC_STATIC_LEECHERCOMMUNITY)->EnableWindow(false);
			GetDlgItem(IDC_STATIC_LEECHERGHOST)->EnableWindow(false);
			GetDlgItem(IDC_STATIC_LEECHERTHIEF)->EnableWindow(false);
			GetDlgItem(IDC_RADIO_LEECHERCOMMUNITY_1)->EnableWindow(false);
			GetDlgItem(IDC_RADIO_LEECHERCOMMUNITY_2)->EnableWindow(false);
			GetDlgItem(IDC_RADIO_LEECHERGHOST_1)->EnableWindow(false);
			GetDlgItem(IDC_RADIO_LEECHERGHOST_2)->EnableWindow(false);
			GetDlgItem(IDC_RADIO_LEECHERTHIEF_1)->EnableWindow(false);
			GetDlgItem(IDC_RADIO_LEECHERTHIEF_2)->EnableWindow(false);
		}
		//Xman end
}

BOOL CPPgAntiLeech::OnApply()
{
	//Xman Anti-Leecher
	thePrefs.SetAntiLeecher(IsDlgButtonChecked(IDC_ANTILEECHER_CHECK)!=0);
	thePrefs.SetAntiLeecherLog(IsDlgButtonChecked(IDC_ANTILEECHERLOG_CHECK) != 0);
	thePrefs.SetAntiLeecherName(IsDlgButtonChecked(IDC_ANTILEECHERNAME_CHECK)!=0);
	thePrefs.SetAntiGhost(IsDlgButtonChecked(IDC_ANTIGHOST_CHECK)!=0);
	thePrefs.SetAntiLeecherBadHello(IsDlgButtonChecked(IDC_ANTILEECHERBADHELLO_CHECK)!=0);
	thePrefs.SetAntiLeecherSnafu(IsDlgButtonChecked(IDC_ANTILEECHERSNAFU_CHECK)!=0);
	thePrefs.SetAntiLeecherMod(IsDlgButtonChecked(IDC_ANTILEECHERMOD_CHECK)!=0);
	thePrefs.SetAntiLeecherThief(IsDlgButtonChecked(IDC_ANTILEECHERTHIEF_CHECK)!=0);
	thePrefs.SetAntiLeecherSpammer(IsDlgButtonChecked(IDC_ANTILEECHERSPAMMER_CHECK)!=0);
	thePrefs.SetAntiLeecherXSExploiter(IsDlgButtonChecked(IDC_ANTILEECHERXSEXPLOITER_CHECK)!=0);
	thePrefs.SetAntiLeecheremcrypt(IsDlgButtonChecked(IDC_ANTILEECHEREMCRYPT_CHECK)!=0);
	thePrefs.SetAntiLeecheruserhash(IsDlgButtonChecked(IDC_ANTILEECHERUSERHASH_CHECK)!=0);
	thePrefs.SetAntiLeecherCommunity_Action(IsDlgButtonChecked(IDC_RADIO_LEECHERCOMMUNITY_2)!=0);
	thePrefs.SetAntiLeecherGhost_Action(IsDlgButtonChecked(IDC_RADIO_LEECHERGHOST_2)!=0);
	thePrefs.SetAntiLeecherThief_Action(IsDlgButtonChecked(IDC_RADIO_LEECHERTHIEF_2)!=0);
	//Xman end

	LoadSettings();
	SetModified(FALSE);

	theApp.emuledlg->serverwnd->ToggleDLPWindow();
	theApp.emuledlg->serverwnd->UpdateLogTabSelection();

	return CPropertyPage::OnApply();
}

void CPPgAntiLeech::Localize(void)
{	
	if(m_hWnd)
	{
		CString buffer;

		//Xman Anti-Leecher
		GetDlgItem(IDC_ANTILEECHER_GROUP)->SetWindowText(GetResString(IDS_ANTILEECHER_GROUP));
		GetDlgItem(IDC_ANTILEECHER_CHECK)->SetWindowText(GetResString(IDS_ANTILEECHER_CHECK));
		GetDlgItem(IDC_ANTILEECHERLOG_CHECK)->SetWindowText(GetResString(IDS_ANTILEECHER_LOG));
		GetDlgItem(IDC_ANTILEECHERNAME_CHECK)->SetWindowText(GetResString(IDS_ANTILEECHERNAME_CHECK));
		GetDlgItem(IDC_ANTIGHOST_CHECK)->SetWindowText(GetResString(IDS_ANTIGHOST));
		GetDlgItem(IDC_ANTILEECHERBADHELLO_CHECK)->SetWindowText(GetResString(IDS_ANTILEECHERBADHELLO_CHECK));
		GetDlgItem(IDC_ANTILEECHERSNAFU_CHECK)->SetWindowText(GetResString(IDS_ANTILEECHERSNAFU_CHECK));
		GetDlgItem(IDC_ANTILEECHERMOD_CHECK)->SetWindowText(GetResString(IDS_ANTILEECHERMOD_CHECK));
		GetDlgItem(IDC_ANTILEECHERTHIEF_CHECK)->SetWindowText(GetResString(IDS_ANTILEECHERTHIEF_CHECK));
		GetDlgItem(IDC_ANTILEECHERSPAMMER_CHECK)->SetWindowText(GetResString(IDS_ANTILEECHERSPAMMER_CHECK));
		GetDlgItem(IDC_ANTILEECHERXSEXPLOITER_CHECK)->SetWindowText(GetResString(IDS_ANTILEECHERXSEXPLOITER_CHECK));
		GetDlgItem(IDC_ANTILEECHEREMCRYPT_CHECK)->SetWindowText(GetResString(IDS_ANTILEECHEREMCRYPT_CHECK));
		GetDlgItem(IDC_ANTILEECHERUSERHASH_CHECK)->SetWindowText(GetResString(IDS_ANTILEECHERUSERHASH_CHECK));
		GetDlgItem(IDC_STATIC_LEECHERCOMMUNITY)->SetWindowText(GetResString(IDS_STATIC_LEECHERCOMMUNITY));
		GetDlgItem(IDC_STATIC_LEECHERGHOST)->SetWindowText(GetResString(IDS_STATIC_LEECHERGHOST));
		GetDlgItem(IDC_STATIC_LEECHERTHIEF)->SetWindowText(GetResString(IDS_STATIC_LEECHERTHIEF));
		GetDlgItem(IDC_RADIO_LEECHERCOMMUNITY_1)->SetWindowText(GetResString(IDS_LEECHER_ACTION_BAN));
		GetDlgItem(IDC_RADIO_LEECHERCOMMUNITY_2)->SetWindowText(GetResString(IDS_LEECHER_ACTION_REDUCE));
		GetDlgItem(IDC_RADIO_LEECHERGHOST_1)->SetWindowText(GetResString(IDS_LEECHER_ACTION_BAN));
		GetDlgItem(IDC_RADIO_LEECHERGHOST_2)->SetWindowText(GetResString(IDS_LEECHER_ACTION_REDUCE));
		GetDlgItem(IDC_RADIO_LEECHERTHIEF_1)->SetWindowText(GetResString(IDS_LEECHER_ACTION_BAN));
		GetDlgItem(IDC_RADIO_LEECHERTHIEF_2)->SetWindowText(GetResString(IDS_LEECHER_ACTION_REDUCE));
		GetDlgItem(IDC_DLPRELOAD)->SetWindowText(GetResString(IDS_SF_RELOAD));

		//VC-dgkang
		GetDlgItem(IDC_ANTILEECHER_UPDATE_CHECK)->SetWindowText(GetResString(IDS_ANTILEECHER_UPDATE));

		//Xman DLP
		if(CGlobalVariable::dlp->IsDLPavailable())
		{
			buffer.Format(_T("antiLeech.dll (DLP v%u)"), CGlobalVariable::dlp->GetDLPVersion());
			GetDlgItem(IDC_DLP_STATIC)->SetWindowText(buffer);
		}
		else
			GetDlgItem(IDC_DLP_STATIC)->SetWindowText(_T("failed"));
		//Xman end

	}
}


//Xman Anti-Leecher
void CPPgAntiLeech::OnBnClickedAntiLeecher()
{
	if(!IsDlgButtonChecked(IDC_ANTILEECHER_CHECK))
	{
		GetDlgItem(IDC_ANTILEECHERNAME_CHECK)->EnableWindow(false);
		GetDlgItem(IDC_ANTILEECHERLOG_CHECK)->EnableWindow(false);
		GetDlgItem(IDC_ANTIGHOST_CHECK)->EnableWindow(false);
		GetDlgItem(IDC_ANTILEECHERBADHELLO_CHECK)->EnableWindow(false);
		GetDlgItem(IDC_ANTILEECHERSNAFU_CHECK)->EnableWindow(false);
		GetDlgItem(IDC_ANTILEECHERMOD_CHECK)->EnableWindow(false);
		GetDlgItem(IDC_ANTILEECHERTHIEF_CHECK)->EnableWindow(false);
		GetDlgItem(IDC_ANTILEECHERSPAMMER_CHECK)->EnableWindow(false);
		GetDlgItem(IDC_ANTILEECHERXSEXPLOITER_CHECK)->EnableWindow(false);
		GetDlgItem(IDC_ANTILEECHEREMCRYPT_CHECK)->EnableWindow(false);
		GetDlgItem(IDC_ANTILEECHERUSERHASH_CHECK)->EnableWindow(false);
		GetDlgItem(IDC_STATIC_LEECHERCOMMUNITY)->EnableWindow(false);
		GetDlgItem(IDC_STATIC_LEECHERGHOST)->EnableWindow(false);
		GetDlgItem(IDC_STATIC_LEECHERTHIEF)->EnableWindow(false);
		GetDlgItem(IDC_RADIO_LEECHERCOMMUNITY_1)->EnableWindow(false);
		GetDlgItem(IDC_RADIO_LEECHERCOMMUNITY_2)->EnableWindow(false);
		GetDlgItem(IDC_RADIO_LEECHERGHOST_1)->EnableWindow(false);
		GetDlgItem(IDC_RADIO_LEECHERGHOST_2)->EnableWindow(false);
		GetDlgItem(IDC_RADIO_LEECHERTHIEF_1)->EnableWindow(false);
		GetDlgItem(IDC_RADIO_LEECHERTHIEF_2)->EnableWindow(false);
	}
	else
	{
		GetDlgItem(IDC_ANTILEECHERNAME_CHECK)->EnableWindow(true);
		GetDlgItem(IDC_ANTILEECHERLOG_CHECK)->EnableWindow(true);
		GetDlgItem(IDC_ANTIGHOST_CHECK)->EnableWindow(true);
		GetDlgItem(IDC_ANTILEECHERBADHELLO_CHECK)->EnableWindow(true);
		GetDlgItem(IDC_ANTILEECHERSNAFU_CHECK)->EnableWindow(true);
		GetDlgItem(IDC_ANTILEECHERMOD_CHECK)->EnableWindow(true);
		GetDlgItem(IDC_ANTILEECHERTHIEF_CHECK)->EnableWindow(true);
		GetDlgItem(IDC_ANTILEECHERSPAMMER_CHECK)->EnableWindow(true);
		GetDlgItem(IDC_ANTILEECHERXSEXPLOITER_CHECK)->EnableWindow(true);
		GetDlgItem(IDC_ANTILEECHEREMCRYPT_CHECK)->EnableWindow(true);
		GetDlgItem(IDC_ANTILEECHERUSERHASH_CHECK)->EnableWindow(true);
		GetDlgItem(IDC_STATIC_LEECHERCOMMUNITY)->EnableWindow(true);
		GetDlgItem(IDC_STATIC_LEECHERGHOST)->EnableWindow(true);
		GetDlgItem(IDC_STATIC_LEECHERTHIEF)->EnableWindow(true);
		GetDlgItem(IDC_RADIO_LEECHERCOMMUNITY_1)->EnableWindow(true);
		GetDlgItem(IDC_RADIO_LEECHERCOMMUNITY_2)->EnableWindow(true);
		GetDlgItem(IDC_RADIO_LEECHERGHOST_1)->EnableWindow(true);
		GetDlgItem(IDC_RADIO_LEECHERGHOST_2)->EnableWindow(true);
		GetDlgItem(IDC_RADIO_LEECHERTHIEF_1)->EnableWindow(true);
		GetDlgItem(IDC_RADIO_LEECHERTHIEF_2)->EnableWindow(true);
	}
	OnSettingsChange();
}
//Xman end


//Xman DLP
void CPPgAntiLeech::OnBnClickedDlpreload()
{
	CGlobalVariable::dlp->Reload();
	LoadSettings();
}
//Xman end





void CPPgAntiLeech::OnBnClickedAntileecherUpdateCheck()
{
	// TODO: Add your control notification handler code here
	//{begin} VC-dgkang
	thePrefs.m_bUpdateAntiLeecher = IsDlgButtonChecked(IDC_ANTILEECHER_UPDATE_CHECK);
	//{end}
	SetModified();
}
