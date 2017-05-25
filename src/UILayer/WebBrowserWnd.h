/*
 * $Id: WebBrowserWnd.h 6147 2008-07-10 03:16:33Z dgkang $
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

#include "htmlctrl.h"
#include "browsertoolbarctrl.h"
#include "comboboxenter.h"
#include "afxcmn.h"
#include "afxwin.h"
#include "ResizableLib\ResizableDialog.h"
#include "WBNotifyReceiver.h"
#include "Resource.h"
#include "TabItemAffector.h"


typedef enum  // 浏览器浏览不同页面状态
{
	EB_PT_LOADER, // 启动页
	EB_PT_PAGE    // 普通页面
}EM_BROWSER_PAGETYPE;

// CWebBrowserWnd 对话框

class CWebBrowserWnd : public CResizableDialog, public CTabItemAffector
{
	DECLARE_DYNAMIC(CWebBrowserWnd)

public:
	CWebBrowserWnd(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CWebBrowserWnd();

// 对话框数据
	enum { IDD = IDD_WEBBROWSER };
public:
	HICON          m_browsericon;
	void			SetOpenUrl(LPCTSTR lpszUrl){m_strOpenUrl = lpszUrl;}
	CString			GetUrl()const {return m_strOpenUrl;}
	CString			GetRealUrl();
	CHtmlCtrl*			m_pExplorer; //Added by thilon 2006.10.12
protected:
	EM_BROWSER_PAGETYPE m_pagetype;
	CString			m_strOpenUrl;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
	afx_msg void OnNewAddress();
	afx_msg void OnNewAddressEnter();

	afx_msg	void OnBack();
	afx_msg void OnForward();
	afx_msg void OnStop();
	afx_msg void OnHomePage();
	afx_msg void OnRefresh();

public:
	void Localize(void);
	CAnimateCtrl m_animate;
	CStatic m_status;
	CStatic m_staticError; // VC-SearchDream[2006-12-26]: Added for Runtime Error 
	UINT	m_uStridDisableReason;

	BOOL	IsBrowserCanUse(){return NULL != m_pExplorer;}
	void	DisableBrowser(UINT uStridReason);
public:
	EM_BROWSER_PAGETYPE GetPageType(void) { return m_pagetype;}

	void SetAddress(LPCTSTR lpszURL);
	void StartAnimation();
	void StopAnimation();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	CString ResourceToURL(LPCTSTR lpszURL);
	//Chocobo Start
	//浏览器访问指定页面，added by Chocobo on 2006.08.07
	//方便帮助链接在内置浏览器中显示
	void Navigate(LPCTSTR lpszURL);
	//Chocobo End
	virtual BOOL OnInitDialog();
	afx_msg LRESULT OnHcBeforeNavi(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnHcNaviCmpl(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDocCmpl(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnStatusTxtChange(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnTitleChange(WPARAM wParam, LPARAM lParam);
protected:
	virtual void OnOK();
	virtual void OnCancel();
};
