/*
 * $Id: ComboBoxEnter.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
// ComboBoxEnter.cpp : 实现文件
//

#include "stdafx.h"
#include "emule.h"
#include "ComboBoxEnter.h"
#include ".\comboboxenter.h"


// CComboBoxEnter

IMPLEMENT_DYNAMIC(CComboBoxEnter, CComboBox)
CComboBoxEnter::CComboBoxEnter()
{
}

CComboBoxEnter::~CComboBoxEnter()
{
}


BEGIN_MESSAGE_MAP(CComboBoxEnter, CComboBox)
END_MESSAGE_MAP()



// CComboBoxEnter 消息处理程序


BOOL CComboBoxEnter::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	ASSERT( pMsg->message != WM_CHARTOITEM );

	if (pMsg->message == WM_KEYDOWN)
	{
		GetParent()->SendMessage(pMsg->message, pMsg->wParam,  pMsg->lParam);
	}
	return CComboBox::PreTranslateMessage(pMsg);
}
