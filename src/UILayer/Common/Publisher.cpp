/*
 * $Id: Publisher.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
#include ".\publisher.h"

CPublisher::CPublisher(void)
{
}

CPublisher::~CPublisher(void)
{
}

void CPublisher::RegisterWnd(HWND hWnd)
{
	m_lstReceivers.AddTail(hWnd);
}
void CPublisher::UnRegisterWnd(HWND hWnd)
{
	m_lstReceivers.RemoveAt(m_lstReceivers.Find(hWnd));
}

void CPublisher::SendMsgToAllReceivers(UINT uMsg, WPARAM wParam, LPARAM lParam, HWND hExcept)
{
	HWND		hWnd;
	POSITION	pos = m_lstReceivers.GetHeadPosition();
	while (NULL != pos)
	{
		hWnd = m_lstReceivers.GetNext(pos);

		if (hExcept != hWnd)
			::SendMessage(hWnd, uMsg, wParam, lParam);
	}
}
