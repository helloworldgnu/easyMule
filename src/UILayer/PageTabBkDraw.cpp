/*
 * $Id: PageTabBkDraw.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
#include ".\pagetabbkdraw.h"
#include "FaceManager.h"

CPageTabBkDraw::CPageTabBkDraw(void)
{
}

CPageTabBkDraw::~CPageTabBkDraw(void)
{
}

void CPageTabBkDraw::Draw(CDC *pDC, const CRect &rect)
{
	CFaceManager::GetInstance()->DrawImage(II_PAGETAB_BK, pDC->GetSafeHdc(), rect, 2);
}
