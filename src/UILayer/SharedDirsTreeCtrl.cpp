/* 
 * $Id: SharedDirsTreeCtrl.cpp 5170 2008-03-28 07:12:43Z thilon $
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

#include "stdafx.h"
#include "emule.h"
#include "SharedDirsTreeCtrl.h"
#include "preferences.h"
#include "otherfunctions.h"
#include "SharedFilesCtrl.h"
#include "Knownfile.h"
#include "MenuCmds.h"
#include "partfile.h"
#include "webservices.h"
#include "emuledlg.h"
#include "TransferWnd.h"
#include "SharedFileList.h"
#include "SharedFilesWnd.h"
#include "UILayer/ShareFileSelect.h"
#include ".\shareddirstreectrl.h"
#include "cif.h"

#include "CreateCheckboxImageList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//**********************************************************************************
// CDirectoryItem

CDirectoryItem::CDirectoryItem(CString strFullPath, HTREEITEM htItem, ESpecialDirectoryItems eItemType, int nCatFilter){
	m_htItem = htItem;
	m_strFullPath = strFullPath;
	m_eItemType = eItemType;
	m_nCatFilter = nCatFilter;
}
	
CDirectoryItem::~CDirectoryItem(){
	while (liSubDirectories.GetHeadPosition() != NULL){
		delete liSubDirectories.RemoveHead();
	}
}

// search tree for a given filter
HTREEITEM CDirectoryItem::FindItem(CDirectoryItem* pContentToFind) const
{
	if (pContentToFind == NULL){
		ASSERT( false );
		return NULL;
	}

	if (pContentToFind->m_eItemType == m_eItemType && pContentToFind->m_strFullPath == m_strFullPath && pContentToFind->m_nCatFilter == m_nCatFilter)
		return m_htItem;

	POSITION pos = liSubDirectories.GetHeadPosition();
	while (pos != NULL){
		CDirectoryItem* pCurrent = liSubDirectories.GetNext(pos);
		HTREEITEM htResult;
		if ( (htResult = pCurrent->FindItem(pContentToFind)) != NULL)
			return htResult;
	}
	return NULL;
}

//**********************************************************************************
// CSharedDirsTreeCtrl


IMPLEMENT_DYNAMIC(CSharedDirsTreeCtrl, CTreeCtrl)

BEGIN_MESSAGE_MAP(CSharedDirsTreeCtrl, CTreeCtrl)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_CANCELMODE()
	ON_WM_LBUTTONUP()
	ON_WM_SYSCOLORCHANGE()
	ON_NOTIFY_REFLECT(TVN_ITEMEXPANDING, OnTvnItemexpanding)
	ON_NOTIFY_REFLECT(TVN_GETDISPINFO, OnTvnGetdispinfo)
	ON_NOTIFY_REFLECT(TVN_BEGINDRAG, OnLvnBegindrag)
	ON_WM_DRAWITEM()
	ON_WM_MEASUREITEM()
	ON_WM_LBUTTONDOWN()
	ON_NOTIFY_REFLECT(NM_CLICK, OnNMClick)
END_MESSAGE_MAP()

CSharedDirsTreeCtrl::CSharedDirsTreeCtrl()
{
	m_pRootDirectoryItem = NULL;
	m_bCreatingTree = false;
	m_pSharedFilesCtrl = NULL;
	m_pRootUnsharedDirectries = NULL;
	m_pDraggingItem = NULL;
	m_pMenuXP = NULL;
}

CSharedDirsTreeCtrl::~CSharedDirsTreeCtrl()
{
	delete m_pRootDirectoryItem;
	delete m_pRootUnsharedDirectries;

	if (m_StateImage.GetSafeHandle())
	{
		m_StateImage.DeleteImageList();
	}

	if(m_pMenuXP)
	{
		delete m_pMenuXP;
	}
}

void CSharedDirsTreeCtrl::Initalize(CSharedFilesCtrl* pSharedFilesCtrl){
	m_pSharedFilesCtrl = pSharedFilesCtrl;
	
	SendMessage(CCM_SETUNICODEFORMAT, TRUE);

	//SetImageList(NULL, TVSIL_STATE);

	//WORD wWinVer = thePrefs.GetWindowsVersion();
	m_bUseIcons = true;/*(wWinVer == _WINVER_2K_ || wWinVer == _WINVER_XP_ || wWinVer == _WINVER_ME_);*/
	SetAllIcons();
	InitalizeStandardItems();
	FilterTreeReloadTree();
	CreateCheckboxImages();	//Added by thilon on 2008.02.18
	//CreateMenues();
}

void CSharedDirsTreeCtrl::OnSysColorChange()
{
	CTreeCtrl::OnSysColorChange();
	SetAllIcons();
	//CreateMenues();
}

void CSharedDirsTreeCtrl::SetAllIcons()
{
	if (m_StateImage.GetSafeHandle())
	{
		m_StateImage.DeleteImageList();
	}

	CImageList iml;
	iml.Create(16, 16, theApp.m_iDfltImageListColorFlags | ILC_MASK, 0, 1);
	iml.Add(CTempIconLoader(_T("ALLFILES")));		// All Directory
	iml.Add(CTempIconLoader(_T("INCOMPLETE")));		// Temp Directory
	iml.Add(CTempIconLoader(_T("INCOMINGFILES")));		// Incoming Directory
	iml.Add(CTempIconLoader(_T("CATEGORY")));		// Cats
	iml.Add(CTempIconLoader(_T("FILESYSTEM")));     // Shared files // VC-SearchDream[2007-02-12]:
	iml.Add(CTempIconLoader(_T("HARDDISK")));		// All Dirs
	
	CString strTempDir = thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR);
	if (strTempDir.Right(1) != "\\"){
		strTempDir += "\\";
	}
	//MODIFIED by fengwen on 2007/01/17	<begin> :	提升启动速度
	//int nImage = theApp.GetFileTypeSystemImageIdx(strTempDir); // System Folder Icon
	//if (theApp.GetSystemImageList() != NULL){
	//	iml.Add(::ImageList_GetIcon(theApp.GetSystemImageList(), nImage, 0));
	//}
	//else{
	//	iml.Add(CTempIconLoader(_T("OPENFOLDER")));
	//}
	iml.Add(CTempIconLoader(_T("SHAREDIRS")));
	//MODIFIED by fengwen on 2007/01/17	<end> :	提升启动速度
	iml.SetOverlayImage(iml.Add(CTempIconLoader(_T("ClientSecureOvl"))), 1);

	iml.Add(CTempIconLoader(_T("SHAREFILES")));

	SetImageList(&iml, TVSIL_NORMAL);
	m_imlTree.DeleteImageList();
	m_imlTree.Attach(iml.Detach());

	COLORREF crBk = GetSysColor(COLOR_WINDOW);
	COLORREF crFg = GetSysColor(COLOR_WINDOWTEXT);
	theApp.LoadSkinColorAlt(_T("SharedDirsTvBk"), _T("DefLvBk"), crBk);
	theApp.LoadSkinColorAlt(_T("SharedDirsTvFg"), _T("DefLvFg"), crFg);
	SetBkColor(crBk);
	SetTextColor(crFg);
}

void CSharedDirsTreeCtrl::Localize(){
	InitalizeStandardItems();
	FilterTreeReloadTree();
	//CreateMenues();
}

void CSharedDirsTreeCtrl::InitalizeStandardItems(){
	// add standard items
	DeleteAllItems();
	delete m_pRootDirectoryItem;
	delete m_pRootUnsharedDirectries;

	FetchSharedDirsList();

	m_pRootDirectoryItem = new CDirectoryItem(CString(""), TVI_ROOT);
	CDirectoryItem* pAll = new CDirectoryItem(CString(""), 0, SDI_ALL);
	pAll->m_htItem = InsertItem(TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE, GetResString(IDS_ALLSHAREDFILES), 0, 0, 0, 0, (LPARAM)pAll, TVI_ROOT, TVI_LAST);
	SetItemState( pAll->m_htItem, INDEXTOSTATEIMAGEMASK(0), TVIS_STATEIMAGEMASK );
	m_pRootDirectoryItem->liSubDirectories.AddTail(pAll);
	
	CDirectoryItem* pIncoming = new CDirectoryItem(CString(""), TVI_ROOT, SDI_INCOMING);
	pIncoming->m_htItem = InsertItem(TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE, GetResString(IDS_INCOMING_FILES), 2, 2, 0, 0, (LPARAM)pIncoming, TVI_ROOT, TVI_LAST);
	SetItemState( pIncoming->m_htItem, INDEXTOSTATEIMAGEMASK(0), TVIS_STATEIMAGEMASK );
	m_pRootDirectoryItem->liSubDirectories.AddTail(pIncoming);
	
	CDirectoryItem* pTemp = new CDirectoryItem(CString(""), TVI_ROOT, SDI_TEMP);
	pTemp->m_htItem = InsertItem(TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE, GetResString(IDS_INCOMPLETE_FILES), 1, 1, 0, 0, (LPARAM)pTemp, TVI_ROOT, TVI_LAST);
	SetItemState( pTemp->m_htItem, INDEXTOSTATEIMAGEMASK(0), TVIS_STATEIMAGEMASK );
	m_pRootDirectoryItem->liSubDirectories.AddTail(pTemp);

	/*CDirectoryItem* pDir = new CDirectoryItem(CString(""), TVI_ROOT, SDI_DIRECTORY);
	pDir->m_htItem = InsertItem(TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE, GetResString(IDS_SHARED_DIRECTORIES), 6, 6, TVIS_EXPANDED, TVIS_EXPANDED, (LPARAM)pDir, TVI_ROOT, TVI_LAST);
	m_pRootDirectoryItem->liSubDirectories.AddTail(pDir);*/

#ifdef _ENABLE_SHARE_FILES
    // VC-SearchDream[2007-02-12]: Add for shared files
	CDirectoryItem* pSharedFiles = new CDirectoryItem(CString(""), TVI_ROOT, SDI_SHAREEDFILES);
	pSharedFiles->m_htItem = InsertItem(TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE, GetResString(IDS_SHARED_FILES), 8, 8, 0, 0, (LPARAM)pSharedFiles, TVI_ROOT, TVI_LAST);
	m_pRootDirectoryItem->liSubDirectories.AddTail(pSharedFiles);
#endif

	m_pRootUnsharedDirectries = new CDirectoryItem(CString(""), TVI_ROOT, SDI_FILESYSTEMPARENT);
	m_pRootUnsharedDirectries->m_htItem = InsertItem(TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN, GetResString(IDS_ALLDIRECTORIES), 4, 4, 0, 0, (LPARAM)m_pRootUnsharedDirectries, TVI_ROOT, TVI_LAST);
	SetItemState( m_pRootUnsharedDirectries->m_htItem, INDEXTOSTATEIMAGEMASK(0), TVIS_STATEIMAGEMASK );
}

bool CSharedDirsTreeCtrl::FilterTreeIsSubDirectory(CString strDir, CString strRoot, CStringList& liDirs){
	POSITION pos = liDirs.GetHeadPosition();
	strRoot.MakeLower();
	strDir.MakeLower();
	if (strDir.Right(1) != _T("\\")){
		strDir += _T("\\");
	}
	if (strRoot.Right(1) != _T("\\")){
		strRoot += _T("\\");
	}
	while (pos){
		CString strCurrent = thePrefs.shareddir_list.GetNext(pos);
		strCurrent.MakeLower();
		if (strCurrent.Right(1) != _T("\\")){
			strCurrent += _T("\\");
		}
		if (strRoot.Find(strCurrent, 0) != 0 && strDir.Find(strCurrent, 0) == 0 && strCurrent != strRoot && strCurrent != strDir)
			return true;
	}
	return false;
}

void CSharedDirsTreeCtrl::FilterTreeAddSubDirectories(CDirectoryItem* pDirectory, CStringList& liDirs, int nLevel){
	// just some sanity check against too deep shared dirs
	// shouldnt be needed, but never trust the filesystem or a recursive function ;)
	if (nLevel > 14){
		ASSERT( false );
		return;
	}
	POSITION pos = liDirs.GetHeadPosition();
	CString strDirectoryPath = pDirectory->m_strFullPath;
	strDirectoryPath.MakeLower();
	while (pos){
		CString strCurrent = thePrefs.shareddir_list.GetNext(pos);
		CString strCurrentLow = strCurrent;
		strCurrentLow.MakeLower();
		if ( (strDirectoryPath.IsEmpty() || strCurrentLow.Find(strDirectoryPath + _T("\\"), 0) == 0) && strCurrentLow != strDirectoryPath){
			if (!FilterTreeIsSubDirectory(strCurrentLow, strDirectoryPath, liDirs)){
				CString strName = strCurrent;
				if (strName.Right(1) == "\\"){
					strName = strName.Left(strName.GetLength()-1);
				}
				strName = strName.Right(strName.GetLength() - (strName.ReverseFind('\\')+1));
				CDirectoryItem* pNewItem = new CDirectoryItem(strCurrent);
				pNewItem->m_htItem = InsertItem(TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE, strName, 5, 5, 0, 0, (LPARAM)pNewItem, pDirectory->m_htItem, TVI_LAST);
				pDirectory->liSubDirectories.AddTail(pNewItem);
				FilterTreeAddSubDirectories(pNewItem, liDirs, nLevel+1);
		
			}
		}
	}
}


void CSharedDirsTreeCtrl::FilterTreeReloadTree()
{
	m_bCreatingTree = true;
	// store current selection
	CDirectoryItem* pOldSelectedItem = NULL;
	if (GetSelectedFilter() != NULL)
	{
		pOldSelectedItem = GetSelectedFilter()->CloneContent();
	}


	// create the tree substructure of directories we want to show
	POSITION pos = m_pRootDirectoryItem->liSubDirectories.GetHeadPosition();
	while (pos != NULL){
		CDirectoryItem* pCurrent = m_pRootDirectoryItem->liSubDirectories.GetNext(pos);
		// clear old items
		DeleteChildItems(pCurrent);

		switch( pCurrent->m_eItemType ){

			case SDI_ALL:
				break;
			case SDI_INCOMING:{
				CString strMainIncDir = thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR);
				if (strMainIncDir.Right(1) == "\\"){
					strMainIncDir = strMainIncDir.Left(strMainIncDir.GetLength()-1);
				}
				if (thePrefs.GetCatCount() > 1){
					m_strliCatIncomingDirs.RemoveAll();
					for (int i = 0; i < thePrefs.GetCatCount(); i++){
						Category_Struct* pCatStruct = thePrefs.GetCategory(i);
						if (pCatStruct != NULL){
							CString strCatIncomingPath = pCatStruct->strIncomingPath;
							if (strCatIncomingPath.Right(1) == "\\"){
								strCatIncomingPath = strCatIncomingPath.Left(strCatIncomingPath.GetLength()-1);
							}
							if (!strCatIncomingPath.IsEmpty() && strCatIncomingPath.CompareNoCase(strMainIncDir) != 0
								&& m_strliCatIncomingDirs.Find(strCatIncomingPath) == NULL)
							{
								m_strliCatIncomingDirs.AddTail(strCatIncomingPath);
								CString strName = strCatIncomingPath;
								if (strName.Right(1) == "\\"){
									strName = strName.Left(strName.GetLength()-1);
								}
								strName = strName.Right(strName.GetLength() - (strName.ReverseFind('\\')+1));
								CDirectoryItem* pCatInc = new CDirectoryItem(strCatIncomingPath, 0, SDI_CATINCOMING);
								pCatInc->m_htItem = InsertItem(TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE, strName, 5, 5, 0, 0, (LPARAM)pCatInc, pCurrent->m_htItem, TVI_LAST);
								pCurrent->liSubDirectories.AddTail(pCatInc);
							}
						}
					}
				}
				break;
			}
			case SDI_TEMP:
				if (thePrefs.GetCatCount() > 1){
					for (int i = 0; i < thePrefs.GetCatCount(); i++){
						Category_Struct* pCatStruct = thePrefs.GetCategory(i);
						if (pCatStruct != NULL){
							//temp dir
							CDirectoryItem* pCatTemp = new CDirectoryItem(CString(""), 0, SDI_TEMP, i);
							pCatTemp->m_htItem = InsertItem(TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE, CString(pCatStruct->strTitle), 3, 3, 0, 0, (LPARAM)pCatTemp, pCurrent->m_htItem, TVI_LAST);
							pCurrent->liSubDirectories.AddTail(pCatTemp);

						}
					}
				}
				break;
			case SDI_DIRECTORY:
				// add subdirectories
				FilterTreeAddSubDirectories(pCurrent, m_strliSharedDirs);
				break;
#ifdef _ENABLE_SHARE_FILES
			case SDI_SHAREEDFILES:
				{
				
				}
				break;
#endif
			default:
				ASSERT( false );
		}
	}

	// restore selection
	HTREEITEM htOldSection;
	if (pOldSelectedItem != NULL && (htOldSection = m_pRootDirectoryItem->FindItem(pOldSelectedItem)) != NULL){
		Select(htOldSection, TVGN_CARET);
		EnsureVisible(htOldSection);
	}
	else if( GetSelectedItem() == NULL && !m_pRootDirectoryItem->liSubDirectories.IsEmpty()){
		Select(m_pRootDirectoryItem->liSubDirectories.GetHead()->m_htItem, TVGN_CARET);
	}
	delete pOldSelectedItem;
	m_bCreatingTree = false;
}

CDirectoryItem* CSharedDirsTreeCtrl::GetSelectedFilter() const{
	if (GetSelectedItem() != NULL)
		return (CDirectoryItem*)GetItemData(GetSelectedItem());
	else
		return NULL;
}

void CSharedDirsTreeCtrl::CreateMenues()
{
	if (m_PrioMenu) VERIFY( m_PrioMenu.DestroyMenu() );
	if (m_SharedFilesMenu) VERIFY( m_SharedFilesMenu.DestroyMenu() );
	if (m_ShareDirsMenu) VERIFY( m_ShareDirsMenu.DestroyMenu() );

	

	m_PrioMenu.CreateMenu();
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOVERYLOW,GetResString(IDS_PRIOVERYLOW));
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOLOW,GetResString(IDS_PRIOLOW));
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIONORMAL,GetResString(IDS_PRIONORMAL));
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOHIGH, GetResString(IDS_PRIOHIGH));
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOVERYHIGH, GetResString(IDS_PRIORELEASE));
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOAUTO, GetResString(IDS_PRIOAUTO));//UAP

	m_SharedFilesMenu.CreatePopupMenu();
	//m_SharedFilesMenu.AddMenuTitle(GetResString(IDS_SHAREDFILES), true);
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_OPENFOLDER, GetResString(IDS_OPENFOLDER), _T("OPENFOLDER"));
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_REMOVE, GetResString(IDS_DELETE_FILE), _T("DELETE"));
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_SEPARATOR);
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_PrioMenu.m_hMenu, GetResString(IDS_PRIORITY) + _T(" (") + GetResString(IDS_PW_CON_UPLBL) + _T(")"), _T("FILEPRIORITY"));

	cif.AddIcon((UINT_PTR)m_PrioMenu.m_hMenu, theApp.LoadIcon(_T("FILEPRIORITY")));
	cif.AddIcon((UINT_PTR)m_PrioMenu.m_hMenu, theApp.LoadIcon(_T("FILEPRIORITY")), TRUE);

	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_SEPARATOR);	
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_DETAIL, GetResString(IDS_SHOWDETAILS), _T("FILEINFO"));
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_CMT, GetResString(IDS_CMT_ADD), _T("FILECOMMENTS")); 

	if (thePrefs.GetShowCopyEd2kLinkCmd())
		m_SharedFilesMenu.AppendMenu(MF_STRING,MP_GETED2KLINK, GetResString(IDS_DL_LINK1), _T("ED2KLINK") );
	else
		m_SharedFilesMenu.AppendMenu(MF_STRING,MP_SHOWED2KLINK, GetResString(IDS_DL_SHOWED2KLINK), _T("ED2KLINK") );
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_SEPARATOR);

	m_ShareDirsMenu.CreatePopupMenu();
	//m_ShareDirsMenu.AddMenuTitle(GetResString(IDS_SHAREDFILES), false);
	m_ShareDirsMenu.AppendMenu(MF_STRING,MP_SHAREDIR,GetResString(IDS_SHAREDIR));
	m_ShareDirsMenu.AppendMenu(MF_STRING,MP_SHAREDIRSUB,GetResString(IDS_SHAREDIRSUB));
#ifdef _ENABLE_SHARE_FILES
	m_ShareDirsMenu.AppendMenu(MF_STRING,MP_SHAREFILES, GetResString(IDS_SHAREDFILES));
#endif
	m_ShareDirsMenu.AppendMenu(MF_STRING|MF_SEPARATOR);	
	m_ShareDirsMenu.AppendMenu(MF_STRING,MP_UNSHAREDIR,GetResString(IDS_UNSHAREDIR));
	m_ShareDirsMenu.AppendMenu(MF_STRING,MP_UNSHAREDIRSUB,GetResString(IDS_UNSHAREDIRSUB));
	m_ShareDirsMenu.AppendMenu(MF_STRING|MF_SEPARATOR);	
	m_ShareDirsMenu.AppendMenu(MF_STRING,MP_OPENFOLDER,GetResString(IDS_OPENFOLDER));
}

void CSharedDirsTreeCtrl::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{	
	CreateMenues();
	CDirectoryItem* pSelectedDir = GetSelectedFilter();
	if (pSelectedDir != NULL && pSelectedDir->m_eItemType != SDI_UNSHAREDDIRECTORY && pSelectedDir->m_eItemType != SDI_FILESYSTEMPARENT)
	{
		int iSelectedItems = m_pSharedFilesCtrl->GetItemCount();
		int iCompleteFileSelected = -1;
		UINT uPrioMenuItem = 0;
		bool bFirstItem = true;
		for (int i = 0; i < iSelectedItems; i++)
		{
			const CKnownFile* pFile = (CKnownFile*)m_pSharedFilesCtrl->GetItemData(i);

			int iCurCompleteFile = pFile->IsPartFile() ? 0 : 1;
			if (bFirstItem)
				iCompleteFileSelected = iCurCompleteFile;
			else if (iCompleteFileSelected != iCurCompleteFile)
				iCompleteFileSelected = -1;

			UINT uCurPrioMenuItem = 0;
			if (pFile->IsAutoUpPriority())
				uCurPrioMenuItem = MP_PRIOAUTO;
			else if (pFile->GetUpPriority() == PR_VERYLOW)
				uCurPrioMenuItem = MP_PRIOVERYLOW;
			else if (pFile->GetUpPriority() == PR_LOW)
				uCurPrioMenuItem = MP_PRIOLOW;
			else if (pFile->GetUpPriority() == PR_NORMAL)
				uCurPrioMenuItem = MP_PRIONORMAL;
			else if (pFile->GetUpPriority() == PR_HIGH)
				uCurPrioMenuItem = MP_PRIOHIGH;
			else if (pFile->GetUpPriority() == PR_VERYHIGH)
				uCurPrioMenuItem = MP_PRIOVERYHIGH;
			else
				ASSERT(0);

			if (bFirstItem)
				uPrioMenuItem = uCurPrioMenuItem;
			else if (uPrioMenuItem != uCurPrioMenuItem)
				uPrioMenuItem = 0;

			bFirstItem = false;
		}

		bool bWideRangeSelection = true;
		if(pSelectedDir->m_nCatFilter != -1 || pSelectedDir->m_eItemType == SDI_NO)
		{
			// just avoid that users get bad ideas by showing the comment/delete-option for the "all" selections
			// as the same comment for all files/all incimplete files/ etc is probably not too usefull
			// - even if it can be done in other ways if the user really wants to do it
			bWideRangeSelection = false;
		}

		m_SharedFilesMenu.EnableMenuItem((UINT_PTR)m_PrioMenu.m_hMenu, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
		m_PrioMenu.CheckMenuRadioItem(MP_PRIOVERYLOW, MP_PRIOAUTO, uPrioMenuItem, 0);

		m_SharedFilesMenu.EnableMenuItem(MP_OPENFOLDER, (pSelectedDir != NULL && (!pSelectedDir->m_strFullPath.IsEmpty())) ? MF_ENABLED : MF_GRAYED);
		m_SharedFilesMenu.EnableMenuItem(MP_REMOVE, (iCompleteFileSelected > 0 && !bWideRangeSelection) ? MF_ENABLED : MF_GRAYED);
		m_SharedFilesMenu.EnableMenuItem(MP_CMT, (iSelectedItems > 0 && !bWideRangeSelection) ? MF_ENABLED : MF_GRAYED);
		m_SharedFilesMenu.EnableMenuItem(MP_DETAIL, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
		m_SharedFilesMenu.EnableMenuItem(thePrefs.GetShowCopyEd2kLinkCmd() ? MP_GETED2KLINK : MP_SHOWED2KLINK, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);


		CTitleMenu WebMenu;
		WebMenu.CreateMenu();
		WebMenu.AddMenuTitle(NULL, true);
		int iWebMenuEntries = theWebServices.GetFileMenuEntries(&WebMenu);
		UINT flag2 = (iWebMenuEntries == 0 || iSelectedItems != 1) ? MF_GRAYED : MF_STRING;
		m_SharedFilesMenu.AppendMenu(flag2 | MF_POPUP, (UINT_PTR)WebMenu.m_hMenu, GetResString(IDS_WEBSERVICES), _T("WEB"));


		m_pMenuXP = new CMenuXP();
		m_pMenuXP->AddMenu(&m_SharedFilesMenu, TRUE);

		GetPopupMenuPos(*this, point);
		m_SharedFilesMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON,point.x,point.y,this);

		delete m_pMenuXP;
		m_pMenuXP = NULL;

		m_SharedFilesMenu.RemoveMenu(m_SharedFilesMenu.GetMenuItemCount()-1,MF_BYPOSITION);
		VERIFY( WebMenu.DestroyMenu() );
	}
	else if(pSelectedDir != NULL && pSelectedDir->m_eItemType == SDI_UNSHAREDDIRECTORY){
		m_ShareDirsMenu.EnableMenuItem(MP_UNSHAREDIR, FileSystemTreeIsShared(pSelectedDir->m_strFullPath) ? MF_ENABLED : MF_GRAYED);
		m_ShareDirsMenu.EnableMenuItem(MP_UNSHAREDIRSUB, (FileSystemTreeIsShared(pSelectedDir->m_strFullPath) || FileSystemTreeHasSharedSubdirectory(pSelectedDir->m_strFullPath)) ? MF_ENABLED : MF_GRAYED);
		m_ShareDirsMenu.EnableMenuItem(MP_SHAREDIR, !FileSystemTreeIsShared(pSelectedDir->m_strFullPath) ? MF_ENABLED : MF_GRAYED);
		m_ShareDirsMenu.EnableMenuItem(MP_SHAREDIRSUB, FileSystemTreeHasSubdirectories(pSelectedDir->m_strFullPath) ? MF_ENABLED : MF_GRAYED);

		m_pMenuXP = new CMenuXP();
		m_pMenuXP->AddMenu(&m_ShareDirsMenu, TRUE);

		GetPopupMenuPos(*this, point);
		m_ShareDirsMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON,point.x,point.y,this);

		delete m_pMenuXP;
		m_pMenuXP = NULL;
	}
}

void CSharedDirsTreeCtrl::OnRButtonDown(UINT /*nFlags*/, CPoint point)
{
	UINT uHitFlags;
	HTREEITEM hItem = HitTest(point, &uHitFlags);
	if (hItem != NULL && (uHitFlags & TVHT_ONITEM))
	{
		Select(hItem, TVGN_CARET);
		SetItemState(hItem, TVIS_SELECTED, TVIS_SELECTED);
	}
	return;
}

BOOL CSharedDirsTreeCtrl::OnCommand(WPARAM wParam, LPARAM /*lParam*/)
{
	CTypedPtrList<CPtrList, CKnownFile*> selectedList;
	int iSelectedItems = m_pSharedFilesCtrl->GetItemCount();
	for (int i = 0; i < iSelectedItems; i++)
	{
		selectedList.AddTail((CKnownFile*)m_pSharedFilesCtrl->GetItemData(i));
	}
	CDirectoryItem* pSelectedDir = GetSelectedFilter();

	// folder based
	if (pSelectedDir != NULL){
		switch (wParam){
			case MP_OPENFOLDER:
				if (pSelectedDir && !pSelectedDir->m_strFullPath.IsEmpty() /*&& pSelectedDir->m_eItemType == SDI_NO*/){
					ShellExecute(NULL, _T("open"), pSelectedDir->m_strFullPath, NULL, NULL, SW_SHOW);
				}
				break;
			case MP_SHAREDIR:
				EditSharedDirectories(pSelectedDir, true, false);
				break;
			case MP_SHAREDIRSUB:
				EditSharedDirectories(pSelectedDir, true, true);
				break;
			case MP_UNSHAREDIR:
				EditSharedDirectories(pSelectedDir, false, false);
				break;
			case MP_UNSHAREDIRSUB:
				EditSharedDirectories(pSelectedDir, false, true);
				break;
			case MP_SHAREFILES:
				{
					CShareFileSelect  *pFile = new CShareFileSelect();
					pFile->Create(CShareFileSelect::IDD);
					pFile->ShowWindow(SW_SHOW);
					pFile->ShowFile(pSelectedDir->m_strFullPath);
				}
				break;
		}
	}

	// file based
	if (selectedList.GetCount() > 0 && pSelectedDir != NULL)
	{
		CKnownFile* file = NULL;
		if (selectedList.GetCount() == 1)
			file = selectedList.GetHead();

		switch (wParam){
			case MP_GETED2KLINK:{
				CString str;
				POSITION pos = selectedList.GetHeadPosition();
				while (pos != NULL)
				{
					file = selectedList.GetNext(pos);
					if (!str.IsEmpty())
						str += _T("\r\n");
					str += CreateED2kLink(file);
				}
				theApp.CopyTextToClipboard(str);
				break;
			}
			// file operations
			case MP_REMOVE:
			case MPG_DELETE:{
				  if(IDNO == MessageBox(GetResString(IDS_CONFIRM_FILEDELETE),GetResString(IDS_CAPTION),MB_ICONWARNING | MB_ICONQUESTION | MB_DEFBUTTON2 | MB_YESNO))
					return TRUE;

				m_pSharedFilesCtrl->SetRedraw(FALSE);
				bool bRemovedItems = false;
				while (!selectedList.IsEmpty())
				{
					CKnownFile* myfile = selectedList.RemoveHead();
					if (!myfile || myfile->IsPartFile())
						continue;
					
					BOOL delsucc = FALSE;
					if (!PathFileExists(myfile->GetFilePath()))
						delsucc = TRUE;
					else{
						// Delete
						if (!thePrefs.GetRemoveToBin()){
							delsucc = DeleteFile(myfile->GetFilePath());
						}
						else{
							// delete to recycle bin :(
							TCHAR todel[MAX_PATH+1];
							memset(todel, 0, sizeof todel);
							_tcsncpy(todel, myfile->GetFilePath(), ARRSIZE(todel)-2);

							SHFILEOPSTRUCT fp = {0};
							fp.wFunc = FO_DELETE;
							fp.hwnd = theApp.emuledlg->m_hWnd;
							fp.pFrom = todel;
							fp.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_SILENT;// | FOF_NOERRORUI
							delsucc = (SHFileOperation(&fp) == 0);
						}
					}
					if (delsucc){
						CGlobalVariable::sharedfiles->RemoveFile(myfile);
						bRemovedItems = true;
						if (myfile->IsKindOf(RUNTIME_CLASS(CPartFile)))
							theApp.emuledlg->transferwnd->downloadlistctrl.ClearCompleted(static_cast<CPartFile*>(myfile));
					}
					else{
						CString strError;
						strError.Format( GetResString(IDS_ERR_DELFILE) + _T("\r\n\r\n%s"), myfile->GetFilePath(), GetErrorMessage(GetLastError()));
						AfxMessageBox(strError);
					}
				}
				m_pSharedFilesCtrl->SetRedraw(TRUE);
				if (bRemovedItems)
					m_pSharedFilesCtrl->AutoSelectItem();
				break; 
			}
			case MP_CMT:
				ShowFileDialog(selectedList, IDD_COMMENT);
                break; 
			case MP_DETAIL:
			case MPG_ALTENTER:
				ShowFileDialog(selectedList);
				break;
			case MP_SHOWED2KLINK:
				ShowFileDialog(selectedList, IDD_ED2KLINK);
				break;
			case MP_PRIOVERYLOW:
			case MP_PRIOLOW:
			case MP_PRIONORMAL:
			case MP_PRIOHIGH:
			case MP_PRIOVERYHIGH:
			case MP_PRIOAUTO:
				{
					POSITION pos = selectedList.GetHeadPosition();
					while (pos != NULL)
					{
						CKnownFile* file = selectedList.GetNext(pos);
						switch (wParam) {
							case MP_PRIOVERYLOW:
								file->SetAutoUpPriority(false);
								file->SetUpPriority(PR_VERYLOW);
								m_pSharedFilesCtrl->UpdateFile(file);
								break;
							case MP_PRIOLOW:
								file->SetAutoUpPriority(false);
								file->SetUpPriority(PR_LOW);
								m_pSharedFilesCtrl->UpdateFile(file);
								break;
							case MP_PRIONORMAL:
								file->SetAutoUpPriority(false);
								file->SetUpPriority(PR_NORMAL);
								m_pSharedFilesCtrl->UpdateFile(file);
								break;
							case MP_PRIOHIGH:
								file->SetAutoUpPriority(false);
								file->SetUpPriority(PR_HIGH);
								m_pSharedFilesCtrl->UpdateFile(file);
								break;
							case MP_PRIOVERYHIGH:
								file->SetAutoUpPriority(false);
								file->SetUpPriority(PR_VERYHIGH);
								m_pSharedFilesCtrl->UpdateFile(file);
								break;	
							case MP_PRIOAUTO:
								file->SetAutoUpPriority(true);
								file->UpdateAutoUpPriority();
								m_pSharedFilesCtrl->UpdateFile(file); 
								break;
						}
					}
					break;
				}
			default:
				if (wParam>=MP_WEBURL && wParam<=MP_WEBURL+256){
					theWebServices.RunURL(file, wParam);
				}
				break;
		}
	}
	return TRUE;
}

void CSharedDirsTreeCtrl::ShowFileDialog(CTypedPtrList<CPtrList, CKnownFile*>& aFiles, UINT uPshInvokePage)
{
	m_pSharedFilesCtrl->ShowFileDialog(aFiles, uPshInvokePage);
}

void CSharedDirsTreeCtrl::FileSystemTreeCreateTree()
{
	TCHAR drivebuffer[500];
	::GetLogicalDriveStrings(ARRSIZE(drivebuffer), drivebuffer); // e.g. "a:\ c:\ d:\"

	const TCHAR* pos = drivebuffer;
	while(*pos != _T('\0')){

		// Copy drive name
		TCHAR drive[4];
		_tcsncpy(drive, pos, ARRSIZE(drive));
		drive[ARRSIZE(drive) - 1] = _T('\0');

		switch(drive[0]){
			case _T('a'):
			case _T('A'):
			case _T('b'):
			case _T('B'):
			// Skip floppy disk
			break;
		default:
			drive[2] = _T('\0');
			FileSystemTreeAddChildItem(m_pRootUnsharedDirectries, drive, true); // e.g. ("c:")
		}

		// Point to the next drive (4 chars interval)
		pos = &pos[4];
	}
}

void CSharedDirsTreeCtrl::FileSystemTreeAddChildItem(CDirectoryItem* pRoot, CString strText, bool bTopLevel)
{
	CString strPath = pRoot->m_strFullPath;
	if (strPath.Right(1) != _T("\\") && !strPath.IsEmpty())
		strPath += _T("\\");
	CString strDir = strPath + strText;
	TV_INSERTSTRUCT itInsert;
	memset(&itInsert, 0, sizeof(itInsert));
	
	if(m_bUseIcons)		
	{
		itInsert.item.mask = TVIF_CHILDREN | TVIF_HANDLE | TVIF_TEXT | TVIF_STATE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		itInsert.item.stateMask = TVIS_BOLD | TVIS_STATEIMAGEMASK;
	}
	else
	{
		itInsert.item.mask = TVIF_CHILDREN | TVIF_HANDLE | TVIF_TEXT | TVIF_STATE;
		itInsert.item.stateMask = TVIS_BOLD;
	}

	
	if (FileSystemTreeHasSharedSubdirectory(strDir) || CGlobalVariable::filemgr.HasFileSharedInDir(strDir) || CGlobalVariable::sharedfiles->IsHashing(strDir))
	{
		itInsert.item.state = TVIS_BOLD | INDEXTOSTATEIMAGEMASK(TRISTATE);
		itInsert.item.stateMask = TVIS_BOLD | TVIS_STATEIMAGEMASK;
	}
	else
	{
		itInsert.item.state = 0 | INDEXTOSTATEIMAGEMASK(UNCHECKED);
		itInsert.item.stateMask = TVIS_STATEIMAGEMASK;
	}

	if (FileSystemTreeHasSubdirectories(strDir))
		itInsert.item.cChildren = I_CHILDRENCALLBACK;		// used to display the + symbol next to each item
	else
		itInsert.item.cChildren = 0;

	if (strDir.Right(1) == "\\"){
		strDir = strDir.Left(strPath.GetLength()-1);
	}
	CDirectoryItem* pti = new CDirectoryItem(strDir, 0, SDI_UNSHAREDDIRECTORY);

	itInsert.item.pszText = const_cast<LPTSTR>((LPCTSTR)strText);
	itInsert.hInsertAfter = !bTopLevel ? TVI_SORT : TVI_LAST;
	itInsert.hParent = pRoot->m_htItem;
	itInsert.item.mask |= TVIF_PARAM;
	itInsert.item.lParam = (LPARAM)pti;
	
	if(m_bUseIcons)		
	{
		if (FileSystemTreeIsShared(strDir)){
			itInsert.item.stateMask |= TVIS_OVERLAYMASK;
			itInsert.item.state |= INDEXTOOVERLAYMASK(1);
		}

		CString strTemp = strDir;
		if(strTemp.Right(1) != _T("\\"))
			strTemp += _T("\\");
		
		UINT nType = GetDriveType(strTemp);
		if(DRIVE_REMOVABLE <= nType && nType <= DRIVE_RAMDISK)
			itInsert.item.iImage = nType;
	
		SHFILEINFO shFinfo;
		shFinfo.szDisplayName[0] = _T('\0');
		if(!SHGetFileInfo(strTemp, 0, &shFinfo,	sizeof(shFinfo),
						  SHGFI_ICON | SHGFI_SMALLICON | SHGFI_DISPLAYNAME))
		{
			TRACE(_T("Error Gettting SystemFileInfo!"));
			itInsert.itemex.iImage = 0; // :(
		}
		else
		{
			itInsert.itemex.iImage = AddSystemIcon(shFinfo.hIcon, shFinfo.iIcon);
			DestroyIcon(shFinfo.hIcon);
			if (bTopLevel && shFinfo.szDisplayName[0] != _T('\0'))
			{
				strText = shFinfo.szDisplayName;
				itInsert.item.pszText = const_cast<LPTSTR>((LPCTSTR)strText);
			}
		}

		if(!SHGetFileInfo(strTemp, 0, &shFinfo, sizeof(shFinfo), SHGFI_ICON | SHGFI_OPENICON | SHGFI_SMALLICON))
		{
			TRACE(_T("Error Gettting SystemFileInfo!"));
			itInsert.itemex.iImage = 0;
		}
		else{
			itInsert.itemex.iSelectedImage = AddSystemIcon(shFinfo.hIcon, shFinfo.iIcon);
			DestroyIcon(shFinfo.hIcon);
		}
	}

	pti->m_htItem = InsertItem(&itInsert);
	pRoot->liSubDirectories.AddTail(pti);

	//Added by thilon on 2008.02.18 for Checkbox
	if (FileSystemTreeIsShared(strDir))
	{
		SetCheck(pti->m_htItem);
	}

	strText.ReleaseBuffer();
}

bool CSharedDirsTreeCtrl::FileSystemTreeHasSubdirectories(CString strDir)
{
	if (strDir.Right(1) != _T('\\'))
		strDir += _T('\\');
	CFileFind finder;
	BOOL bWorking = finder.FindFile(strDir+_T("*.*"));
	while (bWorking)
	{
		bWorking = finder.FindNextFile();
		if (finder.IsDots())
			continue;
		if (finder.IsSystem())
			continue;
		if (!finder.IsDirectory())
			continue;
		finder.Close();
		return true;
	}
	finder.Close();
	return false;
}

bool CSharedDirsTreeCtrl::FileSystemTreeHasSharedSubdirectory(CString strDir)
{
	if (strDir.Right(1) != _T('\\'))
		strDir += _T('\\');
	strDir.MakeLower();
	for (POSITION pos = m_strliSharedDirs.GetHeadPosition(); pos != NULL; )
	{
		CString strCurrent = m_strliSharedDirs.GetNext(pos);
		strCurrent.MakeLower();
		if (strCurrent.Find(strDir) == 0 && strDir != strCurrent)
			return true;
	}
	return false;
}

void CSharedDirsTreeCtrl::FileSystemTreeAddSubdirectories(CDirectoryItem* pRoot)
{
	CString strDir = pRoot->m_strFullPath;
	if (strDir.Right(1) != _T("\\"))
		strDir += _T("\\");
	CFileFind finder;
	BOOL bWorking = finder.FindFile(strDir+_T("*.*"));
	while (bWorking)
	{
		bWorking = finder.FindNextFile();
		if (finder.IsDots() || finder.IsSystem() || !finder.IsDirectory() || finder.IsHidden() )
			continue;
		
		CString strFilename = finder.GetFileName();
		if (strFilename.ReverseFind(_T('\\')) != -1)
			strFilename = strFilename.Mid(strFilename.ReverseFind(_T('\\')) + 1);
		FileSystemTreeAddChildItem(pRoot, strFilename, false);
	}
	finder.Close();
}

int	CSharedDirsTreeCtrl::AddSystemIcon(HICON hIcon, int nSystemListPos){
	int nPos = 0;
	if (!m_mapSystemIcons.Lookup(nSystemListPos, nPos)){
		nPos = GetImageList(TVSIL_NORMAL)->Add(hIcon);
		m_mapSystemIcons.SetAt(nSystemListPos, nPos);
	}
	return nPos;
}

void CSharedDirsTreeCtrl::OnTvnItemexpanding(NMHDR *pNMHDR, LRESULT *pResult)
{
	CWaitCursor curWait;
	SetRedraw(FALSE);

	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	CDirectoryItem* pExpanded = (CDirectoryItem*)pNMTreeView->itemNew.lParam;
	if (pExpanded != NULL){
		if (pExpanded->m_eItemType == SDI_UNSHAREDDIRECTORY && !pExpanded->m_strFullPath.IsEmpty()){
			// remove all subitems
			DeleteChildItems(pExpanded);
			// fetch all subdirectories and add them to the node
			FileSystemTreeAddSubdirectories(pExpanded);
		}
		else if(pExpanded->m_eItemType == SDI_FILESYSTEMPARENT){
			DeleteChildItems(pExpanded);
			FileSystemTreeCreateTree();
		}
	}
	else
		ASSERT( false );

	SetRedraw(TRUE);
	Invalidate();
	*pResult = 0;
}

void CSharedDirsTreeCtrl::DeleteChildItems(CDirectoryItem* pParent){
	while(!pParent->liSubDirectories.IsEmpty()){
		CDirectoryItem* pToDelete = pParent->liSubDirectories.RemoveHead();
		DeleteItem(pToDelete->m_htItem);
		DeleteChildItems(pToDelete);
		delete pToDelete;
	}
}

bool CSharedDirsTreeCtrl::FileSystemTreeIsShared(CString strDir)
{
	if (strDir.Right(1) != _T('\\'))
		strDir += _T('\\');
	for (POSITION pos = m_strliSharedDirs.GetHeadPosition(); pos != NULL; )
	{
		CString str = m_strliSharedDirs.GetNext(pos);
		if (str.Right(1) != _T('\\'))
			str += _T('\\');
		if (str.CompareNoCase(strDir) == 0)
			return true;
	}
	return false;
}

void CSharedDirsTreeCtrl::OnTvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTVDISPINFO pTVDispInfo = reinterpret_cast<LPNMTVDISPINFO>(pNMHDR);
	pTVDispInfo->item.cChildren = 1;
	*pResult = 0;
}

void CSharedDirsTreeCtrl::AddSharedDirectory(CString strDir, bool bSubDirectories){
	if (!FileSystemTreeIsShared(strDir)){
		m_strliSharedDirs.AddTail(strDir);
	}
	if (bSubDirectories){
		if (strDir.Right(1) != _T("\\"))
			strDir += _T("\\");
		CFileFind finder;
		BOOL bWorking = finder.FindFile(strDir+_T("*.*"));
		while (bWorking)
		{
			bWorking = finder.FindNextFile();
			if (finder.IsDots() || finder.IsSystem() || !finder.IsDirectory())
				continue;
			AddSharedDirectory(strDir + finder.GetFileName(), true);
		}
		finder.Close();
	}
}

void CSharedDirsTreeCtrl::RemoveSharedDirectory(CString strDir, bool bSubDirectories){
	if (strDir.Right(1) == "\\"){
		strDir = strDir.Left(strDir.GetLength()-1);
	}
	strDir.MakeLower();
	POSITION pos1, pos2;
	for (pos1 = m_strliSharedDirs.GetHeadPosition();( pos2 = pos1 ) != NULL;)
	{
		m_strliSharedDirs.GetNext(pos1);
		CString str = m_strliSharedDirs.GetAt(pos2);
		str.MakeLower();
		if (str.CompareNoCase(strDir) == 0)
			m_strliSharedDirs.RemoveAt(pos2);
		else if (bSubDirectories && str.Find(strDir) == 0)
			m_strliSharedDirs.RemoveAt(pos2);
	}

	if (thePrefs.m_bShareIncomingDir)
	{
		CString str2Cmp = strDir;
		if (str2Cmp.Right(1) != _T("\\"))
			str2Cmp.Append(_T("\\"));
		if (0 == str2Cmp.CompareNoCase(thePrefs.m_strShareIncomingDir))
			thePrefs.m_bShareIncomingDir = FALSE;
	}
}

void CSharedDirsTreeCtrl::FileSystemTreeUpdateBoldState(CDirectoryItem* pDir){
	if (pDir == NULL)
		pDir = m_pRootUnsharedDirectries;

	CString strDir = pDir->m_strFullPath;
	SetItemState(pDir->m_htItem, (FileSystemTreeHasSharedSubdirectory(pDir->m_strFullPath) || CGlobalVariable::filemgr.HasFileSharedInDir(pDir->m_strFullPath) || CGlobalVariable::sharedfiles->IsHashing(pDir->m_strFullPath) ? TVIS_BOLD : 0), TVIS_BOLD);

	if (!pDir->m_strFullPath.IsEmpty())
	{
		int nState = GetStateImage(pDir->m_htItem);

		SetItemState(pDir->m_htItem, INDEXTOSTATEIMAGEMASK(nState), TVIS_STATEIMAGEMASK);
	}
	
	POSITION pos = pDir->liSubDirectories.GetHeadPosition();
	while (pos != NULL)
	{
		FileSystemTreeUpdateBoldState(pDir->liSubDirectories.GetNext(pos));
	}
}

void CSharedDirsTreeCtrl::FileSystemTreeSetShareState(CDirectoryItem* pDir, bool bShared, bool bSubDirectories){
	if (m_bUseIcons && pDir->m_htItem != NULL)
	{
		SetItemState(pDir->m_htItem,bShared ? INDEXTOOVERLAYMASK(UNCHECKED) : 0, TVIS_OVERLAYMASK);
		m_pSharedFilesCtrl->SetDirectoryState(bShared);
	}

	if (bSubDirectories)
	{
		POSITION pos = pDir->liSubDirectories.GetHeadPosition();
		while (pos != NULL)
		{
			FileSystemTreeSetShareState(pDir->liSubDirectories.GetNext(pos), bShared, true);
		}
	}
}

void CSharedDirsTreeCtrl::EditSharedDirectories(CDirectoryItem* pDir, bool bAdd, bool bSubDirectories){
	ASSERT(pDir->m_eItemType == SDI_UNSHAREDDIRECTORY);
	CWaitCursor curWait;
	if (bAdd){
		AddSharedDirectory(pDir->m_strFullPath, bSubDirectories);
	}
	else{
		RemoveSharedDirectory(pDir->m_strFullPath, bSubDirectories);
	}
	FileSystemTreeSetShareState(pDir, bAdd, bSubDirectories);
	FileSystemTreeUpdateBoldState();
	FilterTreeReloadTree();

	// sync with the preferences list
	thePrefs.shareddir_list.RemoveAll();
	POSITION pos = m_strliSharedDirs.GetHeadPosition();
	// copy list
	while (pos){
		CString strPath = m_strliSharedDirs.GetNext(pos);
		if (strPath.Right(1) != "\\"){
			strPath += _T("\\");
		}
		thePrefs.shareddir_list.AddTail(strPath);
	}

	//  update the sharedfiles list
	theApp.emuledlg->sharedfileswnd->Reload();
	thePrefs.Save();
}

void CSharedDirsTreeCtrl::Reload(bool bForce){
	bool bChanged = false;
	if (!bForce){
		// check for changes in shared dirs
		if (thePrefs.shareddir_list.GetCount() == m_strliSharedDirs.GetCount()){
			POSITION pos = m_strliSharedDirs.GetHeadPosition();
			POSITION pos2 = thePrefs.shareddir_list.GetHeadPosition();
			while (pos != NULL && pos2 != NULL){
				CString str1 = m_strliSharedDirs.GetNext(pos);
				CString str2 = thePrefs.shareddir_list.GetNext(pos2);
				if (str1.Right(1) == "\\"){
					str1 = str1.Left(str1.GetLength()-1);
				}
				if (str2.Right(1) == "\\"){
					str2 = str2.Left(str2.GetLength()-1);
				}
				if  (str1.CompareNoCase(str2) != 0){
					bChanged = true;
					break;
				}
			}
		}
		else
			bChanged = true;

		// check for changes in categories incoming dirs
		CString strMainIncDir = thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR);
		if (strMainIncDir.Right(1) == _T("\\"))
			strMainIncDir = strMainIncDir.Left(strMainIncDir.GetLength()-1);
		CStringList strliFound;
		for (int i = 0; i < thePrefs.GetCatCount(); i++){
			Category_Struct* pCatStruct = thePrefs.GetCategory(i);
			if (pCatStruct != NULL){
				CString strCatIncomingPath = pCatStruct->strIncomingPath;
				if (strCatIncomingPath.Right(1) == _T("\\"))
					strCatIncomingPath = strCatIncomingPath.Left(strCatIncomingPath.GetLength()-1);

				if (!strCatIncomingPath.IsEmpty() && strCatIncomingPath.CompareNoCase(strMainIncDir) != 0
					&& strliFound.Find(strCatIncomingPath) == NULL)
				{
					POSITION pos = m_strliCatIncomingDirs.Find(strCatIncomingPath);
					if (pos != NULL){
						strliFound.AddTail(strCatIncomingPath);
					}
					else{
						bChanged = true;
						break;
					}
				}
			}
		}
		if (strliFound.GetCount() != m_strliCatIncomingDirs.GetCount())
			bChanged = true;

	}
	if (bChanged || bForce){
		FetchSharedDirsList();
		FilterTreeReloadTree();
		Expand(m_pRootUnsharedDirectries->m_htItem, TVE_COLLAPSE); // collapsing is enough to sync for the filtetree, as all items are recreated on every expanding
	}
}

void CSharedDirsTreeCtrl::FetchSharedDirsList(){
	m_strliSharedDirs.RemoveAll();
	POSITION pos = thePrefs.shareddir_list.GetHeadPosition();
	// copy list
	while (pos){
		CString strPath = thePrefs.shareddir_list.GetNext(pos);
		if (strPath.Right(1) == "\\"){
			strPath = strPath.Left(strPath.GetLength()-1);
		}
		m_strliSharedDirs.AddTail(strPath);
	}
}

void CSharedDirsTreeCtrl::OnLvnBegindrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LPNMTREEVIEW lpnmtv = (LPNMTREEVIEW)pNMHDR;
	*pResult = 0;

	CDirectoryItem* pToDrag = (CDirectoryItem*)lpnmtv->itemNew.lParam;
	if (pToDrag == NULL || pToDrag->m_eItemType != SDI_UNSHAREDDIRECTORY || FileSystemTreeIsShared(pToDrag->m_strFullPath))
		return;

	ASSERT( m_pDraggingItem == NULL );
	delete m_pDraggingItem;
	m_pDraggingItem = pToDrag->CloneContent(); // to be safe we store a copy, as items can be deleted when collapsing the tree etc

	CImageList* piml = NULL;
	POINT ptOffset;
	RECT rcItem;
	if ((piml = CreateDragImage(lpnmtv->itemNew.hItem)) == NULL)
		return;

	/* get the bounding rectangle of the item being dragged (rel to top-left of control) */
	if (GetItemRect(lpnmtv->itemNew.hItem, &rcItem, TRUE))
	{
		CPoint ptDragBegin;
		int nX, nY;
		/* get offset into image that the mouse is at */
		/* item rect doesn't include the image */
		ptDragBegin = lpnmtv->ptDrag;
		ImageList_GetIconSize(piml->GetSafeHandle(), &nX, &nY);
		ptOffset.x = (ptDragBegin.x - rcItem.left) + (nX - (rcItem.right - rcItem.left));
		ptOffset.y = (ptDragBegin.y - rcItem.top) + (nY - (rcItem.bottom - rcItem.top));
		/* convert the item rect to screen co-ords, for use later */
		MapWindowPoints(NULL, &rcItem);
	}
	else
	{
		GetWindowRect(&rcItem);
		ptOffset.x = ptOffset.y = 8;
	}

	if (piml->BeginDrag(0, ptOffset))
	{
		CPoint ptDragEnter = lpnmtv->ptDrag;
		ClientToScreen(&ptDragEnter);
		piml->DragEnter(NULL, ptDragEnter);
	}
	delete piml;

	/* set the focus here, so we get a WM_CANCELMODE if needed */
	SetFocus();

	/* redraw item being dragged, otherwise it remains (looking) selected */
	InvalidateRect(&rcItem, TRUE);
	UpdateWindow();

	/* Hide the mouse cursor, and direct mouse input to this window */
	SetCapture(); 
}

void CSharedDirsTreeCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_pDraggingItem != NULL)
	{
		CPoint pt;

		/* drag the item to the current position */
		pt = point;
		ClientToScreen(&pt);

		CImageList::DragMove(pt);
		CImageList::DragShowNolock(FALSE);
		if (CWnd::WindowFromPoint(pt) != this)
			SetCursor(AfxGetApp()->LoadStandardCursor(IDC_NO));
		else
		{
			TVHITTESTINFO tvhti;
			tvhti.pt = pt;
			ScreenToClient(&tvhti.pt);
			HTREEITEM hItemSel = HitTest(&tvhti);
			CDirectoryItem* pDragTarget;
			if (hItemSel != NULL && (pDragTarget = (CDirectoryItem*)GetItemData(hItemSel)) != NULL){
				//only allow dragging to shared folders
				if (pDragTarget->m_eItemType == SDI_DIRECTORY || pDragTarget->m_eItemType == SDI_NO){
					SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
					SelectDropTarget(pDragTarget->m_htItem);
				}
				else
					SetCursor(AfxGetApp()->LoadStandardCursor(IDC_NO));
			}
			else{
				SetCursor(AfxGetApp()->LoadStandardCursor(IDC_NO));
			}
		}

		CImageList::DragShowNolock(TRUE);
	}

	CTreeCtrl::OnMouseMove(nFlags, point);
}

void CSharedDirsTreeCtrl::OnLButtonUp(UINT nFlags, CPoint point){
	
	if (m_pDraggingItem != NULL){
		CPoint pt;
		pt = point;
		ClientToScreen(&pt);

		TVHITTESTINFO tvhti;
		tvhti.pt = pt;
		ScreenToClient(&tvhti.pt);
		HTREEITEM hItemSel = HitTest(&tvhti);
		CDirectoryItem* pDragTarget;
		if (hItemSel != NULL && (pDragTarget = (CDirectoryItem*)GetItemData(hItemSel)) != NULL){
			//only allow dragging to shared folders
			if (pDragTarget->m_eItemType == SDI_DIRECTORY || pDragTarget->m_eItemType == SDI_NO){
				CDirectoryItem* pRealDragItem;
				HTREEITEM htReal = m_pRootUnsharedDirectries->FindItem(m_pDraggingItem);
				// get the original drag src
				if (htReal != NULL && (pRealDragItem = (CDirectoryItem*)GetItemData(htReal)) != NULL){
					EditSharedDirectories(pRealDragItem, true, false);
				}
				else{
					// item was deleted - no problem as when we dont need to update the visible part
					// we can just as well use the contentcopy
					EditSharedDirectories(m_pDraggingItem, true, false);
				}
			}
		}
		
		CImageList::DragLeave(NULL);
		CImageList::EndDrag();
		ReleaseCapture();
		ShowCursor(TRUE);
		SelectDropTarget(NULL);

		delete m_pDraggingItem;
		m_pDraggingItem = NULL;

		RedrawWindow();
	}
	CTreeCtrl::OnLButtonUp(nFlags, point);
}

void CSharedDirsTreeCtrl::OnCancelMode() 
{
	if (m_pDraggingItem != NULL){
		CImageList::DragLeave(NULL);
		CImageList::EndDrag();
		ReleaseCapture();
		ShowCursor(TRUE);
		SelectDropTarget(NULL);

		delete m_pDraggingItem;
		m_pDraggingItem = NULL;
		RedrawWindow();
	}
	CTreeCtrl::OnCancelMode();
}


void CSharedDirsTreeCtrl::OnDrawItem(int /*nIDCtl*/, LPDRAWITEMSTRUCT lpDrawItemStruct)// VC-linhai[2007-08-07]:warning C4100: “nIDCtl” : 未引用的形参
{
	m_pMenuXP->DrawItem(lpDrawItemStruct);
}

void CSharedDirsTreeCtrl::OnMeasureItem(int /*nIDCtl*/, LPMEASUREITEMSTRUCT lpMeasureItemStruct)// VC-linhai[2007-08-07]:warning C4100: “nIDCtl” : 未引用的形参
{
	m_pMenuXP->MeasureItem(lpMeasureItemStruct);
}

BOOL CSharedDirsTreeCtrl::CreateCheckboxImages()
{
	CDC *pDC = GetDC();
	ASSERT(pDC);
	BOOL rc = HDCheckboxImageList::CreateCheckboxImageList(pDC, m_StateImage, 16, GetSysColor(COLOR_WINDOW));
	ReleaseDC(pDC);
	SetImageList(&m_StateImage, TVSIL_STATE);
	return rc;
}

void CSharedDirsTreeCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	UINT uHitFlags;
	HTREEITEM hItem = HitTest(point, &uHitFlags);

	if (hItem && (uHitFlags & TVHT_ONITEMSTATEICON))
	{
		SelectItem(hItem);
		ShareSubDirectories(hItem, nFlags & MK_CONTROL);
	}

	CTreeCtrl::OnLButtonDown(nFlags, point);
}

int CSharedDirsTreeCtrl::GetStateImage(HTREEITEM hItem)
{
	int nState = 0;

	CDirectoryItem* pCurrent = (CDirectoryItem*)GetItemData(hItem);

	if (FileSystemTreeIsShared(pCurrent->m_strFullPath) )
	{
		nState =  CHECKED;
	}
	else if ( !FileSystemTreeIsShared(pCurrent->m_strFullPath) && ( FileSystemTreeHasSharedSubdirectory(pCurrent->m_strFullPath) || CGlobalVariable::filemgr.HasFileSharedInDir(pCurrent->m_strFullPath) || CGlobalVariable::sharedfiles->IsHashing(pCurrent->m_strFullPath)))
	{
		nState =  TRISTATE;
	}
	else 
	{
		nState =  UNCHECKED;
	}

	return nState;
}

void CSharedDirsTreeCtrl::MarkChilds(HTREEITEM hChild,BOOL mark)
{
	SetCheck(hChild,mark);  
	Expand(hChild, TVE_TOGGLE); 
	HTREEITEM hChild2; 
	hChild2 = GetChildItem(hChild); 
	while( hChild2 != NULL) 
	{ 
		MarkChilds(hChild2,mark); 
		hChild2 = GetNextSiblingItem( hChild2 ); 
	} 
	Expand(hChild, TVE_TOGGLE);
}

void CSharedDirsTreeCtrl::ShareSubDirectories(HTREEITEM hItem, BOOL bShare)
{
	CWaitCursor curWait;
	SetRedraw(FALSE);

	if (bShare)
	{
		Expand(hItem, TVE_TOGGLE);
		HTREEITEM hChild = GetChildItem(hItem);
		while (hChild != NULL)
		{
			int nState = GetStateImage(hItem);
			MarkChilds(hChild, nState);
			hChild = GetNextSiblingItem(hChild);
		}
		Expand(hItem, TVE_TOGGLE);
	}

	CDirectoryItem* pSelectedDir = (CDirectoryItem*)GetItemData(hItem);

	int nState = GetStateImage(hItem);
	EditSharedDirectories(pSelectedDir, nState == CHECKED ? false : true, bShare ? true : false);

	SetRedraw(TRUE);
	Invalidate();
}
void CSharedDirsTreeCtrl::OnNMClick(NMHDR * /*pNMHDR*/, LRESULT *pResult)
{
	CPoint point;
	::GetCursorPos(&point);
	ScreenToClient(&point);

	UINT flags = 0;
	HitTest(point, &flags);

	if(flags & TVHT_ONITEMSTATEICON) 
	{
		*pResult = 1;
	}
	else
	{
		*pResult = 0;
	}
}

bool CSharedDirsTreeCtrl::GetDirectoryState(CString strDir)
{
	return FileSystemTreeIsShared(strDir);
}

HTREEITEM CSharedDirsTreeCtrl::FindItem(CString strDir, HTREEITEM hItem)
{
	if (strDir.GetLength() == 0)
	{
		return GetSelectedItem();
	}

	HTREEITEM hItemSel = hItem ? hItem : GetSelectedItem();
	HTREEITEM hItemCur = GetNextItem(hItemSel);

	if (hItemCur == NULL)
	{
		hItemCur = GetRootItem();
	}

	while (hItemCur && (hItemCur != hItemSel))
	{
		CDirectoryItem* pDirectoryItem = (CDirectoryItem*)GetItemData(hItemCur);

		CString tempDir = pDirectoryItem->m_strFullPath;

		if (tempDir.Right(1) != _T('\\'))
		{
			tempDir += _T('\\');
		}

		if (strDir.Right(1) != _T('\\'))
		{
			strDir += _T("\\");
		}

		if (CompareLocaleStringNoCase(tempDir, strDir) == 0)
		{
			return hItemCur;
		}

		hItemCur = GetNextItem(hItemCur);

		if ((hItemCur == NULL))
		{
			break;
		}

		if ((hItemCur == NULL) && (hItemSel != NULL))
		{
			hItemCur = GetRootItem();
		}
	}

	return NULL;
}

HTREEITEM CSharedDirsTreeCtrl::GetNextItem(HTREEITEM hItem)
{
	HTREEITEM hItemNext = NULL;

	ASSERT(hItem);

	if (hItem)
	{
		if (ItemHasChildren(hItem))
		{
			hItemNext = GetChildItem(hItem);
		}

		if (hItemNext == NULL)
		{
			while ((hItemNext = GetNextSiblingItem(hItem)) == NULL)
			{
				if ((hItem = GetParentItem(hItem)) == NULL)
				{
					return NULL;
				}
			}
		}
	}

	return hItemNext;
}

void CSharedDirsTreeCtrl::UpdateTreeItem(CKnownFile* file)
{
	CString strDir;

	if (file)
	{
		strDir = file->GetPath();
	}

	HTREEITEM hFound = FindItem(strDir, GetRootItem());
	
	if (!hFound)
	{
		return;
	}

	CDirectoryItem* pSelectedDir = (CDirectoryItem*)GetItemData(hFound);

	if (!pSelectedDir)
	{
		return;
	}

	if (pSelectedDir->m_eItemType == SDI_UNSHAREDDIRECTORY)
	{
		FileSystemTreeSetShareState(pSelectedDir, FileSystemTreeIsShared(pSelectedDir->m_strFullPath), false);
		FileSystemTreeUpdateBoldState();
		FilterTreeReloadTree();
	}
}

void CSharedDirsTreeCtrl::ExpandList()
{
	Expand(m_pRootUnsharedDirectries->m_htItem, TVE_TOGGLE); //Added by thilon on 2008.03.27
}