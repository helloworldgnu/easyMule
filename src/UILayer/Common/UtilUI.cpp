/*
 * $Id: UtilUI.cpp 5089 2008-03-21 09:49:19Z fengwen $
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
#include ".\utilui.h"

void GradientFillV(HDC hdc, long x1, long y1, long x2, long y2, COLORREF c1, COLORREF c2)
{
	TRIVERTEX        vert[3] ;
	GRADIENT_RECT    gRect;
	vert [0] .x      = x1;
	vert [0] .y      = y1;
	vert [0] .Red    = RGB_RED(c1) << 8;
	vert [0] .Green  = RGB_GREEN(c1) << 8;
	vert [0] .Blue   = RGB_BLUE(c1) << 8;
	vert [0] .Alpha  = 0x0000;

	vert [1] .x      = x2;
	vert [1] .y      = y2;
	vert [1] .Red    = RGB_RED(c2) << 8;
	vert [1] .Green  = RGB_GREEN(c2) << 8;
	vert [1] .Blue   = RGB_BLUE(c2) << 8;
	vert [1] .Alpha  = 0x0000;

	gRect.UpperLeft  = 0;
	gRect.LowerRight = 1;
	GradientFill(hdc, vert, 2, &gRect, 1, GRADIENT_FILL_RECT_V);
}

void GradientFillH(HDC hdc, long x1, long y1, long x2, long y2, COLORREF c1, COLORREF c2)
{
	TRIVERTEX        vert[3] ;
	GRADIENT_RECT    gRect;
	vert [0] .x      = x1;
	vert [0] .y      = y1;
	vert [0] .Red    = RGB_RED(c1) << 8;
	vert [0] .Green  = RGB_GREEN(c1) << 8;
	vert [0] .Blue   = RGB_BLUE(c1) << 8;
	vert [0] .Alpha  = 0x0000;

	vert [1] .x      = x2;
	vert [1] .y      = y2;
	vert [1] .Red    = RGB_RED(c2) << 8;
	vert [1] .Green  = RGB_GREEN(c2) << 8;
	vert [1] .Blue   = RGB_BLUE(c2) << 8;
	vert [1] .Alpha  = 0x0000;

	gRect.UpperLeft  = 0;
	gRect.LowerRight = 1;
	GradientFill(hdc, vert, 2, &gRect, 1, GRADIENT_FILL_RECT_H);
}

void Draw2GradLayerRect(HDC hdc, const CRect &rect, COLORREF c1, COLORREF c2, COLORREF c3, COLORREF c4, UINT nScale)
{
	if(nScale < 0)
	{
		nScale = 0;
	}

	if(nScale > 100)
	{
		nScale = 100;
	}

	long	m = rect.top + (long)(rect.Height() * nScale / 100.000);
	GradientFillV(hdc, rect.left, rect.top, rect.right, m, c1, c2);
	GradientFillV(hdc, rect.left, m, rect.right, rect.bottom, c3, c4);
}

void RectToVertexArr(const RECT &rect, LPPOINT lpVertexArr, int iStartPos)
{
	int	iIndex = 0;
	int	iPos = iStartPos;

	for (iIndex = 0; iIndex < 4; iIndex++)
	{
		iPos %= 4;
		switch(iPos) 
		{
		case 0:
			lpVertexArr[iIndex].x = rect.left;
			lpVertexArr[iIndex].y = rect.top;
			break;
		case 1:
			lpVertexArr[iIndex].x = rect.right - 1;
			lpVertexArr[iIndex].y = rect.top;
			break;
		case 2:
			lpVertexArr[iIndex].x = rect.right - 1;
			lpVertexArr[iIndex].y = rect.bottom - 1;
			break;
		case 3:
		default:
			lpVertexArr[iIndex].x = rect.left;
			lpVertexArr[iIndex].y = rect.bottom - 1;
			break;
		}
		iPos++;
	}
}

void Draw3Borders(HDC hdc, const RECT &rect, int iStartPos)
{
	CPoint	ptarrVertex[4];
	BYTE	barrType[4] = {PT_MOVETO, PT_LINETO, PT_LINETO, PT_LINETO};

	RectToVertexArr(rect, ptarrVertex, iStartPos);
	PolyDraw(hdc, ptarrVertex, barrType, 4);
	MoveToEx(hdc, ptarrVertex[3].x, ptarrVertex[3].y, NULL);
	LineTo(hdc, ptarrVertex[3].x, ptarrVertex[3].y+1);
}

void DrawRound(HDC hdc, const RECT &rect, int iStartPos, int iRound)
{
//	CPoint arrPoint[10] = {0};
//	
//	//start
//	arrPoint[0].x = rect.left;
//	arrPoint[0].y = rect.bottom - 1;
//
//	arrPoint[1].x = rect.left;
//	arrPoint[1].y = rect.top + 10;
//
//	
//	arrPoint[2].x = rect.left;
//	arrPoint[2].y = rect.top + 10;
//
//    //leftTop
//	arrPoint[3].x = rect.left;
//	arrPoint[3].y = rect.top;
//
//	arrPoint[4].x = rect.left + 10;
//	arrPoint[4].y = rect.top;
/////////////////////////////////////////////////////////////
//	arrPoint[5].x = rect.right - 11;
//	arrPoint[5].y = rect.top;
//
//	arrPoint[6].x = rect.right - 11;
//	arrPoint[6].y = rect.top;
//
//	//righttop
//	arrPoint[7].x = rect.right - 1;
//	arrPoint[7].y = rect.top;
//
//	arrPoint[8].x = rect.right - 1;
//	arrPoint[8].y = rect.top + 9;
//
//	arrPoint[9].x = rect.right - 1;
//	arrPoint[9].y = rect.bottom;
//
//	BYTE	barrType[10] = {PT_MOVETO, PT_LINETO, PT_BEZIERTO, PT_BEZIERTO, PT_BEZIERTO, PT_LINETO, PT_BEZIERTO, PT_BEZIERTO, PT_BEZIERTO, PT_LINETO};
//	PolyDraw(hdc, arrPoint, barrType, 10);


	CPoint arrPoint[6] = {0};

	switch (iStartPos)
	{
	case 3:
		//start
		arrPoint[0].x = rect.left;
		arrPoint[0].y = rect.bottom - 1;
	
		arrPoint[1].x = rect.left;
		arrPoint[1].y = rect.top + iRound;
	
		
		arrPoint[2].x = rect.left + iRound;
		arrPoint[2].y = rect.top;

		arrPoint[3].x = rect.right - iRound - 1;
		arrPoint[3].y = rect.top;

		arrPoint[4].x = rect.right - 1;
		arrPoint[4].y = rect.top + iRound;
	
		arrPoint[5].x = rect.right - 1;
		arrPoint[5].y = rect.bottom;
		break;
	case 1:
		arrPoint[0].x = rect.right - 1;
		arrPoint[0].y = rect.top;

		arrPoint[1].x = rect.right - 1;
		arrPoint[1].y = rect.bottom - iRound - 1;


		arrPoint[2].x = rect.right - iRound - 1;
		arrPoint[2].y = rect.bottom - 1;

		arrPoint[3].x = rect.left + iRound;
		arrPoint[3].y = rect.bottom - 1;

		arrPoint[4].x = rect.left;
		arrPoint[4].y = rect.bottom - iRound - 1;

		arrPoint[5].x = rect.left;
		arrPoint[5].y = rect.top - 1;
		break;
	}


		BYTE	barrType[6] = {PT_MOVETO, PT_LINETO, PT_LINETO, PT_LINETO, PT_LINETO, PT_LINETO};
		PolyDraw(hdc, arrPoint, barrType, 6);

}
void RegisterSimpleWndClass(void)
{
	WNDCLASS wndcls;
	HINSTANCE hInst = AfxGetInstanceHandle();

	if (!(::GetClassInfo(hInst, _T("SimpleWnd"), &wndcls)))
	{
		wndcls.style            = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		wndcls.lpfnWndProc      = ::DefWindowProc;
		wndcls.cbClsExtra       = wndcls.cbWndExtra = 0;
		wndcls.hInstance        = hInst;
		wndcls.hIcon            = NULL;
		wndcls.hCursor          = AfxGetApp()->LoadStandardCursor(IDC_ARROW);
		wndcls.hbrBackground    = (HBRUSH) (COLOR_3DFACE + 1);
		wndcls.lpszMenuName     = NULL;
		wndcls.lpszClassName    = _T("SimpleWnd");

		if (!AfxRegisterClass(&wndcls))
		{
			AfxThrowResourceException();
		}
	}
}

void BringWindowToTopExtremely(HWND hWnd)
{
	if (::IsWindow(hWnd))
	{
		SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	}
}

typedef HRESULT (WINAPI* PFN_SetWindowTheme)(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList);

void RemoveWndTheme(HWND hWnd)
{
	PFN_SetWindowTheme	pfnSetWindowTheme;
	HMODULE hUxTheme = LoadLibrary(_T("UxTheme.dll"));
	if (NULL != hUxTheme)
	{
		pfnSetWindowTheme = (PFN_SetWindowTheme) GetProcAddress(hUxTheme, "SetWindowTheme");
		if (NULL != pfnSetWindowTheme)
		{
			pfnSetWindowTheme(hWnd, L"", L"");

		}
	}
}
void DrawShdText(HDC hdc, LPCTSTR lpszText, int nCount, LPRECT lpRect, UINT uFormat, COLORREF clrShadow1, COLORREF clrShadow2)
{
	COLORREF clrOldText;
	
	clrOldText = ::SetTextColor(hdc, clrShadow1);
	::OffsetRect(lpRect, 1, 1);
	DrawText(hdc, lpszText, nCount, lpRect, uFormat);
	::SetTextColor(hdc, clrOldText);

	clrOldText = ::SetTextColor(hdc, clrShadow2);
	::OffsetRect(lpRect, 1, 1);
	DrawText(hdc, lpszText, nCount, lpRect, uFormat);
	::SetTextColor(hdc, clrOldText);

	::OffsetRect(lpRect, -2, -2);
	DrawText(hdc, lpszText, nCount, lpRect, uFormat);
}

void CopyDcImage(CDC *pDestDC, CDC *pSrcDC)
{
	CRect	rtClipBox;
	pDestDC->GetClipBox(&rtClipBox);
	pDestDC->BitBlt(rtClipBox.left, rtClipBox.top, rtClipBox.Width(), rtClipBox.Height(),
			pSrcDC, rtClipBox.left, rtClipBox.top, SRCCOPY);
}

void DrawParentBk(HWND hThisWnd, HDC hDestDC, const CRect *prectInThisWnd)
{
	CRect	rtParent;
	CRect	rtParentClient;
	CRect	rtWnd;

	HWND	hParentWnd;
	HDC		hParentDC;
	HDC		hMemDC;
	HBITMAP	hMemBmp;
	HBITMAP hOldBmp;
	
	hParentWnd = ::GetParent(hThisWnd);
	if (NULL == hParentWnd)
		return;

	::GetWindowRect(hParentWnd, &rtParent);
	::GetClientRect(hParentWnd, &rtParentClient);
	
	if (NULL == prectInThisWnd)
		::GetWindowRect(hThisWnd, &rtWnd);
	else
	{
		rtWnd = *prectInThisWnd;
		::ClientToScreen(hThisWnd, &rtWnd);
	}

	hParentDC = ::GetWindowDC(hParentWnd);
	hMemDC = ::CreateCompatibleDC(hParentDC);
	hMemBmp = ::CreateCompatibleBitmap(hParentDC, rtParent.Width(), rtParent.Height());
	hOldBmp = (HBITMAP) ::SelectObject(hMemDC, hMemBmp);

	::SendMessage(hParentWnd, WM_ERASEBKGND, (WPARAM) hMemDC, 1);
	if (NULL != prectInThisWnd)
		::BitBlt(hDestDC, prectInThisWnd->left, prectInThisWnd->top, prectInThisWnd->Width(), prectInThisWnd->Height(),
				hMemDC, rtWnd.left - rtParent.left, rtWnd.top - rtParent.top, SRCCOPY);
	else
		::BitBlt(hDestDC, 0, 0, rtWnd.Width(), rtWnd.Height(),
				hMemDC, rtWnd.left - rtParent.left, rtWnd.top - rtParent.top, SRCCOPY);

	::DeleteObject(::SelectObject(hMemDC, hOldBmp));
	::DeleteDC(hMemDC);
	::ReleaseDC(hParentWnd, hParentDC);
}

void CenterRect(CRect *prtDest, const CRect &rtSrc, CSize size)
{
	*prtDest = rtSrc;
	prtDest->DeflateRect((rtSrc.Width() - size.cx)/2, (rtSrc.Height() - size.cy)/2);
}

void ToolBarCtrl_SetText(CToolBarCtrl *ptbc, int iIndex, LPCTSTR lpszText)
{
	CString			strText;
	TBBUTTONINFO	tbbi;

	ZeroMemory(&tbbi, sizeof(tbbi));
	tbbi.cbSize = sizeof(tbbi);
	tbbi.dwMask = TBIF_BYINDEX | TBIF_TEXT;

	strText = lpszText;
	tbbi.pszText = strText.LockBuffer();
	tbbi.cchText = strText.GetLength();
	ptbc->SetButtonInfo(iIndex, &tbbi);
	strText.UnlockBuffer();
}

void CalcTextRect(CRect &rtCalc, LPCTSTR lpszText, LPCTSTR lpszFace, int iFontSize)
{
	HDC hdc = CreateDC(_T("DISPLAY"), NULL, NULL, NULL);

	{
		CFontDC font(hdc, lpszFace);
		font = iFontSize;
		::DrawText(hdc, lpszText, _tcslen(lpszText), &rtCalc, DT_CALCRECT | DT_VCENTER | DT_SINGLELINE | DT_CENTER);
	}

	DeleteDC(hdc);

}

HRGN CreateRgnFromBitmap(CWnd *pWnd, HBITMAP hBmp, COLORREF color)
{
	if (!hBmp)
		return NULL;

	CDC* pDC = pWnd->GetDC();
	if (!pDC)
		return NULL;

	BITMAP bm;
	GetObject( hBmp, sizeof(BITMAP), &bm ); // get bitmap attributes

	CDC dcBmp;
	dcBmp.CreateCompatibleDC(pDC);	//Creates a memory device context for the bitmap
	HGDIOBJ hOldBmp = dcBmp.SelectObject(hBmp);			//selects the bitmap in the device context

	HRGN hRgn = CreateRgnFromBitmap(&dcBmp, bm.bmWidth, bm.bmHeight, color);

	dcBmp.SelectObject(hOldBmp);
	dcBmp.DeleteDC();	//release the bitmap
	pWnd->ReleaseDC(pDC);

	return hRgn;
}

HRGN CreateRgnFromImage(CWnd *pWnd, CxImage *pImage, COLORREF color)
{
	if (NULL == pImage)
		return NULL;

	CDC* pDC = pWnd->GetDC();
	if (!pDC)
		return NULL;

	int iWidth = pImage->GetWidth();
	int iHeight = pImage->GetHeight();

	CDC dcBmp;
	CBitmap	bmp;
	dcBmp.CreateCompatibleDC(pDC);	//Creates a memory device context for the bitmap
	bmp.CreateCompatibleBitmap(pDC, iWidth, iHeight);
	dcBmp.SelectObject(&bmp);
	pImage->Draw(dcBmp.GetSafeHdc());

	HRGN hRgn = CreateRgnFromBitmap(&dcBmp, iWidth, iHeight, color);

	bmp.DeleteObject();
	dcBmp.DeleteDC();
	pWnd->ReleaseDC(pDC);

	return hRgn;
}
HRGN CreateRgnFromBitmap(CDC *pDC, int iWidth, int iHeight, COLORREF color)
{
	const DWORD RDHDR = sizeof(RGNDATAHEADER);
	const DWORD MAXBUF = 40;		// size of one block in RECTs
									// (i.e. MAXBUF*sizeof(RECT) in bytes)
	LPRECT	pRects;
	DWORD	cBlocks = 0;			// number of allocated blocks

	INT		i, j;					// current position in mask image
	INT		first = 0;				// left position of current scan line
									// where mask was found
	bool	wasfirst = false;		// set when if mask was found in current scan line
	bool	ismask;					// set when current color is mask color

	// allocate memory for region data
	RGNDATAHEADER* pRgnData = (RGNDATAHEADER*)new BYTE[ RDHDR + ++cBlocks * MAXBUF * sizeof(RECT) ];
	memset( pRgnData, 0, RDHDR + cBlocks * MAXBUF * sizeof(RECT) );
	// fill it by default
	pRgnData->dwSize	= RDHDR;
	pRgnData->iType		= RDH_RECTANGLES;
	pRgnData->nCount	= 0;
	for ( i = 0; i < iHeight; i++ )
		for ( j = 0; j < iWidth; j++ ){
			// get color
			ismask=(pDC->GetPixel(j,iHeight-i-1)!=color);
			// place part of scan line as RECT region if transparent color found after mask color or
			// mask color found at the end of mask image
			if (wasfirst && ((ismask && (j==(iWidth-1)))||(ismask ^ (j<iWidth)))){
				// get offset to RECT array if RGNDATA buffer
				pRects = (LPRECT)((LPBYTE)pRgnData + RDHDR);
				// save current RECT
				pRects[ pRgnData->nCount++ ] = CRect( first, iHeight - i - 1, j+(j==(iWidth-1)), iHeight - i );
				// if buffer full reallocate it
				if ( pRgnData->nCount >= cBlocks * MAXBUF ){
					LPBYTE pRgnDataNew = new BYTE[ RDHDR + ++cBlocks * MAXBUF * sizeof(RECT) ];
					memcpy( pRgnDataNew, pRgnData, RDHDR + (cBlocks - 1) * MAXBUF * sizeof(RECT) );
					delete[] pRgnData;
					pRgnData = (RGNDATAHEADER*)pRgnDataNew;
				}
				wasfirst = false;
			} else if ( !wasfirst && ismask ){		// set wasfirst when mask is found
				first = j;
				wasfirst = true;
			}
		}

	// create region
	/*	Under WinNT the ExtCreateRegion returns NULL (by Fable@aramszu.net) */
	//	HRGN hRgn = ExtCreateRegion( NULL, RDHDR + pRgnData->nCount * sizeof(RECT), (LPRGNDATA)pRgnData );
	/* ExtCreateRegion replacement { */
	HRGN hRgn=CreateRectRgn(0, 0, 0, 0);
	ASSERT( hRgn!=NULL );
	pRects = (LPRECT)((LPBYTE)pRgnData + RDHDR);
	for(i=0;i<(int)pRgnData->nCount;i++)
	{
		HRGN hr=CreateRectRgn(pRects[i].left, pRects[i].top, pRects[i].right, pRects[i].bottom);
		VERIFY(CombineRgn(hRgn, hRgn, hr, RGN_OR)!=ERROR);
		if (hr) DeleteObject(hr);
	}
	ASSERT( hRgn!=NULL );
	/* } ExtCreateRegion replacement */

	delete[] pRgnData;
	return hRgn;
}

BOOL IsWindowHide(HWND hWnd)
{
	WINDOWPLACEMENT	wp;
	ZeroMemory(&wp, sizeof(wp));
	wp.length = sizeof(wp);
	if (GetWindowPlacement(hWnd, &wp))
		return wp.showCmd & SW_HIDE;

	return TRUE;
}
