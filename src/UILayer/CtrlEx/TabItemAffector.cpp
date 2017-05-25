/*
 * $Id: TabItemAffector.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
// TabItemAffector.cpp : 实现文件
//

#include "stdafx.h"
#include "TabItemAffector.h"
#include "TabItem.h"

// CTabItemAffector
CTabItemAffector::CTabItemAffector()
{
	m_pAssocItem = NULL;
}

CTabItemAffector::~CTabItemAffector()
{
	if (NULL != m_pAssocItem)
		m_pAssocItem->SetAffector(NULL);
}

void CTabItemAffector::SetAssocItem(CTabItem *pAssocItem)
{
	if (NULL != m_pAssocItem)
		m_pAssocItem->m_pAffector = NULL;

	m_pAssocItem = pAssocItem;
}

void CTabItemAffector::SetItemCaption(LPCTSTR lpszCaption)
{
	if (NULL == m_pAssocItem)
		return;

	m_pAssocItem->m_strCaption = lpszCaption;
	m_pAssocItem->Invalidate();
}

// CTabItemAffector 成员函数
