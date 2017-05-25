/*
 * $Id: FaceManager.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
#include ".\facemanager.h"
#include "resource.h"

#define SAFE_SET_FACE(index, face) if(m_ButtonFaces.size() <=(index)) m_ButtonFaces.resize((index)+1); \
	m_ButtonFaces[(index)] = (face);

CFaceManager::CFaceManager(void)
{
}

CFaceManager::~CFaceManager(void)
{
	ClearAllRes();
}

void CFaceManager::ClearAllRes()
{
	std::vector<BUTTONFACE *>::iterator it=m_ButtonFaces.begin();
	for(; it!=m_ButtonFaces.end(); ++it)
	{
		BUTTONFACE * p=*it;
		if(p->left) DeleteObject(p->left);
		if(p->mid) DeleteObject(p->mid);
		if(p->right) DeleteObject(p->right);

		if(p->left!=p->left_a && p->left_a) DeleteObject(p->left_a);
		if(p->mid!=p->mid_a && p->mid_a) DeleteObject(p->mid_a);
		if(p->right!=p->right_a && p->right_a) DeleteObject(p->right_a);

		if(p->left!=p->left_h && p->left_h) DeleteObject(p->left_h);
		if(p->mid!=p->mid_h && p->mid_h) DeleteObject(p->mid_h);
		if(p->right!=p->right_h && p->right_h) DeleteObject(p->right_h);

		delete p;
	}

	m_ButtonFaces.clear();

}

void CFaceManager::Init()
{
	BUTTONFACE * pFace;

	pFace = new BUTTONFACE;
	ZeroMemory(pFace, sizeof(BUTTONFACE));
	pFace->left = LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_SHAREFILECOUNT_L));
	pFace->mid = LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_SHAREFILECOUNT_M));
	pFace->right = LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_SHAREFILECOUNT_R));
	SAFE_SET_FACE(FI_SHARE_FILE_COUNT, pFace);



	CDC dc;
	dc.Attach(GetDC(NULL));
	VERIFY(m_DC.CreateCompatibleDC(&dc) );
	ReleaseDC(NULL, dc.m_hDC);
	dc.Detach();

	//	Initialize images	<begin>
	m_arrImages[II_DETAILTAB_N].LoadResource(FindResource(NULL, _T("PNG_DETAILTAB_N"), _T("PNG")), CXIMAGE_FORMAT_PNG);
	m_arrImages[II_DETAILTAB_H].LoadResource(FindResource(NULL, _T("PNG_DETAILTAB_H"), _T("PNG")), CXIMAGE_FORMAT_PNG);
	m_arrImages[II_DETAILTAB_A].LoadResource(FindResource(NULL, _T("PNG_DETAILTAB_A"), _T("PNG")), CXIMAGE_FORMAT_PNG);

	m_arrImages[II_SEARCHBTN_N].LoadResource(FindResource(NULL, _T("PNG_SEARCHBTN_N"), _T("PNG")), CXIMAGE_FORMAT_PNG);
	m_arrImages[II_SEARCHBTN_H].LoadResource(FindResource(NULL, _T("PNG_SEARCHBTN_H"), _T("PNG")), CXIMAGE_FORMAT_PNG);
	m_arrImages[II_SEARCHBTN_P].LoadResource(FindResource(NULL, _T("PNG_SEARCHBTN_P"), _T("PNG")), CXIMAGE_FORMAT_PNG);

	m_arrImages[II_MAINTAB_BK].LoadResource(FindResource(NULL, _T("PNG_MAINTABBK"), _T("PNG")), CXIMAGE_FORMAT_PNG);
	m_arrImages[II_PAGETAB_BK].LoadResource(FindResource(NULL, _T("PNG_PAGETABBK"), _T("PNG")), CXIMAGE_FORMAT_PNG);

	m_arrImages[II_VERYCDSEARCH].LoadResource(FindResource(NULL, _T("PNG_SEARCHBAR_ICO"), _T("PNG")), CXIMAGE_FORMAT_PNG);
	
	m_arrImages[II_MAINTABMORE_N].LoadResource(FindResource(NULL, _T("PNG_MAINTABMORE_N"), _T("PNG")), CXIMAGE_FORMAT_PNG);
	m_arrImages[II_MAINTABMORE_H].LoadResource(FindResource(NULL, _T("PNG_MAINTABMORE_H"), _T("PNG")), CXIMAGE_FORMAT_PNG);
	m_arrImages[II_MAINTABMORE_P].LoadResource(FindResource(NULL, _T("PNG_MAINTABMORE_P"), _T("PNG")), CXIMAGE_FORMAT_PNG);
	
	m_arrImages[II_SPLITER_H].LoadResource(FindResource(NULL, _T("PNG_SPLITER_H"), _T("PNG")), CXIMAGE_FORMAT_PNG);
	m_arrImages[II_SPLITER_V].LoadResource(FindResource(NULL, _T("PNG_SPLITER_V"), _T("PNG")), CXIMAGE_FORMAT_PNG);
	

	//	Initialize images	<end>

	
	//	Initialize image bars	<begin>
	m_arrImageBars[IBI_MAINBTN_A].left.LoadResource(FindResource(NULL, _T("PNG_MAINBTN_A_L"), _T("PNG")), CXIMAGE_FORMAT_PNG);
	m_arrImageBars[IBI_MAINBTN_A].mid.LoadResource(FindResource(NULL, _T("PNG_MAINBTN_A_M"), _T("PNG")), CXIMAGE_FORMAT_PNG);
	m_arrImageBars[IBI_MAINBTN_A].right.LoadResource(FindResource(NULL, _T("PNG_MAINBTN_A_R"), _T("PNG")), CXIMAGE_FORMAT_PNG);
	
	m_arrImageBars[IBI_MAINBTN_N].left.LoadResource(FindResource(NULL, _T("PNG_MAINBTN_N_L"), _T("PNG")), CXIMAGE_FORMAT_PNG);
	m_arrImageBars[IBI_MAINBTN_N].mid.LoadResource(FindResource(NULL, _T("PNG_MAINBTN_N_M"), _T("PNG")), CXIMAGE_FORMAT_PNG);
	m_arrImageBars[IBI_MAINBTN_N].right.LoadResource(FindResource(NULL, _T("PNG_MAINBTN_N_R"), _T("PNG")), CXIMAGE_FORMAT_PNG);

	m_arrImageBars[IBI_MAINBTN_H].left.LoadResource(FindResource(NULL, _T("PNG_MAINBTN_H_L"), _T("PNG")), CXIMAGE_FORMAT_PNG);
	m_arrImageBars[IBI_MAINBTN_H].mid.LoadResource(FindResource(NULL, _T("PNG_MAINBTN_H_M"), _T("PNG")), CXIMAGE_FORMAT_PNG);
	m_arrImageBars[IBI_MAINBTN_H].right.LoadResource(FindResource(NULL, _T("PNG_MAINBTN_H_R"), _T("PNG")), CXIMAGE_FORMAT_PNG);

	m_arrImageBars[IBI_SEARCHBAR_EDIT].left.LoadResource(FindResource(NULL, _T("PNG_SEARCHBAREDIT_L"), _T("PNG")), CXIMAGE_FORMAT_PNG);
	m_arrImageBars[IBI_SEARCHBAR_EDIT].mid.LoadResource(FindResource(NULL, _T("PNG_SEARCHBAREDIT_M"), _T("PNG")), CXIMAGE_FORMAT_PNG);
	m_arrImageBars[IBI_SEARCHBAR_EDIT].right.LoadResource(FindResource(NULL, _T("PNG_SEARCHBAREDIT_R"), _T("PNG")), CXIMAGE_FORMAT_PNG);

	m_arrImageBars[IBI_PAGETAB_N].left.LoadResource(FindResource(NULL, _T("PNG_PAGETAB_N_L"), _T("PNG")), CXIMAGE_FORMAT_PNG);
	m_arrImageBars[IBI_PAGETAB_N].mid.LoadResource(FindResource(NULL, _T("PNG_PAGETAB_N_M"), _T("PNG")), CXIMAGE_FORMAT_PNG);
	m_arrImageBars[IBI_PAGETAB_N].right.LoadResource(FindResource(NULL, _T("PNG_PAGETAB_N_R"), _T("PNG")), CXIMAGE_FORMAT_PNG);
	
	m_arrImageBars[IBI_PAGETAB_A].left.LoadResource(FindResource(NULL, _T("PNG_PAGETAB_A_L"), _T("PNG")), CXIMAGE_FORMAT_PNG);
	m_arrImageBars[IBI_PAGETAB_A].mid.LoadResource(FindResource(NULL, _T("PNG_PAGETAB_A_M"), _T("PNG")), CXIMAGE_FORMAT_PNG);
	m_arrImageBars[IBI_PAGETAB_A].right.LoadResource(FindResource(NULL, _T("PNG_PAGETAB_A_R"), _T("PNG")), CXIMAGE_FORMAT_PNG);

	m_arrImageBars[IBI_PAGETAB_H].left.LoadResource(FindResource(NULL, _T("PNG_PAGETAB_H_L"), _T("PNG")), CXIMAGE_FORMAT_PNG);
	m_arrImageBars[IBI_PAGETAB_H].mid.LoadResource(FindResource(NULL, _T("PNG_PAGETAB_H_M"), _T("PNG")), CXIMAGE_FORMAT_PNG);
	m_arrImageBars[IBI_PAGETAB_H].right.LoadResource(FindResource(NULL, _T("PNG_PAGETAB_H_R"), _T("PNG")), CXIMAGE_FORMAT_PNG);
	//	Initialize image bars	<end>
}

CFaceManager * CFaceManager::GetInstance()
{
	static std::auto_ptr<CFaceManager> s;
	if(s.get()==NULL)
	{
		s.reset(new CFaceManager);
	}

	return s.get();
}

void CFaceManager::DrawFace(int nFaceIndex, HDC hDC, int nState, const CRect & rect, bool bVertDraw)
{
	ASSERT(nFaceIndex<(int)m_ButtonFaces.size());
	ASSERT(m_DC.m_hDC);

	if(nFaceIndex>=(int)m_ButtonFaces.size()) return;

	BUTTONFACE * pFace = m_ButtonFaces[nFaceIndex];
	if(! pFace)
	{
		ASSERT(FALSE);
		return;
	}

	HBITMAP bmLeft=NULL, bmRight=NULL, bmMid=NULL;
	switch(nState)
	{
	case FS_ACTIVE:
		bmLeft = pFace->left_a;
		bmMid = pFace->mid_a;
		bmRight = pFace->right_a;
		break;
	case FS_NORMAL:
		bmLeft = pFace->left;
		bmMid = pFace->mid;
		bmRight = pFace->right;
		break;
	case FS_HOVER:
		bmLeft = pFace->left_h;
		bmMid = pFace->mid_h;
		bmRight = pFace->right_h;
		break;
	}

	if((bmLeft==NULL || bmRight==NULL) && bmMid==NULL)
	{
		bmLeft = pFace->left;
		bmMid = pFace->mid;
		bmRight = pFace->right;
	}

	ASSERT(bmMid);
	if(! bmMid) return;

	//BITMAP bminfLeft, bminfRight, bminfMid;
	// VC-linhai[2007-08-14]:初始化变量
	// 
	BITMAP bminfLeft = {0};		//warning C4701: 局部变量“bminfLeft”可能尚未初始化即被使用
	BITMAP bminfMid = {0};		//warning C4701: 局部变量“bminfMid”可能尚未初始化即被使用
	BITMAP bminfRight = {0};	//warning C4701: 局部变量“bminfRight”可能尚未初始化即被使用

	if(bmLeft) 
		::GetObject(bmLeft, sizeof(BITMAP), &bminfLeft);

	::GetObject(bmMid, sizeof(BITMAP), &bminfMid);

	if(bmRight) 
		::GetObject(bmRight, sizeof(BITMAP), &bminfRight);

	int height = min(rect.Height(), bminfMid.bmHeight);
	int width = min(rect.Width(), bminfMid.bmWidth);

	HBITMAP hOldBmp;
	if(bmMid && bmLeft==NULL && bmRight==NULL)
	{
		hOldBmp=(HBITMAP)::SelectObject(m_DC.m_hDC, bmMid);

		::SelectObject(m_DC.m_hDC, bmMid);

		if(bVertDraw)
		{
			StretchBlt(hDC, rect.left, rect.top, rect.Width(), rect.Height(), m_DC.m_hDC,
				0, 0, bminfMid.bmWidth, bminfMid.bmHeight, SRCCOPY);
		}		
		else
		{
			StretchBlt(hDC, rect.left, rect.top, rect.Width(), rect.Height(), m_DC.m_hDC,
				0, 0, bminfMid.bmWidth, bminfMid.bmHeight, SRCCOPY);
		}
		::SelectObject(m_DC.m_hDC, hOldBmp);
	}
	else
	{
		hOldBmp=(HBITMAP)::SelectObject(m_DC.m_hDC, bmLeft);
		if(bVertDraw)
		{
			BitBlt(hDC, rect.left, rect.top, width, bminfLeft.bmHeight,
				m_DC.m_hDC, 0, 0, SRCCOPY);
		}
		else
		{
			BitBlt(hDC, rect.left, rect.top, bminfLeft.bmWidth, height, m_DC.m_hDC, 0, 0, SRCCOPY);
		}

		::SelectObject(m_DC.m_hDC, bmMid);

		if(bVertDraw)
		{
			int nMidHeight=rect.Height() - bminfLeft.bmHeight- bminfRight.bmHeight;
			StretchBlt(hDC, rect.left, rect.top+bminfLeft.bmHeight, width, nMidHeight, m_DC.m_hDC,
				0, 0, bminfMid.bmWidth, bminfMid.bmHeight, SRCCOPY);
		}
		else
		{
			int nMidWidth=rect.Width() - bminfLeft.bmWidth - bminfRight.bmWidth;
			StretchBlt(hDC, rect.left+bminfLeft.bmWidth, rect.top, nMidWidth, height, m_DC.m_hDC,
				0, 0, bminfMid.bmWidth, bminfMid.bmHeight, SRCCOPY);
		}

		::SelectObject(m_DC.m_hDC, bmRight);
		if(bVertDraw)
		{
			int nBtmPos=rect.Height() - bminfRight.bmHeight;
			BitBlt(hDC, rect.left, rect.top + nBtmPos, width, bminfRight.bmHeight, m_DC.m_hDC, 0, 0, SRCCOPY);
		}
		else
		{
			int nRightPos=rect.Width() - bminfRight.bmWidth;
			BitBlt(hDC, rect.left+nRightPos, rect.top, bminfRight.bmWidth, height, m_DC.m_hDC, 0, 0, SRCCOPY);
		}

		::SelectObject(m_DC.m_hDC, hOldBmp);
	}
}

void CFaceManager::DrawImage(int nImageIndex, HDC hDC, const CRect & rect, int iMode)
{
	ASSERT(nImageIndex < II_MAX);
	if (nImageIndex >= II_MAX)
		return;

	switch(iMode) 
	{
	case 0:
		m_arrImages[nImageIndex].Draw(hDC, rect);
		break;
	case 1:
		{
			CRect	rtTile = rect;	
			m_arrImages[nImageIndex].Tile(hDC, &rtTile);
		}
		break;
	case 2:
		m_arrImages[nImageIndex].Stretch(hDC, rect);
		break;
	default:
		break;
	}
}

BOOL CFaceManager::GetImageSize(int nImageIndex, CSize &size)
{
	ASSERT(nImageIndex < II_MAX);
	if (nImageIndex >= II_MAX)
		return FALSE;
	
	size.cx = m_arrImages[nImageIndex].GetWidth();
	size.cy = m_arrImages[nImageIndex].GetHeight();

	return TRUE;
}

void CFaceManager::DrawImageBar(int nImageBarIndex, HDC hDC, const CRect & rect, bool bVertDraw)
{
	ASSERT(nImageBarIndex < IBI_MAX);
	if( nImageBarIndex >= IBI_MAX) return;

	CSize	sizeLeft;
	CSize	sizeRight;
	int		iMidLength;

	sizeLeft.cx = m_arrImageBars[nImageBarIndex].left.GetWidth();
	sizeLeft.cy = m_arrImageBars[nImageBarIndex].left.GetHeight();
	sizeRight.cx = m_arrImageBars[nImageBarIndex].right.GetWidth();
	sizeRight.cy = m_arrImageBars[nImageBarIndex].right.GetHeight();

	if (bVertDraw)
	{
		m_arrImageBars[nImageBarIndex].left.Draw(hDC, rect.left, rect.top, rect.Width(), -1);
		m_arrImageBars[nImageBarIndex].right.Draw(hDC, rect.left, rect.bottom - sizeRight.cy, rect.Width(), -1);

		iMidLength = rect.Height() - sizeLeft.cy - sizeRight.cy;
		if (iMidLength > 0)
			m_arrImageBars[nImageBarIndex].mid.Draw(hDC, rect.left, rect.top + sizeLeft.cy, rect.Width(), iMidLength);
	}
	else
	{
		m_arrImageBars[nImageBarIndex].left.Draw(hDC, rect.left, rect.top, -1, rect.Height());
		m_arrImageBars[nImageBarIndex].right.Draw(hDC, rect.right - sizeRight.cx, rect.top, -1, rect.Height());

		iMidLength = rect.Width() - sizeLeft.cx - sizeRight.cx;
		if (iMidLength > 0)
			m_arrImageBars[nImageBarIndex].mid.Draw(hDC, rect.left + sizeLeft.cx, rect.top, iMidLength, rect.Height());

	}
}
