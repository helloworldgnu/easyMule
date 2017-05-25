/* 
 * $Id: SharedFileList.cpp 11864 2009-04-01 03:46:20Z huby $
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
#include "SharedFileList.h"
#include "KnownFileList.h"
#include "Packets.h"
#include "Kademlia/Kademlia/Kademlia.h"
#include "kademlia/kademlia/search.h"
#include "kademlia/kademlia/SearchManager.h"
#include "kademlia/kademlia/prefs.h"
#include "DownloadQueue.h"
#include "Statistics.h"
#include "Preferences.h"
#include "OtherFunctions.h"
#include "KnownFile.h"
#include "Sockets.h"
#include "SafeFile.h"
#include "Server.h"
#include "UpDownClient.h"
#include "PartFile.h"
//#include "emuledlg.h"
//#include "SharedFilesWnd.h"
#include "StringConversion.h"
#include "ClientList.h"
#include "Log.h"
#include "Collection.h"
#include "GlobalVariable.h"
#include "UIMessage.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


typedef CSimpleArray<CKnownFile*> CSimpleKnownFileArray;


///////////////////////////////////////////////////////////////////////////////
// CPublishKeyword

class CPublishKeyword
{
public:
	CPublishKeyword(const CStringW& rstrKeyword)
	{
		m_strKeyword = rstrKeyword;
		// min. keyword char is allowed to be < 3 in some cases (see also 'CSearchManager::GetWords')
		//ASSERT( rstrKeyword.GetLength() >= 3 );
		ASSERT( !rstrKeyword.IsEmpty() );
		KadGetKeywordHash(rstrKeyword, &m_nKadID);
		SetNextPublishTime(0);
		SetPublishedCount(0);
	}

	const Kademlia::CUInt128& GetKadID() const { return m_nKadID; }
	const CStringW& GetKeyword() const { return m_strKeyword; }
	int GetRefCount() const { return m_aFiles.GetSize(); }
	const CSimpleKnownFileArray& GetReferences() const { return m_aFiles; }

	UINT GetNextPublishTime() const { return m_tNextPublishTime; }
	void SetNextPublishTime(UINT tNextPublishTime) { m_tNextPublishTime = tNextPublishTime; }

	UINT GetPublishedCount() const { return m_uPublishedCount; }
	void SetPublishedCount(UINT uPublishedCount) { m_uPublishedCount = uPublishedCount; }
	void IncPublishedCount() { m_uPublishedCount++; }

	BOOL AddRef(CKnownFile* pFile)
	{
		if (m_aFiles.Find(pFile) != -1)
		{
			return FALSE;
		}
		return m_aFiles.Add(pFile);
	}

	int RemoveRef(CKnownFile* pFile)
	{
		m_aFiles.Remove(pFile);
		return m_aFiles.GetSize();
	}

	void RemoveAllReferences()
	{
		m_aFiles.RemoveAll();
	}

	void RotateReferences(int iRotateSize)
	{
		if (m_aFiles.GetSize() > iRotateSize)
		{
			CKnownFile** ppRotated = (CKnownFile**)malloc(m_aFiles.m_nAllocSize * sizeof(*m_aFiles.GetData()));
			if (ppRotated != NULL)
			{
				memcpy(ppRotated, m_aFiles.GetData() + iRotateSize, (m_aFiles.GetSize() - iRotateSize) * sizeof(*m_aFiles.GetData()));
				memcpy(ppRotated + m_aFiles.GetSize() - iRotateSize, m_aFiles.GetData(), iRotateSize * sizeof(*m_aFiles.GetData()));
				free(m_aFiles.GetData());
				m_aFiles.m_aT = ppRotated;
			}
		}
	}

protected:
	CStringW m_strKeyword;
	Kademlia::CUInt128 m_nKadID;
	UINT m_tNextPublishTime;
	UINT m_uPublishedCount;
	CSimpleKnownFileArray m_aFiles;
};


///////////////////////////////////////////////////////////////////////////////
// CPublishKeywordList

class CPublishKeywordList
{
public:
	CPublishKeywordList();
	~CPublishKeywordList();

	void AddKeywords(CKnownFile* pFile);
	void RemoveKeywords(CKnownFile* pFile);
	void RemoveAllKeywords();

	void RemoveAllKeywordReferences();
	void PurgeUnreferencedKeywords();

	int GetCount() const { return m_lstKeywords.GetCount(); }

	CPublishKeyword* GetNextKeyword();
	void ResetNextKeyword();

	UINT GetNextPublishTime() const { return m_tNextPublishKeywordTime; }
	void SetNextPublishTime(UINT tNextPublishKeywordTime) { m_tNextPublishKeywordTime = tNextPublishKeywordTime; }

#ifdef _DEBUG
	void Dump();
#endif

protected:
	// can't use a CMap - too many disadvantages in processing the 'list'
	//CTypedPtrMap<CMapStringToPtr, CString, CPublishKeyword*> m_lstKeywords;
	CTypedPtrList<CPtrList, CPublishKeyword*> m_lstKeywords;
	POSITION m_posNextKeyword;
	UINT m_tNextPublishKeywordTime;

	CPublishKeyword* FindKeyword(const CStringW& rstrKeyword, POSITION* ppos = NULL) const;
};

CPublishKeywordList::CPublishKeywordList()
{
	ResetNextKeyword();
	SetNextPublishTime(0);
}

CPublishKeywordList::~CPublishKeywordList()
{
	RemoveAllKeywords();
}

CPublishKeyword* CPublishKeywordList::GetNextKeyword()
{
	if (m_posNextKeyword == NULL)
	{
		m_posNextKeyword = m_lstKeywords.GetHeadPosition();
		if (m_posNextKeyword == NULL)
			return NULL;
	}
	return m_lstKeywords.GetNext(m_posNextKeyword);
}

void CPublishKeywordList::ResetNextKeyword()
{
	m_posNextKeyword = m_lstKeywords.GetHeadPosition();
}

CPublishKeyword* CPublishKeywordList::FindKeyword(const CStringW& rstrKeyword, POSITION* ppos) const
{
	POSITION pos = m_lstKeywords.GetHeadPosition();
	while (pos)
	{
		POSITION posLast = pos;
		CPublishKeyword* pPubKw = m_lstKeywords.GetNext(pos);
		if (pPubKw->GetKeyword() == rstrKeyword)
		{
			if (ppos)
				*ppos = posLast;
			return pPubKw;
		}
	}
	return NULL;
}

void CPublishKeywordList::AddKeywords(CKnownFile* pFile)
{
	const Kademlia::WordList& wordlist = pFile->GetKadKeywords();
	//ASSERT( wordlist.size() > 0 );
	Kademlia::WordList::const_iterator it;
	for (it = wordlist.begin(); it != wordlist.end(); it++)
	{
		const CStringW& strKeyword = *it;
		CPublishKeyword* pPubKw = FindKeyword(strKeyword);
		if (pPubKw == NULL)
		{
			pPubKw = new CPublishKeyword(strKeyword);
			m_lstKeywords.AddTail(pPubKw);
			SetNextPublishTime(0);
		}
		if(pPubKw->AddRef(pFile) && pPubKw->GetNextPublishTime() > MIN2S(30))
		{
			// User may be adding and removing files, so if this is a keyword that
			// has already been published, we reduce the time, but still give the user
			// enough time to finish what they are doing.
			// If this is a hot node, the Load list will prevent from republishing.
			pPubKw->SetNextPublishTime(MIN2S(30));
		}
	}
}

void CPublishKeywordList::RemoveKeywords(CKnownFile* pFile)
{
	const Kademlia::WordList& wordlist = pFile->GetKadKeywords();
	//ASSERT( wordlist.size() > 0 );
	Kademlia::WordList::const_iterator it;
	for (it = wordlist.begin(); it != wordlist.end(); it++)
	{
		const CStringW& strKeyword = *it;
		POSITION pos;
		CPublishKeyword* pPubKw = FindKeyword(strKeyword, &pos);
		if (pPubKw != NULL)
		{
			if (pPubKw->RemoveRef(pFile) == 0)
			{
				if (pos == m_posNextKeyword)
					(void)m_lstKeywords.GetNext(m_posNextKeyword);
				m_lstKeywords.RemoveAt(pos);
				delete pPubKw;
				SetNextPublishTime(0);
			}
		}
	}
}

void CPublishKeywordList::RemoveAllKeywords()
{
	POSITION pos = m_lstKeywords.GetHeadPosition();
	while (pos)
		delete m_lstKeywords.GetNext(pos);
	m_lstKeywords.RemoveAll();
	ResetNextKeyword();
	SetNextPublishTime(0);
}

void CPublishKeywordList::RemoveAllKeywordReferences()
{
	POSITION pos = m_lstKeywords.GetHeadPosition();
	while (pos)
		m_lstKeywords.GetNext(pos)->RemoveAllReferences();
}

void CPublishKeywordList::PurgeUnreferencedKeywords()
{
	POSITION pos = m_lstKeywords.GetHeadPosition();
	while (pos)
	{
		POSITION posLast = pos;
		CPublishKeyword* pPubKw = m_lstKeywords.GetNext(pos);
		if (pPubKw->GetRefCount() == 0)
		{
			if (posLast == m_posNextKeyword)
				(void)m_lstKeywords.GetNext(m_posNextKeyword);
			m_lstKeywords.RemoveAt(posLast);
			delete pPubKw;
			SetNextPublishTime(0);
		}
	}
}

#ifdef _DEBUG
void CPublishKeywordList::Dump()
{
	int i = 0;
	POSITION pos = m_lstKeywords.GetHeadPosition();
	while (pos)
	{
		CPublishKeyword* pPubKw = m_lstKeywords.GetNext(pos);
		TRACE(_T("%3u: %-10ls  ref=%u  %s\n"), i, pPubKw->GetKeyword(), pPubKw->GetRefCount(), CastSecondsToHM(pPubKw->GetNextPublishTime()));
		i++;
	}
}
#endif


///////////////////////////////////////////////////////////////////////////////
// CSharedFileList

CSharedFileList::CSharedFileList(CServerConnect* in_server)
{
	server = in_server;
//	output = 0;
	m_Files_map.InitHashTable(1031);
	m_keywords = new CPublishKeywordList;
	m_lastPublishED2K = 0;
	m_lastPublishED2KFlag = true;
	m_currFileSrc = 0;
	m_currFileNotes = 0;
	m_lastPublishKadSrc = 0;
	m_lastPublishKadNotes = 0;
	m_currFileKey = 0;
	//FindSharedFiles();
}

CSharedFileList::~CSharedFileList(){
	while (!waitingforhash_list.IsEmpty()){
		UnknownFile_Struct* nextfile = waitingforhash_list.RemoveHead();
		delete nextfile;
	}
	// SLUGFILLER: SafeHash
	while (!currentlyhashing_list.IsEmpty()){
		UnknownFile_Struct* nextfile = currentlyhashing_list.RemoveHead();
		delete nextfile;
	}
	// SLUGFILLER: SafeHash
	delete m_keywords;
}

void CSharedFileList::CopySharedFileMap(CMap<CCKey,const CCKey&,CKnownFile*,CKnownFile*> &Files_Map)
{
	if (!m_Files_map.IsEmpty())
	{
		POSITION pos = m_Files_map.GetStartPosition();
		while (pos)
		{
			CCKey key;
			CKnownFile* cur_file;
			m_Files_map.GetNextAssoc(pos, key, cur_file);
			Files_Map.SetAt(key, cur_file);
		}
	}
}

/**
*	<BR>功能说明: 把某个目录下的文件按参数限定的逻辑规则添加到分享列表中			
*	@author VC-Huby
*	@param strDir              需要处理的目录..  
*	@param bSubDirectories     是否处理该目录下的子目录
*	@param strListAdded        用于记录已被处理的分享目录列表..
*	@param bAddOnlyInKnownFile 只有在knownfile中已经存在的hash记录的才被处理(保证用户从网上下的文件即使移动到子目录后也能被分享)
*	@param bOnlyAddFileInSubDir 是否只加载该目录下的子目录下的文件，该目录下的被忽略
*	@return
*/
void CSharedFileList::AddShareFilesFromDir(CString strDir, bool bSubDirectories,CStringList& strListAdded,bool bAddOnlyInKnownFile,bool bOnlyAddFileInSubDir,int iLevel)
{
	if(!bOnlyAddFileInSubDir)
	{
		AddFilesFromDirectory(strDir,bAddOnlyInKnownFile,true);
		strDir.MakeLower();
		strListAdded.AddTail(strDir);
	}

	iLevel--;
	if(iLevel==0)
		return;  //[VC-Huby-080701]: 目录层数限制
	
	if (bSubDirectories)
	{
		if (strDir.Right(1) != _T("\\"))
			strDir += _T("\\");
		CFileFind finder;
		BOOL bWorking = finder.FindFile(strDir+_T("*.*"));
		while (bWorking)
		{
			bWorking = finder.FindNextFile();
			if (finder.IsDots() || finder.IsSystem() || !finder.IsDirectory())
				continue;
			AddShareFilesFromDir(strDir + finder.GetFileName(), true,strListAdded,bAddOnlyInKnownFile,false,iLevel);
		}
		finder.Close();
	}
}

void CSharedFileList::FindSharedFiles()
{
	if (!m_Files_map.IsEmpty())
	{
		CSingleLock listlock(&m_mutWriteList);
		
		POSITION pos = m_Files_map.GetStartPosition();
		while (pos)
		{
			CCKey key;
			CKnownFile* cur_file;
			m_Files_map.GetNextAssoc(pos, key, cur_file);
			if (cur_file->IsKindOf(RUNTIME_CLASS(CPartFile)) 
				&& !CGlobalVariable::downloadqueue->IsPartFile(cur_file) 
				&& !CGlobalVariable::knownfiles->IsFilePtrInList(cur_file)
				&& _taccess(cur_file->GetFilePath(), 0) == 0)
				continue;
			m_UnsharedFiles_map.SetAt(CSKey(cur_file->GetFileHash()), true);
			listlock.Lock();
			m_Files_map.RemoveKey(key);
			listlock.Unlock();
		}
		//  Comment By yunchenn
		//ASSERT( CGlobalVariable::downloadqueue );
		//if (CGlobalVariable::downloadqueue)
		//	CGlobalVariable::downloadqueue->AddPartFilesToShare(); // read partfiles
	}

	//  moved By yunchenn
	ASSERT( CGlobalVariable::downloadqueue );
	if (CGlobalVariable::downloadqueue)
		CGlobalVariable::downloadqueue->AddPartFilesToShare(); // read partfiles

	// khaos::kmod+ Fix: Shared files loaded multiple times.
	CStringList l_sAdded;
	CString tempDir;
	CString ltempDir;

	tempDir = thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR);
	if (tempDir.Right(1)!=_T("\\"))
		tempDir+=_T("\\");	
	AddFilesFromDirectory(tempDir); //缺省下载目录第一级下的所有文件都被分享(包括本地的文件)
	tempDir.MakeLower();
	l_sAdded.AddHead( tempDir );

	/// 考虑有些用户会在incoming 下建子目录，然后把incoming 下已下载完成文件的移动到下面的子目录..	
	AddShareFilesFromDir(tempDir, true,l_sAdded,true,true,3);
	
	AddFilesFromDirectory(thePrefs.GetMuleDirectory(EMULE_UPDATEDIR));

	for (int ix=1;ix<thePrefs.GetCatCount();ix++)
	{
		tempDir=CString( thePrefs.GetCatPath(ix) );
		if (tempDir.Right(1)!=_T("\\"))
			tempDir+=_T("\\");
		ltempDir=tempDir;
		ltempDir.MakeLower();

		if( l_sAdded.Find( ltempDir ) ==NULL ) {
			l_sAdded.AddHead( ltempDir );
			AddFilesFromDirectory(tempDir);
		}
	}

	for (POSITION pos = thePrefs.shareddir_list.GetHeadPosition();pos != 0;)
	{
		tempDir = thePrefs.shareddir_list.GetNext(pos);
		if (tempDir.Right(1)!=_T("\\"))
			tempDir+=_T("\\");
		ltempDir= tempDir;
		ltempDir.MakeLower();

		if( l_sAdded.Find( ltempDir ) ==NULL ) 
		{
			l_sAdded.AddHead( ltempDir );
			AddFilesFromDirectory(tempDir);
		}
	}

	// khaos::kmod-
	//  Comment UI
	if (waitingforhash_list.IsEmpty())
		AddLogLine(false,GetResString(IDS_SHAREDFOUND), m_Files_map.GetCount());
	else
		AddLogLine(false,GetResString(IDS_SHAREDFOUNDHASHING), m_Files_map.GetCount(), waitingforhash_list.GetCount());
	
	HashNextFile();
}

/// 过滤不需要被分享的文件 [VC-Huby-2008/03/06]
bool CSharedFileList::FileIsIgnored( const CFileFind& ff )
{
	if (ff.IsDirectory() || ff.IsDots() || ff.IsSystem() || ff.IsTemporary() || ff.IsHidden() || ff.GetLength()==0 || ff.GetLength()>MAX_EMULE_FILE_SIZE)
		return true;

	// ignore real(!) LNK files
	TCHAR szExt[_MAX_EXT];
	_tsplitpath(ff.GetFileName(), NULL, NULL, NULL, szExt);
	if (_tcsicmp(szExt, _T(".lnk")) == 0)
	{
		SHFILEINFO info;
		if (SHGetFileInfo(ff.GetFilePath(), 0, &info, sizeof(info), SHGFI_ATTRIBUTES) && (info.dwAttributes & SFGAO_LINK))
		{
			CComPtr<IShellLink> pShellLink;
			if (SUCCEEDED(pShellLink.CoCreateInstance(CLSID_ShellLink))){
				CComQIPtr<IPersistFile> pPersistFile = pShellLink;
				if (pPersistFile)
				{
					USES_CONVERSION;
					if (SUCCEEDED(pPersistFile->Load(T2COLE(ff.GetFilePath()), STGM_READ)))
					{
						TCHAR szResolvedPath[MAX_PATH];
						if (pShellLink->GetPath(szResolvedPath, ARRSIZE(szResolvedPath), NULL, 0) == NOERROR)
						{
							TRACE(_T("%hs: Did not share file \"%s\" - not supported file type\n"), __FUNCTION__, ff.GetFilePath());
							return true;
						}
					}
				}
			}
		}
	}

	// ignore real(!) thumbs.db files -- seems that lot of ppl have 'thumbs.db' files without the 'System' file attribute
	if (ff.GetFileName().CompareNoCase(_T("thumbs.db")) == 0)
	{
		// if that's a valid 'Storage' file, we declare it as a "thumbs.db" file.
		USES_CONVERSION;
		CComPtr<IStorage> pStorage;
		if (StgOpenStorage(T2CW(ff.GetFilePath()), NULL, STGM_READ | STGM_SHARE_DENY_WRITE, NULL, 0, &pStorage) == S_OK)
		{
			CComPtr<IEnumSTATSTG> pEnumSTATSTG;
			if (SUCCEEDED(pStorage->EnumElements(0, NULL, 0, &pEnumSTATSTG)))
			{
				STATSTG statstg = {0};
				if (pEnumSTATSTG->Next(1, &statstg, 0) == S_OK)
				{
					CoTaskMemFree(statstg.pwcsName);
					statstg.pwcsName = NULL;
					TRACE(_T("%hs: Did not share file \"%s\" - not supported file type\n"), __FUNCTION__, ff.GetFilePath());
					return true;
				}
			}
		}
	}

	CString strTempPath = ff.GetFilePath();

	if(strTempPath.Right(5).CompareNoCase(_T(".part"))==0
		|| strTempPath.Right(9).CompareNoCase(_T(".part.met"))==0
		|| strTempPath.Right(13).CompareNoCase(_T(".part.met.bak"))==0)
	{
		return true;
	}

	return false;
}

// 此函数不处理子目录下的文件..
// bAddOnlyInKnownFile : 是否只添加已经在KnownFile中有hash信息保存的
void CSharedFileList::AddFilesFromDirectory(const CString& rstrDirectory,bool bAddOnlyInKnownFile,bool bOnlyGetKnownPath)
{
	CFileFind ff;
	
	CString searchpath;
	searchpath.Format(_T("%s\\*"),rstrDirectory);
	bool end = !ff.FindFile(searchpath,0);
	if (end)
		return;

	while (!end)
	{
		end = !ff.FindNextFile();

		if( FileIsIgnored(ff) )
			continue;

		CTime lwtime;
		try{
			ff.GetLastWriteTime(lwtime);
		}
		catch(CException* ex){
			ex->Delete();
		}
		uint32 fdate = (UINT)lwtime.GetTime();
		if (fdate == 0)
			fdate = (UINT)-1;
		if (fdate == -1){
			if (thePrefs.GetVerbose())
				AddDebugLogLine(false, _T("Failed to get file date of \"%s\""), ff.GetFilePath());
		}
		else
			AdjustNTFSDaylightFileTime(fdate, ff.GetFilePath());

		CKnownFile* toadd = CGlobalVariable::knownfiles->FindKnownFile(ff.GetFileName(), fdate, ff.GetLength());
		if (toadd)
		{
			CCKey key(toadd->GetFileHash());
			CKnownFile* pFileInMap;
			if (m_Files_map.Lookup(key, pFileInMap))
			{
				TRACE(_T("%hs: File already in shared file list: %s \"%s\"\n"), __FUNCTION__, md4str(pFileInMap->GetFileHash()), pFileInMap->GetFilePath());
				TRACE(_T("%hs: File to add:                      %s \"%s\"\n"), __FUNCTION__, md4str(toadd->GetFileHash()), ff.GetFilePath());
				//  Comment UI
				if (!pFileInMap->IsKindOf(RUNTIME_CLASS(CPartFile)) || CGlobalVariable::downloadqueue->IsPartFile(pFileInMap)
					&& pFileInMap->GetFilePath()!=ff.GetFilePath() )
					LogWarning( GetResString(IDS_ERR_DUPL_FILES) , pFileInMap->GetFilePath(), ff.GetFilePath());
			}
			else
			{
				toadd->SetPath(rstrDirectory);
				toadd->SetFilePath(ff.GetFilePath());
				if(!bOnlyGetKnownPath)
					AddFile(toadd);
			}
		}
		else
		{
			if(!bAddOnlyInKnownFile)
			{
				//not in knownfilelist - start adding thread to hash file if the hashing of this file isnt already waiting
				// SLUGFILLER: SafeHash - don't double hash, MY way
				if (!IsHashing(rstrDirectory, ff.GetFileName()) && !thePrefs.IsTempFile(rstrDirectory, ff.GetFileName())){
					UnknownFile_Struct* tohash = new UnknownFile_Struct;
					tohash->strDirectory = rstrDirectory;
					tohash->strName = ff.GetFileName();
					waitingforhash_list.AddTail(tohash);
				}
				else
					TRACE(_T("%hs: Did not share file \"%s\" - already hashing or temp. file\n"), __FUNCTION__, ff.GetFilePath());
				// SLUGFILLER: SafeHash
			}			
		}
	}
	ff.Close();
}

void CSharedFileList::AddFileFromNewlyCreatedCollection(const CString& path, const CString& fileName)
{
	//JOHNTODO: I do not have much knowledge on the hashing 
	//          process.. Is this safe for me to do??
	if (!IsHashing(path, fileName))
	{
		UnknownFile_Struct* tohash = new UnknownFile_Struct;
		tohash->strDirectory = path;
		tohash->strName = fileName;
		waitingforhash_list.AddTail(tohash);
		HashNextFile();
	}
}

bool CSharedFileList::SafeAddKFile(CKnownFile* toadd, bool bOnlyAdd)
{
	bool bAdded = false;
	RemoveFromHashing(toadd);	// SLUGFILLER: SafeHash - hashed ok, remove from list, in case it was on the list
	bAdded = AddFile(toadd);
	if (bOnlyAdd)
		return bAdded;
	//  Comment UI
	//if (bAdded && output)
	//	output->AddFile(toadd);
	if(bAdded)
		UINotify(WM_SHAREDFILE_ADDFILE, 0, (LPARAM)toadd, toadd);
	m_lastPublishED2KFlag = true;
	return bAdded;
}

void CSharedFileList::RepublishFile(CKnownFile* pFile)
{
	CServer* pCurServer = server->GetCurrentServer();
	if (pCurServer && (pCurServer->GetTCPFlags() & SRV_TCPFLG_COMPRESSION))
	{
		m_lastPublishED2KFlag = true;
		pFile->SetPublishedED2K(false); // FIXME: this creates a wrong 'No' for the ed2k shared info in the listview until the file is shared again.
	}
}

bool CSharedFileList::AddFile(CKnownFile* pFile)
{
	ASSERT( pFile->GetHashCount() == pFile->GetED2KPartHashCount() );
	ASSERT( !pFile->IsKindOf(RUNTIME_CLASS(CPartFile)) || !STATIC_DOWNCAST(CPartFile, pFile)->hashsetneeded );

	if( this->IsFileAlreadyExist( pFile ) )
		return false;

	if( pFile->HasNullHash() )
		return false;

	CCKey key(pFile->GetFileHash());
	CKnownFile* pFileInMap;
	if (m_Files_map.Lookup(key, pFileInMap))
	{
		TRACE(_T("%hs: File already in shared file list: %s \"%s\" \"%s\"\n"), __FUNCTION__, md4str(pFileInMap->GetFileHash()), pFileInMap->GetFileName(), pFileInMap->GetFilePath());
		TRACE(_T("%hs: File to add:                      %s \"%s\" \"%s\"\n"), __FUNCTION__, md4str(pFile->GetFileHash()), pFile->GetFileName(), pFile->GetFilePath());
		//  Comment UI
		if (!pFileInMap->IsKindOf(RUNTIME_CLASS(CPartFile)) || CGlobalVariable::downloadqueue->IsPartFile(pFileInMap)
			&& pFileInMap->GetFilePath()!=pFile->GetFilePath() )
			LogWarning(GetResString(IDS_ERR_DUPL_FILES), pFileInMap->GetFilePath(), pFile->GetFilePath());
		return false;
	}
	m_UnsharedFiles_map.RemoveKey(CSKey(pFile->GetFileHash()));
	
	CSingleLock listlock(&m_mutWriteList);
	listlock.Lock();	
	m_Files_map.SetAt(key, pFile);
	listlock.Unlock();

	bool bKeywordsNeedUpdated = true;

	if(!pFile->IsPartFile() && !pFile->m_pCollection && CCollection::HasCollectionExtention(pFile->GetFileName()))
	{
		pFile->m_pCollection = new CCollection();
		if(!pFile->m_pCollection->InitCollectionFromFile(pFile->GetFilePath(), pFile->GetFileName()))
		{
			delete pFile->m_pCollection;
			pFile->m_pCollection = NULL;
		}
		else if (!pFile->m_pCollection->GetCollectionAuthorKeyString().IsEmpty())
		{
			//If the collection has a key, resetting the file name will
			//cause the key to be added into the wordlist to be stored
			//into Kad.
			pFile->SetFileName(pFile->GetFileName());
			//During the initial startup, sharedfiles is not accessable
			//to SetFileName which will then not call AddKeywords..
			//But when it is accessable, we don't allow it to readd them.
			if(CGlobalVariable::sharedfiles)
				bKeywordsNeedUpdated = false;
		}
	}

	if(bKeywordsNeedUpdated)
		m_keywords->AddKeywords(pFile);

	return true;
}

bool CSharedFileList::IsFileAlreadyExist(CKnownFile* pFile)
{
	POSITION p = this->m_Files_map.GetStartPosition();

	CCKey key;
	CKnownFile* file = 0;

	while( p != NULL ) {
		this->m_Files_map.GetNextAssoc( p , key , file );
		if( file == pFile )
			return true;
	}

	return false;
}

bool CSharedFileList::FileHashingFinished(CKnownFile* file,CKnownFile** ppFileInKnownList)
{
	// File hashing finished for a shared file (none partfile)
	//	- reading shared directories at startup and hashing files which were not found in known.met
	//	- reading shared directories during runtime (user hit Reload button, added a shared directory, ...)

	ASSERT( !IsFilePtrInList(file) );
	ASSERT( !CGlobalVariable::knownfiles->IsFilePtrInList(file) );

	CKnownFile* found_file = GetFileByID(file->GetFileHash());
	if (found_file == NULL)
	{
		SafeAddKFile(file);
		CGlobalVariable::knownfiles->SafeAddKFile(file,false,ppFileInKnownList);
		return true;
	}
	else
	{
		TRACE(_T("%hs: File already in shared file list: %s \"%s\"\n"), __FUNCTION__, md4str(found_file->GetFileHash()), found_file->GetFilePath());
		TRACE(_T("%hs: File to add:                      %s \"%s\"\n"), __FUNCTION__, md4str(file->GetFileHash()), file->GetFilePath());
		//  Comment UI
		//LogWarning(GetResString(IDS_ERR_DUPL_FILES), found_file->GetFilePath(), file->GetFilePath());

		RemoveFromHashing(file);
		if (!IsFilePtrInList(file) && !CGlobalVariable::knownfiles->IsFilePtrInList(file))
			delete file;
		else
			ASSERT(0);

		return false;
	}
}

bool CSharedFileList::RemoveFile(CKnownFile* pFile,bool bOnlyUnShare)
{
	if (pFile->HasNullHash())
	{
		UINotify(WM_SHAREDFILE_REMOVEFILE, 0, (LPARAM)pFile);
		return true;
	}

	CSingleLock listlock(&m_mutWriteList);
	listlock.Lock();
	bool bResult = (m_Files_map.RemoveKey(CCKey(pFile->GetFileHash())) != FALSE);
	listlock.Unlock();

//	CGlobalVariable::filemgr.RemoveFileItem(pFile);
	if (bResult)
	{
		//  Comment UI
		//output->RemoveFile(pFile);
		//SendMessage(CGlobalVariable::m_hListenWnd, WM_SHAREDFILE_REMOVEFILE, 0, (LPARAM)pFile);
		if( !bOnlyUnShare )
		{
			UINotify(WM_SHAREDFILE_REMOVEFILE, 0, (LPARAM)pFile);
		}

		m_UnsharedFiles_map.SetAt(CSKey(pFile->GetFileHash()), true);
	}

	m_keywords->RemoveKeywords(pFile);
	return bResult;
}

void CSharedFileList::Reload()
{
	m_keywords->RemoveAllKeywordReferences();
	FindSharedFiles();
	m_keywords->PurgeUnreferencedKeywords();

	RemoveHashingInUnSharedDir( );
	CGlobalVariable::filemgr.ReloadLocalSharedFiles();
	//  Comment UI
	//if (output)
	//	output->ReloadFileList();
	PostMessage(CGlobalVariable::m_hListenWnd, WM_SHAREDFILE_RELOADFILELIST, 0, 0);
}

//void CSharedFileList::SetOutputCtrl(CSharedFilesCtrl* in_ctrl)
//{
//	//  Comment UI
//	/*output = in_ctrl;
//	output->ReloadFileList();*/
//	HashNextFile();		// SLUGFILLER: SafeHash - if hashing not yet started, start it now
//}

uint8 GetRealPrio(uint8 in)
{
	switch(in) {
		case 4 : return 0;
		case 0 : return 1;
		case 1 : return 2;
		case 2 : return 3;
		case 3 : return 4;
	}
	return 0;
}

void CSharedFileList::SendListToServer(){
	if (m_Files_map.IsEmpty() || !server->IsConnected())
	{
		return;
	}
	
	CServer* pCurServer = server->GetCurrentServer();
	CSafeMemFile files(1024);
	CCKey bufKey;
	CKnownFile* cur_file,cur_file2;
	POSITION pos,pos2;
	CTypedPtrList<CPtrList, CKnownFile*> sortedList;
	bool added=false;

	for(pos=m_Files_map.GetStartPosition(); pos!=0;)
	{
		m_Files_map.GetNextAssoc(pos, bufKey, cur_file);
		added=false;
		//insertsort into sortedList
		if(!cur_file->GetPublishedED2K() && (!cur_file->IsLargeFile() || (pCurServer != NULL && pCurServer->SupportsLargeFilesTCP())))
		{
			for (pos2 = sortedList.GetHeadPosition();pos2 != 0 && !added;sortedList.GetNext(pos2))
			{
				if (GetRealPrio(sortedList.GetAt(pos2)->GetUpPriority()) <= GetRealPrio(cur_file->GetUpPriority()) )
				{
					sortedList.InsertBefore(pos2,cur_file);
					added=true;
				}
			}
			if (!added)
			{
				sortedList.AddTail(cur_file);
			}
		}
	}

	
	// add to packet
	uint32 limit = pCurServer ? pCurServer->GetSoftFiles() : 0;
	if( limit == 0 || limit > 200 )
	{
		limit = 200;
	}
	if( (uint32)sortedList.GetCount() < limit )
	{
		limit = sortedList.GetCount();
		if (limit == 0)
		{
			m_lastPublishED2KFlag = false;
			return;
		}
	}
	files.WriteUInt32(limit);
	uint32 count=0;
	for (pos = sortedList.GetHeadPosition();pos != 0 && count<limit; )
	{
		count++;
		CKnownFile* file = sortedList.GetNext(pos);
		CreateOfferedFilePacket(file, &files, pCurServer);
		file->SetPublishedED2K(true);
	}
	sortedList.RemoveAll();
	Packet* packet = new Packet(&files);
	packet->opcode = OP_OFFERFILES;
	// compress packet
	//   - this kind of data is highly compressable (N * (1 MD4 and at least 3 string meta data tags and 1 integer meta data tag))
	//   - the min. amount of data needed for one published file is ~100 bytes
	//   - this function is called once when connecting to a server and when a file becomes shareable - so, it's called rarely.
	//   - if the compressed size is still >= the original size, we send the uncompressed packet
	// therefor we always try to compress the packet
	if (pCurServer && pCurServer->GetTCPFlags() & SRV_TCPFLG_COMPRESSION){
		UINT uUncomprSize = packet->size;
		packet->PackPacket();
		if (thePrefs.GetDebugServerTCPLevel() > 0)
			Debug(_T(">>> Sending OP__OfferFiles(compressed); uncompr size=%u  compr size=%u  files=%u\n"), uUncomprSize, packet->size, limit);
	}
	else{
		if (thePrefs.GetDebugServerTCPLevel() > 0)
			Debug(_T(">>> Sending OP__OfferFiles; size=%u  files=%u\n"), packet->size, limit);
	}
	theStats.AddUpDataOverheadServer(packet->size);
	if (thePrefs.GetVerbose())
		AddDebugLogLine(false, _T("Server, Sendlist: Packet size:%u"), packet->size);
	server->SendPacket(packet,true);
}

CKnownFile* CSharedFileList::GetFileByIndex(int index){
	int count=0;
	CKnownFile* cur_file;
	CCKey bufKey;

	for (POSITION pos = m_Files_map.GetStartPosition();pos != 0;){
		m_Files_map.GetNextAssoc(pos,bufKey,cur_file);
		if (index==count)
			return cur_file;
		count++;
	}
	return 0;
}

void CSharedFileList::ClearED2KPublishInfo()
{
	CKnownFile* cur_file;
	CCKey bufKey;
	m_lastPublishED2KFlag = true;
	for (POSITION pos = m_Files_map.GetStartPosition();pos != 0;)
	{
		m_Files_map.GetNextAssoc(pos,bufKey,cur_file);
		cur_file->SetPublishedED2K(false);
	}
}

void CSharedFileList::ClearKadSourcePublishInfo()
{
	CKnownFile* cur_file;
	CCKey bufKey;
	for (POSITION pos = m_Files_map.GetStartPosition();pos != 0;)
	{
		m_Files_map.GetNextAssoc(pos,bufKey,cur_file);
		cur_file->SetLastPublishTimeKadSrc(0,0);
	}
}

void CSharedFileList::CreateOfferedFilePacket(CKnownFile* cur_file, CSafeMemFile* files, 
											  CServer* pServer, CUpDownClient* pClient)
{
	UINT uEmuleVer = (pClient && pClient->IsEmuleClient()) ? pClient->GetVersion() : 0;

	// NOTE: This function is used for creating the offered file packet for Servers _and_ for Clients..
	files->WriteHash16(cur_file->GetFileHash());

	// *) This function is used for offering files to the local server and for sending
	//    shared files to some other client. In each case we send our IP+Port only, if
	//    we have a HighID.
	// *) Newer eservers also support 2 special IP+port values which are used to hold basic file status info.
	uint32 nClientID = 0;
	uint16 nClientPort = 0;
	if (pServer)
	{
		// we use the 'TCP-compression' server feature flag as indicator for a 'newer' server.
		if (pServer->GetTCPFlags() & SRV_TCPFLG_COMPRESSION)
		{
			if (cur_file->IsPartFile())
			{
				// publishing an incomplete file
				nClientID = 0xFCFCFCFC;
				nClientPort = 0xFCFC;
			}
			else
			{
				// publishing a complete file
				nClientID = 0xFBFBFBFB;
				nClientPort = 0xFBFB;
			}
		}
		else
		{
			// check eD2K ID state
			if (CGlobalVariable::serverconnect->IsConnected() && !CGlobalVariable::serverconnect->IsLowID())
			{
				//  Comment UI
				nClientID = CGlobalVariable::GetID();
				nClientPort = thePrefs.GetPort();
			}
		}
	}
	else
	{
		//  Comment UI
		if (CGlobalVariable::IsConnected() && !CGlobalVariable::IsFirewalled())
		{
			nClientID = CGlobalVariable::GetID();
			nClientPort = thePrefs.GetPort();
		}
	}
	files->WriteUInt32(nClientID);
	files->WriteUInt16(nClientPort);
	//TRACE(_T("Publishing file: Hash=%s  ClientIP=%s  ClientPort=%u\n"), md4str(cur_file->GetFileHash()), ipstr(nClientID), nClientPort);

	CSimpleArray<CTag*> tags;

	tags.Add(new CTag(FT_FILENAME, cur_file->GetFileName()));

	if (!cur_file->IsLargeFile()){
		tags.Add(new CTag(FT_FILESIZE, (uint32)(uint64)cur_file->GetFileSize()));
	}
	else{
		// we send 2*32 bit tags to servers, but a real 64 bit tag to other clients.
		if (pServer != NULL){
			if (!pServer->SupportsLargeFilesTCP()){
				ASSERT( false );
				tags.Add(new CTag(FT_FILESIZE, 0, false));
			}
			else{
				tags.Add(new CTag(FT_FILESIZE, (uint32)(uint64)cur_file->GetFileSize()));
				tags.Add(new CTag(FT_FILESIZE_HI, (uint32)((uint64)cur_file->GetFileSize() >> 32)));
			}
		}
		else{
			if (!pClient->SupportsLargeFiles()){
				ASSERT( false );
				tags.Add(new CTag(FT_FILESIZE, 0, false));
			}
			else{
				tags.Add(new CTag(FT_FILESIZE, cur_file->GetFileSize(), true));
			}
		}
	}

	// eserver 17.6+ supports eMule file rating tag. There is no TCP-capabilities bit available to determine
	// whether the server is really supporting it -- this is by intention (lug). That's why we always send it.
	if (cur_file->GetFileRating()) {
		uint32 uRatingVal = cur_file->GetFileRating();
		if (pClient) {
			// eserver is sending the rating which it received in a different format (see
			// 'CSearchFile::CSearchFile'). If we are creating the packet for an other client
			// we must use eserver's format.
			uRatingVal *= (255/5/*RatingExcellent*/);
		}
		tags.Add(new CTag(FT_FILERATING, uRatingVal));
	}

	// NOTE: Archives and CD-Images are published+searched with file type "Pro"
	bool bAddedFileType = false;
	if (pServer && (pServer->GetTCPFlags() & SRV_TCPFLG_TYPETAGINTEGER)) {
		// Send integer file type tags to newer servers
		EED2KFileType eFileType = GetED2KFileTypeSearchID(GetED2KFileTypeID(cur_file->GetFileName()));
		if (eFileType >= ED2KFT_AUDIO && eFileType <= ED2KFT_CDIMAGE) {
			tags.Add(new CTag(FT_FILETYPE, (UINT)eFileType));
			bAddedFileType = true;
		}
	}
	if (!bAddedFileType) {
		// Send string file type tags to:
		//	- newer servers, in case there is no integer type available for the file type (e.g. emulecollection)
		//	- older servers
		//	- all clients
		CString strED2KFileType(GetED2KFileTypeSearchTerm(GetED2KFileTypeID(cur_file->GetFileName())));
		if (!strED2KFileType.IsEmpty()) {
			tags.Add(new CTag(FT_FILETYPE, strED2KFileType));
			bAddedFileType = true;
		}
	}

	// eserver 16.4+ does not need the FT_FILEFORMAT tag at all nor does any eMule client. This tag
	// was used for older (very old) eDonkey servers only. -> We send it only to non-eMule clients.
	if (pServer == NULL && uEmuleVer == 0) {
		CString strExt;
		int iExt = cur_file->GetFileName().ReverseFind(_T('.'));
		if (iExt != -1){
			strExt = cur_file->GetFileName().Mid(iExt);
			if (!strExt.IsEmpty()){
				strExt = strExt.Mid(1);
				if (!strExt.IsEmpty()){
					strExt.MakeLower();
					tags.Add(new CTag(FT_FILEFORMAT, strExt)); // file extension without a "."
				}
			}
		}
	}

	// only send verified meta data to servers/clients
	if (cur_file->GetMetaDataVer() > 0)
	{
		static const struct
		{
			bool	bSendToServer;
			uint8	nName;
			uint8	nED2KType;
			LPCSTR	pszED2KName;
		} _aMetaTags[] = 
		{
			// Artist, Album and Title are disabled because they should be already part of the filename
			// and would therefore be redundant information sent to the servers.. and the servers count the
			// amount of sent data!
			{ false, FT_MEDIA_ARTIST,	TAGTYPE_STRING, FT_ED2K_MEDIA_ARTIST },
			{ false, FT_MEDIA_ALBUM,	TAGTYPE_STRING, FT_ED2K_MEDIA_ALBUM },
			{ false, FT_MEDIA_TITLE,	TAGTYPE_STRING, FT_ED2K_MEDIA_TITLE },
			{ true,  FT_MEDIA_LENGTH,	TAGTYPE_STRING, FT_ED2K_MEDIA_LENGTH },
			{ true,  FT_MEDIA_BITRATE,	TAGTYPE_UINT32, FT_ED2K_MEDIA_BITRATE },
			{ true,  FT_MEDIA_CODEC,	TAGTYPE_STRING, FT_ED2K_MEDIA_CODEC }
		};
		for (int i = 0; i < ARRSIZE(_aMetaTags); i++)
		{
			if (pServer!=NULL && !_aMetaTags[i].bSendToServer)
				continue;
			CTag* pTag = cur_file->GetTag(_aMetaTags[i].nName);
			if (pTag != NULL)
			{
				// skip string tags with empty string values
				if (pTag->IsStr() && pTag->GetStr().IsEmpty())
					continue;
				
				// skip integer tags with '0' values
				if (pTag->IsInt() && pTag->GetInt() == 0)
					continue;
				
				if (_aMetaTags[i].nED2KType == TAGTYPE_STRING && pTag->IsStr())
				{
					if (pServer && (pServer->GetTCPFlags() & SRV_TCPFLG_NEWTAGS))
						tags.Add(new CTag(_aMetaTags[i].nName, pTag->GetStr()));
					else
						tags.Add(new CTag(_aMetaTags[i].pszED2KName, pTag->GetStr()));
				}
				else if (_aMetaTags[i].nED2KType == TAGTYPE_UINT32 && pTag->IsInt())
				{
					if (pServer && (pServer->GetTCPFlags() & SRV_TCPFLG_NEWTAGS))
						tags.Add(new CTag(_aMetaTags[i].nName, pTag->GetInt()));
					else
						tags.Add(new CTag(_aMetaTags[i].pszED2KName, pTag->GetInt()));
				}
				else if (_aMetaTags[i].nName == FT_MEDIA_LENGTH && pTag->IsInt())
				{
					ASSERT( _aMetaTags[i].nED2KType == TAGTYPE_STRING );
					// All 'eserver' versions and eMule versions >= 0.42.4 support the media length tag with type 'integer'
					if (   pServer!=NULL && (pServer->GetTCPFlags() & SRV_TCPFLG_COMPRESSION)
						|| uEmuleVer >= MAKE_CLIENT_VERSION(0,42,4))
					{
						if (pServer && (pServer->GetTCPFlags() & SRV_TCPFLG_NEWTAGS))
							tags.Add(new CTag(_aMetaTags[i].nName, pTag->GetInt()));
						else
							tags.Add(new CTag(_aMetaTags[i].pszED2KName, pTag->GetInt()));
					}
					else
					{
						CString strValue;
						SecToTimeLength(pTag->GetInt(), strValue);
						tags.Add(new CTag(_aMetaTags[i].pszED2KName, strValue));
					}
				}
				else
					ASSERT(0);
			}
		}
	}

	EUtf8Str eStrEncode;
	if (pServer != NULL && (pServer->GetTCPFlags() & SRV_TCPFLG_UNICODE)){
		// eserver doesn't properly support searching with ASCII-7 strings in BOM-UTF8 published strings
		//eStrEncode = utf8strOptBOM;
		eStrEncode = utf8strRaw;
	}
	else if (pClient && !pClient->GetUnicodeSupport())
		eStrEncode = utf8strNone;
	else
		eStrEncode = utf8strRaw;

	files->WriteUInt32(tags.GetSize());
	for (int i = 0; i < tags.GetSize(); i++)
	{
		const CTag* pTag = tags[i];
		//TRACE(_T("  %s\n"), pTag->GetFullInfo(DbgGetFileMetaTagName));
		if (pServer && (pServer->GetTCPFlags() & SRV_TCPFLG_NEWTAGS) || (uEmuleVer >= MAKE_CLIENT_VERSION(0,42,7)))
			pTag->WriteNewEd2kTag(files, eStrEncode);
		else
			pTag->WriteTagToFile(files, eStrEncode);
		delete pTag;
	}
}

// -khaos--+++> New param:  pbytesLargest, pointer to uint64.
//				Various other changes to accomodate our new statistic...
//				Point of this is to find the largest file currently shared.
uint64 CSharedFileList::GetDatasize(uint64 &pbytesLargest) const
{
	pbytesLargest=0;
	// <-----khaos-
	uint64 fsize;
	fsize=0;

	CCKey bufKey;
	CKnownFile* cur_file;
	for (POSITION pos = m_Files_map.GetStartPosition();pos != 0;){
		m_Files_map.GetNextAssoc(pos,bufKey,cur_file);
		fsize += (uint64)cur_file->GetFileSize();
		// -khaos--+++> If this file is bigger than all the others...well duh.
		if (cur_file->GetFileSize() > pbytesLargest)
			pbytesLargest = cur_file->GetFileSize();
		// <-----khaos-
	}
	return fsize;
}

CKnownFile* CSharedFileList::GetFileByID(const uchar* hash) const
{
	if (hash)
	{
		CKnownFile* found_file;
		CCKey key(hash);
		if (m_Files_map.Lookup(key, found_file))
			return found_file;
	}
	return NULL;
}


bool CSharedFileList::IsFilePtrInList(const CKnownFile* file) const
{
	if (file)
	{
		POSITION pos = m_Files_map.GetStartPosition();
		while (pos)
		{
			CCKey key;
			CKnownFile* cur_file;
			m_Files_map.GetNextAssoc(pos, key, cur_file);
			if (file == cur_file)
				return true;
		}
	}
	return false;
}

void CSharedFileList::HashNextFile()
{
	// SLUGFILLER: SafeHash
	//  Comment UI
	/*if (!theApp.emuledlg || !::IsWindow(theApp.emuledlg->m_hWnd))	// wait for the dialog to open
		return;
	if (theApp.emuledlg && theApp.emuledlg->IsRunning())
		theApp.emuledlg->sharedfileswnd->sharedfilesctrl.ShowFilesCount();*/
	if(CGlobalVariable::IsRunning())
		PostMessage( CGlobalVariable::m_hListenWnd,WM_SHAREDFILE_SHOWCOUNT,0,0 ); 
	else return ;

	if (!currentlyhashing_list.IsEmpty())	// one hash at a time
		return;
	// SLUGFILLER: SafeHash
	if (waitingforhash_list.IsEmpty())
		return;
	UnknownFile_Struct* nextfile = waitingforhash_list.RemoveHead();
	m_hashingLocker.Lock();
	currentlyhashing_list.AddTail(nextfile);	// SLUGFILLER: SafeHash - keep track
	m_hashingLocker.Unlock();
	CAddFileThread* addfilethread = (CAddFileThread*) AfxBeginThread(RUNTIME_CLASS(CAddFileThread), THREAD_PRIORITY_BELOW_NORMAL,0, CREATE_SUSPENDED);
	addfilethread->SetValues(this,nextfile->strDirectory,nextfile->strName);
	addfilethread->ResumeThread();
	// SLUGFILLER: SafeHash - nextfile deleting handled elsewhere
	//delete nextfile;
}

// SLUGFILLER: SafeHash
bool CSharedFileList::IsHashing(const CString& rstrDirectory, const CString& rstrName,bool* pbWaiting){
	
	CString strDir = rstrDirectory;
	if (strDir.Right(1) != _T("\\") )
	{
		strDir += _T("\\");
	}

	if(pbWaiting)
		*pbWaiting = false;

	for (POSITION pos = waitingforhash_list.GetHeadPosition(); pos != 0; )
	{
		const UnknownFile_Struct* pFile = waitingforhash_list.GetNext(pos);
		if (!pFile->strName.CompareNoCase(rstrName) && !CompareDirectories(pFile->strDirectory, strDir))
		{
			if(pbWaiting)		
				*pbWaiting = true;
			return true;
		}
	}

	m_hashingLocker.Lock();
	for (POSITION pos = currentlyhashing_list.GetHeadPosition(); pos != 0; )
	{
		const UnknownFile_Struct* pFile = currentlyhashing_list.GetNext(pos);
		if (!pFile->strName.CompareNoCase(rstrName) && !CompareDirectories(pFile->strDirectory, strDir))
		{			
			m_hashingLocker.Unlock();
			return true;
		}
	}

	m_hashingLocker.Unlock();
	return false;
}

bool CSharedFileList::IsHashing(const CString& rstrDirectory)
{
	CString strDir = rstrDirectory;
	if (strDir.Right(1) != _T("\\") )
	{
		strDir += _T("\\");
	}
	if (rstrDirectory.IsEmpty())
	{
		return false;
	}

	for (POSITION pos = waitingforhash_list.GetHeadPosition(); pos != 0; )
	{
		const UnknownFile_Struct* pFile = waitingforhash_list.GetNext(pos);
		if (!CompareDirectories(pFile->strDirectory, strDir))
		{
			return true;
		}
	}

	m_hashingLocker.Lock();
	for (POSITION pos = currentlyhashing_list.GetHeadPosition(); pos != 0; )
	{
		const UnknownFile_Struct* pFile = currentlyhashing_list.GetNext(pos);
		if (!CompareDirectories(pFile->strDirectory, strDir))
		{			
			m_hashingLocker.Unlock();
			return true;
		}

		CString tempDir(rstrDirectory);
		tempDir.MakeLower();
		if (pFile->strDirectory.Find(tempDir) == 0)
		{
			m_hashingLocker.Unlock();
			return true;
		}
	}
	m_hashingLocker.Unlock();

	return false;
}

bool CSharedFileList::IsShared(const CString& rstrFilePath)
{
	POSITION pos = m_Files_map.GetStartPosition();
	while (pos)
	{
		CCKey key;
		CKnownFile* cur_file;
		m_Files_map.GetNextAssoc(pos, key, cur_file);
		if ( 0==rstrFilePath.CompareNoCase(cur_file->GetFilePath()) )
			return true;
	}

	return false;
}

void CSharedFileList::RemoveFromHashing(CKnownFile* hashed){
	for (POSITION pos = currentlyhashing_list.GetHeadPosition(); pos != 0; ){
		POSITION posLast = pos;
		const UnknownFile_Struct* pFile = currentlyhashing_list.GetNext(pos);
		if (!pFile->strName.CompareNoCase(hashed->GetFileName()) && !CompareDirectories(pFile->strDirectory, hashed->GetPath())){
			currentlyhashing_list.RemoveAt(posLast);
			delete pFile;
			HashNextFile();			// start next hash if possible, but only if a previous hash finished
			return;
		}
	}
}

void CSharedFileList::RemoveHashing(const CString& rstrDirectory, const CString& rstrName)
{
	CString strDir = rstrDirectory;
	if( strDir.Right(1)!=_T("\\")) 
		strDir +=_T("\\");  

	for (POSITION pos = waitingforhash_list.GetHeadPosition(); pos != 0; ){
		POSITION posLast = pos;
		const UnknownFile_Struct* pFile = waitingforhash_list.GetNext(pos);
		if (!pFile->strName.CompareNoCase(rstrName) && !CompareDirectories(pFile->strDirectory, strDir))
		{
			waitingforhash_list.RemoveAt(posLast);
			delete pFile;
			HashNextFile();			// start next hash if possible, but only if a previous hash finished
			return;
		}
	}

	m_hashingLocker.Lock();
	for (POSITION pos = currentlyhashing_list.GetHeadPosition(); pos != 0; )
	{
		POSITION posLast = pos;
		const UnknownFile_Struct* pFile = currentlyhashing_list.GetNext(pos);
		if (!pFile->strName.CompareNoCase(rstrName) && !CompareDirectories(pFile->strDirectory, strDir)){
			currentlyhashing_list.RemoveAt(posLast);
			delete pFile;
			HashNextFile();			// start next hash if possible, but only if a previous hash finished
			m_hashingLocker.Unlock();
			return;
		}
	}
	m_hashingLocker.Unlock();
}

void CSharedFileList::RemoveHashingInUnSharedDir( bool bRemoveAll )
{
	m_hashingLocker.Lock();
	for (POSITION pos = currentlyhashing_list.GetHeadPosition(); pos != 0; )
	{
		POSITION posLast = pos;
		const UnknownFile_Struct* pFile = currentlyhashing_list.GetNext(pos);
		if( !IsSharedPath(pFile->strDirectory ) )
		{
			CString strFilePath = pFile->strDirectory;
			if(strFilePath.Right(1)!=_T("\\")) 
				strFilePath+=_T("\\");
			strFilePath += pFile->strName;			
			if( !bRemoveAll && CGlobalVariable::filemgr.IsWaitforHash(strFilePath) )
				continue;
			currentlyhashing_list.RemoveAt(posLast);
			delete pFile;
		}
	}
	m_hashingLocker.Unlock();

	for (POSITION pos = waitingforhash_list.GetHeadPosition(); pos != 0; )
	{
		POSITION posLast = pos;
		const UnknownFile_Struct* pFile = waitingforhash_list.GetNext(pos);
		if( !IsSharedPath(pFile->strDirectory ) )
		{
			CString strFilePath = pFile->strDirectory;
			if(strFilePath.Right(1)!=_T("\\")) 
				strFilePath+=_T("\\");
			strFilePath += pFile->strName;			
			if( !bRemoveAll && CGlobalVariable::filemgr.IsWaitforHash(strFilePath) )
				continue;
			waitingforhash_list.RemoveAt(posLast);
			delete pFile;
		}
	}
}

void CSharedFileList::HashFailed(UnknownFile_Struct* hashed){
	for (POSITION pos = currentlyhashing_list.GetHeadPosition(); pos != 0; ){
		POSITION posLast = pos;
		const UnknownFile_Struct* pFile = currentlyhashing_list.GetNext(pos);
		if (!pFile->strName.CompareNoCase(hashed->strName) && !CompareDirectories(pFile->strDirectory, hashed->strDirectory)){
			currentlyhashing_list.RemoveAt(posLast);
			delete pFile;
			HashNextFile();			// start next hash if possible, but only if a previous hash finished
			break;
		}
	}
	delete hashed;
}
// SLUGFILLER: SafeHash

IMPLEMENT_DYNCREATE(CAddFileThread, CWinThread)

CAddFileThread::CAddFileThread()
{
	m_pOwner = NULL;
	m_partfile = NULL;
}

void CAddFileThread::SetValues(CSharedFileList* pOwner, LPCTSTR directory, LPCTSTR filename, CPartFile* partfile)
{
	 m_pOwner = pOwner;
	 m_strDirectory = directory;
	 m_strFilename = filename;
	 m_partfile = partfile;
}

BOOL CAddFileThread::InitInstance()
{
	InitThreadLocale();
	return TRUE;
}

int CAddFileThread::Run()
{
 	DbgSetThreadName("Hashing %s", m_strFilename);
	//  Comment UI
	if ( !(m_pOwner || m_partfile) || m_strFilename.IsEmpty() || !CGlobalVariable::IsRunning() )
		return 0;
	
	CoInitialize(NULL);

// 	if (m_partfile)
// 	{
// 		//CString strFileHash = (CString)m_partfile->GetFileHash();
// 	  //  if ( !strFileHash.IsEmpty())
// 	  //  {
// 		//CKnownFile* newrecord = new CKnownFile();
// 
// 	//	// 
// 	//	CString strFilePath;
// 	//	_tmakepathlimit(strFilePath.GetBuffer(MAX_PATH), NULL, m_strDirectory, m_strFilename, NULL);
// 	//	newrecord->SetFilePath( strFilePath );
// 	//	newrecord->SetFileSize( m_partfile->GetFileSize() );
// 	//	
//        if (m_partfile->GetPartCount() > 0)
//        {
// 		    m_partfile->SetHash(m_partfile);	
//        }
//  
// 		if(m_partfile && m_partfile->GetFileOp() == PFOP_HASHING )
// 			m_partfile->SetFileOp(PFOP_NONE);
// 
//  		CGlobalVariable::filemgr.HashCompleted( m_partfile );
//  	    VERIFY( PostMessage(CGlobalVariable::m_hListenWnd, WM_FILE_FINISHEDHASHING, (m_pOwner ? 0: (WPARAM)m_partfile), (LPARAM)m_partfile) );
// 
//  		CoUninitialize();
// 
// 		return 0;
// 	//	}
// 	}

	// locking that hashing thread is needed because we may create a couple of those threads at startup when rehashing
	// potentially corrupted downloading part files. if all those hash threads would run concurrently, the io-system would be
	// under very heavy load and slowly progressing
	//  Comment UI
	CSingleLock sLock1(&CGlobalVariable::hashing_mut); // only one filehash at a time
	sLock1.Lock();

	CString strFilePath;
	_tmakepathlimit(strFilePath.GetBuffer(MAX_PATH), NULL, m_strDirectory, m_strFilename, NULL);

	strFilePath.ReleaseBuffer();
	if (m_partfile)
		Log(GetResString(IDS_HASHINGFILE) + _T(" \"%s\" \"%s\""), m_partfile->GetFileName(), strFilePath);
	else
		Log(GetResString(IDS_HASHINGFILE) + _T(" \"%s\""), strFilePath);
	
	CKnownFile* newrecord = new CKnownFile();
	if (newrecord->CreateFromFile(m_strDirectory, m_strFilename, m_partfile) && CGlobalVariable::IsRunning()) // SLUGFILLER: SafeHash - in case of shutdown while still hashing
	{
		if (m_partfile && m_partfile->GetFileOp() == PFOP_HASHING)
			m_partfile->SetFileOp(PFOP_NONE);

		CGlobalVariable::filemgr.HashCompleted(newrecord);
		VERIFY( PostMessage(CGlobalVariable::m_hListenWnd, WM_FILE_FINISHEDHASHING, (m_pOwner ? 0: (WPARAM)m_partfile), (LPARAM)newrecord) );
	}
	else
	{
		if (CGlobalVariable::IsRunning())
		{
			if (m_partfile && m_partfile->GetFileOp() == PFOP_HASHING)
				m_partfile->SetFileOp(PFOP_NONE);
		

			CGlobalVariable::filemgr.HashFailed(strFilePath);
			// SLUGFILLER: SafeHash - inform main program of hash failure
			if (m_pOwner)
			{
				UnknownFile_Struct* hashed = new UnknownFile_Struct;
				hashed->strDirectory = m_strDirectory;
				hashed->strName = m_strFilename;
				VERIFY( PostMessage(CGlobalVariable::m_hListenWnd,WM_FILE_HASHFAILED,0,(LPARAM)hashed) );
			}
		}

		// SLUGFILLER: SafeHash
		delete newrecord;
	}

	sLock1.Unlock();
	CoUninitialize();

	return 0;
}
	
void CSharedFileList::UpdateFile(CKnownFile* toupdate)
{
	//  Comment UI
	UINotify(WM_SHAREDFILE_UPDATE, 0, (LPARAM)toupdate, toupdate);
}

void CSharedFileList::Process()
{
	Publish();
	if( !m_lastPublishED2KFlag || ( ::GetTickCount() - m_lastPublishED2K < ED2KREPUBLISHTIME ) )
	{
		return;
	}
	SendListToServer();
	m_lastPublishED2K = ::GetTickCount();
}

void CSharedFileList::Publish()
{
	// Variables to save cpu.
	UINT tNow = time(NULL);
	bool isFirewalled =true;
	//  Comment UI
	isFirewalled= CGlobalVariable::IsFirewalled();

	if( Kademlia::CKademlia::IsConnected() && ( !isFirewalled || ( isFirewalled && CGlobalVariable::clientlist->GetBuddyStatus() == Connected)) && GetCount() && Kademlia::CKademlia::GetPublish())
	{ 
		//We are connected to Kad. We are either open or have a buddy. And Kad is ready to start publishing.
		if( Kademlia::CKademlia::GetTotalStoreKey() < KADEMLIATOTALSTOREKEY)
		{
			//We are not at the max simultaneous keyword publishes 
			if (tNow >= m_keywords->GetNextPublishTime())
			{
				//Enough time has passed since last keyword publish

				//Get the next keyword which has to be (re)-published
				CPublishKeyword* pPubKw = m_keywords->GetNextKeyword();
				if(pPubKw)
				{
					//We have the next keyword to check if it can be published

					//Debug check to make sure things are going well.
					ASSERT( pPubKw->GetRefCount() != 0 );

					if (tNow >= pPubKw->GetNextPublishTime())
					{
						//This keyword can be published.
						Kademlia::CSearch* pSearch = Kademlia::CSearchManager::PrepareLookup(Kademlia::CSearch::STOREKEYWORD, false, pPubKw->GetKadID());
						if (pSearch)
						{
							//pSearch was created. Which means no search was already being done with this HashID.
							//This also means that it was checked to see if network load wasn't a factor.

							//This sets the filename into the search object so we can show it in the gui.
							pSearch->SetFileName(pPubKw->GetKeyword());

							//Add all file IDs which relate to the current keyword to be published
							const CSimpleKnownFileArray& aFiles = pPubKw->GetReferences();
							uint32 count = 0;
							for (int f = 0; f < aFiles.GetSize(); f++)
							{
								//Debug check to make sure things are working well.
								ASSERT_VALID( aFiles[f] );
								// JOHNTODO - Why is this happening.. I think it may have to do with downloading a file that is already
								// in the known file list..
//								ASSERT( IsFilePtrInList(aFiles[f]) );

								//Only publish complete files as someone else should have the full file to publish these keywords.
								//As a side effect, this may help reduce people finding incomplete files in the network.
								if( !aFiles[f]->IsPartFile() && IsFilePtrInList(aFiles[f]))
								{
									count++;
									pSearch->AddFileID(Kademlia::CUInt128(aFiles[f]->GetFileHash()));
									if( count > 150 )
									{
										//We only publish up to 150 files per keyword publish then rotate the list.
										pPubKw->RotateReferences(f);
										break;
									}
								}
							}

							if( count )
							{
								//Start our keyword publish
								pPubKw->SetNextPublishTime(tNow+(KADEMLIAREPUBLISHTIMEK));
								pPubKw->IncPublishedCount();
								Kademlia::CSearchManager::StartSearch(pSearch);
							}
							else
							{
								//There were no valid files to publish with this keyword.
								delete pSearch;
							}
						}
					}
				}
				m_keywords->SetNextPublishTime(KADEMLIAPUBLISHTIME+tNow);
			}
		}
		
		if( Kademlia::CKademlia::GetTotalStoreSrc() < KADEMLIATOTALSTORESRC)
		{
			if(tNow >= m_lastPublishKadSrc)
			{
				if(m_currFileSrc > GetCount())
					m_currFileSrc = 0;
				CKnownFile* pCurKnownFile = GetFileByIndex(m_currFileSrc);
				if(pCurKnownFile)
				{
					if(pCurKnownFile->PublishSrc())
					{
						if(Kademlia::CSearchManager::PrepareLookup(Kademlia::CSearch::STOREFILE, true, Kademlia::CUInt128(pCurKnownFile->GetFileHash()))==NULL)
							pCurKnownFile->SetLastPublishTimeKadSrc(0,0);
					}	
				}
				m_currFileSrc++;

				// even if we did not publish a source, reset the timer so that this list is processed
				// only every KADEMLIAPUBLISHTIME seconds.
				m_lastPublishKadSrc = KADEMLIAPUBLISHTIME+tNow;
			}
		}

		if( Kademlia::CKademlia::GetTotalStoreNotes() < KADEMLIATOTALSTORENOTES)
		{
			if(tNow >= m_lastPublishKadNotes)
			{
				if(m_currFileNotes > GetCount())
					m_currFileNotes = 0;
				CKnownFile* pCurKnownFile = GetFileByIndex(m_currFileNotes);
				if(pCurKnownFile)
				{
					if(pCurKnownFile->PublishNotes())
					{
						if(Kademlia::CSearchManager::PrepareLookup(Kademlia::CSearch::STORENOTES, true, Kademlia::CUInt128(pCurKnownFile->GetFileHash()))==NULL)
							pCurKnownFile->SetLastPublishTimeKadNotes(0);
					}	
				}
				m_currFileNotes++;

				// even if we did not publish a source, reset the timer so that this list is processed
				// only every KADEMLIAPUBLISHTIME seconds.
				m_lastPublishKadNotes = KADEMLIAPUBLISHTIME+tNow;
			}
		}
	}
}

void CSharedFileList::AddKeywords(CKnownFile* pFile)
{
	m_keywords->AddKeywords(pFile);
}

void CSharedFileList::RemoveKeywords(CKnownFile* pFile)
{
	m_keywords->RemoveKeywords(pFile);
}

void CSharedFileList::DeletePartFileInstances() const
{
	// this is only allowed during shut down
	ASSERT( CGlobalVariable::m_app_state == APP_STATE_SHUTTINGDOWN );
	ASSERT( CGlobalVariable::knownfiles );

	POSITION pos = m_Files_map.GetStartPosition();
	while (pos)
	{
		CCKey key;
		CKnownFile* cur_file;
		m_Files_map.GetNextAssoc(pos, key, cur_file);
		if (cur_file->IsKindOf(RUNTIME_CLASS(CPartFile)))
		{
			if (!CGlobalVariable::downloadqueue->IsPartFile(cur_file) && !CGlobalVariable::knownfiles->IsFilePtrInList(cur_file))
				delete cur_file; // this is only allowed during shut down
		}
	}
}

bool CSharedFileList::IsUnsharedFile(const uchar* auFileHash) const {
	bool bFound;
	if (auFileHash){
		CSKey key(auFileHash);
		if (m_UnsharedFiles_map.Lookup(key, bFound))
			return true;
	}
	return false;
}

CKnownFile * CSharedFileList::GetKnownFile(const CCKey & key)
{
	CKnownFile* pFileInMap=NULL;
	m_Files_map.Lookup(key, pFileInMap);
	return pFileInMap;
}

int CSharedFileList::GetHashingCount()
{
	int waitingforhash = waitingforhash_list.GetCount();
	int currentlyhashing = currentlyhashing_list.GetCount();
	return waitingforhash+currentlyhashing;
}

bool CSharedFileList::HashNewFile(const CString & strFilePath,bool bAddtoWaiting/*=true*/)
{	
	int nPathEnd=strFilePath.ReverseFind('\\');
	CString strDirectory = strFilePath.Mid(0, nPathEnd+1);
	CString strName =  strFilePath.Mid(nPathEnd+1);
	
	CKnownFile* pKnownFileExist = CGlobalVariable::knownfiles->FindKnownFileByPath(strFilePath,true);
	if( NULL!=pKnownFileExist )
	{
		CGlobalVariable::filemgr.HashCompleted(pKnownFileExist);
		PostMessage( CGlobalVariable::m_hListenWnd, WM_FILE_FINISHEDHASHING, 0, (LPARAM)pKnownFileExist );
	}
	else if (!IsHashing(strDirectory, strName) && !thePrefs.IsTempFile(strDirectory, strFilePath))
	{
		UnknownFile_Struct* tohash = new UnknownFile_Struct;
		tohash->strDirectory = strDirectory;
		tohash->strName = strFilePath.Mid(nPathEnd+1);

		if( bAddtoWaiting )
		{
			waitingforhash_list.AddTail(tohash);
			HashNextFile();
			return true;
		}
		else
		{
			m_hashingLocker.Lock();
			currentlyhashing_list.AddTail(tohash);
			m_hashingLocker.Unlock();
			return true;
		}		
	}
	return false;
}

void CSharedFileList::Initialize()
{
	FindSharedFiles();
}

int CSharedFileList::GetAllSharedFile(CList<CKnownFile *, CKnownFile*> & filelist)
{
	filelist.RemoveAll();

	CCKey bufKey;
	CKnownFile* cur_file;
	for (POSITION pos = m_Files_map.GetStartPosition(); pos != 0; )
	{
		m_Files_map.GetNextAssoc(pos, bufKey, cur_file);
		filelist.AddTail(cur_file);
	}
	return filelist.GetCount();
}

bool CSharedFileList::IsSharedPath(CString strPath)
{
	if(strPath.Right(1)!=_T("\\")) strPath+=_T("\\");
	CString tempDir=thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR);
	if (tempDir.Right(1)!=_T("\\"))
		tempDir+=_T("\\");
	if(tempDir.CompareNoCase(strPath)==0) 
		return true;

	for (POSITION pos = thePrefs.shareddir_list.GetHeadPosition();pos != 0;)
	{
		tempDir = thePrefs.shareddir_list.GetNext(pos);
		if (tempDir.Right(1)!=_T("\\"))
			tempDir+=_T("\\");

		if(tempDir.CompareNoCase(strPath)==0) 
			return true;
	}

	return false;
}
