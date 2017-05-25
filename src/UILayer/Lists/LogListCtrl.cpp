/*
 * $Id: LogListCtrl.cpp 5298 2008-04-15 08:35:54Z thilon $
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
// LogListCtrl.cpp : 实现文件
//

#include "stdafx.h"
#include "emule.h"
#include "LogListCtrl.h"
#include "emuleDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
// CLogListCtrl

IMPLEMENT_DYNAMIC(EventItem_Struct, CObject)

IMPLEMENT_DYNAMIC(CLogListCtrl, CListCtrl)
CLogListCtrl::CLogListCtrl()
{
	m_crWindow = 0;
	m_crWindowTextBk = m_crWindow;
	m_iRedrawCount = 0;
}

CLogListCtrl::~CLogListCtrl()
{
	while (m_EventItems.empty() == false)
	{
		delete m_EventItems.begin()->second;
		m_EventItems.erase(m_EventItems.begin());
	}
}


BEGIN_MESSAGE_MAP(CLogListCtrl, CListCtrl)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnNMCustomdraw)
	ON_WM_ERASEBKGND()
	ON_WM_SYSCOLORCHANGE()
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnLvnGetdispinfo)
END_MESSAGE_MAP()



// CLogListCtrl 消息处理程序
void CLogListCtrl::OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult)
{
	if( !CGlobalVariable::IsRunning() )
	{
		return;
	}

	try
	{
		LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW)pNMHDR;

		switch(lplvcd->nmcd.dwDrawStage)
		{
		case CDDS_PREPAINT:
			*pResult = CDRF_NOTIFYITEMDRAW;
			break;
		case CDDS_ITEMPREPAINT:
			*pResult = CDRF_NOTIFYSUBITEMDRAW;
			break;
		case (CDDS_ITEMPREPAINT | CDDS_SUBITEM):
			{
				EventItem_Struct* lpEventItem = reinterpret_cast<EventItem_Struct*>(GetItemData(lplvcd->nmcd.dwItemSpec));

				CTraceEvent* event = (CTraceEvent*)lpEventItem->event;
				ASSERT(event != NULL);
				ASSERT(lplvcd->iSubItem >= 0);

				if (!IsBadReadPtr(event, sizeof(CTraceEvent*)))
				{
					lplvcd->clrText = event->GetTextColor();
					lplvcd->clrTextBk = event->GetBkColor();
				}

				*pResult = CDRF_DODEFAULT;
			}
			break;

		default:
			break;
		}
	}
	catch (...)
	{
		*pResult = CDRF_DODEFAULT;
	}
}

void CLogListCtrl::RemoveEvents(void)
{
	SetRedraw(FALSE);
	int pre = 0;
	int post = 0;

	for (int i = 0; i < GetItemCount(); i++)
	{
		EventItem_Struct* item = (EventItem_Struct*)GetItemData(i);

		pre++;
		item->dwUpdated = 0;
		item->status.DeleteObject();
		DeleteItem(i--);
		post++;
	}
	SetRedraw(TRUE);
}

void CLogListCtrl::SetColors()
{
	m_crWindow      = ::GetSysColor(COLOR_WINDOW);
	m_crWindowTextBk = m_crWindow;

	SetBkColor(m_crWindow);
	SetTextBkColor(m_crWindowTextBk);
}

void CLogListCtrl::AddLog(CTraceEvent* add)
{
	if (!theApp.emuledlg->IsRunning())
	{
		return;
	}

	EventItem_Struct* newevent = new EventItem_Struct;
	int iItemCount = GetItemCount();

	newevent->owner = NULL;
	newevent->event = add;
	newevent->dwUpdated = 0;

	m_EventItems.insert(EventItemsPair(add, newevent));

	InsertItem(LVIF_PARAM | LVIF_TEXT, iItemCount, LPSTR_TEXTCALLBACK, 0, 0, 0, (LPARAM)newevent);

	SetScrollBar();
}

bool CLogListCtrl::RemoveLog(CTraceEvent* remove)
{
	bool bResult = false;
	if (!theApp.emuledlg->IsRunning())
	{
		return bResult;
	}

	for (EventItems::iterator it = m_EventItems.begin(); it != m_EventItems.end();)
	{
		EventItem_Struct* delEvent = it->second;

		if (delEvent->event == remove)
		{
			it = m_EventItems.erase(it);

			LVFINDINFO find;
			find.flags = LVFI_PARAM;
			find.lParam = (LPARAM)remove;
			int result = FindItem(&find);

			if (result != -1) 
			{
				DeleteItem(result);
			}

			delete delEvent;
			bResult = true;
		}
		else
		{
			it++;
		}
	}

	return bResult;
}

//changed by thilon on 2007.12.07, for刷新问题
void CLogListCtrl::ShowSelectedPeerLogs(CUpDownClient* pClient)
{
	if (!pClient)
	{
		return;
	}

	RemoveEvents();

	SetRedraw(FALSE);
	for(POSITION pos = pClient->GetEventList()->GetTailPosition(); pos != NULL;)
	{
		AddLog((pClient->GetEventList())->GetPrev(pos));
	}
	SetRedraw(TRUE);

	SetScrollBar();
}

//changed by thilon on 2007.12.07, for刷新问题
void CLogListCtrl::ShowSelectedFileLogs(CPartFile* pPartFile)
{
	if (!pPartFile)
	{
		return;
	}

	RemoveEvents();

	SetRedraw(FALSE);
	for(POSITION pos = pPartFile->GetEventList()->GetTailPosition(); pos != NULL;)
	{
		AddLog((pPartFile->GetEventList())->GetPrev(pos));
	}

	SetRedraw(TRUE);

	SetScrollBar();
}

int CLogListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CLogListCtrl* pLogListCtrl = (CLogListCtrl*)lParamSort;
	CString StrItem1 = pLogListCtrl->GetItemText(lParam1, 0);
	CString StrItem2 = pLogListCtrl->GetItemText(lParam2, 0);
	return 0;
}

void CLogListCtrl::Init(void)
{
	SortItems(SortProc, (LPARAM)this);
}

BOOL CLogListCtrl::OnEraseBkgnd(CDC* pDC)
{
	int itemCount = GetItemCount();

	if (!itemCount)
	{
		return CListCtrl::OnEraseBkgnd(pDC);
	}

	RECT clientRect;
	RECT itemRect;
	int topIndex = GetTopIndex();
	int maxItems = GetCountPerPage();
	int drawnItems = itemCount < maxItems ? itemCount : maxItems;
	CRect rcClip;

	GetClientRect(&clientRect);

	rcClip = clientRect;
	GetItemRect(topIndex, &itemRect, LVIR_BOUNDS);
	clientRect.bottom = itemRect.top;

	if (m_crWindowTextBk != CLR_NONE)
	{
		pDC->FillSolidRect(&clientRect,GetBkColor());
	}
	else
	{
		rcClip.top = itemRect.top;
	}

	if(topIndex + maxItems >= itemCount) 
	{
		GetClientRect(&clientRect);
		GetItemRect(topIndex + drawnItems - 1, &itemRect, LVIR_BOUNDS);
		clientRect.top = itemRect.bottom;
		rcClip.bottom = itemRect.bottom;
		if (m_crWindowTextBk != CLR_NONE)
		{
			pDC->FillSolidRect(&clientRect, GetBkColor());
		}
	}

	if (itemRect.right < clientRect.right) 
	{
		GetClientRect(&clientRect);
		clientRect.left = itemRect.right;
		rcClip.right = itemRect.right;
		if (m_crWindowTextBk != CLR_NONE)
		{
			pDC->FillSolidRect(&clientRect, GetBkColor());
		}
	}

	if (m_crWindowTextBk == CLR_NONE)
	{
		CRect rcClipBox;
		pDC->GetClipBox(&rcClipBox);
		rcClipBox.SubtractRect(&rcClipBox, &rcClip);

		if (!rcClipBox.IsRectEmpty())
		{
			pDC->ExcludeClipRect(&rcClip);
			CListCtrl::OnEraseBkgnd(pDC);
			InvalidateRect(&rcClip, FALSE);
		}
	}

	return TRUE;
} 

void CLogListCtrl::PreSubclassWindow()
{
	SetColors();
	CListCtrl::PreSubclassWindow();
}

void CLogListCtrl::OnSysColorChange()
{
	CListCtrl::OnSysColorChange();
	SetColors();
}

void CLogListCtrl::OnLvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	try
	{
		NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);

		if (pDispInfo->item.mask & LVIF_TEXT)
		{
			const EventItem_Struct* pEvent = reinterpret_cast<EventItem_Struct*>(pDispInfo->item.lParam);

			if (pEvent != NULL && pEvent->event != NULL)
			{
				switch(pDispInfo->item.iSubItem)
				{
				case 0:
					if (pDispInfo->item.cchTextMax > 0)
					{
						_tcsncpy(pDispInfo->item.pszText, ((CTraceEvent*)pEvent->event)->GetTime(), pDispInfo->item.cchTextMax);
						pDispInfo->item.pszText[pDispInfo->item.cchTextMax-1] = _T('\0');
					}
					break;
				case 1:
					if (pDispInfo->item.cchTextMax > 0)
					{
						_tcsncpy(pDispInfo->item.pszText, ((CTraceEvent*)pEvent->event)->GetText(), pDispInfo->item.cchTextMax);
						pDispInfo->item.pszText[pDispInfo->item.cchTextMax-1] = _T('\0');
					}
					break;

				default:
					pDispInfo->item.pszText[0] = _T('\0');
				}
			}
		}

		*pResult = 0;
	}
	catch (...)
	{
		
	}
}

void CLogListCtrl::SetScrollBar()
{
	SendMessage(WM_VSCROLL,   SB_BOTTOM,   0);
	Invalidate();
}