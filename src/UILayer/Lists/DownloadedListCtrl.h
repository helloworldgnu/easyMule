/*
 * $Id: DownloadedListCtrl.h 4483 2008-01-02 09:19:06Z soarchin $
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


// CDownloadedListCtrl
#include "resource.h"
#include "KnownFile.h"
#include "Localizee.h"
#include "MenuXP.h"

#include "FileMgr.h"
#include "AffirmDeleteDlg.h"

#include "MuleListCtrl.h"
struct ItemData;
class CDownloadedListCtrl : public CMuleListCtrl, public CLocalizee
{
	DECLARE_DYNAMIC(CDownloadedListCtrl)
	LOCALIZEE_WND_CANLOCALIZE()
public:
	CDownloadedListCtrl();
	virtual ~CDownloadedListCtrl();

protected:
	void OpenFile(const CKnownFile* file);
	void UpdateFile(const CKnownFile* file);
	int FindFile(const CKnownFile* pFile);
	int FindCompleteFile(const CFileTaskItem *pCompleteFile);

protected:
	CMenuXP* m_pMenuXP;
	enum EColId
	{
		CI_FILE_NAME,
		CI_FILE_SIZE,
		CI_FILE_PATH,
		CI_FILE_FINISHEDTIME,
		CI_FILE_COMMENT,

		CI_MAX
	};
	static int CALLBACK CmpProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	int		m_iCurSortCol;
	bool	m_bSortAscending;
	bool			sortstat[4];
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnAddFile(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnRemoveFile(WPARAM wParam, LPARAM lParam);

    afx_msg LRESULT OnCompletedAdd(WPARAM WParam,LPARAM lParam);
	afx_msg LRESULT OnCompletedDelete(WPARAM WParam,LPARAM lParam);

	afx_msg LRESULT OnUpdateFile(WPARAM WParam,LPARAM lParam);

	afx_msg void OnNMRclick(NMHDR *pNMHDR, LRESULT *pResult);
protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	static int CALLBACK SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
public:
	afx_msg void OnLvnColumnclick(NMHDR *pNMHDR, LRESULT *pResult);
	virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
	afx_msg void OnLvnItemchanged(NMHDR *pNMHDR, LRESULT *pResult);
	void Localize(void);
	afx_msg void OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};
struct ItemData
{
public:
	 int type;
	 void *pItem;
};

