/*
 * $Id: Draw.h 4483 2008-01-02 09:19:06Z soarchin $
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
///////////////////////////////////////////////////////////////////////////////
//
// Draw.h : header file
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

///////////////////////////////////////////////////////////////////////////////
typedef DWORD HLSCOLOR;
#define HLS(h,l,s) ((HLSCOLOR)(((BYTE)(h)|((WORD)((BYTE)(l))<<8))|(((DWORD)(BYTE)(s))<<16)))

///////////////////////////////////////////////////////////////////////////////
#define HLS_H(hls) ((BYTE)(hls))
#define HLS_L(hls) ((BYTE)(((WORD)(hls)) >> 8))
#define HLS_S(hls) ((BYTE)((hls)>>16))

///////////////////////////////////////////////////////////////////////////////
HLSCOLOR RGB2HLS (COLORREF rgb);
COLORREF HLS2RGB (HLSCOLOR hls);

///////////////////////////////////////////////////////////////////////////////
// Performs some modifications on the specified color : luminance and saturation
COLORREF HLS_TRANSFORM (COLORREF rgb, int percent_L, int percent_S);

///////////////////////////////////////////////////////////////////////////////
HBITMAP WINAPI GetScreenBitmap (LPCRECT pRect);


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class CBufferDC : public CDC
{
    HDC     m_hDestDC;
	//CBitmap m_bitmap;     // Bitmap in Memory DC
	HBITMAP	m_hBitmap;		// Bitmap in Memory DC		// replace CBitmap with GdiHandle to solve the problem of that CBitmap's Destruction is called later than CBufferDC's Destruction.
    CRect   m_rect;
    HGDIOBJ m_hOldBitmap; // Previous Bitmap

public:
    CBufferDC (HDC hDestDC, const CRect& rcPaint/* = CRect(0,0,0,0)*/);
   ~CBufferDC ();
};


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class CPenDC
{
protected:
    CPen m_pen;
    HDC m_hDC;
    HPEN m_hOldPen;

public:
    CPenDC (HDC hDC, COLORREF crColor = CLR_NONE, int nWidth = 1);
   ~CPenDC ();

    void Color (COLORREF crColor);
    COLORREF Color () const;
};


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class CBrushDC
{
protected:
    CBrush m_brush;
    HDC m_hDC;
    HBRUSH m_hOldBrush;

public:
    CBrushDC (HDC hDC, COLORREF crColor = CLR_NONE);
   ~CBrushDC ();

    void Color (COLORREF crColor);
    COLORREF Color () const;
};


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class CFontDC
{
protected:
    HFONT m_hFont;
    HDC m_hDC;
    HFONT m_hDefFont;
    COLORREF m_crTextOld;

public:
    CFontDC (HDC hDC, LPCTSTR sFaceName, COLORREF crText = CLR_DEFAULT);
    CFontDC (HDC hDC, BYTE nStockFont, COLORREF crText = CLR_DEFAULT);
    CFontDC (HDC hDC, HFONT hFont, COLORREF crText = CLR_DEFAULT);
	CFontDC (HDC hDC, int iSize, COLORREF crText = CLR_DEFAULT);
   ~CFontDC ();

    const CFontDC& operator = (LPCTSTR sFaceName);
    const CFontDC& operator = (BYTE nStockFont);
    const CFontDC& operator = (HFONT hFont);
    const CFontDC& operator = (COLORREF crText);
	const CFontDC& operator = (int size);
    operator LPCTSTR ();
    operator COLORREF ();

	void	SetWeight(LONG lWeight);
};

class CWndFontDC
{
public:
	CWndFontDC(HDC hdc, HWND hWnd);
	~CWndFontDC();
protected:
	HDC		m_hdc;
	HFONT	m_hOldFont;
};
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class CBoldDC
{
protected:
    CFont m_fontBold;
    HDC m_hDC;
    HFONT m_hDefFont;

public:
    CBoldDC (HDC hDC, bool bBold);
   ~CBoldDC ();
};

class CTextDC
{
public:
	CTextDC(HDC hDC, COLORREF clrText, int iBkMode = TRANSPARENT);
	~CTextDC();
	void SetTextColor(COLORREF clrText);

protected:
	HDC			m_hDC;
	COLORREF	m_clrOldText;
	int			m_iOldBkMode;	
};
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// CDrawButton types
//
#define DB_EMPTY_       0x0000 // Empty button
#define DB_UP          0x0001 // Up arrow
#define DB_DOWN        0x0002 // Down arrow
#define DB_LEFT        0x0003 // Left arrow
#define DB_RIGHT       0x0004 // Right arrow
#define DB_UPDOWN      0x0005 // Up and down arrows
#define DB_LEFTRIGHT   0x0006 // Left and right arrow
#define DB_3POINTS     0x0007 // Three points (assistant, more, ...)
#define DB_CROSS       0x0008 // Cross like small close button
// CDrawButton styles
#define DB_ENABLED     0x0000 // Enabled button(s)   [Default]
#define DB_DISABLED    0x0100 // Disabled button(s)
#define DB_BORDER      0x0200 // Buttons have border on left and top
#define DB_WINDOWDC    0x0400 // Positions are in screen coordinates
#define DB_FLAT        0x0800 // Office 2000 look & feel
#define DB_PRESSED     0x1000 // First button is pressed
#define DB_PRESSED2    0x2000 // Second button is pressed (if exists)
#define DB_OVER        0x4000 // Mouse is over button
#define DB_TRANSPARENT 0x8000 // Don't draw background (flat button)

#define DB_DEFAULT DB_EMPTY_

///////////////////////////////////////////////////////////////////////////////
class CDrawButton
{
protected:
    DWORD m_wStyle;
    CRect m_Rect;

public:
    CDrawButton (DWORD wStyle = DB_EMPTY_);
    CDrawButton (DWORD wStyle, LPCRECT pRect);

    virtual void Draw (CDC* pDC, DWORD wStyle = DB_DEFAULT) const;

    DWORD Click (CWnd* pWnd, CPoint pt, UINT nIDRepeat = 0) const;

    void SetRect (LPCRECT pRect);
    bool PtInRect (POINT pt) const;

    void CheckForMouseOver (CWnd* pWnd, CPoint pt);
};

///////////////////////////////////////////////////////////////////////////////
inline CDrawButton::CDrawButton (DWORD wStyle) :
    m_wStyle (wStyle), m_Rect (0, 0, 0, 0)
{
}

///////////////////////////////////////////////////////////////////////////////
inline CDrawButton::CDrawButton (DWORD wStyle, LPCRECT pRect) :
    m_wStyle (wStyle), m_Rect (pRect)
{
}

///////////////////////////////////////////////////////////////////////////////
inline void CDrawButton::SetRect (LPCRECT pRect)
{
    m_Rect = pRect;
}

///////////////////////////////////////////////////////////////////////////////
inline bool CDrawButton::PtInRect (POINT pt) const
{
    return m_Rect.PtInRect (pt) != 0;
}
