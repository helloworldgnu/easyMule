/*
 * $Id: HTRichEditCtrl.h 4483 2008-01-02 09:19:06Z soarchin $
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

#include "TitleMenu.h"

class CHTRichEditCtrl : public CRichEditCtrl
{
	DECLARE_DYNAMIC(CHTRichEditCtrl)

public:
	CHTRichEditCtrl();
	virtual ~CHTRichEditCtrl();

	void Init(LPCTSTR pszTitle, LPCTSTR pszSkinKey = NULL);
	void SetProfileSkinKey(LPCTSTR pszSkinKey);
	void SetTitle(LPCTSTR pszTitle);
	void Localize();
	void ApplySkin();

	void AddEntry(LPCTSTR pszMsg);
	void Add(LPCTSTR pszMsg, int iLen = -1);
	void AddTyped(LPCTSTR pszMsg, int iLen, UINT uFlags);
	void AddLine(LPCTSTR pszMsg, int iLen = -1, bool bLink = false, COLORREF cr = CLR_DEFAULT,COLORREF bk = CLR_DEFAULT, DWORD mask = 0);
	void Reset();
	CString GetLastLogEntry();
	CString GetAllLogEntries();
	bool SaveLog(LPCTSTR pszDefName = NULL);

	void AppendText(const CString& sText);
	void AppendHyperLink(const CString& sText, const CString& sTitle, const CString& sCommand, const CString& sDirectory);
	void AppendKeyWord(const CString& sText, COLORREF cr);
	void AppendColoredText(LPCTSTR pszText, COLORREF cr, COLORREF bk = CLR_DEFAULT,DWORD mask = 0);

	CString GetText() const;
	bool IsAutoScroll() const { return m_bAutoScroll; }
	void ScrollToLastLine(bool bForceLastLineAtBottom = false);

	void SetFont(CFont* pFont, BOOL bRedraw = TRUE);
	CFont* GetFont() const;

protected:
	bool m_bRichEdit;
	int m_iLimitText;
	bool m_bAutoScroll;
	CStringArray m_astrBuff;
	bool m_bNoPaint;
	bool m_bEnErrSpace;
	CString m_strTitle;
	CString m_strSkinKey;
	bool m_bRestoreFormat;
	CHARFORMAT m_cfDefault;
	bool m_bForceArrowCursor;
	HCURSOR m_hArrowCursor;

	void SelectAllItems();
	void CopySelectedItems();
	int GetMaxSize();
	void SafeAddLine(int nPos, LPCTSTR pszLine, int iLen, long& nStartChar, long& nEndChar, bool bLink, COLORREF cr, COLORREF bk, DWORD mask);
	void FlushBuffer();
	void AddString(int nPos, LPCTSTR pszString, bool bLink, COLORREF cr, COLORREF bk, DWORD mask);

	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnEnErrspace();
	afx_msg void OnEnMaxtext();
	afx_msg BOOL OnEnLink(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSysColorChange();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
};
