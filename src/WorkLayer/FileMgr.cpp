/*
 * $Id: FileMgr.cpp 11398 2009-03-17 11:00:27Z huby $
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
#include "StdAfx.h"
#include ".\filemgr.h"
#include "Preferences.h"
#include "GlobalVariable.h"
#include "UIMessage.h"
#include "ED2KLink.h"
#include "resource.h"
#include <io.h>

#include "DNSManager.h"
#include "SourceURL.h"

#include "UserMsgs.h"
#include "WndMgr.h"
#include "CmdFuncs.h"
#include "StringConversion.h"

#include "DownloadedListCtrl.h"

#define FM_LOCK  CSingleLock lock(&m_Mutex, true)

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CFileMgr::CFileMgr(void)
{
	m_bLoadFinished = false;
}

CFileMgr::~CFileMgr(void)
{
	POSITION pos = m_FileList.GetHeadPosition();
	while(pos)
	{
		CFileTaskItem * item= m_FileList.GetNextValue(pos);
		delete item;
	}
	m_FileList.RemoveAll();

	pos=m_UrlList.GetHeadPosition();
	while(pos)
	{
		delete m_UrlList.GetNextValue(pos);
	}
	m_UrlList.RemoveAll();

	if(m_fmdb != NULL)
		delete m_fmdb;
	//KillTimer(NULL,1234);
}

CString CFileMgr::GetDatabaseFile()                   
{
	return thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("FileList.db");
}

bool CFileMgr::NewDownloadFile(CString strlink, CString strFilepath, int cat,bool bNewTask,CFileTaskItem* pFileTaskItem)  
{
	FM_LOCK;

	if(strFilepath.IsEmpty())  
	{
		//strFilepath = thePrefs.GetTempDir();
		strFilepath = thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR);
	}

	bool bOk=false;            
	int curPos = 0;
	CString resToken = strlink.Tokenize(_T("\t\n\r"), curPos); 
	resToken.Trim();
	while (resToken != _T(""))
	{                              
		try
		{
			if(_tcsnicmp(resToken, _T("http://"), 7) == 0 || _tcsnicmp(resToken, _T("ftp://"), 6) == 0)
			{
				if( FILESTATE_NOT_EXIST==CGlobalVariable::filemgr.GetUrlTaskState(resToken) )
					CmdFuncs::ActualllyAddUrlDownload(resToken,strFilepath);
			}
			else
			{
				if (resToken.Right(1) != _T("/"))
					resToken += _T("/");       
				CED2KLink* pLink = CED2KLink::CreateLinkFromUrl(resToken);
				if (pLink)
				{
					if (pLink->GetKind() == CED2KLink::kFile)
					{
						if(strFilepath.GetAt(strFilepath.GetLength()-1)!='\\')
							strFilepath+='\\';

						CED2KFileLink * pFilelink = (CED2KFileLink*) pLink;
						pFilelink->m_strFilepath = strFilepath;
						CPartFile * partfile=CGlobalVariable::downloadqueue->AddFileLinkToDownload(pLink->GetFileLink(), cat,bNewTask);
						if(partfile)
						{
							bOk=true;						
							if( partfile->GetFileSize()>(uint64)0 )
								partfile->SetPartFileSizeStatus( FS_KNOWN );
							if(bNewTask)
							{
								CFileTaskItem * pItem=new CFileTaskItem;
								pItem->m_nFileState = FILESTATE_DOWNLOADING; 
								pItem->m_FileName = partfile->GetFileName();
								pItem->m_strFilePath = strFilepath+partfile->GetFileName();
								pItem->m_strEd2kLink = resToken;
								pItem->m_FileSize = partfile->GetFileSize();
								pItem->m_metBakId = partfile->GetMetBakId(); 
								m_FileList.SetAt(partfile->GetFileHash(), pItem);
								partfile->m_pFileTaskItem = pItem;
								m_fmdb->UpdateFile(pItem, partfile->GetFileHash(), TRUE);
							}
							else if( pFileTaskItem )
							{
								pFileTaskItem->m_metBakId = partfile->GetMetBakId();
								partfile->LoadUrlSiteList( pFileTaskItem->m_lMetaLinkURLList );
							}
						}
					}
					else
					{
						delete pLink;
						throw CString(_T("bad link"));
					}
					delete pLink;
				}
			}//else

		}
		catch(CString error)
		{
			TCHAR szBuffer[200];
			_sntprintf(szBuffer, ARRSIZE(szBuffer), GetResString(IDS_ERR_INVALIDLINK), error);
			LogError(LOG_STATUSBAR, GetResString(IDS_ERR_LINKERROR), szBuffer);
		}
		resToken = strlink.Tokenize(_T("\t\n\r"), curPos);
	}
   
/*	if( bOk && bNewTask ) 
		SaveFileInfo();*/
	return bOk;
}
bool CFileMgr::LoadSharedFile(CFileTaskItem * pNewItem,const uchar* pFileHash)
{
	//原来的实现大部分代码和AddLocalSharedFile重复,所以重构
	return AddLocalSharedFile( pNewItem->m_strFilePath,true,true,pNewItem,pFileHash);
}

bool CFileMgr::LoadDownloadFile(CFileTaskItem * pNewItem)   
{
	CPartFile* toadd = new CPartFile();
	toadd->SetMetBakId(pNewItem->m_metBakId);
	if (toadd->LoadPartFile(pNewItem->m_strFilePath))
	{
		toadd->m_pFileTaskItem = pNewItem;	
		toadd->LoadUrlSiteList( pNewItem->m_lMetaLinkURLList );
		CGlobalVariable::downloadqueue->AddNewPartFile(toadd);			// to downloadqueue
		if (toadd->GetStatus(true) == PS_READY)
		{
			int n=pNewItem->m_strFilePath.ReverseFind('\\');
			CString strPath=pNewItem->m_strFilePath.Mid(0, n+1);
			/*
			if(! CGlobalVariable::sharedfiles->IsSharedPath(strPath))
				CGlobalVariable::sharedfiles->SafeAddKFile(toadd); // part files are always shared files
			*/
		}
		//  Comment UI
		SendMessage(CGlobalVariable::m_hListenWnd, WM_FILE_ADD_DOWNLOAD, 0, (LPARAM)toadd);
		//theApp.emuledlg->transferwnd->downloadlistctrl.AddFile(toadd);// show in downloadwindow
	}
	else
	{
		// VC-yunchenn.chen[2007-07-12]: 可能会是另一个emule打开，在这里判断是否这种情况
		CFileException fexpPart;
		CFile fileTest;
		if (!fileTest.Open(pNewItem->m_strFilePath + _T(".part"), CFile::modeReadWrite|CFile::shareDenyWrite|CFile::osSequentialScan, &fexpPart))
		{
			if(fexpPart.m_cause==CFileException::sharingViolation)
			{
				delete toadd;
				return true;
			}
			TRACE(_T("Failed to open met file\n"));
		}
		else fileTest.Close();

		if (toadd->LoadPartFile(pNewItem->m_strFilePath, true))
		{
			toadd->m_pFileTaskItem = pNewItem;			
			toadd->SavePartFile(); // resave backup
			
			toadd->LoadUrlSiteList( pNewItem->m_lMetaLinkURLList );
			m_FileList.SetAt(toadd->GetFileHash(), pNewItem);
			CGlobalVariable::downloadqueue->AddNewPartFile(toadd);
			/*
			if (toadd->GetStatus(true) == PS_READY)
				CGlobalVariable::sharedfiles->SafeAddKFile(toadd); // part files are always shared files
			*/
			//  Comment UI
			SendMessage(CGlobalVariable::m_hListenWnd, WM_FILE_ADD_DOWNLOAD, 0, (LPARAM)toadd);
			//theApp.emuledlg->transferwnd->downloadlistctrl.AddFile(toadd);// show in downloadwindow

			AddLogLine(false, GetResString(IDS_RECOVERED_PARTMET), toadd->GetFileName());
		}
		else
		{
			delete toadd;
			if(pNewItem->m_nFileState == FILESTATE_DOWNLOADING)
			{
				CString strPartFileName = pNewItem->m_strFilePath+_T(".part");
				_tremove(strPartFileName);
                if( 0!=pNewItem->m_metBakId )
				{
					CString strMetBAkFileName;
					strMetBAkFileName.Format(_T("%s%u.part.met.bak"),thePrefs.GetMuleDirectory(EMULE_METBAKDIR),pNewItem->m_metBakId);
					_tremove(strMetBAkFileName);
				}
				//加入到下载列表中
				CString strFilePath = pNewItem->m_strFilePath;
				int len = strFilePath.GetLength();
				int index = strFilePath.ReverseFind('\\');
				strFilePath.Delete(index+1,len-index);
				NewDownloadFile(pNewItem->m_strEd2kLink,strFilePath,0,false,pNewItem);
				return true;
			}
			return false;
		}
	}

	if( toadd->GetFileSize()>(uint64)0 && toadd->GetPartFileSizeStatus()==FS_UNKNOWN )
		toadd->SetPartFileSizeStatus( FS_KNOWN );

	return true;
}

bool CFileMgr::LoadDownloadUrlFile( CString& strUrl, CFileTaskItem * pFileTaskItem )  
{
	CPartFile* toadd = new CPartFile();
	toadd->SetMetBakId(pFileTaskItem->m_metBakId);
	if (toadd->LoadPartFile(pFileTaskItem->m_strFilePath))
	{
		toadd->m_pFileTaskItem = pFileTaskItem;		
		toadd->StopFile(false, false); // Pause the file
		toadd->LoadUrlSiteList( pFileTaskItem->m_lMetaLinkURLList );
		CGlobalVariable::downloadqueue->AddNewPartFile(toadd);			// to downloadqueue		
		if (toadd->GetStatus(true) == PS_READY)
		{
			int n = pFileTaskItem->m_strFilePath.ReverseFind('\\');
			CString strPath = pFileTaskItem->m_strFilePath.Mid(0, n+1);
/*
			if(! CGlobalVariable::sharedfiles->IsSharedPath(strPath))
			{
				CGlobalVariable::sharedfiles->SafeAddKFile(toadd); // part files are always shared files
			}
*/
		}

		if( toadd->GetFileSize()>(uint64)0 && toadd->GetPartFileSizeStatus()==FS_UNKNOWN )
			toadd->SetPartFileSizeStatus( FS_KNOWN );
		
		toadd->SetPartFileURL(strUrl);
	
		//  Comment UI
		SendMessage(CGlobalVariable::m_hListenWnd, WM_FILE_ADD_DOWNLOAD, 0, (LPARAM)toadd);
	}
	else
	{
		CFileException fexpPart;
		CFile fileTest;
		if (!fileTest.Open(pFileTaskItem->m_strFilePath + _T(".part"), CFile::modeReadWrite|CFile::shareDenyWrite|CFile::osSequentialScan, &fexpPart))
		{
			if(fexpPart.m_cause==CFileException::sharingViolation)
			{
				delete toadd;	
				return true;
			}
			TRACE(_T("Failed to open met file\n"));
		}
		else fileTest.Close();

		if (toadd->LoadPartFile(pFileTaskItem->m_strFilePath, true))
		{
			toadd->m_pFileTaskItem = pFileTaskItem;
			toadd->SavePartFile(); // resave backup
			m_UrlList.SetAt(strUrl, pFileTaskItem);

			toadd->StopFile(false, false); // Pause the file
			toadd->LoadUrlSiteList( pFileTaskItem->m_lMetaLinkURLList );
			CGlobalVariable::downloadqueue->AddNewPartFile(toadd);
			
			if (toadd->GetStatus(true) == PS_READY)
			{
				//CGlobalVariable::sharedfiles->SafeAddKFile(toadd); // part files are always shared files
			}
			else
			{
				if( CGlobalVariable::m_DNSManager )
				CGlobalVariable::m_DNSManager->AddUrlToDNS(strUrl, toadd);
			}

			//  Comment UI
			SendMessage(CGlobalVariable::m_hListenWnd, WM_FILE_ADD_DOWNLOAD, 0, (LPARAM)toadd);

			AddLogLine(false, GetResString(IDS_RECOVERED_PARTMET), toadd->GetFileName());
		}
		else 
		{
			delete toadd;	
			if(pFileTaskItem->m_nFileState == FILESTATE_DOWNLOADING)
			{ 
				CString strPartFileName = pFileTaskItem->m_strFilePath+_T(".part");
				_tremove(strPartFileName);
				if( 0!=pFileTaskItem->m_metBakId )
				{
					CString strMetBAkFileName;
					strMetBAkFileName.Format(_T("%s%u.part.met.bak"),thePrefs.GetMuleDirectory(EMULE_METBAKDIR),pFileTaskItem->m_metBakId);
					_tremove(strMetBAkFileName);
				}
				CString strPath;
				strPath = pFileTaskItem->m_strFilePath;
				int index = strPath.ReverseFind('\\');
				strPath.Delete(index,strPath.GetLength()-index);
				CmdFuncs::ActualllyAddUrlDownload(strUrl,strPath,false,pFileTaskItem);							  
				return true;
			}
		}
	}
	return true;
}

void CFileMgr::HashFailed (CString strFile)
{
/*
#ifdef _DEBUG	
	if( CGlobalVariable::IsRunning() )
		ASSERT(false);
#endif
*/
	FM_LOCK;

	strFile.MakeLower();
	if(m_WaitforHashList.RemoveKey(strFile))
		m_fmdb->RemoveHashing(strFile);
}

bool CFileMgr::IsWaitforHash( CString& strFile )
{
	FM_LOCK;

	strFile.MakeLower();

	int nFileState;
	return m_WaitforHashList.Lookup(strFile,nFileState); 	
}

void CFileMgr::HashCompleted(CKnownFile* file)
{
	if( !file ) 
		return;

	FM_LOCK;

	CString strKey= file->GetFilePath();
	strKey.MakeLower();
	int nFileState = 0;

	if(m_WaitforHashList.Lookup(strKey, nFileState))
	{
		m_WaitforHashList.RemoveKey(strKey);
		m_fmdb->RemoveHashing(strKey);

		CFileTaskItem * pItem =NULL;
		FILEKEY key=file->GetFileHash();

		if(m_FileList.Lookup(key, pItem))
		{
			pItem->m_nFileState = nFileState;
			m_fmdb->UpdateFile(pItem, key.key, FALSE, FALSE);
		}
		else if(nFileState)
		{
			pItem = new CFileTaskItem;
			pItem->m_nFileState = nFileState;
		    pItem->m_FileName = file->GetFileName();
			pItem->m_strFilePath = file->GetFilePath();// >GetFileName();
			pItem->m_FileSize = file->GetFileSize();
			pItem->m_strEd2kLink = CreateED2kLink(file);
			m_FileList.SetAt(key, pItem);
			m_fmdb->UpdateFile(pItem, key.key, TRUE);
		}
	}
	else
	{
		CFileTaskItem * pItem = NULL;
		if(m_FileList.Lookup(file->GetFileHash(), pItem) && pItem )
		{
			pItem->m_nFileState = FILESTATE_DOWNLOADED_SHARE;
			m_fmdb->UpdateFile(pItem,file->GetFileHash());
		}
	}

	//Added by thilon on 2008.03.05 通知界面
	//UINotify(WM_SHAREDFILE_UPDATECHECKBOX, 1, (LPARAM)file, file);
}

void CFileMgr::RemoveEdLinkFileTask(CString strEd2kUrl)
{
	if(!m_FileList.IsEmpty())
	{  
		CFileTaskItem *pFileItem = NULL;
		FILEKEY key;
		POSITION pos = m_FileList.GetHeadPosition();
		while(pos!=NULL)
		{
			m_FileList.GetNextAssoc( pos,key, pFileItem);
			if(pFileItem->m_strEd2kLink == strEd2kUrl)
			{
				m_fmdb->RemoveFile(pFileItem, key.key);
				m_FileList.RemoveKey(key);
				delete pFileItem;
				break;
			}		 
		}
	}
}

void CFileMgr::RemoveFileItem(CAbstractFile * pFile,bool bKeepShared)
{
	if(! pFile) return;

	FM_LOCK;

	CFileTaskItem * pItem =NULL;
	FILEKEY key=pFile->GetFileHash();
	if( m_FileList.Lookup(key, pItem) && pItem )
	{
		if(bKeepShared)
		{
			pItem->m_nFileState = FILESTATE_SHARE_TASK_DELED;
			m_fmdb->UpdateFile(pItem, key.key, FALSE, FALSE);
		}
		else
		{
			m_fmdb->RemoveFile(pItem, key.key);
			m_FileList.RemoveKey(key);
			delete pItem;
		}
	}
}
void CFileMgr::UpdateFileItem(const CKnownFile *pFile)
{
   if(!pFile) return;
   FM_LOCK;
   CFileTaskItem *pItem = NULL;
   FILEKEY key = pFile->GetFileHash();
   m_FileList.Lookup(key,pItem);
   if( NULL==pItem )   
   {
	   m_UrlList.Lookup(pFile->GetPartFileURL(),pItem); 
   }
	
   if( pItem )
   {
	   pItem->m_FileName = pFile->GetFileName();
	   pItem->m_strFilePath = pFile->GetFilePath();
	   if(pItem->m_strFilePath.Right(5).CompareNoCase(_T(".part"))==0)
	   {
		   pItem->m_strFilePath.Delete(pItem->m_strFilePath.GetLength()-5, 5);
	   }
	   if( pFile->IsKindOf(RUNTIME_CLASS(CPartFile)) )
			pItem->m_metBakId = ((CPartFile*)pFile)->GetMetBakId();
	   m_fmdb->UpdateFile(pItem, key.key, FALSE, FALSE);
   }     
}
void CFileMgr::SaveFile()
{
	FM_LOCK;
	// 
	// 	CFile file;
	// 	if(file.Open(strWholeFilePath, CFile::modeWrite|CFile::modeCreate))
	// 	{
	// 		try
	// 		{
	// 			CArchive ar(&file, CArchive::store);
	// 			ar<<(uint32)CURRENT_FILELIST_VERSION;
	// 			uint32 nTotalfile = m_FileList.GetCount();
	// 			ar<<nTotalfile;
	// 
	// 			POSITION pos = m_FileList.GetHeadPosition();
	// 			for(; pos; )
	// 			{
	// 				FILEKEY fk=m_FileList.GetKeyAt(pos);
	// 				CFileTaskItem * pNewItem = m_FileList.GetNextValue(pos);
	// 
	// 				ar.Write(fk.key, 16);
	// 				pNewItem->Serialize(ar, CURRENT_FILELIST_VERSION);
	// 			}
	// 
	// 			nTotalfile = m_WaitforHashList.GetCount();
	// 			ar<<nTotalfile;
	// 			pos=m_WaitforHashList.GetHeadPosition();
	// 			for(; pos; )
	// 			{
	// 				ar<<m_WaitforHashList.GetKeyAt(pos);
	// 				ar<<m_WaitforHashList.GetNextValue(pos);
	// 			}
	// 
	// 			nTotalfile = m_UrlList.GetCount();
	// 			ar<<nTotalfile;
	// 			pos=m_UrlList.GetHeadPosition();
	// 			while(pos)
	// 			{
	// 				ar<<m_UrlList.GetKeyAt(pos);
	// 				CFileTaskItem * pFileTaskItem = m_UrlList.GetNextValue(pos);
	// 				pFileTaskItem->Serialize(ar,CURRENT_FILELIST_VERSION);
	// 			}
	// 			ar.Flush();   
	// 			ar.Close();
	// 			file.Close();
	// 		}
	// 		catch(...)
	// 		{
	// 			ASSERT(false);
	// 			TRACE("Failed to save filelist\n");
	// 			return;
	// 		}
	// 	}
	/*    CString strFileName;
	int index = strWholeFilePath.ReverseFind(_T('/'));
	strFileName = strWholeFilePath.Left(strWholeFilePath.GetLength() - index);

	CSafeBufferedFile file;
	CFileException fexp;
	if (!file.Open(strWholeFilePath, CFile::modeWrite|CFile::modeCreate|CFile::typeBinary|CFile::shareDenyWrite, &fexp))
	{
	CString strError;
	strError.Format(GetResString(IDS_ERR_SAVEMET), strFileName, strFileName);
	TCHAR szError[MAX_CFEXP_ERRORMSG];
	if (fexp.GetErrorMessage(szError, ARRSIZE(szError)))
	{
	strError += _T(" - ");
	strError += szError;
	}
	LogError(_T("%s"), strError);
	}

	setvbuf(file.m_pStream, NULL, _IOFBF, 16384);*/
	try
	{ 
		//version
		/* 	  file.WriteUInt32(CURRENT_FILELIST_VERSION);

		UINT uTagCount = 0; 
		ULONG uTagCountFilePos = (ULONG)file.GetPosition();
		file.WriteUInt32(uTagCount);

		UINT32 nTotalfile  = m_FileList.GetCount();
		CTag filenumTag(FLT_FILE_NUM,nTotalfile);
		filenumTag.WriteTagToFile(&file);
		uTagCount++;*/

		POSITION pos; 
		m_fmdb->RemoveAll();
		m_fmdb->UpdateFiles(m_FileList);

		//POSITION pos = m_FileList.GetHeadPosition();
		//while (pos)
		//{
		//	FILEKEY fk=m_FileList.GetKeyAt(pos);
		//	CFileTaskItem *pNewItem = m_FileList.GetNextValue(pos);
		//	m_fmdb->UpdateFile(pNewItem, fk.key, TRUE);

		//	//CTag filehashTag(FLT_FILE_HASH,fk.key);
		//	//filehashTag.WriteNewEd2kTag(&file);
		//	//uTagCount++;         
		//	//uTagCount = SaveFileItem(file,pNewItem,uTagCount); 
		//} 
		


		/*	  nTotalfile = m_WaitforHashList.GetCount();
		CTag hashlistnum(WFH_NUM,nTotalfile);
		filenumTag.WriteTagToFile(&file);
		uTagCount++;*/

		pos = m_WaitforHashList.GetHeadPosition();
		while (pos)
		{
			m_fmdb->UpdateHashing(m_WaitforHashList.GetKeyAt(pos), m_WaitforHashList.GetNextValue(pos));
			/*		  CTag filehashTag(WFH_HASH,);
			filehashTag.WriteTagToFile(&file);
			uTagCount++;

			CTag fileValueTag(WFH_VALUE,);
			fileValueTag.WriteTagToFile(&file);
			uTagCount++;		  */
		}

		/*	  nTotalfile = m_UrlList.GetCount();
		CTag urllistnum(URL_NUM,nTotalfile);
		urllistnum.WriteTagToFile(&file);
		uTagCount++;*/

		pos = m_UrlList.GetHeadPosition();
		while (pos)
		{
			/*         CTag urltag(URL_HASH,m_UrlList.GetKeyAt(pos));
			urltag.WriteTagToFile(&file);
			uTagCount++;*/

			CFileTaskItem *pItem = m_UrlList.GetNextValue(pos);
			m_fmdb->UpdateFile(pItem, NULL, TRUE);
			/*		 uTagCount = SaveFileItem(file,pItem,uTagCount);*/
		}

		/*	  file.Seek(uTagCountFilePos, CFile::begin);
		file.WriteUInt32(uTagCount);
		file.SeekToEnd();*/
	}
	catch (...)
	{
		ASSERT(false);
		TRACE("Failed to save filelist\n");
		return;
	}

}
/*UINT CFileMgr::SaveFileItem(CSafeBufferedFile &file,CFileTaskItem *pItem,UINT uTagCount)
{       
	CTag filestateTag(FLT_FILE_STATE,pItem->m_nFileState);
	filestateTag.WriteTagToFile(&file);
	uTagCount++;

	CTag filepathTag(FLT_FILE_PATH,pItem->m_strFilePath);
	filepathTag.WriteTagToFile(&file);
	uTagCount++;

	CTag urlTag(FLT_FILE_URL,pItem->m_strUrl);
	urlTag.WriteTagToFile(&file);
	uTagCount++;

	CTag filesizeTag(FLT_FILE_SIZE,pItem->m_FileSize);
	filesizeTag.WriteTagToFile(&file);
	uTagCount++;

	CTag filenameTag(FLT_FILE_NAME,pItem->m_FileName);
	filenameTag.WriteTagToFile(&file);
	uTagCount++;

	CTag ed2klinkTag(FLT_FILE_ED2KLINK,pItem->m_strEd2kLink);
	ed2klinkTag.WriteTagToFile(&file);
	uTagCount++;

	CTag filetimeTag(FLT_FILE_TIME,(UINT)pItem->m_tFiletime.GetTime());
	filetimeTag.WriteTagToFile(&file);
	uTagCount++;

	CTag metIDTag(FLT_FILE_BAKID,pItem->m_metBakId);
	metIDTag.WriteTagToFile(&file);
	uTagCount++;

	UINT32 urllistnum = pItem->m_lMetaLinkURLList.GetCount();
	CTag urllistTag(UST_NUM,urllistnum);
	urllistTag.WriteTagToFile(&file);
	uTagCount++;

	POSITION pos = pItem->m_lMetaLinkURLList.GetHeadPosition();
	while (pos)
	{
		CUrlSite pSite = pItem->m_lMetaLinkURLList.GetNext(pos);
        
		CTag siteurlTag(UST_URL,pSite.m_strUrl);
		siteurlTag.WriteTagToFile(&file);
		uTagCount++;

		CTag sitefromTag(UST_FROM,pSite.m_dwFromWhere);
		sitefromTag.WriteTagToFile(&file);
		uTagCount++;

		CTag siteData(UST_ONLYDATA,pSite.m_dwDataTransferedWithoutPayload);
		siteData.WriteTagToFile(&file);
		uTagCount++;

		CTag siteTotalData(UST_DATAWITHPAYLOAD,pSite.m_dwDataTransferedWithPayload);
		siteTotalData.WriteTagToFile(&file);
		uTagCount++;

		CTag siteBad(UST_BADSITE,pSite.m_bBadSite,pSite.m_bBadSite);
		siteBad.WriteTagToFile(&file);
		uTagCount++;

		CTag sitePreference(UST_PREFERENCE,pSite.m_dwPreference);
		sitePreference.WriteTagToFile(&file);
		uTagCount++;

		CTag siteUrlCommitted(UST_ORIGINALCOMMITTED,pSite.m_bNeedCommitted);
		siteUrlCommitted.WriteTagToFile(&file);
		uTagCount++;
	}

	return uTagCount;
}*/
/*void CFileMgr::SaveFileInfo()                             
{
	FM_LOCK;

	CFile file;
	//if(file.Open(GetDatabaseFile()+_T(".tmp"), CFile::modeWrite|CFile::modeCreate))
	CString strFilePath ;
	strFilePath = GetDatabaseFile()+_T(".bak");
	SaveFile(strFilePath);

// 		CString strNewfile = GetDatabaseFile()+_T(".tmp");
// 		if(_taccess(strNewfile, 0)==0)
// 		{
// 			DeleteFile(GetDatabaseFile());
// 			CFile::Rename(strNewfile, GetDatabaseFile());
// 		}
    strFilePath = GetDatabaseFile();
	SaveFile(strFilePath);
}*/

/*
void CFileMgr::NewDownloadBySearchFile(CSearchFile* toadd, uint8 paused, CString path, int cat)
{
	if (toadd->GetFileSize()== (uint64)0 || CGlobalVariable::downloadqueue->IsFileExisting(toadd->GetFileHash()))
		return;

	if (toadd->GetFileSize() > OLD_MAX_EMULE_FILE_SIZE && !thePrefs.CanFSHandleLargeFiles()){
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_FSCANTHANDLEFILE));
		return;
	}

	if(path.GetAt(path.GetLength()-1)!='\\')
		path+='\\';

	CPartFile* newfile = new CPartFile(toadd,path, cat);
	if (newfile->GetStatus() == PS_ERROR)
	{
		delete newfile;
		return;
	}
	else
	{
		FM_LOCK;

		CFileTaskItem * pNewItem=new CFileTaskItem;
		pNewItem->m_nFileState = FILESTATE_DOWNLOAD;
		pNewItem->m_strFilePath = path+newfile->GetFileName();
		m_FileList.SetAt(newfile->GetFileHash(), pNewItem);
	}	
	if (paused == 2)
		paused = (uint8)thePrefs.AddNewFilesPaused();
	CGlobalVariable::downloadqueue->AddDownload(newfile, (paused==1));

	// If the search result is from OP_GLOBSEARCHRES there may also be a source
	if (toadd->GetClientID() && toadd->GetClientPort()){
		CSafeMemFile sources(1+4+2);
		try{
			sources.WriteUInt8(1);
			sources.WriteUInt32(toadd->GetClientID());
			sources.WriteUInt16(toadd->GetClientPort());
			sources.SeekToBegin();
			newfile->AddSources(&sources, toadd->GetClientServerIP(), toadd->GetClientServerPort(), false);
		}
		catch(CFileException* error){
			ASSERT(0);
			error->Delete();
		}
	}

	// Add more sources which were found via global UDP search
	const CSimpleArray<CSearchFile::SClient>& aClients = toadd->GetClients();
	for (int i = 0; i < aClients.GetSize(); i++){
		CSafeMemFile sources(1+4+2);
		try{
			sources.WriteUInt8(1);
			sources.WriteUInt32(aClients[i].m_nIP);
			sources.WriteUInt16(aClients[i].m_nPort);
			sources.SeekToBegin();
			newfile->AddSources(&sources,aClients[i].m_nServerIP, aClients[i].m_nServerPort, false);
		}
		catch(CFileException* error){
			ASSERT(0);
			error->Delete();
			break;
		}
	}

	SaveFileInfo();
}*/


void CFileMgr::DownloadCompleted(CPartFile * partfile)
{
	if( partfile==NULL )
		return;

	FM_LOCK;

	CFileTaskItem * pFileTaskItem = NULL;
	CString strUrl = partfile->GetPartFileURL().Trim();
	if( strUrl!=_T("") )
	{		
		if( m_UrlList.Lookup( strUrl,pFileTaskItem ) )
		{
			ASSERT(pFileTaskItem);
			if( partfile->GetFileSize()==(uint64)0 )
			{
				pFileTaskItem->m_nFileState = FILESTATE_ZEROSIZE_DOWNLOADED;
				m_fmdb->UpdateFile(pFileTaskItem, NULL, FALSE, FALSE);
			}
			else
			{
				pFileTaskItem->m_nFileState = FILESTATE_DOWNLOADED_SHARE;
				pFileTaskItem->m_lMetaLinkURLList.RemoveAll();
				m_UrlList.RemoveKey(strUrl);
				m_FileList.SetAt(partfile->GetFileHash(),pFileTaskItem); //moved from m_UrlList to m_FileList		
				m_fmdb->RemoveFile(pFileTaskItem);
			}			
		}
		// else ASSERT(false);
	}
	
	if( !pFileTaskItem && !isnulmd4(partfile->GetFileHash()) ) // [11/26/2007 VC-Huby]: 有可能PartFileURL不为空,但已经拿到了FileHash的任务
	{
		FILEKEY key=partfile->GetFileHash();
		m_FileList.Lookup(key, pFileTaskItem); 	
		ASSERT(pFileTaskItem);
		if(NULL==pFileTaskItem)
		{
			pFileTaskItem = new CFileTaskItem;
			m_FileList.SetAt(key, pFileTaskItem);
		}
		else
			m_fmdb->RemoveFile(pFileTaskItem, key.key);
	}

	ASSERT(pFileTaskItem);

	//更新此已下载任务的其它信息..
	if(pFileTaskItem)
	{
		pFileTaskItem->m_strFilePath = partfile->GetFilePath();	
		pFileTaskItem->m_FileName = partfile->GetFileName();
		pFileTaskItem->m_strUrl = strUrl;
		pFileTaskItem->m_strEd2kLink = CreateED2kLink(partfile);
		pFileTaskItem->m_FileSize = partfile->GetFileSize();
		pFileTaskItem->m_tFiletime = CTime::GetCurrentTime();	
		m_fmdb->UpdateFile(pFileTaskItem, isnulmd4(partfile->GetFileHash()) ? NULL : partfile->GetFileHash(), TRUE, FALSE);
	}
}

/*
bool CFileMgr::IsFileDownloading(CString strFilename)
{
	FM_LOCK;

	POSITION pos=m_FileList.GetHeadPosition();
	while(pos)
	{
		CFileTaskItem * pItem = m_FileList.GetNextValue(pos);
		if(pItem)
		{
			CString strPart=pItem->m_strFilePath+_T(".part");
			CString strPartMet=pItem->m_strFilePath+_T(".part.met");
			CString strPartMetBak=pItem->m_strFilePath+_T(".part.met.bak");
			if(strFilename.CompareNoCase(strPart)==0 ||
				strFilename.CompareNoCase(strPartMetBak)==0 ||
				strFilename.CompareNoCase(strPartMet)==0)
			return true;
		}
	}

	return false;
}*/

bool CFileMgr::AfterLoadFileItem(CFileTaskItem * pFileItem,const uchar* pFileHash)
{
	if(pFileItem->m_nFileState == FILESTATE_DOWNLOADING)
		return LoadDownloadFile(pFileItem);
	else if(pFileItem->m_nFileState == FILESTATE_DOWNLOADED_SHARE || pFileItem->m_nFileState==FILESTATE_LOCAL_SHARE
		|| pFileItem->m_nFileState==FILESTATE_HASH||pFileItem->m_nFileState == FILESTATE_COMPLETED
		|| pFileItem->m_nFileState == FILESTATE_DELETED || pFileItem->m_nFileState ==FILESTATE_SHARE_TASK_DELED)
		return LoadSharedFile(pFileItem,pFileHash);
	else if( pFileItem->m_nFileState == FILESTATE_DOWNLOADED_UNSHARE)
	{
		::SendMessage(theWndMgr.GetWndHandle(CWndMgr::WI_DOWNLOADED_LISTCTRL), UM_CPL_LC_ADDFILE, 0, (LPARAM)pFileItem);
		return true;
	}
	return false;
}

void CFileMgr::LoadFiles(CString strFileListPath)
{
	CGlobalVariable::sharedfiles->Initialize();

	FM_LOCK;
	try
	{
		m_fmdb = new CFileMgrDB(strFileListPath);
	}
	catch (...)
	{
		m_fmdb = NULL;
		throw;
	}

	bool bSaveAfterLoad = false;
	CUnSafeBufferFile file;

	try
	{
		strFileListPath = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("FileList.dat");
		if (!PathFileExists(strFileListPath))
		{
			strFileListPath = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("FileList.dat.bak");
		}
	
		if(file.Open(strFileListPath, CFile::modeRead,NULL))
		{
			bSaveAfterLoad = true;
			CArchive ar(&file, CArchive::load);
	        int iFileListVer;
			int nTotalfile = 0;
			ar>>iFileListVer;

			if (iFileListVer < 3)
			{
				MessageBox(NULL,GetResString(IDS_FILE_LIST),GetResString(IDS_CAPTION),MB_OK|MB_ICONWARNING);
				m_FileList.RemoveAll();
				m_UrlList.RemoveAll();
				bSaveAfterLoad = true;
			}
			else if (iFileListVer < 6)
			{
				ar>>nTotalfile;
				TRACE("\n\n%s: begin load file (%d)\n\n", __FUNCTION__, nTotalfile);
				for(int i=0; i<nTotalfile; i++)
				{
					FILEKEY key;
					ar.Read(key.key, 16);

					CFileTaskItem * pNewItem = new CFileTaskItem;
					pNewItem->Serialize(ar, iFileListVer);

					TRACE("filestate on loading: %d\n", pNewItem->m_nFileState);

					if( pNewItem->m_FileName.IsEmpty() && pNewItem->m_strFilePath.IsEmpty() )
					{
						delete pNewItem; 
						pNewItem  = NULL;
						bSaveAfterLoad = true;	
						continue;
					}					
					int nOldState = pNewItem->m_nFileState;
					UINT32 nOldMetBakId = pNewItem->m_metBakId;

					if(!AfterLoadFileItem(pNewItem))
					{
						pNewItem->m_nFileState = FILESTATE_DELETED;
					}					
#ifdef _DEBUG
					if(pNewItem) TRACE(_T("%02d file loaded: %s\n"), i, pNewItem->m_strFilePath);
#endif
					if( nOldState==FILESTATE_LOCAL_SHARE && pNewItem->m_nFileState==FILESTATE_DELETED )
					{
						delete pNewItem; //对于本地添加的分享,如果已经被删除,filemgr不再管理,其它的仍然保存					
						pNewItem  = NULL;
					}
					else
					{
						m_FileList.SetAt(key, pNewItem);
					}

					if( pNewItem==NULL || pNewItem->m_nFileState!=nOldState
						|| nOldMetBakId != pNewItem->m_metBakId )
						bSaveAfterLoad = true;						
				}

				ar>>nTotalfile;
				for(int i=0; i<nTotalfile; i++)
				{
					CString strFile;
					int state;
					ar>>strFile;
					ar>>state;

					m_WaitforHashList.SetAt(strFile, state);
					CGlobalVariable::sharedfiles->HashNewFile(strFile);
				}

				ar>>nTotalfile;
				for(int i=0; i<nTotalfile; i++)
				{
					CFileTaskItem * pFileTaskItem = new CFileTaskItem;
					CString url;
					ar>>url;
					pFileTaskItem->Serialize(ar, iFileListVer);
					ASSERT( pFileTaskItem->m_nFileState==FILESTATE_DOWNLOADING || pFileTaskItem->m_nFileState==FILESTATE_ZEROSIZE_DOWNLOADED );
					if(pFileTaskItem && (pFileTaskItem->m_nFileState == FILESTATE_DOWNLOADING || pFileTaskItem->m_nFileState == FILESTATE_ZEROSIZE_DOWNLOADED))
						m_UrlList.SetAt(url, pFileTaskItem);
				}
			}
			ar.Close();
			file.Close();
			DeleteFile(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("FileList.dat.bak"));
			DeleteFile(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("FileList.dat"));
		}
		else
		{
			m_fmdb->LoadAll(this);
			bSaveAfterLoad |= AsyncShareList();
		}
	}	
	catch ( CException * /* e */ )
	{
		file.Close();

		CString strBakPath;
		strBakPath = GetDatabaseFile() + _T(".bak");
		if(strFileListPath == strBakPath)
		{
	 		MessageBox(NULL,GetResString(IDS_DESTROYED_FILE),GetResString(IDS_CAPTION),MB_OK|MB_ICONWARNING);
			m_FileList.RemoveAll();
	 		m_UrlList.RemoveAll();
			//如果FileList.dat和FileList.dat.bak同时损坏时
			//此处的代码会造成崩溃
			bSaveAfterLoad = true;
//			e->Delete();
		}      
		else
		{
           LoadFiles(strBakPath);
		}
		bSaveAfterLoad = true;
		//修改FileList.dat造成的崩溃
//		e->Delete();
	}

	//emule 的下载任务导入到easyMule

    //if(!PathFileExists(GetDatabaseFile())) //一句废话，永远不可能运行
	if (thePrefs.tempdir.GetCount() > 0)
	{
		CGlobalVariable::downloadqueue->LoadOldVersionPartFile();
		thePrefs.tempdir.RemoveAll();
		thePrefs.tempdir.Add(_T("TempDir Exported"));
	}

	CGlobalVariable::downloadqueue->SortByPriority();
	CGlobalVariable::downloadqueue->CheckDiskspace();
	CGlobalVariable::downloadqueue->ExportPartMetFilesOverview();
	
	PostMessage(CGlobalVariable::m_hListenWnd, WM_SHAREDFILE_RELOADFILELIST, 0, 0);

	for (POSITION pos = m_UrlList.GetHeadPosition(); pos;)
	{
		CString strUrl = m_UrlList.GetKeyAt(pos);
		CFileTaskItem * pFileTaskItem = m_UrlList.GetNextValue(pos);
		if( pFileTaskItem->m_nFileState==FILESTATE_ZEROSIZE_DOWNLOADED)
		{
			::SendMessage(theWndMgr.GetWndHandle(CWndMgr::WI_DOWNLOADED_LISTCTRL), UM_CPL_LC_ADDFILE, 0, (LPARAM)pFileTaskItem);
		}
		else if(pFileTaskItem->m_nFileState != FILESTATE_DOWNLOADED_SHARE)
		{
			UINT32 iOldMetBakId = pFileTaskItem->m_metBakId;
			if( !LoadDownloadUrlFile(strUrl, pFileTaskItem) )
			{
				pFileTaskItem->m_nFileState = FILESTATE_DELETED;
				bSaveAfterLoad = true;
			}
			else if( iOldMetBakId!=pFileTaskItem->m_metBakId )
			{
				bSaveAfterLoad = true;
			}
		}
	}

	if (CGlobalVariable::downloadqueue)
		CGlobalVariable::downloadqueue->AddPartFilesToShare(); // read partfiles to sharedfilelist

	if(bSaveAfterLoad)
	{
		SaveFile();
	}

	m_bLoadFinished = true;
}

/**
*	<BR>功能说明：添加本地单个分享文件（包括已下载分享和本地添加的分享）
                  现在有两种情况调用此函数:
				  1 从分享页面UI上"add share files",选择单个或多个文件来分享（注意不是选择目录分享）
				    这种情况需要算hash后开始分享
				  2 从本地filelist.dat加载
*	@param strFile 完整的文件路径名
*	@param bNeedHash 如果没有已知hash信息，是否需要hash
*	@param bLoadFromFileListDat 是否是从本地filelist.dat中加载,而不是从其它接口新增..
*	@return true 添加成功
*/
bool CFileMgr::AddLocalSharedFile(const CString & strFile, bool bNeedHash, bool bLoadFromFileListDat, CFileTaskItem *pFileItem,const uchar* pFileHash)
{
	if(!bLoadFromFileListDat)
	{
		POSITION pos=m_FileList.GetHeadPosition();
		while(pos)
		{
			CFileTaskItem * pItem=m_FileList.GetNextValue(pos);
			if(pItem && (pItem->m_nFileState==FILESTATE_LOCAL_SHARE || pItem->m_nFileState==FILESTATE_DOWNLOADED_SHARE)
				&& pItem->m_strFilePath.CompareNoCase(strFile)==0)
			{
				//  exists the local shared file checked by filename
				return true;
			}
		}
	}

	CKnownFile* toadd = NULL;

	if(pFileHash)
	{
		toadd = CGlobalVariable::knownfiles->FindKnownFileByID(pFileHash);
		if( toadd && toadd->GetPath()==_T("") )
			toadd = NULL;
		else if( pFileItem && toadd && pFileItem->m_strFilePath.CompareNoCase(toadd->GetFilePath())!=0 )
			pFileItem->m_strFilePath = toadd->GetFilePath(); //用户incoming下移动了文件
	}
	
	if(!toadd)
	{
		int nPathEnd=strFile.ReverseFind('\\');
		CString strDirectory = strFile.Mid(0, nPathEnd);
/*
		if( !bLoadFromFileListDat && CGlobalVariable::sharedfiles->IsSharedPath(strDirectory))
			return false;
*/

		CFile file;
		if( !file.Open(strFile, CFile::modeRead) )
		{
			AddDebugLogLine(false, _T("Failed to open file of \"%s\""), strFile);
			if( pFileItem && (pFileItem->m_nFileState==FILESTATE_DOWNLOADED_SHARE || pFileItem->m_nFileState==FILESTATE_DELETED) )
				::SendMessage(theWndMgr.GetWndHandle(CWndMgr::WI_DOWNLOADED_LISTCTRL), UM_CPL_LC_ADDFILE, 0, (LPARAM)pFileItem); //属于已下载任务,但文件被删除的情况         
			return false;
		}
		else if( pFileItem && pFileItem->m_nFileState == FILESTATE_DELETED)
		{
			::SendMessage(theWndMgr.GetWndHandle(CWndMgr::WI_DOWNLOADED_LISTCTRL),UM_CPL_LC_ADDFILE,0,(LPARAM)pFileItem);
			return true;
		}

		CFileStatus fs;
		if( !file.GetStatus(fs) ) 
			return false;

		uint32 fdate = (UINT)fs.m_mtime.GetTime();
		if (fdate == 0)
			fdate = (UINT)-1;
		if (fdate == -1)
		{
			if (thePrefs.GetVerbose())
				AddDebugLogLine(false, _T("Failed to get file date of \"%s\""), strFile);
		}
		else
			AdjustNTFSDaylightFileTime(fdate, strFile);

		toadd = CGlobalVariable::knownfiles->FindKnownFile(file.GetFileName(), fdate, file.GetLength());
		if(toadd)
		{
			toadd->SetPath(strDirectory);			
			toadd->SetFilePath(strFile);
		}
	}	

	if (toadd)
	{
		ASSERT( !toadd->HasNullHash() );
		CCKey key(toadd->GetFileHash());
		if(pFileItem)
			toadd->SetPartFileURL(pFileItem->m_strUrl);
		CKnownFile* pFileInMap=CGlobalVariable::sharedfiles->GetKnownFile(key);
		if (!pFileInMap)
		{			
			CGlobalVariable::sharedfiles->AddFile(toadd);
		}
		else
		{
			TRACE(_T("%hs: File already in shared file list: %s \"%s\"\n"), __FUNCTION__, md4str(pFileInMap->GetFileHash()), pFileInMap->GetFilePath());		
		}

		if(!bLoadFromFileListDat) 
		{
			FILEKEY fk=toadd->GetFileHash();
			CFileTaskItem * pItem=NULL;
			bool bNewFileItem = false;
			if(! m_FileList.Lookup(fk, pItem) || pItem==NULL)
			{
				pItem=new CFileTaskItem;			
				m_FileList.SetAt(fk, pItem);
				bNewFileItem = true;
			}			
			pItem->m_strFilePath = strFile;
			pItem->m_FileName = toadd->GetFileName();
			pItem->m_FileSize = toadd->GetFileSize();
			if( pItem->m_nFileState == FILESTATE_DOWNLOADED_UNSHARE )
				pItem->m_nFileState = FILESTATE_DOWNLOADED_SHARE;
			else
				pItem->m_nFileState = FILESTATE_LOCAL_SHARE;	
			m_fmdb->UpdateFile(pItem, fk.key, bNewFileItem, FALSE);

			//UINotify(WM_SHAREDFILE_ADDFILE, 0, (LPARAM)toadd, toadd);
			//Added by thilon on 2008.03.05 通知上层更新界面
			UINotify(WM_SHAREDFILE_UPDATECHECKBOX, 1,(LPARAM)toadd, toadd);
		}

		if( pFileItem && pFileItem->m_nFileState==FILESTATE_DOWNLOADED_SHARE)
			SendMessage(CGlobalVariable::m_hListenWnd, WM_FILE_ADD_COMPLETED, 0, (LPARAM)toadd);
	}
	else
	{
		CString strKey=strFile;
		strKey.MakeLower();
		m_WaitforHashList.SetAt(strKey, FILESTATE_LOCAL_SHARE);
		m_fmdb->UpdateHashing(strKey,FILESTATE_LOCAL_SHARE,TRUE);

		if(bNeedHash)
		{
			//UINotify(WM_SHAREDFILE_FILEHASHING, 1, 0, 0);
			CGlobalVariable::sharedfiles->HashNewFile(strFile);
		}
	}
	
	return true;
}

/// 移除文件分享
bool CFileMgr::RemoveFileShare(const CString & strFilePath)
{
	CFileTaskItem* pItemToRemove = NULL;

	POSITION pos=m_FileList.GetHeadPosition();		
	while(pos)
	{
		CFileTaskItem * pItem=m_FileList.GetNextValue(pos);
		if(pItem && (pItem->m_nFileState==FILESTATE_LOCAL_SHARE || pItem->m_nFileState==FILESTATE_DOWNLOADED_SHARE
			|| pItem->m_nFileState==FILESTATE_SHARE_TASK_DELED )
			&& pItem->m_strFilePath.CompareNoCase(strFilePath)==0)
		{
			//  exists the local shared file checked by filename
			pItemToRemove = pItem;
			break;
		}
	}
	
	if( NULL==pItemToRemove )
	{
		POSITION pos = m_WaitforHashList.GetHeadPosition();
		while(pos)
		{
			CString strFileWaitHash = m_WaitforHashList.GetKeyAt(pos);
			if( strFileWaitHash.CompareNoCase(strFilePath)==0 )
			{
				m_WaitforHashList.RemoveAt(pos);
				m_fmdb->RemoveHashing(strFileWaitHash);
				int nPathEnd=strFilePath.ReverseFind('\\');			
				CGlobalVariable::sharedfiles->RemoveHashing(strFilePath.Mid(0, nPathEnd),strFilePath.Mid(nPathEnd+1));
				return true;
			}
			m_WaitforHashList.GetNext(pos);
		}

		return false;
	}	

	int nPathEnd=strFilePath.ReverseFind('\\');
	CString strDirectory = strFilePath.Mid(0, nPathEnd);	
	CKnownFile* pKnownFileExist = CGlobalVariable::knownfiles->FindKnownFileByPath(strFilePath,true);
	if (pKnownFileExist)
	{
		FILEKEY fk= pKnownFileExist->GetFileHash();
		CGlobalVariable::sharedfiles->RemoveFile(pKnownFileExist,true);

		if( pItemToRemove->m_nFileState == FILESTATE_LOCAL_SHARE 
			|| pItemToRemove->m_nFileState == FILESTATE_SHARE_TASK_DELED ) //从filelist.db中移走
		{
			m_FileList.RemoveKey(fk);
			m_fmdb->RemoveFile(pItemToRemove,pKnownFileExist->GetFileHash());
			delete pItemToRemove;
		}
		else if( pItemToRemove->m_nFileState == FILESTATE_DOWNLOADED_SHARE )
		{
			pItemToRemove->m_nFileState = FILESTATE_DOWNLOADED_UNSHARE; //在filelist.db中保持已下载记录,但不再分享
			m_fmdb->UpdateFile(pItemToRemove, fk.key, FALSE, FALSE);
		}

		UINotify(WM_SHAREDFILE_UPDATECHECKBOX, 0,(LPARAM)pKnownFileExist, pKnownFileExist);
		return true;
	}

	return false;
}

/// 检测某个目录下是否有被分享的文件
bool CFileMgr::HasFileSharedInDir(const CString& strDir )
{
	CFileTaskItem* pItem = NULL;
	
	CString strDirTemp = strDir;
	if( strDirTemp.Right(1)!=_T("\\") )
		strDirTemp += _T("\\");
	strDirTemp.MakeLower();	//Added by thilon on 2008.03.19

	POSITION pos=m_FileList.GetHeadPosition();		
	while(pos)
	{
		pItem=m_FileList.GetNextValue(pos);

		CString strFilePath = pItem->m_strFilePath;
		strFilePath.MakeLower();								//changed by thilon on 2008.03.25

		if(pItem && (pItem->m_nFileState == FILESTATE_LOCAL_SHARE 
				 || pItem->m_nFileState == FILESTATE_DOWNLOADED_SHARE
				 || pItem->m_nFileState == FILESTATE_SHARE_TASK_DELED)
				 && strFilePath.Find(strDirTemp) == 0)
		{
			return true;
		}
	}

	return false;
}

bool CFileMgr::IsFileLocalShared(CKnownFile * file)
{
	return GetFileState(file->GetFileHash())==FILESTATE_LOCAL_SHARE;
}

void CFileMgr::ReloadLocalSharedFiles()
{
	POSITION pos = m_FileList.GetHeadPosition();
	POSITION pos2;
	while(pos)
	{
		pos2 = pos;
		FILEKEY filekey = m_FileList.GetKeyAt(pos);
		CFileTaskItem * pItem = m_FileList.GetNextValue(pos);
		
		if(pItem->m_nFileState != FILESTATE_LOCAL_SHARE)
		{
			//  共享文件会丢失的bug
			if(pItem->m_nFileState != FILESTATE_DOWNLOADED_SHARE)
				continue;
		}

		CString strFile = pItem->m_strFilePath;
		CFile file;
		if(! file.Open(strFile, CFile::modeRead))
		{
			AddDebugLogLine(false, _T("Failed to open file of \"%s\""), strFile);
			m_FileList.RemoveAt(pos2);
			m_fmdb->RemoveFile(pItem,filekey.key);
			continue ;
		}

		CFileStatus fs;
		if(! file.GetStatus(fs)) continue;

		uint32 fdate = (UINT)fs.m_mtime.GetTime();
		if (fdate == 0)
			fdate = (UINT)-1;
		if (fdate == -1)
		{
			if (thePrefs.GetVerbose())
				AddDebugLogLine(false, _T("Failed to get file date of \"%s\""), strFile);
		}
		else
			AdjustNTFSDaylightFileTime(fdate, strFile);

		int nPathEnd=strFile.ReverseFind('\\');
		CString strDirectory = strFile.Mid(0, nPathEnd);
		CKnownFile* toadd = CGlobalVariable::knownfiles->FindKnownFile(file.GetFileName(), fdate, file.GetLength());
		if (toadd)
		{
			CCKey key(toadd->GetFileHash());
			CKnownFile* pFileInMap=CGlobalVariable::sharedfiles->GetKnownFile(key);
			if (!pFileInMap)
			{
				toadd->SetPath(strDirectory);
				toadd->SetFilePath(strFile);
				if(! CGlobalVariable::sharedfiles->IsSharedPath(strDirectory))
					CGlobalVariable::sharedfiles->AddFile(toadd);
			}
		}
		else
		{
			CString strKey=strFile;
			strKey.MakeLower();
			m_WaitforHashList.SetAt(strKey, FILESTATE_LOCAL_SHARE);
			m_fmdb->UpdateHashing(strKey,FILESTATE_LOCAL_SHARE,TRUE);

			CGlobalVariable::sharedfiles->HashNewFile(strFile);
		}
	}
}

bool CFileMgr::ModifyURLState(CString strUrl, CString strLocalPath, int nFileState)
{
	FM_LOCK;

	strUrl.MakeLower();
	CFileTaskItem * pFileTaskItem = NULL;
	if( m_UrlList.Lookup(strUrl, pFileTaskItem) && pFileTaskItem )
	{
		if( nFileState!=-1 ) 
			pFileTaskItem->m_nFileState = nFileState;
		if( !strLocalPath.IsEmpty() ) 
			pFileTaskItem->m_strFilePath = strLocalPath;
		m_fmdb->UpdateFile(pFileTaskItem, NULL, FALSE, FALSE);
		return true;
	}

	return false;
}

CFileTaskItem* CFileMgr::AddURLTask(CString strUrl,const CString strLocalDir)
{
	FM_LOCK;

	CFileTaskItem * pFileTaskItem = NULL;
	BOOL bNew = FALSE;
	if( !m_UrlList.Lookup(strUrl.Trim(), pFileTaskItem) )
	{
		pFileTaskItem = new CFileTaskItem;		
		m_UrlList.SetAt(strUrl.Trim(), pFileTaskItem);
		bNew = TRUE;
	}

	if(pFileTaskItem)
	{
		pFileTaskItem->m_nFileState = FILESTATE_DOWNLOADING;		
		pFileTaskItem->m_strFilePath = strLocalDir;
		pFileTaskItem->m_strUrl = strUrl;
		m_fmdb->UpdateFile(pFileTaskItem, NULL, bNew, FALSE);
	}

	return pFileTaskItem;
}
void CFileMgr::RemoveURLTask(CString strUrl)
{
	FM_LOCK;

	CFileTaskItem * pFileTaskItem = NULL;
	if(m_UrlList.Lookup(strUrl, pFileTaskItem))
	{
		m_fmdb->RemoveFile(pFileTaskItem);
		m_UrlList.RemoveKey(strUrl);	
	}	
}
CString CFileMgr::GetFileName(const CString & strLink)
{
	CString strFileName;
	
	int curPos = 0;
	CString resToken = strLink.Tokenize(_T("\t\n\r"), curPos);
	while (resToken != _T(""))
	{
		if (resToken.Right(1) != _T("/"))
			resToken += _T("/");
		try
		{
			CED2KLink* pLink = CED2KLink::CreateLinkFromUrl(resToken.Trim());
			if (pLink)
			{
				if (pLink->GetKind() == CED2KLink::kFile)
				{
					CED2KFileLink * pFilelink = (CED2KFileLink*) pLink;
					strFileName = pFilelink->GetName();
				}
				delete pLink;
			}
			if(!strFileName.IsEmpty()) break;
		}
		catch(CString error)
		{
			TCHAR szBuffer[200];
			_sntprintf(szBuffer, ARRSIZE(szBuffer), GetResString(IDS_ERR_INVALIDLINK), error);
			LogError(LOG_STATUSBAR, GetResString(IDS_ERR_LINKERROR), szBuffer);
		}
		resToken = strLink.Tokenize(_T("\t\n\r"), curPos);
	}
	return strFileName;
}
CString CFileMgr::GetUrlFileName(CString strUrl)
{
	CString strFileName;
	FM_LOCK;

	strUrl.Trim();
	strUrl.MakeLower();	

	CString strUrlConverted;		
	if( strUrl.Find(_T('<'))>0 )
		strUrlConverted = UrlConvert( strUrl.Left(strUrl.Find(_T('<'))) ); 
	else
		strUrlConverted = UrlConvert( strUrl);
	if(strUrlConverted.Find(_T('#'))>0 )
		strUrlConverted.Remove(_T('#'));

	CFileTaskItem * pFileTaskItem = NULL;
	CString sUrlInFileMgr;
	POSITION pos = m_UrlList.GetHeadPosition();
	while(pos)
	{
		sUrlInFileMgr = m_UrlList.GetKeyAt(pos);
		if( sUrlInFileMgr.Find(_T('<'))>0 )
			sUrlInFileMgr = sUrlInFileMgr.Left( sUrlInFileMgr.Find(_T('<')) ); //cut the extra info
		if(sUrlInFileMgr.Find(_T('#'))>0)
			sUrlInFileMgr.Remove(_T('#'));
		sUrlInFileMgr.MakeLower();
		sUrlInFileMgr = UrlConvert( sUrlInFileMgr );
		pFileTaskItem = m_UrlList.GetNextValue(pos);
		if( !sUrlInFileMgr.CompareNoCase(strUrlConverted) )
		{
			strFileName = pFileTaskItem->m_FileName;
			if (strFileName.IsEmpty())
			{
				CString strFilePath = pFileTaskItem->m_strFilePath;
				int index = strFilePath.ReverseFind('\\');
				strFileName = strFilePath.Mid(index + 1,strFilePath.GetLength() - index);
			}
		}
	}

	pos = m_FileList.GetHeadPosition();
	while(pos)
	{
		pFileTaskItem = m_FileList.GetNextValue(pos);
		sUrlInFileMgr = pFileTaskItem->m_strUrl;
		if( sUrlInFileMgr.IsEmpty() )
			continue;
		if( sUrlInFileMgr.Find(_T('<'))>0 )
			sUrlInFileMgr = sUrlInFileMgr.Left( sUrlInFileMgr.Find(_T('<')) ); //cut the extra info
		if(sUrlInFileMgr.Find(_T('#'))>0 )
			sUrlInFileMgr.Remove(_T('#'));
		sUrlInFileMgr.MakeLower();
		sUrlInFileMgr = UrlConvert( sUrlInFileMgr );
		if( !sUrlInFileMgr.CompareNoCase(strUrlConverted) )
		{
			strFileName = pFileTaskItem->m_FileName;
		}
	}
	return strFileName;
}
int CFileMgr::GetFileState(const CString & strlink)
{
	int nRet = 0;
	int curPos = 0;
	CString resToken = strlink.Tokenize(_T("\t\n\r"), curPos);
	while (resToken != _T(""))
	{
		if (resToken.Right(1) != _T("/"))
			resToken += _T("/");
		try
		{
			CED2KLink* pLink = CED2KLink::CreateLinkFromUrl(resToken.Trim());
			if (pLink)
			{
				if (pLink->GetKind() == CED2KLink::kFile)
				{
					CED2KFileLink * pFilelink = (CED2KFileLink*) pLink;

					nRet = GetFileState(pFilelink->GetHashKey());

					if(0==nRet && CGlobalVariable::downloadqueue->IsFileExisting(pFilelink->GetHashKey()) )
						nRet = FILESTATE_LOCAL_SHARE;
						
				}
				delete pLink;
			}
			if(nRet) 
				break;
		}
		catch(CString error)
		{
			TCHAR szBuffer[200];
			_sntprintf(szBuffer, ARRSIZE(szBuffer), GetResString(IDS_ERR_INVALIDLINK), error);
			LogError(LOG_STATUSBAR, GetResString(IDS_ERR_LINKERROR), szBuffer);
		}
		resToken = strlink.Tokenize(_T("\t\n\r"), curPos);
	}

	return nRet;
}

/// 1 必须去掉extraInfo <referer=...> 后来判断处理
/// 2 strUrl 参数必须做统一编码后来判断是否重复
int CFileMgr::GetUrlTaskState(CString strUrl)
{
	FM_LOCK;

	strUrl.Trim();
	strUrl.MakeLower();	
		
	CString strUrlConverted;	///VC-Huby[2007-09-12]: process the the url with <referer=..>				
	if( strUrl.Find(_T('<'))>0 )
		strUrlConverted = UrlConvert( strUrl.Left(strUrl.Find(_T('<'))) ); //need cut the <referer=..> info first	
	else
		strUrlConverted = UrlConvert( strUrl );
	if(strUrlConverted.Find(_T('#'))>0 )
		strUrlConverted.Remove(_T('#'));

	CFileTaskItem * pFileTaskItem = NULL;
	CString sUrlInFileMgr;
	POSITION pos = m_UrlList.GetHeadPosition();
	while(pos)
	{
		sUrlInFileMgr = m_UrlList.GetKeyAt(pos);
  	   if( sUrlInFileMgr.Find(_T('<'))>0 )
			sUrlInFileMgr = sUrlInFileMgr.Left( sUrlInFileMgr.Find(_T('<')) ); //cut the extra info
		if(sUrlInFileMgr.Find(_T('#'))>0)
			sUrlInFileMgr.Remove(_T('#'));
		sUrlInFileMgr.MakeLower();
		sUrlInFileMgr = UrlConvert( sUrlInFileMgr );
		pFileTaskItem = m_UrlList.GetNextValue(pos);
		if( !sUrlInFileMgr.CompareNoCase(strUrlConverted) )
		{
			return pFileTaskItem->m_nFileState;
		}
	}

	pos = m_FileList.GetHeadPosition();
	while(pos)
	{
		pFileTaskItem = m_FileList.GetNextValue(pos);
		sUrlInFileMgr = pFileTaskItem->m_strUrl;
		if( sUrlInFileMgr.IsEmpty() )
			continue;
		if( sUrlInFileMgr.Find(_T('<'))>0 )
			sUrlInFileMgr = sUrlInFileMgr.Left( sUrlInFileMgr.Find(_T('<')) ); //cut the extra info
		if(sUrlInFileMgr.Find(_T('#'))>0 )
			sUrlInFileMgr.Remove(_T('#'));
		sUrlInFileMgr.MakeLower();
		sUrlInFileMgr = UrlConvert( sUrlInFileMgr );
		if( !sUrlInFileMgr.CompareNoCase(strUrlConverted) )
		{
			return pFileTaskItem->m_nFileState;
		}
	}

/* the origin code,don't process extrainfo of <referer=...>
	if(m_UrlList.Lookup(strUrl, pInfo))
	{
		return pInfo->m_nFileState;
	}
*/
	return FILESTATE_NOT_EXIST;
}

CString CFileMgr::GetUrl(const uchar * hashkey)
{
	FM_LOCK;
	CString strUrl;
	CFileTaskItem * pItem = NULL;
	if(m_FileList.Lookup(hashkey, pItem) && pItem)
	{
		strUrl = pItem->m_strUrl;
	}
	return strUrl;
}

int CFileMgr::GetFileState(const uchar * hashkey)
{
	FM_LOCK;
	CFileTaskItem * pItem = NULL;
	if(m_FileList.Lookup(hashkey, pItem) && pItem)
	{
		return pItem->m_nFileState;
	}
	return 0;
}

CFileTaskItem* CFileMgr::GetFileTaskItem(const uchar * hashkey)
{
	FM_LOCK;
	CFileTaskItem * pItem = NULL;
	if(m_FileList.Lookup(hashkey, pItem) && pItem)
	{
		return pItem;
	}
	return NULL;
}

CFileTaskItem* CFileMgr::GetFileTaskItem(const CString& strUrl)
{
	FM_LOCK;
	CFileTaskItem * pItem = NULL;
	if(m_UrlList.Lookup(strUrl, pItem) && pItem)
	{
		return pItem;
	}
	return NULL;
}


bool CFileMgr::AddPartFile(CPartFile * partfile, int filestate)
{
	if(partfile == NULL)
		return false;
	FM_LOCK;

	if(m_FileList.Lookup(partfile->GetFileHash()))
	{   
		return false;
	}

	CFileTaskItem * pItem=new CFileTaskItem;
	pItem->m_nFileState = filestate;
	pItem->m_FileName = partfile->GetFileName();
	pItem->m_strFilePath = partfile->GetFilePath();//+_T("\\") +partfile->GetFileName();
	if(pItem->m_strFilePath.GetLength()>5)
	{
		CString test=pItem->m_strFilePath.Right(5);
		if(test.CompareNoCase(_T(".part"))==0)
		{
			pItem->m_strFilePath.Delete(pItem->m_strFilePath.GetLength()-5, 5);
		}
	}

	if( !partfile->HasNullHash() )
	{
		pItem->m_FileSize = partfile->GetFileSize();
		pItem->m_strEd2kLink = CreateED2kLink(partfile);
		m_FileList.SetAt(partfile->GetFileHash(), pItem);
		m_fmdb->UpdateFile(pItem, partfile->GetFileHash(), TRUE, FALSE);
	}
	else if( partfile->GetPartFileURL()!=_T("") )
	{
		m_UrlList.SetAt( partfile->GetPartFileURL(),pItem);
		m_fmdb->UpdateFile(pItem, NULL, TRUE, FALSE);
	}

	return true;
}

/// strUrl have not cut <referer=...>
//  strFilePath is only path,no filename
BOOL CFileMgr::AddDownLoadRequest(const CString & strUrl, const CString & strFilePath, CPartFile* &pPartFile,bool bNewTask)
{
	CString strFileName;
	strFileName = GetFileNameFromUrlStr( strUrl );

	pPartFile = new CPartFile(); //CPartFile * 
	pPartFile->SetFileName(strFileName);
	pPartFile->SetPartFileSizeStatus(FS_UNKNOWN);
	pPartFile->SetFileSize((uint64)1u/*(uint64)PARTSIZE*/);	
	pPartFile->SetPath(strFilePath);//pPartFile->SetFilePath(strFilePath);
	pPartFile->SetPartFileURL(strUrl);
    
	CGlobalVariable::downloadqueue->AddDownload(pPartFile, thePrefs.AddNewFilesPaused(),bNewTask);

	CString strTmp=strFilePath;
	// VC-yunchenn.chen[2007-07-16]: fix a bug, path lost '\' between directory and file
	if(! strFilePath.IsEmpty() && strFilePath.GetAt(strFilePath.GetLength()-1)!='\\')
	{
		strTmp += _T("\\");
	}

	// VC-SearchDream[2007-07-12]: Add the URL to File Manager
	if(bNewTask)
	{
		CString strWholeFilePath = strTmp + pPartFile->GetFileName();
		CFileTaskItem* pFileTaskItem = AddURLTask(strUrl,strWholeFilePath);
		if(pPartFile) 
			pPartFile->m_pFileTaskItem = pFileTaskItem;
	}
	return TRUE;
}

void CFileMgr::OnPartfileUrlChanged( const CString & old_url , const CString & new_url , CPartFile * /* partfile */ )
{
	CFileTaskItem *pItem = NULL;
	m_UrlList.Lookup( old_url , pItem );
	if( pItem ) {
		m_fmdb->RemoveFile(pItem);
		m_UrlList.RemoveKey( old_url );
		m_UrlList.SetAt( new_url , pItem );

		pItem->m_strUrl = new_url;
//		pItem->m_strFilePath = _T("");
		pItem->m_FileName = _T("");
		pItem->m_FileSize = 0;
		m_fmdb->UpdateFile(pItem, NULL, TRUE);
	}
}


// VC-wangna[2007-012-18]: Add the MetalinkURL to FileList
void CFileMgr::OnAddMetaLinkUrl(CONST BYTE * lpHash, CString strUrl,CUrlSite* pUrlSite)
{
    CFileTaskItem *pItem = NULL;
	bool bIsHash = true;
	if(lpHash != NULL && !isnulmd4(lpHash))
		m_FileList.Lookup(lpHash, pItem);
	else
	{
		m_UrlList.Lookup(strUrl,pItem);
		bIsHash = false;
	}

	if(NULL==pItem)
		return;

	POSITION pos = pItem->m_lMetaLinkURLList.GetHeadPosition();
	while(pos)
	{  
		CUrlSite& urlSite = pItem->m_lMetaLinkURLList.GetNext(pos);
		if(pUrlSite->m_strUrl == urlSite.m_strUrl)
		{
			return;
		}
	}  

	pItem->m_lMetaLinkURLList.AddTail(*pUrlSite);
	m_fmdb->UpdateUrl(pItem, pUrlSite, bIsHash ? lpHash : NULL);
}

void CFileMgr::OnTimer(UINT /*nIDEvent*/)
{
   POSITION pos = m_UrlList.GetHeadPosition();
   while (pos)
   {
	   CFileTaskItem *pItem = m_UrlList.GetValueAt(pos);
	   if (pItem->m_nFileState ==  FILESTATE_DOWNLOADING)
	   {
		   POSITION position = pItem->m_lMetaLinkURLList.GetHeadPosition();
		   while (position)
		   {
			   CUrlSite pSite = pItem->m_lMetaLinkURLList.GetAt(position);
			   if (pSite.m_dwDataTransferedWithoutPayload != pSite.m_dwOldDataTransferedWithoutPayload)
			   {
				   m_fmdb->UpdateUrl(pItem,&pSite,NULL);
			   } 
			   pItem->m_lMetaLinkURLList.GetNext(position);
		   } 
	   }
	   m_UrlList.GetNext(pos);
   }
}

bool CFileMgr::AsyncShareList()
{
	bool result = false;
	POSITION posNext = m_FileList.GetHeadPosition();
	POSITION pos;
	while (posNext)
	{
		pos = posNext;
		FILEKEY filekey = m_FileList.GetKeyAt(pos);
		CFileTaskItem* pNewItem = m_FileList.GetNextValue(posNext);

		if( pNewItem->m_FileName.IsEmpty() && pNewItem->m_strFilePath.IsEmpty() )
		{
			m_FileList.RemoveAt(pos);
			result = true;
			delete pNewItem; 
			pNewItem  = NULL;
			continue;
		}					
		int nOldState = pNewItem->m_nFileState;
		UINT32 nOldMetBakId = pNewItem->m_metBakId;

		if(!AfterLoadFileItem(pNewItem,filekey.key))
		{
			pNewItem->m_nFileState = FILESTATE_DELETED;

		}
#ifdef _DEBUG
		if(pNewItem) TRACE(_T("%02d file loaded: %s\n"), pos, pNewItem->m_strFilePath);
#endif
		if( nOldState==FILESTATE_LOCAL_SHARE && pNewItem->m_nFileState==FILESTATE_DELETED )
		{
			m_FileList.RemoveAt(pos);
			delete pNewItem; //对于本地添加的分享,如果已经被删除,filemgr不再管理,其它的仍然保存					
			pNewItem  = NULL;
		}

		if( pNewItem==NULL || pNewItem->m_nFileState!=nOldState
			|| nOldMetBakId != pNewItem->m_metBakId )
			result = true;
	}

	pos = m_WaitforHashList.GetHeadPosition();
	while (pos)
	{
		CString strFile = m_WaitforHashList.GetKeyAt(pos);
		CGlobalVariable::sharedfiles->HashNewFile(strFile);
		m_WaitforHashList.GetNext(pos);
	}
	return result;
}
// VC-wangna[2007-012-18]: Add the MetalinkURL to FileList

