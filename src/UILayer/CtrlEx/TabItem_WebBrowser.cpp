/*
 * $Id: TabItem_WebBrowser.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
#include ".\tabitem_webbrowser.h"

CTabItem_WebBrowser::CTabItem_WebBrowser(void)
{
	m_pWbw = NULL;
	//m_iProgressPercent = 100;

	//m_arrProgressIcons[0] = LoadIcon(::AfxGetInstanceHandle(), _T("CONTACT4"));
	//m_arrProgressIcons[1] = LoadIcon(::AfxGetInstanceHandle(), _T("CONTACT3"));
	//m_arrProgressIcons[2] = LoadIcon(::AfxGetInstanceHandle(), _T("CONTACT2"));
	//m_arrProgressIcons[3] = LoadIcon(::AfxGetInstanceHandle(), _T("CONTACT1"));
	//m_arrProgressIcons[4] = LoadIcon(::AfxGetInstanceHandle(), _T("CONTACT0"));

	//m_hIcon = m_arrProgressIcons[4];
}

CTabItem_WebBrowser::~CTabItem_WebBrowser(void)
{
	//int	i;
	//for (i = 0; i < ICON_COUNT; i ++)
	//{
	//	if (NULL != m_arrProgressIcons[i])
	//	{
	//		DestroyIcon(m_arrProgressIcons[i]);
	//	}
	//}
}

int CTabItem_WebBrowser::GetDesireLength()
{
	return 150;
}
void CTabItem_WebBrowser::SetWbWnd(CWebBrowserWnd *pWbw)
{
	m_pWbw = pWbw;
}

//void CTabItem_WebBrowser::OnProgressChanged(long lProgress, long lProgressMax)
//{
//	if (0 == lProgressMax)
//		m_iProgressPercent = 100;
//	else
//		m_iProgressPercent = lProgress * 100 / lProgressMax;
//
//	int	iIconIndex;
//	if (100 == m_iProgressPercent)
//		iIconIndex = ICON_COUNT - 1;
//	else
//		iIconIndex = m_iProgressPercent / 25;
//
//	m_hIcon = m_arrProgressIcons[iIconIndex];
//	
//	Invalidate();
//
////	UpdateCaption();
//}

//void CTabItem_WebBrowser::UpdateCaption()
//{
//	//if (m_iProgressPercent >= 100)
//	//	m_strCaption = m_strTitle;
//	//else
//		m_strCaption.Format(_T("[%3d%%] %s"), m_iProgressPercent, m_strTitle);
//	Invalidate();

