// ScanDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "eMule.h"
#include "ScanDlg.h"
#include ".\scandlg.h"


// CScanDlg 对话框

IMPLEMENT_DYNAMIC(CScanDlg, CDialog)
CScanDlg::CScanDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CScanDlg::IDD, pParent)
{
	m_pParent = (CPPgDownloadSecurity*)pParent;
}

CScanDlg::~CScanDlg()
{
}

void CScanDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CScanDlg, CDialog)
END_MESSAGE_MAP()


// CScanDlg 消息处理程序

BOOL CScanDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	InitWindowStyles(this);

	LoadSettings();
	Localize();

	return TRUE; 
}

void CScanDlg::Localize()
{
	if(m_hWnd)
	{
		GetDlgItem(IDC_STATIC_TITLE)->SetWindowText(GetResString(IDS_SCAN_TITLE));
		GetDlgItem(IDC_RADIO_REMOTE)->SetWindowText(GetResString(IDS_SCAN_REMOTE));
		GetDlgItem(IDC_RADIO_LOCAL)->SetWindowText(GetResString(IDS_SCAN_LOCAL));

		//{begin} VC-dgkang 2008年7月15日
		GetDlgItem(IDOK)->SetWindowText(GetResString(IDS_SCAN_OK));
		GetDlgItem(IDCANCEL)->SetWindowText(GetResString(IDS_SCAN_CANCEL));
		//{end}
	}
}

void CScanDlg::LoadSettings()
{
	CheckDlgButton(IDC_RADIO_REMOTE, TRUE);
}

void CScanDlg::OnOK()
{
	
	if (IsDlgButtonChecked(IDC_RADIO_REMOTE))
	{
		m_pParent->SetSelectAntiVirus(PDS_REMOTE);
	}

	if (IsDlgButtonChecked(IDC_RADIO_LOCAL))
	{
		m_pParent->SetSelectAntiVirus(PDS_LOCAL);
	}

	CDialog::OnOK();
}
