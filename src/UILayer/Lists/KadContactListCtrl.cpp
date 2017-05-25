/* 
 * $Id: KadContactListCtrl.cpp 4783 2008-02-02 08:17:12Z soarchin $
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
#include "Ini2.h"
#include "OtherFunctions.h"
#include "emuledlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// CONContactListCtrl

enum ECols
{
	colID = 0,
	colType,
	colDistance
};

IMPLEMENT_DYNAMIC(CKadContactListCtrl, CMuleListCtrl)

BEGIN_MESSAGE_MAP(CKadContactListCtrl, CMuleListCtrl)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
	ON_WM_DESTROY()
	ON_WM_SYSCOLORCHANGE()
END_MESSAGE_MAP()

CKadContactListCtrl::CKadContactListCtrl()
{
	SetGeneralPurposeFind(true);
	SetName(_T("ONContactListCtrl"));
}

CKadContactListCtrl::~CKadContactListCtrl()
{
}

void CKadContactListCtrl::Init()
{
	SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

	InsertColumn(colID,GetResString(IDS_ID),LVCFMT_LEFT,100);
	InsertColumn(colType,GetResString(IDS_TYPE) ,LVCFMT_LEFT,50);
	InsertColumn(colDistance,GetResString(IDS_KADDISTANCE),LVCFMT_LEFT,50);
	SetAllIcons();
	Localize();

	LoadSettings();
	int iSortItem = GetSortItem();
	bool bSortAscending = GetSortAscending();

	SetSortArrow(iSortItem, bSortAscending);
	SortItems(SortProc, MAKELONG(iSortItem, (bSortAscending ? 0 : 0x0001)));
}

void CKadContactListCtrl::SaveAllSettings()
{
	SaveSettings();
}

void CKadContactListCtrl::OnSysColorChange()
{
	CMuleListCtrl::OnSysColorChange();
	SetAllIcons();
}

void CKadContactListCtrl::SetAllIcons()
{
	CImageList iml;
	iml.Create(16,16,theApp.m_iDfltImageListColorFlags|ILC_MASK,0,1);
	iml.SetBkColor(CLR_NONE);
	iml.Add(CTempIconLoader(_T("Contact0")));
	iml.Add(CTempIconLoader(_T("Contact1")));
	iml.Add(CTempIconLoader(_T("Contact2")));
	iml.Add(CTempIconLoader(_T("Contact3")));
	iml.Add(CTempIconLoader(_T("Contact4")));
	ASSERT( (GetStyle() & LVS_SHAREIMAGELISTS) == 0 );
	HIMAGELIST himl = ApplyImageList(iml.Detach());
	if (himl)
		ImageList_Destroy(himl);
}

void CKadContactListCtrl::Localize()
{
	CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
	HDITEM hdi;
	hdi.mask = HDI_TEXT;
	CString strRes;

	for (int icol=0;icol< pHeaderCtrl->GetItemCount() ;icol++) 
	{
		switch (icol) 
		{
			case colID: strRes = GetResString(IDS_ID); break;
			case colType: strRes = GetResString(IDS_TYPE); break;
			case colDistance: strRes = GetResString(IDS_KADDISTANCE); break;
		}
	
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(icol, &hdi);
	}

	int iItems = GetItemCount();
	for (int i = 0; i < iItems; i++)
		UpdateContact(i, (Kademlia::CContact*)GetItemData(i), true);
}
	
void CKadContactListCtrl::UpdateContact(int iItem, const Kademlia::CContact* contact, bool bLocalize)
{
	CString id;
	if (!bLocalize) // update the following fields only if really needed (it's quite expensive to always update them)
	{
		contact->GetClientID(&id);
		SetItemText(iItem,colID,id);

		id.Format(_T("%i(%u)"),contact->GetType(), contact->GetVersion());
		SetItemText(iItem,colType,id);

		contact->GetDistance(&id);
		SetItemText(iItem,colDistance,id);

		SetItem(iItem,0,LVIF_IMAGE,0,contact->GetType()>4?4:contact->GetType(),0,0,0,0);
	}
}

void CKadContactListCtrl::UpdateKadContactCount()
{
	CString id;
	id.Format(_T("%s (%i)"), GetResString(IDS_KADCONTACTLAB), GetItemCount());
	theApp.emuledlg->kademliawnd->GetDlgItem(IDC_KADCONTACTLAB)->SetWindowText(id);
}

bool CKadContactListCtrl::ContactAdd(const Kademlia::CContact* contact)
{
	bool bResult = false;
	try
	{
		ASSERT( contact != NULL );
		int iItem = InsertItem(LVIF_TEXT|LVIF_PARAM,GetItemCount(),NULL,0,0,0,(LPARAM)contact);
		if (iItem >= 0)
		{
			bResult = true;
	//		Trying to update all the columns causes one of the connection freezes in win98
	//		ContactRef(contact);
			// If it still doesn't work under Win98, uncomment the '!afxData.bWin95' term
			if (iItem >= 0)
				UpdateContact(iItem, contact);
			UpdateKadContactCount();
		}
	}
	catch(...){ASSERT(0);}
	return bResult;
}

void CKadContactListCtrl::ContactRem(const Kademlia::CContact* contact)
{
	try
	{
		ASSERT( contact != NULL );
		LVFINDINFO find;
		find.flags = LVFI_PARAM;
		find.lParam = (LPARAM)contact;
		int iItem = FindItem(&find);
		if (iItem != -1)
		{
			DeleteItem(iItem);
			UpdateKadContactCount();
		}
	}
	catch(...){ASSERT(0);}
}

void CKadContactListCtrl::ContactRef(const Kademlia::CContact* contact)
{
	try
	{
		ASSERT( contact != NULL );
		LVFINDINFO find;
		find.flags = LVFI_PARAM;
		find.lParam = (LPARAM)contact;
		int iItem = FindItem(&find);
		if (iItem != -1)
			UpdateContact(iItem, contact);
	}
	catch(...){ASSERT(0);}
}

BOOL CKadContactListCtrl::OnCommand(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	// ???
	return TRUE;
}

void CKadContactListCtrl::OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	// Determine ascending based on whether already sorted on this column
	int iSortItem = GetSortItem();
	bool bOldSortAscending = GetSortAscending();
	bool bSortAscending = (iSortItem != pNMListView->iSubItem) ? true : !bOldSortAscending;

	// Item is column clicked
	iSortItem = pNMListView->iSubItem;

	// Sort table
	UpdateSortHistory(MAKELONG(iSortItem, (bSortAscending ? 0 : 0x0001)));
	SetSortArrow(iSortItem, bSortAscending);
	SortItems(SortProc, MAKELONG(iSortItem, (bSortAscending ? 0 : 0x0001)));

	*pResult = 0;
}

int CKadContactListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	Kademlia::CContact* item1 = (Kademlia::CContact*)lParam1;
	Kademlia::CContact* item2 = (Kademlia::CContact*)lParam2; 
	if((item1 == NULL) || (item2 == NULL))
		return 0;
		
	__try
	{		
		int iResult;
		switch(LOWORD(lParamSort))
		{
			case colID:
			{
				Kademlia::CUInt128 i1;
				Kademlia::CUInt128 i2;
				item1->GetClientID(&i1);
				item2->GetClientID(&i2);
				iResult = i1.CompareTo(i2);
				break;
			}
			case colType:
				iResult = item1->GetType() - item2->GetType();
				break;
			case colDistance:
			{
				Kademlia::CUInt128 distance1, distance2;
				item1->GetDistance(&distance1);
				item2->GetDistance(&distance2);
				iResult = distance1.CompareTo(distance2);
				break;
			}
			default:
				return 0;
		}
		if (HIWORD(lParamSort))
			iResult = -iResult;
		return iResult;
	}
	//catch(...)
	__except(0)
	{
		return 0;
	}	
}
