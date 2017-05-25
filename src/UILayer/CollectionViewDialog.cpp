/* 
 * $Id: CollectionViewDialog.cpp 4483 2008-01-02 09:19:06Z soarchin $
 * 
 * this file is part of eMule
 * Copyright (C)2002-2005 Merkur ( devs@emule-project.net / http://www.emule-project.net )
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

// CollectionViewDialog.cpp : implementation file
//

#include "stdafx.h"
#include "emule.h"
#include "emuledlg.h"
#include "CollectionViewDialog.h"
#include "OtherFunctions.h"
#include "Collection.h"
#include "CollectionFile.h"
#include "DownloadQueue.h"
#include "TransferWnd.h"
#include "CatDialog.h"
#include "SearchDlg.h"
#include "Partfile.h"
#include "CmdFuncs.h"
// CCollectionViewDialog dialog

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

enum ECols
{
	colName = 0,
	colSize,
	colHash
};

IMPLEMENT_DYNAMIC(CCollectionViewDialog, CDialog)

BEGIN_MESSAGE_MAP(CCollectionViewDialog, CResizableDialog)
	ON_NOTIFY(NM_DBLCLK, IDC_COLLECTIONVEWLIST, OnNMDblclkCollectionvewlist)
	ON_BN_CLICKED(IDC_VIEWCOLLECTIONDL, OnBnClickedViewcollectiondl)
	ON_BN_CLICKED(IDC_VCOLL_CLOSE, OnBnClickedOk)
END_MESSAGE_MAP()

CCollectionViewDialog::CCollectionViewDialog(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CCollectionViewDialog::IDD, pParent)
	, m_pCollection(NULL)
{
	m_icoWnd = NULL;
	m_icoColl = NULL;
}

CCollectionViewDialog::~CCollectionViewDialog()
{
	if (m_icoWnd)
		VERIFY( DestroyIcon(m_icoWnd) );
	if (m_icoColl)
		VERIFY( DestroyIcon(m_icoColl) );
}

void CCollectionViewDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COLLECTIONVEWLIST, m_CollectionViewList);
	DDX_Control(pDX, IDC_COLLECTIONVIEWCATEGORYCHECK, m_AddNewCatagory);
	DDX_Control(pDX, IDC_COLLECTIONVIEWLISTLABEL, m_CollectionViewListLabel);
	DDX_Control(pDX, IDC_COLLECTIONVIEWLISTICON, m_CollectionViewListIcon);
	DDX_Control(pDX, IDC_VIEWCOLLECTIONDL, m_CollectionDownload);
	DDX_Control(pDX, IDC_VCOLL_CLOSE, m_CollectionExit);
	DDX_Control(pDX, IDC_COLLECTIONVIEWAUTHOR, m_CollectionViewAuthor);
	DDX_Control(pDX, IDC_COLLECTIONVIEWAUTHORKEY, m_CollectionViewAuthorKey);
}

void CCollectionViewDialog::SetCollection(CCollection* pCollection)
{
	if(!pCollection)
	{
		ASSERT(0);
		return;
	}
	m_pCollection = pCollection;
}

BOOL CCollectionViewDialog::OnInitDialog(void)
{
	CDialog::OnInitDialog();

	if(!m_pCollection)
	{
		ASSERT(0);
		return TRUE;
	}

	m_CollectionViewList.Init(_T("CollectionView"));
	SetIcon(m_icoWnd = theApp.LoadIcon(_T("Collection_View")), FALSE);

	m_AddNewCatagory.SetCheck(false);

	SetWindowText(m_pCollection->m_sCollectionName);

	m_CollectionViewListIcon.SetIcon(m_icoColl = theApp.LoadIcon(_T("COLLECTION")));
	m_CollectionDownload.SetWindowText(GetResString(IDS_DOWNLOAD));
	m_CollectionExit.SetWindowText(GetResString(IDS_CW_CLOSE));
	m_CollectionViewListLabel.SetWindowText(GetResString(IDS_COLLECTIONLIST));
	SetDlgItemText(IDC_COLLECTIONVIEWAUTHORLABEL, GetResString(IDS_AUTHOR) + _T(":") );
	SetDlgItemText(IDC_COLLECTIONVIEWAUTHORKEYLABEL, GetResString(IDS_AUTHORKEY) + _T(":") );
	SetDlgItemText(IDC_COLLECTIONVIEWCATEGORYCHECK, GetResString(IDS_COLL_ADDINCAT) );
	SetDlgItemText(IDC_VCOLL_DETAILS, GetResString(IDS_DETAILS));
	SetDlgItemText(IDC_VCOLL_OPTIONS, GetResString(IDS_OPTIONS) );

	m_CollectionViewAuthor.SetWindowText(m_pCollection->m_sCollectionAuthorName);
	m_CollectionViewAuthorKey.SetWindowText(m_pCollection->GetAuthorKeyHashString());


	AddAnchor(IDC_COLLECTIONVEWLIST, TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(IDC_VCOLL_DETAILS, BOTTOM_LEFT, BOTTOM_RIGHT);
	AddAnchor(IDC_VCOLL_OPTIONS, BOTTOM_LEFT, BOTTOM_RIGHT);
	AddAnchor(IDC_COLLECTIONVIEWAUTHORLABEL, BOTTOM_LEFT);
	AddAnchor(IDC_COLLECTIONVIEWAUTHORKEYLABEL, BOTTOM_LEFT);
	AddAnchor(IDC_COLLECTIONVIEWCATEGORYCHECK, BOTTOM_LEFT);
	AddAnchor(IDC_COLLECTIONVIEWAUTHOR, BOTTOM_LEFT, BOTTOM_RIGHT);
	AddAnchor(IDC_COLLECTIONVIEWAUTHORKEY, BOTTOM_LEFT, BOTTOM_RIGHT);
	AddAnchor(IDC_VCOLL_CLOSE, BOTTOM_RIGHT);
	AddAnchor(IDC_VIEWCOLLECTIONDL, BOTTOM_RIGHT);


	POSITION pos = m_pCollection->m_CollectionFilesMap.GetStartPosition();
	CCollectionFile* pCollectionFile;
	CSKey key;
	while( pos != NULL )
	{
		m_pCollection->m_CollectionFilesMap.GetNextAssoc( pos, key, pCollectionFile );
		int iItem = m_CollectionViewList.InsertItem(LVIF_TEXT|LVIF_PARAM,m_CollectionViewList.GetItemCount(),NULL,0,0,0,(LPARAM)pCollectionFile);
		if (iItem != -1)
		{
			m_CollectionViewList.SetItemText(iItem,colName,pCollectionFile->GetFileName());
			m_CollectionViewList.SetItemText(iItem,colSize,CastItoXBytes(pCollectionFile->GetFileSize()));
			m_CollectionViewList.SetItemText(iItem,colHash,::md4str(pCollectionFile->GetFileHash()));
		}
	}

	int iItem = m_CollectionViewList.GetItemCount();

	while(iItem)
		m_CollectionViewList.SetItemState(--iItem, LVIS_SELECTED, LVIS_SELECTED);

	return TRUE;
}

void CCollectionViewDialog::OnNMDblclkCollectionvewlist(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	DownloadSelected();
	*pResult = 0;
}

void CCollectionViewDialog::DownloadSelected(void)
{
	int iNewIndex = 0;
	for (int iIndex = 1; iIndex < thePrefs.GetCatCount(); iIndex++)
	{
		if(!m_pCollection->m_sCollectionName.CompareNoCase(thePrefs.GetCategory(iIndex)->strTitle))
		{
			iNewIndex = iIndex;
			break;
		}
	}

	if(m_AddNewCatagory.GetCheck() && !iNewIndex)
	{
		iNewIndex = theApp.emuledlg->transferwnd->AddCategory(m_pCollection->m_sCollectionName, thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR), _T(""), _T(""), true);
		theApp.emuledlg->searchwnd->UpdateCatTabs();
	}

	CTypedPtrList<CPtrList, CCollectionFile*> collectionFileList;
	POSITION pos = m_CollectionViewList.GetFirstSelectedItemPosition();
	while (pos != NULL)
	{
		int index = m_CollectionViewList.GetNextSelectedItem(pos);
		if (index >= 0)
			collectionFileList.AddTail((CCollectionFile*)m_CollectionViewList.GetItemData(index));
	}

	while (collectionFileList.GetCount() > 0)
	{
		CCollectionFile* pCollectionFile = collectionFileList.RemoveHead();
		if(pCollectionFile)
			//MODIFIED by fengwen on 2007/02/09	<begin> :
			//CGlobalVariable::downloadqueue->AddSearchToDownload(::CreateED2kLink(pCollectionFile, true), thePrefs.AddNewFilesPaused(), iNewIndex);
			CmdFuncs::AddSearchToDownload(::CreateED2kLink(pCollectionFile, true), thePrefs.AddNewFilesPaused(), iNewIndex);
			//MODIFIED by fengwen on 2007/02/09	<end> :
	}
}

void CCollectionViewDialog::OnBnClickedViewcollectiondl()
{
	DownloadSelected();
	OnBnClickedOk();
}

void CCollectionViewDialog::OnBnClickedOk()
{
	OnOK();
}
