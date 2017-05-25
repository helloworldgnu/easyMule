/* 
 * $Id: ED2kLinkDlg.cpp 4679 2008-01-24 04:45:46Z thilon $
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
#include "ED2kLinkDlg.h"
#include "KnownFile.h"
#include "partfile.h"
#include "OtherFunctions.h"
#include "preferences.h"
#include "shahashset.h"
#include "UserMsgs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNAMIC(CED2kLinkDlg, CResizablePage) 

BEGIN_MESSAGE_MAP(CED2kLinkDlg, CResizablePage) 
	ON_BN_CLICKED(IDC_LD_CLIPBOARDBUT, OnBnClickedClipboard)
	ON_BN_CLICKED(IDC_LD_SOURCECHE, OnSettingsChange)
	ON_BN_CLICKED(IDC_LD_EMULEHASHCHE, OnSettingsChange)
	ON_BN_CLICKED(IDC_LD_HTMLCHE, OnSettingsChange)
	ON_BN_CLICKED(IDC_LD_HOSTNAMECHE, OnSettingsChange)
	ON_BN_CLICKED(IDC_LD_HASHSETCHE, OnSettingsChange)
	ON_MESSAGE(UM_DATA_CHANGED, OnDataChanged)
END_MESSAGE_MAP() 

CED2kLinkDlg::CED2kLinkDlg() 
   : CResizablePage(CED2kLinkDlg::IDD, IDS_CMT_READALL) 
{ 
	m_paFiles = NULL;
	m_bDataChanged = false;
	m_strCaption = GetResString(IDS_SW_LINK);
	m_psp.pszTitle = m_strCaption;
	m_psp.dwFlags |= PSP_USETITLE;
} 

CED2kLinkDlg::~CED2kLinkDlg() 
{ 
} 

void CED2kLinkDlg::DoDataExchange(CDataExchange* pDX) 
{ 
	CResizablePage::DoDataExchange(pDX); 
	DDX_Control(pDX, IDC_LD_LINKEDI, m_ctrlLinkEdit);
} 

BOOL CED2kLinkDlg::OnInitDialog()
{ 
	CResizablePage::OnInitDialog(); 
	InitWindowStyles(this);

	AddAnchor(IDC_LD_LINKGROUP,TOP_LEFT,BOTTOM_RIGHT);
	AddAnchor(IDC_LD_LINKEDI,TOP_LEFT,BOTTOM_RIGHT);
	AddAnchor(IDC_LD_CLIPBOARDBUT,BOTTOM_RIGHT);
	AddAnchor(IDC_LD_BASICGROUP,BOTTOM_LEFT,BOTTOM_RIGHT);
	AddAnchor(IDC_LD_SOURCECHE,BOTTOM_LEFT,BOTTOM_LEFT);
	AddAnchor(IDC_LD_EMULEHASHCHE,BOTTOM_LEFT,BOTTOM_LEFT);
	AddAnchor(IDC_LD_ADVANCEDGROUP,BOTTOM_LEFT,BOTTOM_RIGHT);
	AddAnchor(IDC_LD_HTMLCHE,BOTTOM_LEFT,BOTTOM_LEFT);
	AddAnchor(IDC_LD_HASHSETCHE,BOTTOM_LEFT,BOTTOM_LEFT);
	AddAnchor(IDC_LD_HOSTNAMECHE,BOTTOM_LEFT,BOTTOM_LEFT);

	// enabled/disable checkbox depending on situation
	if (CGlobalVariable::IsConnected() && !CGlobalVariable::IsFirewalled())
	{
		GetDlgItem(IDC_LD_SOURCECHE)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_LD_SOURCECHE)->EnableWindow(FALSE);
		((CButton*)GetDlgItem(IDC_LD_SOURCECHE))->SetCheck(BST_UNCHECKED);
	}

	if (CGlobalVariable::IsConnected() && !CGlobalVariable::IsFirewalled() && !thePrefs.GetYourHostname().IsEmpty() && thePrefs.GetYourHostname().Find(_T('.')) != -1)
	{
		GetDlgItem(IDC_LD_HOSTNAMECHE)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_LD_HOSTNAMECHE)->EnableWindow(FALSE);
		((CButton*)GetDlgItem(IDC_LD_HOSTNAMECHE))->SetCheck(BST_UNCHECKED);
	}

	Localize(); 

	return TRUE; 
} 

BOOL CED2kLinkDlg::OnSetActive()
{
	if (!CResizablePage::OnSetActive())
		return FALSE;

	if (m_bDataChanged)
	{
		//hashsetlink - check if at least one file has a hasset
		BOOL bShow = FALSE;
		for (int i = 0; i != m_paFiles->GetSize(); i++){
			const CKnownFile* file = STATIC_DOWNCAST(CKnownFile, (*m_paFiles)[i]);
			if (!(file->GetHashCount() > 0 && file->GetHashCount() == file->GetED2KPartHashCount()))
			{
				continue;
			}
			bShow = TRUE;
			break;
		}
		GetDlgItem(IDC_LD_HASHSETCHE)->EnableWindow(bShow);
		if (!bShow)
			((CButton*)GetDlgItem(IDC_LD_HASHSETCHE))->SetCheck(BST_UNCHECKED);

		//aich hash - check if at least one file has a valid hash
		bShow = FALSE;
		for (int i = 0; i != m_paFiles->GetSize(); i++){
			const CKnownFile* file = STATIC_DOWNCAST(CKnownFile, (*m_paFiles)[i]);
			if (file->GetAICHHashset()->HasValidMasterHash() 
				&& (file->GetAICHHashset()->GetStatus() == AICH_VERIFIED || file->GetAICHHashset()->GetStatus() == AICH_HASHSETCOMPLETE))
			{	
				bShow = TRUE;
				break;
			}
		}
		GetDlgItem(IDC_LD_EMULEHASHCHE)->EnableWindow(bShow);
		if (!bShow)
			((CButton*)GetDlgItem(IDC_LD_EMULEHASHCHE))->SetCheck(BST_UNCHECKED);
		else
			((CButton*)GetDlgItem(IDC_LD_EMULEHASHCHE))->SetCheck(BST_CHECKED);

		UpdateLink();
		m_bDataChanged = false;
	}

	return TRUE;
}

LRESULT CED2kLinkDlg::OnDataChanged(WPARAM, LPARAM)
{
	m_bDataChanged = true;
	return 1;
}

void CED2kLinkDlg::Localize(void)
{ 
	GetDlgItem(IDC_LD_BASICGROUP)->SetWindowText(GetResString(IDS_LD_BASICOPT));
	GetDlgItem(IDC_LD_SOURCECHE)->SetWindowText(GetResString(IDS_LD_ADDSOURCE)); 
	GetDlgItem(IDC_LD_EMULEHASHCHE)->SetWindowText(GetResString(IDS_LD_EMULEHASH)); 
	GetDlgItem(IDC_LD_ADVANCEDGROUP)->SetWindowText(GetResString(IDS_LD_ADVANCEDOPT)); 
	GetDlgItem(IDC_LD_HTMLCHE)->SetWindowText(GetResString(IDS_LD_ADDHTML)); 
	GetDlgItem(IDC_LD_HASHSETCHE)->SetWindowText(GetResString(IDS_LD_ADDHASHSET)); 
	//GetDlgItem(IDC_LD_KADLOWIDCHE)->SetWindowText(GetResString(IDS_LD_PREFERKAD)); 
	GetDlgItem(IDC_LD_LINKGROUP)->SetWindowText(GetResString(IDS_SW_LINK)); 
	GetDlgItem(IDC_LD_CLIPBOARDBUT)->SetWindowText(GetResString(IDS_LD_COPYCLIPBOARD));
	GetDlgItem(IDC_LD_HOSTNAMECHE)->SetWindowText(GetResString(IDS_LD_HOSTNAME)); 
}

void CED2kLinkDlg::UpdateLink()
{
	CString strLinks;
	CString strBuffer;
	const bool bHashset = ((CButton*)GetDlgItem(IDC_LD_HASHSETCHE))->GetCheck() == BST_CHECKED;
	const bool bHTML = ((CButton*)GetDlgItem(IDC_LD_HTMLCHE))->GetCheck() == BST_CHECKED;
	const bool bSource = ((CButton*)GetDlgItem(IDC_LD_SOURCECHE))->GetCheck() == BST_CHECKED && CGlobalVariable::IsConnected() && !CGlobalVariable::IsFirewalled();
	const bool bHostname = ((CButton*)GetDlgItem(IDC_LD_HOSTNAMECHE))->GetCheck() == BST_CHECKED && CGlobalVariable::IsConnected() && !CGlobalVariable::IsFirewalled()
		&& !thePrefs.GetYourHostname().IsEmpty() && thePrefs.GetYourHostname().Find(_T('.')) != -1;
	const bool bEMHash = ((CButton*)GetDlgItem(IDC_LD_EMULEHASHCHE))->GetCheck() == BST_CHECKED;

	for (int i = 0; i != m_paFiles->GetSize(); i++)
	{
		if (!strLinks.IsEmpty())
			strLinks += _T("\r\n\r\n");

		if (bHTML)
			strLinks += _T("<a href=\"");
		
		const CKnownFile* file = STATIC_DOWNCAST(CKnownFile, (*m_paFiles)[i]);
		strLinks += CreateED2kLink(file, false);
		
		if (bHashset && file->GetHashCount() > 0 && file->GetHashCount() == file->GetED2KPartHashCount())
		{
			strLinks += _T("p=");
			for (UINT j = 0; j < file->GetHashCount(); j++)
			{
				if (j > 0)
					strLinks += _T(':');
				strLinks += EncodeBase16(file->GetPartHash(j), 16);
			}
			strLinks += _T('|');
		}

		if (bEMHash && file->GetAICHHashset()->HasValidMasterHash() && 
			(file->GetAICHHashset()->GetStatus() == AICH_VERIFIED || file->GetAICHHashset()->GetStatus() == AICH_HASHSETCOMPLETE))
		{
			strBuffer.Format(_T("h=%s|"), file->GetAICHHashset()->GetMasterHash().GetString() );
			strLinks += strBuffer;			
		}

		if (file->HasNullHash())
		{
			strLinks += file->GetPartFileURL();
		}
		else
		{
			strLinks += _T('/');
		}
		
		if (bHostname)
		{
			strBuffer.Format(_T("|sources,%s:%i|/"), thePrefs.GetYourHostname(), thePrefs.GetPort() );
			strLinks += strBuffer;
		}
		else if(bSource)
		{
			uint32 dwID = CGlobalVariable::GetID();
			strBuffer.Format(_T("|sources,%i.%i.%i.%i:%i|/"),(uint8)dwID,(uint8)(dwID>>8),(uint8)(dwID>>16),(uint8)(dwID>>24), thePrefs.GetPort() );
			strLinks += strBuffer;
		}

		if (bHTML)
			strLinks += _T("\">") + StripInvalidFilenameChars(file->GetFileName(), true) + _T("</a>");
	}

	m_ctrlLinkEdit.SetWindowText(strLinks);

}

void CED2kLinkDlg::OnBnClickedClipboard()
{
	CString strBuffer;
	m_ctrlLinkEdit.GetWindowText(strBuffer);
	theApp.CopyTextToClipboard(strBuffer);
}

void CED2kLinkDlg::OnSettingsChange()
{
	UpdateLink();
}

BOOL CED2kLinkDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (LOWORD(wParam) == IDCANCEL)
	{
		return ::SendMessage(::GetParent(m_hWnd), WM_COMMAND, wParam, lParam);
	}

	return CResizablePage::OnCommand(wParam, lParam);
}
