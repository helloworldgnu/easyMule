/*
 * $Id: SplashWndThread.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
// SplashWndThread.cpp : 实现文件
//

#include "stdafx.h"
#include "emule.h"
#include "SplashWndThread.h"
#include "SplashScreen.h"


// CSplashWndThread

IMPLEMENT_DYNCREATE(CSplashWndThread, CWinThread)

CSplashWndThread::CSplashWndThread()
{
}

CSplashWndThread::~CSplashWndThread()
{
}

BOOL CSplashWndThread::InitInstance()
{
	// TODO: 在此执行任意逐线程初始化
	m_splashScreen.Create(m_splashScreen.IDD, CWnd::GetDesktopWindow());
	m_splashScreen.ShowWindow(SW_SHOW);

	return TRUE;
}

int CSplashWndThread::ExitInstance()
{
	// TODO: 在此执行任意逐线程清理
	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(CSplashWndThread, CWinThread)
END_MESSAGE_MAP()


// CSplashWndThread 消息处理程序
