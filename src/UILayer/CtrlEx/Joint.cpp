/*
 * $Id: Joint.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
#include ".\joint.h"
#include "Util.h"
namespace TabWnd{

CJoint::CJoint(void)
{
	m_rtInTabWnd.SetRect(0, 0, 0, 0);
	m_ptStart.SetPoint(0, 0);
	m_ptEnd.SetPoint(0, 0);
	m_clrJoint = RGB(0, 0, 0);
	m_clrGap = RGB(0, 0, 0);
}

CJoint::~CJoint(void)
{
}

void CJoint::Draw(CDC *pDC)
{
	CBrush		brush(m_clrJoint);
	pDC->FrameRect(&m_rtInTabWnd, &brush);

	CBrush		brushGap(m_clrGap);
	CRect		rtGap(m_ptStart, m_ptEnd);
	pDC->FrameRect(&rtGap, &brushGap);

	//CPenDC	penDc(pDC->GetSafeHdc(), m_clrGap);
	//pDC->MoveTo(m_ptStart);
	//pDC->LineTo(m_ptEnd);
}

void CJoint::SetGap(CPoint ptStart, CPoint ptEnd)
{
	m_ptStart = ptStart;
	m_ptEnd = ptEnd;
}

void CJoint::SetJointColor(COLORREF clrJoint, COLORREF clrGap)
{
	m_clrJoint = clrJoint;
	m_clrGap = clrGap;
}

}//namespace TabWnd{
