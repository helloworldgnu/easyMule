/*
 * $Id: HotKeyEdit.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
/** @file HotKeyEdit.cpp @brief 本文件的简要说明，分行无效。
 <pre>
 *	Copyright (c) 2007，Emule
 *	All rights reserved.
 *
 *	当前版本：
 *	作    者：kernel
 *	完成日期：2007-05-15
 *
 *	取代版本：none
 *	作    者：none
 *	完成日期：none
 </pre>*/

//

#include "stdafx.h"
#include "emule.h"
#include "HotKeyEdit.h"


// CHotKeyEdit

IMPLEMENT_DYNAMIC(CHotKeyEdit, CHotKeyCtrl)
CHotKeyEdit::CHotKeyEdit()
{
}

CHotKeyEdit::~CHotKeyEdit()
{
}


BEGIN_MESSAGE_MAP(CHotKeyEdit, CHotKeyCtrl)
	//ON_MESSAGE(WM_KEYDOWN,OnKeyDown)
	//ON_MESSAGE(WM_SYSKEYDOWN,OnKeyDown)
	//ON_KEYDOWN()
	ON_WM_KEYDOWN()
	ON_WM_SYSKEYDOWN()
END_MESSAGE_MAP()



// CHotKeyEdit 消息处理程序

void CHotKeyEdit::OnKeyDown(UINT nChar,UINT nRepCnt,UINT nFlags)
{
	GetParent()->PostMessage(WM_HK_CHANGE);
	CHotKeyCtrl::OnKeyDown(nChar,nRepCnt,nFlags);
}

void CHotKeyEdit::OnSysKeyDown(UINT nChar,UINT nRepCnt,UINT nFlags)
{
	GetParent()->PostMessage(WM_HK_CHANGE);
	CHotKeyCtrl::OnSysKeyDown(nChar,nRepCnt,nFlags);
}
