/*
 * $Id: BrowserToolbarInfo.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
#include ".\browsertoolbarinfo.h"

CBrowserToolbarInfo::CBrowserToolbarInfo(void)
{
	m_pBrowserToolbarCtrl = NULL;
	m_pHtmlCtrl = NULL;
}

CBrowserToolbarInfo::~CBrowserToolbarInfo(void)
{
	if(m_pBrowserToolbarCtrl)
	{
		m_pBrowserToolbarCtrl = NULL;
	}

	if(m_pBrowserToolbarCtrl)
	{
		m_pHtmlCtrl = NULL;
	}
}

void CBrowserToolbarInfo::SetBrowserToolbarCtrl(CBrowserToolbarCtrl* pBrowserToolbarCtrl)
{
	m_pBrowserToolbarCtrl = pBrowserToolbarCtrl;
}

CBrowserToolbarCtrl* CBrowserToolbarInfo::GetBrowserToolbarCtrl(void)
{
	return m_pBrowserToolbarCtrl;
}

void CBrowserToolbarInfo::SetHtmlCtrl(CHtmlCtrl* pHtmlCtrl)
{
	m_pHtmlCtrl = pHtmlCtrl;
}

CHtmlCtrl* CBrowserToolbarInfo::GetHtmlCtrl(void)
{
	return m_pHtmlCtrl;
}

BOOL CBrowserToolbarInfo::Enable(void)
{
	if(m_pHtmlCtrl && m_pBrowserToolbarCtrl)
	{
		return TRUE;
	}

	return FALSE;
}
