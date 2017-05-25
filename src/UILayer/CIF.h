/*
 * $Id: CIF.h 4483 2008-01-02 09:19:06Z soarchin $
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
/****************************************************************************

    文件名: CIF.h

      说明: CIF的意思为:Color、Icon、Font三个单词的缩写,其目的是希望能够统一
	        管理程序中要用到的颜色、图标和字体
      
  主要函数:

      历史:

  当前版本: Demo
  
  取代版本:

																2006.1.27
															  Thilon版权所有
****************************************************************************/

#pragma once

#define RGB_RED(rgb)	((BYTE)((rgb >> 0) & 0xff))
#define RGB_GREEN(rgb)	((BYTE)((rgb >> 8) & 0xff))
#define RGB_BLUE(rgb)	((BYTE)((rgb >> 16) & 0xff))

typedef struct
{
	WORD wVersion;
	WORD wWidth;
	WORD wHeight;
	WORD wItemCount;
	WORD* items() { return (WORD*)(this+1); }
} TOOLBAR_RES;

class CCIF
{

public:
	CCIF(void);
public:
	~CCIF(void);

//////////////////////////////////////////
//Color
//////////////////////////////////////////
public:
	BOOL		m_bCustom;
	COLORREF	m_crWindow;	//窗体颜色
	COLORREF	m_crMidtone;
	COLORREF	m_crHighlight;
	COLORREF	m_crText;
	COLORREF	m_crHiText;
	COLORREF	m_crBackNormal;
	COLORREF	m_crBackSel;
	COLORREF	m_crBackCheck;
	COLORREF	m_crBackCheckSel;
	COLORREF	m_crMargin;
	COLORREF	m_crBorder;
	COLORREF	m_crShadow;
	COLORREF	m_crCmdText;
	COLORREF	m_crCmdTextSel;
	COLORREF	m_crDisabled;
	COLORREF	m_crNoFocusLine;

public:
	void					InitColor();
		   COLORREF			GetDialogBackColor();
	static COLORREF			CalculateColor(COLORREF crFore, COLORREF crBack, int nAlpha);

//////////////////////////////////////////
//Icon
//////////////////////////////////////////
public:
	CMapStringToPtr			m_pNameMap;
	CMapPtrToPtr			m_pImageMap;			//指向图片的指针
	CImageList				m_ImageList;			//存放菜单图标

	CMapPtrToPtr			m_pGrayImageMap;		//指向灰度图片的指针
	CImageList				m_GrayImageList;		//存放灰度图片

protected:
	CSize					m_czBuffer;
	CDC						m_dcBuffer;
	CBitmap					m_bmBuffer;
	HBITMAP					m_bmOldBuffer;

public:
	BOOL					DrawWatermark(CDC* pDC, CRect* pRect, CBitmap* pMark, int nOffX = 0, int nOffY = 0);
	BOOL					ConfirmImageList();
	BOOL					ConfirmGrayImageList();

	void					NameCommand(UINT nID, LPCTSTR pszName);
	void					Clear();
	void					CopyIcon(UINT nFromID, UINT nToID);
	void					AddIcon(UINT nID, HICON hIcon, BOOL bGrayscale = FALSE);

	UINT					NameToID(LPCTSTR pszName);
	BOOL					AddImagesFromToolbar(UINT nIDToolBar, COLORREF crBack = RGB(0,255,0));
	CDC*					GetBuffer(CDC& dcScreen, const CSize& szItem);
	
	int						ImageForID(UINT nID);
	int						GrayImageForID(UINT nID);
	HICON					ExtractIcon(UINT nID);
	static HICON			CreateGrayscaleIcon(HICON hIcon);

//////////////////////////////////////////
//Icon
//////////////////////////////////////////
public:
		CFont		m_fntUP;
		CFont		m_fntBold;
		CFont		m_fntOver;
		CFont		m_fntDown;
public:
	void CreateFonts(LPCTSTR pszFace = NULL, int nSize = 0);
	void GradientFillEx(HDC hdc, long x1, long y1, long x2, long y2, COLORREF crFore, COLORREF crBack, bool bHorz = true);

	void Draw2GradLayerRect(HDC hdc, const CRect &rect, COLORREF crFore1, COLORREF crBack1, COLORREF crFore2, COLORREF crBack2, UINT nScale);
	void DrawRound(HDC hdc, const RECT& rect, int iStartPos = 0, int iRound = 3);
	void RemoveIcon(UINT nID, BOOL bGrayscale);

};

extern CCIF cif;
