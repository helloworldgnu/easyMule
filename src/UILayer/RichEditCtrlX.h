/*
 * $Id: RichEditCtrlX.h 4483 2008-01-02 09:19:06Z soarchin $
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

/////////////////////////////////////////////////////////////////////////////
// CRichEditCtrlX window

class CRichEditCtrlX : public CRichEditCtrl
{
public:
	CRichEditCtrlX();
	virtual ~CRichEditCtrlX();

	void SetDisableSelectOnFocus(bool bDisable = true);
	void SetSyntaxColoring(const LPCTSTR* ppszKeywords = NULL, LPCTSTR pszSeperators = NULL);

	CRichEditCtrlX& operator<<(LPCTSTR psz);
	CRichEditCtrlX& operator<<(char* psz);
	CRichEditCtrlX& operator<<(UINT uVal);
	CRichEditCtrlX& operator<<(int iVal);
	CRichEditCtrlX& operator<<(double fVal);

	void SetRTFText(const CStringA& rstrText);

protected:
	bool m_bDisableSelectOnFocus;
	bool m_bSelfUpdate;
	bool m_bForceArrowCursor;
	HCURSOR m_hArrowCursor;
	CStringArray m_astrKeywords;
	CString m_strSeperators;
	CHARFORMAT m_cfDef;
	CHARFORMAT m_cfKeyword;

	void UpdateSyntaxColoring();
	static DWORD CALLBACK StreamInCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb);

	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg UINT OnGetDlgCode();
	afx_msg BOOL OnEnLink(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnEnChange();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
};
