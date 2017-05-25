/*
 * $Id: Wizard.h 12458 2009-04-27 10:31:25Z huby $
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

#include "PShtWiz1.h"
#include "ToolTipCtrlZ.h"
#include "UILayer/NumericEdit.h"

class CConnectionWizardDlg : public CDlgPageWizard
{
	DECLARE_DYNAMIC(CConnectionWizardDlg)
public:
	//CConnectionWizardDlg(CWnd* pParent = NULL);   // standard constructor
	CConnectionWizardDlg(LPCTSTR pszCaption = NULL, LPCTSTR pszHeaderTitle = NULL, LPCTSTR pszHeaderSubTitle = NULL);
	virtual ~CConnectionWizardDlg();

	enum { IDD = IDD_WIZ1_CONNECTION };

	void Localize();

protected:
	HICON m_icnWnd;
	CListCtrl m_provider;
	int m_iOS;
	int m_iTotalDownload;
	int m_iConnectionType;

	CNumericEdit m_MaxHalfConnEdit;
	CNumericEdit m_MaxNewConnPer5Edit;

	CToolTipCtrlZ	m_ttc;

	void SetCustomItemsActivation();
	void RecommendHalpOpenChanging();
	BOOL ValidateInputs();

	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedApply();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedWizRadioOsNtxp();
	afx_msg void OnBnClickedWizRadioUs98me();
	afx_msg void OnBnClickedWizLowdownloadRadio();
	afx_msg void OnBnClickedWizMediumdownloadRadio();
	afx_msg void OnBnClickedWizHighdownloadRadio();
	afx_msg void OnBnClickedWizResetButton();
	afx_msg void OnNMClickProviders(NMHDR *pNMHDR, LRESULT *pResult);

	virtual LRESULT OnWizardNext();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
public:
	afx_msg void OnBnClickedKbits();
	afx_msg void OnBnClickedKbytes();

};
