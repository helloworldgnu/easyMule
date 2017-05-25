/* 
 * $Id: PShtWiz1.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
#include <afxinet.h>

#include "emule.h"
#include "enbitmap.h"
#include "OtherFunctions.h"
#include "Preferences.h"
#include "emuledlg.h"
#include "Statistics.h"
#include "ListenSocket.h"
#include "ClientUDPSocket.h"
#include "opcodes.h"
#include "UserMsgs.h"
#include "PShtWiz1.h"
#include "Wizard.h"
#include "../CxImage/xImage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


///////////////////////////////////////////////////////////////////////////////
// CDlgPageWizard dialog


IMPLEMENT_DYNCREATE(CDlgPageWizard, CPropertyPageEx)

BEGIN_MESSAGE_MAP(CDlgPageWizard, CPropertyPageEx)
END_MESSAGE_MAP()

CDlgPageWizard::CDlgPageWizard() 
	: CPropertyPageEx()
{
}

void CDlgPageWizard::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageEx::DoDataExchange(pDX);
}

BOOL CDlgPageWizard::OnSetActive() 
{
	CPropertySheetEx* pSheet = (CPropertySheetEx*)GetParent();
	if (pSheet->IsWizard())
	{
		int iPages = pSheet->GetPageCount();
		int iActPage = pSheet->GetActiveIndex();
		DWORD dwButtons = 0;
		if (iActPage > 0)
			dwButtons |= PSWIZB_BACK;
		if (iActPage < iPages)
			dwButtons |= PSWIZB_NEXT;
		if (iActPage == iPages-1)
		{
			if (pSheet->m_psh.dwFlags & PSH_WIZARDHASFINISH)
				dwButtons &= ~PSWIZB_NEXT;
			dwButtons |= PSWIZB_FINISH;
		}
		pSheet->SetWizardButtons(dwButtons);
	}
	return CPropertyPageEx::OnSetActive();
}


///////////////////////////////////////////////////////////////////////////////
// CPPgWiz1Welcome dialog

class CPPgWiz1Welcome : public CDlgPageWizard
{
	DECLARE_DYNAMIC(CPPgWiz1Welcome)

public:
	CPPgWiz1Welcome();
	CPPgWiz1Welcome(UINT nIDTemplate, LPCTSTR pszCaption = NULL, LPCTSTR pszHeaderTitle = NULL, LPCTSTR pszHeaderSubTitle = NULL)
		: CDlgPageWizard(nIDTemplate, pszCaption, pszHeaderTitle, pszHeaderSubTitle)
	{
	}
	virtual ~CPPgWiz1Welcome();
	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_WIZ1_WELCOME };

protected:
	CFont m_FontTitle;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};

IMPLEMENT_DYNAMIC(CPPgWiz1Welcome, CDlgPageWizard)

BEGIN_MESSAGE_MAP(CPPgWiz1Welcome, CDlgPageWizard)
END_MESSAGE_MAP()

CPPgWiz1Welcome::CPPgWiz1Welcome()
	: CDlgPageWizard(CPPgWiz1Welcome::IDD)
{
}

CPPgWiz1Welcome::~CPPgWiz1Welcome()
{
}

void CPPgWiz1Welcome::DoDataExchange(CDataExchange* pDX)
{
	CDlgPageWizard::DoDataExchange(pDX);
}

BOOL CPPgWiz1Welcome::OnInitDialog()
{
	CFont fontVerdanaBold;
	fontVerdanaBold.CreatePointFont(120, _T("Verdana Bold"));
	LOGFONT lf;
	fontVerdanaBold.GetLogFont(&lf);
	lf.lfWeight = FW_BOLD;
	m_FontTitle.CreateFontIndirect(&lf);

	CStatic* pStatic = (CStatic*)GetDlgItem(IDC_WIZ1_TITLE);
	pStatic->SetFont(&m_FontTitle);

	CDlgPageWizard::OnInitDialog();
	InitWindowStyles(this);
	GetDlgItem(IDC_WIZ1_TITLE)->SetWindowText(GetResString(IDS_WIZ1_WELCOME_TITLE));
	GetDlgItem(IDC_WIZ1_ACTIONS)->SetWindowText(GetResString(IDS_WIZ1_WELCOME_ACTIONS));
	GetDlgItem(IDC_WIZ1_BTN_HINT)->SetWindowText(GetResString(IDS_WIZ1_WELCOME_BTN_HINT));
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// CPPgWiz1General dialog

class CPPgWiz1General : public CDlgPageWizard
{
	DECLARE_DYNAMIC(CPPgWiz1General)

public:
	CPPgWiz1General();
	CPPgWiz1General(UINT nIDTemplate, LPCTSTR pszCaption = NULL, LPCTSTR pszHeaderTitle = NULL, LPCTSTR pszHeaderSubTitle = NULL)
		: CDlgPageWizard(nIDTemplate, pszCaption, pszHeaderTitle, pszHeaderSubTitle)
	{
		//Chocobo Start
		//第一次运行向导，modified by Chocobo on 2006.08.02
		//默认自动连接、自动启动及浏览器选项为选中
		m_iAutoConnectAtStart = 1;
		m_iAutoStart          = thePrefs.GetAutoStart();
		m_iWebbrowser         = 1;
	}
	virtual ~CPPgWiz1General();
	virtual BOOL OnInitDialog();

	
	virtual LRESULT OnWizardNext(){CDlgPageWizard::OnWizardNext(); return IDD_WIZ1_DIR;}	//ADDED by VC-fengwen on 2007/07/17  : 越过port设置

// Dialog Data
	enum { IDD = IDD_WIZ1_GENERAL };

	CString m_strNick;
	int m_iAutoConnectAtStart;
	int m_iAutoStart;
	//Chocobo Start
	//第一次运行向导，added by Chocobo on 2006.08.02
	//浏览器选项
	int m_iWebbrowser;
	//Chocobo End;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};

IMPLEMENT_DYNAMIC(CPPgWiz1General, CDlgPageWizard)

BEGIN_MESSAGE_MAP(CPPgWiz1General, CDlgPageWizard)
END_MESSAGE_MAP()

CPPgWiz1General::CPPgWiz1General()
	: CDlgPageWizard(CPPgWiz1General::IDD)
{
	//Chocobo Start
	//第一次运行向导，modified by Chocobo on 2006.08.02
	//默认自动连接、自动启动选项为选中
	m_iAutoConnectAtStart = 1;
	m_iAutoStart = 1;
	//Chocobo End
}

CPPgWiz1General::~CPPgWiz1General()
{
}

void CPPgWiz1General::DoDataExchange(CDataExchange* pDX)
{
	CDlgPageWizard::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_NICK, m_strNick);
	//DDX_Check(pDX, IDC_AUTOCONNECT, m_iAutoConnectAtStart);
	DDX_Check(pDX, IDC_AUTOSTART, m_iAutoStart);
	//Chocobo Start
	//第一次运行向导，added by Chocobo on 2006.08.02
	//浏览器选项
	//DDX_Check(pDX, IDC_USEWEBBROWSER, m_iWebbrowser); // Added by GGSoSo for webbrowser
	//Chocobo End
}

BOOL CPPgWiz1General::OnInitDialog()
{
	CDlgPageWizard::OnInitDialog();
	InitWindowStyles(this);
	((CEdit*)GetDlgItem(IDC_NICK))->SetLimitText(thePrefs.GetMaxUserNickLength());
	GetDlgItem(IDC_NICK_FRM)->SetWindowText(GetResString(IDS_ENTERUSERNAME));
	//GetDlgItem(IDC_AUTOCONNECT)->SetWindowText(GetResString(IDS_FIRSTAUTOCON));
	GetDlgItem(IDC_AUTOSTART)->SetWindowText(GetResString(IDS_WIZ_STARTWITHWINDOWS));
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// CPPgWiz1Ports & Connections test dialog

class CPPgWiz1Ports : public CDlgPageWizard
{
	DECLARE_DYNAMIC(CPPgWiz1Ports)

public:
	CPPgWiz1Ports();
	CPPgWiz1Ports(UINT nIDTemplate, LPCTSTR pszCaption = NULL, LPCTSTR pszHeaderTitle = NULL, LPCTSTR pszHeaderSubTitle = NULL)
		: CDlgPageWizard(nIDTemplate, pszCaption, pszHeaderTitle, pszHeaderSubTitle)
	{
	}

	void ValidateShownPorts();

	virtual ~CPPgWiz1Ports();
	virtual BOOL OnInitDialog();
	afx_msg void OnStartConTest();
	afx_msg void OnEnChangeUDPDisable();

	afx_msg void OnEnChangeUDP();
	afx_msg void OnEnChangeTCP();
	//Chocobo Start
	//第一次运行向导，added by Chocobo on 2006.08.02
	afx_msg void OnBnClickedRandomPort(); // 处理随机端口 --GGSoSo
	//Chocobo End
	void OnPortChange();

	CString m_sTestURL,m_sUDP,m_sTCP;
	uint16 GetTCPPort();
	uint16 GetUDPPort();

	//virtual LRESULT OnWizardNext(){CDlgPageWizard::OnWizardNext(); return IDD_WIZ1_END;}	//ADDED by fengwen on 2006/09/06 : 只留1，2步。
	virtual LRESULT OnWizardNext(){CDlgPageWizard::OnWizardNext(); return IDD_WIZ1_DIR;}	//MODIFIED by fengwen on 2006/11/28 : 增加对存放数据的目录的修改。
	

// Dialog Data
	enum { IDD = IDD_WIZ1_PORTS };

protected:
	CString lastudp;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};

IMPLEMENT_DYNAMIC(CPPgWiz1Ports, CDlgPageWizard)

BEGIN_MESSAGE_MAP(CPPgWiz1Ports, CDlgPageWizard)
	//ON_BN_CLICKED(IDC_STARTTEST, OnStartConTest)
	//ON_BN_CLICKED(IDC_UDPDISABLE, OnEnChangeUDPDisable)
	//Chocobo Start
	//第一次运行向导，added by Chocobo on 2006.08.02
	ON_BN_CLICKED(IDC_RANDOM_PORT, OnBnClickedRandomPort) // 随机端口按钮 --GGSoSo
	//Chocobo End
	ON_EN_CHANGE(IDC_TCP, OnEnChangeTCP)
	ON_EN_CHANGE(IDC_UDP, OnEnChangeUDP)
END_MESSAGE_MAP()

CPPgWiz1Ports::CPPgWiz1Ports()
	: CDlgPageWizard(CPPgWiz1Ports::IDD)
{
}

CPPgWiz1Ports::~CPPgWiz1Ports()
{
}

void CPPgWiz1Ports::DoDataExchange(CDataExchange* pDX)
{
	CDlgPageWizard::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_TCP, m_sTCP);
	DDX_Text(pDX, IDC_UDP, m_sUDP);
}

void CPPgWiz1Ports::OnEnChangeTCP() {
	OnPortChange();
}
void CPPgWiz1Ports::OnEnChangeUDP() {
	OnPortChange();
}

uint16 CPPgWiz1Ports::GetTCPPort() {
	CString buffer;

	GetDlgItem(IDC_TCP)->GetWindowText(buffer);
	return (uint16)_tstoi(buffer);
}

uint16 CPPgWiz1Ports::GetUDPPort() {
	uint16 udp=0;
	CString buffer;

	//if (IsDlgButtonChecked(IDC_UDPDISABLE)==0) {
	//	GetDlgItem(IDC_UDP)->GetWindowText(buffer);
	//	udp = (uint16)_tstoi(buffer);
	//}
	return udp;
}

void CPPgWiz1Ports::OnPortChange() {
	
	// VC-linhai[2007-08-07]: "flag" : 局部变量已初始化但不引用
	//bool flag= (theApp.IsPortchangeAllowed() && 
	//	( 
	//	(CGlobalVariable::listensocket->GetConnectedPort()!=GetTCPPort()  || CGlobalVariable::listensocket->GetConnectedPort()==0)
	//	||
	//	(CGlobalVariable::clientudp->GetConnectedPort()!=GetUDPPort() || CGlobalVariable::clientudp->GetConnectedPort()==0 )    
	//	)	
	//);

	//GetDlgItem(IDC_STARTTEST)->EnableWindow(flag);
}

void CPPgWiz1Ports::OnStartConTest() {

	uint16 tcp=GetTCPPort();
	uint16 udp=GetUDPPort();

	if (tcp==0)
		return;

	if ( (tcp!=CGlobalVariable::listensocket->GetConnectedPort() || udp!=CGlobalVariable::clientudp->GetConnectedPort() ) ) {

		if (!theApp.IsPortchangeAllowed()) {
			//AfxMessageBox(GetResString(IDS_NOPORTCHANGEPOSSIBLE));
			MessageBox(GetResString(IDS_NOPORTCHANGEPOSSIBLE),GetResString(IDS_CAPTION),MB_OK);
			return;
		}

		// set new ports
		thePrefs.port=tcp;
		thePrefs.udpport=udp;

		CGlobalVariable::listensocket->Rebind() ;
		CGlobalVariable::clientudp->Rebind();
	}

	TriggerPortTest(tcp,udp);
}


BOOL CPPgWiz1Ports::OnInitDialog()
{
	CDlgPageWizard::OnInitDialog();
	//CheckDlgButton(IDC_UDPDISABLE, m_sUDP.IsEmpty() || m_sUDP == _T("0"));
	//GetDlgItem(IDC_UDP)->EnableWindow(IsDlgButtonChecked(IDC_UDPDISABLE) == 0);
	InitWindowStyles(this);
	
	lastudp = m_sUDP;

	// disable changing ports to prevent harm
	SetDlgItemText(IDC_PORTINFO , GetResString(IDS_PORTINFO) );
	SetDlgItemText(IDC_TESTFRAME , GetResString(IDS_CONNECTIONTEST) );
	SetDlgItemText(IDC_TESTINFO , GetResString(IDS_TESTINFO) );
	//SetDlgItemText(IDC_STARTTEST, GetResString(IDS_STARTTEST) );
	//SetDlgItemText(IDC_UDPDISABLE, GetResString(IDS_UDPDISABLED));
	//Chocobo Start
	//第一次运行向导，added by Chocobo on 2006.08.02
	//随机端口按钮文字
	SetDlgItemText(IDC_RANDOM_PORT, GetResString(IDS_RANDOMPORT));
	//Chocobo End

	return TRUE;
}

void CPPgWiz1Ports::OnEnChangeUDPDisable()
{
	//bool disabled = IsDlgButtonChecked(IDC_UDPDISABLE)!=0;
	//GetDlgItem(IDC_UDP)->EnableWindow(!disabled);
	
	//if (disabled) {
	//	GetDlgItemText(IDC_UDP, lastudp);
	//	GetDlgItem(IDC_UDP)->SetWindowText(_T("0"));
	//}
	//else
	//	GetDlgItem(IDC_UDP)->SetWindowText(lastudp);
	
	OnPortChange();
}

//Chocobo Start
//第一次运行向导，added by Chocobo on 2006.08.02
//随机端口处理
void CPPgWiz1Ports::OnBnClickedRandomPort() // 处理随机端口 --GGSoSo
{
	uint16 port = SafeRandomPort();
	CString szText;
	szText.Format(_T("%d"),port);
	GetDlgItem(IDC_TCP)->SetWindowText(szText);
	szText.Format(_T("%d"),port+10);
	GetDlgItem(IDC_UDP)->SetWindowText(szText);
}
//Chocobo End


///////////////////////////////////////////////////////////////////////////////
// CPPgWiz1UlPrio dialog

class CPPgWiz1UlPrio : public CDlgPageWizard
{
	DECLARE_DYNAMIC(CPPgWiz1UlPrio)

public:
	CPPgWiz1UlPrio();
	CPPgWiz1UlPrio(UINT nIDTemplate, LPCTSTR pszCaption = NULL, LPCTSTR pszHeaderTitle = NULL, LPCTSTR pszHeaderSubTitle = NULL)
		: CDlgPageWizard(nIDTemplate, pszCaption, pszHeaderTitle, pszHeaderSubTitle)
	{
		m_iUAP = 1;
		m_iDAP = 1;
	}
	virtual ~CPPgWiz1UlPrio();
	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_WIZ1_ULDL_PRIO };

	int m_iUAP;
	int m_iDAP;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};

IMPLEMENT_DYNAMIC(CPPgWiz1UlPrio, CDlgPageWizard)

BEGIN_MESSAGE_MAP(CPPgWiz1UlPrio, CDlgPageWizard)
END_MESSAGE_MAP()

CPPgWiz1UlPrio::CPPgWiz1UlPrio()
	: CDlgPageWizard(CPPgWiz1UlPrio::IDD)
{
	m_iUAP = 1;
	m_iDAP = 1;
}

CPPgWiz1UlPrio::~CPPgWiz1UlPrio()
{
}

void CPPgWiz1UlPrio::DoDataExchange(CDataExchange* pDX)
{
	CDlgPageWizard::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_UAP, m_iUAP);
	DDX_Check(pDX, IDC_DAP, m_iDAP);
}

BOOL CPPgWiz1UlPrio::OnInitDialog()
{
	CDlgPageWizard::OnInitDialog();
	InitWindowStyles(this);
	GetDlgItem(IDC_UAP)->SetWindowText(GetResString(IDS_FIRSTAUTOUP));
	GetDlgItem(IDC_DAP)->SetWindowText(GetResString(IDS_FIRSTAUTODOWN));

	return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// CPPgWiz1Upload dialog

class CPPgWiz1Upload : public CDlgPageWizard
{
	DECLARE_DYNAMIC(CPPgWiz1Upload)

public:
	CPPgWiz1Upload();
	CPPgWiz1Upload(UINT nIDTemplate, LPCTSTR pszCaption = NULL, LPCTSTR pszHeaderTitle = NULL, LPCTSTR pszHeaderSubTitle = NULL)
		: CDlgPageWizard(nIDTemplate, pszCaption, pszHeaderTitle, pszHeaderSubTitle)
	{
		m_iULFullChunks = 1;
	}
	virtual ~CPPgWiz1Upload();
	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_WIZ1_UPLOAD };

	int m_iULFullChunks;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};

IMPLEMENT_DYNAMIC(CPPgWiz1Upload, CDlgPageWizard)

BEGIN_MESSAGE_MAP(CPPgWiz1Upload, CDlgPageWizard)
END_MESSAGE_MAP()

CPPgWiz1Upload::CPPgWiz1Upload()
	: CDlgPageWizard(CPPgWiz1Upload::IDD)
{
	m_iULFullChunks = 1;
}

CPPgWiz1Upload::~CPPgWiz1Upload()
{
}

void CPPgWiz1Upload::DoDataExchange(CDataExchange* pDX)
{
	CDlgPageWizard::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_FULLCHUNKTRANS, m_iULFullChunks);
}

BOOL CPPgWiz1Upload::OnInitDialog()
{
	CDlgPageWizard::OnInitDialog();
	InitWindowStyles(this);
	GetDlgItem(IDC_FULLCHUNKTRANS)->SetWindowText(GetResString(IDS_FIRSTFULLCHUNK));
	return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// CPPgWiz1Server dialog

class CPPgWiz1Server : public CDlgPageWizard
{
	DECLARE_DYNAMIC(CPPgWiz1Server)

public:
	CPPgWiz1Server();
	CPPgWiz1Server(UINT nIDTemplate, LPCTSTR pszCaption = NULL, LPCTSTR pszHeaderTitle = NULL, LPCTSTR pszHeaderSubTitle = NULL)
		: CDlgPageWizard(nIDTemplate, pszCaption, pszHeaderTitle, pszHeaderSubTitle)
	{
		m_iSafeServerConnect = 0;
		m_iKademlia = 1;
		m_iED2K = 1;
	}
	virtual ~CPPgWiz1Server();
	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_WIZ1_SERVER };

	int m_iSafeServerConnect;
	int m_iKademlia;
	int m_iED2K;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};

IMPLEMENT_DYNAMIC(CPPgWiz1Server, CDlgPageWizard)

BEGIN_MESSAGE_MAP(CPPgWiz1Server, CDlgPageWizard)
END_MESSAGE_MAP()

CPPgWiz1Server::CPPgWiz1Server()
	: CDlgPageWizard(CPPgWiz1Server::IDD)
{
	m_iSafeServerConnect = 0;
	m_iKademlia = 1;
	m_iED2K = 1;
}

CPPgWiz1Server::~CPPgWiz1Server()
{
}

void CPPgWiz1Server::DoDataExchange(CDataExchange* pDX)
{
	CDlgPageWizard::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_SAFESERVERCONNECT, m_iSafeServerConnect);
	DDX_Check(pDX, IDC_WIZARD_NETWORK_KADEMLIA, m_iKademlia);
	DDX_Check(pDX, IDC_WIZARD_NETWORK_ED2K, m_iED2K);
}

BOOL CPPgWiz1Server::OnInitDialog()
{
	CDlgPageWizard::OnInitDialog();
	InitWindowStyles(this);
	GetDlgItem(IDC_SAFESERVERCONNECT)->SetWindowText(GetResString(IDS_FIRSTSAFECON));
	GetDlgItem(IDC_WIZARD_NETWORK)->SetWindowText(GetResString(IDS_WIZARD_NETWORK));
	GetDlgItem(IDC_WIZARD_ED2K)->SetWindowText(GetResString(IDS_WIZARD_ED2K));
	return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// CPPgWiz1End dialog

class CPPgWiz1End : public CDlgPageWizard
{
	DECLARE_DYNAMIC(CPPgWiz1End)

public:
	CPPgWiz1End();
	CPPgWiz1End(UINT nIDTemplate, LPCTSTR pszCaption = NULL, LPCTSTR pszHeaderTitle = NULL, LPCTSTR pszHeaderSubTitle = NULL)
		: CDlgPageWizard(nIDTemplate, pszCaption, pszHeaderTitle, pszHeaderSubTitle)
	{
	}
	virtual ~CPPgWiz1End();
	virtual BOOL OnInitDialog();

	//COMMENTED by fengwen on 2006/11/28 : 增加对存放数据的目录的修改。
	//virtual LRESULT OnWizardBack(){CDlgPageWizard::OnWizardBack(); return IDD_WIZ1_PORTS;}	//ADDED by fengwen on 2006/09/06 : 只留1，2步。

// Dialog Data
	enum { IDD = IDD_WIZ1_END };

protected:
	CFont m_FontTitle;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};

IMPLEMENT_DYNAMIC(CPPgWiz1End, CDlgPageWizard)

BEGIN_MESSAGE_MAP(CPPgWiz1End, CDlgPageWizard)
END_MESSAGE_MAP()

CPPgWiz1End::CPPgWiz1End()
	: CDlgPageWizard(CPPgWiz1End::IDD)
{
}

CPPgWiz1End::~CPPgWiz1End()
{
}

void CPPgWiz1End::DoDataExchange(CDataExchange* pDX)
{
	CDlgPageWizard::DoDataExchange(pDX);
}

BOOL CPPgWiz1End::OnInitDialog()
{
	CFont fontVerdanaBold;
	fontVerdanaBold.CreatePointFont(120, _T("Verdana Bold"));
	LOGFONT lf;
	fontVerdanaBold.GetLogFont(&lf);
	lf.lfWeight = FW_BOLD;
	m_FontTitle.CreateFontIndirect(&lf);

	CStatic* pStatic = (CStatic*)GetDlgItem(IDC_WIZ1_TITLE);
	pStatic->SetFont(&m_FontTitle);

	CDlgPageWizard::OnInitDialog();
	InitWindowStyles(this);
	GetDlgItem(IDC_WIZ1_TITLE)->SetWindowText(GetResString(IDS_WIZ1_END_TITLE));
	GetDlgItem(IDC_WIZ1_ACTIONS)->SetWindowText(GetResString(IDS_FIRSTCOMPLETE));
	GetDlgItem(IDC_WIZ1_BTN_HINT)->SetWindowText(GetResString(IDS_WIZ1_END_BTN_HINT));

	return TRUE;
}

class CPPgWiz1Dir : public CDlgPageWizard
{
	DECLARE_DYNAMIC(CPPgWiz1Dir)

public:
	CPPgWiz1Dir();
	CPPgWiz1Dir(UINT nIDTemplate, LPCTSTR pszCaption = NULL, LPCTSTR pszHeaderTitle = NULL, LPCTSTR pszHeaderSubTitle = NULL)
		: CDlgPageWizard(nIDTemplate, pszCaption, pszHeaderTitle, pszHeaderSubTitle)
	{
	}
	virtual ~CPPgWiz1Dir();

	// 对话框数据
	enum { IDD = IDD_WIZ1_DIR };

	virtual BOOL OnInitDialog();
	//MODIFIED by VC-fengwen on 2007/07/17 <begin>	:	不显示port设置
	//virtual LRESULT OnWizardBack(){CDlgPageWizard::OnWizardBack(); return IDD_WIZ1_PORTS;}
	virtual LRESULT OnWizardBack(){CDlgPageWizard::OnWizardBack(); return IDD_WIZ1_GENERAL;}
	//MODIFIED by VC-fengwen on 2007/07/17 <end>	:	不显示port设置
	
	virtual LRESULT OnWizardNext();

	CString		m_strIncoming;
	CString		m_strTemp;
	BOOL		m_bImportUnfinishedTasks;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedSelincdir();
	afx_msg void OnBnClickedSeltempdir();
	afx_msg void OnBnClickedSeltempdiradd();
};

IMPLEMENT_DYNAMIC(CPPgWiz1Dir, CDlgPageWizard)
CPPgWiz1Dir::CPPgWiz1Dir()
: CDlgPageWizard(CPPgWiz1Dir::IDD)
{
	m_bImportUnfinishedTasks = TRUE;
}

CPPgWiz1Dir::~CPPgWiz1Dir()
{
}

void CPPgWiz1Dir::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_IMPORT, m_bImportUnfinishedTasks);
}


BEGIN_MESSAGE_MAP(CPPgWiz1Dir, CDlgPageWizard)
	//ON_BN_CLICKED(IDC_SELTEMPDIR, OnBnClickedSeltempdir)
	ON_BN_CLICKED(IDC_SELINCDIR,OnBnClickedSelincdir)
	//ON_BN_CLICKED(IDC_SELTEMPDIRADD, OnBnClickedSeltempdiradd)
END_MESSAGE_MAP()

BOOL CPPgWiz1Dir::OnInitDialog()
{
	CDlgPageWizard::OnInitDialog();

	((CEdit*)GetDlgItem(IDC_INCFILES))->SetLimitText(509);
	//((CEdit*)GetDlgItem(IDC_TEMPFILES))->SetLimitText(509);

	GetDlgItem(IDC_INCFILES)->SetWindowText(thePrefs.m_strIncomingDir);
	m_strIncoming = thePrefs.m_strIncomingDir;

	CString tempfolders;
	for (int i=0;i<thePrefs.tempdir.GetCount();i++) {
		tempfolders.Append(thePrefs.GetTempDir(i));
		if (i+1<thePrefs.tempdir.GetCount())
			tempfolders.Append(_T("|") );
	}
	//GetDlgItem(IDC_TEMPFILES)->SetWindowText(tempfolders);
	//m_strTemp = tempfolders;


	GetDlgItem(IDC_INCOMING_FRM)->SetWindowText(GetResString(IDS_WIZ1_DIR_INC_TITLE));
	//GetDlgItem(IDC_TEMP_FRM)->SetWindowText(GetResString(IDS_WIZ1_DIR_TMP_TITLE));
	GetDlgItem(IDC_STATIC_INCM_DESC)->SetWindowText(GetResString(IDS_WIZ1_DIR_INC_DESC));
	//GetDlgItem(IDC_STATIC_TMP_DESC)->SetWindowText(GetResString(IDS_WIZ1_DIR_TMP_DESC));
	GetDlgItem(IDC_SELINCDIR)->SetWindowText(GetResString(IDS_PW_BROWSE));
	//GetDlgItem(IDC_SELTEMPDIR)->SetWindowText(GetResString(IDS_PW_BROWSE));
	/*GetDlgItem(IDC_CHECK_IMPORT)->SetWindowText(GetResString(IDS_WZ1_IMPT_IN_FOLDER));*/
    CheckDlgButton(IDC_CHECK_IMPORT,1);



	return TRUE;
}

LRESULT CPPgWiz1Dir::OnWizardNext()
{
//	bool testtempdirchanged=false;
	CString testincdirchanged=thePrefs.m_strIncomingDir;

	CString strIncomingDir;
	GetDlgItemText(IDC_INCFILES, strIncomingDir);
	if (strIncomingDir.IsEmpty()){
		strIncomingDir = thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR);
		SetDlgItemText(IDC_INCFILES, strIncomingDir);
	}
	if (thePrefs.IsInstallationDirectory(strIncomingDir)){
		AfxMessageBox(GetResString(IDS_WRN_INCFILE_RESERVED));
		return -1;
	}

	// checking specified tempdir(s)
	//CString strTempDir;
	//GetDlgItemText(IDC_TEMPFILES, strTempDir);
	//if (strTempDir.IsEmpty()){
		//strTempDir = thePrefs.GetMuleDirectory(EMULE_EXECUTEABLEDIR) + _T("temp");
		//SetDlgItemText(IDC_TEMPFILES, strTempDir);
	//}

//	int curPos=0;
	CStringArray temptempfolders;
	//CString atmp=strTempDir.Tokenize(_T("|"), curPos);
	//while (!atmp.IsEmpty())
	//{
	//	atmp.Trim();
	//	if (!atmp.IsEmpty()) {
	//		if (CompareDirectories(strIncomingDir, atmp)==0){
	//			AfxMessageBox(GetResString(IDS_WRN_INCTEMP_SAME));
	//			return -1;
	//		}	
	//		if (thePrefs.IsInstallationDirectory(atmp)){
	//			AfxMessageBox(GetResString(IDS_WRN_TEMPFILES_RESERVED));
	//			return -1;
	//		}
	//		bool doubled=false;
	//		for (int i=0;i<temptempfolders.GetCount();i++)	// avoid double tempdirs
	//			if (temptempfolders.GetAt(i).CompareNoCase(atmp)==0) {
	//				doubled=true;
	//				break;
	//			}
	//			if (!doubled) {
	//				temptempfolders.Add(atmp);
	//				if (thePrefs.tempdir.GetCount()>=temptempfolders.GetCount()) {
	//					if( atmp.CompareNoCase(thePrefs.GetTempDir(temptempfolders.GetCount()-1))!=0	)
	//						testtempdirchanged=true;
	//				} else testtempdirchanged=true;

	//			}
	//	}
	//	atmp = strTempDir.Tokenize(_T("|"), curPos);
	//}
	
	GetDlgItemText(IDC_INCFILES, m_strIncoming);
	//GetDlgItemText(IDC_TEMPFILES, m_strTemp);

	return 0;
}

// CPPgWiz1Dir 消息处理程序
void CPPgWiz1Dir::OnBnClickedSelincdir()
{
	TCHAR buffer[MAX_PATH] = {0};
	GetDlgItemText(IDC_INCFILES, buffer, ARRSIZE(buffer));
	if(SelectDir(GetSafeHwnd(),buffer,GetResString(IDS_SELECT_INCOMINGDIR)))
		GetDlgItem(IDC_INCFILES)->SetWindowText(buffer);
	
}

void CPPgWiz1Dir::OnBnClickedSeltempdir()
{
//	TCHAR buffer[MAX_PATH] = {0};
	//GetDlgItemText(IDC_TEMPFILES, buffer, ARRSIZE(buffer));
	//if(SelectDir(GetSafeHwnd(),buffer,GetResString(IDS_SELECT_TEMPDIR)))
		//GetDlgItem(IDC_TEMPFILES)->SetWindowText(buffer);
}

void CPPgWiz1Dir::OnBnClickedSeltempdiradd()
{
	CString paths;
	//GetDlgItemText(IDC_TEMPFILES, paths);

	TCHAR buffer[MAX_PATH] = {0};

	if(SelectDir(GetSafeHwnd(),buffer,GetResString(IDS_SELECT_TEMPDIR))) {
		paths.Append(_T("|"));
		paths.Append(buffer);
		//SetDlgItemText(IDC_TEMPFILES, paths);
	}
}


///////////////////////////////////////////////////////////////////////////////
// CPShtWiz1

class CPShtWiz1 : public CPropertySheetEx
{
	DECLARE_DYNAMIC(CPShtWiz1)

public:
	CPShtWiz1(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	virtual ~CPShtWiz1();

protected:
	DECLARE_MESSAGE_MAP()
};

IMPLEMENT_DYNAMIC(CPShtWiz1, CPropertySheetEx)

BEGIN_MESSAGE_MAP(CPShtWiz1, CPropertySheetEx)
END_MESSAGE_MAP()

CPShtWiz1::CPShtWiz1(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheetEx(nIDCaption, pParentWnd, iSelectPage)
{
}

CPShtWiz1::~CPShtWiz1()
{
}

//ADDED by fengwen on 2006/11/28	<begin> : 设置数据存储目录
BOOL ApplyDirSet(CPPgWiz1Dir &page)
{
// 	bool testtempdirchanged=false;
	CString testincdirchanged = thePrefs.m_strIncomingDir;

	CString strIncomingDir;
	strIncomingDir = page.m_strIncoming;
	if (strIncomingDir.IsEmpty()){
		strIncomingDir = thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR);
	}
	if (thePrefs.IsInstallationDirectory(strIncomingDir)){
		AfxMessageBox(GetResString(IDS_WRN_INCFILE_RESERVED));
		return FALSE;
	}

	// checking specified tempdir(s)
/*
	CString strTempDir;
	strTempDir = page.m_strTemp;
	if (strTempDir.IsEmpty()){
		strTempDir = thePrefs.GetTempDir();
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
		temptempfolders.Add(strTempDir = thePrefs.GetMuleDirectory(EMULE_TEMPDIR));

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
		thePrefs.tempdir.Add(thePrefs.GetMuleDirectory(EMULE_TEMPDIR));
*/

	// strIncomingDir
	thePrefs.m_strIncomingDir = strIncomingDir;
	MakeFoldername(thePrefs.m_strIncomingDir);
	thePrefs.GetCategory(0)->strIncomingPath = thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR);


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
					//在第一次启动向导中，默认为yes
					//if (AfxMessageBox(GetResString(IDS_UPDATECATINCOMINGDIRS),MB_YESNO)==IDNO)
					//	break;
				}
				thePrefs.GetCategory(cat)->strIncomingPath = thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR) + oldpath.Mid(testincdirchanged.GetLength());
			}
		}
	}

	if (page.m_bImportUnfinishedTasks)
		CGlobalVariable::downloadqueue->ScanPartFile(page.m_strIncoming);

	return TRUE;
}
//ADDED by fengwen on 2006/11/28	<end> : 设置数据存储目录

BOOL FirstTimeWizard()
{

	CEnBitmap bmWatermark;
	VERIFY(bmWatermark.LoadBitmap(IDB_WIZ1_WATERMARK));

	CEnBitmap bmHeader;
	VERIFY( bmHeader.LoadImage(IDR_WIZ1_HEADER, _T("GIF"), NULL, GetSysColor(COLOR_WINDOW)) );
	CPropertySheetEx sheet(GetResString(IDS_WIZ1), NULL, 0, bmWatermark, NULL, bmHeader);
	sheet.m_psh.dwFlags |= PSH_WIZARD;
#ifdef _DEBUG
	sheet.m_psh.dwFlags |= PSH_WIZARDHASFINISH;
#endif
	sheet.m_psh.dwFlags |= PSH_WIZARD97;

	CPPgWiz1Welcome	page1(IDD_WIZ1_WELCOME, GetResString(IDS_WIZ1));
	page1.m_psp.dwFlags |= PSP_HIDEHEADER;
	sheet.AddPage(&page1);

	CPPgWiz1General page2(IDD_WIZ1_GENERAL, GetResString(IDS_WIZ1), GetResString(IDS_PW_GENERAL), GetResString(IDS_QL_USERNAME));
	sheet.AddPage(&page2);

	CPPgWiz1Ports page3(IDD_WIZ1_PORTS, GetResString(IDS_WIZ1), GetResString(IDS_PORTSCON), GetResString(IDS_PW_CONNECTION));
	sheet.AddPage(&page3);
	
	CPPgWiz1UlPrio page4(IDD_WIZ1_ULDL_PRIO, GetResString(IDS_WIZ1), GetResString(IDS_PW_CON_DOWNLBL) + _T(" / ") + GetResString(IDS_PW_CON_UPLBL), GetResString(IDS_PRIORITY));
	sheet.AddPage(&page4);
	
	CPPgWiz1Upload page5(IDD_WIZ1_UPLOAD, GetResString(IDS_WIZ1), GetResString(IDS_PW_CON_UPLBL), GetResString(IDS_WIZ1_UPLOAD_SUBTITLE));
	sheet.AddPage(&page5);
	
	CPPgWiz1Server page6(IDD_WIZ1_SERVER, GetResString(IDS_WIZ1), GetResString(IDS_PW_SERVER), GetResString(IDS_NETWORK));
	sheet.AddPage(&page6);

	CPPgWiz1Dir page8(IDD_WIZ1_DIR, GetResString(IDS_WIZ1), GetResString(IDS_PW_DIR), GetResString(IDS_WIZ1_DIR_SUBTITLE));
	sheet.AddPage(&page8);

	//ADDED by VC-fengwen on 2007/07/17 <begin>	:	把CConnectionWizardDlg合并到向导中来。
	CConnectionWizardDlg page9(GetResString(IDS_WIZ1), GetResString(IDS_PW_CONNECTION), GetResString(IDS_PW_CONNECTION));
	sheet.AddPage(&page9);
	//ADDED by VC-fengwen on 2007/07/17 <end>	:	把CConnectionWizardDlg合并到向导中来。

	
	CPPgWiz1End page7(IDD_WIZ1_END, GetResString(IDS_WIZ1));
	page7.m_psp.dwFlags |= PSP_HIDEHEADER;
	sheet.AddPage(&page7);

	page2.m_strNick = thePrefs.GetUserNick();
	if (page2.m_strNick.IsEmpty())
		page2.m_strNick = DEFAULT_NICK;
	//Chocobo Start
	//第一次运行向导，added by Chocobo on 2006.08.02
	page2.m_iAutoConnectAtStart = 1;
	page2.m_iWebbrowser = 1; // Added by GGSoSo for webbrowser
	//Chocobo End
	page3.m_sTCP.Format(_T("%u"), thePrefs.GetPort());
	page3.m_sUDP.Format(_T("%u"), thePrefs.GetUDPPort());
	page4.m_iDAP = 1;
	page4.m_iUAP = 1;
	page5.m_iULFullChunks = 1;
	page6.m_iSafeServerConnect = 0;
	page6.m_iKademlia = 1;
	page6.m_iED2K = 1;

	uint16 oldtcpport=thePrefs.GetPort();
	uint16 oldudpport=thePrefs.GetUDPPort();

	int iResult = sheet.DoModal();
	if (iResult == IDCANCEL) {

		// restore port settings?
		thePrefs.port=oldtcpport;
		thePrefs.udpport=oldudpport;
		CGlobalVariable::listensocket->Rebind() ;
		CGlobalVariable::clientudp->Rebind();
	//	if( !thePrefs.m_strIncomingDir.IsEmpty() ) //默认也要导入该目录下存在的下载任务
	//		CGlobalVariable::downloadqueue->ScanPartFile(thePrefs.m_strIncomingDir);
		return FALSE;
	}

	page2.m_strNick.Trim();
	if (page2.m_strNick.IsEmpty())
		page2.m_strNick = DEFAULT_NICK;

	thePrefs.SetUserNick(page2.m_strNick);
	//thePrefs.SetAutoConnect(page2.m_iAutoConnectAtStart!=0);
	//Chocobo Start
	//第一次运行向导，added by Chocobo on 2006.08.02
	//thePrefs.m_bShowBroswer =(page2.m_iWebbrowser!=0);

	thePrefs.SetAutoStart(page2.m_iAutoStart!=0);
	if( thePrefs.GetAutoStart() )
		AddAutoStart();
	else
		RemAutoStart();
	thePrefs.SetNewAutoDown(page4.m_iDAP!=0);
	thePrefs.SetNewAutoUp(page4.m_iUAP!=0);
	thePrefs.SetTransferFullChunks(page5.m_iULFullChunks!=0);
	thePrefs.SetSafeServerConnectEnabled(page6.m_iSafeServerConnect!=0);
	thePrefs.SetNetworkKademlia(page6.m_iKademlia!=0);
	thePrefs.SetNetworkED2K(page6.m_iED2K!=0);

	// set ports
	if (thePrefs.port != (uint16)_tstoi(page3.m_sTCP))
	{
		CString strMutextName;
		strMutextName.Format(_T("%s:%s"), EMULE_GUID, page3.m_sTCP);
		HWND hMainDlg = theApp.emuledlg->GetSafeHwnd();
	
		theApp.m_pSingleInst->AppEnd();
		delete theApp.m_pSingleInst;
		theApp.m_pSingleInst = new CSingleInst(strMutextName);
		if (theApp.m_pSingleInst->AppStart())
			theApp.m_pSingleInst->InitCompleted(&hMainDlg, sizeof(HWND));





		//CloseHandle(theApp.m_hMutexOneInstance);

		//CString strMutextName;
		//strMutextName.Format(_T("%s:%s"), EMULE_GUID, page3.m_sTCP);
		//theApp.m_hMutexOneInstance = ::CreateMutex(NULL, FALSE, strMutextName);
		
		thePrefs.port=(uint16)_tstoi(page3.m_sTCP);
		thePrefs.SavePort();
	}

	thePrefs.port=(uint16)_tstoi(page3.m_sTCP);
	thePrefs.udpport=(uint16)_tstoi(page3.m_sUDP);
	ASSERT( thePrefs.port!=0 && thePrefs.udpport!=0+10 );
	if (thePrefs.port == 0)
		thePrefs.port = thePrefs.GetRandomTCPPort();
	if (thePrefs.udpport == 0+10)
		thePrefs.udpport = thePrefs.GetRandomUDPPort();
	if ( (thePrefs.port!=CGlobalVariable::listensocket->GetConnectedPort()) || (thePrefs.udpport!=CGlobalVariable::clientudp->GetConnectedPort()) )
		if (!theApp.IsPortchangeAllowed())
			AfxMessageBox(GetResString(IDS_NOPORTCHANGEPOSSIBLE));
		else {
			CGlobalVariable::listensocket->Rebind() ;
			CGlobalVariable::clientudp->Rebind();
			
			//ADDED by fengwen on 2007/01/10	<begin> : redo upnp when ports have been changed.
			if (NULL != theApp.emuledlg)
				theApp.emuledlg->PostMessage(UM_PORT_CHANGED);
			//ADDED by fengwen on 2007/01/10	<end> : redo upnp when ports have been changed.
		}
	
	//ADDED by fengwen on 2006/11/28	<begin> : 设置数据存储目录
	ApplyDirSet(page8);
	//ADDED by fengwen on 2006/11/28	<end> : 设置数据存储目录

	return TRUE;
}
