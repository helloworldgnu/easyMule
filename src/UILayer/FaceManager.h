/*
 * $Id: FaceManager.h 4483 2008-01-02 09:19:06Z soarchin $
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

#include <vector>
#include "CxImage\xImage.h"

typedef struct tagBUTTONFACE
{
	//  normal state
	HBITMAP left, mid, right;
	//  active state
	HBITMAP left_a, mid_a, right_a;
	//  hover state
	HBITMAP left_h, mid_h, right_h;
}BUTTONFACE;

typedef struct tagImageFace
{
	CxImage		left;
	CxImage		mid;
	CxImage		right;
} IMAGEBAR;

enum EFaceIndex
{
	FI_SHARE_FILE_COUNT,
	// 尽量使用下面的Image，which support png format and so on.

	FI_MAX
};

enum EImageIndex
{
	II_DETAILTAB_N,
	II_DETAILTAB_H,
	II_DETAILTAB_A,
	II_SEARCHBTN_N,
	II_SEARCHBTN_H,
	II_SEARCHBTN_P,
	II_MAINTAB_BK,
	II_PAGETAB_BK,
	II_VERYCDSEARCH,
	II_MAINTABMORE_N,
	II_MAINTABMORE_H,
	II_MAINTABMORE_P,
	II_SPLITER_H,
	II_SPLITER_V,

	II_MAX
};

enum EImageBarIndex
{
	IBI_MAINBTN_A,
	IBI_MAINBTN_N,
	IBI_MAINBTN_H,
	IBI_SEARCHBAR_EDIT,
	IBI_PAGETAB_N,
	IBI_PAGETAB_A,
	IBI_PAGETAB_H,

	IBI_MAX
};

//  face state
#define FS_NORMAL		0
#define FS_ACTIVE		1
#define FS_HOVER		2

class CFaceManager
{
public:
	CFaceManager(void);
	~CFaceManager(void);

	void Init();
	static CFaceManager * GetInstance();

	void DrawFace(int nFaceIndex, HDC hDC, int nState, const CRect & rect, bool bVertDraw = false);
	void DrawImage(int nImageIndex, HDC hDC, const CRect & rect, int iMode = 0); // Mode: 0.Normal, 1.Tile, 2.Stretch
	BOOL GetImageSize(int nImageIndex, CSize &size);
	void DrawImageBar(int nImageBarIndex, HDC hDC, const CRect & rect, bool bVertDraw = false);
private:
	void ClearAllRes();
	std::vector<BUTTONFACE *> m_ButtonFaces;
	CxImage					m_arrImages[II_MAX];
	IMAGEBAR				m_arrImageBars[IBI_MAX];
public:

	CDC m_DC;
};
