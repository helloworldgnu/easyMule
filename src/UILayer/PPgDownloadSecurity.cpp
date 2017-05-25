/*
 * $Id: PPgDownloadSecurity.cpp 6525 2008-08-05 05:58:06Z dgkang $
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
// E:\EasyMule\src\UILayer\PPgDownloadSecurity.cpp : 实现文件
//

#include "stdafx.h"
#include "eMule.h"
#include "PPgDownloadSecurity.h"
#include "Preferences.h"
#include "AntiVirusIDs.h"
#include "ScanDlg.h"
#include ".\ppgdownloadsecurity.h"

// CPPgDownloadSecurity 对话框

IMPLEMENT_DYNAMIC(CPPgDownloadSecurity, CPropertyPage)
CPPgDownloadSecurity::CPPgDownloadSecurity()
	: CPropertyPage(CPPgDownloadSecurity::IDD)
{
	m_SelectAntiVirus = PDS_REMOTE;
	m_bCreated = false;
}

CPPgDownloadSecurity::~CPPgDownloadSecurity()
{
}

void CPPgDownloadSecurity::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_ANTIVIRUSMODEL, m_AntiVirusModelCtrl);
}


BEGIN_MESSAGE_MAP(CPPgDownloadSecurity, CPropertyPage)
	ON_BN_CLICKED(IDC_ENABLESCANVIRUS_CHECK, OnBnClickedEnablescanvirusCheck)
	ON_BN_CLICKED(IDC_SELPROGRAM, OnBnClickedSelprogram)
	ON_BN_CLICKED(IDC_RADIO_ANTIVIRUS_MODEL, OnBnClickedRadioAntivirusModel)
	ON_BN_CLICKED(IDC_RADIO_ANTIANRUS_PROGS, OnBnClickedRadioAntianrusProgs)
	ON_EN_CHANGE(IDC_EDIT_FORMAT, OnEnChangeEditFormat)
	ON_EN_CHANGE(IDC_EDIT_ANTIVIRUS_ARGS, OnEnChangeEditAntivirusArgs)
	ON_BN_CLICKED(IDC_AUTOCHECK, OnBnClickedAutocheck)
	ON_EN_CHANGE(IDC_EDIT_ANTIVIRUS, OnEnChangeEditAntivirus)
	ON_CBN_SELCHANGE(IDC_COMBO_ANTIVIRUSMODEL, OnCbnSelchangeComboAntivirusmodel)
	ON_BN_CLICKED(IDC_ENABLESHAREFILES_CHECK, OnBnClickedEnablesharefilesCheck)
END_MESSAGE_MAP()


// CPPgDownloadSecurity 消息处理程序

void CPPgDownloadSecurity::OnBnClickedEnablescanvirusCheck()
{	

	GetDlgItem(IDC_RADIO_ANTIVIRUS_MODEL)->EnableWindow(IsDlgButtonChecked(IDC_ENABLESCANVIRUS_CHECK));
	GetDlgItem(IDC_COMBO_ANTIVIRUSMODEL)->EnableWindow(IsDlgButtonChecked(IDC_ENABLESCANVIRUS_CHECK) && (thePrefs.m_AntiVirusModel == ANTIANRUS_MODEL));

	GetDlgItem(IDC_RADIO_ANTIANRUS_PROGS)->EnableWindow(IsDlgButtonChecked(IDC_ENABLESCANVIRUS_CHECK));
	GetDlgItem(IDC_EDIT_ANTIVIRUS)->EnableWindow(IsDlgButtonChecked(IDC_ENABLESCANVIRUS_CHECK) && (thePrefs.m_AntiVirusModel == PROGRAM_MODEL));
	GetDlgItem(IDC_SELPROGRAM)->EnableWindow(IsDlgButtonChecked(IDC_ENABLESCANVIRUS_CHECK) && thePrefs.m_AntiVirusModel);
	GetDlgItem(IDC_AUTOCHECK)->EnableWindow(IsDlgButtonChecked(IDC_ENABLESCANVIRUS_CHECK) && thePrefs.m_AntiVirusModel);
	GetDlgItem(IDC_EDIT_ANTIVIRUS_ARGS)->EnableWindow(IsDlgButtonChecked(IDC_ENABLESCANVIRUS_CHECK) && thePrefs.m_AntiVirusModel);
	GetDlgItem(IDC_EDIT_FORMAT)->EnableWindow(IsDlgButtonChecked(IDC_ENABLESCANVIRUS_CHECK));

	SetModified();
}

void CPPgDownloadSecurity::LoadSettings()
{
	for(int i = 0; i < m_AntiVirusModelCtrl.GetCount(); i++)
	{
		if(m_AntiVirusModelCtrl.GetItemData(i) == thePrefs.GetAntiVirusID())
		{
			m_AntiVirusModelCtrl.SetCurSel(i);
		}
	}

	CheckDlgButton(IDC_ENABLESCANVIRUS_CHECK, thePrefs.IsEnableScanVirus());

	GetDlgItem(IDC_RADIO_ANTIVIRUS_MODEL)->EnableWindow(thePrefs.IsEnableScanVirus());
	GetDlgItem(IDC_COMBO_ANTIVIRUSMODEL)->EnableWindow(thePrefs.IsEnableScanVirus() && (thePrefs.m_AntiVirusModel == ANTIANRUS_MODEL));

	GetDlgItem(IDC_RADIO_ANTIANRUS_PROGS)->EnableWindow(thePrefs.IsEnableScanVirus());
	GetDlgItem(IDC_EDIT_ANTIVIRUS)->EnableWindow(thePrefs.IsEnableScanVirus() && (thePrefs.m_AntiVirusModel == PROGRAM_MODEL));
	GetDlgItem(IDC_SELPROGRAM)->EnableWindow(thePrefs.IsEnableScanVirus() && thePrefs.m_AntiVirusModel);
	GetDlgItem(IDC_AUTOCHECK)->EnableWindow(thePrefs.IsEnableScanVirus() && thePrefs.m_AntiVirusModel);
	GetDlgItem(IDC_EDIT_ANTIVIRUS_ARGS)->EnableWindow(thePrefs.IsEnableScanVirus() && thePrefs.m_AntiVirusModel);
	GetDlgItem(IDC_EDIT_FORMAT)->EnableWindow(thePrefs.IsEnableScanVirus());

	if (thePrefs.m_AntiVirusModel)
	{
		CheckDlgButton(IDC_RADIO_ANTIANRUS_PROGS, TRUE);
	}
	else
	{
		CheckDlgButton(IDC_RADIO_ANTIVIRUS_MODEL, TRUE);
	}

	GetDlgItem(IDC_EDIT_ANTIVIRUS)->SetWindowText(thePrefs.m_strAntiVirusPath);
	GetDlgItem(IDC_EDIT_FORMAT)->SetWindowText(thePrefs.m_strScanFormat);
	GetDlgItem(IDC_EDIT_ANTIVIRUS_ARGS)->SetWindowText(thePrefs.m_strProgsArgs);

	//{begin} VC-dgkang 2008年7月14日
	CheckDlgButton(IDC_ENABLESHAREFILES_CHECK,thePrefs.m_iSeeShares == vsfaEverybody || thePrefs.m_iSeeShares == vsfaFriends);
	//{end}
}

BOOL CPPgDownloadSecurity::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);

	CWordArray aAntiVirusIDs;
	thePrefs.GetAntiVirusProgs(aAntiVirusIDs);

	for (int i = 0; i < aAntiVirusIDs.GetSize(); i++)
	{
		TCHAR szAntiVirus[128];
		if (aAntiVirusIDs[i] == ANTIVIRUS_RISING)
		{
			_tcscpy(szAntiVirus, thePrefs.GetAntiVirusNameByID(ANTIVIRUS_RISING));
		}

		if (aAntiVirusIDs[i] == ANTIVIRUS_KING)
		{
			_tcscpy(szAntiVirus, thePrefs.GetAntiVirusNameByID(ANTIVIRUS_KING));
		}
		
		m_AntiVirusModelCtrl.SetItemData(m_AntiVirusModelCtrl.AddString(szAntiVirus),aAntiVirusIDs[i]);
	}

	CRect rect;
	GetDlgItem(IDC_STATIC_LINK)->GetWindowRect(rect);
	::MapWindowPoints(NULL, m_hWnd, (LPPOINT)&rect, 2);

	m_wndModelLink.CreateEx(NULL,0,_T("MsgWnd"),WS_VISIBLE | WS_CHILD | HTC_WORDWRAP | HTC_UNDERLINE_HOVER,rect.left,rect.top,rect.Width(),rect.Height(),m_hWnd,0);
	
	m_wndModelLink.SetBkColor(::GetSysColor(COLOR_WINDOW)); // still not the right color, will fix this later (need to merge the .rc file before it changes ;) )
	m_wndModelLink.SetFont(GetFont());

	if (!m_bCreated)
	{
		m_bCreated = true;
		//m_wndModelLink.AppendHyperLink(GetResString(IDS_MODEL_LINK),0,CString(_T("http://www.easymule.com/zh-cn/anti-virus/rising.html")),0,0);
		m_wndModelLink.AppendHyperLink(GetResString(IDS_MODEL_LINK),0,thePrefs.m_strAntivirusHomePage,0,0);
	}

	LoadSettings();
	Localize();

	//GetDlgItem(IDC_AUTOCHECK)->SendMessage(BM_CLICK, 0, 0);

	return TRUE;
}

BOOL CPPgDownloadSecurity::OnApply()
{
	thePrefs.SetEnableScanVirus(IsDlgButtonChecked(IDC_ENABLESCANVIRUS_CHECK)!=0);

	GetDlgItem(IDC_EDIT_ANTIVIRUS)->GetWindowText(thePrefs.m_strAntiVirusPath);
	thePrefs.m_strAntiVirusPath.Trim();

	GetDlgItem(IDC_EDIT_FORMAT)->GetWindowText(thePrefs.m_strScanFormat);
	thePrefs.m_strScanFormat.Trim();

	GetDlgItem(IDC_EDIT_ANTIVIRUS_ARGS)->GetWindowText(thePrefs.m_strProgsArgs);
	thePrefs.m_strProgsArgs.Trim();

	thePrefs.SetAntiVirusModel(IsDlgButtonChecked(IDC_RADIO_ANTIANRUS_PROGS) != ANTIANRUS_MODEL);

	if (m_AntiVirusModelCtrl.GetCurSel() != CB_ERR)
	{
		WORD wNewModel = (WORD)m_AntiVirusModelCtrl.GetItemData(m_AntiVirusModelCtrl.GetCurSel());

		if (thePrefs.GetAntiVirusID() != wNewModel)
		{
			thePrefs.SetAntiVirusID(wNewModel);
			thePrefs.SetSecurity();
		}
	}

	//VC-dgkang 2008年7月14日
	thePrefs.m_iSeeShares = IsDlgButtonChecked(IDC_ENABLESHAREFILES_CHECK) ? vsfaEverybody : vsfaNobody;

	LoadSettings();
	SetModified(FALSE);

	return CPropertyPage::OnApply();
}

void CPPgDownloadSecurity::OnBnClickedSelprogram()
{
	CString strAntiVirusPath;
	GetDlgItemText(IDC_EDIT_ANTIVIRUS, strAntiVirusPath);

	CFileDialog dlgFile(TRUE, _T("exe"), strAntiVirusPath, OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY, _T("Executable (*.exe)|*.exe||"), NULL, 0);
	if (dlgFile.DoModal() == IDOK)
	{
		GetDlgItem(IDC_EDIT_ANTIVIRUS)->SetWindowText(dlgFile.GetPathName());
		thePrefs.SetAntiVirusPath(dlgFile.GetPathName());
		SetModified();
	}
}
void CPPgDownloadSecurity::OnBnClickedRadioAntivirusModel()
{
	if (IsDlgButtonChecked(IDC_RADIO_ANTIVIRUS_MODEL))   
	{
		GetDlgItem(IDC_COMBO_ANTIVIRUSMODEL)->EnableWindow(true);

		GetDlgItem(IDC_EDIT_ANTIVIRUS)->EnableWindow(false);
		GetDlgItem(IDC_SELPROGRAM)->EnableWindow(false);
		GetDlgItem(IDC_AUTOCHECK)->EnableWindow(false);
		GetDlgItem(IDC_EDIT_ANTIVIRUS_ARGS)->EnableWindow(false);
		//GetDlgItem(IDC_EDIT_FORMAT)->EnableWindow(false);
	} 
	else
	{
		GetDlgItem(IDC_COMBO_ANTIVIRUSMODEL)->EnableWindow(false);

		GetDlgItem(IDC_EDIT_ANTIVIRUS)->EnableWindow(true);
		GetDlgItem(IDC_SELPROGRAM)->EnableWindow(true);
		GetDlgItem(IDC_AUTOCHECK)->EnableWindow(true);
		GetDlgItem(IDC_EDIT_ANTIVIRUS_ARGS)->EnableWindow(true);
		//GetDlgItem(IDC_EDIT_FORMAT)->EnableWindow(true);
	}

	thePrefs.SetAntiVirusModel(IsDlgButtonChecked(IDC_RADIO_ANTIANRUS_PROGS) != ANTIANRUS_MODEL);

	SetModified();
}

void CPPgDownloadSecurity::OnBnClickedRadioAntianrusProgs()
{
	if (IsDlgButtonChecked(IDC_RADIO_ANTIANRUS_PROGS))
	{
		GetDlgItem(IDC_COMBO_ANTIVIRUSMODEL)->EnableWindow(false);

		GetDlgItem(IDC_EDIT_ANTIVIRUS)->EnableWindow(true);
		GetDlgItem(IDC_SELPROGRAM)->EnableWindow(true);
		GetDlgItem(IDC_AUTOCHECK)->EnableWindow(true);
		GetDlgItem(IDC_EDIT_ANTIVIRUS_ARGS)->EnableWindow(true);
		//GetDlgItem(IDC_EDIT_FORMAT)->EnableWindow(true);
	} 
	else
	{
		GetDlgItem(IDC_COMBO_ANTIVIRUSMODEL)->EnableWindow(true);

		GetDlgItem(IDC_EDIT_ANTIVIRUS)->EnableWindow(false);
		GetDlgItem(IDC_SELPROGRAM)->EnableWindow(false);
		GetDlgItem(IDC_AUTOCHECK)->EnableWindow(false);
		GetDlgItem(IDC_EDIT_ANTIVIRUS_ARGS)->EnableWindow(false);
		//GetDlgItem(IDC_EDIT_FORMAT)->EnableWindow(false);
	}

	thePrefs.SetAntiVirusModel(IsDlgButtonChecked(IDC_RADIO_ANTIANRUS_PROGS) != ANTIANRUS_MODEL);

	SetModified();
}

void CPPgDownloadSecurity::OnEnChangeEditFormat()
{
	SetModified();
}

void CPPgDownloadSecurity::OnEnChangeEditAntivirusArgs()
{
	SetModified();
}

void CPPgDownloadSecurity::Localize(void)
{
	if(m_hWnd)
	{
		GetDlgItem(IDC_STATIC)->SetWindowText(GetResString(IDS_ANTIVIRUSSETTING));
		GetDlgItem(IDC_ENABLESCANVIRUS_CHECK)->SetWindowText(GetResString(IDS_ENABLESCANVIRUS_CHECK));
		GetDlgItem(IDC_RADIO_ANTIVIRUS_MODEL)->SetWindowText(GetResString(IDS_ANTIVIRUSMODEL));
		GetDlgItem(IDC_RADIO_ANTIANRUS_PROGS)->SetWindowText(GetResString(IDS_OTHERPROGRAM));
		GetDlgItem(IDC_SELPROGRAM)->SetWindowText(GetResString(IDS_SELPROGRAM));
		GetDlgItem(IDC_AUTOCHECK)->SetWindowText(GetResString(IDS_AUTOCHECK));
		GetDlgItem(IDC_STATIC_ARGS)->SetWindowText(GetResString(IDS_STATIC_ARGS));
		GetDlgItem(IDC_STATIC_FORMAT)->SetWindowText(GetResString(IDS_STATIC_FORMAT));
		GetDlgItem(IDC_STATIC_SHARE)->SetWindowText(GetResString(IDS_STATIC_SHARE));

		//{begin} VC-dgkang 2008年7月17日
		GetDlgItem(IDC_ENABLESHAREFILES_CHECK)->SetWindowText(GetResString(IDS_ENABLESHAREFILES_CHECK));

		/* 这样写居然不行，看样子这个类写的有问题。
		CPreparedHyperText * pText = m_wndModelLink.GetHyperText();
		if (pText)
		{
			pText->SetText(GetResString(IDS_MODEL_LINK));
		}
		*/
		m_wndModelLink.Clear();
		//VC-dgkang 2008年8月5日
		//m_wndModelLink.AppendHyperLink(GetResString(IDS_MODEL_LINK),0,CString(_T("http://www.easymule.com/zh-cn/anti-virus/rising.html")),0,0);
		m_wndModelLink.AppendHyperLink(GetResString(IDS_MODEL_LINK),0,thePrefs.m_strAntivirusHomePage,0,0);
		//{end}
	}
}

void CPPgDownloadSecurity::OnBnClickedAutocheck()
{
	CString strInstallPath;
	CString strName;
	CString strArguments;

	INT_PTR nRet = -1;
	CScanDlg ScanDlg(this);

	if (Check(strInstallPath, strArguments) == true)
	{
		GetDlgItem(IDC_EDIT_ANTIVIRUS)->SetWindowText(strInstallPath);
		GetDlgItem(IDC_EDIT_ANTIVIRUS_ARGS)->SetWindowText(strArguments);

	}
	else
	{
		nRet = ScanDlg.DoModal();

		switch(nRet)
		{
		case IDOK:
			if (m_SelectAntiVirus == PDS_REMOTE)
			{
				//ShellExecute(NULL, _T("open"), _T("http://www.easymule.com/zh-cn/anti-virus/rising.html"), NULL, NULL, SW_SHOWNORMAL);
				//VC-dgkang 2008年8月5日
				ShellExecute(NULL, _T("open"), thePrefs.m_strAntivirusHomePage, NULL, NULL, SW_SHOWNORMAL);
			}

			if (m_SelectAntiVirus == PDS_LOCAL)
			{
				CString strAntiVirusPath;
				CFileDialog dlgFile(TRUE, _T("exe"), strAntiVirusPath, OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY, _T("Executable (*.exe)|*.exe||"), NULL, 0);

				if (dlgFile.DoModal() == IDOK)
				{
					GetDlgItem(IDC_EDIT_ANTIVIRUS)->SetWindowText(dlgFile.GetPathName());
					thePrefs.SetAntiVirusPath(dlgFile.GetPathName());
					SetModified();
				}
			}
			break;
		case IDCANCEL:
			break;
		default:
			break;
		}
		
	}
}

//strPath[out] 杀毒软件路径
//strName[out] 杀毒软件名称
//success return 1 fail return 0
bool  CPPgDownloadSecurity::Check(CString& strPath, CString& strArguments)
{
	HKEY hKey = NULL;
	bool bRes = false;
	
	TCHAR szPath[100];

	DWORD dwType = REG_SZ; 
	DWORD SizePath = sizeof(szPath);

	SAntiVirusProgs* pSAntiVirusProgs = _SAntiVirusProgs;

	if (pSAntiVirusProgs)
	{
		while (pSAntiVirusProgs->antivirusid)
		{
			*szPath = 0;
			RegOpenKeyEx(HKEY_LOCAL_MACHINE, pSAntiVirusProgs->pszRegKey, 0, KEY_READ,&hKey);
			if (RegQueryValueEx(hKey, pSAntiVirusProgs->pszRegValPath, NULL, &dwType, (LPBYTE)szPath, &SizePath) != ERROR_SUCCESS)
			{
				RegCloseKey(hKey);
			}
			else
			{
				pSAntiVirusProgs->bSupported = true;
			}

			if (pSAntiVirusProgs->bSupported)
			{
				CString strInstallPath = CString(szPath);
				strInstallPath += _T("\\");
				strInstallPath += pSAntiVirusProgs->pszFileName;

				strPath = strInstallPath;
				strArguments = pSAntiVirusProgs->pszArguments;

				bRes = true;
				break;
			}
			else
			{
				pSAntiVirusProgs++;
			}
		}
	}
	else
	{
		return bRes;
	}

	RegCloseKey(hKey);
	return bRes;
}

void CPPgDownloadSecurity::OnEnChangeEditAntivirus()
{
	SetModified();
}

void CPPgDownloadSecurity::OnCbnSelchangeComboAntivirusmodel()
{
	SetModified();
}

void CPPgDownloadSecurity::SetSelectAntiVirus(UINT in)
{
	m_SelectAntiVirus = in;
}

//{begin} VC-dgkang 2008年7月14日
void CPPgDownloadSecurity::OnBnClickedEnablesharefilesCheck()
{
	// TODO: Add your control notification handler code here
	SetModified();
}
// {end}
