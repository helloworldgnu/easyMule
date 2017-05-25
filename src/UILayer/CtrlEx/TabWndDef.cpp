/*
 * $Id: TabWndDef.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
#include ".\TabWndDef.h"

namespace TabWnd
{

	void Real2Logic(CRect &rect, ETabBarPos eRealPos)
	{
		CRect	rtResult;
		switch (eRealPos)
		{
		case TBP_TOP:
		default:
			rtResult = rect;
			break;
		case TBP_RIGHT:
			rtResult.left	= rect.top;
			rtResult.top	= - rect.right;
			rtResult.right	= rect.bottom;
			rtResult.bottom	= - rect.left;
			break;
		case TBP_BOTTOM:
			rtResult.left	= rect.left;
			rtResult.top	= - rect.bottom;
			rtResult.right	= rect.right;
			rtResult.bottom	= - rect.top;
			break;
		case TBP_LEFT:
			rtResult.left	= rect.top;
			rtResult.top	= rect.left;
			rtResult.right	= rect.bottom;
			rtResult.bottom	= rect.right;
			break;
		}
		rect = rtResult;
	}

	void Logic2Real(CRect &rect, ETabBarPos eRealPos)
	{
		CRect	rtResult;
		switch (eRealPos)
		{
		case TBP_TOP:
		default:
			rtResult = rect;
			break;
		case TBP_RIGHT:
			rtResult.left	= - rect.bottom;
			rtResult.top	= rect.left;
			rtResult.right	= - rect.top;
			rtResult.bottom	= rect.right;
			break;
		case TBP_BOTTOM:
			rtResult.left	= rect.left;
			rtResult.top	= - rect.bottom;
			rtResult.right	= rect.right;
			rtResult.bottom	= - rect.top;
			break;
		case TBP_LEFT:
			rtResult.left	= rect.top;
			rtResult.top	= rect.left;
			rtResult.right	= rect.bottom;
			rtResult.bottom	= rect.right;
			break;
		}
		rect = rtResult;
	}
}
