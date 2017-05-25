/*
 * $Id: AddTaskDoc.cpp 5171 2008-03-28 07:47:48Z fengwen $
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
#include "StdAfx.h"
#include ".\addtaskdoc.h"
#include "UserMsgs.h"

CAddTaskDoc::CAddTaskDoc(void)
{
}

CAddTaskDoc::~CAddTaskDoc(void)
{
}

set<CFileHashKey>	CAddTaskDoc::GetAllKeysInDoc()
{
	set<CFileHashKey>	set;
	ItemsMapIt			it;
	
	for (it = m_mapItems.begin();
		m_mapItems.end() != it;
		it++)
	{
		set.insert(it->first);
	}

	return set;
}

void CAddTaskDoc::SetItem(const CFileHashKey &key, const SItem &item, DWORD dwItemMask, HWND hNotifyExcept)
{
	ItemsMapIt		it;

	DWORD		dwModifiedMask = 0;

	it = m_mapItems.find(key);
	if (m_mapItems.end() != it)
	{
		if (IM_TEXT & dwItemMask)
		{
			if (it->second.strLinkText != item.strLinkText)
			{
				dwModifiedMask |= IM_TEXT;
				it->second.strLinkText = item.strLinkText;
			}
		}
		if (IM_CHECK & dwItemMask)
		{
			if (it->second.bCheck != item.bCheck)
			{
				dwModifiedMask |= IM_CHECK;
				it->second.bCheck = item.bCheck;
			}
		}

		SendMsgToAllReceivers(UM_ADDTASK_DOC_MODIFIED, dwModifiedMask, (LPARAM) &key, hNotifyExcept);
	}
	else
	{
		SItem	itemAdd;
		itemAdd.Set(item, dwItemMask);
		m_mapItems.insert(ItemsMapPair(key, itemAdd));
		SendMsgToAllReceivers(UM_ADDTASK_DOC_ADDED, 0, (LPARAM) &key, hNotifyExcept);
	}
}

BOOL CAddTaskDoc::GetItem(const CFileHashKey &key, SItem &item)
{
	ItemsMapIt	it = m_mapItems.find(key);

	if (m_mapItems.end() == it)
		return FALSE;

	item = it->second;
	return TRUE;
}

void CAddTaskDoc::RemoveItem(const CFileHashKey &key, HWND hNotifyExcept)
{
	m_mapItems.erase(key);
	SendMsgToAllReceivers(UM_ADDTASK_DOC_REMOVED, 0, (LPARAM) &key, hNotifyExcept);
}
//
//void CAddTaskDoc::SetLinkText(const CFileHashKey &key, LPCTSTR szLinkText, HWND hNotifyExcept)
//{
//	ItemsMapIt		it;
//
//	it = m_mapItems.find(key);
//	if (m_mapItems.end() != it)
//	{
//		if (0 != it->second.strLinkText.Compare(szLinkText))
//		{
//			it->second.strLinkText = szLinkText;
//			SendMsgToAllReceivers(UM_ADDTASK_DOC_MODIFIED, 0, (LPARAM) &key, hNotifyExcept);
//		}
//	}
//	else
//	{
//		SItem	item;
//		item.strLinkText = szLinkText;
//		item.bCheck = TRUE;
//		m_mapItems.insert(ItemsMapPair(key, item));
//	}
//}
//
//void CAddTaskDoc::ModifyLinkCheck(const CFileHashKey &key, BOOL bCheck, HWND hNotifyExcept)
//{
//	ItemsMapIt		it;
//
//	it = m_mapItems.find(key);
//	if (m_mapItems.end() == it)
//		return;
//
//	if (it->second.bCheck != bCheck)
//	{
//		it->second.bCheck = bCheck;
//		SendMsgToAllReceivers(UM_ADDTASK_DOC_MODIFIED, 0, (LPARAM) &key, hNotifyExcept);
//	}
//}
//
void CAddTaskDoc::AppendUrl(LPCTSTR lpszUrl)
{
	m_mapUrls.SetAt(CString(lpszUrl), (void*)TRUE);
	SendMsgToAllReceivers(UM_ADDTASK_DOC_URL_ADDED, 0, (LPARAM)lpszUrl, NULL);
}

void CAddTaskDoc::AddUrl(LPCTSTR lpszUrl, HWND hNotifyExcept)
{
	CMapStringToPtr	set;
	set.SetAt(lpszUrl, NULL);

	UpdateUrlItems(&set, hNotifyExcept);
}

void CAddTaskDoc::UpdateUrlItems(CMapStringToPtr *pUrlSet, HWND hNotifyExcept)
{
	if (NULL == pUrlSet)
		return;

	CString		strKey;
	BOOL		bChecked;
	POSITION	posSet;
	void*		pDummy;

	// 对已经存在的url的处理。
	POSITION	posMap = m_mapUrls.GetStartPosition();
	while (NULL != posMap)
	{
		m_mapUrls.GetNextAssoc(posMap, strKey, (void*&)bChecked);

		if (pUrlSet->Lookup(strKey, pDummy))
		{
			pUrlSet->RemoveKey(strKey);
			if (!bChecked)
			{
				m_mapUrls.SetAt(strKey, (void*)TRUE);
				SendMsgToAllReceivers(UM_ADDTASK_DOC_URL_MODIFIED, TRUE, (LPARAM)(LPCTSTR)strKey, hNotifyExcept);
			}
		}
		else
		{
			if (bChecked)
			{
				//m_mapUrls.SetAt(strKey, FALSE);
				//SendMsgToAllReceivers(UM_ADDTASK_DOC_URL_MODIFIED, FALSE, (LPARAM)(LPCTSTR)strKey, hNotifyExcept);
				m_mapUrls.RemoveKey(strKey);
				SendMsgToAllReceivers(UM_ADDTASK_DOC_URL_REMOVED, 0, (LPARAM)(LPCTSTR)strKey, hNotifyExcept);
			}
		}
	}

	// 对新增的Url的处理
	posSet = pUrlSet->GetStartPosition();
	while (NULL != posSet)
	{
		pUrlSet->GetNextAssoc(posSet, strKey, pDummy);
		m_mapUrls.SetAt(strKey, (void*)TRUE);
		SendMsgToAllReceivers(UM_ADDTASK_DOC_URL_ADDED, 0, (LPARAM)(LPCTSTR)strKey, hNotifyExcept);
	}
}

void CAddTaskDoc::CheckUrl(LPCTSTR lpszUrl, BOOL bCheck, HWND hNotifyExcept)
{
	BOOL	bOldCheck;
	if (!m_mapUrls.Lookup(lpszUrl, (void*&)bOldCheck))
		return;

	if (bOldCheck != bCheck)
	{
		m_mapUrls.SetAt(lpszUrl, (void*) bCheck);
		SendMsgToAllReceivers(UM_ADDTASK_DOC_URL_MODIFIED, bCheck, (LPARAM)lpszUrl, hNotifyExcept);

	}

}
