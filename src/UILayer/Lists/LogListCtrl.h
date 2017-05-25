/*
 * $Id: LogListCtrl.h 5298 2008-04-15 08:35:54Z thilon $
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
#include "TraceEvent.h"
#include <map>
// CLogListCtrl

class EventItem_Struct : public CObject
{
	DECLARE_DYNAMIC(EventItem_Struct)

public:
	~EventItem_Struct() { status.DeleteObject(); }

	void*				owner;
	CTraceEvent*		event;
	DWORD				dwUpdated;
	CBitmap				status;
};

class CLogListCtrl : public CListCtrl
{
	DECLARE_DYNAMIC(CLogListCtrl)

public:
	CLogListCtrl();
	virtual ~CLogListCtrl();

protected:
	COLORREF        m_crWindow;
	COLORREF        m_crWindowTextBk;

	int				m_iRedrawCount;

protected:
	typedef std::pair<void*, EventItem_Struct*> EventItemsPair;
	typedef std::multimap<void*, EventItem_Struct*> EventItems;
	EventItems	m_EventItems;

public:
	void	AddLog(CTraceEvent* add);
	bool	RemoveLog(CTraceEvent* remove);

	void	RemoveEvents(void);

	void	ShowSelectedPeerLogs(CUpDownClient* client);
	void	ShowSelectedFileLogs(CPartFile* pPartFile);

	void SetRedraw(BOOL bRedraw = TRUE)
	{
		if(bRedraw) 
		{
			if(m_iRedrawCount > 0 && --m_iRedrawCount == 0)
			{
				CListCtrl::SetRedraw(TRUE);
			}
		} 
		else 
		{
			if(m_iRedrawCount++ == 0)
			{
				CListCtrl::SetRedraw(FALSE);
			}
		}
	}

protected:
	void SetColors();
	void SetScrollBar();

protected:
	static int CALLBACK SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult);
	void Init(void);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSysColorChange();
protected:
	virtual void PreSubclassWindow();
public:
	afx_msg void OnLvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult);
};
