/*
 * $Id: UtilUI.h 4483 2008-01-02 09:19:06Z soarchin $
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

#include "Tools.h"
#include "Draw.h"
#include "CxImage\xImage.h"

inline BOOL IsWindow(CWnd *pWnd)
{
	if ( (NULL != pWnd) && (NULL != pWnd->GetSafeHwnd()) )
		return TRUE;
	
	return FALSE;
}

#define RGB_RED(rgb)	((BYTE)((rgb >> 0) & 0xff))
#define RGB_GREEN(rgb)	((BYTE)((rgb >> 8) & 0xff))
#define RGB_BLUE(rgb)	((BYTE)((rgb >> 16) & 0xff))

void GradientFillV(HDC hdc, long x1, long y1, long x2, long y2, COLORREF c1, COLORREF c2);
void GradientFillH(HDC hdc, long x1, long y1, long x2, long y2, COLORREF c1, COLORREF c2);

void Draw2GradLayerRect(HDC hdc, const CRect &rect, COLORREF c1, COLORREF c2, COLORREF c3, COLORREF c4, UINT nScale);

void RectToVertexArr(const RECT &rect, LPPOINT lpVertexArr, int iStartPos = 0);
void Draw3Borders(HDC hdc, const RECT &rect, int iStartPos = 0);
void DrawRound(HDC hdc, const RECT &rect, int iStartPos = 0, int iRound = 3);

void RegisterSimpleWndClass(void);

void BringWindowToTopExtremely(HWND hWnd);

void RemoveWndTheme(HWND hWnd);

void DrawShdText(HDC hdc, LPCTSTR lpszText, int nCount, LPRECT lpRect, UINT uFormat, COLORREF clrShadow1, COLORREF clrShadow2);

void CopyDcImage(CDC *pDestDC, CDC *pSrcDC);

void DrawParentBk(HWND hThisWnd, HDC hDestDC, const CRect *prectInThisWnd = NULL);

void CenterRect(CRect *prtDest, const CRect &rtSrc, CSize size);

void ToolBarCtrl_SetText(CToolBarCtrl *ptbc, int iIndex, LPCTSTR lpszText);

void CalcTextRect(CRect &rtCalc, LPCTSTR lpszText, LPCTSTR lpszFace, int iFontSize);

HRGN CreateRgnFromBitmap(CWnd *pWnd, HBITMAP hBmp, COLORREF color);
HRGN CreateRgnFromImage(CWnd *pWnd, CxImage *pImage, COLORREF color);
HRGN CreateRgnFromBitmap(CDC *pDC, int iWidth, int iHeight, COLORREF color);

BOOL IsWindowHide(HWND hWnd);
