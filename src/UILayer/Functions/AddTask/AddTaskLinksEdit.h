/*
 * $Id: AddTaskLinksEdit.h 5171 2008-03-28 07:47:48Z fengwen $
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

#include "AddTaskDoc.h"
// CAddTaskLinksEdit

class CAddTaskLinksEdit : public CEdit
{
	DECLARE_DYNAMIC(CAddTaskLinksEdit)

public:
	CAddTaskLinksEdit();
	virtual ~CAddTaskLinksEdit();

	void	SetDoc(CAddTaskDoc	*pDoc){m_pDoc = pDoc;}
	void	UpdateLinksByWindowText();
protected:
	void	AddText(LPCTSTR lpszText);
	void	RemoveLine(LPCTSTR lpszText);

	void	SetText(const CFileHashKey &key, LPCTSTR lpszText);
	void	RemoveText(const CFileHashKey &key);

	CString	RemoveLine(const CString &str, int iStart, int iEnd);

	BOOL	FindLineByKey(const CFileHashKey &key, int &iStartPos, int &iEndPos);

	CAddTaskDoc	*m_pDoc;
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEnChange();
	afx_msg LRESULT OnDocAdded(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDocModified(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDocUrlAdded(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDocUrlModified(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDocUrlRemoved(WPARAM wParam, LPARAM lParam);
};


