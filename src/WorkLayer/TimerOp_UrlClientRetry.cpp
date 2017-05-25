/*
 * $Id: TimerOp_UrlClientRetry.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
// TimerOp_UrlClientRetry.cpp : 实现文件
//

#include "stdafx.h"
#include "TimerOp_UrlClientRetry.h"
#include "HttpClient.h"
#include "Preferences.h"
#include "Log.h"


// CTimerOp_UrlClientRetry

IMPLEMENT_DYNAMIC(CTimerOp_UrlClientRetry, CTimerOpBase)
CTimerOp_UrlClientRetry::CTimerOp_UrlClientRetry()
{
}

CTimerOp_UrlClientRetry::~CTimerOp_UrlClientRetry()
{
}

BEGIN_MESSAGE_MAP(CTimerOp_UrlClientRetry, CTimerOpBase)
END_MESSAGE_MAP()

void CTimerOp_UrlClientRetry::TimerOp(WPARAM wParam, LPARAM /*lParam*/)
{
	CHttpClient	*pUrlClient = (CHttpClient*) wParam;

	if (NULL != pUrlClient && !IsBadWritePtr(pUrlClient, sizeof(CHttpClient)) )
	{
		if (thePrefs.GetVerbose())
			AddDebugLogLine(false, _T("Retry to connect <%s>"), pUrlClient->GetUserName());

		pUrlClient->TryToConnect(true);
	}
}


// CTimerOp_UrlClientRetry 消息处理程序

