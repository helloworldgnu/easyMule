/*
 * $Id: Resizer.h 4483 2008-01-02 09:19:06Z soarchin $
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
#include "TabWndDef.h"

namespace TabWnd{

class CResizee;
class CResizer
{
public:
	CResizer(void);
	~CResizer(void);

	void	ResizeAll(const CRect &rtBound, ETabBarPos eBarPos);
	void	ResizeFrom(const CRect &rtBound, CResizee *pResizeeStart, ETabBarPos eBarPos);
	//void	SetRectByPrev(const CRect &rtBound, ETabBarPos eBarPos, CResizee *pResizee);
	//BOOL	CalRectByPrev(const CRect &rtBound, ETabBarPos eBarPos, CResizee *pResizee, CRect &rtItem);
	
	const CRect&	GetBarMarginLogic(){return m_rtBarMarginLogic;}
	void			SetBarMarginLogic(const CRect &rect);

	void	MoveResizeeTo(CResizee *pResizee, CResizee *pResizeeAfter, BOOL bAfter = TRUE);
protected:
	friend class CResizee;
	POSITION	AddResizee(CResizee *pResizee);
	void		RemoveResizee(CResizee *pResizee);

	CList<CResizee*, CResizee*>		m_resizees;

	CRect		m_rtBarMarginLogic;
	int			m_iFixedTabsTotalLength;
	int			m_iFixedTabsCount;
};
}//namespace TabWnd{
