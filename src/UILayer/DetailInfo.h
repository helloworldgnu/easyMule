/*
 * $Id: DetailInfo.h 4483 2008-01-02 09:19:06Z soarchin $
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

#include "resource.h"
// CDetailInfo 对话框
#include "KnownFile.h"
#include "afxcmn.h"
#include "ResizableLib\ResizableDialog.h"

#include "MenuCmds.h"
//#define MP_SELECTALL         10000
//#define MP_COPYSELECTED      10001 
#include "Localizee.h"

class CDetailInfo : public CResizableDialog, public CLocalizee
{
	DECLARE_DYNAMIC(CDetailInfo)
	LOCALIZEE_WND_CANLOCALIZE()

public:
	CDetailInfo(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDetailInfo();

// 对话框数据
	enum { IDD = IDD_DIALOG_DETAIL };

public:
	CString	GetLink(CKnownFile* pFile);

	enum InfoMask
	{
		IM_FILENAME		= 0x1,
		IM_FILESIZE		= 0x2,
		IM_FILETYPE		= 0x4,
		IM_LINK			= 0x8,
		IM_PRIORITY		= 0x10,
		IM_FILEHASH		= 0x20,
		IM_REQUEST		= 0x40,
		IM_TRANSFERED	= 0x80,
		IM_FILEPATH		= 0x100,
		IM_ACCEPT		= 0x200,
		IM_SOURCE		= 0x400,

		IM_REMAIN		= 0x800,
		IM_LASTCOMPLETE	= 0x1000,
		IM_LASTRECV		= 0x2000,
		IM_CATEGORY		= 0x4000,

		IM_SOURCEURL	= 0x8000, 

		IM_COMBINE_SHARE	= 		IM_FILENAME	| IM_FILESIZE | IM_FILETYPE | IM_LINK | IM_PRIORITY | IM_FILEHASH 
									| IM_REQUEST | IM_TRANSFERED | IM_FILEPATH | IM_ACCEPT | IM_SOURCE | IM_SOURCEURL,

		IM_COMBINE_DOWNLOADED	= 	IM_FILENAME	| IM_FILESIZE | IM_FILETYPE | IM_LINK | IM_FILEHASH 
									| IM_REQUEST | IM_TRANSFERED | IM_FILEPATH | IM_ACCEPT | IM_SOURCE | IM_SOURCEURL,

		IM_COMBINE_DOWNLOAD	=		IM_FILENAME | IM_FILETYPE | IM_LINK | IM_FILEPATH | IM_PRIORITY | IM_TRANSFERED
									| IM_REMAIN | IM_LASTCOMPLETE | IM_LASTRECV /*| IM_CATEGORY*/,

		IM_ALL			= 0xffffffff
	};
	void UpdateInfo(CPartFile* pFile, DWORD dwMask);
    void FileInfo(CFileTaskItem *pFile);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
    virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

	CPartFile		*m_pCurFile;
	DWORD			m_dwCurMask;
	CMenuXP*		m_pMenuXP;
public:
	CListCtrl m_ListDetail;
	int       m_ItemIndex[11];
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnCopySelected();
	afx_msg void OnSelectAll();
	void Localize(void);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
};
