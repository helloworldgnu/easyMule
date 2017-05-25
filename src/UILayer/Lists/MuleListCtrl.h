/*
 * $Id: MuleListCtrl.h 4483 2008-01-02 09:19:06Z soarchin $
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
#include "Preferences.h"
#include "resource.h"

class CIni;

///////////////////////////////////////////////////////////////////////////////
// CMuleListCtrl

class CMuleListCtrl : public CListCtrl
{
	DECLARE_DYNAMIC(CMuleListCtrl)

public:
	CMuleListCtrl(PFNLVCOMPARE pfnCompare = SortProc, DWORD dwParamSort = 0);
	virtual ~CMuleListCtrl();

	// Default sort proc, this does nothing
	static int CALLBACK SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

	// Sets the list name, used for hide/show menu
	void SetName(LPCTSTR lpszName);

	// Save to preferences
	//void SaveSettings(CPreferences::Table tID);
	void SaveSettings();

	// Load from preferences
	//void LoadSettings(CPreferences::Table tID);
	void LoadSettings();

	DWORD SetExtendedStyle(DWORD dwNewStyle) { return CListCtrl::SetExtendedStyle(dwNewStyle | LVS_EX_HEADERDRAGDROP); }

	// Hide the column
	void HideColumn(int iColumn);

	// Unhide the column
	void ShowColumn(int iColumn);

	// Check to see if the column is hidden
	BOOL IsColumnHidden(int iColumn) const {
		if(iColumn < 1 || iColumn >= m_iColumnsTracked)
			return false;

		return m_aColumns[iColumn].bHidden;
	}

	// Get the correct column width even if column is hidden
	int GetColumnWidth(int iColumn) const {
		if(iColumn < 0 || iColumn >= m_iColumnsTracked)
			return 0;
		
		if(m_aColumns[iColumn].bHidden)
			return m_aColumns[iColumn].iWidth;
		else
			return CListCtrl::GetColumnWidth(iColumn);
	}

	// Call SetRedraw to allow changes to be redrawn or to prevent changes from being redrawn.
	void SetRedraw(BOOL bRedraw = TRUE) {
		if(bRedraw) {
			if(m_iRedrawCount > 0 && --m_iRedrawCount == 0)
				CListCtrl::SetRedraw(TRUE);
		} else {
			if(m_iRedrawCount++ == 0)
				CListCtrl::SetRedraw(FALSE);
		}
	}

	// Sorts the list
	BOOL SortItems(PFNLVCOMPARE pfnCompare, DWORD_PTR dwData) {
		return CListCtrl::SortItems(pfnCompare, dwData);
	}

	// Sorts the list
	BOOL SortItems(DWORD dwData) { return CListCtrl::SortItems(m_SortProc, dwData); }

	// Sets the sorting procedure
	void SetSortProcedure(PFNLVCOMPARE funcSortProcedure) { m_SortProc = funcSortProcedure; }

	// Gets the sorting procedure
	PFNLVCOMPARE GetSortProcedure() { return m_SortProc; }

	// Retrieves the data (lParam) associated with a particular item.
	DWORD_PTR GetItemData(int iItem);

	// Retrieves the number of items in the control.
	int GetItemCount() const { return m_Params.GetCount(); };

	enum ArrowType { arrowDown = IDB_DOWN, arrowUp = IDB_UP,
		arrowDoubleDown = IDB_DOWN2X, arrowDoubleUp = IDB_UP2X };

	int	GetSortType(ArrowType at);
	ArrowType GetArrowType(int iat);
	int GetSortItem() const { return m_iCurrentSortItem; }
	bool GetSortAscending() const { return m_atSortArrow == arrowUp || m_atSortArrow == arrowDoubleUp; }
	// Places a sort arrow in a column
	void SetSortArrow(int iColumn, ArrowType atType);
	void SetSortArrow() { SetSortArrow(m_iCurrentSortItem, m_atSortArrow); }
	void SetSortArrow(int iColumn, bool bAscending) { SetSortArrow(iColumn, bAscending ? arrowUp : arrowDown); }

	HIMAGELIST ApplyImageList(HIMAGELIST himl);

	// General purpose listview find dialog+functions (optional)
	void	SetGeneralPurposeFind(bool bEnable, bool bCanSearchInAllColumns = true) { m_bGeneralPurposeFind = bEnable; m_bCanSearchInAllColumns = bCanSearchInAllColumns; }
	void	DoFind(int iStartItem, int iDirection /*1=down, 0 = up*/, BOOL bShowError);
	void	DoFindNext(BOOL bShowError);

	void	AutoSelectItem();
	int		GetNextSortOrder(int dwCurrentSortOrder) const;
	void	UpdateSortHistory(int dwNewOrder, int dwInverseValue = 100);

	enum EUpdateMode {
		lazy,
		direct,
		none
	};
	enum EUpdateMode SetUpdateMode(enum EUpdateMode eMode);

protected:
	virtual void PreSubclassWindow();
	virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual BOOL OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

	afx_msg void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSysColorChange();

	DECLARE_MESSAGE_MAP()

	// Checks the item to see if it is in order
	int          UpdateLocation(int iItem);
	// Moves the item in list and returns the new index
	int          MoveItem(int iOldIndex, int iNewIndex);
	// Update the colors
	void         SetColors(LPCTSTR pszLvKey = NULL);

	CString         m_Name;
	PFNLVCOMPARE    m_SortProc;
	DWORD           m_dwParamSort;
	COLORREF        m_crWindow;
	COLORREF        m_crWindowText;
	COLORREF        m_crWindowTextBk;
	COLORREF        m_crHighlight;
	COLORREF		m_crHighlightText;
	COLORREF		m_crGlow;
	COLORREF        m_crFocusLine;
	COLORREF        m_crNoHighlight;
	COLORREF        m_crNoFocusLine;
	NMLVCUSTOMDRAW  m_lvcd;
	BOOL            m_bCustomDraw;
	CImageList		m_imlHeaderCtrl;
	CList<int, int>	m_liSortHistory;
	UINT			m_uIDAccel;
	HACCEL			m_hAccel;
	enum EUpdateMode m_eUpdateMode;

	// General purpose listview find dialog+functions (optional)
	bool m_bGeneralPurposeFind;
	bool m_bCanSearchInAllColumns;
	CString m_strFindText;
	bool m_bFindMatchCase;
	int m_iFindDirection;
	int m_iFindColumn;
	void OnFindStart();
	void OnFindNext();
	void OnFindPrev();

private:
	static int	IndexToOrder(CHeaderCtrl* pHeader, int iIndex);

	struct MULE_COLUMN {
		int iWidth;
		int iLocation;
		bool bHidden;
	};

	int          m_iColumnsTracked;
	MULE_COLUMN *m_aColumns;

	int GetHiddenColumnCount() const {
		int iHidden = 0;
		for(int i = 0; i < m_iColumnsTracked; i++)
			if(m_aColumns[i].bHidden)
				iHidden++;
		return iHidden;
	}

	int       m_iCurrentSortItem;
	ArrowType m_atSortArrow;

	int m_iRedrawCount;
	CList<DWORD_PTR> m_Params;

	DWORD_PTR GetParamAt(POSITION pos, int iPos) {
		LPARAM lParam = m_Params.GetAt(pos);
		if(lParam == 0xFEEBDEEF) //same as MLC_MAGIC!
			m_Params.SetAt(pos, lParam = CListCtrl::GetItemData(iPos));
		return lParam;
	}
};

void GetContextMenuPosition(CListCtrl& lv, CPoint& point);
