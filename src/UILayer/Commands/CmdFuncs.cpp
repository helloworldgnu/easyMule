/*
 * $Id: CmdFuncs.cpp 11398 2009-03-17 11:00:27Z huby $
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
// CmdFuncs.cpp : 实现文件
//

#include "stdafx.h"
#include "CmdFuncs.h"
#include "eMule.h"
#include "eMuleDlg.h"
#include "WebBrowserWnd.h"
#include "DlgAddTask.h"
#include "GlobalVariable.h"

#include "TabWnd.h"
#include "TabItem_Normal.h"
#include "TabItem_NormalCloseable.h"
#include "TabItem_MainButton.h"
#include <StrSafe.h>
#include "otherfunctions.h"

#include "DNSManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace CmdFuncs
{
	void	OpenPreferencesWnd(void)
	{
		theApp.emuledlg->ShowPreferences((UINT)-1);
	}

	void	OpenNewUrl(LPCTSTR lpszUrl, LPCTSTR lpszCaption)
	{
		//if(theApp.emuledlg->webbrowser!=NULL
		//	&& theApp.emuledlg->webbrowser->IsBrowserCanUse())
		//{
			SetMainActiveTab(CMainTabWnd::TI_RESOURCE);
			theApp.emuledlg->m_mainTabWnd.m_dlgResource.OpenNewUrl(lpszUrl, lpszCaption);
		//}
		//else
		//	ShellExecute(NULL, NULL, lpszUrl, NULL, thePrefs.GetMuleDirectory(EMULE_EXECUTEABLEDIR), SW_SHOWDEFAULT);
	}

	void	GotoGuide()
	{
		OpenNewUrl(_T("http://www.easymule.com/?go=help.html"), GetResString(IDS_HELP));
	}

	void	SetMainActiveTab(CMainTabWnd::ETabId eTabId)
	{
		theApp.emuledlg->m_mainTabWnd.SetActiveTab(theApp.emuledlg->m_mainTabWnd.m_aposTabs[eTabId]);
	}

	void	PopupNewTaskDlg(void)
	{
		CDlgAddTask::PopBlankTaskDlg();
	}


	void	AddEd2kLinksToDownload(CString strlink, int cat)
	{
		int curPos = 0;
		CString resToken = strlink.Tokenize(_T("\t\n\r"), curPos);
		while (resToken != _T(""))
		{
			if (resToken.Right(1) != _T("/"))
				resToken += _T("/");
			try
			{
				CED2KLink* pLink = CED2KLink::CreateLinkFromUrl(resToken.Trim());
				if (pLink)
				{
					if (pLink->GetKind() == CED2KLink::kFile)
					{
						/// CDlgAddTask::AddNewTask(pLink->GetFileLink(), cat);
						CDlgAddTask::AddNewTask(strlink, cat);
					}
					else
					{
						delete pLink;
						throw CString(_T("bad link"));
					}
					delete pLink;
				}
			}
			catch(CString error)
			{
				TCHAR szBuffer[200];
				_sntprintf(szBuffer, ARRSIZE(szBuffer), GetResString(IDS_ERR_INVALIDLINK), error);
				LogError(LOG_STATUSBAR, GetResString(IDS_ERR_LINKERROR), szBuffer);
			}
			resToken = strlink.Tokenize(_T("\t\n\r"), curPos);
		}

		//CDlgAddTask::AddNewTask(strlink, cat);
	}

	void	AddFileLinkToDownload(CED2KFileLink* pLink, int cat)
	{
		CDlgAddTask::AddNewTask(pLink, cat);
	}

	void	AddSearchToDownload(CSearchFile* toadd, uint8 paused, int cat)
	{
		CDlgAddTask::AddNewTask(toadd, paused, cat);
	}

	void	AddSearchToDownload(CString strlink, uint8 paused, int cat)
	{
		CDlgAddTask::AddNewTask(strlink, paused, cat);
	}

	void	AddUrlToDownload(LPCTSTR lpszUrl)
	{
		CDlgAddTask::AddNewUrlTask(lpszUrl);
	}

	void	AddMultiLinksTask(LPCTSTR lpszLinks)
	{
		CString links = lpszLinks;
		CList<CString>		lstLinks;
		::ConvertStrToStrList( &lstLinks , links );

		bool is_have = false;
		POSITION pos = lstLinks.GetHeadPosition();
		while( pos ) {
			CString single_link = lstLinks.GetNext(pos);
			if( FILESTATE_NOT_EXIST != CGlobalVariable::filemgr.GetUrlTaskState( single_link ) ||
				FILESTATE_NOT_EXIST != CGlobalVariable::filemgr.GetFileState( single_link ) ) {
				// 已经存在
				} else {
					is_have = true;
					break;
				}
		}

		if( is_have )
			CDlgAddTask::AddMultiLinks(lpszLinks);
	}

	CString	GetFileSizeDisplayStr(const EMFileSize &fs)
	{
		uint64		u64FileSize;
		CString		strFileSize;

		u64FileSize = fs;
		if (u64FileSize > 1024*1024)
			strFileSize.Format(_T("%.2lf MB"), ((double)u64FileSize)/(1024*1024));
		else if (u64FileSize > 1024)
			strFileSize.Format(_T("%.2lf KB"), ((double)u64FileSize)/1024);
		else
			strFileSize.Format(_T("%d Bytes"), u64FileSize);

		return strFileSize;
	}

	POSITION	TabWnd_AddNormalTab(CTabWnd *pTabWnd, LPCTSTR lpszCaption, HWND hRelativeWnd, LPCTSTR lpszPngIcon)
	{
		CTabItem_Normal	*pNormalTabItem = NULL;

		pNormalTabItem = new CTabItem_Normal;
		pNormalTabItem->SetCaption(lpszCaption);
		pNormalTabItem->SetRelativeWnd(hRelativeWnd);
		pNormalTabItem->SetIcon(lpszPngIcon);
		return pTabWnd->AddTab(pNormalTabItem);
	}

	POSITION	TabWnd_AddCloseTab(CTabWnd *pTabWnd, LPCTSTR lpszCaption,
									HWND hRelativeWnd, BOOL bAutoDelRelaWndObject, CWnd* pRelaWndObjectToDel,
									BOOL bSetActive)
	{
		CTabItem_NormalCloseable	*pTabItem = NULL;

		pTabItem = new CTabItem_NormalCloseable;
		pTabItem->SetCaption(lpszCaption);
		pTabItem->SetRelativeWnd(hRelativeWnd, NULL, bAutoDelRelaWndObject, pRelaWndObjectToDel);
		return pTabWnd->AddTab(pTabItem, bSetActive);
	}

	POSITION	TabWnd_AddMainButton(CTabWnd *pTabWnd, LPCTSTR lpszCaption, HWND hRelativeWnd, LPCTSTR lpszPngIcon, LPCTSTR lpszPngActiveIcon,
									BOOL bSetActive, POSITION posInsertBeside, BOOL bAfter)
	{
		CTabItem_MainButton	*pTabItem = NULL;

		pTabItem = new CTabItem_MainButton;
		pTabItem->SetCaption(lpszCaption);
		pTabItem->SetRelativeWnd(hRelativeWnd);
		pTabItem->SetIcon(lpszPngIcon);
		pTabItem->SetActivedIcon(lpszPngActiveIcon);
		return pTabWnd->AddTab(pTabItem, bSetActive, posInsertBeside, bAfter);
	}

	void	SetShareTabText(LPCTSTR lpszText)
	{
		theApp.emuledlg->m_mainTabWnd.m_dlgShare.SetShareText(lpszText);
	}

	void	SetResActiveSearchTabText(LPCTSTR lpszText)
	{
		CTabWnd *ptw = &(theApp.emuledlg->m_mainTabWnd.m_dlgResource.m_tabWnd);
		POSITION posActive = ptw->GetActiveTab();

		DWORD dwCustomData = ptw->GetTabData(posActive);
		DWORD HighData = dwCustomData & 0xFFFF0000;

		
		if (HighData == 0xF0F00000)
		{
			ptw->SetTabText(posActive, lpszText);
		}
	}

	void	UpdateResSearchParam(int iIndex, SSearchParams *pSearchParams)
	{
		theApp.emuledlg->m_mainTabWnd.m_dlgResource.UpdateSearchParam(iIndex, pSearchParams);
	}

	//{begin} VC-dgkang 2008年7月9日
	void	CreateNewTabForSearchED2K(LPCTSTR lpszCaption,ESearchType * pSearchType/* = NULL */)
	{		
		theApp.emuledlg->m_mainTabWnd.m_dlgResource.OpenNewSearchResult(lpszCaption,pSearchType);

	}
	//{end} VC-dgkang
	void	ImportUnfinishedTasks(void)
	{
		TCHAR buffer[MAX_PATH] = {0};
		
		StringCchCopy(buffer, MAX_PATH, thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR));
		if(SelectDir(AfxGetMainWnd()->GetSafeHwnd(),buffer,GetResString(IDS_SEL_IMPORT_TASK_DIR), NULL, FALSE))
		{
			CGlobalVariable::downloadqueue->ScanPartFile(CString(buffer));
		}

	}

	void	OpenFolder(CKnownFile *file)
	{
		if (file->GetFileSize() == (uint64)0)
		{
			CString	strParam;
			strParam.Format(_T(" %s, /select, %s"), file->GetPath(), file->GetFilePath());
            ShellExecute(NULL, _T("open"), _T("explorer.exe"), file->GetPath(), NULL, SW_SHOW);
		}
		else if (file && !file->IsPartFile())
		{
			CString	strParam;
			strParam.Format(_T(" %s, /select, %s"), file->GetPath(), file->GetFilePath());
			if(PathFileExists(file->GetFilePath()))
			   ShellExecute(NULL, _T("open"), _T("explorer.exe"), strParam, NULL, SW_SHOW);
			else
			{
				 if(IDYES == MessageBox(NULL,GetResString(IDS_OPENFOLDERINFO),GetResString(IDS_CAPTION),MB_ICONQUESTION|MB_DEFBUTTON2|MB_YESNO))
					ShellExecute(NULL, _T("open"), _T("explorer.exe"), file->GetPath(), NULL, SW_SHOW);
			}
		}
	}

	//注意: strLocation 只是存放的的路径名，不包含文件名
	void ActualllyAddUrlDownload(const CString &strUrl, const CString & strLocation, bool bNewTask,CFileTaskItem* pFileTaskItem)
	{
		//CString strUrlEncoded = URLEncode( strUrl );
		//CString urlDecoded = OptUtf8ToStr(URLDecode(strUrl));
		//CString urlConverted = UrlConvert(strUrl);

		// VC-SearchDream[2007-04-06]: Direct HTTP and FTP DownLoad
		if ( (strUrl.Left(7).CompareNoCase(_T("http://")) == 0) || (strUrl.Left(6).CompareNoCase(_T("ftp://")) == 0) )
		{
			TCHAR szScheme[INTERNET_MAX_SCHEME_LENGTH];
			TCHAR szHostName[INTERNET_MAX_HOST_NAME_LENGTH];
			TCHAR szUrlPath[INTERNET_MAX_PATH_LENGTH];
			TCHAR szUserName[INTERNET_MAX_USER_NAME_LENGTH];
			TCHAR szPassword[INTERNET_MAX_PASSWORD_LENGTH];
			TCHAR szExtraInfo[INTERNET_MAX_URL_LENGTH];

			// URL Split
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

			if (InternetCrackUrl(strUrl, 0, 0, &Url) && Url.dwHostNameLength > 0)
			{
				CPartFile* pPartFile=NULL;
				if( CGlobalVariable::filemgr.AddDownLoadRequest(strUrl,strLocation, pPartFile,bNewTask) ) //urlConverted
				{		
					ASSERT( pPartFile );
					if( bNewTask )
					{
						pPartFile->RecordUrlSource( strUrl,true,0,sfStartDown );
					}
					else /// 任务文件丢失后恢复下载任务
					{  						
						pPartFile->StopFile();
						if(pFileTaskItem) 
						{
							pFileTaskItem->m_metBakId = pPartFile->GetMetBakId();
							pPartFile->LoadUrlSiteList( pFileTaskItem->m_lMetaLinkURLList );
						}
					}
				}
			}
		}
	}
}
