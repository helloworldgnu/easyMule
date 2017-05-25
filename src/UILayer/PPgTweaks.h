/*
 * $Id: PPgTweaks.h 4904 2008-03-06 03:38:23Z wangna $
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
#include "TreeOptionsCtrlEx.h"
#include "BetterSP2.h"   //Added by thilon 2006.08.07

enum Data_Type
{
	type_int,
	type_float,
	type_uint,
	type_string
};

class CPPgTweaks : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgTweaks)

public:
	CPPgTweaks();
	virtual ~CPPgTweaks();

// Dialog Data
	enum { IDD = IDD_PPG_TWEAKS };

	void Localize(void);

protected:
	UINT m_iFileBufferSize;
	UINT m_iQueueSize;

	int	m_iMaxSourceConnect;
	int	m_iRetryNumber;
	int m_iRetryDelay;
	int m_iPublicMaxConnectLimit;


	int m_iMaxConnPerFive;
	//int m_iMaxHalfOpen;
	bool m_bConditionalTCPAccept;
	bool m_bAutoTakeEd2kLinks;
	bool m_bVerbose;
	bool m_bDebugSourceExchange;
	bool m_bLogBannedClients;
	bool m_bLogRatingDescReceived;
	bool m_bLogSecureIdent;
	bool m_bLogFilteredIPs;
	bool m_bLogFileSaving;
    bool m_bLogA4AF;
	bool m_bLogUlDlEvents;
	bool m_bCreditSystem;
	bool m_bLog2Disk;
	bool m_bDebug2Disk;
	int m_iCommitFiles;
	bool m_bFilterLANIPs;
	bool m_bExtControls;
	UINT m_uServerKeepAliveTimeout;
	bool m_bSparsePartFiles;
	bool m_bFullAlloc;
	bool m_bCheckDiskspace;
	float m_fMinFreeDiskSpaceMB;
	CString m_sYourHostname;
	bool m_bFirewallStartup;
	int m_iLogLevel;
	bool m_bDisablePeerCache;
    bool m_bDynUpEnabled;
    int m_iDynUpMinUpload;
    int m_iDynUpPingTolerance;
    int m_iDynUpPingToleranceMilliseconds;
    int m_iDynUpRadioPingTolerance;
    int m_iDynUpGoingUpDivider;
    int m_iDynUpGoingDownDivider;
    int m_iDynUpNumberOfPings;

	//int	m_isXP;					//Added by thilon on 2006.08.07
	//CBetterSP2 m_betterSP2;		//Added by thilon on 2006.08.07
	//int m_iTCPIPInit;			//Added by thilon on 2006.08.07

	//EastShare Start - added by AndCycle, IP to Country
	HTREEITEM m_htiIP2CountryName;
	HTREEITEM m_htiIP2CountryName_DISABLE;
	HTREEITEM m_htiIP2CountryName_SHORT;
	HTREEITEM m_htiIP2CountryName_MID;
	HTREEITEM m_htiIP2CountryName_LONG;
	HTREEITEM m_htiIP2CountryShowFlag;
	int m_iIP2CountryName;
	int m_bIP2CountryShowFlag;
	//EastShare End - added by AndCycle, IP to Country
    bool m_bA4AFSaveCpu;
	bool m_bAutoArchDisable;
	int m_iExtractMetaData;

	int m_iShareeMule;		//Added by thilon on 2007.05.25
	bool bShowedWarning;

	CSliderCtrl m_ctlFileBuffSize;
	CSliderCtrl m_ctlQueueSize;
    CTreeOptionsCtrlEx m_ctrlTreeOptions;
	bool m_bInitializedTreeOpts;

	HTREEITEM m_htiConnectLimitGroup;
	HTREEITEM m_htiMaxSourceConnect;
	HTREEITEM m_htiRetryNumber;
	HTREEITEM m_htiRetryDelay;
	HTREEITEM m_htiPublicMaxConnectLimit;
	HTREEITEM m_htiTCPGroup;
	HTREEITEM m_htiMaxCon5Sec;
	HTREEITEM m_htiMaxHalfOpen;
	HTREEITEM m_htiConditionalTCPAccept;
	HTREEITEM m_htiAutoTakeEd2kLinks;
	HTREEITEM m_htiVerboseGroup;
	HTREEITEM m_htiVerbose;
	HTREEITEM m_htiDebugSourceExchange;
	HTREEITEM m_htiLogBannedClients;
	HTREEITEM m_htiLogRatingDescReceived;
	HTREEITEM m_htiLogSecureIdent;
	HTREEITEM m_htiLogFilteredIPs;
	HTREEITEM m_htiLogFileSaving;
    HTREEITEM m_htiLogA4AF;
	HTREEITEM m_htiLogUlDlEvents;
	HTREEITEM m_htiCreditSystem;
	HTREEITEM m_htiLog2Disk;
	HTREEITEM m_htiDebug2Disk;
	HTREEITEM m_htiCommit;
	HTREEITEM m_htiCommitNever;
	HTREEITEM m_htiCommitOnShutdown;
	HTREEITEM m_htiCommitAlways;
	HTREEITEM m_htiFilterLANIPs;
	HTREEITEM m_htiExtControls;
	HTREEITEM m_htiServerKeepAliveTimeout;
	HTREEITEM m_htiSparsePartFiles;
	HTREEITEM m_htiFullAlloc;
	HTREEITEM m_htiCheckDiskspace;
	HTREEITEM m_htiMinFreeDiskSpace;
	HTREEITEM m_htiYourHostname;
	HTREEITEM m_htiFirewallStartup;
	HTREEITEM m_htiLogLevel;
	HTREEITEM m_htiDisablePeerCache;
    HTREEITEM m_htiDynUp;
	HTREEITEM m_htiDynUpEnabled;
    HTREEITEM m_htiDynUpMinUpload;
    HTREEITEM m_htiDynUpPingTolerance;
    HTREEITEM m_htiDynUpPingToleranceMilliseconds;
    HTREEITEM m_htiDynUpPingToleranceGroup;
    HTREEITEM m_htiDynUpRadioPingTolerance;
    HTREEITEM m_htiDynUpRadioPingToleranceMilliseconds;
    HTREEITEM m_htiDynUpGoingUpDivider;
    HTREEITEM m_htiDynUpGoingDownDivider;
    HTREEITEM m_htiDynUpNumberOfPings;
    HTREEITEM m_htiA4AFSaveCpu;
	HTREEITEM m_htiExtractMetaData;
	HTREEITEM m_htiExtractMetaDataNever;
	HTREEITEM m_htiExtractMetaDataID3Lib;
	HTREEITEM m_htiAutoArch;

	//Added by thilon on 2007.05.25
	HTREEITEM m_htiShareeMule;
	HTREEITEM m_htiShareeMuleMultiUser;
	HTREEITEM m_htiShareeMulePublicUser;
	HTREEITEM m_htiShareeMuleOldStyle;
	//HTREEITEM m_htiExtractMetaDataMediaDet;

	int m_uUploadClients;			//Added by thilon on 2006.08.08, 固定上传线程
	HTREEITEM m_htiUploadClients;	//Added by thilon on 2006.08.08, 固定上传线程

	//upnp_start - Added by thilon on 2006.09.24
/*
	HTREEITEM m_htiUPnPNat;
	HTREEITEM m_htiUPnPTryRandom;
	bool m_iUPnPNat;
	bool m_iUPnPTryRandom;
*/
	//upnp_end

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	virtual BOOL OnKillActive();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnTreeOptsCtrlNotify(WPARAM wParam, LPARAM lParam);
	afx_msg void OnHelp();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
public:
	int m_iOrgMaxSourceConnect;
	int	m_iOrgRetryNumber;
	int m_iOrgRetryDelay;
	int m_iOrgPublicMaxConnectLimit;
	int m_iOrgMaxConnPerFive;
	UINT m_uOrgServerKeepAliveTimeout;
	float m_fOrgMinFreeDiskSpaceMB;
	int m_uOrgUploadClients;
	int m_iOrgLogLevel;
	int m_iOrgDynUpMinUpload;
	int m_iOrgDynUpPingTolerance;
	int m_iOrgDynUpPingToleranceMilliseconds;
	int m_iOrgDynUpGoingUpDivider;
	int m_iOrgDynUpGoingDownDivider;
	int m_iOrgDynUpNumberOfPings;
	CString m_sOrgYourHostname;
	void MinMaxInt(CDataExchange* pDX,int nIDC,  HTREEITEM hItem, int value, int minVal, int maxVal,int orgValue);
	void MinMaxFloat(CDataExchange* pDX,int nIDC, HTREEITEM hItem,float value,float minVal,float maxVal,float orgVal);

	struct Item_Value
	{
		CString minVal;
		CString maxVal;
		CString orgVal;
		Data_Type type;
	};
	CRBMap <HTREEITEM ,Item_Value> m_ItemValueList;
	void AddItemValueList(HTREEITEM hItem,CString minVal,CString maxVal,CString orgVal,Data_Type type);
	Item_Value m_ItemValue;
	void ModifyItemValue(HTREEITEM hItem,int iVal);
	void ModifyItemValue(HTREEITEM hItem,float fVal);
	void ModifyItemValue(HTREEITEM hItem,UINT uVal);
	void ModifyItemValue(HTREEITEM hItem,CString strVal);
};
