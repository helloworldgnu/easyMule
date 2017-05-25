/* 
 * $Id: ToolTipCtrlX.h 4483 2008-01-02 09:19:06Z soarchin $
 * 
 * this file is part of eMule
 * Copyright (C)2002-2006 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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

class CToolTipCtrlX : public CToolTipCtrl
{
	DECLARE_DYNAMIC(CToolTipCtrlX)
public:
	CToolTipCtrlX();
	virtual ~CToolTipCtrlX();

	void SetCol1DrawTextFlags(DWORD dwFlags);
	void SetCol2DrawTextFlags(DWORD dwFlags);

protected:
	bool m_bCol1Bold;
	CRect m_rcScreen;
	int m_iScreenWidth4;
	COLORREF m_crTooltipBkColor;
	DWORD m_dwCol1DrawTextFlags;
	DWORD m_dwCol2DrawTextFlags;
	CFont m_fontBold;

	void ResetSystemMetrics();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnNMCustomDraw(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMThemeChanged(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSysColorChange();
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
};
