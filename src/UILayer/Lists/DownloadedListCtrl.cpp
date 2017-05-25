/*
 * $Id: DownloadedListCtrl.cpp 7232 2008-09-11 10:39:39Z huby $
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
// DownloadedListCtrl.cpp : 实现文件
//
#include "stdafx.h"
#include "DownloadedListCtrl.h"
#include ".\downloadedlistctrl.h"
#include "otherfunctions.h"
#include "WndMgr.h"
#include "UserMsgs.h"
#include "CmdFuncs.h"
#include "TitleMenu.h"
#include "MenuCmds.h"
#include "JavaScriptEscape.h"
#include "eMule.h"
#include "InputBox.h"
#include "emuleDlg.h"
#include "CollectionViewDialog.h"
#include "GlobalVariable.h"
#include "TransferWnd.h"
#include "DownloadListCtrl.h"
#include "MemDC.h"

#include "SharedFilesCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CDownloadedListCtrl

IMPLEMENT_DYNAMIC(CDownloadedListCtrl, CMuleListCtrl)
CDownloadedListCtrl::CDownloadedListCtrl()
{
	memset(&sortstat, 0, sizeof(sortstat));
	m_iCurSortCol		= CI_FILE_NAME;
	m_bSortAscending	= true;

	m_pMenuXP = NULL;
}

CDownloadedListCtrl::~CDownloadedListCtrl()
{
	if(m_pMenuXP)
	{
		delete m_pMenuXP;
	}
}


BEGIN_MESSAGE_MAP(CDownloadedListCtrl, CMuleListCtrl)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_MESSAGE(UM_DLED_LC_ADDFILE, OnAddFile)
	ON_MESSAGE(UM_DLED_LC_REMOVEFILE, OnRemoveFile)
    
	ON_MESSAGE(UM_CPL_LC_ADDFILE,OnCompletedAdd)
	ON_MESSAGE(UM_CPL_LC_DELETEFILE,OnCompletedDelete)
    ON_MESSAGE(UM_FILE_UPDATE,OnUpdateFile)

	ON_NOTIFY_REFLECT(NM_RCLICK, OnNMRclick)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnLvnColumnclick)
	ON_NOTIFY_REFLECT(LVN_INSERTITEM, OnLvnItemchanged)
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, OnLvnItemchanged)
	ON_NOTIFY_REFLECT(LVN_DELETEITEM, OnLvnItemchanged)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclk)
	ON_WM_DRAWITEM()
	ON_WM_MEASUREITEM()
	ON_WM_KEYDOWN()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()


int CALLBACK CDownloadedListCtrl::CmpProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	if (0 == lParam1 || 0 == lParam2 || 0 == lParamSort)
		return 0;

	CKnownFile *pFile1	= (CKnownFile*) lParam1;
	CKnownFile *pFile2	= (CKnownFile*) lParam2;
	CDownloadedListCtrl	*pCtrl = (CDownloadedListCtrl*) lParamSort;

	int	iRet;
	switch(pCtrl->m_iCurSortCol)
	{
	case CI_FILE_NAME:
		iRet = pFile1->GetFileName() - pFile2->GetFileName();
		break;
	case CI_FILE_SIZE:
		if (pFile1->GetFileSize() > pFile2->GetFileSize())
			iRet = 1;
		else
			iRet = -1;
		break;
	case CI_FILE_PATH:
		iRet = pFile1->GetPath() - pFile2->GetPath();
		break;
	case CI_FILE_FINISHEDTIME:
		iRet = 0;
		break;
	case CI_FILE_COMMENT:
		iRet = 0;
		break;
	default:
		iRet = 0;
		break;
	}

	if (!pCtrl->m_bSortAscending)
		iRet = -iRet;

	return iRet;
}

// CDownloadedListCtrl 消息处理程序


int CDownloadedListCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CListCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	ModifyStyle(LVS_SINGLESEL,0);
	ListView_SetExtendedListViewStyle(m_hWnd, LVS_EX_FULLROWSELECT);

	theWndMgr.SetWndHandle(CWndMgr::WI_DOWNLOADED_LISTCTRL, m_hWnd);

	SetName(_T("DownloadedListCtrl"));

	CImageList ilDummyImageList; //dummy list for getting the proper height of listview entries
	ilDummyImageList.Create(1, theApp.GetSmallSytemIconSize().cy, theApp.m_iDfltImageListColorFlags|ILC_MASK, 1, 1); 
	SetImageList(&ilDummyImageList, LVSIL_SMALL);
	ASSERT( (GetStyle() & LVS_SHAREIMAGELISTS) == 0 );
	ilDummyImageList.Detach();


	// TODO:  在此添加您专用的创建代码
	InsertColumn(CI_FILE_NAME, GetResString(IDS_DLED_LC_FILENAME), LVCFMT_LEFT, 300);
	InsertColumn(CI_FILE_SIZE, GetResString(IDS_DLED_LC_FILESIZE), LVCFMT_LEFT, 100);
	InsertColumn(CI_FILE_PATH, GetResString(IDS_DLED_LC_FILEPATH), LVCFMT_LEFT, 200);
//	InsertColumn(CI_FILE_COMMENT, GetResString(IDS_DLED_LC_FILECOMMENT), LVCFMT_LEFT, 150);
    InsertColumn(CI_FILE_FINISHEDTIME, GetResString(IDS_DLED_LC_FinishedTime), LVCFMT_LEFT, 150);
    LoadSettings();
	SetSortArrow();
	
	SortItems(SortProc, GetSortItem() + (GetSortAscending() ? 0:20));

	return 0;
}

void CDownloadedListCtrl::OnDestroy()
{
	int n = GetItemCount();
	for (int i = 0; i < n; i++)
	{
		ItemData* pData = (ItemData*) CListCtrl::GetItemData(i);
		if (pData)
		{
			pData->pItem = NULL;
			delete pData;
		}
	}

	DeleteAllItems();
	theWndMgr.SetWndHandle(CWndMgr::WI_DOWNLOADED_LISTCTRL, NULL);

	CListCtrl::OnDestroy();
}

LRESULT CDownloadedListCtrl::OnAddFile(WPARAM /*wParam*/, LPARAM lParam)
{
	if (0 == lParam)
		return 0;

	CKnownFile *pFile = (CKnownFile*) lParam;
	CString strFilePath = pFile->GetFilePath();
	CString strPath;

	ItemData *pData = new ItemData;
	pData->type = FILE_TYPE;
	pData->pItem = pFile;

	int	iItemIndex;
	iItemIndex = InsertItem(GetItemCount(), NULL);
	
	SetItemData( iItemIndex, (DWORD_PTR)pData);
	SetItemText( iItemIndex, 0, pFile->GetFileName() );
	SetItemText( iItemIndex, 1, CmdFuncs::GetFileSizeDisplayStr(pFile->GetFileSize()));
	if(pFile->GetPath().Right(1) != _T("\\"))
		 strPath = pFile->GetPath();
	else
	{
		 strPath = pFile->GetPath();
		 int index = strPath.ReverseFind(_T('\\'));
		 strPath.Delete(index,1);
	}
		 
	SetItemText( iItemIndex, 2, strPath);
	CTime tempTime;
	CString str;
	if(pFile->GetFileSize() == (uint64)0)
	{
		tempTime = CTime::GetCurrentTime();
		str = tempTime.Format(_T("%c"));
	}
	else
	{
		CFileFind finder;
		BOOL   bWorking = finder.FindFile(strFilePath);   
		if(bWorking)   
		{   
			finder.FindNextFile();   
			if   (finder.GetLastWriteTime(tempTime))
			{   
				str   =   tempTime.Format(_T("%c"));
			}   
		}
		else
		{
			tempTime = CTime::GetCurrentTime();
			str = tempTime.Format(_T("%c"));
		}  
	}
	SetItemText( iItemIndex, 3,str);
	return 0;
}

LRESULT CDownloadedListCtrl::OnRemoveFile(WPARAM /*wParam*/, LPARAM lParam)
{
	__try
	{
		if (0 == lParam)
			return 0;

		int iItemIndex = FindFile((CKnownFile*) lParam);
		if (-1 != iItemIndex)
			DeleteItem(iItemIndex);

		return 0;
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		return 0;
	}
}

LRESULT CDownloadedListCtrl::OnCompletedAdd(WPARAM /*wParam*/, LPARAM lParam)
{
	if (0 == lParam)
		return 0;

	CFileTaskItem *pFile = (CFileTaskItem *)lParam;

	ItemData *pData = new ItemData;
	pData->type = FILE_TASK;
	pData->pItem = pFile;

	int	iItemIndex;
	iItemIndex = InsertItem(GetItemCount(), NULL);
    int len =pFile->m_strFilePath.GetLength();
	CString FilePath = pFile->m_strFilePath;
	int index = pFile->m_strFilePath.ReverseFind('\\');
	FilePath.Delete(index,len-index);

	SetItemData( iItemIndex, (DWORD_PTR)pData);
	SetItemText( iItemIndex, 0, pFile->m_FileName);
	SetItemText( iItemIndex, 1, CmdFuncs::GetFileSizeDisplayStr(pFile->m_FileSize));
	SetItemText( iItemIndex, 2, FilePath);
	CString time = pFile->m_tFiletime.Format(_T("%c"));
	SetItemText( iItemIndex, 3, time);
	return 0;
}
LRESULT CDownloadedListCtrl::OnCompletedDelete(WPARAM/*WParam*/,LPARAM lParam)
{
	if (0 == lParam)
		return 0;

	int iItemIndex = FindCompleteFile((CFileTaskItem *)lParam);
	if (-1 != iItemIndex)
		DeleteItem(iItemIndex);

	return 0;
}
LRESULT CDownloadedListCtrl::OnUpdateFile(WPARAM/*WParam*/,LPARAM lParam)
{
   if(0 == lParam) 
	   return 0;
   UpdateFile((CKnownFile *)lParam);
   return 0;
}
void CDownloadedListCtrl::OnNMRclick(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE	lpnmitem;
	lpnmitem = (LPNMITEMACTIVATE) pNMHDR;

	if (-1 == lpnmitem->iItem)
		return;

	CTitleMenu	menu;
	menu.CreatePopupMenu();

	//menu.AddMenuTitle(GetResString(IDS_MENUTITLE_COMPLETED), true);
	menu.AppendMenu(MF_STRING,MP_OPEN, GetResString(IDS_OPENFILE), _T("OPENFILE"));
	menu.AppendMenu(MF_STRING,MP_OPENFOLDER, GetResString(IDS_OPENFOLDER), _T("OPENFOLDER"));
	menu.AppendMenu(MF_STRING,MP_RENAME, GetResString(IDS_RENAME) + _T("..."), _T("FILERENAME"));
	menu.AppendMenu(MF_STRING,MP_REMOVE, GetResString(IDS_DELETE_FILE), _T("DELETE"));
	menu.AppendMenu(MF_STRING|MF_SEPARATOR);
	menu.AppendMenu(MF_STRING,MP_GETED2KLINK, GetResString(IDS_DL_LINK1), _T("ED2KLINK") );
//	menu.AppendMenu(MF_STRING|MF_SEPARATOR);
	//menu.AppendMenu(MF_STRING,MP_VIRUS, GetResString(IDS_VIRUS), _T("ANTIVIRUS"));

	//设置菜单状态
	int iSelectedItems = GetSelectedCount();

	if(iSelectedItems && iSelectedItems == 1)
	{
		menu.EnableMenuItem(MP_OPEN, MF_ENABLED);
		menu.EnableMenuItem(MP_OPENFOLDER, MF_ENABLED);
		menu.EnableMenuItem(MP_GETED2KLINK, MF_ENABLED);
		menu.EnableMenuItem(MP_VIRUS, MF_ENABLED);
	}
	else
	{	
		menu.EnableMenuItem(MP_OPEN, MF_GRAYED);
		menu.EnableMenuItem(MP_OPENFOLDER, MF_GRAYED);
		menu.EnableMenuItem(MP_GETED2KLINK, MF_GRAYED);
		menu.EnableMenuItem(MP_VIRUS, MF_GRAYED);
	}


	CPoint	pt;
	pt = lpnmitem->ptAction;
	ClientToScreen(&pt);

	m_pMenuXP = new CMenuXP();
	m_pMenuXP->AddMenu(&menu, TRUE);
	menu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, pt.x, pt.y,this);
	menu.DestroyMenu();

	delete m_pMenuXP;
	m_pMenuXP = NULL;

	*pResult = 0;
}

BOOL CDownloadedListCtrl::OnCommand(WPARAM wParam, LPARAM /*lParam*/)
{
	// TODO: 在此添加专用代码和/或调用基类
	wParam = LOWORD(wParam);

//	CTypedPtrList<CPtrList, CKnownFile*> selectedList;
	CTypedPtrList<CPtrList, ItemData*> selectedList;
	POSITION pos = GetFirstSelectedItemPosition();
	while (pos != NULL)
	{
		int index = GetNextSelectedItem(pos);
		if (index >= 0)
		{
		    selectedList.AddTail((ItemData *)GetItemData(index));
		}
	}

	if (   wParam == MP_CREATECOLLECTION || wParam == MP_FIND || wParam == MP_NEW/*|| wParam == MP_VIRUS*/ || selectedList.GetCount() > 0)
	{
		CKnownFile* file = NULL;
		ItemData *pItemData = NULL;
		CFileTaskItem *pfile = NULL;
		if (selectedList.GetCount() == 1)
		{ 
			pItemData = selectedList.GetHead();
			if(pItemData->type == FILE_TYPE)
			       file = (CKnownFile *)pItemData->pItem;
			if(pItemData->type ==FILE_TASK)
				   pfile = (CFileTaskItem *)pItemData->pItem;
		}

		switch (wParam){
			//case Irc_SetSendLink:
			//	if (file)
			//		theApp.emuledlg->ircwnd->SetSendFileString(CreateED2kLink(file));
			//	break;
			case MP_NEW:
				CmdFuncs::PopupNewTaskDlg();
				break;
			case MP_GETED2KLINK:{
				CString str;
				POSITION pos = selectedList.GetHeadPosition();
				while (pos != NULL)
				{
					pItemData =selectedList.GetNext(pos);
					if(pItemData->type == FILE_TYPE)
					{
						file = (CKnownFile *)pItemData->pItem;
					    if (!str.IsEmpty())
						      str += _T("\r\n");
					     str += CreateED2kLink(file);
					}
					if(pItemData->type == FILE_TASK)
					{
						CFileTaskItem* pTask = (CFileTaskItem*)pItemData->pItem;
						str += pTask->m_strEd2kLink;
					}
				}
				theApp.CopyTextToClipboard(str);
				break;
								}
			case MP_GETKADSOURCELINK:{
				CString str;
				POSITION pos = selectedList.GetHeadPosition();
				while (pos != NULL)
				{
				//	file = selectedList.GetNext(pos);
					pItemData = selectedList.GetNext(pos);
					if(pItemData->type ==FILE_TYPE)
					{
						file =(CKnownFile *)pItemData->pItem;
					}
					if (!str.IsEmpty())
						str += _T("\r\n");
					str += theApp.CreateKadSourceLink(file);
				}
				theApp.CopyTextToClipboard(str);
				break;
									 }
									 // file operations
			case MP_OPEN:
			case IDA_ENTER:
				{
					if(pItemData->type == FILE_TYPE)
					{
                        file = (CKnownFile *)pItemData->pItem;
						if(!PathFileExists(file->GetFilePath()))
						{
							MessageBox(GetResString(IDS_DELETEFILEINFO),GetResString(IDS_CAPTION),MB_OK|MB_ICONWARNING);
						}
						else
							OpenFile(file);
					}
					if(pItemData->type == FILE_TASK)
					{
                         pfile = (CFileTaskItem *)pItemData->pItem;
						 CString wholepath = pfile->m_strFilePath;
						 if(pfile->m_FileSize == (uint64)0)
						 {
						   MessageBox(GetResString(IDS_ZEROFILEINFO),GetResString(IDS_CAPTION),MB_OK|MB_ICONWARNING);
						 }
						 else if(!PathFileExists(wholepath))
						 {
                           MessageBox(GetResString(IDS_DELETEFILEINFO),GetResString(IDS_CAPTION),MB_OK|MB_ICONWARNING);
						 }
					}
				}
			//	if (file && !file->IsPartFile())
			//		OpenFile(file);
				break; 
			//case MP_INSTALL_SKIN:
			//	if (file && !file->IsPartFile())
			//		InstallSkin(file->GetFilePath());
			//	break;
			case MP_OPENFOLDER:
				{
					if(pItemData->type == FILE_TYPE)
					{
						file = (CKnownFile *)pItemData->pItem;
                        CmdFuncs::OpenFolder(file);
					}
					if(pItemData->type ==FILE_TASK)
					{
						pfile = (CFileTaskItem *)pItemData->pItem;
						CString strFilePath = pfile->m_strFilePath;
						int len = strFilePath.GetLength();
						int index = strFilePath.ReverseFind('\\');
                        strFilePath.Delete(index+1,len-index);
						if(pfile->m_FileSize == (uint64)0)
						{
                           ShellExecute(NULL, _T("open"),_T("explorer.exe"), strFilePath,NULL, SW_SHOW);
						}
						else if(!PathFileExists(pfile->m_strFilePath))
						{
							if(IDYES == MessageBox(GetResString(IDS_OPENFOLDERINFO),GetResString(IDS_CAPTION),MB_ICONQUESTION | MB_DEFBUTTON2 | MB_YESNO))
								ShellExecute(NULL, _T("open"), _T("explorer.exe"),strFilePath, NULL, SW_SHOW);
						}
						else
						{
							CString	strParam;
							strParam.Format(_T(" %s, /select, %s"), strFilePath, pfile->m_strFilePath);
							ShellExecute(NULL, _T("open"), _T("explorer.exe"), strParam, NULL, SW_SHOW);
						}
					}
				}
				break; 
			case MP_RENAME:
			case MPG_F2:
				if (file && !file->IsPartFile()){
					InputBox inputbox;
					CString title = GetResString(IDS_RENAME);
					title.Remove(_T('&'));
					inputbox.SetLabels(title, GetResString(IDS_DL_FILENAME), file->GetFileName());
					inputbox.SetEditFilenameMode();
					inputbox.DoModal();
					CString newname = inputbox.GetInput();
					if (!inputbox.WasCancelled() && newname.GetLength()>0)
					{
						// at least prevent users from specifying something like "..\dir\file"
						static const TCHAR _szInvFileNameChars[] = _T("\\/:*?\"<>|");
						if (newname.FindOneOf(_szInvFileNameChars) != -1){
							AfxMessageBox(GetErrorMessage(ERROR_BAD_PATHNAME));
							break;
						}

						CString newpath;
						PathCombine(newpath.GetBuffer(MAX_PATH), file->GetPath(), newname);
						newpath.ReleaseBuffer();
						if (_trename(file->GetFilePath(), newpath) != 0){
							CString strError;
							strError.Format(GetResString(IDS_ERR_RENAMESF), file->GetFilePath(), newpath, _tcserror(errno));
							AfxMessageBox(strError);
							break;
						}

						if (file->IsKindOf(RUNTIME_CLASS(CPartFile)))
						{
							file->SetFileName(newname);
							STATIC_DOWNCAST(CPartFile, file)->SetFullName(newpath); 
						}
						else
						{
							CGlobalVariable::sharedfiles->RemoveKeywords(file);
							file->SetFileName(newname);
							CGlobalVariable::sharedfiles->AddKeywords(file);
						}
						file->SetFilePath(newpath);
						UpdateFile(file);
					}
				}
				else
				{
					MessageBeep(MB_OK);
					if(selectedList.GetCount() == 1)
					   MessageBox(GetResString(IDS_DELETEFILEINFO),GetResString(IDS_CAPTION),MB_OK);
				}
				break;
			case MP_REMOVE:
			case MP_CANCEL: 
			case MPG_DELETE:
				{  
					int selectedCount = selectedList.GetCount();
					CAffirmDeleteDlg affirmDelete;
					if(selectedCount >= 1)
					{
						SetRedraw(false);
						if(affirmDelete.DoModal() == IDCANCEL)
						{
							SetRedraw(true);
							return true;
						}
					}

		            BOOL delsucc = FALSE;
					//bool bRemovedItems = false;
					while (!selectedList.IsEmpty())
					{				
				//		CKnownFile* myfile = selectedList.RemoveHead();
						ItemData *pData =selectedList.RemoveHead();
						
						if(pData->type ==FILE_TYPE)
						{   
							CKnownFile* pmyfile =(CKnownFile *)pData->pItem;
												 
							
								
							if (!PathFileExists(pmyfile->GetFilePath()) || pmyfile->GetFileSize() == (uint64)0)
							{
								delsucc = TRUE;
								SendMessage(UM_DLED_LC_REMOVEFILE, 0, (LPARAM)pmyfile); //UI上的已下载列表上移除
								if (pmyfile->IsKindOf(RUNTIME_CLASS(CPartFile)))
									theApp.emuledlg->transferwnd->downloadlistctrl.ClearCompleted(static_cast<CPartFile*>(pmyfile));
								CString delUrl = pmyfile->GetPartFileURL();
								CGlobalVariable::filemgr.RemoveURLTask(delUrl);
								CGlobalVariable::filemgr.RemoveFileItem(pmyfile);
							}
							else
							{								
								if(affirmDelete.bIsDeleteFile)
								{
									// delete to recycle bin :(
									if (!thePrefs.GetRemoveToBin())
									{
										delsucc = DeleteFile(pmyfile->GetFilePath());
									}
									else// Delete
									{
										TCHAR todel[MAX_PATH+1];
										memset(todel, 0, sizeof todel);
										_tcsncpy(todel, pmyfile->GetFilePath(), ARRSIZE(todel)-2);
										SHFILEOPSTRUCT fp = {0};
										fp.wFunc = FO_DELETE;
										fp.hwnd = theApp.emuledlg->m_hWnd;
										fp.pFrom = todel;
										fp.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_SILENT;// | FOF_NOERRORUI
										delsucc = (SHFileOperation(&fp) == 0);
									}

									if (delsucc)
									{
										CGlobalVariable::sharedfiles->RemoveFile(pmyfile);
										SendMessage(UM_DLED_LC_REMOVEFILE, 0, (LPARAM)pmyfile);

										//bRemovedItems = true;
										if (pmyfile->IsKindOf(RUNTIME_CLASS(CPartFile)))
											theApp.emuledlg->transferwnd->downloadlistctrl.ClearCompleted(static_cast<CPartFile*>(pmyfile));
										CString delUrl = pmyfile->GetPartFileURL();
										CGlobalVariable::filemgr.RemoveURLTask(delUrl);
										CGlobalVariable::filemgr.RemoveFileItem(pmyfile);
									}
									else
									{
										CString strError;
										strError.Format( GetResString(IDS_ERR_DELFILE) + _T("\r\n\r\n%s"), pmyfile->GetFilePath(), GetErrorMessage(GetLastError()));
										MessageBox(strError,GetResString(IDS_CAPTION),MB_OK);
									}
								}
								else
								{  
									SendMessage(UM_CPL_LC_DELETEFILE, 0, (LPARAM)pmyfile); 
									if (pmyfile->IsKindOf(RUNTIME_CLASS(CPartFile)))
										theApp.emuledlg->transferwnd->downloadlistctrl.ClearCompleted(static_cast<CPartFile*>(pmyfile));
									CString delUrl = pmyfile->GetPartFileURL();
									if(!delUrl.IsEmpty())
										CGlobalVariable::filemgr.RemoveURLTask(delUrl);
									CGlobalVariable::filemgr.RemoveFileItem(pmyfile,true);
								}
							}
						}
						else if(pData->type ==FILE_TASK)
						{
							CFileTaskItem* pmyfile = (CFileTaskItem *)pData->pItem;
							
							SendMessage(UM_CPL_LC_DELETEFILE, 0, (LPARAM)pmyfile); 
							CGlobalVariable::filemgr.RemoveURLTask(pmyfile->m_strUrl);
							CGlobalVariable::filemgr.RemoveEdLinkFileTask(pmyfile->m_strEd2kLink);
						}
#ifdef _DEBUG
						else
						{
							ASSERT(FALSE);
						}
#endif
						delete pData;
					}
					SetRedraw(true);
                  }
					break; 
				
			//case MP_CMT:
			//	ShowFileDialog(selectedList, IDD_COMMENT);
			//	break; 
			//case MPG_ALTENTER:
			//case MP_DETAIL:
			//	ShowFileDialog(selectedList);
			//	break;
			//case MP_FIND:
			//	OnFindStart();
			//	break;
			//case MP_CREATECOLLECTION:
			//	{
			//		CCollection* pCollection = new CCollection();
			//		POSITION pos = selectedList.GetHeadPosition();
			//		while (pos != NULL)
			//		{
			//			pCollection->AddFileToCollection(selectedList.GetNext(pos),true);
			//		}
			//		CCollectionCreateDialog dialog;
			//		dialog.SetCollection(pCollection,true);
			//		dialog.DoModal();
			//		//We delete this collection object because when the newly created
			//		//collection file is added to the sharedfile list, it is read and verified
			//		//and which creates the colleciton object that is attached to that file..
			//		delete pCollection;
			//		break;
			//	}
			//case MP_SEARCHAUTHOR:
			//	if (selectedList.GetCount() == 1 && file->m_pCollection)
			//	{
			//		SSearchParams* pParams = new SSearchParams;
			//		pParams->strExpression = file->m_pCollection->GetCollectionAuthorKeyString();
			//		pParams->eType = SearchTypeKademlia;
			//		pParams->strFileType = ED2KFTSTR_EMULECOLLECTION;
			//		pParams->strSpecialTitle = file->m_pCollection->m_sCollectionAuthorName;
			//		if (pParams->strSpecialTitle.GetLength() > 50){
			//			pParams->strSpecialTitle = pParams->strSpecialTitle.Left(50) + _T("...");
			//		}
			//		theApp.emuledlg->searchwnd->m_pwndResults->StartSearch(pParams);
			//	}
			//	break;
			//case MP_VIEWCOLLECTION:
			//	if (selectedList.GetCount() == 1 && file->m_pCollection)
			//	{
			//		CCollectionViewDialog dialog;
			//		dialog.SetCollection(file->m_pCollection);
			//		dialog.DoModal();
			//	}
			//	break;
			//case MP_MODIFYCOLLECTION:
			//	if (selectedList.GetCount() == 1 && file->m_pCollection)
			//	{
			//		CCollectionCreateDialog dialog;
			//		CCollection* pCollection = new CCollection(file->m_pCollection);
			//		dialog.SetCollection(pCollection,false);
			//		dialog.DoModal();
			//		delete pCollection;				
			//	}
			//	break;
			//case MP_SHOWED2KLINK:
			//	ShowFileDialog(selectedList, IDD_ED2KLINK);
			//	break;
			//case MP_PRIOVERYLOW:
			//case MP_PRIOLOW:
			//case MP_PRIONORMAL:
			//case MP_PRIOHIGH:
			//case MP_PRIOVERYHIGH:
			//case MP_PRIOAUTO:
			//	{
			//		POSITION pos = selectedList.GetHeadPosition();
			//		while (pos != NULL)
			//		{
			//			CKnownFile* file = selectedList.GetNext(pos);
			//			switch (wParam) {
			//case MP_PRIOVERYLOW:
			//	file->SetAutoUpPriority(false);
			//	file->SetUpPriority(PR_VERYLOW);
			//	UpdateFile(file);
			//	break;
			//case MP_PRIOLOW:
			//	file->SetAutoUpPriority(false);
			//	file->SetUpPriority(PR_LOW);
			//	UpdateFile(file);
			//	break;
			//case MP_PRIONORMAL:
			//	file->SetAutoUpPriority(false);
			//	file->SetUpPriority(PR_NORMAL);
			//	UpdateFile(file);
			//	break;
			//case MP_PRIOHIGH:
			//	file->SetAutoUpPriority(false);
			//	file->SetUpPriority(PR_HIGH);
			//	UpdateFile(file);
			//	break;
			//case MP_PRIOVERYHIGH:
			//	file->SetAutoUpPriority(false);
			//	file->SetUpPriority(PR_VERYHIGH);
			//	UpdateFile(file);
			//	break;	
			//case MP_PRIOAUTO:
			//	file->SetAutoUpPriority(true);
			//	file->UpdateAutoUpPriority();
			//	UpdateFile(file); 
			//	break;
			//			}
			//		}
			//		break;
			//	}

				//Added by thilon 2006.08.28
			case MP_VIRUS:
				{
					CString str;
					CString url;

					POSITION pos = selectedList.GetHeadPosition();

					if(pos != NULL)
					{
			//			file = selectedList.GetNext(pos);
						JavaScriptEscape	jse;
						pItemData = selectedList.GetNext(pos);
						if(pItemData->type == FILE_TYPE)
						{
							 file = (CKnownFile *)pItemData->pItem;
							 str = jse.Escape(file->GetFilePath());
						}
						if(pItemData->type == FILE_TASK)
						{
						    pfile = (CFileTaskItem *)pItemData->pItem;
							CString WholePath = pfile->m_strFilePath + pfile->m_FileName;
							str = jse.Escape(WholePath);
						}
						url = _T("http://zs.kingsoft.com/EmuleOnlineScan/index.html?ScanPath=") + str + _T("&ScanType=0&IsAutoScan=1");
						ShellOpenFile(url);

					}
					break;
				}
			default:
				//if (wParam>=MP_WEBURL && wParam<=MP_WEBURL+256){
				//	theWebServices.RunURL(file, wParam);
				//}
				break;
		}
	}
	return TRUE;


	//return CListCtrl::OnCommand(wParam, lParam);
}

void CDownloadedListCtrl::OpenFile(const CKnownFile* file)
{
	if(file->m_pCollection)
	{
		CCollectionViewDialog dialog;
		dialog.SetCollection(file->m_pCollection);
		dialog.DoModal();
	}
	else
		ShellOpenFile(file->GetFilePath(), NULL);
}

void CDownloadedListCtrl::UpdateFile(const CKnownFile* file)
{
	if (!file || !theApp.emuledlg->IsRunning())
		return;
	int iItem = FindFile(file);
	if (iItem != -1)
	{   
		SetItemText(iItem,0,file->GetFileName());
		CString strFilePath = file->GetFilePath();
		int index = strFilePath.ReverseFind('\\');
		strFilePath.Delete(index+1,file->GetFilePath().GetLength()-index);
		SetItemText(iItem,2,strFilePath);
        
		Update(iItem);

		CGlobalVariable::filemgr.UpdateFileItem(file);
	}
}

int CDownloadedListCtrl::FindFile(const CKnownFile* pFile)
{	
	__try
	{
		LVFINDINFO find;
		for(int i = 0;i<GetItemCount();i++)
		{   
			ItemData *pData = NULL;
			pData = (ItemData *)GetItemData(i);
			if(pData->pItem == pFile)
			{
				find.flags = LVFI_PARAM;
				find.lParam = (LPARAM)pData;
				break;
			}
		}
		return FindItem(&find);
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		return -1;
	}
}
int CDownloadedListCtrl::FindCompleteFile(const CFileTaskItem *pCompleteFile)
{
	LVFINDINFO find;
	for(int i = 0;i<GetItemCount();i++)
	{   
		ItemData *pData = NULL;
		pData = (ItemData *)GetItemData(i);
		if(pData->pItem == pCompleteFile)
		{
			find.flags = LVFI_PARAM;
			find.lParam = (LPARAM)pData;
			break;
		}
	}
	return FindItem(&find);
}
void CDownloadedListCtrl::OnLvnColumnclick(NMHDR *pNMHDR, LRESULT *pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	
	bool sortAscending = (GetSortItem() != pNMListView->iSubItem) ? true : !GetSortAscending();

	int adder=0;
	if (pNMListView->iSubItem>=5 && pNMListView->iSubItem<=7)
	{
		ASSERT( pNMListView->iSubItem - 5 < ARRSIZE(sortstat) );
		if (!sortAscending)
		{
			sortstat[pNMListView->iSubItem - 5] = !sortstat[pNMListView->iSubItem - 5];
		}
		adder = sortstat[pNMListView->iSubItem-5] ? 0 : 100;
	}
	else if (pNMListView->iSubItem==11)
	{
		ASSERT( 3 < ARRSIZE(sortstat) );
		if (!sortAscending)
		{
			sortstat[3] = !sortstat[3];
		}
		adder = sortstat[3] ? 0 : 100;
	}

	// Sort table
	if (adder==0)	
		SetSortArrow(pNMListView->iSubItem, sortAscending); 
	else
		SetSortArrow(pNMListView->iSubItem, sortAscending ? arrowDoubleUp : arrowDoubleDown);

	UpdateSortHistory(pNMListView->iSubItem + adder + (sortAscending ? 0:20),20);
	SortItems(SortProc, pNMListView->iSubItem + adder + (sortAscending ? 0:20));

	*pResult = 0;
}

void CDownloadedListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if (!theApp.emuledlg->IsRunning())
	{
		return;
	}
	if (!lpDrawItemStruct->itemData)
	{
		return;
	}

	CDC* odc = CDC::FromHandle(lpDrawItemStruct->hDC);
	BOOL bCtrlFocused = ((GetFocus() == this) || (GetStyle() & LVS_SHOWSELALWAYS));

	if (lpDrawItemStruct->itemState & ODS_SELECTED) 
	{
		if (bCtrlFocused)
		{
			odc->SetBkColor(m_crHighlight);
		}
		else
		{
			odc->SetBkColor(m_crNoHighlight);
		}
	}
	else
	{
		odc->SetBkColor(GetBkColor());
	}

	CMemDC dc(odc, &lpDrawItemStruct->rcItem);

//	LOGFONT lf = {0};
	CFont* pOldFont = dc.SelectObject(GetFont());
	CRect cur_rec(lpDrawItemStruct->rcItem);

	int iOldBkMode;
	if (m_crWindowTextBk == CLR_NONE)
	{
		DefWindowProc(WM_ERASEBKGND, (WPARAM)(HDC)dc, 0);
		iOldBkMode = dc.SetBkMode(TRANSPARENT);
	}
	else
	{
		iOldBkMode = OPAQUE;
	}

	CHeaderCtrl *pHeaderCtrl = GetHeaderCtrl();
	int iCount = pHeaderCtrl->GetItemCount();
	const int iMarginX = 4;

	cur_rec.right = cur_rec.left - iMarginX*2;
	cur_rec.left += iMarginX;
	CString buffer;
	CString	strItemCol;

	int iIconDrawWidth = theApp.GetSmallSytemIconSize().cx + 3;

	for(int iCurrent = 0; iCurrent < iCount; iCurrent++)
	{
		int iColumn = pHeaderCtrl->OrderToIndex(iCurrent);

		if( !IsColumnHidden(iColumn))
		{
			UINT uDTFlags = DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS;
			cur_rec.right += GetColumnWidth(iColumn);
			strItemCol = GetItemText(lpDrawItemStruct->itemID, iColumn);

			switch(iColumn)
			{
			case 0:
				{
					int iImage = theApp.GetFileTypeSystemImageIdx(strItemCol);
					if (theApp.GetSystemImageList() != NULL)
					{
						::ImageList_Draw(theApp.GetSystemImageList(), iImage, dc.GetSafeHdc(), cur_rec.left, cur_rec.top, ILD_NORMAL|ILD_TRANSPARENT);
						cur_rec.left += 19;
					}

					buffer = strItemCol;
					break;
				}
			case 1:
				buffer = strItemCol;
				break;
			case 2:
				buffer = strItemCol;
				break;
			case 3:
				buffer = strItemCol;
				break;
			default:
				break;
			}

			dc->DrawText(buffer, buffer.GetLength(), &cur_rec, uDTFlags);

			if (iColumn == 0)
			{
				cur_rec.left -= iIconDrawWidth;
			}
			cur_rec.left += GetColumnWidth(iColumn);
		}
	}

	if (lpDrawItemStruct->itemState & ODS_SELECTED)
	{
		RECT outline_rec = lpDrawItemStruct->rcItem;

		outline_rec.top--;
		outline_rec.bottom++;
		dc.FrameRect(&outline_rec, &CBrush(m_crWindow));
		outline_rec.top++;
		outline_rec.bottom--;
		outline_rec.left++;
		outline_rec.right--;

		if (lpDrawItemStruct->itemID > 0 && GetItemState(lpDrawItemStruct->itemID - 1, LVIS_SELECTED))
		{
			outline_rec.top--;
		}

		if (lpDrawItemStruct->itemID + 1 < (UINT)GetItemCount() && GetItemState(lpDrawItemStruct->itemID + 1, LVIS_SELECTED))
		{
			outline_rec.bottom++;
		}

		if(bCtrlFocused)
		{
			dc.FrameRect(&outline_rec, &CBrush(m_crFocusLine));
		}
		else
		{
			dc.FrameRect(&outline_rec, &CBrush(m_crNoFocusLine));
		}
	}

	if (m_crWindowTextBk == CLR_NONE)
	{
		dc.SetBkMode(iOldBkMode);
	}

	dc.SelectObject(pOldFont);
}

void CDownloadedListCtrl::OnLvnItemchanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	if( !CGlobalVariable::IsRunning())
	{
		return;
	}

	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	
	POSITION posSelected = GetFirstSelectedItemPosition();
	if (NULL == posSelected)
	{
		((CDownloadTabWnd*)GetParent())->m_Toolbar.EnableButton(MP_OPENFOLDER, FALSE);
		((CDownloadTabWnd*)GetParent())->m_Toolbar.EnableButton(MP_CANCEL, FALSE);
	}
	else
	{
		ItemData *pItemData = (ItemData *)GetItemData(GetNextSelectedItem(posSelected));
		CKnownFile *pSelFile = NULL;
		CFileTaskItem *pFileTaskItem = NULL;

		if( pItemData && pItemData->type == FILE_TYPE)
		{
			 pSelFile = (CKnownFile *)pItemData->pItem;
			 if( theApp.emuledlg->m_mainTabWnd.m_dlgDownload.IsRemarkTabActived() )
				theApp.emuledlg->m_mainTabWnd.m_dlgDownload.RefreshLowerPannel(pSelFile);
		}

		if( pItemData && pItemData->type == FILE_TASK)
		{
			 pFileTaskItem = (CFileTaskItem *)pItemData->pItem;
			 if( theApp.emuledlg->m_mainTabWnd.m_dlgDownload.IsRemarkTabActived() )
				theApp.emuledlg->m_mainTabWnd.m_dlgDownload.RefreshLowerPannel(pFileTaskItem);
		}

		if (NULL != pSelFile)
		{
			theWndMgr.SendMsgTo(CWndMgr::WI_MAINTAB_DOWNLOAD_DLG, UM_MTDD_CUR_SEL_FILE, 1, (LPARAM) pSelFile);
		}
		else if( pFileTaskItem != NULL)
		{
			theWndMgr.SendMsgTo(CWndMgr::WI_MAINTAB_DOWNLOAD_DLG, UM_MTDD_CUR_SEL_FILE_TASK, 0, (LPARAM)pFileTaskItem);
		}
	}

	//Toolbar
	int iSelectedItems = GetSelectedCount();

	if(iSelectedItems && iSelectedItems == 1)
	{
		((CDownloadTabWnd*)GetParent())->m_Toolbar.EnableButton(MP_OPENFOLDER, TRUE);
	}
	else
	{	
		((CDownloadTabWnd*)GetParent())->m_Toolbar.EnableButton(MP_OPENFOLDER, FALSE);
	}

	if (iSelectedItems && iSelectedItems >= 1)
	{
		((CDownloadTabWnd*)GetParent())->m_Toolbar.EnableButton(MP_CANCEL, TRUE);
	}
	else
	{
		((CDownloadTabWnd*)GetParent())->m_Toolbar.EnableButton(MP_CANCEL, FALSE);
	}

	RedrawItems(pNMLV->iItem, pNMLV->iItem);

	*pResult = 0;
}

void CDownloadedListCtrl::Localize()
{
	LVCOLUMN	lc;
	CString		str;

	lc.mask = LVCF_TEXT;
	
	str = GetResString(IDS_DLED_LC_FILENAME);
	lc.pszText = str.LockBuffer();
	SetColumn(CI_FILE_NAME, &lc);
	str.UnlockBuffer();

	str = GetResString(IDS_DLED_LC_FILESIZE);
	lc.pszText = str.LockBuffer();
	SetColumn(CI_FILE_SIZE, &lc);
	str.UnlockBuffer();

	str = GetResString(IDS_DLED_LC_FILEPATH);
	lc.pszText = str.LockBuffer();
	SetColumn(CI_FILE_PATH, &lc);
	str.UnlockBuffer();

	str = GetResString(IDS_DLED_LC_FinishedTime);
	lc.pszText = str.LockBuffer();
	SetColumn(CI_FILE_FINISHEDTIME, &lc);
	str.UnlockBuffer();
}
void CDownloadedListCtrl::OnNMDblclk(NMHDR* /*pNMHDR*/, LRESULT *pResult)
{
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);

	if (iSel != -1)
	{   
		ItemData *pData = (ItemData*)GetItemData(iSel);

		if(pData->type == FILE_TYPE)
		{
		    CKnownFile* pfile = (CKnownFile*)pData->pItem;
         
		   if(PathFileExists(pfile->GetPath()))
		   {
		      if (!pfile->IsPartFile())
		      {
			    OpenFile(pfile);
		      }
		   }
		   else
		   {
			    MessageBox(GetResString(IDS_DELETEFILEINFO),GetResString(IDS_CAPTION),MB_OK);
		   }
			 
		}
		if(pData->type == FILE_TASK)
		{
			CFileTaskItem *pFileItem = (CFileTaskItem *)pData->pItem;
			CString WholePath;
			WholePath = pFileItem->m_strFilePath + pFileItem->m_FileName;
			if(PathFileExists(WholePath))
			{   
				ShellOpenFile(WholePath);
			}
			else	
			{
				MessageBox(GetResString(IDS_DELETEFILEINFO),GetResString(IDS_CAPTION),MB_OK);
			}
		}
	}
	*pResult = 0;
}

void CDownloadedListCtrl::OnDrawItem(int /*nIDCtl*/, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	m_pMenuXP->DrawItem(lpDrawItemStruct);
}

void CDownloadedListCtrl::OnMeasureItem(int /*nIDCtl*/, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	m_pMenuXP->MeasureItem(lpMeasureItemStruct);
}

void CDownloadedListCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nChar == 'A' && ::GetAsyncKeyState(VK_CONTROL)<0)
	{
		// Ctrl+A: Select all items
		LV_ITEM theItem;
		theItem.mask = LVIF_STATE;
		theItem.iItem = -1;
		theItem.iSubItem = 0;
		theItem.state = LVIS_SELECTED;
		theItem.stateMask = 2;
		SetItemState(-1, &theItem);
	}

	__super::OnKeyDown(nChar, nRepCnt, nFlags);
}

int CDownloadedListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	if (lParam1 == 0)
	{
		return 0;
	}
	ItemData* itemdata1 = (ItemData*)lParam1;
	ItemData* itemdata2 = (ItemData*)lParam2;

	const CFileTaskItem* item1 = NULL;
	const CFileTaskItem* item2 = NULL;

	if (itemdata1->type == FILE_TYPE)
	{
		item1 = CGlobalVariable::filemgr.GetFileTaskItem(((CKnownFile*)(itemdata1->pItem))->GetFileHash());
		if(item1 == NULL)
			item1 = CGlobalVariable::filemgr.GetFileTaskItem(((CKnownFile*)(itemdata1->pItem))->GetPartFileURL());
	}

	if (itemdata1->type == FILE_TASK)
	{
		item1 = (CFileTaskItem*)itemdata1->pItem;
	}

	if (itemdata2->type == FILE_TYPE)
	{
		item2 = CGlobalVariable::filemgr.GetFileTaskItem(((CKnownFile*)(itemdata2->pItem))->GetFileHash());
		if(NULL==item2)
			item2 = CGlobalVariable::filemgr.GetFileTaskItem(((CKnownFile*)(itemdata2->pItem))->GetPartFileURL());
	}

	if (itemdata2->type == FILE_TASK)
	{
		item2 = (CFileTaskItem*)itemdata2->pItem;
	}

	if(NULL==item1 || NULL==item2)
		return 0;

	int iResult=0;
	
	switch(lParamSort)
	{
		case 0: //filename asc
			iResult=CompareLocaleStringNoCase(item1->m_FileName,item2->m_FileName);
			break;
		case 20: //filename desc
			iResult=CompareLocaleStringNoCase(item2->m_FileName,item1->m_FileName);
			break;

		case 1: //filesize asc
			iResult=item1->m_FileSize==item2->m_FileSize?0:(item1->m_FileSize>item2->m_FileSize?1:-1);
			break;

		case 21: //filesize desc
			iResult=item1->m_FileSize==item2->m_FileSize?0:(item2->m_FileSize>item1->m_FileSize?1:-1);
			break;

		case 2: //folder asc
			iResult=CompareLocaleStringNoCase(item1->m_strFilePath,item2->m_strFilePath);
			break;
		case 22: //folder desc
			iResult=CompareLocaleStringNoCase(item2->m_strFilePath,item1->m_strFilePath);
			break;
		case 3: //Time asc
		//	iResult=CompareLocaleStringNoCase(item1->m_tFiletime.Format(_T("%c")),item2->m_tFiletime.Format(_T("%c")));
			iResult=item1->m_tFiletime == item2->m_tFiletime?0:(item1->m_tFiletime>item2->m_tFiletime?1:-1);
			break;
		case 23: //Time desc
		//	iResult=CompareLocaleStringNoCase(item2->m_tFiletime.Format(_T("%c")),item1->m_tFiletime.Format(_T("%c")));
			iResult=item1->m_tFiletime == item2->m_tFiletime?0:(item2->m_tFiletime>item1->m_tFiletime?1:-1);
			break;
	
		default: 
			iResult=0;
			break;
	}

	int dwNextSort;
	
	if (iResult == 0 && (dwNextSort = theApp.emuledlg->m_mainTabWnd.m_dlgDownload.m_lcDownloaded.GetNextSortOrder(lParamSort)) != (-1))
	{
		iResult= SortProc(lParam1, lParam2, dwNextSort);
	}

	return iResult;

}
void CDownloadedListCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	LVHITTESTINFO test;
	test.pt = point;
	test.flags = 0;
	test.iItem = -1;
	test.iSubItem = -1;

	if(ListView_SubItemHitTest(m_hWnd, &test) == -1)
	{
		return;
	}

	__super::OnMouseMove(nFlags, point);
}
