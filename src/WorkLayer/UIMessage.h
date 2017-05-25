/*
 * $Id: UIMessage.h 5038 2008-03-19 02:55:54Z thilon $
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

#define WM_UIMSG_BASE				(WM_USER + 2010)

#define WM_SERVER_ADD_SVR			(WM_UIMSG_BASE + 1)
#define WM_SERVER_REFRESH			(WM_UIMSG_BASE + 2)

#define WM_FILE_ADD_DOWNLOAD		(WM_UIMSG_BASE + 20)
#define WM_FILE_ADD_PEER			(WM_UIMSG_BASE + 21)
#define WM_FILE_SHOWNOTIFIER		(WM_UIMSG_BASE + 22)
#define WM_FILE_REMOVE_PEER			(WM_UIMSG_BASE + 23)	//partfile|0,source
#define WM_FILE_UPDATE_PEER			(WM_UIMSG_BASE + 24)
#define WM_FILE_REMOVE_DOWNLOAD		(WM_UIMSG_BASE + 25)
#define WM_FILE_UPDATE_FILECOUNT	(WM_UIMSG_BASE + 26)
#define WM_FILE_HIDE_DOWNLOAD		(WM_UIMSG_BASE + 27)
#define WM_FILE_UPDATE_DOWNLOAD		(WM_UIMSG_BASE + 28)
#define WM_FILE_UPDATE_UPLOADRANK	(WM_UIMSG_BASE + 29)
#define	WM_FILE_FINISHEDHASHING		(WM_UIMSG_BASE + 30) // TM_FINISHEDHASHING
#define WM_FILE_HASHFAILED			(WM_UIMSG_BASE + 31)
#define WM_FILE_ADD_SOURCE			(WM_UIMSG_BASE + 32)
#define WM_FILE_ADD_SOURCE_NA		(WM_UIMSG_BASE + 33)
#define WM_FILE_REMOVE_SOURCE		(WM_UIMSG_BASE + 34)
#define WM_FILE_NOTIFYSTATUSCHANGE	(WM_UIMSG_BASE + 35)
#define WM_FILE_UPDATE_DOWNLOADING	(WM_UIMSG_BASE + 36)
#define WM_FILE_DOWNLOAD_COMPLETED	(WM_UIMSG_BASE + 37)
#define WM_FILE_ADD_COMPLETED		(WM_UIMSG_BASE + 38)
#define WM_FILE_OPPROGRESS			(WM_UIMSG_BASE + 39)

//Log
#define WM_FILE_ADD_PEERLOG			(WM_UIMSG_BASE + 40)
#define WM_FILE_UPDATE_PEERLOG		(WM_UIMSG_BASE + 42)
#define WM_FILE_UPDATE_FILELOG		(WM_UIMSG_BASE + 43)
#define WM_FILE_REMOVE_EVENTLOG		(WM_UIMSG_BASE + 44)

#define WM_SEARCH_UPDATESEARCH      (WM_UIMSG_BASE + 60)
#define WM_SEARCH_REMOVERESULT      (WM_UIMSG_BASE + 61)
#define WM_SEARCH_NEWTAB            (WM_UIMSG_BASE + 62)
#define WM_SEARCH_CANCELSEARCH      (WM_UIMSG_BASE + 63)
#define WM_SEARCH_LOCALED2KEND      (WM_UIMSG_BASE + 64)
#define WM_SEARCH_ADDED2KRESULT     (WM_UIMSG_BASE + 65)
#define WM_SEARCH_CANCELKADSEARCH   (WM_UIMSG_BASE + 66)
#define WM_SEARCH_NEWKADSEARCH      (WM_UIMSG_BASE + 67)
#define WM_SEARCH_NEWED2KSEARCH     (WM_UIMSG_BASE + 68)
#define WM_SEARCH_UPDATESOURCE      (WM_UIMSG_BASE + 69)
#define WM_SEARCH_SHOWRESULT        (WM_UIMSG_BASE + 70)
#define WM_SEARCH_ADDRESULT         (WM_UIMSG_BASE + 71)

// Shared File( 文件分享要处理的消息 ) 
#define WM_SHAREDFILE_UPDATE			(WM_UIMSG_BASE + 100)
#define WM_SHAREDFILE_SHOWCOUNT			(WM_UIMSG_BASE + 101)
#define WM_SHAREDFILE_SETAICHHASHING	(WM_UIMSG_BASE + 102) 
#define WM_SHAREDFILE_RELOADFILELIST	(WM_UIMSG_BASE + 103)
#define WM_SHAREDFILE_ADDFILE			(WM_UIMSG_BASE + 104)
#define WM_SHAREDFILE_REMOVEFILE		(WM_UIMSG_BASE + 105)
#define WM_SHAREDFILE_REMOVESHARE       (WM_UIMSG_BASE + 106)
#define WM_SHAREDFILE_UPDATECHECKBOX	(WM_UIMSG_BASE + 107)
//  the other
#define WM_ADD_LOGTEXT					(WM_UIMSG_BASE + 120)
#define WM_SHOW_CONNECTION_STATE		(WM_UIMSG_BASE + 121)
#define WM_LINE_SIG						(WM_UIMSG_BASE + 122)
#define WM_SHOW_TRANSFER_RATE			(WM_UIMSG_BASE + 123)
#define WM_GET_IL_COLORFLAGS			(WM_UIMSG_BASE + 124)
#define WM_SHOW_USERCOUNT				(WM_UIMSG_BASE + 125)


// kad
#define WM_KAD_CONTACTREF				(WM_UIMSG_BASE + 140)
#define WM_KAD_CONTACTADD				(WM_UIMSG_BASE + 141)
#define WM_KAD_CONTACTREM				(WM_UIMSG_BASE + 142)
#define WM_KAD_SEARCHADD				(WM_UIMSG_BASE + 143)
#define WM_KAD_SEARCHREM				(WM_UIMSG_BASE + 144)
#define WM_KAD_SEARCHREF				(WM_UIMSG_BASE + 145)
#define WM_KAD_HIDECONTACTS				(WM_UIMSG_BASE + 146)
#define WM_KAD_SHOWCONTACTS				(WM_UIMSG_BASE + 147)

//Movie UI
#define WM_UPDATE_GUI_FILEPROGRESS		(WM_UIMSG_BASE + 148)
#define WM_UPDATE_GUI_START				(WM_UIMSG_BASE + 149)
#define WM_UPDATE_GUI_STOP				(WM_UIMSG_BASE + 150)

//WebServer UI
#define WM_GET_PARTLIST					(WM_UIMSG_BASE + 160)

//CSharedDirsTreeCtrl
#define WM_UPDATTREEITEM				(WM_UIMSG_BASE + 170)
