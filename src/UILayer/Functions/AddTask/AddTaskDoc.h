/*
 * $Id: AddTaskDoc.h 5171 2008-03-28 07:47:48Z fengwen $
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

#include <map>
#include <set>
using namespace std;

#include <AfxTempl.h>
#include "Publisher.h"
#include "FileHashKey.h"

class CAddTaskDoc : public CPublisher
{
	//	ed2k	<begin>
public:
	struct SItem;

	CAddTaskDoc(void);
	~CAddTaskDoc(void);

	set<CFileHashKey>					GetAllKeysInDoc();
	const map<CFileHashKey, SItem>*		GetData(){return &m_mapItems;}

	void	SetItem(const CFileHashKey &key, const SItem &item, DWORD dwItemMask = IM_ALL, HWND hNotifyExcept = NULL);
	BOOL	GetItem(const CFileHashKey &key, SItem &item);
	void	RemoveItem(const CFileHashKey &key, HWND hNotifyExcept = NULL);
	//void	SetLinkText(const CFileHashKey &key, LPCTSTR szLinkText, HWND hNotifyExcept = NULL);
	//void	ModifyLinkCheck(const CFileHashKey &key, BOOL bCheck, HWND hNotifyExcept = NULL);
protected:
	typedef map<CFileHashKey, SItem>::iterator	ItemsMapIt;
	typedef pair<CFileHashKey, SItem>			ItemsMapPair;
	map<CFileHashKey, SItem>		m_mapItems;
	//	ed2k	<end>

	// Normal Url	<begin>
public:
	void	AppendUrl(LPCTSTR lpszUrl);
	void	AddUrl(LPCTSTR lpszUrl, HWND hNotifyExcept = NULL);
	void	UpdateUrlItems(CMapStringToPtr *pUrlSet, HWND hNotifyExcept = NULL);
	void	CheckUrl(LPCTSTR lpszUrl, BOOL bCheck, HWND hNotifyExcept = NULL);
	
	const CMapStringToPtr*	GetUrlData(){return &m_mapUrls;}
protected:
	CMapStringToPtr				m_mapUrls;
	// Normal Url	<end>
	


public:
	enum ItemMask
	{
		IM_TEXT			= 0x1,
		IM_CHECK		= 0x2,
		IM_CATEGORY		= 0x4,
		IM_PAUSE		= 0x8,

		IM_ALL		= 0xffff
	};
	struct SItem
	{
		CString	strLinkText;
		BOOL	bCheck;
		int		iCategory;
		uint8	uPause;

	public:
		void Empty()
		{
			strLinkText.Empty();
			bCheck	= FALSE;
			iCategory = 0;
			uPause = 0;
		}
		SItem& Set(const SItem &data, DWORD dwMask = IM_ALL)
		{
			if (this == &data)	return *this;

			if (IM_TEXT & dwMask)
				strLinkText = data.strLinkText;
			if (IM_CHECK & dwMask)
				bCheck	= data.bCheck;
			if (IM_CATEGORY & dwMask)
				iCategory = data.iCategory;
			if (IM_PAUSE & dwMask)
				uPause = data.uPause;

			return *this;
		}

		SItem(){Empty();}
		~SItem(){}
		SItem(const SItem &data){Set(data);}
		SItem& operator =(const SItem &data){return Set(data);}
	};

};
