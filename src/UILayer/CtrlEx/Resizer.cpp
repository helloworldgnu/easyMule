/*
 * $Id: Resizer.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
#include ".\resizer.h"
#include "Resizee.h"

namespace TabWnd{

CResizer::CResizer(void)
{
	m_rtBarMarginLogic.SetRect(0, 0, 0, 0);
	m_iFixedTabsTotalLength	= 0;
	m_iFixedTabsCount		= 0;
}

CResizer::~CResizer(void)
{
	m_resizees.RemoveAll();
}

void CResizer::ResizeAll(const CRect &rtBound, ETabBarPos eBarPos)
{
	if (m_resizees.GetCount() <= 0)
		return;
	
	if (rtBound.IsRectEmpty())
		return;


	int iTotalLength;

	CRect			rtBoundLogic = rtBound;
	TabWnd::Real2LogicSolid(rtBoundLogic, eBarPos);

	iTotalLength = rtBoundLogic.Width() - m_rtBarMarginLogic.left - m_rtBarMarginLogic.right;


	POSITION	pos;
	CResizee	*pResizee;

	int iTotalDesireLength = 0;
	int iUnfixedTotalLength = 0;

	pos = m_resizees.GetHeadPosition();
	while (NULL != pos)
	{
		pResizee = m_resizees.GetNext(pos);

		if (NULL != pResizee)
		{
			iTotalDesireLength += pResizee->GetDesireLength();
			if (!pResizee->IsFixedLength())
				iUnfixedTotalLength += pResizee->GetDesireLength();
		}
	}

	int			iBaseLeft;
	int			iBaseRight;
	int			iLength;
	int			iFillPlus;
	float		fShrinkRate;
	CRect		rtItem;

	iBaseLeft = rtBoundLogic.left + m_rtBarMarginLogic.left;
	iBaseRight = rtBoundLogic.right + m_rtBarMarginLogic.right;


	if (iTotalDesireLength > iTotalLength)
	{
		if (iTotalLength - m_iFixedTabsTotalLength > 0)
			fShrinkRate = (float)(iTotalLength - m_iFixedTabsTotalLength) / iUnfixedTotalLength ;
		else
			fShrinkRate = 0.0f;

		iFillPlus = 0;
	}
	else
	{
		fShrinkRate = 1.0f;
		iFillPlus = iTotalLength - iTotalDesireLength;
	}

	pos = m_resizees.GetHeadPosition();
	while (NULL != pos)
	{
		pResizee = m_resizees.GetNext(pos);

		if (NULL != pResizee)
		{
			//	calculate length	<begin>
			iLength = pResizee->GetDesireLength();
			if (!pResizee->IsFixedLength())
				iLength = (int)(iLength * fShrinkRate);
			if (pResizee->IsFill())
				iLength += iFillPlus;
			//	calculate length	<end>

			if (pResizee->IsStickTail())
			{
				rtItem.right = iBaseRight;
				rtItem.left = rtItem.right - iLength + 1;
				iBaseRight = rtItem.left - 1;
			}
			else
			{
				rtItem.left = iBaseLeft;
				rtItem.right = rtItem.left + iLength - 1;
				iBaseLeft = rtItem.right + 1;
			}

			rtItem.top = rtBoundLogic.top + m_rtBarMarginLogic.top;
			rtItem.bottom = rtBoundLogic.bottom - m_rtBarMarginLogic.bottom;

			TabWnd::LogicSolid2Real(rtItem, eBarPos);
			pResizee->SetRect(rtItem);
		}
	}


	//if (m_resizees.GetCount() > 0)
	//	ResizeFrom(rtBound, m_resizees.GetHead(), eBarPos);
}

void CResizer::ResizeFrom(const CRect &rtBound, CResizee *pResizeeStart, ETabBarPos eBarPos)
{
	if (rtBound.IsRectEmpty())
		return;

	CRect			rtBoundLogic = rtBound;
	TabWnd::Real2LogicSolid(rtBoundLogic, eBarPos);

	int iTotalLength;
	int	iUnfixedItemCount;
	int iEachItemLength;

	iTotalLength = rtBoundLogic.Width() - m_rtBarMarginLogic.left - m_rtBarMarginLogic.right;
	iUnfixedItemCount = (int)m_resizees.GetCount() - m_iFixedTabsCount;
	if (0 == iUnfixedItemCount)
		iEachItemLength = 0;
	else
		iEachItemLength = (iTotalLength - m_iFixedTabsTotalLength) / iUnfixedItemCount;


	int			iBaseLeft;
	int			iBaseRight;
	int			iLength;
	CResizee	*pResizee = NULL;
	POSITION	pos = NULL;
	CRect		rtItem;


	//	from head 
	iBaseLeft = rtBoundLogic.left + m_rtBarMarginLogic.left;
	iBaseRight = rtBoundLogic.right + m_rtBarMarginLogic.right;

	pos = pResizeeStart->m_posInResizer;
	while (NULL != pos)
	{
		pResizee = m_resizees.GetNext(pos);
		if (NULL != pResizee)
		{
			//	calculate length	<begin>
			if (pResizee->IsFixedLength())
				iLength = pResizee->GetDesireLength();
			else
			{
				if (iEachItemLength > pResizee->GetDesireLength())
					iLength = pResizee->GetDesireLength();
				else
					iLength = iEachItemLength;
			}
			//	calculate length	<end>


			if (pResizee->IsStickTail())
			{
				rtItem.right = iBaseRight;
				rtItem.left = rtItem.right - iLength + 1;
				iBaseRight = rtItem.left - 1;
			}
			else
			{
				rtItem.left = iBaseLeft;
				rtItem.right = rtItem.left + iLength - 1;
				iBaseLeft = rtItem.right + 1;
			}

			rtItem.top = rtBoundLogic.top + m_rtBarMarginLogic.top;
			rtItem.bottom = rtBoundLogic.bottom - m_rtBarMarginLogic.bottom;

			TabWnd::LogicSolid2Real(rtItem, eBarPos);
			pResizee->SetRect(rtItem);
		}
	}
}

//void CResizer::SetRectByPrev(const CRect &rtBound, ETabBarPos eBarPos, CResizee *pResizee)
//{
//	if (NULL == pResizee)
//		return;
//	CRect	rect;
//	if (CalRectByPrev(rtBound, eBarPos, pResizee, rect))
//		pResizee->SetRect(rect);
//}
//
//BOOL CResizer::CalRectByPrev(const CRect &rtBound, ETabBarPos eBarPos, CResizee *pResizee, CRect &rtItem)
//{
//	if (NULL == pResizee)
//		return FALSE;
//	if (NULL == pResizee->m_posInResizer)
//		return FALSE;
//
//	CRect		rtBarLogic = rtBound;
//	TabWnd::Real2LogicSolid(rtBarLogic, eBarPos);
//
//	int iItemLeft = rtBarLogic.left + m_rtBarMarginLogic.left;
//
//	POSITION	posPrev;
//	CResizee	*pPrevResizee;
//
//	posPrev = pResizee->m_posInResizer;
//	m_resizees.GetPrev(posPrev);
//	if (NULL != posPrev)
//	{
//		pPrevResizee = m_resizees.GetAt(posPrev);
//		if (NULL != pPrevResizee)
//		{
//			CRect	rtItemInBarLogic;
//			rtItemInBarLogic = pPrevResizee->GetRect();
//			TabWnd::Real2LogicSolid(rtItemInBarLogic, eBarPos);
//			iItemLeft = rtItemInBarLogic.right + 1;
//		}
//	}
//
//
//	rtItem.left = iItemLeft;
//	rtItem.right = rtItem.left + pResizee->GetDesireLength();
//	rtItem.top = rtBarLogic.top + m_rtBarMarginLogic.top;
//	rtItem.bottom = rtBarLogic.bottom - m_rtBarMarginLogic.bottom;
//
//	TabWnd::LogicSolid2Real(rtItem, eBarPos);
//	return TRUE;
//}

void CResizer::SetBarMarginLogic(const CRect &rect)
{
	m_rtBarMarginLogic = rect;
}

void CResizer::MoveResizeeTo(CResizee *pResizee, CResizee *pResizeeAfter, BOOL bAfter)
{
	m_resizees.RemoveAt(pResizee->m_posInResizer);
	
	POSITION	posNew;
	if (bAfter)
		posNew = m_resizees.InsertAfter(pResizeeAfter->m_posInResizer, pResizee);
	else
		posNew = m_resizees.InsertBefore(pResizeeAfter->m_posInResizer, pResizee);

	pResizee->m_posInResizer = posNew;
}
POSITION CResizer::AddResizee(CResizee *pResizee)
{
	if (pResizee->IsFixedLength())
	{
		m_iFixedTabsTotalLength += pResizee->GetDesireLength();
		m_iFixedTabsCount ++;
	}
	
	return 	m_resizees.AddTail(pResizee);
;
}
void CResizer::RemoveResizee(CResizee *pResizee)
{
	m_resizees.RemoveAt(pResizee->m_posInResizer);

	if (pResizee->IsFixedLength())
	{
		m_iFixedTabsTotalLength -= pResizee->GetDesireLength();
		m_iFixedTabsCount--;
	}
}

}//namespace TabWnd{
