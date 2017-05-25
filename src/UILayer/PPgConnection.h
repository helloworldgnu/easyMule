/*
 * $Id: PPgConnection.h 5130 2008-03-25 10:43:20Z fengwen $
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
#include "BetterSP2.h"   //Added by thilon 2006.08.07
#include "ToolTipCtrlZ.h"
#include "numericedit.h"
class CPPgConnection : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgConnection)

public:
	CPPgConnection();
	virtual ~CPPgConnection();

// Dialog Data
	enum { IDD = IDD_PPG_CONNECTION };

	void Localize(void);
	void LoadSettings(void);

protected:
	bool guardian;

	// VC-kernel[2007-03-02]:
	CSliderCtrl m_ctlMaxDown;
	CSliderCtrl m_ctlMaxUp;
	CComboBox m_ctlConnectionType;

	// VC-kernel[2007-02-27]:
	bool m_iUPnPNat;
	bool m_iUPnPTryRandom;

	int m_iMaxHalfOpen;
	int	m_isXP;					//Added by thilon on 2006.08.07
	CBetterSP2 m_betterSP2;		//Added by thilon on 2006.08.07
	int m_iTCPIPInit;			//Added by thilon on 2006.08.07

	CToolTipCtrlZ	m_ttc;

	//ADDED by VC-fengwen on 2008/03/18 <begin> : 
	void LoadSpeedValues();
	void SaveSpeedValues();

	BOOL m_bConnectionTypeChanging;
	void UpdateConnectionType(int iType);
	void ConnTypeToCustomize();

	UINT GetDownCapacity();
	UINT GetUpCapacity();
	BOOL GetDownLimitSwitch(){return IsDlgButtonChecked(IDC_DLIMIT_LBL);}
	BOOL GetUpLimitSwitch(){return IsDlgButtonChecked(IDC_ULIMIT_LBL);}
	UINT GetDownLimitValue(){return m_uDownloadLimitValue;}
	UINT GetUpLimitValue(){return m_uUploadLimitValue;}

	void UpdateDownCapacity(UINT uCapacity, BOOL bUpdateEdit = TRUE);
	void UpdateUpCapacity(UINT uCapacity, BOOL bUpdateEdit = TRUE);
	void UpdateDownLimitSwitch(BOOL bLimit);
	void UpdateUpLimitSwitch(BOOL bLimit);
	void UpdateDownLimitValue(UINT uLimit);
	void UpdateUpLimitValue(UINT uLimit);
	//ADDED by VC-fengwen on 2008/03/18 <end> : 

	BOOL	ValidateInputs(BOOL bPrompt = TRUE, BOOL bFocus = TRUE);

	void SetRateSliderTicks(CSliderCtrl& rRate);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSettingsChange()					{ SetModified(); }
	afx_msg void OnEnChangeUDPDisable();
	afx_msg void OnBnClickedDLimit();
	afx_msg void OnBnClickedULimit();
	afx_msg void OnBnClickedWizard();
	afx_msg void OnBnClickedNetworkKademlia();
	afx_msg void OnHelp();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnBnClickedOpenports();
	afx_msg void OnStartPortTest();
	afx_msg void OnEnChangeTCP();
	afx_msg void OnEnChangeUDP();
	afx_msg void OnEnChangePorts(uint8 istcpport);
public:
	afx_msg void OnBnClickedRandomPort();
	afx_msg void OnChangeSpin(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEnChangeDownloadCap();
	afx_msg void OnEnChangeUploadCap();
	afx_msg void OnCbnSelchangeConnectiontype();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	CNumericEdit m_MaxHalfConEdit;
	CNumericEdit m_MaxConEdit;
	CNumericEdit m_MaxSourcePerFileEdit;
	CNumericEdit m_DownloadEdit;
	CNumericEdit m_UploadEdit;
	CNumericEdit m_PortEdit;
	CNumericEdit m_UDPPortEdit;

	UINT	m_uDownloadLimitValue;		// 因为ScrollCtrl不能及时更新它的Position值，所以用变量来表示。
	UINT	m_uUploadLimitValue;		// 因为ScrollCtrl不能及时更新它的Position值，所以用变量来表示。
};
