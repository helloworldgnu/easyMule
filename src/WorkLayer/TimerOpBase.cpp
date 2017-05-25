/*
 * $Id: TimerOpBase.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
// TimerOpBase.cpp : 实现文件
//

#include "stdafx.h"
#include "TimerOpBase.h"


// CTimerOpBase

IMPLEMENT_DYNAMIC(CTimerOpBase, CWnd)
CTimerOpBase::CTimerOpBase()
{
	m_wParam		= 0;
	m_lParam		= 0;
	m_uLeftTimes	= 0;

	CreateEx(0, _T("static"), NULL, 0, CRect(0, 0, 0, 0), NULL, 0);
}

CTimerOpBase::~CTimerOpBase()
{
	Stop();
	DestroyWindow();
}

BEGIN_MESSAGE_MAP(CTimerOpBase, CWnd)
	ON_WM_TIMER()
END_MESSAGE_MAP()

// CTimerOpBase 消息处理程序

BOOL CTimerOpBase::Start(UINT uInterval, WPARAM wParam, LPARAM lParam, UINT uTimes)
{
	m_wParam		= wParam;
	m_lParam		= lParam;
	m_uLeftTimes	= uTimes;

	SetTimer(1, uInterval, NULL);

	return TRUE;
}

void CTimerOpBase::Stop()
{
	KillTimer(1);

	m_wParam		= 0;
	m_lParam		= 0;
	m_uLeftTimes	= 0;
}

void CTimerOpBase::OnTimer(UINT nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (1 == nIDEvent)
	{
		if (0 != m_uLeftTimes)	// 当 0 == m_uLeftTimes 时，表示让定时器一直循环下去。
		{
			m_uLeftTimes--;
			if (0 == m_uLeftTimes)
				KillTimer(1);
		}
		
		TimerOp(m_wParam, m_lParam);
	}

	CWnd::OnTimer(nIDEvent);
}
