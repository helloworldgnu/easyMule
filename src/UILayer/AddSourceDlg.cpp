/* 
 * $Id: AddSourceDlg.cpp 5687 2008-05-28 10:44:28Z huby $
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
#include "AddSourceDlg.h"
#include "PartFile.h"
#include "OtherFunctions.h"
#include "UpDownClient.h"
#include "DownloadQueue.h"
#include <winsock2.h>
#include <wininet.h>
//#include "ed2klink.h"
#include "DNSManager.h"

#include "Ed2kUpDownClient.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// CAddSourceDlg dialog

IMPLEMENT_DYNAMIC(CAddSourceDlg, CDialog)

BEGIN_MESSAGE_MAP(CAddSourceDlg, CResizableDialog)
	ON_BN_CLICKED(IDC_RSRC, OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RURL, OnBnClickedRadio4)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()

CAddSourceDlg::CAddSourceDlg(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CAddSourceDlg::IDD, pParent)
	, m_nSourceType(0)
{
	m_pFile = NULL;
}

CAddSourceDlg::~CAddSourceDlg()
{
}

void CAddSourceDlg::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RSRC, m_nSourceType);
}

void CAddSourceDlg::SetFile(CPartFile *pFile)
{
	m_pFile = pFile;
}

BOOL CAddSourceDlg::OnInitDialog()
{
	CResizableDialog::OnInitDialog();
	InitWindowStyles(this);

	AddAnchor(IDC_SOURCE_TYPE, TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(IDC_EDIT10, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDOK, BOTTOM_RIGHT);
	AddAnchor(IDC_BUTTON1, BOTTOM_RIGHT);
	AddAnchor(IDCANCEL, BOTTOM_RIGHT);

	if (m_pFile)
		SetWindowText(m_pFile->GetFileName());

	// localize
	SetDlgItemText(IDC_BUTTON1,GetResString(IDS_ADD));
	SetDlgItemText(IDCANCEL,GetResString(IDS_CANCEL));
	SetDlgItemText(IDC_RSRC,GetResString(IDS_SOURCECLIENT));
	SetDlgItemText(IDC_SOURCE_TYPE,GetResString(IDS_META_SRCTYPE));
	SetDlgItemText(IDC_RURL,GetResString(IDS_SV_URL));
	SetDlgItemText(IDC_UIP,GetResString(IDS_USERSIP));
	SetDlgItemText(IDC_PORT,GetResString(IDS_PORT));	

	EnableSaveRestore(_T("AddSourceDlg"));

	OnBnClickedRadio1();
	return FALSE; // return FALSE, we changed the focus!
}

void CAddSourceDlg::OnBnClickedRadio1()
{
	m_nSourceType = 0;
	GetDlgItem(IDC_EDIT2)->EnableWindow(true);
	GetDlgItem(IDC_EDIT3)->EnableWindow(true);
	GetDlgItem(IDC_EDIT10)->EnableWindow(false);
	GetDlgItem(IDC_EDIT2)->SetFocus();
}

void CAddSourceDlg::OnBnClickedRadio4()
{
	m_nSourceType = 1;
	GetDlgItem(IDC_EDIT2)->EnableWindow(false);
	GetDlgItem(IDC_EDIT3)->EnableWindow(false);
	GetDlgItem(IDC_EDIT10)->EnableWindow(true);
	GetDlgItem(IDC_EDIT10)->SetFocus();
}

inline unsigned int FromHexDigit(TCHAR digit) {
	switch (digit) {
		case _T('0'): return 0;
		case _T('1'): return 1;
		case _T('2'): return 2;
		case _T('3'): return 3;
		case _T('4'): return 4;
		case _T('5'): return 5;
		case _T('6'): return 6;
		case _T('7'): return 7 ;
		case _T('8'): return 8;
		case _T('9'): return 9;
		case _T('A'): return 10;
		case _T('B'): return 11;
		case _T('C'): return 12;
		case _T('D'): return 13;
		case _T('E'): return 14;
		case _T('F'): return 15;
		case _T('a'): return 10;
		case _T('b'): return 11;
		case _T('c'): return 12;
		case _T('d'): return 13;
		case _T('e'): return 14;
		case _T('f'): return 15;
		default: throw GetResString(IDS_ERR_ILLFORMEDHASH);
	}
}

void CAddSourceDlg::OnBnClickedButton1()
{
	if (!m_pFile)
		return;

	switch (m_nSourceType)
	{
		case 0:
		{
			CString sip;
			GetDlgItem(IDC_EDIT2)->GetWindowText(sip);
			if (sip.IsEmpty())
				return;

			// if the port is specified with the IP, ignore any possible specified port in the port control
			uint16 port;
			int iColon = sip.Find(_T(':'));
			if (iColon != -1) {
				port = (uint16)_tstoi(sip.Mid(iColon + 1));
				sip = sip.Left(iColon);
			}
			else {
				BOOL bTranslated = FALSE;
				port = (uint16)GetDlgItemInt(IDC_EDIT3, &bTranslated, FALSE);
				if (!bTranslated)
					return;
			}

			uint32 ip;
			USES_CONVERSION;
			if ((ip = inet_addr(T2CA(sip))) == INADDR_NONE && _tcscmp(sip, _T("255.255.255.255")) != 0)
				ip = 0;
			if (IsGoodIPPort(ip, port))
			{
				CUpDownClient* toadd = new CEd2kUpDownClient(m_pFile, port, ntohl(ip), 0, 0);
				toadd->SetSourceFrom(SF_PASSIVE);
				CGlobalVariable::downloadqueue->CheckAndAddSource(m_pFile, toadd);
			}
			break;
		}
		case 1:
		{
			CString strURL;
			GetDlgItem(IDC_EDIT10)->GetWindowText(strURL);

			if( strURL.Find( _T("peer://|") )!=-1 )
			{				
				int iPos = 0;
				uint32 buddyIP,peerIP;
				uint16 buddyPort,peerKadPort;
				BYTE cBuddyID[16];
				BYTE cUserHash[16];
				memset(cBuddyID,0,16);
				memset(cUserHash,0,16);
				CString sBuddyID,sBuddyIP,sBuddyPort,sUserHash,sPeerIP,sPeerKadPort;

				CString strTok = GetNextString(strURL, _T("|"), iPos);
				if (strTok == _T("peer://"))
				{
					sBuddyID = GetNextString(strURL, _T("|"), iPos);
					sBuddyIP = GetNextString(strURL, _T("|"), iPos);
					sBuddyPort = GetNextString(strURL, _T("|"), iPos);
					sUserHash = GetNextString(strURL, _T("|"), iPos);
					sPeerIP = GetNextString(strURL, _T("|"), iPos);
					sPeerKadPort = GetNextString(strURL, _T("|"), iPos);
				}
				LPCTSTR pszBuddyID = (LPCTSTR)sBuddyID;
				if( sBuddyID!=_T("0") )
				{
					for (int idx = 0; idx < 16; ++idx) 
					{
						cBuddyID[idx] = (BYTE)(FromHexDigit(*pszBuddyID++)*16);
						cBuddyID[idx] = (BYTE)(cBuddyID[idx] + FromHexDigit(*pszBuddyID++));
					}
				}				
				LPCTSTR pszUserHash = (LPCTSTR)sUserHash;
				if(sUserHash!=_T("0"))
				for (int idx = 0; idx < 16; ++idx) 
				{
					cUserHash[idx] = (BYTE)(FromHexDigit(*pszUserHash++)*16);
					cUserHash[idx] = (BYTE)(cUserHash[idx] + FromHexDigit(*pszUserHash++));
				}
				buddyIP = inet_addr( CStringA(sBuddyIP) );
				unsigned long ul;
				ul = _tcstoul( sBuddyPort, 0, 10 );
				buddyPort = static_cast<uint16>(ul);
				peerIP = inet_addr( CStringA(sPeerIP) );
				peerKadPort = static_cast<uint16>(ul);

				CUpDownClient* toadd = new CEd2kUpDownClient(m_pFile,(uint16)-1,1,0,0,false);//tcp(-1) 表示默认支持 Nat Trans										
				toadd->SetSourceFrom(SF_KADEMLIA);				
				toadd->SetBuddyID(cBuddyID);
				toadd->SetBuddyIP(buddyIP);
				toadd->SetBuddyPort(buddyPort);	
				toadd->SetUserHash(cUserHash);
				toadd->SetIP(peerIP);
				toadd->SetKadPort(peerKadPort);	
				CGlobalVariable::downloadqueue->CheckAndAddSource(m_pFile, toadd);
			}
			else if (!strURL.IsEmpty())
			{
				TCHAR szScheme[INTERNET_MAX_SCHEME_LENGTH];
				TCHAR szHostName[INTERNET_MAX_HOST_NAME_LENGTH];
				TCHAR szUrlPath[INTERNET_MAX_PATH_LENGTH];
				TCHAR szUserName[INTERNET_MAX_USER_NAME_LENGTH];
				TCHAR szPassword[INTERNET_MAX_PASSWORD_LENGTH];
				TCHAR szExtraInfo[INTERNET_MAX_URL_LENGTH];
				URL_COMPONENTS Url = {0};
				Url.dwStructSize = sizeof(Url);
				Url.lpszScheme = szScheme;
				Url.dwSchemeLength = ARRSIZE(szScheme);
				Url.lpszHostName = szHostName;
				Url.dwHostNameLength = ARRSIZE(szHostName);
				Url.lpszUserName = szUserName;
				Url.dwUserNameLength = ARRSIZE(szUserName);
				Url.lpszPassword = szPassword;
				Url.dwPasswordLength = ARRSIZE(szPassword);
				Url.lpszUrlPath = szUrlPath;
				Url.dwUrlPathLength = ARRSIZE(szUrlPath);
				Url.lpszExtraInfo = szExtraInfo;
				Url.dwExtraInfoLength = ARRSIZE(szExtraInfo);
				if (InternetCrackUrl(strURL, 0, 0, &Url) && Url.dwHostNameLength > 0)
				{
					SUnresolvedHostname* hostname = new SUnresolvedHostname;
					hostname->strURL = strURL;
					hostname->strHostname = szHostName;
					//CGlobalVariable::m_DNSManager->AddToResolved(m_pFile, hostname);
					m_pFile->RecordUrlSource( strURL,true,0,sfManualAdded );
					delete hostname;
				}
			}
			break;
		}
	}
}

void CAddSourceDlg::OnBnClickedOk()
{
	OnBnClickedButton1();
	OnOK();
}
