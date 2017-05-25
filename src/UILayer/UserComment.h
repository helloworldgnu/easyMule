/*
 * $Id: UserComment.h 7232 2008-09-11 10:39:39Z huby $
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

#include "resource.h"
// CUserComment 对话框
#include "WebBrowserWnd.h"
#include "ResizableLib\ResizableDialog.h"

class CUserComment : public CResizableDialog
{
	DECLARE_DYNAMIC(CUserComment)

public:
	CUserComment(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CUserComment();

	void Refresh(const CKnownFile * file);

// 对话框数据
	enum { IDD = IDD_DIALOG_COMMENT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

	CHtmlCtrl * m_pwebUserComment;
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	CString m_strLastCommentWeb;
};
