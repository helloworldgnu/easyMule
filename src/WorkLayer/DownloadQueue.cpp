/* 
 * $Id: DownloadQueue.cpp 9297 2008-12-24 09:55:04Z dgkang $
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
#include <io.h>
//#include "emule.h"
#include "DownloadQueue.h"
#include "UpDownClient.h"
#include "PartFile.h"
#include "ed2kLink.h"
#include "SearchFile.h"
#include "ClientList.h"
#include "Statistics.h"
#include "SharedFileList.h"
#include "OtherFunctions.h"
#include "SafeFile.h"
#include "Sockets.h"
#include "ServerList.h"
#include "Server.h"
#include "Packets.h"
#include "Kademlia/Kademlia/Kademlia.h"
#include "kademlia/utils/uint128.h"
#include "ipfilter.h"
//#include "emuledlg.h"
//#include "TransferWnd.h"
//#include "TaskbarNotifier.h"
#include "MenuCmds.h"
#include "Log.h"
#include "Internal/InternalSocket.h"    // VC-kernel[2007-01-11]:
#include "UIMessage.h"
#include "GlobalVariable.h"
#include "resource.h"

#include "HttpClient.h"
#include "FtpClient.h"
#include "DNSManager.h"

#include "Ed2kUpDownClient.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CDownloadQueue::CDownloadQueue()
{
	filesrdy = 0;
	datarate = 0;
	cur_udpserver = 0;
	lastfile = 0;
	lastcheckdiskspacetime = 0;
	lastudpsearchtime = 0;
	lastudpstattime = 0;
	SetLastKademliaFileRequest();
	udcounter = 0;
	m_iSearchedServers = 0;
	m_datarateMS=0;
	m_nUDPFileReasks = 0;
	m_nFailedUDPFileReasks = 0;
	m_dwNextTCPSrcReq = 0;
	m_cRequestsSentToServer = 0;
    m_dwLastA4AFtime = 0; // ZZ:DownloadManager

}

void CDownloadQueue::AddPartFilesToShare()
{
	for (POSITION pos = filelist.GetHeadPosition(); pos != 0; )
	{
		CPartFile* cur_file = filelist.GetNext(pos);
		if (cur_file->GetStatus(true) == PS_READY)
			CGlobalVariable::sharedfiles->SafeAddKFile(cur_file, true);
	}
}

void CDownloadQueue::ScanPartFile(CString strDirectory)
{
	CFileFind ff;
	int count = 0;
    
	CString searchPath;
	if(strDirectory.IsEmpty())
	{
      searchPath = thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR);
	  strDirectory = thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR);
	}
	else
	    searchPath = strDirectory;

	searchPath += _T("\\*.part.met");
	CStringList lstTrybak;

	//check all part.met files
	bool end = !ff.FindFile(searchPath, 0);
	while (!end)
	{
		end = !ff.FindNextFile();
		if (ff.IsDirectory())
			continue;
		CPartFile* toadd = new CPartFile();
		if( NULL!=GetFileByMetFileName( strDirectory+ _T("\\") +ff.GetFileName() ) )
		{
			delete toadd;
			continue;// [11/5/2007 huby]: this partfile have been load by this emule/easyMule
		}
		else if (toadd->LoadPartFile(strDirectory, ff.GetFileName()))
		{
			bool bCanAdd = true;

			if(toadd->GetStatus(true) == PS_READY)
			{
				bCanAdd = CGlobalVariable::sharedfiles->SafeAddKFile(toadd); // part files are always shared files					
			}
			else if( !toadd->HasNullHash() )
			{
				CCKey key(toadd->GetFileHash());
				bCanAdd = CGlobalVariable::sharedfiles->GetKnownFile(key)==NULL;			
				//[VC-Huby-20080125]: need check this hash partfile is in waitinghashlist? but the file in waitinghashlist has no hash. 
			}
			
			if(bCanAdd)
			{
				bCanAdd = CGlobalVariable::filemgr.AddPartFile(toadd, FILESTATE_DOWNLOADING);
			}

			if(bCanAdd)
			{
				count++;
				filelist.AddTail(toadd);// add to downloadqueue
				SendMessage(CGlobalVariable::m_hListenWnd, WM_FILE_ADD_DOWNLOAD, 0, (LPARAM)toadd);
			}
			else
			{
				delete toadd;
				continue;
			}
		}
		else
		{
			CString strErr;
			CString metname=ff.GetFileName();
			CString filename;
			if(metname.GetLength()>9) filename = metname.Left(metname.GetLength()-9);
			else filename = metname;

			// VC-yunchenn.chen[2007-07-12]: 可能会是另一个emule打开，在这里判断是否这种情况
			CFileException fexpPart;
			CFile fileTest;
			bool bDoing = false;
 
			if (!fileTest.Open(strDirectory + _T("\\") + filename + _T(".part"), CFile::modeReadWrite|CFile::shareDenyWrite|CFile::osSequentialScan, &fexpPart))
			{
				if(fexpPart.m_cause==CFileException::sharingViolation)
				{
					bDoing = true;
					//lstIgnorebak.AddTail(metname+_T(".backup"));
					strErr.Format(IDS_FAIL_ADD_PARTFILE, filename);
					//AfxMessageBox(strErr);
					CGlobalVariable::ShowNotifier(strErr,TBN_IMPORTANTEVENT);
				}
				TRACE(_T("Failed to open met file\n"));
			}
			else fileTest.Close();

			if(!bDoing)
			{
				lstTrybak.AddTail(metname+_T(".bak"));
			}

			delete toadd;
		}
	}
	ff.Close();

	//try recovering any part.met files
	//	searchPath += _T(".backup");
	//	end = !ff.FindFile(searchPath, 0);
	POSITION pos = lstTrybak.GetHeadPosition();
	while (pos)
	{
		//end = !ff.FindNextFile();
		//if (ff.IsDirectory())
		//	continue;

		CPartFile* toadd = new CPartFile();
		CString strBak = lstTrybak.GetNext(pos);
		if (toadd->LoadPartFile(strDirectory, strBak))
		{
			toadd->SavePartFile(); // resave backup
			count++;
			filelist.AddTail(toadd);			// to downloadqueue
			if (toadd->GetStatus(true) == PS_READY)
				CGlobalVariable::sharedfiles->SafeAddKFile(toadd); // part files are always shared files

			//  Comment UI
			if(CGlobalVariable::filemgr.AddPartFile(toadd, FILESTATE_DOWNLOADING))
			     SendMessage(CGlobalVariable::m_hListenWnd, WM_FILE_ADD_DOWNLOAD, 0, (LPARAM)toadd);
			//theApp.emuledlg->transferwnd->downloadlistctrl.AddFile(toadd);// show in downloadwindow
			//CGlobalVariable::filemgr.AddPartFile(toadd, FILESTATE_DOWNLOAD);
			AddLogLine(false, GetResString(IDS_RECOVERED_PARTMET), toadd->GetFileName());
		}
		else {
			delete toadd;
		}
	}
	//ff.Close();

	if(count == 0)
	{
		AddLogLine(false,GetResString(IDS_NOPARTSFOUND));
	}
	else
	{
		AddLogLine(false,GetResString(IDS_FOUNDPARTS),count);
	}
}

void CDownloadQueue::LoadOldVersionPartFile()
{
	for (int i=0;i<thePrefs.tempdir.GetCount();i++) {
		CString searchPath=thePrefs.GetTempDir(i);
		ScanPartFile(searchPath);
	}
}

BOOL CDownloadQueue::MetFileRecovery(CString &strDir, CString &strFileName)
{
	CString strMetFileName;
	strMetFileName.Format(_T("%s\\%s"), strDir, strFileName);

	CString strBakFileName(strMetFileName);
	strBakFileName.Append(PARTMET_BAK_EXT);

	if (!::CopyFile(strBakFileName, strMetFileName, FALSE))
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

CPartFile* CDownloadQueue::LoadPartFile(CString &strDir, CString &strFileName)
{
	CPartFile* toadd = new CPartFile();

	if (toadd->LoadPartFile(strDir, strFileName))
	{
		filelist.AddTail(toadd);			// to downloadqueue
		if (toadd->GetStatus(true) == PS_READY)
		{
			CGlobalVariable::sharedfiles->SafeAddKFile(toadd); // part files are always shared files
		}

//		theApp.emuledlg->transferwnd->downloadlistctrl.AddFile(toadd); // show in download window
		SendMessage(CGlobalVariable::m_hListenWnd, WM_FILE_ADD_DOWNLOAD, 0, (LPARAM)toadd);

		return toadd;
	}
	else
	{
		delete toadd;
		return NULL;
	}
}

BOOL CDownloadQueue::LoadThisPartFile(CString strDir, CString strFileName)
{
	if (LoadPartFile(strDir, strFileName) != NULL)
	{
		return TRUE;
	}
	else if (MetFileRecovery(strDir, strFileName))
	{
		if (LoadPartFile(strDir, strFileName) != NULL)
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}

	return FALSE;
}

BOOL CDownloadQueue::LoadBackupPartFile(CString strDir, CString strFileName)
{
	CPartFile * pPartFile;

	if ( (pPartFile = LoadPartFile(strDir, strFileName)) != NULL)
	{
		pPartFile->SavePartFile();

		AddLogLine(false, GetResString(IDS_RECOVERED_PARTMET), pPartFile->GetFileName());

		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

void CDownloadQueue::Init()
{
	// find all part files, read & hash them if needed and store into a list
/*	CFileFind ff;
	int count = 0;

	for (int i=0;i<thePrefs.tempdir.GetCount();i++) 
	{
		CString searchPath=thePrefs.GetTempDir(i);

		searchPath += _T("\\*.part.met");

		//check all part.met files
		bool end = !ff.FindFile(searchPath, 0);
		while (!end)
		{
			end = !ff.FindNextFile();
			if (ff.IsDirectory())
			{
				continue;
			}

			if (LoadThisPartFile(thePrefs.GetTempDir(i), ff.GetFileName()))
			{
				count++;
				filelist.AddTail(toadd);			// to downloadqueue
				if (toadd->GetStatus(true) == PS_READY)
					CGlobalVariable::sharedfiles->SafeAddKFile(toadd); // part files are always shared files

				//  Comment UI
				SendMessage(CGlobalVariable::m_hListenWnd, WM_FILE_ADD_DOWNLOAD, 0, (LPARAM)toadd);
				//theApp.emuledlg->transferwnd->downloadlistctrl.AddFile(toadd);// show in downloadwindow
			}
		}
		ff.Close();

		//try recovering any part.met files
		searchPath += _T(".backup");
		end = !ff.FindFile(searchPath, 0);
		while (!end)
		{
			end = !ff.FindNextFile();
			if (ff.IsDirectory())
			{
				continue;
			}

			if (LoadBackupPartFile(thePrefs.GetTempDir(i), ff.GetFileName()))
			{
				count++;
				filelist.AddTail(toadd);			// to downloadqueue
				if (toadd->GetStatus(true) == PS_READY)
					CGlobalVariable::sharedfiles->SafeAddKFile(toadd); // part files are always shared files

				//  Comment UI
				SendMessage(CGlobalVariable::m_hListenWnd, WM_FILE_ADD_DOWNLOAD, 0, (LPARAM)toadd);
				//theApp.emuledlg->transferwnd->downloadlistctrl.AddFile(toadd);// show in downloadwindow

				AddLogLine(false, GetResString(IDS_RECOVERED_PARTMET), toadd->GetFileName());
			}
		}
		ff.Close();
	}
	if(count == 0) {
		AddLogLine(false,GetResString(IDS_NOPARTSFOUND));
	} 
	else 
	{
		AddLogLine(false,GetResString(IDS_FOUNDPARTS),count);
		SortByPriority();
		CheckDiskspace();
	}*/

	//ExportPartMetFilesOverview();
}

CDownloadQueue::~CDownloadQueue(){
	for (POSITION pos = filelist.GetHeadPosition();pos != 0;)
		delete filelist.GetNext(pos);
}

//void CDownloadQueue::AddSearchToDownload(CSearchFile* toadd, uint8 paused, int cat)
//{
//	if (toadd->GetFileSize()== (uint64)0 || IsFileExisting(toadd->GetFileHash()))
//		return;
//
//	if (toadd->GetFileSize() > OLD_MAX_EMULE_FILE_SIZE && !thePrefs.CanFSHandleLargeFiles()){
//		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_FSCANTHANDLEFILE));
//		return;
//	}
//
//	CPartFile* newfile = new CPartFile(toadd,cat);
//	if (newfile->GetStatus() == PS_ERROR){
//		delete newfile;
//		return;
//	}
//
//	if (paused == 2)
//		paused = (uint8)thePrefs.AddNewFilesPaused();
//	AddDownload(newfile, (paused==1));
//
//	// If the search result is from OP_GLOBSEARCHRES there may also be a source
//	if (toadd->GetClientID() && toadd->GetClientPort()){
//		CSafeMemFile sources(1+4+2);
//		try{
//			sources.WriteUInt8(1);
//			sources.WriteUInt32(toadd->GetClientID());
//			sources.WriteUInt16(toadd->GetClientPort());
//		    sources.SeekToBegin();
//		    newfile->AddSources(&sources, toadd->GetClientServerIP(), toadd->GetClientServerPort(), false);
//		}
//		catch(CFileException* error){
//			ASSERT(0);
//			error->Delete();
//		}
//	}
//
//	// Add more sources which were found via global UDP search
//	const CSimpleArray<CSearchFile::SClient>& aClients = toadd->GetClients();
//	for (int i = 0; i < aClients.GetSize(); i++){
//		CSafeMemFile sources(1+4+2);
//		try{
//			sources.WriteUInt8(1);
//			sources.WriteUInt32(aClients[i].m_nIP);
//			sources.WriteUInt16(aClients[i].m_nPort);
//		    sources.SeekToBegin();
//			newfile->AddSources(&sources,aClients[i].m_nServerIP, aClients[i].m_nServerPort, false);
//	    }
//		catch(CFileException* error){
//			ASSERT(0);
//			error->Delete();
//			break;
//		}
//	}
//}

void CDownloadQueue::AddSearchToDownload(CString link, uint8 paused, int cat)
{
	CPartFile* newfile = new CPartFile(link, cat);
	if (newfile->GetStatus() == PS_ERROR){
		delete newfile;
		return;
	}

	if (paused == 2)
		paused = (uint8)thePrefs.AddNewFilesPaused();
	AddDownload(newfile, (paused==1));
}

void CDownloadQueue::StartNextFileIfPrefs(int cat) {
    if (thePrefs.StartNextFile())
		StartNextFile((thePrefs.StartNextFile() > 1?cat:-1), (thePrefs.StartNextFile()!=3));
}

void CDownloadQueue::StartNextFile(int cat, bool force){

	CPartFile*  pfile = NULL;
	CPartFile* cur_file ;
	POSITION pos;
	
	if (cat != -1) {
        // try to find in specified category
		for (pos = filelist.GetHeadPosition();pos != 0;){
			cur_file = filelist.GetNext(pos);
			if (cur_file->GetStatus()==PS_PAUSED &&
                (
				 cur_file->GetCategory()==(UINT)cat || 
				 cat==0 && thePrefs.GetCategory(0)->filter==0 && cur_file->GetCategory()>0
                ) &&
                CPartFile::RightFileHasHigherPrio(pfile, cur_file)
			   ) {
    			pfile = cur_file;
			}
		}
		if (pfile == NULL && !force)
			return;
	}

    if(cat == -1 || pfile == NULL && force) {
	    for (pos = filelist.GetHeadPosition();pos != 0;){
		    cur_file = filelist.GetNext(pos);
		    if (cur_file->GetStatus() == PS_PAUSED &&
                CPartFile::RightFileHasHigherPrio(pfile, cur_file))
		    {
                // pick first found matching file, since they are sorted in prio order with most important file first.
			    pfile = cur_file;
		    }
	    }
    }
	if (pfile) pfile->ResumeFile();
}

CPartFile *CDownloadQueue::AddFileLinkToDownload(CED2KFileLink* pLink, int cat,bool bNewTask)
{
	CPartFile* newfile = new CPartFile(pLink, cat);
	
	if (newfile->GetStatus() == PS_ERROR)
	{
		delete newfile;
		newfile=NULL;
	}
	else 
	{ 
	   AddDownload(newfile, thePrefs.AddNewFilesPaused(),bNewTask);
	}

	CPartFile* partfile = newfile;
	if (partfile == NULL)
		partfile = GetFileByID(pLink->GetHashKey());
	if (partfile)
	{
		if (pLink->HasValidSources())
			partfile->AddClientSources(pLink->SourcesList, 1, false);
		if (pLink->HasValidAICHHash() ){
			if ( !(partfile->GetAICHHashset()->HasValidMasterHash() && partfile->GetAICHHashset()->GetMasterHash() == pLink->GetAICHHash())){
				partfile->GetAICHHashset()->SetMasterHash(pLink->GetAICHHash(), AICH_VERIFIED);
				partfile->GetAICHHashset()->FreeHashSet();
			}
		}
	}

	if (pLink->HasHostnameSources())
	{
		POSITION pos = pLink->m_HostnameSourcesList.GetHeadPosition();
		while (pos != NULL)
		{
			const SUnresolvedHostname* pUnresHost = pLink->m_HostnameSourcesList.GetNext(pos);
			CGlobalVariable::m_DNSManager->AddToResolved(pLink->GetHashKey(), pUnresHost->strHostname, pUnresHost->nPort, pUnresHost->strURL);
		}
	}

	return newfile;
}

void CDownloadQueue::AddDownload(CPartFile* newfile,bool paused,bool bShowNotifer) 
{
	// Barry - Add in paused mode if required
	if (paused)
	{
		newfile->PauseFile();
	}
	else
	{
		newfile->AddFileLog(new CTraceInformation(GetResString(IDS_START_DOWNLOAD)));
	}

	SetAutoCat(newfile);// HoaX_69 / Slugfiller: AutoCat

	filelist.AddTail(newfile);
	SortByPriority();
	CheckDiskspace();

	//  Comment UI
	SendMessage(CGlobalVariable::m_hListenWnd, WM_FILE_ADD_DOWNLOAD, 0, (LPARAM)newfile);

	//AddLogLine(true, GetResString(IDS_NEWDOWNLOAD), newfile->GetFileName());
	CString msgTemp;
	msgTemp.Format(GetResString(IDS_NEWDOWNLOAD) + _T("\n"), newfile->GetFileName());

	//  Comment UI
	if(bShowNotifer)
	{
	   SendMessage(CGlobalVariable::m_hListenWnd,WM_FILE_SHOWNOTIFIER,(WPARAM)&msgTemp,TBN_DOWNLOADADDED);
	   AddLogLine(true, GetResString(IDS_NEWDOWNLOAD), newfile->GetFileName());
	}

	ExportPartMetFilesOverview();
	
	// VC-SearchDream[2007-06-25]: If the file hash is null do not get src
//#ifndef _DEBUG_PEER
	if (!newfile->m_bAlreadyFetchUrlSrc)
	{
		newfile->m_urlSrcFromSvrMgr.GetSrcFromServerAsyn();
	}
//#endif

#ifdef _ENABLE_LAN_TRANSFER
	if ( !newfile->HasNullHash() )
	{
		CGlobalVariable::internalsocket->Broadcast( newfile->GetFileHash() );   // VC-kernel[2007-01-11]:
	}
#endif
}

bool CDownloadQueue::IsFileExisting(const uchar* fileid, bool bLogWarnings) const
{
	uchar hash[16];
	md4clr(hash);

	if (!md4cmp(hash, fileid)) // VC-SearchDream[2007-03-21]: For HTTP and FTP Direct DownLoad
	{
		return false;
	}

	const CKnownFile* file = CGlobalVariable::sharedfiles->GetFileByID(fileid);
	if (file)
	{
		if (bLogWarnings)
		{
			if (file->IsPartFile())
				LogWarning(LOG_STATUSBAR, GetResString(IDS_ERR_ALREADY_DOWNLOADING), file->GetFileName());
			else
				LogWarning(LOG_STATUSBAR, GetResString(IDS_ERR_ALREADY_DOWNLOADED), file->GetFileName());
		}
		return true;
	}
	else if ((file = GetFileByID(fileid)) != NULL)
	{
		if (bLogWarnings)
			LogWarning(LOG_STATUSBAR, GetResString(IDS_ERR_ALREADY_DOWNLOADING), file->GetFileName());
		return true;
	}
	
	return false;
}

void CDownloadQueue::Process()
{
	ProcessLocalRequests(); // send src requests to local server

	uint32 downspeed = 0;
    uint64 maxDownload = thePrefs.GetMaxDownloadInBytesPerSec(true);
	if (maxDownload != UNLIMITED*1024 && datarate > 1500)
	{
		downspeed = (UINT)((maxDownload*100)/(datarate+1));
		if (downspeed < 50)
			downspeed = 50;
		else if (downspeed > 200)
			downspeed = 200;
	}

	while(avarage_dr_list.GetCount()>0 && (GetTickCount() - avarage_dr_list.GetHead().timestamp > 10*1000) )
		m_datarateMS-=avarage_dr_list.RemoveHead().datalen;
	
	if (avarage_dr_list.GetCount()>1)
	{
		datarate = (UINT)(m_datarateMS / avarage_dr_list.GetCount());
	} 
	else 
	{
		datarate = 0;
	}

	uint32 datarateX=0;
	udcounter++;

	//filelist is already sorted by prio, therefore I removed all the extra loops..
	for (POSITION pos = filelist.GetHeadPosition();pos != 0;)
	{
		CPartFile* cur_file = filelist.GetNext(pos);
		if (cur_file->GetStatus() == PS_READY || cur_file->GetStatus() == PS_EMPTY)
		{
			datarateX += cur_file->Process(downspeed, udcounter);
		}
		else
		{
			//This will make sure we don't keep old sources to paused and stoped files..
			cur_file->StopPausedFile();
		}
	}

	TransferredData newitem = {datarateX, ::GetTickCount()};
	avarage_dr_list.AddTail(newitem);
	m_datarateMS+=datarateX;

	if (udcounter == 5)
	{
		if (CGlobalVariable::serverconnect->IsUDPSocketAvailable()){
		    if((!lastudpstattime) || (::GetTickCount() - lastudpstattime) > UDPSERVERSTATTIME){
			    lastudpstattime = ::GetTickCount();
			    CGlobalVariable::serverlist->ServerStats();
		    }
	    }
	}

	if (udcounter == 10)
	{
		udcounter = 0;
		if (CGlobalVariable::serverconnect->IsUDPSocketAvailable())
		{
			if ((!lastudpsearchtime) || (::GetTickCount() - lastudpsearchtime) > UDPSERVERREASKTIME)
				SendNextUDPPacket();
		}
	}

	CheckDiskspaceTimed();

// ZZ:DownloadManager -->
    if((!m_dwLastA4AFtime) || (::GetTickCount() - m_dwLastA4AFtime) > 2*60*1000) 
	{
        CGlobalVariable::clientlist->ProcessA4AFClients();
        m_dwLastA4AFtime = ::GetTickCount();
    }
// <-- ZZ:DownloadManager

}

CPartFile* CDownloadQueue::GetFileByIndex(int index) const
{
	POSITION pos = filelist.FindIndex(index);
	if (pos)
		return filelist.GetAt(pos);
	return NULL;
}

CPartFile* CDownloadQueue::GetFileByID(const uchar* filehash) const
{
	for (POSITION pos = filelist.GetHeadPosition(); pos != 0; )
	{
		CPartFile* cur_file = filelist.GetNext(pos);
		if (!md4cmp(filehash, cur_file->GetFileHash()))
		{
			return cur_file;
		}
	}
	return NULL;
}

CPartFile* CDownloadQueue::GetFileByMetFileName( const CString strMetFileName ) const
{
	for (POSITION pos = filelist.GetHeadPosition(); pos != 0; )
	{
		CPartFile* cur_file = filelist.GetNext(pos);
		if( !strMetFileName.CompareNoCase(cur_file->GetFullName()) )
		{
			return cur_file;
		}
	}
	return NULL;
}

CPartFile* CDownloadQueue::GetFileByKadFileSearchID(uint32 id) const
{
	for (POSITION pos = filelist.GetHeadPosition(); pos != 0; )
	{
		CPartFile* cur_file = filelist.GetNext(pos);
		if (id == cur_file->GetKadFileSearchID())
			return cur_file;
	}
	return NULL;
}

bool CDownloadQueue::IsPartFile(const CKnownFile* file) const
{
	for (POSITION pos = filelist.GetHeadPosition(); pos != 0; )
	{
		if (file == filelist.GetNext(pos))
			return true;
	}
	return false;
}

bool CDownloadQueue::CheckAndAddSource(CPartFile* sender,CUpDownClient* source)
{
	if (source->IsKindOf(RUNTIME_CLASS(CHttpClient)) || 
		source->IsKindOf(RUNTIME_CLASS(CFtpClient))) // VC-SearchDream[2007-03-21]: For HTTP and FTP Direct DownLoad
	{
		// We Directly Add the Client without any process Here for Direct HTTP and FTP DownLoad
		CGlobalVariable::clientlist->AddClient(source,true);
		
		sender->srclist.AddTail(source);

		SendMessage(CGlobalVariable::m_hListenWnd, WM_FILE_ADD_SOURCE_NA, (WPARAM)sender, (LPARAM)source);

		return true;
	}

#ifdef _DEBUG_ONLY_INET
	return false;
#endif

	if (sender->IsStopped())
	{
		delete source;
		return false;
	}

	if( thePrefs.GetFilterPeer()==1 )
	{
		if( !source->HasLowID() )
		{
			delete source;
			return false;
		}
	}
	else if( thePrefs.GetFilterPeer()==2 )
	{
		if( source->HasLowID() )
		{
			delete source;
			return false;
		}
	}

	if (source->HasValidHash())
	{
		if(!md4cmp(source->GetUserHash(), thePrefs.GetUserHash()))
		{
			if (thePrefs.GetVerbose())
				AddDebugLogLine(false, _T("Tried to add source with matching hash to your own."));
			delete source;
			return false;
		}
		// VC-kernel[2007-01-17]:
		//if there is no validhash,IsDeadSource will cause crash
		//so I move the follow code lines

		// xman dlp
		// filter sources which are known to be temporarily dead/useless
		if (CGlobalVariable::clientlist->m_globDeadSourceList.IsDeadSource(source) || sender->m_DeadSourceList.IsDeadSource(source)){
			//if (thePrefs.GetLogFilteredIPs())
//#if defined(_DEBUG) || defined(_BETA) || defined(_VCALPHA)
#if defined(_DEBUG) || defined(_BETA)
			AddDebugLogLine(DLP_DEFAULT, false, _T("Rejected source because it was found on the DeadSourcesList (%s) for file %s : %s")
				,sender->m_DeadSourceList.IsDeadSource(source)? _T("Local") : _T("Global"), sender->GetFileName(), source->DbgGetClientInfo() );
#endif
			delete source;
			return false;
		}
		// xman end
	}

	// filter sources which are known to be temporarily dead/useless
	if( source->GetSourceFrom()!=SF_LAN && 
		(CGlobalVariable::clientlist->m_globDeadSourceList.IsDeadSource(source) || sender->m_DeadSourceList.IsDeadSource(source)) )
	{
		//if (thePrefs.GetLogFilteredIPs())
		//	AddDebugLogLine(DLP_DEFAULT, false, _T("Rejected source because it was found on the DeadSourcesList (%s) for file %s : %s")
		//	,sender->m_DeadSourceList.IsDeadSource(source)? _T("Local") : _T("Global"), sender->GetFileName(), source->DbgGetClientInfo() );
		delete source;
		return false;
	}

	// filter sources which are incompatible with our encryption setting (one requires it, and the other one doesn't supports it)
	if ( (source->RequiresCryptLayer() && (!thePrefs.IsClientCryptLayerSupported() || !source->HasValidHash())) || (thePrefs.IsClientCryptLayerRequired() && (!source->SupportsCryptLayer() || !source->HasValidHash())))
	{
//#if defined(_DEBUG) || defined(_BETA) || defined(_VCALPHA)
#if defined(_DEBUG) || defined(_BETA)
		//if (thePrefs.GetDebugSourceExchange()) // TODO: Uncomment after testing
			AddDebugLogLine(DLP_DEFAULT, false, _T("Rejected source because CryptLayer-Setting (Obfuscation) was incompatible for file %s : %s"), sender->GetFileName(), source->DbgGetClientInfo() );
#endif
		delete source;
		return false;
	}

	// "Filter LAN IPs" and/or "IPfilter" is not required here, because it was already done in parent functions

	// uses this only for temp. clients
	for (POSITION pos = filelist.GetHeadPosition();pos != 0;)
	{
		CPartFile* cur_file = filelist.GetNext(pos);
		for (POSITION pos2 = cur_file->srclist.GetHeadPosition();pos2 != 0; )
		{
			CUpDownClient* cur_client = cur_file->srclist.GetNext(pos2);
			if (cur_client->Compare(source, true) || cur_client->Compare(source, false))
			{
				if (cur_file == sender)
				{ 
					// this file has already this source
					delete source;
					return false;
				}
				// set request for this source
				if (cur_client->AddRequestForAnotherFile(sender))
				{
					//  Comment UI
                    //uint32 struPFilePeer[] = {(uint32)sender,(uint32)cur_client,(uint32)true};
					//SendMessage(CGlobalVariable::m_hListenWnd,WM_FILE_ADD_PEER,0,(LPARAM)struPFilePeer);
					SendMessage(CGlobalVariable::m_hListenWnd,WM_FILE_ADD_SOURCE,(WPARAM)sender,(LPARAM)cur_client);
					//theApp.emuledlg->transferwnd->downloadlistctrl.AddSource(sender,cur_client,true);

					delete source;
                    if(cur_client->GetDownloadState() != DS_CONNECTED) 
					{
                        cur_client->SwapToAnotherFile(_T("New A4AF source found. CDownloadQueue::CheckAndAddSource()"), false, false, false, NULL, true, false); // ZZ:DownloadManager
                    }
					return false;
				}
				else
				{
					delete source;
					return false;
				}
			}
		}
	}
	//our new source is real new but maybe it is already uploading to us?
	//if yes the known client will be attached to the var "source"
	//and the old sourceclient will be deleted
	if (CGlobalVariable::clientlist->AttachToAlreadyKnown(&source,0))
	{
#ifdef _DEBUG
		if (thePrefs.GetVerbose() && source->GetRequestFile())
		{
			// if a client sent us wrong sources (sources for some other file for which we asked but which we are also
			// downloading) we may get a little in trouble here when "moving" this source to some other partfile without
			// further checks and updates.
			if (md4cmp(source->GetRequestFile()->GetFileHash(), sender->GetFileHash()) != 0)
				AddDebugLogLine(false, _T("*** CDownloadQueue::CheckAndAddSource -- added potential wrong source (%u)(diff. filehash) to file \"%s\""), source->GetUserIDHybrid(), sender->GetFileName());
			if (source->GetRequestFile()->GetPartCount() != 0 && source->GetRequestFile()->GetPartCount() != sender->GetPartCount())
				AddDebugLogLine(false, _T("*** CDownloadQueue::CheckAndAddSource -- added potential wrong source (%u)(diff. partcount) to file \"%s\""), source->GetUserIDHybrid(), sender->GetFileName());
		}
#endif
		source->SetRequestFile(sender);
	}
	else
	{
		// here we know that the client instance 'source' is a new created client instance (see callers) 
		// which is therefor not already in the clientlist, we can avoid the check for duplicate client list entries 
		// when adding this client
		CGlobalVariable::clientlist->AddClient(source,true);
	}
	
#ifdef _DEBUG
	if (thePrefs.GetVerbose() && source->GetPartCount()!=0 && source->GetPartCount()!=sender->GetPartCount())
	{
		DEBUG_ONLY(AddDebugLogLine(false, _T("*** CDownloadQueue::CheckAndAddSource -- New added source (%u, %s) had still value in partcount"), source->GetUserIDHybrid(), sender->GetFileName()));
	}
#endif

	sender->srclist.AddTail(source);
    //  Comment UI
    //uint32 struPFilePeer[] = {(uint32)sender,(uint32)source,(uint32)false};
	//SendMessage(CGlobalVariable::m_hListenWnd,WM_FILE_ADD_PEER,0,(LPARAM)struPFilePeer);
	SendMessage(CGlobalVariable::m_hListenWnd,WM_FILE_ADD_SOURCE_NA,(WPARAM)sender,(LPARAM)source);
	//theApp.emuledlg->transferwnd->downloadlistctrl.AddSource(sender,source,false);

	return true;
}

bool CDownloadQueue::CheckAndAddKnownSource(CPartFile* sender,CUpDownClient* source, bool bIgnoreGlobDeadList){
	if (sender->IsStopped())
		return false;
	
	// filter sources which are known to be temporarily dead/useless
	if ( (CGlobalVariable::clientlist->m_globDeadSourceList.IsDeadSource(source) && !bIgnoreGlobDeadList) || sender->m_DeadSourceList.IsDeadSource(source)){
		//if (thePrefs.GetLogFilteredIPs())
		//	AddDebugLogLine(DLP_DEFAULT, false, _T("Rejected source because it was found on the DeadSourcesList (%s) for file %s : %s")
		//	,sender->m_DeadSourceList.IsDeadSource(source)? _T("Local") : _T("Global"), sender->GetFileName(), source->DbgGetClientInfo() );
		return false;
	}

	// xman dlp
	// filter sources which are incompatible with our encryption setting (one requires it, and the other one doesn't supports it)
	if ( (source->RequiresCryptLayer() && (!thePrefs.IsClientCryptLayerSupported() || !source->HasValidHash())) || (thePrefs.IsClientCryptLayerRequired() && (!source->SupportsCryptLayer() || !source->HasValidHash())))
	{
//#if defined(_DEBUG) || defined(_BETA) || defined(_VCALPHA)
#if defined(_DEBUG) || defined(_BETA)
		//if (thePrefs.GetDebugSourceExchange()) // TODO: Uncomment after testing
			AddDebugLogLine(DLP_DEFAULT, false, _T("Rejected source because CryptLayer-Setting (Obfuscation) was incompatible for file %s : %s"), sender->GetFileName(), source->DbgGetClientInfo() );
#endif
		return false;
	}
	// man

	// "Filter LAN IPs" -- this may be needed here in case we are connected to the internet and are also connected
	// to a LAN and some client from within the LAN connected to us. Though this situation may be supported in future
	// by adding that client to the source list and filtering that client's LAN IP when sending sources to
	// a client within the internet.
	//
	// "IPfilter" is not needed here, because that "known" client was already IPfiltered when receiving OP_HELLO.
	if (!source->HasLowID()){
		uint32 nClientIP = ntohl(source->GetUserIDHybrid());
		if (!IsGoodIP(nClientIP)){ // check for 0-IP, localhost and LAN addresses
			//if (thePrefs.GetLogFilteredIPs())
			//	AddDebugLogLine(false, _T("Ignored already known source with IP=%s"), ipstr(nClientIP));
			return false;
		}
	}

	// use this for client which are already know (downloading for example)
	for (POSITION pos = filelist.GetHeadPosition();pos != 0;){
		CPartFile* cur_file = filelist.GetNext(pos);
		if (cur_file->srclist.Find(source)){
			if (cur_file == sender)
				return false;
			if (source->AddRequestForAnotherFile(sender))
				//  Comment UI
				{
				//uint32 struPFilePeer[] = {(uint32)sender,(uint32)source,(uint32)true};
				//SendMessage(CGlobalVariable::m_hListenWnd,WM_FILE_ADD_PEER,0,(LPARAM)struPFilePeer);
					SendMessage(CGlobalVariable::m_hListenWnd,WM_FILE_ADD_SOURCE,(WPARAM)sender,(LPARAM)source);
				}
				//theApp.emuledlg->transferwnd->downloadlistctrl.AddSource(sender,source,true);

                if(source->GetDownloadState() != DS_CONNECTED) {
                    source->SwapToAnotherFile(_T("New A4AF source found. CDownloadQueue::CheckAndAddKnownSource()"), false, false, false, NULL, true, false); // ZZ:DownloadManager
                }
			return false;
		}
	}
#ifdef _DEBUG
	if (thePrefs.GetVerbose() && source->GetRequestFile()){
		// if a client sent us wrong sources (sources for some other file for which we asked but which we are also
		// downloading) we may get a little in trouble here when "moving" this source to some other partfile without
		// further checks and updates.
		if (md4cmp(source->GetRequestFile()->GetFileHash(), sender->GetFileHash()) != 0)
			AddDebugLogLine(false, _T("*** CDownloadQueue::CheckAndAddKnownSource -- added potential wrong source (%u)(diff. filehash) to file \"%s\""), source->GetUserIDHybrid(), sender->GetFileName());
		if (source->GetRequestFile()->GetPartCount() != 0 && source->GetRequestFile()->GetPartCount() != sender->GetPartCount())
			AddDebugLogLine(false, _T("*** CDownloadQueue::CheckAndAddKnownSource -- added potential wrong source (%u)(diff. partcount) to file \"%s\""), source->GetUserIDHybrid(), sender->GetFileName());
	}
#endif
	source->SetRequestFile(sender);
	sender->srclist.AddTail(source);
	source->SetSourceFrom(SF_PASSIVE);
	if (thePrefs.GetDebugSourceExchange())
		AddDebugLogLine(false, _T("SXRecv: Passively added source; %s, File=\"%s\""), source->DbgGetClientInfo(), sender->GetFileName());
#ifdef _DEBUG
	if (thePrefs.GetVerbose() && source->GetPartCount()!=0 && source->GetPartCount()!=sender->GetPartCount()){
		DEBUG_ONLY(AddDebugLogLine(false, _T("*** CDownloadQueue::CheckAndAddKnownSource -- New added source (%u, %s) had still value in partcount"), source->GetUserIDHybrid(), sender->GetFileName()));
	}
#endif

    //  Comment UI
    //uint32 struPFilePeer[] = {(uint32)sender,(uint32)source,(uint32)false};
	//SendMessage(CGlobalVariable::m_hListenWnd,WM_FILE_ADD_PEER,0,(LPARAM)struPFilePeer);
	SendMessage(CGlobalVariable::m_hListenWnd,WM_FILE_ADD_SOURCE_NA,(WPARAM)sender,(LPARAM)source);
	//theApp.emuledlg->transferwnd->downloadlistctrl.AddSource(sender,source,false);

	return true;
}

bool CDownloadQueue::RemoveSource(CUpDownClient* toremove, bool bDoStatsUpdate)
{
	bool bRemovedSrcFromPartFile = false;
	for (POSITION pos = filelist.GetHeadPosition();pos != 0;){
		CPartFile* cur_file = filelist.GetNext(pos);
		for (POSITION pos2 = cur_file->srclist.GetHeadPosition();pos2 != 0; cur_file->srclist.GetNext(pos2)){
			if (toremove == cur_file->srclist.GetAt(pos2)){
				cur_file->srclist.RemoveAt(pos2);
				bRemovedSrcFromPartFile = true;
				if ( bDoStatsUpdate ){
					cur_file->RemoveDownloadingSource(toremove);
					cur_file->UpdatePartsInfo();
				}
				break;
			}
		}
		if ( bDoStatsUpdate )
			cur_file->UpdateAvailablePartsCount();
	}
	
	// remove this source on all files in the downloadqueue who link this source
	// pretty slow but no way arround, maybe using a Map is better, but that's slower on other parts
	POSITION pos3, pos4;
	for(pos3 = toremove->m_OtherRequests_list.GetHeadPosition();(pos4=pos3)!=NULL;)
	{
		toremove->m_OtherRequests_list.GetNext(pos3);				
		POSITION pos5 = toremove->m_OtherRequests_list.GetAt(pos4)->A4AFsrclist.Find(toremove); 
		if(pos5)
		{ 
			toremove->m_OtherRequests_list.GetAt(pos4)->A4AFsrclist.RemoveAt(pos5);
			
			//  Comment UI
			//theApp.emuledlg->transferwnd->downloadlistctrl.RemoveSource(toremove,toremove->m_OtherRequests_list.GetAt(pos4));
			
			WPARAM wParam=(WPARAM)toremove->m_OtherRequests_list.GetAt(pos4);
			toremove->m_OtherRequests_list.RemoveAt(pos4);
			UINotify(WM_FILE_REMOVE_SOURCE,wParam,(LPARAM)toremove, toremove, true);
		}
	}
	for(pos3 = toremove->m_OtherNoNeeded_list.GetHeadPosition();(pos4=pos3)!=NULL;)
	{
		toremove->m_OtherNoNeeded_list.GetNext(pos3);				
		POSITION pos5 = toremove->m_OtherNoNeeded_list.GetAt(pos4)->A4AFsrclist.Find(toremove); 
		if(pos5)
		{ 
			toremove->m_OtherNoNeeded_list.GetAt(pos4)->A4AFsrclist.RemoveAt(pos5);
			//  Comment UI
			WPARAM wParam=(WPARAM)toremove->m_OtherNoNeeded_list.GetAt(pos4);
			toremove->m_OtherNoNeeded_list.RemoveAt(pos4);
			UINotify(WM_FILE_REMOVE_SOURCE, wParam,(LPARAM)toremove, toremove, true);
			//theApp.emuledlg->transferwnd->downloadlistctrl.RemoveSource(toremove,toremove->m_OtherNoNeeded_list.GetAt(pos4));
		}
	}

	if (bRemovedSrcFromPartFile && (toremove->HasFileRating() || !toremove->GetFileComment().IsEmpty()))
	{
		CPartFile* pFile = toremove->GetRequestFile();
		if(pFile)
			pFile->UpdateFileRatingCommentAvail();
	}

	toremove->SetDownloadState(DS_NONE);
	toremove->SetRequestFile(NULL);
    //  Comment UI
	UINotify(WM_FILE_REMOVE_SOURCE,0,(LPARAM)toremove, toremove, true);
	//SendMessage(CGlobalVariable::m_hListenWnd,WM_FILE_REMOVE_SOURCE,0,(LPARAM)toremove);
	//theApp.emuledlg->transferwnd->downloadlistctrl.RemoveSource(toremove,0);
	return bRemovedSrcFromPartFile;
}

void CDownloadQueue::RemoveFile(CPartFile* toremove)
{
	RemoveLocalServerRequest(toremove);

	for (POSITION pos = filelist.GetHeadPosition();pos != 0;filelist.GetNext(pos)){
		if (toremove == filelist.GetAt(pos)){
			filelist.RemoveAt(pos);
			break;
		}
	}
	SortByPriority();
	CheckDiskspace();
	ExportPartMetFilesOverview();
}

void CDownloadQueue::DeleteAll(){
	POSITION pos;
	for (pos = filelist.GetHeadPosition();pos != 0;){
		CPartFile* cur_file = filelist.GetNext(pos);
		cur_file->srclist.RemoveAll();
		// Barry - Should also remove all requested blocks
		// Don't worry about deleting the blocks, that gets handled 
		// when CUpDownClient is deleted in CClientList::DeleteAll()
		cur_file->RemoveAllRequestedBlocks();
	}
}

// Max. file IDs per UDP packet
// ----------------------------
// 576 - 30 bytes of header (28 for UDP, 2 for "E3 9A" edonkey proto) = 546 bytes
// 546 / 16 = 34
#define MAX_UDP_PACKET_DATA				510
#define BYTES_PER_FILE_G1				16
#define BYTES_PER_FILE_G2				20
#define ADDITIONAL_BYTES_PER_LARGEFILE	8

#define MAX_REQUESTS_PER_SERVER		35

bool CDownloadQueue::IsMaxFilesPerUDPServerPacketReached(uint32 nFiles, uint32 nIncludedLargeFiles) const
{
	if (cur_udpserver && cur_udpserver->GetUDPFlags() & SRV_UDPFLG_EXT_GETSOURCES)
	{
		
		const int nBytesPerNormalFile = ((cur_udpserver->GetUDPFlags() & SRV_UDPFLG_EXT_GETSOURCES2) > 0)? BYTES_PER_FILE_G2 : BYTES_PER_FILE_G1; 
		const int nUsedBytes = nFiles*nBytesPerNormalFile + nIncludedLargeFiles*ADDITIONAL_BYTES_PER_LARGEFILE;
		if (nIncludedLargeFiles > 0){
			ASSERT( cur_udpserver->SupportsLargeFilesUDP() );
			ASSERT( cur_udpserver->GetUDPFlags() & SRV_UDPFLG_EXT_GETSOURCES2 );
		}
		return (m_cRequestsSentToServer >= MAX_REQUESTS_PER_SERVER) || (nUsedBytes >= MAX_UDP_PACKET_DATA);
	}
	else{
		ASSERT( nIncludedLargeFiles == 0);
		return nFiles != 0;
	}
}

bool CDownloadQueue::SendGlobGetSourcesUDPPacket(CSafeMemFile* data, bool bExt2Packet, uint32 nFiles, uint32 nIncludedLargeFiles)
{
	bool bSentPacket = false;

	if (cur_udpserver)
	{
#ifdef _DEBUG
		int iPacketSize = (int)data->GetLength();
#endif
		Packet packet(data);
		data = NULL;
		if (bExt2Packet){
			ASSERT( iPacketSize > 0 && (uint32)iPacketSize == nFiles*20 + nIncludedLargeFiles*8);
			packet.opcode = OP_GLOBGETSOURCES2;
		}
		else{
			ASSERT( iPacketSize > 0 && (uint32)iPacketSize == nFiles*16 && nIncludedLargeFiles == 0);
			packet.opcode = OP_GLOBGETSOURCES;
		}
		if (thePrefs.GetDebugServerUDPLevel() > 0)
			Debug(_T(">>> Sending %s to server %-21s (%3u of %3u); FileIDs=%u(%u large)\n"), (packet.opcode == OP_GLOBGETSOURCES2) ? _T("OP__GlobGetSources2") : _T("OP__GlobGetSources1"), ipstr(cur_udpserver->GetAddress(), cur_udpserver->GetPort()), m_iSearchedServers + 1, CGlobalVariable::serverlist->GetServerCount(), nFiles, nIncludedLargeFiles);

		theStats.AddUpDataOverheadServer(packet.size);
		CGlobalVariable::serverconnect->SendUDPPacket(&packet, cur_udpserver, false);

		m_cRequestsSentToServer += nFiles;
		bSentPacket = true;
	}

	return bSentPacket;
}

bool CDownloadQueue::SendNextUDPPacket()
{
	if (   filelist.IsEmpty()
        || !CGlobalVariable::serverconnect->IsUDPSocketAvailable()
        || !CGlobalVariable::serverconnect->IsConnected()
		|| thePrefs.IsClientCryptLayerRequired()) // we cannot use sources received without userhash, so dont ask
		return false;

	CServer* pConnectedServer = CGlobalVariable::serverconnect->GetCurrentServer();
	if (pConnectedServer)
		pConnectedServer = CGlobalVariable::serverlist->GetServerByAddress(pConnectedServer->GetAddress(), pConnectedServer->GetPort());

	if (!cur_udpserver)
	{
		while ((cur_udpserver = CGlobalVariable::serverlist->GetSuccServer(cur_udpserver)) != NULL) {
			if (cur_udpserver == pConnectedServer)
				continue;
			if (cur_udpserver->GetFailedCount() >= thePrefs.GetDeadServerRetries())
				continue;
			break;
		}
		if (cur_udpserver == NULL) {
			StopUDPRequests();
			return false;
		}
		m_cRequestsSentToServer = 0;
	}

	bool bGetSources2Packet = (cur_udpserver->GetUDPFlags() & SRV_UDPFLG_EXT_GETSOURCES2) > 0;
	bool bServerSupportsLargeFiles = cur_udpserver->SupportsLargeFilesUDP();

	// loop until the packet is filled or a packet was sent
	bool bSentPacket = false;
	CSafeMemFile dataGlobGetSources(20);
	int iFiles = 0;
	int iLargeFiles = 0;
	while (!IsMaxFilesPerUDPServerPacketReached(iFiles, iLargeFiles) && !bSentPacket)
	{
		// get next file to search sources for
		CPartFile* nextfile = NULL;
		while (!bSentPacket && !(nextfile && (nextfile->GetStatus() == PS_READY || nextfile->GetStatus() == PS_EMPTY)))
		{
			if (lastfile == NULL) // we just started the global source searching or have switched the server
			{
				// get first file to search sources for
				nextfile = filelist.GetHead();
				lastfile = nextfile;
			}
			else
			{
				POSITION pos = filelist.Find(lastfile);
				if (pos == 0) // the last file is no longer in the DL-list (may have been finished or canceld)
				{
					// get first file to search sources for
					nextfile = filelist.GetHead();
					lastfile = nextfile;
				}
				else
				{
					filelist.GetNext(pos);
					if (pos == 0) // finished asking the current server for all files
					{
						// if there are pending requests for the current server, send them
						if (dataGlobGetSources.GetLength() > 0)
						{
							if (SendGlobGetSourcesUDPPacket(&dataGlobGetSources, bGetSources2Packet, iFiles, iLargeFiles))
								bSentPacket = true;
							dataGlobGetSources.SetLength(0);
							iFiles = 0;
							iLargeFiles = 0;
						}

						// get next server to ask
						while ((cur_udpserver = CGlobalVariable::serverlist->GetSuccServer(cur_udpserver)) != NULL) {
							if (cur_udpserver == pConnectedServer)
								continue;
							if (cur_udpserver->GetFailedCount() >= thePrefs.GetDeadServerRetries())
								continue;
							break;
						}
						m_cRequestsSentToServer = 0;
						if (cur_udpserver == NULL) {
							// finished asking all servers for all files
							if (thePrefs.GetDebugServerUDPLevel() > 0 && thePrefs.GetDebugServerSourcesLevel() > 0)
								Debug(_T("Finished UDP search processing for all servers (%u)\n"), CGlobalVariable::serverlist->GetServerCount());
							StopUDPRequests();
							return false; // finished (processed all file & all servers)
						}
						m_iSearchedServers++;

						// if we already sent a packet, switch to the next file at next function call
						if (bSentPacket){
							lastfile = NULL;
							break;
						}

						bGetSources2Packet = (cur_udpserver->GetUDPFlags() & SRV_UDPFLG_EXT_GETSOURCES2) > 0;
						bServerSupportsLargeFiles = cur_udpserver->SupportsLargeFilesUDP();

						// have selected a new server; get first file to search sources for
						nextfile = filelist.GetHead();
						lastfile = nextfile;
					}
					else
					{
						nextfile = filelist.GetAt(pos);
						lastfile = nextfile;
					}
				}
			}
		}

		if (!bSentPacket && nextfile && nextfile->GetSourceCount() < nextfile->GetMaxSourcePerFileUDP() && (bServerSupportsLargeFiles || !nextfile->IsLargeFile()) )
		{
			if (bGetSources2Packet){
				if (nextfile->IsLargeFile()){
					// GETSOURCES2 Packet Large File (<HASH_16><IND_4 = 0><SIZE_8> *)
					iLargeFiles++;
					dataGlobGetSources.WriteHash16(nextfile->GetFileHash());
					dataGlobGetSources.WriteUInt32(0);
					dataGlobGetSources.WriteUInt64(nextfile->GetFileSize());
				}
				else{
					// GETSOURCES2 Packet (<HASH_16><SIZE_4> *)
					dataGlobGetSources.WriteHash16(nextfile->GetFileHash());
					dataGlobGetSources.WriteUInt32((uint32)(uint64)nextfile->GetFileSize());
				}
			}
			else{
				// GETSOURCES Packet (<HASH_16> *)
				dataGlobGetSources.WriteHash16(nextfile->GetFileHash());
			}
			iFiles++;
			if (thePrefs.GetDebugServerUDPLevel() > 0 && thePrefs.GetDebugServerSourcesLevel() > 0)
				Debug(_T(">>> Queued  %s to server %-21s (%3u of %3u); Buff  %u(%u)=%s\n"), bGetSources2Packet ? _T("OP__GlobGetSources2") : _T("OP__GlobGetSources1"), ipstr(cur_udpserver->GetAddress(), cur_udpserver->GetPort()), m_iSearchedServers + 1, CGlobalVariable::serverlist->GetServerCount(), iFiles, iLargeFiles, DbgGetFileInfo(nextfile->GetFileHash()));
		}
	}

	ASSERT( dataGlobGetSources.GetLength() == 0 || !bSentPacket );

	if (!bSentPacket && dataGlobGetSources.GetLength() > 0)
		SendGlobGetSourcesUDPPacket(&dataGlobGetSources, bGetSources2Packet, iFiles, iLargeFiles);

	// send max 35 UDP request to one server per interval
	// if we have more than 35 files, we rotate the list and use it as queue
	if (m_cRequestsSentToServer >= MAX_REQUESTS_PER_SERVER)
	{
		if (thePrefs.GetDebugServerUDPLevel() > 0 && thePrefs.GetDebugServerSourcesLevel() > 0)
			Debug(_T("Rotating file list\n"));

		// move the last 35 files to the head
		if (filelist.GetCount() >= MAX_REQUESTS_PER_SERVER) {
			for (int i = 0; i != MAX_REQUESTS_PER_SERVER; i++)
				filelist.AddHead(filelist.RemoveTail());
		}

		// and next server
		while ((cur_udpserver = CGlobalVariable::serverlist->GetSuccServer(cur_udpserver)) != NULL) {
			if (cur_udpserver == pConnectedServer)
				continue;
			if (cur_udpserver->GetFailedCount() >= thePrefs.GetDeadServerRetries())
				continue;
			break;
		}
		m_cRequestsSentToServer = 0;
		if (cur_udpserver == NULL) {
			if (thePrefs.GetDebugServerUDPLevel() > 0 && thePrefs.GetDebugServerSourcesLevel() > 0)
				Debug(_T("Finished UDP search processing for all servers (%u)\n"), CGlobalVariable::serverlist->GetServerCount());
			StopUDPRequests();
			return false; // finished (processed all file & all servers)
		}
		m_iSearchedServers++;
		lastfile = NULL;
	}

	return true;
}

void CDownloadQueue::StopUDPRequests()
{
	cur_udpserver = NULL;
	lastudpsearchtime = ::GetTickCount();
	lastfile = NULL;
	m_iSearchedServers = 0;
}

bool CDownloadQueue::CompareParts(POSITION pos1, POSITION pos2){
	CPartFile* file1 = filelist.GetAt(pos1);
	CPartFile* file2 = filelist.GetAt(pos2);
    return CPartFile::RightFileHasHigherPrio(file1, file2);
}

void CDownloadQueue::SwapParts(POSITION pos1, POSITION pos2){
	CPartFile* file1 = filelist.GetAt(pos1);
	CPartFile* file2 = filelist.GetAt(pos2);
	filelist.SetAt(pos1, file2);
	filelist.SetAt(pos2, file1);
}

void CDownloadQueue::HeapSort(UINT first, UINT last)
{
	UINT r;
	POSITION pos1 = filelist.FindIndex(first);
	for ( r = first; !(r & (UINT)INT_MIN) && (r<<1) < last; ){
		UINT r2 = (r<<1)+1;
		POSITION pos2 = filelist.FindIndex(r2);
		if (r2 != last){
			POSITION pos3 = pos2;
			filelist.GetNext(pos3);
			if (!CompareParts(pos2, pos3)){
				pos2 = pos3;
				r2++;
			}
		}
		if (!CompareParts(pos1, pos2)) {
			SwapParts(pos1, pos2);
			r = r2;
			pos1 = pos2;
		}
		else
			break;
	}
}

void CDownloadQueue::SortByPriority()
{
	UINT n = filelist.GetCount();
	if (!n)
		return;
	UINT i;
	for ( i = n/2; i--; )
		HeapSort(i, n-1);
	for ( i = n; --i; ){
		SwapParts(filelist.FindIndex(0), filelist.FindIndex(i));
		HeapSort(0, i-1);
	}
}

void CDownloadQueue::CheckDiskspaceTimed()
{
	if ((!lastcheckdiskspacetime) || (::GetTickCount() - lastcheckdiskspacetime) > DISKSPACERECHECKTIME)
		CheckDiskspace();
}

void CDownloadQueue::CheckDiskspace(bool bNotEnoughSpaceLeft)
{
	lastcheckdiskspacetime = ::GetTickCount();

	// sorting the list could be done here, but I prefer to "see" that function call in the calling functions.
	//SortByPriority();

	// If disabled, resume any previously paused files
	if (!thePrefs.IsCheckDiskspaceEnabled())
	{
		if (!bNotEnoughSpaceLeft) // avoid worse case, if we already had 'disk full'
		{
			for( POSITION pos1 = filelist.GetHeadPosition(); pos1 != NULL; )
			{
				CPartFile* cur_file = filelist.GetNext(pos1);
				switch(cur_file->GetStatus())
				{
				case PS_PAUSED:
				case PS_ERROR:
				case PS_COMPLETING:
				case PS_COMPLETE:
					continue;
				}
				cur_file->ResumeFileInsufficient();
			}
		}
		return;
	}

	//Changed by thilon on 2008.05.04, 临时文件夹已经没用，用Incoming文件夹代替
	uint64 nTotalAvailableSpaceMain = bNotEnoughSpaceLeft ? 0 : GetFreeDiskSpaceX(thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR)/*thePrefs.GetTempDir()*/);

	// 'bNotEnoughSpaceLeft' - avoid worse case, if we already had 'disk full'
	if (thePrefs.GetMinFreeDiskSpace() == 0)
	{
		for( POSITION pos1 = filelist.GetHeadPosition(); pos1 != NULL; )
		{
			CPartFile* cur_file = filelist.GetNext(pos1);

			uint64 nTotalAvailableSpace = bNotEnoughSpaceLeft ? 0 : 
				((thePrefs.GetTempDirCount()==1)?nTotalAvailableSpaceMain:GetFreeDiskSpaceX(cur_file->GetTempPath()));

			switch(cur_file->GetStatus())
			{
			case PS_PAUSED:
			case PS_ERROR:
			case PS_COMPLETING:
			case PS_COMPLETE:
				continue;
			}

			// Pause the file only if it would grow in size and would exceed the currently available free space
			uint64 nSpaceToGo = cur_file->GetNeededSpace();
			if (nSpaceToGo <= nTotalAvailableSpace)
			{
				nTotalAvailableSpace -= nSpaceToGo;
				cur_file->ResumeFileInsufficient();
			}
			else
				cur_file->PauseFile(true/*bInsufficient*/);
		}
	}
	else
	{
		for( POSITION pos1 = filelist.GetHeadPosition(); pos1 != NULL; )
		{
			CPartFile* cur_file = filelist.GetNext(pos1);
			switch(cur_file->GetStatus())
			{
			case PS_PAUSED:
			case PS_ERROR:
			case PS_COMPLETING:
			case PS_COMPLETE:
				continue;
			}

			CString str = cur_file->GetTempPath();
			if (str.IsEmpty())
				str = cur_file->GetPath();

			uint64 nTotalAvailableSpace = bNotEnoughSpaceLeft ? 0 : 
				((thePrefs.GetTempDirCount()==1)?nTotalAvailableSpaceMain:GetFreeDiskSpaceX(str));

			if (nTotalAvailableSpace < thePrefs.GetMinFreeDiskSpace())
			{
				if (cur_file->IsNormalFile())
				{
					// Normal files: pause the file only if it would still grow
					uint64 nSpaceToGrow = cur_file->GetNeededSpace();
					if (nSpaceToGrow > 0)
						cur_file->PauseFile(true/*bInsufficient*/);
				}
				else
				{
					// Compressed/sparse files: always pause the file
					cur_file->PauseFile(true/*bInsufficient*/);
				}
			}
			else
			{
				// doesn't work this way. resuming the file without checking if there is a chance to successfully
				// flush any available buffered file data will pause the file right after it was resumed and disturb
				// the StopPausedFile function.
				//cur_file->ResumeFileInsufficient();
			}
		}
	}
}

void CDownloadQueue::GetDownloadStats(SDownloadStats& results)
{
	memset(&results, 0, sizeof results);
	for (POSITION pos = filelist.GetHeadPosition(); pos != 0; )
	{
		const CPartFile* cur_file = filelist.GetNext(pos);

		results.a[0]  += cur_file->GetSourceCount();
		results.a[1]  += cur_file->GetTransferringSrcCount();
		results.a[2]  += cur_file->GetSrcStatisticsValue(DS_ONQUEUE);
		results.a[3]  += cur_file->GetSrcStatisticsValue(DS_REMOTEQUEUEFULL);
		results.a[4]  += cur_file->GetSrcStatisticsValue(DS_NONEEDEDPARTS);
		results.a[5]  += cur_file->GetSrcStatisticsValue(DS_CONNECTED);
		results.a[6]  += cur_file->GetSrcStatisticsValue(DS_REQHASHSET);
		results.a[7]  += cur_file->GetSrcStatisticsValue(DS_CONNECTING);
		results.a[8]  += cur_file->GetSrcStatisticsValue(DS_WAITCALLBACK);
		results.a[8]  += cur_file->GetSrcStatisticsValue(DS_WAITCALLBACKKAD);
		results.a[9]  += cur_file->GetSrcStatisticsValue(DS_TOOMANYCONNS);
		results.a[9]  += cur_file->GetSrcStatisticsValue(DS_TOOMANYCONNSKAD);
		results.a[10] += cur_file->GetSrcStatisticsValue(DS_LOWTOLOWIP);
		results.a[11] += cur_file->GetSrcStatisticsValue(DS_NONE);
		results.a[12] += cur_file->GetSrcStatisticsValue(DS_ERROR);
		results.a[13] += cur_file->GetSrcStatisticsValue(DS_BANNED);
		results.a[14] += cur_file->src_stats[3];
		results.a[15] += cur_file->GetSrcA4AFCount();
		results.a[16] += cur_file->src_stats[0];
		results.a[17] += cur_file->src_stats[1];
		results.a[18] += cur_file->src_stats[2];
		results.a[19] += cur_file->net_stats[0];
		results.a[20] += cur_file->net_stats[1];
		results.a[21] += cur_file->net_stats[2];
		results.a[22] += cur_file->m_DeadSourceList.GetDeadSourcesCount();
	}
}

CUpDownClient* CDownloadQueue::GetDownloadClientByIP(uint32 dwIP){
	for (POSITION pos = filelist.GetHeadPosition();pos != 0;){
		CPartFile* cur_file = filelist.GetNext(pos);
		for (POSITION pos2 = cur_file->srclist.GetHeadPosition();pos2 != 0; ){
			CUpDownClient* cur_client = cur_file->srclist.GetNext(pos2);
			if (dwIP == cur_client->GetIP()){
				return cur_client;
			}
		}
	}
	return NULL;
}

CUpDownClient* CDownloadQueue::GetDownloadClientByIP_UDP(uint32 dwIP, uint16 nUDPPort, bool bIgnorePortOnUniqueIP, bool* pbMultipleIPs){
	CUpDownClient* pMatchingIPClient = NULL;
	uint32 cMatches = 0;

	for (POSITION pos = filelist.GetHeadPosition();pos != 0;){
		CPartFile* cur_file = filelist.GetNext(pos);
		for (POSITION pos2 = cur_file->srclist.GetHeadPosition();pos2 != 0;){
			CUpDownClient* cur_client = cur_file->srclist.GetNext(pos2);
			if (dwIP == cur_client->GetIP() && nUDPPort == cur_client->GetUDPPort()){
				return cur_client;
			}
			else if (dwIP == cur_client->GetIP() && bIgnorePortOnUniqueIP && cur_client != pMatchingIPClient){
				pMatchingIPClient = cur_client;
				cMatches++;
			}
		}
	}
	if (pbMultipleIPs != NULL)
		*pbMultipleIPs = cMatches > 1;

	if (pMatchingIPClient != NULL && cMatches == 1)
		return pMatchingIPClient;
	else
		return NULL;
}

bool CDownloadQueue::IsInList(const CUpDownClient* client) const
{
	for (POSITION pos = filelist.GetHeadPosition();pos != 0;){
		CPartFile* cur_file = filelist.GetNext(pos);
		for (POSITION pos2 = cur_file->srclist.GetHeadPosition();pos2 != 0;){
			if (cur_file->srclist.GetNext(pos2) == client)
				return true;
		}
	}
	return false;
}

void CDownloadQueue::ResetCatParts(UINT cat)
{
	CPartFile* cur_file;

	for (POSITION pos = filelist.GetHeadPosition(); pos != 0; ){
		cur_file = filelist.GetNext(pos);

		if (cur_file->GetCategory() == cat)
			cur_file->SetCategory(0);
		else if (cur_file->GetCategory() > cat)
			cur_file->SetCategory(cur_file->GetCategory() - 1);
	}
}

void CDownloadQueue::SetCatPrio(UINT cat, uint8 newprio)
{
	for (POSITION pos = filelist.GetHeadPosition(); pos != 0; ){
		CPartFile* cur_file = filelist.GetNext(pos);
		if (cat==0 || cur_file->GetCategory()==cat)
			if (newprio==PR_AUTO) {
				cur_file->SetAutoDownPriority(true);
				cur_file->SetDownPriority(PR_HIGH, false);
			}
			else {
				cur_file->SetAutoDownPriority(false);
				cur_file->SetDownPriority(newprio, false);
			}
	}

    CGlobalVariable::downloadqueue->SortByPriority();
	CGlobalVariable::downloadqueue->CheckDiskspaceTimed();
}

// ZZ:DownloadManager -->
void CDownloadQueue::RemoveAutoPrioInCat(UINT cat, uint8 newprio){
	CPartFile* cur_file;
	for (POSITION pos = filelist.GetHeadPosition();pos != 0;filelist.GetNext(pos)){
		cur_file = filelist.GetAt(pos);
        if (cur_file->IsAutoDownPriority() && (cat==0 || cur_file->GetCategory()==cat)) {
			cur_file->SetAutoDownPriority(false);
			cur_file->SetDownPriority(newprio, false);
		}
	}

    CGlobalVariable::downloadqueue->SortByPriority();
	CGlobalVariable::downloadqueue->CheckDiskspaceTimed();
}
// <-- ZZ:DownloadManager

void CDownloadQueue::SetCatStatus(UINT cat, int newstatus)
{
	bool reset = false;
    bool resort = false;

	POSITION pos= filelist.GetHeadPosition();
	while (pos != 0)
	{
		CPartFile* cur_file = filelist.GetAt(pos);
		if (!cur_file)
			continue;

		if (cat==-1 || 
			(cat==-2 && cur_file->GetCategory()==0) ||
			(cat==0 && cur_file->CheckShowItemInGivenCat(cat)) || 
			(cat>0 && cat==cur_file->GetCategory()))
		{
			switch (newstatus){
				case MP_CANCEL:
					cur_file->DeleteFile();
					reset = true;
					break;
				case MP_PAUSE:
					cur_file->PauseFile(false, false);
                    resort = true;
					break;
				case MP_STOP:
					cur_file->StopFile(false, false);
                    resort = true;
					break;
				case MP_RESUME: 
					if (cur_file->CanResumeFile()){
						if (cur_file->GetStatus() == PS_INSUFFICIENT)
							cur_file->ResumeFileInsufficient();
                        else {
							cur_file->ResumeFile(false);
                            resort = true;
                        }
					}
					break;
			}
		}
		filelist.GetNext(pos);
		if (reset)
		{
			reset = false;
			pos = filelist.GetHeadPosition();
		}
	}

    if(resort) {
	    CGlobalVariable::downloadqueue->SortByPriority();
	    CGlobalVariable::downloadqueue->CheckDiskspace();
    }
}

void CDownloadQueue::MoveCat(UINT from, UINT to)
{
	if (from < to)
		--to;

	POSITION pos= filelist.GetHeadPosition();
	while (pos != 0)
	{
		CPartFile* cur_file = filelist.GetAt(pos);
		if (!cur_file)
			continue;

		UINT mycat = cur_file->GetCategory();
		if ((mycat>=min(from,to) && mycat<=max(from,to)))
		{
			//if ((from<to && (mycat<from || mycat>to)) || (from>to && (mycat>from || mycat<to)) )	continue; //not affected

			if (mycat == from)
				cur_file->SetCategory(to);
			else{
				if (from < to)
					cur_file->SetCategory(mycat - 1);
				else
					cur_file->SetCategory(mycat + 1);
			}
		}
		filelist.GetNext(pos);
	}
}

UINT CDownloadQueue::GetDownloadingFileCount() const
{
	UINT result = 0;
	for (POSITION pos = filelist.GetHeadPosition();pos != 0;){
		UINT uStatus = filelist.GetNext(pos)->GetStatus();
		if (uStatus == PS_READY || uStatus == PS_EMPTY)
			result++;
	}
	return result;
}

UINT CDownloadQueue::GetPausedFileCount() const
{
	UINT result = 0;
	for (POSITION pos = filelist.GetHeadPosition();pos != 0;){
		if (filelist.GetNext(pos)->GetStatus() == PS_PAUSED)
			result++;
	}
	return result;
}

void CDownloadQueue::SetAutoCat(CPartFile* newfile){
	if(thePrefs.GetCatCount()==1)
		return;
	CString catExt;

	for (int ix=1;ix<thePrefs.GetCatCount();ix++){	
		catExt= thePrefs.GetCategory(ix)->autocat;
		if (catExt.IsEmpty())
			continue;

		if (!thePrefs.GetCategory(ix)->ac_regexpeval) {
			// simple string comparison

			int curPos = 0;
			catExt.MakeLower();

			CString fullname = newfile->GetFileName();
			fullname.MakeLower();
			CString cmpExt = catExt.Tokenize(_T("|"), curPos);

			while (!cmpExt.IsEmpty()) {
				// HoaX_69: Allow wildcards in autocat string
				//  thanks to: bluecow, khaos and SlugFiller
				if(cmpExt.Find(_T("*")) != -1 || cmpExt.Find(_T("?")) != -1){
					// Use wildcards
					if(PathMatchSpec(fullname, cmpExt)){
						newfile->SetCategory(ix);
						return;
					}
				}else{
					if(fullname.Find(cmpExt) != -1){
						newfile->SetCategory(ix);
						return;
					}
				}
				cmpExt = catExt.Tokenize(_T("|"),curPos);
			}
		} else {
			// regular expression evaluation
			if (RegularExpressionMatch(catExt,newfile->GetFileName()))
				newfile->SetCategory(ix);
		}
	}
}

void CDownloadQueue::ResetLocalServerRequests()
{
	m_dwNextTCPSrcReq = 0;
	m_localServerReqQueue.RemoveAll();

	POSITION pos = filelist.GetHeadPosition();
	while (pos != NULL)
	{ 
		CPartFile* pFile = filelist.GetNext(pos);
		UINT uState = pFile->GetStatus();
		if( (uState == PS_READY || uState == PS_EMPTY) && !pFile->HasNullHash() )
			pFile->ResumeFile();
		pFile->m_bLocalSrcReqQueued = false;
	}
}

void CDownloadQueue::RemoveLocalServerRequest(CPartFile* pFile)
{
	POSITION pos1, pos2;
	for( pos1 = m_localServerReqQueue.GetHeadPosition(); ( pos2 = pos1 ) != NULL; )
	{
		m_localServerReqQueue.GetNext(pos1);
		if (m_localServerReqQueue.GetAt(pos2) == pFile)
		{
			m_localServerReqQueue.RemoveAt(pos2);
			pFile->m_bLocalSrcReqQueued = false;
			// could 'break' here.. fail safe: go through entire list..
		}
	}
}

void CDownloadQueue::ProcessLocalRequests()
{
	if ( (!m_localServerReqQueue.IsEmpty()) && (m_dwNextTCPSrcReq < ::GetTickCount()) )
	{
		CSafeMemFile dataTcpFrame(22);
		const int iMaxFilesPerTcpFrame = 15;
		int iFiles = 0;
		while (!m_localServerReqQueue.IsEmpty() && iFiles < iMaxFilesPerTcpFrame)
		{
			// find the file with the longest waitingtime
			POSITION pos1, pos2;
			uint32 dwBestWaitTime = 0xFFFFFFFF;
			POSITION posNextRequest = NULL;
			CPartFile* cur_file;
			for( pos1 = m_localServerReqQueue.GetHeadPosition(); ( pos2 = pos1 ) != NULL; ){
				m_localServerReqQueue.GetNext(pos1);
				cur_file = m_localServerReqQueue.GetAt(pos2);
				if (cur_file->GetStatus() == PS_READY || cur_file->GetStatus() == PS_EMPTY)
				{
					uint8 nPriority = cur_file->GetDownPriority();
					if (nPriority > PR_HIGH){
						ASSERT(0);
						nPriority = PR_HIGH;
					}

					if (cur_file->m_LastSearchTime + (PR_HIGH-nPriority) < dwBestWaitTime ){
						dwBestWaitTime = cur_file->m_LastSearchTime + (PR_HIGH-nPriority);
						posNextRequest = pos2;
					}
				}
				else{
					m_localServerReqQueue.RemoveAt(pos2);
					cur_file->m_bLocalSrcReqQueued = false;
					if (thePrefs.GetDebugSourceExchange())
						AddDebugLogLine(false, _T("SXSend: Local server source request for file \"%s\" not sent because of status '%s'"), cur_file->GetFileName(), cur_file->getPartfileStatus());
				}
			}
			
			if (posNextRequest != NULL)
			{
				cur_file = m_localServerReqQueue.GetAt(posNextRequest);
				cur_file->m_bLocalSrcReqQueued = false;
				cur_file->m_LastSearchTime = ::GetTickCount();
				m_localServerReqQueue.RemoveAt(posNextRequest);

				if (cur_file->IsLargeFile() && (CGlobalVariable::serverconnect->GetCurrentServer() == NULL || !CGlobalVariable::serverconnect->GetCurrentServer()->SupportsLargeFilesTCP())){
					ASSERT( false );
					DebugLogError(_T("Large file (%s) on local requestqueue for server without support for large files"), cur_file->GetFileName());
					continue;
				}

				iFiles++;
				
				// create request packet
				CSafeMemFile smPacket;
				smPacket.WriteHash16(cur_file->GetFileHash());
				if (!cur_file->IsLargeFile()){
					smPacket.WriteUInt32((uint32)(uint64)cur_file->GetFileSize());
				}
				else{
					smPacket.WriteUInt32(0); // indicates that this is a large file and a uint64 follows
					smPacket.WriteUInt64(cur_file->GetFileSize());
				}

				uint8 byOpcode = 0;
				if (thePrefs.IsClientCryptLayerSupported() && CGlobalVariable::serverconnect->GetCurrentServer() != NULL && CGlobalVariable::serverconnect->GetCurrentServer()->SupportsGetSourcesObfuscation())
					byOpcode = OP_GETSOURCES_OBFU;
				else
					byOpcode = OP_GETSOURCES;
	
				Packet* packet = new Packet(&smPacket, OP_EDONKEYPROT, byOpcode);
				if (thePrefs.GetDebugServerTCPLevel() > 0)
					Debug(_T(">>> Sending OP__GetSources%s(%2u/%2u); %s\n"), (byOpcode == OP_GETSOURCES) ? _T("") : _T("_OBFU"), iFiles, iMaxFilesPerTcpFrame, DbgGetFileInfo(cur_file->GetFileHash()));
				dataTcpFrame.Write(packet->GetPacket(), packet->GetRealPacketSize());
				delete packet;

				if (thePrefs.GetDebugSourceExchange())
					AddDebugLogLine(false, _T("SXSend: Local server source request; File=\"%s\""), cur_file->GetFileName());
			}
		}

		int iSize = (int)dataTcpFrame.GetLength();
		if (iSize > 0)
		{
			// create one 'packet' which contains all buffered OP_GETSOURCES eD2K packets to be sent with one TCP frame
			// server credits: 16*iMaxFilesPerTcpFrame+1 = 241
			Packet* packet = new Packet(new char[iSize], (UINT)dataTcpFrame.GetLength(), true, false);
			dataTcpFrame.Seek(0, CFile::begin);
			dataTcpFrame.Read(packet->GetPacket(), iSize);
			theStats.AddUpDataOverheadServer(packet->size);
			CGlobalVariable::serverconnect->SendPacket(packet, true);
		}

		// next TCP frame with up to 15 source requests is allowed to be sent in..
		m_dwNextTCPSrcReq = ::GetTickCount() + SEC2MS(iMaxFilesPerTcpFrame*(16+4));
	}
}

void CDownloadQueue::SendLocalSrcRequest(CPartFile* sender){
	ASSERT ( !m_localServerReqQueue.Find(sender) );
	m_localServerReqQueue.AddTail(sender);
}

void CDownloadQueue::GetDownloadStats(int results[],
									  uint64& rui64TotFileSize,
									  uint64& rui64TotBytesLeftToTransfer,
									  uint64& rui64TotNeededSpace)
{
	results[0] = 0;
	results[1] = 0;
	results[2] = 0;
	for (POSITION pos = filelist.GetHeadPosition();pos != 0; )
	{
		const CPartFile* cur_file = filelist.GetNext(pos);
		UINT uState = cur_file->GetStatus();
		if (uState == PS_READY || uState == PS_EMPTY)
		{
			uint64 ui64SizeToTransfer = 0;
			uint64 ui64NeededSpace = 0;
			cur_file->GetSizeToTransferAndNeededSpace(ui64SizeToTransfer, ui64NeededSpace);
			rui64TotFileSize += (uint64)cur_file->GetFileSize();
			rui64TotBytesLeftToTransfer += ui64SizeToTransfer;
			rui64TotNeededSpace += ui64NeededSpace;
			results[2]++;
		}
		results[0] += cur_file->GetSourceCount();
		results[1] += cur_file->GetTransferringSrcCount();
	}
}

bool CDownloadQueue::DoKademliaFileRequest()
{
	return ((::GetTickCount() - lastkademliafilerequest) > KADEMLIAASKTIME);
}

void CDownloadQueue::KademliaSearchFile(uint32 searchID, const Kademlia::CUInt128* pcontactID, const Kademlia::CUInt128* pbuddyID, uint8 type, uint32 ip, uint16 tcp, uint16 udp, uint32 dwBuddyIP, uint16 dwBuddyPort, uint8 byCryptOptions)
{
	//Safty measure to make sure we are looking for these sources
	CPartFile* temp = GetFileByKadFileSearchID(searchID);
	if( !temp || temp->IsStopped() )
		return;

	//Do we need more sources?
	if( temp->GetMaxSources() < temp->GetSourceCount() )
	{  
		int iMaxDownload = (thePrefs.maxdownload==0xFFFF ? 256*1024 : thePrefs.maxdownload)*1024*4/5;
		if( GetDatarate()>(uint32)iMaxDownload )
			return; //速度已经不错了,太多源的情况下可以抛弃一些源
		if( temp->GetMaxSources()*1.5 < temp->GetSourceCount() )
			return; //超过1.5倍的全部抛弃
	}

	uint32 ED2Kip = ntohl(ip);
	if (CGlobalVariable::ipfilter->IsFiltered(ED2Kip))
	{
		if (thePrefs.GetLogFilteredIPs())
			AddDebugLogLine(false, _T("IPfiltered source IP=%s (%s) received from Kademlia"), ipstr(ED2Kip), CGlobalVariable::ipfilter->GetLastHit());
		return;
	}
	if( (ip == Kademlia::CKademlia::GetIPAddress() || ED2Kip == CGlobalVariable::serverconnect->GetClientID()) && tcp == thePrefs.GetPort())
		return;
	CUpDownClient* ctemp = NULL; 
	//DEBUG_ONLY( DebugLog(_T("Kadsource received, type %u, IP %s"), type, ipstr(ED2Kip)) );
	switch( type )
	{
		case 4:
		case 1:
		{
			//NonFirewalled users
			if(!tcp)
			{
				if (thePrefs.GetVerbose())
					AddDebugLogLine(false, _T("Ignored source (IP=%s) received from Kademlia, no tcp port received"), ipstr(ip));
				return;
			}
			ctemp = new CEd2kUpDownClient(temp,tcp,ip,0,0,false);
			ctemp->SetSourceFrom(SF_KADEMLIA);
			// not actually sent or needed for HighID sources
			//ctemp->SetServerIP(serverip);
			//ctemp->SetServerPort(serverport);
			ctemp->SetKadPort(udp);
			byte cID[16];
			pcontactID->ToByteArray(cID);
			ctemp->SetUserHash(cID);
			break;
		}
		case 2:
		{
			//Don't use this type... Some clients will process it wrong..
			break;
		}
		case 5:
		case 3:
		{
			//This will be a firewaled client connected to Kad only.
			// if we are firewalled ourself, the source is useless to us
			//if (theApp.IsFirewalled())
			//	break; //(VeryCD mod Support L2L)

			//We set the clientID to 1 as a Kad user only has 1 buddy.
			ctemp = new CEd2kUpDownClient(temp,tcp,1,0,0,false);
			//The only reason we set the real IP is for when we get a callback
			//from this firewalled source, the compare method will match them.
			ctemp->SetSourceFrom(SF_KADEMLIA);
			ctemp->SetKadPort(udp);
			byte cID[16];
			pcontactID->ToByteArray(cID);
			ctemp->SetUserHash(cID);
			pbuddyID->ToByteArray(cID);
			ctemp->SetBuddyID(cID);
			ctemp->SetBuddyIP(dwBuddyIP);
			ctemp->SetBuddyPort(dwBuddyPort);
			break;
		}
		case 6:
		{
			// firewalled source which supports direct udp callback
			// if we are firewalled ourself, the source is useless to us
			//if (theApp.IsFirewalled())
			//	break; //(VeryCD mod Support L2L)

			if ((byCryptOptions & 0x08) == 0){
				DebugLogWarning(_T("Received Kad source type 6 (direct callback) which has the direct callback flag not set (%s)"), ipstr(ED2Kip));
				break;
			}
			ctemp = new CEd2kUpDownClient(temp, tcp, 1, 0, 0, false);
			ctemp->SetSourceFrom(SF_KADEMLIA);
			ctemp->SetKadPort(udp);
			ctemp->SetIP(ED2Kip); // need to set the Ip address, which cannot be used for TCP but for UDP
			byte cID[16];
			pcontactID->ToByteArray(cID);
			ctemp->SetUserHash(cID);
		}
	}

	if (ctemp != NULL){
		// add encryption settings
		ctemp->SetConnectOptions(byCryptOptions);

		//  added by yunchenn 2006/12/13
		//if(tcp==(ULONG)-1)
		{
			ctemp->SetIP(htonl(ip));
		}

		CheckAndAddSource(temp, ctemp);
	}
}

void CDownloadQueue::ExportPartMetFilesOverview() const
{
	CString strFileListPath = thePrefs.GetMuleDirectory(EMULE_DATABASEDIR) + _T("downloads.txt");
	
	CString strTmpFileListPath = strFileListPath;
	PathRenameExtension(strTmpFileListPath.GetBuffer(MAX_PATH), _T(".tmp"));
	strTmpFileListPath.ReleaseBuffer();

	CSafeBufferedFile file;
	CFileException fexp;
	if (!file.Open(strTmpFileListPath, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary | CFile::shareDenyWrite, &fexp))
	{
		CString strError;
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		if (fexp.GetErrorMessage(szError, ARRSIZE(szError))){
			strError += _T(" - ");
			strError += szError;
		}
		LogError(_T("Failed to create part.met file list%s"), strError);
		return;
	}

	// write Unicode byte-order mark 0xFEFF
	fputwc(0xFEFF, file.m_pStream);

	try
	{
		file.printf(_T("Date:      %s\r\n"), CTime::GetCurrentTime().Format(_T("%c")));
		if (thePrefs.GetTempDirCount()==1)
			file.printf(_T("Directory: %s\r\n"), thePrefs.GetTempDir());
		file.printf(_T("\r\n"));
		file.printf(_T("Part file\teD2K link\r\n"));
		file.printf(_T("--------------------------------------------------------------------------------\r\n"));
		for (POSITION pos = filelist.GetHeadPosition(); pos != 0; )
		{
			const CPartFile* pPartFile = filelist.GetNext(pos);
			if (pPartFile->GetStatus(true) != PS_COMPLETE)
			{
				CString strPartFilePath(pPartFile->GetFilePath());
				TCHAR szNam[_MAX_FNAME];
				TCHAR szExt[_MAX_EXT];
				_tsplitpath(strPartFilePath, NULL, NULL, szNam, szExt);
				if (thePrefs.GetTempDirCount()==1)
					file.printf(_T("%s%s\t%s\r\n"), szNam, szExt, CreateED2kLink(pPartFile));
				else
					file.printf(_T("%s\t%s\r\n"), pPartFile->GetFullName(), CreateED2kLink(pPartFile));
			}
		}

		//  Comment UI
		if(CGlobalVariable::IsRunning())
		{
			file.Flush(); // flush file stream buffers to disk buffers
			if (_commit(_fileno(file.m_pStream)) != 0) // commit disk buffers to disk
				AfxThrowFileException(CFileException::hardIO, GetLastError(), file.GetFileName());
			}
		/*if (thePrefs.GetCommitFiles() >= 2 || (thePrefs.GetCommitFiles() >= 1 && !theApp.emuledlg->IsRunning())){
			file.Flush(); // flush file stream buffers to disk buffers
			if (_commit(_fileno(file.m_pStream)) != 0) // commit disk buffers to disk
				AfxThrowFileException(CFileException::hardIO, GetLastError(), file.GetFileName());
		}*/
		file.Close();

		CString strBakFileListPath = strFileListPath;
		PathRenameExtension(strBakFileListPath.GetBuffer(MAX_PATH), _T(".bak"));
		strBakFileListPath.ReleaseBuffer();

		ASSERT(strBakFileListPath.Right(8).CollateNoCase(_T(".bak.bak")));

		if (_taccess(strBakFileListPath, 0) == 0)
			CFile::Remove(strBakFileListPath);
		if (_taccess(strFileListPath, 0) == 0)
			CFile::Rename(strFileListPath, strBakFileListPath);
		CFile::Rename(strTmpFileListPath, strFileListPath);
	}
	catch(CFileException* e)
	{
		CString strError;
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		if (e->GetErrorMessage(szError, ARRSIZE(szError))){
			strError += _T(" - ");
			strError += szError;
		}
		LogError(_T("Failed to write part.met file list%s"), strError);
		e->Delete();
		file.Abort();
		(void)_tremove(file.GetFilePath());
	}
}

void CDownloadQueue::OnConnectionState(bool bConnected)
{
	for (POSITION pos = filelist.GetHeadPosition(); pos != 0; )
	{
		CPartFile* pPartFile = filelist.GetNext(pos);
		if (pPartFile->GetStatus() == PS_READY || pPartFile->GetStatus() == PS_EMPTY)
			pPartFile->SetActive(bConnected);
	}
}

CString CDownloadQueue::GetOptimalTempDir(UINT nCat, EMFileSize nFileSize){
	// shortcut
	if (thePrefs.tempdir.GetCount() == 1)
		return thePrefs.GetTempDir();

	CMap<int, int, sint64, sint64> mapNeededSpaceOnDrive;
	CMap<int, int, sint64, sint64> mapFreeSpaceOnDrive;
	
	sint64 llBuffer = 0;
	sint64 llHighestFreeSpace = 0;
	int	nHighestFreeSpaceDrive = -1;
	// first collect the free space on drives
	for (int i = 0; i < thePrefs.tempdir.GetCount(); i++) {
		const int nDriveNumber = GetPathDriveNumber(thePrefs.GetTempDir(i));
		if (mapFreeSpaceOnDrive.Lookup(nDriveNumber, llBuffer))
			continue;
		llBuffer = GetFreeDiskSpaceX(thePrefs.GetTempDir(i)) - thePrefs.GetMinFreeDiskSpace();
		mapFreeSpaceOnDrive.SetAt(nDriveNumber, llBuffer);
		if (llBuffer > llHighestFreeSpace){
			nHighestFreeSpaceDrive = nDriveNumber;
			llHighestFreeSpace = llBuffer;
		}
		
	}

	// now get the space we would need to download all files in the current queue
	POSITION pos = filelist.GetHeadPosition();
	while (pos != NULL){
		CPartFile* pCurFile =  filelist.GetNext(pos);
		const int nDriveNumber = GetPathDriveNumber(pCurFile->GetTempPath());

		sint64 llNeededForCompletion = 0;
		switch(pCurFile->GetStatus(false)){
			case PS_READY:
			case PS_EMPTY:
			case PS_WAITINGFORHASH:
			case PS_INSUFFICIENT:
				llNeededForCompletion = pCurFile->GetFileSize() - pCurFile->GetRealFileSize();
				if (llNeededForCompletion < 0)
					llNeededForCompletion = 0;
		}
		llBuffer = 0;
		mapNeededSpaceOnDrive.Lookup(nDriveNumber, llBuffer);
		llBuffer += llNeededForCompletion;
		mapNeededSpaceOnDrive.SetAt(nDriveNumber, llBuffer);
	}

	sint64 llHighestTotalSpace = 0;
	int	nHighestTotalSpaceDir = -1;
	int	nHighestFreeSpaceDir = -1;
	int	nAnyAvailableDir = -1;
	// first round (0): on same drive as incomming and enough space for all downloading
	// second round (1): enough space for all downloading
	// third round (2): most actual free space
	for (int i = 0; i < thePrefs.tempdir.GetCount(); i++) {
		const int nDriveNumber = GetPathDriveNumber(thePrefs.GetTempDir(i));
		llBuffer = 0;

		sint64 llAvailableSpace = 0;
		mapFreeSpaceOnDrive.Lookup(nDriveNumber, llAvailableSpace);
		mapNeededSpaceOnDrive.Lookup(nDriveNumber, llBuffer);
		llAvailableSpace -= llBuffer;

		// no condition can be met for a large file on a FAT volume
		if (nFileSize <= OLD_MAX_EMULE_FILE_SIZE || !IsFileOnFATVolume(thePrefs.GetTempDir(i))){
			// condition 0
			// needs to be same drive and enough space
			if (GetPathDriveNumber(thePrefs.GetCatPath(nCat)) == nDriveNumber &&
				llAvailableSpace > (sint64)nFileSize)
			{
				//this one is perfect
				return thePrefs.GetTempDir(i);
			}
			// condition 1
			// needs to have enough space for downloading
			if (llAvailableSpace > (sint64)nFileSize && llAvailableSpace > llHighestTotalSpace){
				llHighestTotalSpace = llAvailableSpace;
				nHighestTotalSpaceDir = i;
			}
			// condition 2
			// first one which has the highest actualy free space
			if ( nDriveNumber == nHighestFreeSpaceDrive && nHighestFreeSpaceDir == (-1)){
				nHighestFreeSpaceDir = i;
			}
			// condition 3
			// any directory which can be used for this file (ak not FAT for large files)
			if ( nAnyAvailableDir == (-1)){
				nAnyAvailableDir = i;
			}
		}
	}

	if (nHighestTotalSpaceDir != (-1)){	 //condtion 0 was apperently too much, take 1
		return thePrefs.GetTempDir(nHighestTotalSpaceDir);
	}
	else if (nHighestFreeSpaceDir != (-1)){ // condtion 1 could not be met too, take 2
		return thePrefs.GetTempDir(nHighestFreeSpaceDir);
	}
	else if( nAnyAvailableDir != (-1)){
		return thePrefs.GetTempDir(nAnyAvailableDir);
	}
	else{ // so was condtion 2 and 3, take 4.. wait there is no 3 - this must be a bug
		ASSERT( false );
		return thePrefs.GetTempDir();
	}
}

BOOL CDownloadQueue::IsAnyTaskRunning()
{
	BOOL		bRunning = FALSE;
	CPartFile	*pPartFile = NULL;

	POSITION	pos = filelist.GetHeadPosition();
	while (NULL != pos)
	{
		pPartFile = filelist.GetNext(pos);
		if (NULL != pPartFile)
		{
			if (!pPartFile->IsStopped()
				&& !pPartFile->IsPaused())
			{
				bRunning = TRUE;
				break;
			}
		}
	}

	return bRunning;
}

