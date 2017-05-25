/* 
 * $Id: PPgDirectories.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
#include "emuledlg.h"
#include "SharedFilesWnd.h"
#include "PPgDirectories.h"
#include "otherfunctions.h"
#include "InputBox.h"
#include "SharedFileList.h"
#include "Preferences.h"
#include "HelpIDs.h"
#include "UserMsgs.h"
#include ".\ppgdirectories.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNAMIC(CPPgDirectories, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgDirectories, CPropertyPage)
	ON_BN_CLICKED(IDC_SELTEMPDIR, OnBnClickedSeltempdir)
	ON_BN_CLICKED(IDC_SELINCDIR,OnBnClickedSelincdir)
	ON_EN_CHANGE(IDC_INCFILES,	OnSettingsChange)
	//ON_EN_CHANGE(IDC_TEMPFILES, OnSettingsChange)
	ON_BN_CLICKED(IDC_UNCADD,	OnBnClickedAddUNC)
	ON_BN_CLICKED(IDC_UNCREM,	OnBnClickedRemUNC)
	ON_WM_HELPINFO()
	//ON_BN_CLICKED(IDC_SELTEMPDIRADD, OnBnClickedSeltempdiradd)
	ON_BN_CLICKED(IDC_INCOMING_FRM, OnBnClickedIncomingFrm)
	ON_NOTIFY(TVN_SELCHANGED, IDC_SHARESELECTOR, OnTvnSelchangedShareselector)
END_MESSAGE_MAP()

CPPgDirectories::CPPgDirectories()
	: CPropertyPage(CPPgDirectories::IDD)
{
	
}

CPPgDirectories::~CPPgDirectories()
{
}

void CPPgDirectories::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SHARESELECTOR, m_ShareSelector);
	DDX_Control(pDX, IDC_UNCLIST, m_ctlUncPaths);
}

BOOL CPPgDirectories::OnInitDialog()
{
	CWaitCursor curWait; // initialization of that dialog may take a while..
	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);
	m_ShareSelector.Init();	

	((CEdit*)GetDlgItem(IDC_INCFILES))->SetLimitText(509);
	//((CEdit*)GetDlgItem(IDC_TEMPFILES))->SetLimitText(509);
	m_ctlUncPaths.InsertColumn(0, GetResString(IDS_UNCFOLDERS), LVCFMT_LEFT, 280, -1); 
	m_ctlUncPaths.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	//GetDlgItem(IDC_SELTEMPDIRADD)->ShowWindow(thePrefs.IsExtControlsEnabled()?SW_SHOW:SW_HIDE);

	m_ttc.Create(this);
	m_ttc.AddTool(GetDlgItem(IDC_SHARESELECTOR), GetResString(IDS_TIP_SHARE_DIR));
	m_ttc.AddTool(GetDlgItem(IDC_UNCLIST), GetResString(IDS_TIP_UNC_PATH));
	m_ttc.AddTool(GetDlgItem(IDC_INCFILES), GetResString(IDS_TIP_INCOMING_FILES));


	LoadSettings();
	Localize();

	


	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CPPgDirectories::LoadSettings(void)
{
	GetDlgItem(IDC_INCFILES)->SetWindowText(thePrefs.m_strIncomingDir);

	CString tempfolders;
	for (int i=0;i<thePrefs.tempdir.GetCount();i++) {
		tempfolders.Append(thePrefs.GetTempDir(i));
		if (i+1<thePrefs.tempdir.GetCount())
			tempfolders.Append(_T("|") );
	}

	//GetDlgItem(IDC_TEMPFILES)->SetWindowText(tempfolders);

	m_ShareSelector.SetSharedDirectories(&thePrefs.shareddir_list);
	FillUncList();
}

void CPPgDirectories::OnBnClickedSelincdir()
{
	TCHAR buffer[MAX_PATH] = {0};
	GetDlgItemText(IDC_INCFILES, buffer, ARRSIZE(buffer));
	if(SelectDir(GetSafeHwnd(),buffer,GetResString(IDS_SELECT_INCOMINGDIR)))
		GetDlgItem(IDC_INCFILES)->SetWindowText(buffer);
}

void CPPgDirectories::OnBnClickedSeltempdir()
{
	//TCHAR buffer[MAX_PATH] = {0};
	//GetDlgItemText(IDC_TEMPFILES, buffer, ARRSIZE(buffer));
	//if(SelectDir(GetSafeHwnd(),buffer,GetResString(IDS_SELECT_TEMPDIR)))
		//GetDlgItem(IDC_TEMPFILES)->SetWindowText(buffer);
}

BOOL CPPgDirectories::OnApply()
{
	//bool testtempdirchanged=false;
	CString testincdirchanged = thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR);

	CString strIncomingDir;
	GetDlgItemText(IDC_INCFILES, strIncomingDir);
	if (strIncomingDir.IsEmpty())
	{
		strIncomingDir = thePrefs.GetDefaultDirectory(EMULE_INCOMINGDIR, true); // will create the directory here if it doesnt exists
		SetDlgItemText(IDC_INCFILES, strIncomingDir);
	}
	else
	{
		if (thePrefs.IsInstallationDirectory(strIncomingDir))
		{
			AfxMessageBox(GetResString(IDS_WRN_INCFILE_RESERVED));
			return FALSE;
		}
		else
		{
			thePrefs.AddSaveLocation(strIncomingDir);
		}
	}	
	
	// checking specified tempdir(s)
/*
	CString strTempDir;
	//GetDlgItemText(IDC_TEMPFILES, strTempDir);
	if (strTempDir.IsEmpty()){
		strTempDir = thePrefs.GetDefaultDirectory(EMULE_TEMPDIR, true); // will create the directory here if it doesnt exists
		//SetDlgItemText(IDC_TEMPFILES, strTempDir);
	}


	int curPos=0;
	CStringArray temptempfolders;
	CString atmp=strTempDir.Tokenize(_T("|"), curPos);
	while (!atmp.IsEmpty())
	{
		atmp.Trim();
		if (!atmp.IsEmpty()) {
			if (CompareDirectories(strIncomingDir, atmp)==0){
					AfxMessageBox(GetResString(IDS_WRN_INCTEMP_SAME));
					return FALSE;
			}	
			if (thePrefs.IsInstallationDirectory(atmp)){
				AfxMessageBox(GetResString(IDS_WRN_TEMPFILES_RESERVED));
				return FALSE;
			}
			bool doubled=false;
			for (int i=0;i<temptempfolders.GetCount();i++)	// avoid double tempdirs
				if (temptempfolders.GetAt(i).CompareNoCase(atmp)==0) {
					doubled=true;
					break;
				}
			if (!doubled) {
				temptempfolders.Add(atmp);
				if (thePrefs.tempdir.GetCount()>=temptempfolders.GetCount()) {
					if( atmp.CompareNoCase(thePrefs.GetTempDir(temptempfolders.GetCount()-1))!=0	)
						testtempdirchanged=true;
				} else testtempdirchanged=true;

			}
		}
		atmp = strTempDir.Tokenize(_T("|"), curPos);
	}

	if (temptempfolders.IsEmpty())
		temptempfolders.Add(strTempDir = thePrefs.GetDefaultDirectory(EMULE_TEMPDIR, true));

	if (temptempfolders.GetCount()!=thePrefs.tempdir.GetCount())
		testtempdirchanged=true;

	// applying tempdirs
	if (testtempdirchanged) {
		thePrefs.tempdir.RemoveAll();
		for (int i=0;i<temptempfolders.GetCount();i++) {
			CString toadd=temptempfolders.GetAt(i);
			MakeFoldername(toadd.GetBuffer(MAX_PATH));
			toadd.ReleaseBuffer();
			if (!PathFileExists(toadd))
				CreateDirectory(toadd,NULL);
			if (PathFileExists(toadd))
				thePrefs.tempdir.Add(toadd);
		}
	}
	if (thePrefs.tempdir.IsEmpty())
		thePrefs.tempdir.Add(thePrefs.GetDefaultDirectory(EMULE_TEMPDIR, true));
*/
	// strIncomingDir
	thePrefs.m_strIncomingDir = strIncomingDir;
	MakeFoldername(thePrefs.m_strIncomingDir);
	thePrefs.GetCategory(0)->strIncomingPath = thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR);

	thePrefs.shareddir_list.RemoveAll();
	m_ShareSelector.GetSharedDirectories(&thePrefs.shareddir_list);
	for (int i = 0; i < m_ctlUncPaths.GetItemCount(); i++)
		thePrefs.shareddir_list.AddTail(m_ctlUncPaths.GetItemText(i, 0));

	// check shared directories for reserved folder names
	POSITION pos = thePrefs.shareddir_list.GetHeadPosition();
	while (pos){
		POSITION posLast = pos;
		const CString& rstrDir = thePrefs.shareddir_list.GetNext(pos);
		if (!thePrefs.IsShareableDirectory(rstrDir))
			thePrefs.shareddir_list.RemoveAt(posLast);
	}

/*
	if (testtempdirchanged)
		AfxMessageBox(GetResString(IDS_SETTINGCHANGED_RESTART));
*/
	
	// on changing incoming dir, update incoming dirs of category of the same path
	if (testincdirchanged.CompareNoCase(thePrefs.m_strIncomingDir)!=0) {
		CString oldpath;
		bool dontaskagain=false;
		for (int cat=1; cat<=thePrefs.GetCatCount()-1;cat++){
			oldpath=CString(thePrefs.GetCatPath(cat));
			if (oldpath.Left(testincdirchanged.GetLength()).CompareNoCase(testincdirchanged)==0) {

				if (!dontaskagain) {
					dontaskagain=true;
					if (AfxMessageBox(GetResString(IDS_UPDATECATINCOMINGDIRS),MB_YESNO)==IDNO)
						break;
				}
				thePrefs.GetCategory(cat)->strIncomingPath = thePrefs.m_strIncomingDir +  oldpath.Mid(testincdirchanged.GetLength());
			}
		}
		theApp.emuledlg->sharedfileswnd->Reload();
	}

	SetModified(0);
	return CPropertyPage::OnApply();
}

BOOL CPPgDirectories::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == UM_ITEMSTATECHANGED)
		SetModified();	
	else if (wParam == ID_HELP)
	{
		OnHelp();
		return TRUE;
	}
	return CPropertyPage::OnCommand(wParam, lParam);
}

void CPPgDirectories::Localize(void)
{
	if(m_hWnd)
	{
		SetWindowText(GetResString(IDS_PW_DIR));

		GetDlgItem(IDC_INCOMING_FRM)->SetWindowText(GetResString(IDS_PW_INCOMING));
		//GetDlgItem(IDC_TEMP_FRM)->SetWindowText(GetResString(IDS_PW_TEMP));
		GetDlgItem(IDC_SELINCDIR)->SetWindowText(GetResString(IDS_PW_BROWSE));
		//GetDlgItem(IDC_SELTEMPDIR)->SetWindowText(GetResString(IDS_PW_BROWSE));
		GetDlgItem(IDC_SHARED_FRM)->SetWindowText(GetResString(IDS_PW_SHARED));

		m_ttc.UpdateTipText(GetResString(IDS_TIP_SHARE_DIR), GetDlgItem(IDC_SHARESELECTOR));
		m_ttc.UpdateTipText(GetResString(IDS_TIP_UNC_PATH), GetDlgItem(IDC_UNCLIST));
		m_ttc.UpdateTipText(GetResString(IDS_TIP_INCOMING_FILES), GetDlgItem(IDC_INCFILES));

		

	}
}

void CPPgDirectories::FillUncList(void)
{
	m_ctlUncPaths.DeleteAllItems();

	for (POSITION pos = thePrefs.shareddir_list.GetHeadPosition(); pos != 0; )
	{
		CString folder = thePrefs.shareddir_list.GetNext(pos);
		if (PathIsUNC(folder))
			m_ctlUncPaths.InsertItem(0, folder);
	}
}

void CPPgDirectories::OnBnClickedAddUNC()
{
	InputBox inputbox;
	inputbox.SetLabels(GetResString(IDS_UNCFOLDERS), GetResString(IDS_UNCFOLDERS), _T("\\\\Server\\Share"));
	if (inputbox.DoModal() != IDOK)
		return;
	CString unc=inputbox.GetInput();

	// basic unc-check 
	if (!PathIsUNC(unc)){
		AfxMessageBox(GetResString(IDS_ERR_BADUNC), MB_ICONERROR);
		return;
	}

	if (unc.Right(1) == _T("\\"))
		unc.Delete(unc.GetLength()-1, 1);

	for (POSITION pos = thePrefs.shareddir_list.GetHeadPosition();pos != 0;){
		if (unc.CompareNoCase(thePrefs.shareddir_list.GetNext(pos))==0)
			return;
	}
	for (int posi = 0; posi < m_ctlUncPaths.GetItemCount(); posi++){
		if (unc.CompareNoCase(m_ctlUncPaths.GetItemText(posi, 0)) == 0)
			return;
	}

	m_ctlUncPaths.InsertItem(m_ctlUncPaths.GetItemCount(), unc);
	SetModified();
}

void CPPgDirectories::OnBnClickedRemUNC()
{
	int index = m_ctlUncPaths.GetSelectionMark();
	if (index == -1 || m_ctlUncPaths.GetSelectedCount() == 0)
		return;
	m_ctlUncPaths.DeleteItem(index);
	SetModified();
}

void CPPgDirectories::OnHelp()
{
	theApp.ShowHelp(eMule_FAQ_Preferences_Directories);
}

BOOL CPPgDirectories::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	OnHelp();
	return TRUE;
}

void CPPgDirectories::OnBnClickedSeltempdiradd()
{
	CString paths;
	//GetDlgItemText(IDC_TEMPFILES, paths);

	TCHAR buffer[MAX_PATH] = {0};
	//GetDlgItemText(IDC_TEMPFILES, buffer, ARRSIZE(buffer));

	if(SelectDir(GetSafeHwnd(),buffer,GetResString(IDS_SELECT_TEMPDIR))) {
		paths.Append(_T("|"));
		paths.Append(buffer);
		//SetDlgItemText(IDC_TEMPFILES, paths);
	}
}

void CPPgDirectories::OnBnClickedIncomingFrm()
	{
	// TODO: 在此添加控件通知处理程序代码
	}

void CPPgDirectories::OnTvnSelchangedShareselector(NMHDR * /*pNMHDR*/, LRESULT *pResult)
	{
	//LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	}

BOOL CPPgDirectories::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	m_ttc.RelayEvent(pMsg);

	return CPropertyPage::PreTranslateMessage(pMsg);
}
