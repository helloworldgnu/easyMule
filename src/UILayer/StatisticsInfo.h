/*
 * $Id: StatisticsInfo.h 4483 2008-01-02 09:19:06Z soarchin $
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
#include "afxcmn.h"
// CStatisticsInfo 对话框

#include "ProgressCtrlX.h"
#include "IconStatic.h"

#include "Localizee.h"
#include "ResizableLib\ResizableDialog.h"

class CStatisticsInfo : public CResizableDialog, public CLocalizee
{
	DECLARE_DYNAMIC(CStatisticsInfo)
	LOCALIZEE_WND_CANLOCALIZE();

public:
	CStatisticsInfo(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CStatisticsInfo();

// 对话框数据
	enum { IDD = IDD_DIALOG_STATISTICS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
    virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
	
public:
	void SetRequest(int range, int pos);
	void SetAcceptUpload(int range, int pos);
	void SetTransfer(int range, uint64 pos);

	void SetAll(int request,  int accept, uint64 transfer);
	void SetNoFile();

	void SetStatisticsFrmText(CString str);

private:

	CProgressCtrlX pop_bar;
	CProgressCtrlX pop_baraccept;
	CProgressCtrlX pop_bartrans;
	CIconStatic m_ctrlStatisticsFrm;
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	void Localize(void);
};
