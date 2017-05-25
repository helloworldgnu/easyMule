/*
 * $Id: ListCtrlEditable.h 4483 2008-01-02 09:19:06Z soarchin $
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

class CEditableListCtrl : public CListCtrl
{
public:
	CEditableListCtrl();

	CEdit* GetEditCtrl() const { return m_pctrlEdit; }
	void CommitEditCtrl();

	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	CEdit* m_pctrlEdit;
	CComboBox* m_pctrlComboBox;
	int m_iRow;
	int m_iCol;
	int m_iEditRow;
	int m_iEditCol;

	void ShowEditCtrl();
	void ShowComboBoxCtrl();
	void ResetTopPosition();
	void ResetBottomPosition();
	void VerifyScrollPos();

	BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeydown(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDestroy();
	afx_msg void OnEnKillFocus();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnNMCustomDraw(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnEndScroll(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnBeginScroll(NMHDR *pNMHDR, LRESULT *pResult);
};
