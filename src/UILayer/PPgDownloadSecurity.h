/*
 * $Id: PPgDownloadSecurity.h 6544 2008-08-06 08:49:22Z linhai $
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
#pragma once
#include "afxwin.h"
#include "HypertextCtrl.h"
#include "AntiVirusIDs.h"

#define ANTIANRUS_MODEL		0
#define PROGRAM_MODEL		1

#define PDS_REMOTE	2
#define PDS_LOCAL	3

struct SAntiVirusProgs
{
	ANTIVIRUSID antivirusid;		//杀毒软件ID号
	LPCTSTR		pszRegKey;			//注册表项
	LPCTSTR		pszRegValPath;		//注册表键值(路径)
	LPCTSTR		pszFileName;		//杀毒软件文件名
	LPCTSTR     pszArguments;		//命令行参数
	BOOL		bSupported;			//是否可用
};

static SAntiVirusProgs _SAntiVirusProgs[] =
{
	{ANTIVIRUS_RISING, _T("SOFTWARE\\rising\\Rav"), _T("installpath"), _T("Rav.exe"),  NULL,FALSE},
	{ANTIVIRUS_KING, _T("SOFTWARE\\kingsoft\\antivirus"), _T("ProgramPath"), _T("KAV32.exe"),  _T("/m- /b- /z+ /ac /q /Silence"), FALSE},
	{ANTIVIRUS_KASPERSKY, NULL, NULL, NULL, FALSE},	
	{ANTIVIRUS_JIANGMIN, _T("SOFTWARE\\Jiangmin"), _T("InstallPath"), _T("AntiVirus\\kvxp.kxp"), NULL, FALSE},
	{ANTIVIRUS_360SD, NULL, NULL, NULL, FALSE},
	{ANTIVIRUS_BITDEFENDER, NULL, NULL, NULL, FALSE},
	{0, NULL, NULL, NULL, NULL, 0}
};

// CPPgDownloadSecurity 对话框

class CPPgDownloadSecurity : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgDownloadSecurity)

public:
	CPPgDownloadSecurity();
	virtual ~CPPgDownloadSecurity();

public:
	void Localize();

// 对话框数据
	enum { IDD = IDD_PPG_DOWNLOADSECURITY };

protected:
	UINT m_SelectAntiVirus;
	CHyperTextCtrl m_wndModelLink;
	bool m_bCreated;

public:
	void SetSelectAntiVirus(UINT in);
protected:
	void LoadSettings();
	bool Check(CString& strPath, CString& strArguments);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedEnablescanvirusCheck();
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	afx_msg void OnBnClickedSelprogram();
	afx_msg void OnBnClickedRadioAntivirusModel();
	afx_msg void OnBnClickedRadioAntianrusProgs();
	afx_msg void OnEnChangeEditFormat();
	afx_msg void OnEnChangeEditAntivirusArgs();
	afx_msg void OnBnClickedAutocheck();
	afx_msg void OnEnChangeEditAntivirus();
	CComboBox m_AntiVirusModelCtrl;
	afx_msg void OnCbnSelchangeComboAntivirusmodel();
	afx_msg void OnBnClickedEnablesharefilesCheck();
};
