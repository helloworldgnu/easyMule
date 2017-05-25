/*
 * $Id: CIF.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
#include "CIF.h"
#include ".\cif.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CCIF cif;

CCIF::CCIF(void)
{
	InitColor();
	m_czBuffer = CSize( 0, 0 );
}

CCIF::~CCIF(void)
{
	Clear();
}

//////////////////////////////////////////
//Color
//////////////////////////////////////////
/****************************************************************************
                          

函数名:
       CalculateColour(COLORREF crFore, COLORREF crBack, int nAlpha)
函数功能:
		计算带有透明度的颜色值      	
被本函数调用的函数清单:
      		
调用本函数的函数清单:
						CColor::GetDialogBackColor()     
参数:
     COLORREF crFore 前景色(0-255)
	 COLORREF crBack 背景色(0-255)
	 int      nAlpha 透明度(0-255)

返回值:
       COLORREF: 返回混合后的RGB颜色值
内容:
  

****************************************************************************/
COLORREF CCIF::CalculateColor(COLORREF crFore, COLORREF crBack, int nAlpha)
{
		int nRed	= GetRValue( crFore ) * ( 255 - nAlpha ) / 255 + GetRValue( crBack ) * nAlpha / 255;
		int nGreen	= GetGValue( crFore ) * ( 255 - nAlpha ) / 255 + GetGValue( crBack ) * nAlpha / 255;
		int nBlue	= GetBValue( crFore ) * ( 255 - nAlpha ) / 255 + GetBValue( crBack ) * nAlpha / 255;

	return RGB( nRed, nGreen, nBlue );
}

/****************************************************************************
                          

函数名:
       GetDialogBackColor()
函数功能:
		得到对话框的背景色
      	
被本函数调用的函数清单:
      		
调用本函数的函数清单:
      
参数:

返回值:
        COLORREF: 返回对话框的背景色
内容:
  

****************************************************************************/
COLORREF CCIF::GetDialogBackColor()
{
		return CalculateColor(GetSysColor(COLOR_BTNFACE), GetSysColor(COLOR_WINDOW), 200);
}

/****************************************************************************
                          
函数名:
       InitColor()
函数功能:
		初始化颜色值      	
被本函数调用的函数清单:
      					CColor()
调用本函数的函数清单:
      
参数:

返回值:
        
内容:
  

****************************************************************************/
void CCIF::InitColor()
{
		m_crWindow				= GetSysColor( COLOR_WINDOW );
		m_crMidtone				= GetSysColor( COLOR_BTNFACE );
		m_crHighlight			= GetSysColor( COLOR_HIGHLIGHT );
		m_crText					= GetSysColor( COLOR_WINDOWTEXT );
		m_crHiText				= GetSysColor( COLOR_HIGHLIGHTTEXT );
		
		m_crBackNormal			= CalculateColor( m_crMidtone, m_crWindow, 215 );//背景色
		m_crBackSel				= CalculateColor( m_crHighlight, m_crWindow, 178 );//RGB(255,238,194);//高亮色
		m_crBackCheck			= CalculateColor( m_crHighlight, m_crWindow, 200 );//复选标记背景色
		m_crBackCheckSel		= CalculateColor( m_crHighlight, m_crWindow, 127 );//复选标记选中时背景色
		m_crMargin				= CalculateColor( m_crMidtone, m_crWindow, 39 );//CalculateColor( m_crHighlight, m_crWindow, 178 );//菜单左边边缘背景色
		m_crShadow				= CalculateColor( m_crHighlight, GetSysColor( COLOR_3DSHADOW ), 200 );
		m_crBorder				= m_crHighlight;
		m_crCmdText				= GetSysColor( COLOR_MENUTEXT );
		m_crCmdTextSel			= GetSysColor( COLOR_MENUTEXT );
		m_crDisabled			= GetSysColor( COLOR_GRAYTEXT );
		m_crNoFocusLine			= CalculateColor( m_crHighlight, m_crWindow,  150);
}

//============================================================================================================

//////////////////////////////////////////
//Icon
//////////////////////////////////////////
void CCIF::AddIcon(UINT nID, HICON hIcon, BOOL bGrayscale)
{
		if(!bGrayscale)
		{
			ConfirmImageList();
			int nImage = m_ImageList.Add( hIcon );
			m_pImageMap.SetAt((LPVOID)nID, (LPVOID)nImage);
		}
		else
		{
			ConfirmGrayImageList();

			int nImage = m_GrayImageList.Add(CreateGrayscaleIcon(hIcon));
			m_pGrayImageMap.SetAt((LPVOID)nID, (LPVOID)nImage);
		}
}

BOOL CCIF::ConfirmImageList()
{
		if(m_ImageList.m_hImageList != NULL)
		{
			return TRUE;
		}

		if(m_ImageList.Create(16, 16, ILC_COLOR16|ILC_MASK, 16, 4))
		{
			return TRUE;
		}
		else
		{
			return m_ImageList.Create(16, 16, ILC_COLOR24|ILC_MASK, 16, 4);
		}
}

BOOL CCIF::ConfirmGrayImageList()
{
	if(m_GrayImageList.m_hImageList != NULL)
	{
		return TRUE;
	}

	if(m_GrayImageList.Create(16, 16, ILC_COLOR16|ILC_MASK, 16, 4))
	{
		return TRUE;
	}
	else
	{
		return m_GrayImageList.Create(16, 16, ILC_COLOR24|ILC_MASK, 16, 4);
	}
}

void CCIF::CopyIcon(UINT nFromID, UINT nToID)
{
		int nImage;

		if(m_pImageMap.Lookup((LPVOID)nFromID, (void*&)nImage))
		{
			m_pImageMap.SetAt((LPVOID)nToID, (LPVOID)nImage);
		}
}

HICON CCIF::ExtractIcon(UINT nID)
{
		int nImage = ImageForID( nID );//获得图片ID
		if ( nImage >= 0 )
		{
			return m_ImageList.ExtractIcon( nImage );//返回指向图标的句柄
		}

		return NULL;
}

int CCIF::ImageForID(UINT nID)
{
		int nImage;

		if (m_pImageMap.Lookup((LPVOID)nID, (void*&)nImage))
		{
			return nImage;
		}
		return -1;
}

int CCIF::GrayImageForID(UINT nID)
{
	int nImage;

	if (m_pGrayImageMap.Lookup((LPVOID)nID, (void*&)nImage))
	{
		return nImage;
	}
	return -1;
}

void CCIF::Clear()
{
		m_pImageMap.RemoveAll();
		if(m_ImageList.m_hImageList)
		{
			m_ImageList.DeleteImageList();
		}

		m_pGrayImageMap.RemoveAll();
		if(m_GrayImageList.m_hImageList)
		{
			m_GrayImageList.DeleteImageList();
		}

		if(m_bmBuffer.m_hObject != NULL)
		{
			m_dcBuffer.SelectObject( CGdiObject::FromHandle(m_bmOldBuffer));
			m_dcBuffer.DeleteDC();
			m_bmBuffer.DeleteObject();
		}

		m_czBuffer = CSize( 0, 0 );
}

CDC* CCIF::GetBuffer(CDC &dcScreen, const CSize &szItem)
{
		if ( szItem.cx <= m_czBuffer.cx && szItem.cy <= m_czBuffer.cy )
		{
			m_dcBuffer.SelectClipRgn( NULL );
			return &m_dcBuffer;
		}
	
		if ( m_bmBuffer.m_hObject )
		{
			m_dcBuffer.SelectObject( CGdiObject::FromHandle( m_bmOldBuffer ) );
			m_bmBuffer.DeleteObject();
		}
	
		m_czBuffer.cx = max( m_czBuffer.cx, szItem.cx );
		m_czBuffer.cy = max( m_czBuffer.cy, szItem.cy );
	
		if ( m_dcBuffer.m_hDC == NULL ) m_dcBuffer.CreateCompatibleDC( &dcScreen );

		m_bmBuffer.CreateCompatibleBitmap( &dcScreen, m_czBuffer.cx, m_czBuffer.cy );
		m_bmOldBuffer = (HBITMAP)m_dcBuffer.SelectObject( &m_bmBuffer )->GetSafeHandle();
	
		return &m_dcBuffer;
}

BOOL CCIF::AddImagesFromToolbar(UINT nIDToolBar, COLORREF crBack)
{
		ConfirmImageList();
	
		CBitmap pBmp;
		if ( ! pBmp.LoadBitmap( nIDToolBar ) ) return FALSE;
		int nBase = m_ImageList.Add( &pBmp, crBack );
		pBmp.DeleteObject();
	
		if ( nBase < 0 ) return FALSE;
	
		HRSRC hRsrc = FindResource( AfxGetInstanceHandle(), MAKEINTRESOURCE(nIDToolBar), RT_TOOLBAR );
		if ( hRsrc == NULL ) return FALSE;
	
		HGLOBAL hGlobal = LoadResource( AfxGetInstanceHandle(), hRsrc );
		if ( hGlobal == NULL ) return FALSE;
	
		TOOLBAR_RES* pData = (TOOLBAR_RES*)LockResource( hGlobal );
	
		if ( pData == NULL )
		{
			FreeResource( hGlobal );
			return FALSE;
		}
	
		for ( WORD nItem = 0 ; nItem < pData->wItemCount ; nItem++ )
		{
				if ( pData->items()[ nItem ] != ID_SEPARATOR )
				{
					m_pImageMap.SetAt( (LPVOID)(DWORD)pData->items()[ nItem ],(LPVOID)nBase );
					nBase++;
				}
		}
	
		UnlockResource( hGlobal );
		FreeResource( hGlobal );
	
		return TRUE;
}

UINT CCIF::NameToID(LPCTSTR pszName)
{
		UINT nID = 0;
		if ( m_pNameMap.Lookup( pszName, (void*&)nID ) ) return nID;
		if ( _stscanf( pszName, _T("%lu"), &nID ) == 1 ) return nID;

		return 0;
}

void CCIF::NameCommand(UINT nID, LPCTSTR pszName)
{
		m_pNameMap.SetAt( pszName, (LPVOID)nID );
}

BOOL CCIF::DrawWatermark(CDC *pDC, CRect *pRect, CBitmap *pMark, int nOffX, int nOffY)
{
		BITMAP pWatermark;
	CBitmap* pOldMark;
	CDC dcMark;
	
	if ( pDC == NULL || pRect == NULL || pMark == NULL || pMark->m_hObject == NULL )
		return FALSE;
	
	dcMark.CreateCompatibleDC( pDC );
	pOldMark = (CBitmap*)dcMark.SelectObject( pMark );
	pMark->GetBitmap( &pWatermark );
	
	for ( int nY = pRect->top - nOffY ; nY < pRect->bottom ; nY += pWatermark.bmHeight )
	{
		if ( nY + pWatermark.bmHeight < pRect->top ) continue;
		
		for ( int nX = pRect->left - nOffX ; nX < pRect->right ; nX += pWatermark.bmWidth )
		{
			if ( nX + pWatermark.bmWidth < pRect->left ) continue;
			
			pDC->BitBlt( nX, nY, pWatermark.bmWidth, pWatermark.bmHeight, &dcMark, 0, 0, SRCCOPY );
		}
	}
	
	dcMark.SelectObject( pOldMark );
	dcMark.DeleteDC();
	
	return TRUE;

}

//============================================================================================================

//////////////////////////////////////////
//Font
//////////////////////////////////////////
void CCIF::CreateFonts(LPCTSTR pszFace, int nSize)
{
	if ( ! pszFace )
	{
		pszFace = _T("宋体");	//字体(宋体)
	}

	if ( ! nSize )
	{
		nSize = 12;//字号(12)
	}
	
	if ( m_fntUP.m_hObject )
	{
		m_fntUP.DeleteObject();
	}

	if ( m_fntBold.m_hObject )
	{
		m_fntBold.DeleteObject();
	}

	if ( m_fntDown.m_hObject )
	{
		m_fntDown.DeleteObject();
	}
		
	//if ( m_fntCaption.m_hObject ) m_fntCaption.DeleteObject();
	
	m_fntUP.CreateFont( -nSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,DEFAULT_PITCH|FF_DONTCARE, pszFace );
	m_fntBold.CreateFont( -nSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,DEFAULT_PITCH|FF_DONTCARE, pszFace );
	m_fntDown.CreateFont( -nSize, 0, 0, 0, FW_NORMAL, FALSE, TRUE, FALSE,DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,DEFAULT_PITCH|FF_DONTCARE, pszFace );
	
		/*m_fntCaption.CreateFont( -nSize - 2, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH|FF_DONTCARE, pszFace );*/
}

HICON CCIF::CreateGrayscaleIcon(HICON hIcon)
{
	HICON		hGrayIcon	= NULL;	//灰度图标
	HDC			hMainDC		= NULL;
	HDC			hMemDC1		= NULL;
	HDC			hMemDC2		= NULL;

	BITMAP		bmp;
	HBITMAP		hOldBmp1	= NULL;
	HBITMAP		hOldBmp2	= NULL;

	ICONINFO	csII;		//正常图标信息
	ICONINFO	csGrayII;	//灰度图标信息

	BOOL		bRetValue = FALSE;

	bRetValue = GetIconInfo(hIcon, &csII);

	if (bRetValue == FALSE)
	{
		return NULL;
	}

	hMainDC = GetDC(NULL);
	hMemDC1 = CreateCompatibleDC(hMainDC);
	hMemDC2 = CreateCompatibleDC(hMainDC);

	if (hMainDC == NULL || hMemDC1 == NULL || hMemDC2 == NULL)
	{
		return NULL;
	}

	if (GetObject(csII.hbmColor, sizeof(BITMAP), &bmp))
	{
		DWORD	dwWidth	= csII.xHotspot*2;
		DWORD	dwHeight = csII.yHotspot*2;

		csGrayII.hbmColor = ::CreateBitmap(dwWidth, dwHeight, bmp.bmPlanes, bmp.bmBitsPixel, NULL);
		if (csGrayII.hbmColor)
		{
			hOldBmp1 = (HBITMAP)::SelectObject(hMemDC1, csII.hbmColor);
			hOldBmp2 = (HBITMAP)::SelectObject(hMemDC2, csGrayII.hbmColor);

			DWORD		dwLoopY = 0, dwLoopX = 0;
			COLORREF	crPixel = 0;
			BYTE		byNewPixel = 0;

			for (dwLoopY = 0; dwLoopY < dwHeight; dwLoopY++)
			{
				for (dwLoopX = 0; dwLoopX < dwWidth; dwLoopX++)
				{
					crPixel = ::GetPixel(hMemDC1, dwLoopX, dwLoopY);

					byNewPixel = (BYTE)((GetRValue(crPixel) * 0.299) + (GetGValue(crPixel) * 0.587) + (GetBValue(crPixel) * 0.114));
					if (crPixel)
					{
						::SetPixel(hMemDC2, dwLoopX, dwLoopY, RGB(byNewPixel, byNewPixel, byNewPixel));
					}
				}
			}

			SelectObject(hMemDC1, hOldBmp1);
			SelectObject(hMemDC2, hOldBmp2);

			csGrayII.hbmMask = csII.hbmMask;

			csGrayII.fIcon = TRUE;
			hGrayIcon = ::CreateIconIndirect(&csGrayII);
		}

		VERIFY( DeleteObject(csGrayII.hbmColor));
	}

	VERIFY( DeleteObject(csII.hbmColor) );
	VERIFY( DeleteObject(csII.hbmMask) );
	VERIFY( DeleteDC(hMemDC1) );
	VERIFY( DeleteDC(hMemDC2) );
	ReleaseDC(NULL, hMainDC);

	return hGrayIcon;
}
void CCIF::GradientFillEx(HDC hdc, long x1, long y1, long x2, long y2, COLORREF crFore, COLORREF crBack, bool bHorz)
{
	TRIVERTEX        vert[3] ;
	GRADIENT_RECT    gRect;

	if(bHorz)
	{
		vert [0] .x      = x1;
		vert [0] .y      = y1;
		vert [0] .Red    = RGB_RED(crFore) << 8;
		vert [0] .Green  = RGB_GREEN(crFore) << 8;
		vert [0] .Blue   = RGB_BLUE(crFore) << 8;
		vert [0] .Alpha  = 0x0000;

		vert [1] .x      = x2;
		vert [1] .y      = y2;
		vert [1] .Red    = RGB_RED(crBack) << 8;
		vert [1] .Green  = RGB_GREEN(crBack) << 8;
		vert [1] .Blue   = RGB_BLUE(crBack) << 8;
		vert [1] .Alpha  = 0x0000;

		gRect.UpperLeft  = 0;
		gRect.LowerRight = 1;
	}
	else
	{
		vert [0] .x      = x1;
		vert [0] .y      = y1;
		vert [0] .Red    = RGB_RED(crFore) << 8;
		vert [0] .Green  = RGB_GREEN(crFore) << 8;
		vert [0] .Blue   = RGB_BLUE(crFore) << 8;
		vert [0] .Alpha  = 0x0000;

		vert [1] .x      = x2;
		vert [1] .y      = y2;
		vert [1] .Red    = RGB_RED(crBack) << 8;
		vert [1] .Green  = RGB_GREEN(crBack) << 8;
		vert [1] .Blue   = RGB_BLUE(crBack) << 8;
		vert [1] .Alpha  = 0x0000;

		gRect.UpperLeft  = 0;
		gRect.LowerRight = 1;
	}

	GradientFill(hdc, vert, 2, &gRect, 1, GRADIENT_FILL_RECT_V);
}

void CCIF::Draw2GradLayerRect(HDC hdc, const CRect &rect, COLORREF crFore1, COLORREF crBack1, COLORREF crFore2, COLORREF crBack2, UINT nScale)
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

	GradientFillEx(hdc, rect.left, rect.top, rect.right, m, crFore1, crBack1, false);
	GradientFillEx(hdc, rect.left, m, rect.right, rect.bottom, crFore2, crBack2, false);
}
void CCIF::DrawRound(HDC hdc, const RECT& rect, int iStartPos, int iRound)
{
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

void CCIF::RemoveIcon(UINT nID, BOOL bGrayscale)
{
	
	if(bGrayscale)
	{
		if(m_GrayImageList.m_hImageList != NULL)
		{
			m_GrayImageList.Remove(nID);
			m_pGrayImageMap.RemoveKey((LPVOID)nID);
		}
	}
	else
	{
		if(m_ImageList.m_hImageList != NULL)
		{
			m_ImageList.Remove(nID);
			m_pImageMap.RemoveKey((LPVOID)nID);
		}
	}
}
