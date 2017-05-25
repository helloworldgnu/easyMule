/*
 * $Id: GapSpace.h 4483 2008-01-02 09:19:06Z soarchin $
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

#include <AfxTempl.h>

template <class scale = int>
class CGapSpace
{
public:
	typedef scale SCALE;
	typedef struct _Block
	{
		SCALE		start;
		SCALE		end;
	} BLOCK;

public:
	CGapSpace(void);
	CGapSpace(SCALE entireSpaceSize);
	CGapSpace(SCALE start, SCALE end);
	~CGapSpace(void);

public:


	void	AddGap(SCALE start, SCALE end);
	void	AddGap(const BLOCK &gap){AddGap(gap.start, gap.end);}
	void	FillGap(SCALE start, SCALE end);
	void	FillGap(const BLOCK &gap){FillGap(gap.start, gap.end);}
	void	ClearAllGaps(void);

	BOOL	IsNoGap(void);
	BOOL	IsFullGap(void);

	SCALE	GetSpaceStart(void){return m_entirSpace.start;}
	SCALE	GetSpaceEnd(void){return m_entirSpace.end;}
	SCALE	GetSpaceSize(void){return (m_entirSpace.end - m_entirSpace.start + 1);}

	POSITION		GetFirstGapPosition(void){return m_gaplist.GetHeadPosition();}
	const BLOCK*	GetGapAt(POSITION pos){return m_gaplist.GetAt(pos);}
	const BLOCK*	GetGapNext(POSITION &pos){return m_gaplist.GetNext(pos);}

protected:
	BLOCK								m_entirSpace;
	CTypedPtrList<CPtrList, BLOCK*>		m_gaplist;
};

template <class scale>
CGapSpace<scale>::CGapSpace(void)
{
	m_entirSpace.start = 0;
	m_entirSpace.end = 0;
}

template <class scale>
CGapSpace<scale>::CGapSpace(SCALE entireSpaceSize)
{
	m_entirSpace.start = 0;
	m_entirSpace.end = entireSpaceSize - 1;

	ASSERT(m_entirSpace.end >= m_entirSpace.start);
	if (m_entirSpace.end < m_entirSpace.start)
		m_entirSpace.end = m_entirSpace.start;

}

template <class scale>
CGapSpace<scale>::CGapSpace(SCALE start, SCALE end)
{
	m_entirSpace.start = start;
	m_entirSpace.end = end;

	ASSERT(m_entirSpace.end >= m_entirSpace.start);
	if (m_entirSpace.end < m_entirSpace.start)
		m_entirSpace.end = m_entirSpace.start;
}

template <class scale>
CGapSpace<scale>::~CGapSpace(void)
{
	ClearAllGaps();
}

template <class scale>
void CGapSpace<scale>::AddGap(SCALE start, SCALE end)
{
	ASSERT( start <= end );
	if (start > end)
		return;
	if (end < m_entirSpace.start)
		return;
	if (start > m_entirSpace.end)
		return;

	if (start < m_entirSpace.start)
		start = m_entirSpace.start;
	if (end > m_entirSpace.end)
		end = m_entirSpace.end;

	POSITION	posCur = NULL;
	POSITION	posNext = NULL;
	POSITION	posOwner = NULL;
	BLOCK		*pCurGap = NULL;
	BLOCK		*pOwnerGap = NULL;

	posOwner = NULL;
	posCur = NULL;
	posNext = m_gaplist.GetHeadPosition();
	while (NULL != posNext)
	{
		posCur = posNext;
		pCurGap = m_gaplist.GetNext(posNext);

		if (NULL != pCurGap)
		{
			if ( (start >= pCurGap->start) && (end <= pCurGap->end) )	// 包含
			{
				return;
			}
			else if ( end < pCurGap->start - 1)						//左不相连
			{
				if (NULL == posOwner)
				{
					BLOCK *pNewGap = new BLOCK;
					pNewGap->start = start;
					pNewGap->end = end;
					m_gaplist.InsertBefore(posCur, pNewGap);
				}
				
				return;
			}
			else if ( start > pCurGap->end + 1)						//右不相连
			{
				//do nothing, continue to iterate.
			}
			else												//相交或相连
			{
				if (NULL == posOwner)
				{
					posOwner = posCur;
					pOwnerGap = pCurGap;
					pOwnerGap->start = min(start, pCurGap->start);
					pOwnerGap->end = max(end, pCurGap->end);
					start = pOwnerGap->start;
					end = pOwnerGap->end;
				}
				else
				{
					pOwnerGap->start = min(start, pCurGap->start);
					pOwnerGap->end = max(end, pCurGap->end);
					start = pOwnerGap->start;
					end = pOwnerGap->end;

					m_gaplist.RemoveAt(posCur);
					delete pCurGap;
					pCurGap = NULL;
				}

			}


		}

	}

	if (NULL == posOwner)
	{
		BLOCK *pNewGap = new BLOCK;
		pNewGap->start = start;
		pNewGap->end = end;
		m_gaplist.AddTail(pNewGap);
	}
}



template <class scale>
void CGapSpace<scale>::FillGap(SCALE start, SCALE end)
{
	ASSERT( start <= end );
	if (start > end)
		return;
	if (end < m_entirSpace.start)
		return;
	if (start > m_entirSpace.end)
		return;
	
	if (start < m_entirSpace.start)
		start = m_entirSpace.start;
	if (end > m_entirSpace.end)
		end = m_entirSpace.end;


	POSITION	posCur = NULL;
	POSITION	posNext = NULL;
	BLOCK		*pCurGap = NULL;

	posCur = NULL;
	posNext = m_gaplist.GetHeadPosition();
	while (NULL != posNext)
	{
		posCur = posNext;
		pCurGap = m_gaplist.GetNext(posNext);
		
		if (NULL != pCurGap)
		{
			if ( (pCurGap->start >= start) && (pCurGap->end <= end) )		// FillBlock 包含 CurGap
			{
				m_gaplist.RemoveAt(posCur);
				delete pCurGap;
				pCurGap = NULL;
			}
			else if ( (pCurGap->start >= start) && (pCurGap->start <= end) )
			{
				pCurGap->start = end + 1;
			}
			else if ( (pCurGap->end >= start) && (pCurGap->end <= end) )
			{
				pCurGap->end = start - 1;
			}
			else if ( (start > pCurGap->start) && (end < pCurGap->end) )
			{
				BLOCK *pNewGap = new BLOCK;
				pNewGap->start = pCurGap->start;
				pNewGap->end = start - 1;
				m_gaplist.InsertBefore(posCur, pNewGap);

				pCurGap->start = end + 1;
			}

		}
	}
}

template <class scale>
void CGapSpace<scale>::ClearAllGaps(void)
{
	BLOCK		*pGap = NULL;

	while (!m_gaplist.IsEmpty())
	{
		pGap = m_gaplist.RemoveHead();
		if (NULL != pGap)
		{
			delete pGap;
			pGap = NULL;
		}
	}
}

template <class scale>
BOOL CGapSpace<scale>::IsNoGap(void)
{
	return (m_gaplist.IsEmpty());
}

template <class scale>
BOOL CGapSpace<scale>::IsFullGap(void)
{
	if (m_gaplist.IsEmpty())
		return FALSE;


	BLOCK	*pGap = m_gaplist.GetHead();
	if (NULL != pGap
		&& pGap->start <= m_entirSpace.start
		&& pGap->end >= m_entirSpace.end)
	{
		return TRUE;
	}

	return FALSE;
}
