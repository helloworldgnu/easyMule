/* 
 * $Id: PPgTweaks.cpp 6656 2008-08-15 05:22:11Z dgkang $
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

#include "stdafx.h"
#include "emule.h"
#include "SearchDlg.h"
#include "PPgTweaks.h"
#include "Scheduler.h"
#include "DownloadQueue.h"
#include "Preferences.h"
#include "OtherFunctions.h"
#include "TransferWnd.h"
#include "emuledlg.h"
#include "SharedFilesWnd.h"
#include "ServerWnd.h"
#include "HelpIDs.h"
#include "Log.h"
#include "UserMsgs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//EastShare Start - added by AndCycle, IP to Country
#include "ip2country.h"
#include ".\ppgtweaks.h"
//EastShare End - added by AndCycle, IP to Country

///////////////////////////////////////////////////////////////////////////////
// CPPgTweaks dialog

IMPLEMENT_DYNAMIC(CPPgTweaks, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgTweaks, CPropertyPage)
	ON_WM_HSCROLL()
	ON_WM_DESTROY()
	ON_MESSAGE(UM_TREEOPTSCTRL_NOTIFY, OnTreeOptsCtrlNotify)
	ON_WM_HELPINFO()
END_MESSAGE_MAP()

CPPgTweaks::CPPgTweaks()
	: CPropertyPage(CPPgTweaks::IDD)
	, m_ctrlTreeOptions(this ,theApp.m_iDfltImageListColorFlags)
{
	m_iFileBufferSize = 0;
	m_iQueueSize = 0;

	m_iMaxSourceConnect = 0;
	m_iRetryNumber = 0;
	m_iRetryDelay = 0;
    m_htiPublicMaxConnectLimit = 0;
	m_iPublicMaxConnectLimit = 0;

	m_iMaxConnPerFive = 0;
	//m_iMaxHalfOpen = 0;
	m_bConditionalTCPAccept = false;
	m_bAutoTakeEd2kLinks = false;
	m_bVerbose = false;
	m_bDebugSourceExchange = false;
	m_bLogBannedClients = false;
	m_bLogRatingDescReceived = false;
	m_bLogSecureIdent = false;
	m_bLogFilteredIPs = false;
	m_bLogFileSaving = false;
	m_bLogA4AF = false;
	m_bLogUlDlEvents = false;
	m_bCreditSystem = false;
	m_bLog2Disk = false;
	m_bDebug2Disk = false;
	m_iCommitFiles = 0;
	m_bFilterLANIPs = false;
	m_bExtControls = false;
	m_uServerKeepAliveTimeout = 0;
	m_bSparsePartFiles = false;
	m_bFullAlloc = false;
	m_bCheckDiskspace = false;
	m_fMinFreeDiskSpaceMB = 0.0F;
	(void)m_sYourHostname;
	m_bFirewallStartup = false;
	m_iLogLevel = 0;
	m_bDisablePeerCache = false;
    m_bDynUpEnabled = false;
    m_iDynUpMinUpload = 0;
    m_iDynUpPingTolerance = 0;
    m_iDynUpGoingUpDivider = 0;
    m_iDynUpGoingDownDivider = 0;
    m_iDynUpNumberOfPings = 0;
    m_bA4AFSaveCpu = false;
	m_iExtractMetaData = 0;
	m_bAutoArchDisable=true;

	m_iShareeMule = 0; //Added by thilon on 2007.05.25
	bShowedWarning = false;

	m_bInitializedTreeOpts = false;

	m_htiConnectLimitGroup = NULL;
	m_htiMaxSourceConnect = NULL;
	m_htiRetryNumber = NULL;
	m_htiRetryDelay = NULL;
    m_htiPublicMaxConnectLimit = NULL;



	m_htiTCPGroup = NULL;
	m_htiMaxCon5Sec = NULL;
	m_htiMaxHalfOpen = NULL;
	m_htiConditionalTCPAccept = NULL;
	m_htiAutoTakeEd2kLinks = NULL;
	m_htiVerboseGroup = NULL;
	m_htiVerbose = NULL;
	m_htiDebugSourceExchange = NULL;
	m_htiLogBannedClients = NULL;
	m_htiLogRatingDescReceived = NULL;
	m_htiLogSecureIdent = NULL;
	m_htiLogFilteredIPs = NULL;
	m_htiLogFileSaving = NULL;
	m_htiLogUlDlEvents = NULL;
	m_htiCreditSystem = NULL;
	m_htiLog2Disk = NULL;
	m_htiDebug2Disk = NULL;
	m_htiCommit = NULL;
	m_htiCommitNever = NULL;
	m_htiCommitOnShutdown = NULL;
	m_htiCommitAlways = NULL;
	m_htiFilterLANIPs = NULL;
	m_htiExtControls = NULL;
	m_htiServerKeepAliveTimeout = NULL;
	m_htiSparsePartFiles = NULL;
	m_htiFullAlloc = NULL;
	m_htiCheckDiskspace = NULL;
	m_htiMinFreeDiskSpace = NULL;
	m_htiYourHostname = NULL;
	m_htiFirewallStartup = NULL;
	m_htiLogLevel = NULL;
	m_htiDisablePeerCache = NULL;
    m_htiDynUp = NULL;
	m_htiDynUpEnabled = NULL;
    m_htiDynUpMinUpload = NULL;
    m_htiDynUpPingTolerance = NULL;
    m_htiDynUpPingToleranceMilliseconds = NULL;
    m_htiDynUpPingToleranceGroup = NULL;
    m_htiDynUpRadioPingTolerance = NULL;
    m_htiDynUpRadioPingToleranceMilliseconds = NULL;
    m_htiDynUpGoingUpDivider = NULL;
    m_htiDynUpGoingDownDivider = NULL;
    m_htiDynUpNumberOfPings = NULL;
    m_htiA4AFSaveCpu = NULL;
	m_htiLogA4AF = NULL;
	m_htiExtractMetaData = NULL;
	m_htiAutoArch = NULL;

	//Added by thilon on 2007.05.25
	m_htiShareeMule = NULL;
	m_htiShareeMuleMultiUser = NULL;
	m_htiShareeMulePublicUser = NULL;
	m_htiShareeMuleOldStyle = NULL;

	//EastShare Start - added by AndCycle, IP to Country
	m_htiIP2CountryName = NULL;
	m_htiIP2CountryName_DISABLE = NULL;
	m_htiIP2CountryName_SHORT = NULL;
	m_htiIP2CountryName_MID = NULL;
	m_htiIP2CountryName_LONG = NULL;
	m_htiIP2CountryShowFlag = NULL;
	//EastShare End - added by AndCycle, IP to Country

	m_htiUploadClients = NULL;//Added by thilon on 2006.08.08, 固定上传线程

	//m_isXP			= 0;	//added by thilon on 2006.08.07
	//m_iTCPIPInit	= 0;	//added by thilon on 2006.08.07

	m_iOrgMaxSourceConnect = 0;
	m_iOrgRetryNumber = 0;
	m_iOrgRetryDelay = 0;
	m_iOrgPublicMaxConnectLimit = 0;
	m_iOrgMaxConnPerFive = 0;
	m_uOrgServerKeepAliveTimeout = 0;
	m_fOrgMinFreeDiskSpaceMB = 0;
    m_uOrgUploadClients = 0;
    m_iOrgLogLevel = 0;
	m_iOrgDynUpMinUpload = 0;
	m_iOrgDynUpPingTolerance = 0;
	m_iOrgDynUpPingToleranceMilliseconds = 0;
	m_iOrgDynUpGoingUpDivider = 0;
	m_iOrgDynUpGoingDownDivider = 0;
	m_iOrgDynUpNumberOfPings = 0;
}

CPPgTweaks::~CPPgTweaks()
{
}

void CPPgTweaks::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FILEBUFFERSIZE, m_ctlFileBuffSize);
	DDX_Control(pDX, IDC_QUEUESIZE, m_ctlQueueSize);
	DDX_Control(pDX, IDC_EXT_OPTS, m_ctrlTreeOptions);
	if (!m_bInitializedTreeOpts)
	{
		int iImgIP2Country = 8; //EastShare - added by AndCycle, IP to Country
		int iImgBackup = 8; // default icon
		int iImgLog = 8;
		int iImgDynyp = 8;
		int iImgConnection = 8;
		int iImgA4AF = 8;
		int iImgMetaData = 8;
		int iImgShareeMule = 8;	//Added by thilon on 2007.05.25

        CImageList* piml = m_ctrlTreeOptions.GetImageList(TVSIL_NORMAL);
		if (piml){
			iImgBackup =	piml->Add(CTempIconLoader(_T("Harddisk")));
			iImgLog =		piml->Add(CTempIconLoader(_T("Log")));
			iImgDynyp =		piml->Add(CTempIconLoader(_T("upload")));
			iImgConnection=	piml->Add(CTempIconLoader(_T("connection")));
			iImgIP2Country = piml->Add(CTempIconLoader(_T("SEARCHMETHOD_GLOBAL"))); //EastShare - added by AndCycle, IP to Country
            iImgA4AF =		piml->Add(CTempIconLoader(_T("Download")));
            iImgMetaData =	piml->Add(CTempIconLoader(_T("MediaInfo")));
			iImgShareeMule =piml->Add(CTempIconLoader(_T("viewfiles")));	//Added by thilon on 2007.05.25
		}

		/////////////////////////////////////////////////////////////////////////////
		// Connect Limit group
		m_htiConnectLimitGroup = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_LIMIT_GROUP), iImgConnection, TVI_ROOT);

		m_htiMaxSourceConnect = m_ctrlTreeOptions.InsertItem(GetResString(IDS_LIMIT_SOURCE_CONNS), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiConnectLimitGroup);
		m_ctrlTreeOptions.AddEditBox(m_htiMaxSourceConnect, RUNTIME_CLASS(CNumTreeOptionsEdit));

		m_htiRetryNumber = m_ctrlTreeOptions.InsertItem(GetResString(IDS_LIMIT_RETRY_NUMBER), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiConnectLimitGroup);
		m_ctrlTreeOptions.AddEditBox(m_htiRetryNumber, RUNTIME_CLASS(CNumTreeOptionsEdit));

		m_htiRetryDelay = m_ctrlTreeOptions.InsertItem(GetResString(IDS_LIMIT_RETRY_DELAY), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiConnectLimitGroup);
		m_ctrlTreeOptions.AddEditBox(m_htiRetryDelay, RUNTIME_CLASS(CNumTreeOptionsEdit));

        m_htiPublicMaxConnectLimit = m_ctrlTreeOptions.InsertItem(_T("IDS_PUB_CONNECT_LIMIT"), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiConnectLimitGroup);
		m_ctrlTreeOptions.AddEditBox(m_htiPublicMaxConnectLimit,RUNTIME_CLASS(CNumTreeOptionsEdit));
		/////////////////////////////////////////////////////////////////////////////
		// TCP/IP group
		//
		m_htiTCPGroup = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_TCPIP_CONNS), iImgConnection, TVI_ROOT);
		m_htiMaxCon5Sec = m_ctrlTreeOptions.InsertItem(GetResString(IDS_MAXCON5SECLABEL), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiTCPGroup);
		m_ctrlTreeOptions.AddEditBox(m_htiMaxCon5Sec, RUNTIME_CLASS(CNumTreeOptionsEdit));

		//检测系统，成功可修改最大半连接数，失败不能修改最大半连接数 added by thilon on 2006.08.07
		/*if(m_isXP)
		{
			m_htiMaxHalfOpen = m_ctrlTreeOptions.InsertItem(GetResString(IDS_MAXHALFOPENCONS), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiTCPGroup);
			m_ctrlTreeOptions.AddEditBox(m_htiMaxHalfOpen, RUNTIME_CLASS(CNumTreeOptionsEdit));
		}*/
		
		m_htiConditionalTCPAccept = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_CONDTCPACCEPT), m_htiTCPGroup, m_bConditionalTCPAccept);
		m_htiServerKeepAliveTimeout = m_ctrlTreeOptions.InsertItem(GetResString(IDS_SERVERKEEPALIVETIMEOUT), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiTCPGroup);
		m_ctrlTreeOptions.AddEditBox(m_htiServerKeepAliveTimeout, RUNTIME_CLASS(CNumTreeOptionsEdit));

		/////////////////////////////////////////////////////////////////////////////
		// Miscellaneous group
		//
		m_htiAutoTakeEd2kLinks = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_AUTOTAKEED2KLINKS), TVI_ROOT, m_bAutoTakeEd2kLinks);
		m_htiCreditSystem = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_USECREDITSYSTEM), TVI_ROOT, m_bCreditSystem);
		m_htiFirewallStartup = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_FO_PREF_STARTUP), TVI_ROOT, m_bFirewallStartup);
		m_htiFilterLANIPs = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_PW_FILTER), TVI_ROOT, m_bFilterLANIPs);
		m_htiExtControls = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SHOWEXTSETTINGS), TVI_ROOT, m_bExtControls);
        m_htiA4AFSaveCpu = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_A4AF_SAVE_CPU), TVI_ROOT, m_bA4AFSaveCpu); // ZZ:DownloadManager
		m_htiAutoArch  = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DISABLE_AUTOARCHPREV), TVI_ROOT, m_bAutoArchDisable);
		m_htiYourHostname = m_ctrlTreeOptions.InsertItem(GetResString(IDS_YOURHOSTNAME), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, TVI_ROOT);
		m_ctrlTreeOptions.AddEditBox(m_htiYourHostname, RUNTIME_CLASS(CTreeOptionsEditEx));
		m_htiDisablePeerCache = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DISABLEPEERACHE), TVI_ROOT, m_bDisablePeerCache);

		/////////////////////////////////////////////////////////////////////////////
		// File related group
		//
		m_htiSparsePartFiles = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_SPARSEPARTFILES), TVI_ROOT, m_bSparsePartFiles);
		m_htiFullAlloc = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_FULLALLOC), TVI_ROOT, m_bFullAlloc);
		m_htiCheckDiskspace = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_CHECKDISKSPACE), TVI_ROOT, m_bCheckDiskspace);
		m_htiMinFreeDiskSpace = m_ctrlTreeOptions.InsertItem(GetResString(IDS_MINFREEDISKSPACE), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiCheckDiskspace);
		m_ctrlTreeOptions.AddEditBox(m_htiMinFreeDiskSpace, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiCommit = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_COMMITFILES), iImgBackup, TVI_ROOT);
		m_htiCommitNever = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_NEVER), m_htiCommit, m_iCommitFiles == 0);
		m_htiCommitOnShutdown = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_ONSHUTDOWN), m_htiCommit, m_iCommitFiles == 1);
		m_htiCommitAlways = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_ALWAYS), m_htiCommit, m_iCommitFiles == 2);
		m_htiExtractMetaData = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_EXTRACT_META_DATA), iImgMetaData, TVI_ROOT);
		m_htiExtractMetaDataNever = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_NEVER), m_htiExtractMetaData, m_iExtractMetaData == 0);
		m_htiExtractMetaDataID3Lib = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_META_DATA_ID3LIB), m_htiExtractMetaData, m_iExtractMetaData == 1);
		//m_htiExtractMetaDataMediaDet = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_META_DATA_MEDIADET), m_htiExtractMetaData, m_iExtractMetaData == 2);

		//EastShare Start - added by AndCycle, IP to Country
		m_htiIP2CountryName = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_IP2LOCATION), iImgIP2Country, TVI_ROOT);
		m_htiIP2CountryName_DISABLE = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_DISABLED), m_htiIP2CountryName, m_iIP2CountryName == IP2CountryName_DISABLE);
		m_htiIP2CountryName_SHORT = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_LOCATIONNAME_SHORT), m_htiIP2CountryName, m_iIP2CountryName == IP2CountryName_SHORT);
		m_htiIP2CountryName_MID = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_LOCATIONNAME_MID), m_htiIP2CountryName, m_iIP2CountryName == IP2CountryName_MID);
		m_htiIP2CountryName_LONG = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_LOCATIONNAME_LONG), m_htiIP2CountryName, m_iIP2CountryName == IP2CountryName_LONG);
		m_htiIP2CountryShowFlag = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_LOCATIONNAME_SHOWFLAG), m_htiIP2CountryName, m_bIP2CountryShowFlag);
		//EastShare End - added by AndCycle, IP to Country

		//Added by thilon on 2006.08.08,固定上传线程
		m_htiUploadClients = m_ctrlTreeOptions.InsertItem(GetResString(IDS_UPLOADCLIENTS), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, TVI_ROOT);
		m_ctrlTreeOptions.AddEditBox(m_htiUploadClients, RUNTIME_CLASS(CNumTreeOptionsEdit));

		/////////////////////////////////////////////////////////////////////////////
		// Logging group
		//
		m_htiLog2Disk = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_LOG2DISK), TVI_ROOT, m_bLog2Disk);
		if (thePrefs.GetEnableVerboseOptions())
		{
			m_htiVerboseGroup = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_VERBOSE), iImgLog, TVI_ROOT);
			m_htiVerbose = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_ENABLED), m_htiVerboseGroup, m_bVerbose);
			m_htiLogLevel = m_ctrlTreeOptions.InsertItem(GetResString(IDS_LOG_LEVEL), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiVerboseGroup);
			m_ctrlTreeOptions.AddEditBox(m_htiLogLevel, RUNTIME_CLASS(CNumTreeOptionsEdit));
			m_htiDebug2Disk = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_LOG2DISK), m_htiVerboseGroup, m_bDebug2Disk);
			m_htiDebugSourceExchange = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DEBUG_SOURCE_EXCHANGE), m_htiVerboseGroup, m_bDebugSourceExchange);
			m_htiLogBannedClients = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_LOG_BANNED_CLIENTS), m_htiVerboseGroup, m_bLogBannedClients);
			m_htiLogRatingDescReceived = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_LOG_RATING_RECV), m_htiVerboseGroup, m_bLogRatingDescReceived);
			m_htiLogSecureIdent = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_LOG_SECURE_IDENT), m_htiVerboseGroup, m_bLogSecureIdent);
			m_htiLogFilteredIPs = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_LOG_FILTERED_IPS), m_htiVerboseGroup, m_bLogFilteredIPs);
			m_htiLogFileSaving = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_LOG_FILE_SAVING), m_htiVerboseGroup, m_bLogFileSaving);
			m_htiLogA4AF = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_LOG_A4AF), m_htiVerboseGroup, m_bLogA4AF); // ZZ:DownloadManager
			m_htiLogUlDlEvents = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_LOG_ULDL_EVENTS), m_htiVerboseGroup, m_bLogUlDlEvents);
		}

		/////////////////////////////////////////////////////////////////////////////
		// USS group
		//
        m_htiDynUp = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_DYNUP), iImgDynyp, TVI_ROOT);
		m_htiDynUpEnabled = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DYNUPENABLED), m_htiDynUp, m_bDynUpEnabled);
        m_htiDynUpMinUpload = m_ctrlTreeOptions.InsertItem(GetResString(IDS_DYNUP_MINUPLOAD), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiDynUp);
		m_ctrlTreeOptions.AddEditBox(m_htiDynUpMinUpload, RUNTIME_CLASS(CNumTreeOptionsEdit));
        m_htiDynUpPingTolerance = m_ctrlTreeOptions.InsertItem(GetResString(IDS_DYNUP_PINGTOLERANCE), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiDynUp);
		m_ctrlTreeOptions.AddEditBox(m_htiDynUpPingTolerance, RUNTIME_CLASS(CNumTreeOptionsEdit));
        m_htiDynUpPingToleranceMilliseconds = m_ctrlTreeOptions.InsertItem(GetResString(IDS_DYNUP_PINGTOLERANCE_MS), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiDynUp);
        m_ctrlTreeOptions.AddEditBox(m_htiDynUpPingToleranceMilliseconds, RUNTIME_CLASS(CNumTreeOptionsEdit));
        m_htiDynUpPingToleranceGroup = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_DYNUP_RADIO_PINGTOLERANCE_HEADER), iImgDynyp, m_htiDynUp);
        m_htiDynUpRadioPingTolerance = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_DYNUP_RADIO_PINGTOLERANCE_PERCENT), m_htiDynUpPingToleranceGroup, m_iDynUpRadioPingTolerance == 0);
        m_htiDynUpRadioPingToleranceMilliseconds = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_DYNUP_RADIO_PINGTOLERANCE_MS), m_htiDynUpPingToleranceGroup, m_iDynUpRadioPingTolerance == 1);
        m_htiDynUpGoingUpDivider = m_ctrlTreeOptions.InsertItem(GetResString(IDS_DYNUP_GOINGUPDIVIDER), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiDynUp);
		m_ctrlTreeOptions.AddEditBox(m_htiDynUpGoingUpDivider, RUNTIME_CLASS(CNumTreeOptionsEdit));
        m_htiDynUpGoingDownDivider = m_ctrlTreeOptions.InsertItem(GetResString(IDS_DYNUP_GOINGDOWNDIVIDER), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiDynUp);
		m_ctrlTreeOptions.AddEditBox(m_htiDynUpGoingDownDivider, RUNTIME_CLASS(CNumTreeOptionsEdit));
        m_htiDynUpNumberOfPings = m_ctrlTreeOptions.InsertItem(GetResString(IDS_DYNUP_NUMBEROFPINGS), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiDynUp);
		m_ctrlTreeOptions.AddEditBox(m_htiDynUpNumberOfPings, RUNTIME_CLASS(CNumTreeOptionsEdit));
		
		/////////////////////////////////////////////////////////////////////////////
		// eMule Shared User, Added by thilon on 2007.05.25
		//
		m_htiShareeMule = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_SHAREEMULELABEL), iImgShareeMule, TVI_ROOT);
		m_htiShareeMuleMultiUser = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SHAREEMULEMULTI), m_htiShareeMule, m_iCommitFiles == 0);
		m_htiShareeMulePublicUser = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SHAREEMULEPUBLIC), m_htiShareeMule, m_iCommitFiles == 1);
		m_htiShareeMuleOldStyle = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_SHAREEMULEOLD), m_htiShareeMule, m_iCommitFiles == 2);

		m_ctrlTreeOptions.Expand(m_htiConnectLimitGroup, TVE_EXPAND);
	    m_ctrlTreeOptions.Expand(m_htiTCPGroup, TVE_EXPAND);
        if (m_htiVerboseGroup)
		    m_ctrlTreeOptions.Expand(m_htiVerboseGroup, TVE_EXPAND);

		//Added by thilon on 2006.09.24, for UPnP
		//upnp_start
		//m_htiUPnPNat = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_CN_UPNPNAT), TVI_ROOT, m_iUPnPNat);
		//m_htiUPnPTryRandom = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_CN_UPNPTRYRANDOM), TVI_ROOT, m_iUPnPTryRandom);
		//upnp_end

		m_ctrlTreeOptions.Expand(m_htiCommit, TVE_EXPAND);
		m_ctrlTreeOptions.Expand(m_htiCheckDiskspace, TVE_EXPAND);
		m_ctrlTreeOptions.Expand(m_htiDynUp, TVE_EXPAND);
        m_ctrlTreeOptions.Expand(m_htiDynUpPingToleranceGroup, TVE_EXPAND);
		m_ctrlTreeOptions.Expand(m_htiIP2CountryName, TVE_EXPAND);	//VeryCD
		m_ctrlTreeOptions.Expand(m_htiExtractMetaData, TVE_EXPAND);
		m_ctrlTreeOptions.Expand(m_htiShareeMule, TVE_EXPAND);	//Added by thilon on 2007.05.25
        m_ctrlTreeOptions.SendMessage(WM_VSCROLL, SB_TOP);
        m_bInitializedTreeOpts = true;
	}
    CString strMin,strMax,strOrg;
	/////////////////////////////////////////////////////////////////////////////
	// Connect Limit group
	DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiMaxSourceConnect, m_iMaxSourceConnect);
	//DDV_MinMaxInt(pDX, m_iMaxSourceConnect, 1, 10);
	strMin = _T("1");
	strMax = _T("10");
	strOrg.Format(_T("%d"),m_iOrgMaxSourceConnect);
	AddItemValueList(m_htiMaxSourceConnect,strMin,strMax,strOrg,type_int);
	MinMaxInt(pDX,IDC_EXT_OPTS,m_htiMaxSourceConnect,m_iMaxSourceConnect,1,10,m_iOrgMaxSourceConnect);

	DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiRetryNumber, m_iRetryNumber);
	//DDV_MinMaxInt(pDX, m_iRetryNumber, 1, 999);
	strMin = _T("1");
	strMax = _T("999");
	strOrg.Format(_T("%d"),m_iOrgRetryNumber);
    AddItemValueList(m_htiRetryNumber,strMin,strMax,strOrg,type_int);
    MinMaxInt(pDX,IDC_EXT_OPTS,m_htiRetryNumber,m_iRetryNumber,1,999,m_iOrgRetryNumber);

	DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiRetryDelay, m_iRetryDelay);
	//DDV_MinMaxInt(pDX, m_iRetryDelay, 1, 600);
	strMin = _T("1");
	strMax = _T("600");
	strOrg.Format(_T("%d"),m_iOrgRetryDelay);
	AddItemValueList(m_htiRetryDelay,strMin,strMax,strOrg,type_int);
	MinMaxInt(pDX,IDC_EXT_OPTS, m_htiRetryDelay,m_iRetryDelay,1,600,m_iOrgRetryDelay);

	DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiPublicMaxConnectLimit, m_iPublicMaxConnectLimit);
   // DDV_MinMaxInt(pDX, m_iPublicMaxConnectLimit, 1, 999);
	strMin = _T("1");
	strMax = _T("999");
	strOrg.Format(_T("%d"),m_iOrgPublicMaxConnectLimit);
	AddItemValueList(m_htiPublicMaxConnectLimit,strMin,strMax,strOrg,type_int);
	MinMaxInt(pDX, IDC_EXT_OPTS, m_htiPublicMaxConnectLimit,m_iPublicMaxConnectLimit,1,999,m_iOrgPublicMaxConnectLimit);

	/////////////////////////////////////////////////////////////////////////////
	// TCP/IP group
	//
	DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiMaxCon5Sec, m_iMaxConnPerFive);
	//DDV_MinMaxInt(pDX, m_iMaxConnPerFive, 1, INT_MAX);
	strMin = _T("1");
	strMax.Format(_T("%d"),INT_MAX);
	strOrg.Format(_T("%d"),m_iOrgMaxConnPerFive);
    AddItemValueList(m_htiMaxCon5Sec,strMin,strMax,strOrg,type_int);
	MinMaxInt(pDX,IDC_EXT_OPTS,m_htiMaxCon5Sec,m_iMaxConnPerFive,1,INT_MAX,m_iOrgMaxConnPerFive);
	
	//Added by thilon on 2006.08.07
	/*if(m_isXP)
	{
		DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiMaxHalfOpen, m_iMaxHalfOpen);
		DDV_MinMaxInt(pDX, m_iMaxHalfOpen, 1, INT_MAX);
	}*/

	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiConditionalTCPAccept, m_bConditionalTCPAccept);
	DDX_Text(pDX, IDC_EXT_OPTS, m_htiServerKeepAliveTimeout, m_uServerKeepAliveTimeout);
	strMin = _T("0");
	strMax.Format(_T("%d"),INT_MAX);
	strOrg.Format(_T("%d"),m_uOrgServerKeepAliveTimeout);
    AddItemValueList(m_htiServerKeepAliveTimeout,strMin,strMax,strOrg,type_uint);
	/////////////////////////////////////////////////////////////////////////////
	// Miscellaneous group
	//
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiAutoTakeEd2kLinks, m_bAutoTakeEd2kLinks);
	m_ctrlTreeOptions.SetCheckBoxEnable(m_htiAutoTakeEd2kLinks, HaveEd2kRegAccess());
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiCreditSystem, m_bCreditSystem);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiFirewallStartup, m_bFirewallStartup);
	m_ctrlTreeOptions.SetCheckBoxEnable(m_htiFirewallStartup, thePrefs.GetWindowsVersion() == _WINVER_XP_);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiFilterLANIPs, m_bFilterLANIPs);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiExtControls, m_bExtControls);
    DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiA4AFSaveCpu, m_bA4AFSaveCpu);
	DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiYourHostname, m_sYourHostname);
	AddItemValueList(m_htiYourHostname,NULL,NULL,m_sOrgYourHostname,type_string);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiDisablePeerCache, m_bDisablePeerCache);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiAutoArch, m_bAutoArchDisable);

	/////////////////////////////////////////////////////////////////////////////
	// File related group
	//
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiSparsePartFiles, m_bSparsePartFiles);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiFullAlloc, m_bFullAlloc);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiCheckDiskspace, m_bCheckDiskspace);
	DDX_Text(pDX, IDC_EXT_OPTS, m_htiMinFreeDiskSpace, m_fMinFreeDiskSpaceMB);
	//DDV_MinMaxFloat(pDX, m_fMinFreeDiskSpaceMB, 0.0, UINT_MAX / (1024*1024));
	strMin.Format(_T("%d"),0);
	strMax.Format(_T("%d"),UINT_MAX / (1024 *1024));
	strOrg.Format(_T("%1.0f"),m_fOrgMinFreeDiskSpaceMB);
	AddItemValueList(m_htiMinFreeDiskSpace,strMin,strMax,strOrg,type_float);
	MinMaxFloat(pDX,IDC_EXT_OPTS,m_htiMinFreeDiskSpace,m_fMinFreeDiskSpaceMB,0,UINT_MAX / (1024*1024),m_fOrgMinFreeDiskSpaceMB);

	DDX_TreeRadio(pDX, IDC_EXT_OPTS, m_htiCommit, m_iCommitFiles);
	DDX_TreeRadio(pDX, IDC_EXT_OPTS, m_htiExtractMetaData, m_iExtractMetaData);

	/////////////////////////////////////////////////////////////////////////////
	// Logging group
	//
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiLog2Disk, m_bLog2Disk);
	if (m_htiLogLevel){
		DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiLogLevel, m_iLogLevel);
		//DDV_MinMaxInt(pDX, m_iLogLevel, 1, 5);
		strMin = _T("1");
		strMax = _T("5");
		strOrg.Format(_T("%d"),m_iOrgLogLevel);
		AddItemValueList(m_htiLogLevel,strMin,strMax,strOrg,type_int);
		MinMaxInt(pDX,IDC_EXT_OPTS,m_htiLogLevel,m_iLogLevel,1,5,m_iOrgLogLevel);
	}	
	if (m_htiVerbose)				DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiVerbose, m_bVerbose);
	if (m_htiDebug2Disk)			DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiDebug2Disk, m_bDebug2Disk);
	if (m_htiDebug2Disk)			m_ctrlTreeOptions.SetCheckBoxEnable(m_htiDebug2Disk, m_bVerbose);
	if (m_htiDebugSourceExchange)	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiDebugSourceExchange, m_bDebugSourceExchange);
	if (m_htiDebugSourceExchange)	m_ctrlTreeOptions.SetCheckBoxEnable(m_htiDebugSourceExchange, m_bVerbose);
	if (m_htiLogBannedClients)		DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiLogBannedClients, m_bLogBannedClients);
	if (m_htiLogBannedClients)		m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogBannedClients, m_bVerbose);
	if (m_htiLogRatingDescReceived) DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiLogRatingDescReceived, m_bLogRatingDescReceived);
	if (m_htiLogRatingDescReceived) m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogRatingDescReceived, m_bVerbose);
	if (m_htiLogSecureIdent)		DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiLogSecureIdent, m_bLogSecureIdent);
	if (m_htiLogSecureIdent)		m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogSecureIdent, m_bVerbose);
	if (m_htiLogFilteredIPs)		DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiLogFilteredIPs, m_bLogFilteredIPs);
	if (m_htiLogFilteredIPs)		m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogFilteredIPs, m_bVerbose);
	if (m_htiLogFileSaving)			DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiLogFileSaving, m_bLogFileSaving);
	if (m_htiLogFileSaving)			m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogFileSaving, m_bVerbose);
    if (m_htiLogA4AF)			    DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiLogA4AF, m_bLogA4AF);
	if (m_htiLogA4AF)               m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogA4AF, m_bVerbose);
	if (m_htiLogUlDlEvents)			DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiLogUlDlEvents, m_bLogUlDlEvents);
	if (m_htiLogUlDlEvents)         m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogUlDlEvents, m_bVerbose);

	/////////////////////////////////////////////////////////////////////////////
	// USS group
	//
    DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiDynUpEnabled, m_bDynUpEnabled);
    DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiDynUpMinUpload, m_iDynUpMinUpload);
	//DDV_MinMaxInt(pDX, m_iDynUpMinUpload, 1, INT_MAX);
	strMin = _T("1");
	strMax.Format(_T("%d"),INT_MAX);
	strOrg.Format(_T("%d"),m_iOrgDynUpMinUpload);
    AddItemValueList(m_htiDynUpMinUpload,strMin,strMax,strOrg,type_int);
	MinMaxInt(pDX,IDC_EXT_OPTS,m_htiDynUpMinUpload,m_iDynUpMinUpload,1,INT_MAX,m_iOrgDynUpMinUpload);

    DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiDynUpPingTolerance, m_iDynUpPingTolerance);
	//DDV_MinMaxInt(pDX, m_iDynUpPingTolerance, 100, INT_MAX);
	strMin = _T("100");
	strMax.Format(_T("%d"),INT_MAX);
	strOrg.Format(_T("%d"),m_iOrgDynUpPingTolerance);
	AddItemValueList(m_htiDynUpPingTolerance,strMin,strMax,strOrg,type_int);
	MinMaxInt(pDX,IDC_EXT_OPTS,m_htiDynUpPingTolerance,m_iDynUpPingTolerance,100,INT_MAX,m_iOrgDynUpPingTolerance);

    DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiDynUpPingToleranceMilliseconds, m_iDynUpPingToleranceMilliseconds);
	//DDV_MinMaxInt(pDX, m_iDynUpPingTolerance, 1, INT_MAX);
	strMin = _T("1");
	strMax.Format(_T("%d"),INT_MAX);
	strOrg.Format(_T("%d"),m_iOrgDynUpPingToleranceMilliseconds);
	AddItemValueList(m_htiDynUpPingToleranceMilliseconds,strMin,strMax,strOrg,type_int);
	MinMaxInt(pDX,IDC_EXT_OPTS,m_htiDynUpPingToleranceMilliseconds,m_iDynUpPingToleranceMilliseconds,1,INT_MAX,m_iOrgDynUpPingToleranceMilliseconds);

    DDX_TreeRadio(pDX, IDC_EXT_OPTS, m_htiDynUpPingToleranceGroup, m_iDynUpRadioPingTolerance);
    DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiDynUpGoingUpDivider, m_iDynUpGoingUpDivider);
	//DDV_MinMaxInt(pDX, m_iDynUpGoingUpDivider, 1, INT_MAX);
	strMin = _T("1");
	strMax.Format(_T("%d"),INT_MAX);
	strOrg.Format(_T("%d"),m_iOrgDynUpGoingUpDivider);
	AddItemValueList(m_htiDynUpGoingUpDivider,strMin,strMax,strOrg,type_int);
	MinMaxInt(pDX,IDC_EXT_OPTS,m_htiDynUpGoingUpDivider,m_iDynUpGoingUpDivider,1,INT_MAX,m_iOrgDynUpGoingUpDivider);


    DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiDynUpGoingDownDivider, m_iDynUpGoingDownDivider);
	//DDV_MinMaxInt(pDX, m_iDynUpGoingDownDivider, 1, INT_MAX);
	strMin = _T("1");
	strMax.Format(_T("%d"),INT_MAX);
	strOrg.Format(_T("%d"),m_iOrgDynUpGoingDownDivider);
	AddItemValueList(m_htiDynUpGoingDownDivider,strMin,strMax,strOrg,type_int);
	MinMaxInt(pDX,IDC_EXT_OPTS,m_htiDynUpGoingDownDivider,m_iDynUpGoingDownDivider,1,INT_MAX,m_iOrgDynUpGoingDownDivider);

    DDX_TreeEdit(pDX, IDC_EXT_OPTS, m_htiDynUpNumberOfPings, m_iDynUpNumberOfPings);
	//DDV_MinMaxInt(pDX, m_iDynUpNumberOfPings, 1, INT_MAX);
	strMin = _T("1");
	strMax.Format(_T("%d"),INT_MAX);
	strOrg.Format(_T("%d"),m_iOrgDynUpNumberOfPings);
	AddItemValueList(m_htiDynUpNumberOfPings,strMin,strMax,strOrg,type_int);
	MinMaxInt(pDX,IDC_EXT_OPTS,m_htiDynUpNumberOfPings,m_iDynUpNumberOfPings,1,INT_MAX,m_iOrgDynUpNumberOfPings);

	//EastShare Start - added by AndCycle, IP to Country
	DDX_TreeRadio(pDX, IDC_EXT_OPTS, m_htiIP2CountryName, (int &)m_iIP2CountryName);
	DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiIP2CountryShowFlag, m_bIP2CountryShowFlag);
	//EastShare End - added by AndCycle, IP to Country


	//upnp_start - Added by thilon on 2006.09.24
	//DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiUPnPNat, m_iUPnPNat);
	//DDX_TreeCheck(pDX, IDC_EXT_OPTS, m_htiUPnPTryRandom, m_iUPnPTryRandom);
	//m_ctrlTreeOptions.SetCheckBoxEnable(m_htiUPnPTryRandom, m_iUPnPNat);
	//upnp_end

	//Added by thilon on 2006.08.08, 固定上传线程
	DDX_Text(pDX, IDC_EXT_OPTS, m_htiUploadClients, m_uUploadClients);
	strMin = _T("0");
    strMax.Format(_T("%d"),INT_MAX);
	strOrg.Format(_T("%d"),m_uOrgUploadClients);
    AddItemValueList(m_htiUploadClients,strMin,strMax,strOrg,type_int);

	/////////////////////////////////////////////////////////////////////////////
	// eMule Shared User, Added by thilon on 2007.05.25
	//
	DDX_TreeRadio(pDX, IDC_EXT_OPTS, m_htiShareeMule, m_iShareeMule);
	m_ctrlTreeOptions.SetRadioButtonEnable(m_htiShareeMulePublicUser, thePrefs.GetWindowsVersion() == _WINVER_VISTA_);
	m_ctrlTreeOptions.SetRadioButtonEnable(m_htiShareeMuleMultiUser, thePrefs.GetWindowsVersion() != _WINVER_95_ &&thePrefs.GetWindowsVersion() != _WINVER_NT4_);
}
void CPPgTweaks::AddItemValueList(HTREEITEM hItem,CString minVal,CString maxVal,CString orgVal,Data_Type type)
{
	Item_Value itemVal;

	if (m_ItemValueList.Lookup(hItem,itemVal))
	{
		itemVal.minVal = minVal;
		itemVal.maxVal = maxVal;
		itemVal.orgVal = orgVal;
	}
	else
	{
	 	itemVal.minVal = minVal;
		itemVal.maxVal = maxVal;
		itemVal.orgVal = orgVal;
		itemVal.type = type;
		m_ItemValueList.SetAt(hItem,itemVal);
	}
}
void CPPgTweaks::MinMaxInt(CDataExchange* pDX,int nIDC,  HTREEITEM hItem, int value, int minVal, int maxVal,int orgValue)
{
	if(value < minVal || value > maxVal)
	{
		CString text;
		text.Format(GetResString(IDS_EDIT_RANGER), minVal, maxVal);
		MessageBox(text,GetResString(IDS_CAPTION),MB_OK|MB_ICONWARNING);
		CString strOrg;
		strOrg.Format(_T("%d"),orgValue);
		DDX_TreeSetEdit(pDX,nIDC,hItem,strOrg);
		pDX->Fail();
	}
	else
	{
		m_ItemValueList.Lookup(hItem,m_ItemValue);
		CString strVal ;
		strVal.Format(_T("%d"),value);
		m_ItemValue.orgVal = strVal;
	}
		
}
void CPPgTweaks::MinMaxFloat(CDataExchange* pDX,int nIDC, HTREEITEM hItem,float value,float minVal,float maxVal,float orgVal)
{
	if (value < minVal || value > maxVal)
	{
		CString text;
		text.Format(GetResString(IDS_EDIT_RANGER),minVal,maxVal);
		MessageBox(text,GetResString(IDS_CAPTION),MB_OK|MB_ICONWARNING);
		CString strOrgValue;
		strOrgValue.Format(_T("%d"),orgVal);
		DDX_TreeSetEdit(pDX,nIDC,hItem,strOrgValue);
		pDX->Fail();
	}
	else
	{
		m_ItemValueList.Lookup(hItem,m_ItemValue);
		CString strVal ;
		strVal.Format(_T("%d"),value);
		m_ItemValue.orgVal = strVal;
	}
}
BOOL CPPgTweaks::OnInitDialog()
{
	/*说明：Tweaks对话框初始化时，检测系统是不是XPSP2，同时检测tcpip.sys的大小和版本，来确定是否修改
	  当条件满足时，eMule的最大半连接数，可以修改，并且该数字就为tcpip的最大连接数
	  
	*/
	//m_isXP = m_betterSP2.DetectSystemInformation();   //added by thilon on 2006.04.20 检测系统信息
	
	//if(m_isXP)
	//{
	//	m_iTCPIPInit = CGlobalVariable::GetTCPIPVaule();		//added by thilon on 2006.04.20 条件满足，直接赋值，这里通过启动eMule时，获得tcpip.sys文件里的连接数
	//	m_iMaxHalfOpen = m_iTCPIPInit;				//将获得的连接数直接赋值给最大半开连接数
	//}
	//else
	//{
	//	m_iMaxHalfOpen = thePrefs.GetMaxHalfConnections();
	//}

	m_iMaxSourceConnect = thePrefs.GetMaxSourceConnect();
	m_iOrgMaxSourceConnect = m_iMaxSourceConnect;
	m_iRetryNumber = thePrefs.GetRetryNumber();
	m_iOrgRetryNumber = m_iRetryNumber;
	m_iRetryDelay = thePrefs.GetRetryDelay();
	m_iOrgRetryDelay = m_iRetryDelay;
	m_iPublicMaxConnectLimit = thePrefs.GetPublicMaxConnectLimit();
	m_iOrgPublicMaxConnectLimit = m_iPublicMaxConnectLimit;
    

	m_iMaxConnPerFive = thePrefs.GetMaxConperFive();
	m_iOrgMaxConnPerFive = m_iMaxConnPerFive;
	m_bConditionalTCPAccept = thePrefs.GetConditionalTCPAccept();
	m_bAutoTakeEd2kLinks = HaveEd2kRegAccess() ? thePrefs.AutoTakeED2KLinks() : 0;

	if (thePrefs.GetEnableVerboseOptions())
	{
		m_bVerbose = thePrefs.m_bVerbose;
		m_bDebug2Disk = thePrefs.debug2disk;							// do *not* use the according 'Get...' function here!
		m_bDebugSourceExchange = thePrefs.m_bDebugSourceExchange;		// do *not* use the according 'Get...' function here!
		m_bLogBannedClients = thePrefs.m_bLogBannedClients;				// do *not* use the according 'Get...' function here!
		m_bLogRatingDescReceived = thePrefs.m_bLogRatingDescReceived;	// do *not* use the according 'Get...' function here!
		m_bLogSecureIdent = thePrefs.m_bLogSecureIdent;					// do *not* use the according 'Get...' function here!
		m_bLogFilteredIPs = thePrefs.m_bLogFilteredIPs;					// do *not* use the according 'Get...' function here!
		m_bLogFileSaving = thePrefs.m_bLogFileSaving;					// do *not* use the according 'Get...' function here!
        m_bLogA4AF = thePrefs.m_bLogA4AF;                   		    // do *not* use the according 'Get...' function here! // ZZ:DownloadManager
		m_bLogUlDlEvents = thePrefs.m_bLogUlDlEvents;
		m_iLogLevel = 5 - thePrefs.m_byLogLevel;
		m_iOrgLogLevel = m_iLogLevel;
	}
	m_bLog2Disk = thePrefs.log2disk;
	m_bCreditSystem = thePrefs.m_bCreditSystem;
	m_iCommitFiles = thePrefs.m_iCommitFiles;
	m_iExtractMetaData = thePrefs.m_iExtractMetaData;
	m_bFilterLANIPs = thePrefs.filterLANIPs;
	m_bExtControls = thePrefs.m_bExtControls;
	m_uServerKeepAliveTimeout = thePrefs.m_dwServerKeepAliveTimeout / 60000;
	m_uOrgServerKeepAliveTimeout = m_uServerKeepAliveTimeout;
	m_bSparsePartFiles = thePrefs.m_bSparsePartFiles;
	m_bFullAlloc= thePrefs.m_bAllocFull;
	m_bCheckDiskspace = thePrefs.checkDiskspace;
	m_fMinFreeDiskSpaceMB = (float)(thePrefs.m_uMinFreeDiskSpace / (1024.0 * 1024.0));
	m_fOrgMinFreeDiskSpaceMB = m_fMinFreeDiskSpaceMB;
	m_sYourHostname = thePrefs.GetYourHostname();
	m_sOrgYourHostname = m_sYourHostname;
	m_bFirewallStartup = ((thePrefs.GetWindowsVersion() == _WINVER_XP_) ? thePrefs.m_bOpenPortsOnStartUp : 0); 
	m_bDisablePeerCache = !thePrefs.m_bPeerCacheEnabled;
	m_bAutoArchDisable = !thePrefs.m_bAutomaticArcPreviewStart;

    m_bDynUpEnabled = thePrefs.m_bDynUpEnabled;
    m_iDynUpMinUpload = thePrefs.GetMinUpload();
	m_iOrgDynUpMinUpload = m_iDynUpMinUpload;
    m_iDynUpPingTolerance = thePrefs.GetDynUpPingTolerance();
	m_iOrgDynUpPingTolerance = m_iDynUpPingTolerance;
    m_iDynUpPingToleranceMilliseconds = thePrefs.GetDynUpPingToleranceMilliseconds();
	m_iOrgDynUpPingToleranceMilliseconds = m_iDynUpPingToleranceMilliseconds; 
    m_iDynUpRadioPingTolerance = thePrefs.IsDynUpUseMillisecondPingTolerance()?1:0;
    m_iDynUpGoingUpDivider = thePrefs.GetDynUpGoingUpDivider();
	m_iOrgDynUpGoingUpDivider = m_iDynUpGoingUpDivider;
    m_iDynUpGoingDownDivider = thePrefs.GetDynUpGoingDownDivider();
	m_iOrgDynUpGoingDownDivider = m_iDynUpGoingDownDivider;
    m_iDynUpNumberOfPings = thePrefs.GetDynUpNumberOfPings();
    m_iOrgDynUpNumberOfPings = m_iDynUpNumberOfPings;

	//EastShare Start - added by AndCycle, IP to Country
	m_iIP2CountryName = thePrefs.GetIP2CountryNameMode(); 
	m_bIP2CountryShowFlag = thePrefs.IsIP2CountryShowFlag();
	//EastShare End - added by AndCycle, IP to Country

	//Added by thilon on 2006.08.08, 固定上传线程
	m_uUploadClients = thePrefs.m_uUploadClients;
    m_uOrgUploadClients = m_uUploadClients;
	//upnp_start - Added by thilon on 2006.09.24, for UPnP
	//m_iUPnPNat = thePrefs.GetUPnPNat();
	//m_iUPnPTryRandom = thePrefs.GetUPnPNatTryRandom();
	//upnp_end

	m_iShareeMule = thePrefs.m_nCurrentUserDirMode;		//Added by thilon on 2007.05.25

    m_bA4AFSaveCpu = thePrefs.GetA4AFSaveCpu();

	m_ctrlTreeOptions.SetImageListColorFlags(theApp.m_iDfltImageListColorFlags);
    CPropertyPage::OnInitDialog();
	InitWindowStyles(this);
	m_ctrlTreeOptions.SetItemHeight(m_ctrlTreeOptions.GetItemHeight() + 2);

	m_iFileBufferSize = thePrefs.m_iFileBufferSize;
#ifdef _SUPPORT_MEMPOOL
	m_ctlFileBuffSize.SetRange(1, 1024 /*+ 512*/, TRUE); // Change 1024 + 512 to 1024 and Change 16 to 1 SearchDream@2006/12/21
#else
	m_ctlFileBuffSize.SetRange(16, 1024 + 512, TRUE); 
#endif
	
	int iMin, iMax;
	m_ctlFileBuffSize.GetRange(iMin, iMax);
#ifdef _SUPPORT_MEMPOOL
	m_ctlFileBuffSize.SetPos(m_iFileBufferSize/(1024 * 32)); // Change 1024 to 1024 * 32 SearchDream@2006/01/10
#else
	m_ctlFileBuffSize.SetPos(m_iFileBufferSize/(1024));
#endif
	
	int iPage = 128;
	for (int i = ((iMin+iPage-1)/iPage)*iPage; i < iMax; i += iPage)
		m_ctlFileBuffSize.SetTic(i);

	m_ctlFileBuffSize.SetPageSize(iPage);

	m_iQueueSize = thePrefs.m_iQueueSize;
	m_ctlQueueSize.SetRange(20, 100, TRUE);
	m_ctlQueueSize.SetPos(m_iQueueSize/100);
	m_ctlQueueSize.SetTicFreq(10);
	m_ctlQueueSize.SetPageSize(10);

	Localize();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPPgTweaks::OnKillActive()
{
	// if prop page is closed by pressing ENTER we have to explicitly commit any possibly pending
	// data from an open edit control
	m_ctrlTreeOptions.HandleChildControlLosingFocus();
	return CPropertyPage::OnKillActive();
}
void CPPgTweaks::ModifyItemValue(HTREEITEM hItem,UINT uVal)
{
	m_ItemValueList.Lookup(hItem,m_ItemValue);
	CString strOrg = m_ItemValue.orgVal;
	UINT uOrg = _wtoi(strOrg);
	if (uOrg != uVal)
	{
		CString strNew;
		strNew.Format(_T("%d"),uVal);
		m_ItemValue.orgVal = strNew;
		m_ItemValueList.RemoveKey(hItem);
		m_ItemValueList.SetAt(hItem,m_ItemValue);
	}
}
void CPPgTweaks::ModifyItemValue(HTREEITEM hItem,int iVal)
{
    m_ItemValueList.Lookup(hItem,m_ItemValue);
    CString strOrg = m_ItemValue.orgVal;
	int iOrg = _wtoi(strOrg);
	if (iOrg != iVal)
	{
		CString strNew;
		strNew.Format(_T("%d"),iVal);
		m_ItemValue.orgVal = strNew;
		m_ItemValueList.RemoveKey(hItem);
		m_ItemValueList.SetAt(hItem,m_ItemValue);
	}
}
void CPPgTweaks::ModifyItemValue(HTREEITEM hItem,float fVal)
{
	m_ItemValueList.Lookup(hItem,m_ItemValue);
	CString strOrg = m_ItemValue.orgVal;
	double fOrg = _wtof(strOrg);
	if (fOrg != fVal)
	{
		CString strNew;
		strNew.Format(_T("%1.0f"),fVal);
		m_ItemValue.orgVal = strNew;
		m_ItemValueList.RemoveKey(hItem);
		m_ItemValueList.SetAt(hItem,m_ItemValue);
	}
}
void CPPgTweaks::ModifyItemValue(HTREEITEM hItem,CString strVal)
{
	m_ItemValueList.Lookup(hItem,m_ItemValue);
	CString strOrg = m_ItemValue.orgVal;
	if (strOrg != strVal)
	{
		m_ItemValue.orgVal = strVal;
		m_ItemValueList.RemoveKey(hItem);
		m_ItemValueList.SetAt(hItem,m_ItemValue);
	}
}
BOOL CPPgTweaks::OnApply()
{
	// if prop page is closed by pressing ENTER we have to explicitly commit any possibly pending
	// data from an open edit control
	m_ctrlTreeOptions.HandleChildControlLosingFocus();

	if (!UpdateData())
		return FALSE;
  
	thePrefs.SetMaxSourceConnect(m_iMaxSourceConnect);
	ModifyItemValue(m_htiMaxSourceConnect,m_iMaxSourceConnect);

    thePrefs.SetRetryNumbe(m_iRetryNumber);
	ModifyItemValue(m_htiRetryNumber,m_iRetryNumber);

	thePrefs.SetRetryDelay(m_iRetryDelay);
	ModifyItemValue(m_htiRetryDelay,m_iRetryDelay);

	thePrefs.SetPublicMaxConnectLimit(m_iPublicMaxConnectLimit);
    ModifyItemValue(m_htiPublicMaxConnectLimit,m_iPublicMaxConnectLimit);

	thePrefs.SetMaxConsPerFive(m_iMaxConnPerFive ? m_iMaxConnPerFive : DFLT_MAXCONPERFIVE);
	ModifyItemValue(m_htiMaxCon5Sec,m_iMaxConnPerFive);
	theApp.scheduler->original_cons5s = thePrefs.GetMaxConperFive();
	//thePrefs.SetMaxHalfConnections(m_iMaxHalfOpen ? m_iMaxHalfOpen : DFLT_MAXHALFOPEN);
	thePrefs.m_bConditionalTCPAccept = m_bConditionalTCPAccept;

	if (HaveEd2kRegAccess() && thePrefs.AutoTakeED2KLinks() != m_bAutoTakeEd2kLinks)
	{
		thePrefs.autotakeed2klinks = m_bAutoTakeEd2kLinks;
		if (thePrefs.AutoTakeED2KLinks())
			Ask4RegFix(false, true, false);
		else
			RevertReg();
	}

	if (!thePrefs.log2disk && m_bLog2Disk)
		theLog.Open();
	else if (thePrefs.log2disk && !m_bLog2Disk)
		theLog.Close();
	thePrefs.log2disk = m_bLog2Disk;

	if (thePrefs.GetEnableVerboseOptions())
	{
		if (!thePrefs.GetDebug2Disk() && m_bVerbose && m_bDebug2Disk)
			theVerboseLog.Open();
		else if (thePrefs.GetDebug2Disk() && (!m_bVerbose || !m_bDebug2Disk))
			theVerboseLog.Close();
		thePrefs.debug2disk = m_bDebug2Disk;

		thePrefs.m_bDebugSourceExchange = m_bDebugSourceExchange;
		thePrefs.m_bLogBannedClients = m_bLogBannedClients;
		thePrefs.m_bLogRatingDescReceived = m_bLogRatingDescReceived;
		thePrefs.m_bLogSecureIdent = m_bLogSecureIdent;
		thePrefs.m_bLogFilteredIPs = m_bLogFilteredIPs;
		thePrefs.m_bLogFileSaving = m_bLogFileSaving;
        thePrefs.m_bLogA4AF = m_bLogA4AF;
		thePrefs.m_bLogUlDlEvents = m_bLogUlDlEvents;
		thePrefs.m_byLogLevel = 5 - m_iLogLevel;
		ModifyItemValue(m_htiLogLevel,m_iLogLevel);

		thePrefs.m_bVerbose = m_bVerbose; // store after related options were stored!
	}

	thePrefs.m_bCreditSystem = m_bCreditSystem;
	thePrefs.m_iCommitFiles = m_iCommitFiles;
	thePrefs.m_iExtractMetaData = m_iExtractMetaData;
	thePrefs.filterLANIPs = m_bFilterLANIPs;
	thePrefs.m_iFileBufferSize = m_iFileBufferSize;
	thePrefs.m_iQueueSize = m_iQueueSize;
	if (thePrefs.m_bExtControls != m_bExtControls) {
		thePrefs.m_bExtControls = m_bExtControls;
		theApp.emuledlg->transferwnd->downloadlistctrl.CreateMenues();
		theApp.emuledlg->searchwnd->CreateMenus();
		theApp.emuledlg->sharedfileswnd->sharedfilesctrl.CreateMenues();
	}
	thePrefs.m_dwServerKeepAliveTimeout = m_uServerKeepAliveTimeout * 60000;
	ModifyItemValue(m_htiServerKeepAliveTimeout,m_uServerKeepAliveTimeout);

	thePrefs.m_bSparsePartFiles = m_bSparsePartFiles;
	thePrefs.m_bAllocFull= m_bFullAlloc;
	thePrefs.checkDiskspace = m_bCheckDiskspace;
	thePrefs.m_uMinFreeDiskSpace = (UINT)(m_fMinFreeDiskSpaceMB * (1024 * 1024));
	ModifyItemValue(m_htiMinFreeDiskSpace,m_fMinFreeDiskSpaceMB);
	if (thePrefs.GetYourHostname() != m_sYourHostname) {
		thePrefs.SetYourHostname(m_sYourHostname);
		ModifyItemValue(m_htiYourHostname,m_sYourHostname);
		theApp.emuledlg->serverwnd->UpdateMyInfo();
	}
	thePrefs.m_bOpenPortsOnStartUp = m_bFirewallStartup;
	thePrefs.m_bPeerCacheEnabled = !m_bDisablePeerCache;

	//upnp_start - Added by thilon on 2006.09.24, for UPnP
/*
	if(thePrefs.GetUPnPNat()!=m_iUPnPNat || thePrefs.GetUPnPNatTryRandom() != m_iUPnPTryRandom)
	{
		AfxMessageBox(GetResString(IDS_UPNP_MESSAGE));
	}
	thePrefs.SetUPnPNat( m_iUPnPNat );
	thePrefs.SetUPnPNatTryRandom( m_iUPnPTryRandom );*/
	//upnp_end

    thePrefs.m_bDynUpEnabled = m_bDynUpEnabled;
    thePrefs.minupload = (uint16)m_iDynUpMinUpload;
	ModifyItemValue(m_htiDynUpMinUpload,m_iDynUpMinUpload);
    thePrefs.m_iDynUpPingTolerance = m_iDynUpPingTolerance;
	ModifyItemValue(m_htiDynUpPingTolerance,m_iDynUpPingTolerance);
    thePrefs.m_iDynUpPingToleranceMilliseconds = m_iDynUpPingToleranceMilliseconds;
	ModifyItemValue(m_htiDynUpPingToleranceMilliseconds,m_iDynUpPingToleranceMilliseconds);
    thePrefs.m_bDynUpUseMillisecondPingTolerance = (m_iDynUpRadioPingTolerance == 1);
    thePrefs.m_iDynUpGoingUpDivider = m_iDynUpGoingUpDivider;
	ModifyItemValue(m_htiDynUpGoingUpDivider,m_iDynUpGoingUpDivider);
    thePrefs.m_iDynUpGoingDownDivider = m_iDynUpGoingDownDivider;
	ModifyItemValue(m_htiDynUpGoingDownDivider,m_iDynUpGoingDownDivider);
    thePrefs.m_iDynUpNumberOfPings = m_iDynUpNumberOfPings;
	ModifyItemValue(m_htiDynUpNumberOfPings,m_iDynUpNumberOfPings);
	thePrefs.m_bAutomaticArcPreviewStart = !m_bAutoArchDisable;
	
	thePrefs.ChangeUserDirMode(m_iShareeMule);	//Added by thilon on 2007.05.25
	
	//added by Chocobo
	//国家名称显示相关
	//2006.07.28
	//EastShare Start - added by AndCycle, IP to Country
	thePrefs.m_iIP2CountryNameMode = (IP2CountryNameSelection)m_iIP2CountryName;
	thePrefs.m_bIP2CountryShowFlag = m_bIP2CountryShowFlag != 0;
	CGlobalVariable::ip2country->Load();
	CGlobalVariable::ip2country->Refresh();//refresh passive windows
	//EastShare End - added by AndCycle, IP to Country

	thePrefs.m_uUploadClients = m_uUploadClients;//Added by thilon on 2006.08.08, 固定上传线程
	ModifyItemValue(m_htiUploadClients,m_uUploadClients);
    thePrefs.m_bA4AFSaveCpu = m_bA4AFSaveCpu;

	if (thePrefs.GetEnableVerboseOptions())
	{
	    theApp.emuledlg->serverwnd->ToggleDebugWindow();
		theApp.emuledlg->serverwnd->UpdateLogTabSelection();
	}
	CGlobalVariable::downloadqueue->CheckDiskspace();

	// 添加修改XP连接数 added by thilo at 2006.40.20

    /*if(m_isXP)
	{
		if(m_iTCPIPInit != m_iMaxHalfOpen)
		{
			if(m_betterSP2.ChangeTCPIPValue(m_iMaxHalfOpen))
			{
				::MessageBox(this->m_hWnd, GetResString(IDS_TCPIPCHANGED), _T("eMule"), MB_OK);
				m_iTCPIPInit = m_iMaxHalfOpen;
			}
		}
	}
	else
	{
		thePrefs.SetMaxHalfConnections(m_iMaxHalfOpen ? m_iMaxHalfOpen : DFLT_MAXHALFOPEN);
	}*/
	
	SetModified(FALSE);
	return CPropertyPage::OnApply();
}

void CPPgTweaks::OnHScroll(UINT /*nSBCode*/, UINT /*nPos*/, CScrollBar* pScrollBar) 
{
	if (pScrollBar->GetSafeHwnd() == m_ctlFileBuffSize.m_hWnd)
	{
#ifdef _SUPPORT_MEMPOOL
		m_iFileBufferSize = m_ctlFileBuffSize.GetPos() * 1024 * 32; // Add * 32 SearchDream@2006/01/10
#else
		m_iFileBufferSize = m_ctlFileBuffSize.GetPos() * 1024; 
#endif
		
        CString temp;
		temp.Format(_T("%s: %s"), GetResString(IDS_FILEBUFFERSIZE), CastItoXBytes(m_iFileBufferSize, false, false));
		GetDlgItem(IDC_FILEBUFFERSIZE_STATIC)->SetWindowText(temp);
		SetModified(TRUE);
	}
	else if (pScrollBar->GetSafeHwnd() == m_ctlQueueSize.m_hWnd)
	{
		m_iQueueSize = ((CSliderCtrl*)pScrollBar)->GetPos() * 100;
		CString temp;
		temp.Format(_T("%s: %s"), GetResString(IDS_QUEUESIZE), GetFormatedUInt(m_iQueueSize));
		GetDlgItem(IDC_QUEUESIZE_STATIC)->SetWindowText(temp);
		SetModified(TRUE);
	}
}

void CPPgTweaks::Localize(void)
{	
	if(m_hWnd)
	{
		SetWindowText(GetResString(IDS_PW_TWEAK));
		GetDlgItem(IDC_WARNING)->SetWindowText(GetResString(IDS_TWEAKS_WARNING));

		if (m_htiConnectLimitGroup){m_ctrlTreeOptions.SetItemText(m_htiConnectLimitGroup, GetResString(IDS_LIMIT_GROUP));}
		if (m_htiMaxSourceConnect) m_ctrlTreeOptions.SetEditLabel(m_htiMaxSourceConnect, GetResString(IDS_LIMIT_SOURCE_CONNS));
		if (m_htiRetryNumber) m_ctrlTreeOptions.SetEditLabel(m_htiRetryNumber, GetResString(IDS_LIMIT_RETRY_NUMBER));
		if (m_htiRetryDelay) m_ctrlTreeOptions.SetEditLabel(m_htiRetryDelay, GetResString(IDS_LIMIT_RETRY_DELAY));
        if (m_htiPublicMaxConnectLimit) m_ctrlTreeOptions.SetEditLabel(m_htiPublicMaxConnectLimit,GetResString(IDS_PUB_CONNECT_LIMIT));


		if (m_htiTCPGroup) m_ctrlTreeOptions.SetItemText(m_htiTCPGroup, GetResString(IDS_TCPIP_CONNS));
		if (m_htiMaxCon5Sec) m_ctrlTreeOptions.SetEditLabel(m_htiMaxCon5Sec, GetResString(IDS_MAXCON5SECLABEL));
		if (m_htiMaxHalfOpen) m_ctrlTreeOptions.SetEditLabel(m_htiMaxHalfOpen, GetResString(IDS_MAXHALFOPENCONS));
		if (m_htiConditionalTCPAccept) m_ctrlTreeOptions.SetItemText(m_htiConditionalTCPAccept, GetResString(IDS_CONDTCPACCEPT));
		if (m_htiAutoTakeEd2kLinks) m_ctrlTreeOptions.SetItemText(m_htiAutoTakeEd2kLinks, GetResString(IDS_AUTOTAKEED2KLINKS));
		if (m_htiCreditSystem) m_ctrlTreeOptions.SetItemText(m_htiCreditSystem, GetResString(IDS_USECREDITSYSTEM));
		if (m_htiLog2Disk) m_ctrlTreeOptions.SetItemText(m_htiLog2Disk, GetResString(IDS_LOG2DISK));
		if (m_htiVerboseGroup) m_ctrlTreeOptions.SetItemText(m_htiVerboseGroup, GetResString(IDS_VERBOSE));
		if (m_htiVerbose) m_ctrlTreeOptions.SetItemText(m_htiVerbose, GetResString(IDS_ENABLED));
		if (m_htiDebug2Disk) m_ctrlTreeOptions.SetItemText(m_htiDebug2Disk, GetResString(IDS_LOG2DISK));
		if (m_htiDebugSourceExchange) m_ctrlTreeOptions.SetItemText(m_htiDebugSourceExchange, GetResString(IDS_DEBUG_SOURCE_EXCHANGE));
		if (m_htiLogBannedClients) m_ctrlTreeOptions.SetItemText(m_htiLogBannedClients, GetResString(IDS_LOG_BANNED_CLIENTS));
		if (m_htiLogRatingDescReceived) m_ctrlTreeOptions.SetItemText(m_htiLogRatingDescReceived, GetResString(IDS_LOG_RATING_RECV));
		if (m_htiLogSecureIdent) m_ctrlTreeOptions.SetItemText(m_htiLogSecureIdent, GetResString(IDS_LOG_SECURE_IDENT));
		if (m_htiLogFilteredIPs) m_ctrlTreeOptions.SetItemText(m_htiLogFilteredIPs, GetResString(IDS_LOG_FILTERED_IPS));
		if (m_htiLogFileSaving) m_ctrlTreeOptions.SetItemText(m_htiLogFileSaving, GetResString(IDS_LOG_FILE_SAVING));
		if (m_htiLogLevel) m_ctrlTreeOptions.SetEditLabel(m_htiLogLevel, GetResString(IDS_LOG_LEVEL));
		if (m_htiLogA4AF) m_ctrlTreeOptions.SetItemText(m_htiLogA4AF, GetResString(IDS_LOG_A4AF));
		if (m_htiLogUlDlEvents) m_ctrlTreeOptions.SetItemText(m_htiLogUlDlEvents, GetResString(IDS_LOG_ULDL_EVENTS));
		if (m_htiCommit) m_ctrlTreeOptions.SetItemText(m_htiCommit, GetResString(IDS_COMMITFILES));
		if (m_htiCommitNever) m_ctrlTreeOptions.SetItemText(m_htiCommitNever, GetResString(IDS_NEVER));
		if (m_htiCommitOnShutdown) m_ctrlTreeOptions.SetItemText(m_htiCommitOnShutdown, GetResString(IDS_ONSHUTDOWN));
		if (m_htiCommitAlways) m_ctrlTreeOptions.SetItemText(m_htiCommitAlways, GetResString(IDS_ALWAYS));
		if (m_htiExtractMetaData) m_ctrlTreeOptions.SetItemText(m_htiExtractMetaData, GetResString(IDS_EXTRACT_META_DATA));
		if (m_htiExtractMetaDataNever) m_ctrlTreeOptions.SetItemText(m_htiExtractMetaDataNever, GetResString(IDS_NEVER));
		if (m_htiExtractMetaDataID3Lib) m_ctrlTreeOptions.SetItemText(m_htiExtractMetaDataID3Lib, GetResString(IDS_META_DATA_ID3LIB));
		//if (m_htiExtractMetaDataMediaDet) m_ctrlTreeOptions.SetItemText(m_htiExtractMetaDataMediaDet, GetResString(IDS_META_DATA_MEDIADET));
		if (m_htiFilterLANIPs) m_ctrlTreeOptions.SetItemText(m_htiFilterLANIPs, GetResString(IDS_PW_FILTER));
		if (m_htiExtControls) m_ctrlTreeOptions.SetItemText(m_htiExtControls, GetResString(IDS_SHOWEXTSETTINGS));
		if (m_htiServerKeepAliveTimeout) m_ctrlTreeOptions.SetEditLabel(m_htiServerKeepAliveTimeout, GetResString(IDS_SERVERKEEPALIVETIMEOUT));
		if (m_htiSparsePartFiles) m_ctrlTreeOptions.SetItemText(m_htiSparsePartFiles, GetResString(IDS_SPARSEPARTFILES));
		if (m_htiCheckDiskspace) m_ctrlTreeOptions.SetItemText(m_htiCheckDiskspace, GetResString(IDS_CHECKDISKSPACE));
		if (m_htiMinFreeDiskSpace) m_ctrlTreeOptions.SetEditLabel(m_htiMinFreeDiskSpace, GetResString(IDS_MINFREEDISKSPACE));
		if (m_htiYourHostname) m_ctrlTreeOptions.SetEditLabel(m_htiYourHostname, GetResString(IDS_YOURHOSTNAME));	// itsonlyme: hostnameSource
		if (m_htiFirewallStartup) m_ctrlTreeOptions.SetItemText(m_htiFirewallStartup, GetResString(IDS_FO_PREF_STARTUP));
		if (m_htiDisablePeerCache) m_ctrlTreeOptions.SetItemText(m_htiDisablePeerCache, GetResString(IDS_DISABLEPEERACHE));
        if (m_htiDynUp) m_ctrlTreeOptions.SetItemText(m_htiDynUp, GetResString(IDS_DYNUP));
		if (m_htiDynUpEnabled) m_ctrlTreeOptions.SetItemText(m_htiDynUpEnabled, GetResString(IDS_DYNUPENABLED));
        if (m_htiDynUpMinUpload) m_ctrlTreeOptions.SetEditLabel(m_htiDynUpMinUpload, GetResString(IDS_DYNUP_MINUPLOAD));

        if (m_htiDynUpPingTolerance) m_ctrlTreeOptions.SetEditLabel(m_htiDynUpPingTolerance, GetResString(IDS_DYNUP_PINGTOLERANCE));
		if (m_htiDynUpPingToleranceMilliseconds) m_ctrlTreeOptions.SetEditLabel(m_htiDynUpPingToleranceMilliseconds,GetResString(IDS_DYNUP_PINGTOLERANCE_MS));
		if (m_htiDynUpPingToleranceGroup) m_ctrlTreeOptions.SetItemText(m_htiDynUpPingToleranceGroup,GetResString(IDS_DYNUP_RADIO_PINGTOLERANCE_HEADER));
		if (m_htiDynUpRadioPingTolerance) m_ctrlTreeOptions.SetItemText(m_htiDynUpRadioPingTolerance,GetResString(IDS_DYNUP_RADIO_PINGTOLERANCE_PERCENT));
		if (m_htiDynUpRadioPingToleranceMilliseconds) m_ctrlTreeOptions.SetItemText(m_htiDynUpRadioPingToleranceMilliseconds,GetResString(IDS_DYNUP_RADIO_PINGTOLERANCE_MS));

        if (m_htiDynUpGoingUpDivider) m_ctrlTreeOptions.SetEditLabel(m_htiDynUpGoingUpDivider, GetResString(IDS_DYNUP_GOINGUPDIVIDER));
        if (m_htiDynUpGoingDownDivider) m_ctrlTreeOptions.SetEditLabel(m_htiDynUpGoingDownDivider, GetResString(IDS_DYNUP_GOINGDOWNDIVIDER));
        if (m_htiDynUpNumberOfPings) m_ctrlTreeOptions.SetEditLabel(m_htiDynUpNumberOfPings, GetResString(IDS_DYNUP_NUMBEROFPINGS));
        if (m_htiA4AFSaveCpu) m_ctrlTreeOptions.SetItemText(m_htiA4AFSaveCpu, GetResString(IDS_A4AF_SAVE_CPU));
        if (m_htiFullAlloc) m_ctrlTreeOptions.SetItemText(m_htiFullAlloc, GetResString(IDS_FULLALLOC));
		if (m_htiAutoArch) m_ctrlTreeOptions.SetItemText(m_htiAutoArch, GetResString(IDS_DISABLE_AUTOARCHPREV));
		if (m_htiUploadClients) m_ctrlTreeOptions.SetEditLabel(m_htiUploadClients, GetResString(IDS_UPLOADCLIENTS));	//Added by thilon on 2006.08.08, 固定上传线程

		//upnp_start - Added by thilon on 2006.09.24, for UPnP
		//if (m_htiUPnPNat) m_ctrlTreeOptions.SetItemText(m_htiUPnPNat, GetResString(IDS_CN_UPNPNAT));
		//if (m_htiUPnPTryRandom) m_ctrlTreeOptions.SetItemText(m_htiUPnPTryRandom, GetResString(IDS_CN_UPNPTRYRANDOM));
		//upnp_end

		if (m_htiShareeMule) m_ctrlTreeOptions.SetItemText(m_htiShareeMule, GetResString(IDS_SHAREEMULELABEL));
		if (m_htiShareeMuleMultiUser) m_ctrlTreeOptions.SetItemText(m_htiShareeMuleMultiUser, GetResString(IDS_SHAREEMULEMULTI));
		if (m_htiShareeMulePublicUser) m_ctrlTreeOptions.SetItemText(m_htiShareeMulePublicUser, GetResString(IDS_SHAREEMULEPUBLIC));
		if (m_htiShareeMuleOldStyle) m_ctrlTreeOptions.SetItemText(m_htiShareeMuleOldStyle, GetResString(IDS_SHAREEMULEOLD));

        CString temp;
		temp.Format(_T("%s: %s"), GetResString(IDS_FILEBUFFERSIZE), CastItoXBytes(m_iFileBufferSize, false, false));
		GetDlgItem(IDC_FILEBUFFERSIZE_STATIC)->SetWindowText(temp);
		temp.Format(_T("%s: %s"), GetResString(IDS_QUEUESIZE), GetFormatedUInt(m_iQueueSize));
		GetDlgItem(IDC_QUEUESIZE_STATIC)->SetWindowText(temp);


		//EastShare Start - added by AndCycle, IP to Country
		if(m_htiIP2CountryName) m_ctrlTreeOptions.SetItemText(m_htiIP2CountryName,GetResString(IDS_IP2LOCATION));
		if(m_htiIP2CountryName_DISABLE) m_ctrlTreeOptions.SetItemText(m_htiIP2CountryName_DISABLE,GetResString(IDS_DISABLED));
		if(m_htiIP2CountryName_SHORT) m_ctrlTreeOptions.SetItemText(m_htiIP2CountryName_SHORT,GetResString(IDS_LOCATIONNAME_SHORT));
		if(m_htiIP2CountryName_MID) m_ctrlTreeOptions.SetItemText(m_htiIP2CountryName_MID,GetResString(IDS_LOCATIONNAME_MID));
		if(m_htiIP2CountryName_LONG) m_ctrlTreeOptions.SetItemText(m_htiIP2CountryName_LONG,GetResString(IDS_LOCATIONNAME_LONG));
		if(m_htiIP2CountryShowFlag) m_ctrlTreeOptions.SetItemText(m_htiIP2CountryShowFlag,GetResString(IDS_LOCATIONNAME_SHOWFLAG));
		
		

	}
}

void CPPgTweaks::OnDestroy()
{
	m_ctrlTreeOptions.DeleteAllItems();
	m_ctrlTreeOptions.DestroyWindow();
	m_bInitializedTreeOpts = false;

	m_htiConnectLimitGroup = NULL;
	m_htiMaxSourceConnect = NULL;
	m_htiRetryNumber = NULL;
	m_htiRetryDelay = NULL;
	m_htiPublicMaxConnectLimit =NULL;

	m_htiTCPGroup = NULL;
	m_htiMaxCon5Sec = NULL;
	m_htiMaxHalfOpen = NULL;
	m_htiConditionalTCPAccept = NULL;
	m_htiAutoTakeEd2kLinks = NULL;
	m_htiVerboseGroup = NULL;
	m_htiVerbose = NULL;
	m_htiDebugSourceExchange = NULL;
	m_htiLogBannedClients = NULL;
	m_htiLogRatingDescReceived = NULL;
	m_htiLogSecureIdent = NULL;
	m_htiLogFilteredIPs = NULL;
	m_htiLogFileSaving = NULL;
    m_htiLogA4AF = NULL;
	m_htiLogLevel = NULL;
	m_htiLogUlDlEvents = NULL;
	m_htiCreditSystem = NULL;
	m_htiLog2Disk = NULL;
	m_htiDebug2Disk = NULL;
	m_htiCommit = NULL;
	m_htiCommitNever = NULL;
	m_htiCommitOnShutdown = NULL;
	m_htiCommitAlways = NULL;
	m_htiFilterLANIPs = NULL;
	m_htiExtControls = NULL;
	m_htiServerKeepAliveTimeout = NULL;
	m_htiSparsePartFiles = NULL;
	m_htiFullAlloc = NULL;
	m_htiCheckDiskspace = NULL;
	m_htiMinFreeDiskSpace = NULL;
	m_htiYourHostname = NULL;
	m_htiFirewallStartup = NULL;
	m_htiDisablePeerCache = NULL;
    m_htiDynUp = NULL;
	m_htiDynUpEnabled = NULL;
    m_htiDynUpMinUpload = NULL;
    m_htiDynUpPingTolerance = NULL;
    m_htiDynUpPingToleranceMilliseconds = NULL;
    m_htiDynUpPingToleranceGroup = NULL;
    m_htiDynUpRadioPingTolerance = NULL;
    m_htiDynUpRadioPingToleranceMilliseconds = NULL;
    m_htiDynUpGoingUpDivider = NULL;
    m_htiDynUpGoingDownDivider = NULL;
    m_htiDynUpNumberOfPings = NULL;
    m_htiA4AFSaveCpu = NULL;
	m_htiExtractMetaData = NULL;
	m_htiExtractMetaDataNever = NULL;
	m_htiExtractMetaDataID3Lib = NULL;
	m_htiAutoArch = NULL;
	//m_htiExtractMetaDataMediaDet = NULL;
   
	//EastShare Start - added by AndCycle, IP to Country
	m_htiIP2CountryName = NULL;
	m_htiIP2CountryName_DISABLE = NULL;
	m_htiIP2CountryName_SHORT = NULL;
	m_htiIP2CountryName_MID = NULL;
	m_htiIP2CountryName_LONG = NULL;
	m_htiIP2CountryShowFlag = NULL;
	//EastShare End - added by AndCycle, IP to Country
	
	//Added by thilon on 2006.08.08, 固定上传线程
	m_htiUploadClients = NULL;

	//upnp_start - Added by thilon on 2006.09.24, for UPnP
/*
	m_htiUPnPNat = NULL;
	m_htiUPnPTryRandom = NULL;
	m_iUPnPNat = 0;
	m_iUPnPTryRandom = 0;
*/
	//upnp_end

	//Added by thilon on 2007.05.25
	m_htiShareeMule = NULL;
	m_htiShareeMuleMultiUser = NULL;
	m_htiShareeMulePublicUser = NULL;
	m_htiShareeMuleOldStyle = NULL;

    CPropertyPage::OnDestroy();
}

LRESULT CPPgTweaks::OnTreeOptsCtrlNotify(WPARAM wParam, LPARAM lParam)
{
	if (wParam == IDC_EXT_OPTS)
	{
		TREEOPTSCTRLNOTIFY* pton = (TREEOPTSCTRLNOTIFY*)lParam;
		if (m_htiVerbose && pton->hItem == m_htiVerbose)
		{
			BOOL bCheck;
			if (m_ctrlTreeOptions.GetCheckBox(m_htiVerbose, bCheck))
			{
				if (m_htiDebug2Disk)			m_ctrlTreeOptions.SetCheckBoxEnable(m_htiDebug2Disk, bCheck);
				if (m_htiDebugSourceExchange)	m_ctrlTreeOptions.SetCheckBoxEnable(m_htiDebugSourceExchange, bCheck);
				if (m_htiLogBannedClients)		m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogBannedClients, bCheck);
				if (m_htiLogRatingDescReceived) m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogRatingDescReceived, bCheck);
				if (m_htiLogSecureIdent)		m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogSecureIdent, bCheck);
				if (m_htiLogFilteredIPs)		m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogFilteredIPs, bCheck);
				if (m_htiLogFileSaving)			m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogFileSaving, bCheck);
                if (m_htiLogA4AF)			    m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogA4AF, bCheck);
				if (m_htiLogUlDlEvents)			m_ctrlTreeOptions.SetCheckBoxEnable(m_htiLogUlDlEvents, bCheck);
			}
		}
		else if ((m_htiShareeMuleMultiUser && pton->hItem == m_htiShareeMuleMultiUser)
			|| (m_htiShareeMulePublicUser && pton->hItem == m_htiShareeMulePublicUser)
			|| (m_htiShareeMuleOldStyle && pton->hItem == m_htiShareeMuleOldStyle))
		{
			if (m_htiShareeMule && !bShowedWarning)
			{
				HTREEITEM tmp;
				int nIndex;
				m_ctrlTreeOptions.GetRadioButton(m_htiShareeMule, nIndex, tmp);
				if (nIndex != thePrefs.m_nCurrentUserDirMode)
				{
					// TODO offer cancel option
					MessageBox(GetResString(IDS_SHAREEMULEWARNING),GetResString(IDS_CAPTION),MB_OK|MB_ICONWARNING);
					bShowedWarning = true;
				}
			}
		}

		//Added by thilon on 2006.08.07
		if(m_htiDynUp && pton->hItem == m_htiDynUpEnabled )
		{
			BOOL bCheck;
			if(m_ctrlTreeOptions.GetCheckBox(m_htiDynUpEnabled, bCheck))
			{
				if(bCheck)
				{
					if(IDOK == MessageBox(GetResString(IDS_USSCHANGED), GetResString(IDS_CAPTION), MB_OKCANCEL))
					{
						if(m_htiDynUpEnabled) m_ctrlTreeOptions.SetCheckBox(m_htiDynUpEnabled,bCheck);
					}
					else
					{
						if(m_htiDynUpEnabled) m_ctrlTreeOptions.SetCheckBox(m_htiDynUpEnabled, !bCheck);
					}
				}
			}
		 }

		//upnp_start - Added by thilon on 2006.09.24, for UPnP
/*
		if (m_htiUPnPNat && pton->hItem == m_htiUPnPNat)
		{
			BOOL bCheck;
			if (m_ctrlTreeOptions.GetCheckBox(m_htiUPnPNat, bCheck))
			{
				if (m_htiUPnPTryRandom)	m_ctrlTreeOptions.SetCheckBoxEnable(m_htiUPnPTryRandom, bCheck);
			}
		}
*/
		//upnp_end

		SetModified();
	}
	return 0;
}

void CPPgTweaks::OnHelp()
{
	theApp.ShowHelp(eMule_FAQ_Preferences_Extended_Settings);
}

BOOL CPPgTweaks::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_HELP)
	{
		OnHelp();
		return TRUE;
	}
	return __super::OnCommand(wParam, lParam);
}

BOOL CPPgTweaks::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	OnHelp();
	return TRUE;
}

