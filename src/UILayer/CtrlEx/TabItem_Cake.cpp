/*
 * $Id: TabItem_Cake.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
#include ".\tabitem_cake.h"
#include "Util.h"
#include "FaceManager.h"

CTabItem_Cake::CTabItem_Cake(void)
{
	SetAttribute(ATTR_FIXLEN);
}

CTabItem_Cake::~CTabItem_Cake(void)
{
}

void CTabItem_Cake::Paint(CDC* pDC)
{
	CRect		rtCake;
	rtCake = GetRect();

	if (GetActive())
		CFaceManager::GetInstance()->DrawImage(II_DETAILTAB_A, pDC->GetSafeHdc(), rtCake);
	else
	{
		if (IsHover())
			CFaceManager::GetInstance()->DrawImage(II_DETAILTAB_H, pDC->GetSafeHdc(), rtCake);
		else
			CFaceManager::GetInstance()->DrawImage(II_DETAILTAB_N, pDC->GetSafeHdc(), rtCake);
	}
	CRect		rtIcon;
	enum{ICONSIZE = 32};

	rtIcon = rtCake;
	rtIcon.DeflateRect((rtCake.Width() - ICONSIZE)/2, (rtCake.Height() - ICONSIZE)/2);
	rtIcon.OffsetRect(1, 0);

	if (m_bHasIcon)
		m_imgIcon.Draw(pDC->GetSafeHdc(), rtIcon.left, rtIcon.top);

}

