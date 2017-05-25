/*
 * $Id: SearchParamsWnd.h 4483 2008-01-02 09:19:06Z soarchin $
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

#include "EditX.h"
#include "ComboBoxEx2.h"
#include "ListCtrlEditable.h"
class CCustomAutoComplete;

typedef enum EOptsRows
{
	orMinSize,
	orMaxSize,
	orAvailability,
	orCompleteSources,
	orExtension,
	orCodec,
	orBitrate,
	orLength,
	orTitle,
	orAlbum,
	orArtist
};

class CSearchParamsWnd : public CDialogBar
{
	DECLARE_DYNAMIC(CSearchParamsWnd);

// Construction
public:
	CSearchParamsWnd();
	virtual ~CSearchParamsWnd();

// Dialog Data
	enum { IDD = IDD_SEARCH_PARAMS };

	CEditX  m_ctlName;
	CButton m_ctlStart;
	CButton m_ctlCancel;
	CButton m_ctlMore;
	CButton m_ctlUnicode;

	CSearchResultsWnd* m_searchdlg;

	void Localize();
	void ResetHistory();
	void SaveSettings();

	SSearchParams* GetParameters();
	void SetParameters(const SSearchParams* pParams);

	virtual CSize CalcDynamicLayout(int, DWORD nMode);
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	HCURSOR m_hcurMove;
	CComboBoxEx2 m_ctlMethod;
	CComboBoxEx2 m_ctlFileType;
	CEditableListCtrl m_ctlOpts;
	CRect m_rcNameLbl;
	CRect m_rcName;
	CRect m_rcDropDownArrow;
	CRect m_rcFileTypeLbl;
	CRect m_rcFileType;
	CRect m_rcReset;
	CRect m_rcMethodLbl;
	CRect m_rcMethod;
	CRect m_rcOpts;
	CRect m_rcStart;
	CRect m_rcMore;
	CRect m_rcCancel;
	CRect m_rcUnicode;
	CRect m_rcClear;

	CImageList m_imlSearchMethods;
	CImageList m_imlFileType;
	CSize m_szMRU;
	CSize m_szFloat;
	CCustomAutoComplete* m_pacSearchString;

	void UpdateControls();
	void UpdateUnicodeCtrl();
	BOOL SaveSearchStrings();
	void SetAllIcons();
	void InitMethodsCtrl();
	void InitFileTypesCtrl();
	uint64 GetSearchAttrSize(const CString& rstrExpr);
	ULONG GetSearchAttrNumber(const CString& rstrExpr);
	ULONG GetSearchAttrLength(const CString& rstrExpr);

	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnInitDialog(WPARAM, LPARAM);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedStart();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedMore();
	afx_msg void OnCbnSelChangeMethod();
	afx_msg void OnCbnSelEndOkMethod();
	afx_msg void OnDDClicked();
	afx_msg void OnBnClickedSearchReset();
	afx_msg void OnEnChangeName();
	afx_msg void OnDestroy();
	afx_msg void OnSysColorChange();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
public:
	afx_msg void OnBnClickedClear();
};
