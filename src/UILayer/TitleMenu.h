/*
 * $Id: TitleMenu.h 4783 2008-02-02 08:17:12Z soarchin $
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
// TitleMenu.h: interface for the CTitleMenu class.
// Based on the code of Per Fikse(1999/06/16) on codeguru.earthweb.com
// Author: Arthur Westerman
// Bug reports by : Brian Pearson 
//////////////////////////////////////////////////////////////////////
#pragma once

typedef BOOL (WINAPI* TSetMenuInfo)(
  HMENU hmenu,       // handle to menu
  LPCMENUINFO lpcmi  // menu information
);
typedef BOOL (WINAPI* TGetMenuInfo)(
  HMENU hmenu,            // handle to menu
  LPCMENUINFO lpcmi       // menu information
);

class CTitleMenu : public CMenu
{
public:
	CTitleMenu();
	virtual ~CTitleMenu();

	void EnableIcons();
	void AddMenuTitle(LPCTSTR lpszTitle, bool bIsIconMenu = false);
	BOOL AppendMenu(UINT nFlags, UINT_PTR nIDNewItem = 0, LPCTSTR lpszNewItem = NULL, LPCTSTR lpszIconName = NULL);
	BOOL InsertMenu(UINT nPosition, UINT nFlags, UINT_PTR nIDNewItem = 0, LPCTSTR lpszNewItem = NULL, LPCTSTR lpszIconName = NULL);

	long GetColor() { return m_clLeft; }
	void SetColor(long cl) { m_clLeft = cl; }

	long GetGradientColor() { return m_clRight; }
	void SetGradientColor(long cl) { m_clRight = cl; }

	long GetTextColor() { return m_clText; }
	void SetTextColor(long cl) { m_clText = cl; }

	long GetEdge() { return m_uEdgeFlags; }
	void SetEdge(bool shown, UINT remove = 0, UINT add = 0)	{ m_bDrawEdge = shown; (m_uEdgeFlags ^= remove) |= add; }

	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMIS);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);

protected:
	CString m_strTitle;
	long m_clRight;
	long m_clLeft;
	long m_clText;
	bool m_bDrawEdge;
	bool m_bIconMenu;
	UINT m_uEdgeFlags;
	CImageList m_ImageList;
	CMap<int, int, int, int> m_mapIconPos;

	typedef UINT (WINAPI* LPFNGRADIENTFILL)(HDC, CONST PTRIVERTEX, DWORD, CONST PVOID, DWORD, DWORD);
	LPFNGRADIENTFILL m_pfnGradientFill;
	HINSTANCE m_hLibMsimg32;

	static bool m_bInitializedAPI;
	static bool LoadAPI();
	static void FreeAPI();
	static TSetMenuInfo SetMenuInfo;
	static TGetMenuInfo GetMenuInfo;
	void DrawMonoIcon(int nIconPos, CPoint nDrawPos, CDC *dc);
	void SetMenuBitmap(UINT nFlags, UINT_PTR nIDNewItem, LPCTSTR lpszNewItem, LPCTSTR lpszIconName);
};
