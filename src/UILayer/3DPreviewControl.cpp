/*
 * $Id: 3DPreviewControl.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
#include "stdafx.h"
#include "eMule.h"
#include "3DPreviewControl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CBarShader C3DPreviewControl::s_preview(16,32); 

// C3DPreviewControl
IMPLEMENT_DYNAMIC(C3DPreviewControl, CStatic)

BEGIN_MESSAGE_MAP(C3DPreviewControl, CStatic)
	ON_WM_PAINT()
END_MESSAGE_MAP()

C3DPreviewControl::C3DPreviewControl()
: m_iSliderPos(0) // use flat 
{
}

C3DPreviewControl::~C3DPreviewControl()
{
}

// Sets "slider" position for type of preview


void C3DPreviewControl::SetSliderPos(int iPos)
{
	if ( iPos <= 5 && iPos >= -5)
	{
		m_iSliderPos	=	iPos;
	}
	if ( GetSafeHwnd() )
	{
		Invalidate();
		UpdateWindow();
	}
}

void C3DPreviewControl::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	RECT outline_rec;
	outline_rec.top=0;
	outline_rec.bottom=18;
	outline_rec.left=0;
	outline_rec.right=34;
	CBrush gdiBrush(RGB(104,104,104));
	CBrush* pOldBrush = dc.SelectObject(&gdiBrush);	//eklmn: select a new brush
	dc.FrameRect(&outline_rec, &gdiBrush);
	dc.SelectObject(pOldBrush);						//eklmn: recover an old brush
	s_preview.SetFileSize((uint64)32);
	s_preview.Fill(RGB(192,192,255)); 
	s_preview.DrawPreview(&dc, 1, 1, m_iSliderPos); 
}
