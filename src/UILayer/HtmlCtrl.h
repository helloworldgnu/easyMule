/*
 * $Id: HtmlCtrl.h 9297 2008-12-24 09:55:04Z dgkang $
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
#include "IEManagerWnd.h"


// CHtmlCtrl Html 视图

#include "ProgressStatic.h"

class CHtmlCtrl : public CHtmlView
{
	DECLARE_DYNCREATE(CHtmlCtrl)

public:
	CHtmlCtrl(){ };           // 动态创建所使用的受保护的构造函数
	virtual ~CHtmlCtrl()
	{
	}

	BOOL CreateFromStatic(UINT nID, CWnd* pParent);

public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	/*
	virtual void BeforeNavigate2(LPDISPATCH pDisp, VARIANT* URL,
		VARIANT* Flags, VARIANT* TargetFrameName, VARIANT* PostData,
		VARIANT* Headers, VARIANT_BOOL* Cancel);
	*/

	virtual HRESULT OnTranslateUrl(DWORD dwTranslate,
		OLECHAR* pchURLIn, OLECHAR** ppchURLOut);

	virtual BOOL OnAmbientProperty(COleControlSite* pSite,DISPID dispid, VARIANT* pvar);


	DECLARE_MESSAGE_MAP()

	CProgressStatic		m_sttcProgress;
public:
	virtual void OnNewWindow2(LPDISPATCH* ppDisp, BOOL* Cancel);
	virtual void OnBeforeNavigate2(LPCTSTR lpszURL, DWORD nFlags, LPCTSTR lpszTargetFrameName, CByteArray& baPostedData, LPCTSTR lpszHeaders, BOOL* pbCancel);
	afx_msg void OnDestroy();
	virtual void OnDocumentComplete(LPCTSTR lpszURL);
	afx_msg int  OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	virtual void OnNavigateComplete2(LPCTSTR strURL);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual void OnStatusTextChange(LPCTSTR lpszText);
protected:
	virtual void PostNcDestroy();
public:
	virtual void OnTitleChange(LPCTSTR lpszText);
	virtual void OnProgressChange(long nProgress, long nProgressMax);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void OnCommandStateChange(long nCommand, BOOL bEnable);
	LRESULT OnNvTo(WPARAM wParam, LPARAM lParam);

protected:
	HWND GetBrowserHwnd(void);
	CIEManagerWnd m_IEManagerWnd;

public:
	//VC-dgkang 
	CString m_NewWindowsURL;
	BOOL	m_bCancel;
	BOOL bForwordEnable;
	BOOL bBackEnable;
};

