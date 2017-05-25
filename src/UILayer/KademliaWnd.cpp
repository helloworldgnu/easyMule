/* 
 * $Id: KademliaWnd.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
#include "KademliaWnd.h"
#include "KadContactListCtrl.h"
#include "KadContactHistogramCtrl.h"
#include "KadSearchListCtrl.h"
#include "Kademlia/Kademlia/kademlia.h"
#include "Kademlia/Kademlia/prefs.h"
#include "Kademlia/net/kademliaudplistener.h"
#include "Ini2.h"
#include "CustomAutoComplete.h"
#include "OtherFunctions.h"
#include "emuledlg.h"
#include "clientlist.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define	ONBOOTSTRAP_STRINGS_PROFILE	_T("AC_BootstrapIPs.dat")

// KademliaWnd dialog

IMPLEMENT_DYNAMIC(CKademliaWnd, CDialog)

BEGIN_MESSAGE_MAP(CKademliaWnd, CResizableDialog)
	ON_BN_CLICKED(IDC_BOOTSTRAPBUTTON, OnBnClickedBootstrapbutton)
	ON_BN_CLICKED(IDC_FIREWALLCHECKBUTTON, OnBnClickedFirewallcheckbutton)
	ON_BN_CLICKED(IDC_KADCONNECT, OnBnConnect)
	ON_WM_SYSCOLORCHANGE()
	ON_EN_SETFOCUS(IDC_BOOTSTRAPIP, OnEnSetfocusBootstrapip)
	ON_EN_CHANGE(IDC_BOOTSTRAPIP, UpdateControlsState)
	ON_EN_CHANGE(IDC_BOOTSTRAPPORT, UpdateControlsState)
	ON_BN_CLICKED(IDC_RADCLIENTS, UpdateControlsState)
	ON_BN_CLICKED(IDC_RADIP, UpdateControlsState)
END_MESSAGE_MAP()

CKademliaWnd::CKademliaWnd(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CKademliaWnd::IDD, pParent)
{
	m_contactListCtrl = new CKadContactListCtrl;
	m_contactHistogramCtrl = new CKadContactHistogramCtrl;
	searchList = new CKadSearchListCtrl;
	m_pacONBSIPs = NULL;

	icon_kadcont=NULL;
	icon_kadsea=NULL;
}

CKademliaWnd::~CKademliaWnd()
{
	if (m_pacONBSIPs){
		m_pacONBSIPs->Unbind();
		m_pacONBSIPs->Release();
	}
	delete m_contactListCtrl;
	delete m_contactHistogramCtrl;
	delete searchList;

	if (icon_kadcont)
		VERIFY( DestroyIcon(icon_kadcont) );
	if (icon_kadsea)
		VERIFY( DestroyIcon(icon_kadsea) );
}

BOOL CKademliaWnd::SaveAllSettings()
{
	if (m_pacONBSIPs)
		m_pacONBSIPs->SaveList(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + ONBOOTSTRAP_STRINGS_PROFILE);

	return TRUE;
}

BOOL CKademliaWnd::OnInitDialog()
{
	CResizableDialog::OnInitDialog();
	InitWindowStyles(this);
	m_contactListCtrl->Init();
	searchList->Init();
	SetAllIcons();
	Localize();

	AddAnchor(IDC_KADICO1, TOP_LEFT);
	AddAnchor(IDC_CONTACTLIST,TOP_LEFT, CSize(100,50));
	AddAnchor(IDC_KAD_HISTOGRAM,TOP_RIGHT, CSize(100,50));
	AddAnchor(IDC_KADICO2, CSize(0,50));
	AddAnchor(IDC_SEARCHLIST,CSize(0,50),CSize(100,100));
	AddAnchor(IDC_KADCONTACTLAB,TOP_LEFT);
	AddAnchor(IDC_FIREWALLCHECKBUTTON, TOP_RIGHT);
	AddAnchor(IDC_KADCONNECT, TOP_RIGHT);
	AddAnchor(IDC_KADSEARCHLAB,CSize(0,50));
	AddAnchor(IDC_BSSTATIC, TOP_RIGHT);
	AddAnchor(IDC_BOOTSTRAPBUTTON, TOP_RIGHT);
	AddAnchor(IDC_BOOTSTRAPPORT, TOP_RIGHT);
	AddAnchor(IDC_BOOTSTRAPIP, TOP_RIGHT);
	AddAnchor(IDC_SSTATIC4, TOP_RIGHT);
	AddAnchor(IDC_SSTATIC7, TOP_RIGHT);
	AddAnchor(IDC_RADCLIENTS, TOP_RIGHT);
	AddAnchor(IDC_RADIP, TOP_RIGHT);

	searchList->UpdateKadSearchCount();
	m_contactListCtrl->UpdateKadContactCount();

	if (thePrefs.GetUseAutocompletion()){
		m_pacONBSIPs = new CCustomAutoComplete();
		m_pacONBSIPs->AddRef();
		if (m_pacONBSIPs->Bind(::GetDlgItem(m_hWnd, IDC_BOOTSTRAPIP), ACO_UPDOWNKEYDROPSLIST | ACO_AUTOSUGGEST | ACO_FILTERPREFIXES ))
			m_pacONBSIPs->LoadList(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + ONBOOTSTRAP_STRINGS_PROFILE);
	}

	CheckDlgButton(IDC_RADCLIENTS,1);

	return true;
}

void CKademliaWnd::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CONTACTLIST, *m_contactListCtrl);
	DDX_Control(pDX, IDC_KAD_HISTOGRAM, *m_contactHistogramCtrl);
	DDX_Control(pDX, IDC_SEARCHLIST, *searchList);
	DDX_Control(pDX, IDC_KADCONTACTLAB, kadContactLab);
	DDX_Control(pDX, IDC_KADSEARCHLAB, kadSearchLab);
	DDX_Control(pDX, IDC_BSSTATIC, m_ctrlBootstrap);
}

BOOL CKademliaWnd::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_KEYDOWN)
	{
		// Don't handle Ctrl+Tab in this window. It will be handled by main window.
		if (pMsg->wParam == VK_TAB && GetAsyncKeyState(VK_CONTROL) < 0)
			return FALSE;
	}

	return CResizableDialog::PreTranslateMessage(pMsg);
}

void CKademliaWnd::OnEnSetfocusBootstrapip()
{
	CheckRadioButton(IDC_RADIP, IDC_RADCLIENTS, IDC_RADIP);
}

void CKademliaWnd::OnBnClickedBootstrapbutton()
{
	CString strIP;
	uint16 nPort = 0;

	if (!IsDlgButtonChecked(IDC_RADCLIENTS))
	{
		GetDlgItem(IDC_BOOTSTRAPIP)->GetWindowText(strIP);
		strIP.Trim();

		// auto-handle ip:port
		int iPos;
		if ((iPos = strIP.Find(_T(':'))) != -1)
		{
			GetDlgItem(IDC_BOOTSTRAPPORT)->SetWindowText(strIP.Mid(iPos+1));
			strIP = strIP.Left(iPos);
			GetDlgItem(IDC_BOOTSTRAPIP)->SetWindowText(strIP);
		}

		CString strPort;
		GetDlgItem(IDC_BOOTSTRAPPORT)->GetWindowText(strPort);
		strPort.Trim();
		nPort = (uint16)_ttoi(strPort);

		// invalid IP/Port
		if (strIP.GetLength()<7 || nPort==0)
			return;

		if (m_pacONBSIPs && m_pacONBSIPs->IsBound())
			m_pacONBSIPs->AddItem(strIP + _T(":") + strPort, 0);
	}

	if( !Kademlia::CKademlia::IsRunning() )
	{
		Kademlia::CKademlia::Start();
		theApp.emuledlg->ShowConnectionState();
	}
	if (!strIP.IsEmpty() && nPort)
	{
		// JOHNTODO - Switch between Kad1 and Kad2
		Kademlia::CKademlia::Bootstrap(strIP, nPort, true);
	}
}

void CKademliaWnd::OnBnClickedFirewallcheckbutton()
{
	Kademlia::CKademlia::RecheckFirewalled();
}

void CKademliaWnd::OnBnConnect()
{
	if (Kademlia::CKademlia::IsConnected())
		Kademlia::CKademlia::Stop();
	else if (Kademlia::CKademlia::IsRunning())
		Kademlia::CKademlia::Stop();
	else
		Kademlia::CKademlia::Start();
	theApp.emuledlg->ShowConnectionState();
}

void CKademliaWnd::OnSysColorChange()
{
	CResizableDialog::OnSysColorChange();
	SetAllIcons();
}

void CKademliaWnd::SetAllIcons()
{
	// frames
	m_ctrlBootstrap.SetIcon(_T("KadBootstrap"));

	if (icon_kadcont)
		VERIFY( DestroyIcon(icon_kadcont) );
	icon_kadcont = theApp.LoadIcon(_T("KadContactList"), 16, 16);
	((CStatic*)GetDlgItem(IDC_KADICO1))->SetIcon(icon_kadcont);

	if (icon_kadsea)
		VERIFY( DestroyIcon(icon_kadsea) );
	icon_kadsea = theApp.LoadIcon(_T("KadCurrentSearches"), 16, 16);
	((CStatic*)GetDlgItem(IDC_KADICO2))->SetIcon(icon_kadsea);
}

void CKademliaWnd::Localize()
{
	m_ctrlBootstrap.SetWindowText(GetResString(IDS_BOOTSTRAP));
	GetDlgItem(IDC_BOOTSTRAPBUTTON)->SetWindowText(GetResString(IDS_BOOTSTRAP));
	GetDlgItem(IDC_SSTATIC4)->SetWindowText(GetResString(IDS_SV_ADDRESS) + _T(":"));
	GetDlgItem(IDC_SSTATIC7)->SetWindowText(GetResString(IDS_SV_PORT) + _T(":"));
	GetDlgItem(IDC_FIREWALLCHECKBUTTON)->SetWindowText(GetResString(IDS_KAD_RECHECKFW));
	
	SetDlgItemText(IDC_KADCONTACTLAB,GetResString(IDS_KADCONTACTLAB));
	SetDlgItemText(IDC_KADSEARCHLAB,GetResString(IDS_KADSEARCHLAB));

	SetDlgItemText(IDC_RADCLIENTS,GetResString(IDS_RADCLIENTS));

	UpdateControlsState();
	m_contactHistogramCtrl->Localize();
	m_contactListCtrl->Localize();
	searchList->Localize();
}

void CKademliaWnd::UpdateControlsState()
{
	if (!::IsWindow(m_hWnd))
		return;

	CString strLabel;
	if (Kademlia::CKademlia::IsConnected())
		strLabel = GetResString(IDS_MAIN_BTN_DISCONNECT);
	else if (Kademlia::CKademlia::IsRunning())
		strLabel = GetResString(IDS_MAIN_BTN_CANCEL);
	else
		strLabel = GetResString(IDS_MAIN_BTN_CONNECT);
	strLabel.Remove(_T('&'));
	GetDlgItem(IDC_KADCONNECT)->SetWindowText(strLabel);

	CString strBootstrapIP;
	GetDlgItemText(IDC_BOOTSTRAPIP, strBootstrapIP);
	CString strBootstrapPort;
	GetDlgItemText(IDC_BOOTSTRAPPORT, strBootstrapPort);
	GetDlgItem(IDC_BOOTSTRAPBUTTON)->EnableWindow(
		!Kademlia::CKademlia::IsConnected()
		&& (  (   IsDlgButtonChecked(IDC_RADIP)>0
		       && !strBootstrapIP.IsEmpty()
			   && (strBootstrapIP.Find(_T(':')) != -1 || !strBootstrapPort.IsEmpty())
			  )
		    || IsDlgButtonChecked(IDC_RADCLIENTS)>0));
}

UINT CKademliaWnd::GetContactCount() const
{
	return m_contactListCtrl->GetItemCount();
}

void CKademliaWnd::UpdateKadContactCount()
{
	m_contactListCtrl->UpdateKadContactCount();
}

void CKademliaWnd::ShowContacts()
{
	m_contactHistogramCtrl->ShowWindow(SW_SHOW);
	m_contactListCtrl->Visable();
}

void CKademliaWnd::HideContacts()
{
	m_contactHistogramCtrl->ShowWindow(SW_HIDE);
	m_contactListCtrl->Hide();
}

bool CKademliaWnd::ContactAdd(const Kademlia::CContact* contact)
{
	ASSERT( contact );
	m_contactHistogramCtrl->ContactAdd(contact);
	return m_contactListCtrl->ContactAdd(contact);
}

void CKademliaWnd::ContactRem(const Kademlia::CContact* contact)
{
	ASSERT( contact );
	m_contactHistogramCtrl->ContactRem(contact);
	m_contactListCtrl->ContactRem(contact);
}

void CKademliaWnd::ContactRef(const Kademlia::CContact* contact)
{
	if( NULL!=contact )
		m_contactListCtrl->ContactRef(contact);
}
