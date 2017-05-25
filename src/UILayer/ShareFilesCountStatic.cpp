/*
 * $Id: ShareFilesCountStatic.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
// ShareFilesCountStatic.cpp : 实现文件
//

#include "stdafx.h"
#include "ShareFilesCountStatic.h"
#include ".\ShareFilesCountStatic.h"
#include "FaceManager.h"
#include "resource.h"
#include "otherfunctions.h"
#include "Util.h"
#include "PageTabBkDraw.h"
// CShareFilesCountStatic

IMPLEMENT_DYNAMIC(CShareFilesCountStatic, CStatic)
CShareFilesCountStatic::CShareFilesCountStatic()
{
}

CShareFilesCountStatic::~CShareFilesCountStatic()
{
}


BEGIN_MESSAGE_MAP(CShareFilesCountStatic, CStatic)
END_MESSAGE_MAP()


void CShareFilesCountStatic::DrawBk(CDC *pDC, const CRect &rect)
{
	CPageTabBkDraw	bkDraw;
	bkDraw.Draw(pDC, rect);
}

void CShareFilesCountStatic::DrawText(CDC *pDC, const CRect &rect)
{
	CFaceManager::GetInstance()->DrawFace(FI_SHARE_FILE_COUNT, pDC->GetSafeHdc(), FS_NORMAL, rect);

	CRect	rtText;
	rtText = rect;

	CString	str;
	GetWindowText(str);

	CFontDC	fontDC(pDC->GetSafeHdc(), _T("宋体"), RGB(255, 255, 255));
	fontDC = 12;

	int			iOldBkMode = pDC->SetBkMode(TRANSPARENT);
	pDC->DrawText(str, &rtText, DT_CENTER | DT_SINGLELINE | DT_VCENTER | DT_WORD_ELLIPSIS);
	pDC->SetBkMode(iOldBkMode);
}

// CShareFilesCountStatic 消息处理程序

void CShareFilesCountStatic::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	// TODO:  添加您的代码以绘制指定项
	CDC	dc;
	dc.Attach(lpDrawItemStruct->hDC);

	CRect	rect = lpDrawItemStruct->rcItem;
	DrawBk(&dc, rect);
	rect.DeflateRect(8, 3);
	DrawText(&dc, rect);

	dc.Detach();
}

void CShareFilesCountStatic::PreSubclassWindow()
{
	// TODO: 在此添加专用代码和/或调用基类
	ModifyStyle(0, SS_OWNERDRAW);

	CStatic::PreSubclassWindow();
}
