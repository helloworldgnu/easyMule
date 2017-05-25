/*
 * $Id: DlgMainTabAdvance.h 8435 2008-11-24 08:52:24Z huby $
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
#include "AdvanceTabWnd.h"

// CDlgMainTabAdvance 对话框

class CDlgMainTabAdvance : public CDialog
{
	DECLARE_DYNAMIC(CDlgMainTabAdvance)

public:
	CDlgMainTabAdvance(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgMainTabAdvance();

// 对话框数据
	enum { IDD = IDD_MAINTAB_ADVANCE };
protected:
	CAdvanceTabWnd				m_tbwAdvance;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	BOOL		IsTabActive(CAdvanceTabWnd::ETabId eTabId){return m_tbwAdvance.IsTabActive(eTabId);} 
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
protected:
	virtual void OnCancel();
	virtual void OnOK();
};
