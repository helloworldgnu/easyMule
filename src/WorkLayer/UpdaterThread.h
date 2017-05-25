/*
 * $Id: UpdaterThread.h 4483 2008-01-02 09:19:06Z soarchin $
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

#include "EmuleUpdater.h"

//////////// Updater.exe 返回值/////////////////////////////////////////////
const int RESULT_NEWVERSION		= 100;

const int ERROR_UNKNOWN			= 0;	// 未知错误
const int ERROR_NOCONNECTION	= 1;	// 无连接
const int ERROR_SERVER			= 2;	// 无法连接到服务器

const int ERROR_CHECKFAIL		= 3;	// 校验失败
const int ERROR_MEMNOTCREATE	= 4;	// 共享内存创建失败
const int ERROR_MEMNOTOPEN		= 5;	// 共享内存打开失败
const int ERROR_MEMNOTMAP		= 6;	// 共享内存映射失败
const int ERROR_WRITEMEM		= 7;	// 无法写入共享内存

const int ERROR_NONEWVERSION	= 8;	// 没有新版本可用
const int ERROR_LOADFAIL		= 9;	// Load失败
////////////////////////////////////////////////////////////////////////////

// CUpdaterThread

class CUpdaterThread : public CWinThread
{
	DECLARE_DYNCREATE(CUpdaterThread)

protected:
	CUpdaterThread();           // 动态创建所使用的受保护的构造函数

public:
	virtual ~CUpdaterThread();
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	void SetParent(CDialog* pParentThread);

public:
	CString		m_strFileToDownloadInto;
	CEmuleUpdater		m_updater;

protected:
	int m_iResult;
	CDialog* m_pParent;
protected:
	DECLARE_MESSAGE_MAP()
	
	void PostMessageToParent(UINT message, WPARAM wParam, LPARAM lParam);
};
