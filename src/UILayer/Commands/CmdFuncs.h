/*
 * $Id: CmdFuncs.h 6280 2008-07-17 11:29:03Z dgkang $
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
#pragma once

//#include "stdafx.h"
#include "MainTabWnd.h"
#include "ED2KLink.h"
#include "SearchFile.h"

class CTabWnd;


namespace CmdFuncs
{
	void	OpenPreferencesWnd(void);
	void	OpenNewUrl(LPCTSTR lpszUrl, LPCTSTR lpszCaption);
	void	GotoGuide();
	void	SetMainActiveTab(CMainTabWnd::ETabId eTabId);

	void	PopupNewTaskDlg(void);
	void	AddEd2kLinksToDownload(CString strlink, int cat = 0);
	void	AddFileLinkToDownload(CED2KFileLink* pLink, int cat = 0);
	void	AddSearchToDownload(CSearchFile* toadd, uint8 paused = 2, int cat = 0);
	void	AddSearchToDownload(CString strlink, uint8 paused = 2, int cat = 0);
	void	AddMultiLinksTask(LPCTSTR lpszLinks);

	void	AddUrlToDownload(LPCTSTR lpszUrl);

	CString	GetFileSizeDisplayStr(const EMFileSize &fs);


	POSITION	TabWnd_AddNormalTab(CTabWnd *pTabWnd, LPCTSTR lpszCaption, HWND hRelativeWnd, LPCTSTR lpszPngIcon = NULL);
	POSITION	TabWnd_AddCloseTab(CTabWnd *pTabWnd, LPCTSTR lpszCaption,
									HWND hRelativeWnd, BOOL bAutoDelRelaWndObject = FALSE, CWnd* pRelaWndObjectToDel = NULL,
									BOOL bSetActive = FALSE);
	POSITION	TabWnd_AddMainButton(CTabWnd *pTabWnd, LPCTSTR lpszCaption, HWND hRelativeWnd, LPCTSTR lpszPngIcon = NULL, LPCTSTR lpszPngActiveIcon = NULL,
								BOOL bSetActive = FALSE, POSITION posInsertBeside = NULL, BOOL bAfter = TRUE);

	void	SetShareTabText(LPCTSTR lpszText);

	void	SetResActiveSearchTabText(LPCTSTR lpszText);
	void	UpdateResSearchParam(int iIndex, SSearchParams *pSearchParams);

	//{begin} VC-dgkang 2008Äê7ÔÂ9ÈÕ
	void    CreateNewTabForSearchED2K(LPCTSTR lpszText,ESearchType * pSearchType = NULL);
	//{end}

	void	ImportUnfinishedTasks(void);

	void	OpenFolder(CKnownFile *file);

	void ActualllyAddUrlDownload(const CString &strUrl, const CString & strLocation,bool bNewTask = true,CFileTaskItem* pFileTaskItem=NULL);
}
