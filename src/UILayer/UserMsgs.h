/*
 * $Id: UserMsgs.h 5411 2008-04-29 08:21:19Z thilon $
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

// ALL WM_USER messages are to be declared here *after* 'WM_FIRST_EMULE_USER_MSG'
enum EUserWndMessages
{
	// *) Do *NOT* place any message *before* WM_FIRST_EMULE_USER_MSG !!
	// *) Do *NOT* use any WM_USER messages in the range WM_USER - WM_USER+0x100 !!
	UM_FIRST_EMULE_USER_MSG = (WM_USER + 0x100 + 1),

	// Taskbar
	UM_TASKBARNOTIFIERCLICKED,
	UM_TRAY_ICON_NOTIFY_MESSAGE,
	UM_CLOSE_MINIMULE,

	// Webserver
	WEB_GUI_INTERACTION,
	WEB_CLEAR_COMPLETED,
	WEB_FILE_RENAME,
	WEB_ADDDOWNLOADS,
	WEB_CATPRIO,
	WEB_ADDREMOVEFRIEND,

	// VC
	UM_VERSIONCHECK_RESPONSE,

	// PC
	UM_PEERCHACHE_RESPONSE,

	UM_CLOSETAB,
	UM_QUERYTAB,
	UM_DBLCLICKTAB,

	UM_CPN_SELCHANGE,
	UM_CPN_DROPDOWN,
	UM_CPN_CLOSEUP,
	UM_CPN_SELENDOK,
	UM_CPN_SELENDCANCEL,

	UM_MEDIA_INFO_RESULT,
	UM_ITEMSTATECHANGED,
	UM_SPN_SIZED,
	UM_TABMOVED,
	UM_TREEOPTSCTRL_NOTIFY,
	UM_DATA_CHANGED,
	UM_OSCOPEPOSITION,
	UM_DELAYED_EVALUATE,
	UM_ARCHIVESCANDONE,

	UM_STARTED2KUPDATE,			//Added by thilon for Updates
	UM_ED2KUPDATECOMPLETE,		//Added by thilon for Updates
	UM_EASYMULECHECKUPDATEFINISHED,

	UM_USER_MAPPING_FINISHED,				// upnp has been finished.
	UM_PORT_CHANGED,

	UM_IP2COUNTRY_COMPLETED,

	NMC_TW_ACTIVE_TAB_CHANDED,
	NMC_TW_TAB_DESTROY,
	NMC_TW_TAB_CREATE,

	UM_RECREATE_PARTFILE,		// Download ListCtrl

	UM_DLED_LC_ADDFILE,			//	Downloaded ListCtrl AddFile
	UM_DLED_LC_REMOVEFILE,		//	Downloaded ListCtrl RemoveFile

	UM_CPL_LC_ADDFILE,          //  Completed ListCtrl AddFile
	UM_CPL_LC_DELETEFILE,       //  Completed ListCtrl DeleteFile

	UM_FILE_UPDATE,

	UM_MTDD_CUR_SEL_FILE,		//	MainTab_DownloadDlg_Current_Selected_File
	UM_MTDD_CUR_SEL_FILE_TASK,  //  MainTab_DownloadDlg_Current_Selected_FileInfo
	UM_MTDD_CUR_SEL_PEER,		//  MainTab_DownloadDlg_Current_Selected_File
	UM_MTSD_CUR_SEL_FILE,		//	MainTab_ShareDlg_Current_Selected_File

	UM_HC_BEFORE_NAVI,			//	HtmlCtrl_BeforeNavigate
	UM_HC_NAVI_CMPL,			//	HtmlCtrl_NavigateComplete
	UM_HC_DOC_CMPL,				//	HtmlCtrl_DocumentComplete
	UM_HC_STATUS_TXT_CHANGE,	//	HtmlCtrl_StatusTextChange
	UM_HC_TITLE_CHANGE,

	UM_ADDTASK_DOC_ADDED,
	UM_ADDTASK_DOC_MODIFIED,
	UM_ADDTASK_DOC_REMOVED,
	
	UM_ADDTASK_DOC_URL_ADDED,
	UM_ADDTASK_DOC_URL_MODIFIED,
	UM_ADDTASK_DOC_URL_REMOVED,

	UM_TB_CHANGE_CONN_STATE,
	
	UM_GET_DESIRE_LENGTH,

	UM_RESTAB_WB_DESTROY,		// a WebBrowser window have been destroy in Resource Tab

	UM_SPLITTER_CLICKED,

	UM_NAT_CHECK_STRATEGIES,
	UM_REMOVEFROMALLQ_IN_THROTTLER,


	//the end of custom message
	UM_END
};
