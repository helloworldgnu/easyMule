/* 
 * $Id: SharedFileList.h 9073 2008-12-18 04:38:51Z dgkang $
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

#pragma once
#include "MapKey.h"

class CKnownFileList;
class CServerConnect;
class CPartFile;
class CKnownFile;
class CPublishKeywordList;
class CSafeMemFile;
class CServer;
class CCollection;

struct UnknownFile_Struct{
	CString strName;
	CString strDirectory;
};

class CSharedFileList
{
	//friend class CSharedFilesCtrl;
	friend class CClientReqSocket;

public:
	CSharedFileList(CServerConnect* in_server);
	~CSharedFileList();

	void	SendListToServer();
	void	Reload();
	bool	SafeAddKFile(CKnownFile* toadd, bool bOnlyAdd = false);
	void	RepublishFile(CKnownFile* pFile);
	int		GetAllSharedFile(CList<CKnownFile *, CKnownFile*> & filelist);
	bool	RemoveFile(CKnownFile* toremove,bool bOnlyUnShare=false);
	CKnownFile* GetFileByID(const uchar* filehash) const;
	CKnownFile*	GetFileByIndex(int index);
	bool	IsFilePtrInList(const CKnownFile* file) const;
	bool	IsSharedPath(CString strPath);
	void	PublishNextTurn()	{ m_lastPublishED2KFlag=true;	}
	void	CreateOfferedFilePacket(CKnownFile* cur_file, CSafeMemFile* files, CServer* pServer, CUpDownClient* pClient = NULL);
	uint64	GetDatasize(uint64 &pbytesLargest) const;
	int		GetCount()	{return m_Files_map.GetCount(); }
	int		GetHashingCount();	// SLUGFILLER SafeHash
	void	UpdateFile(CKnownFile* toupdate);
	bool	FileIsIgnored( const CFileFind& ff );
	void	AddFilesFromDirectory(const CString& rstrDirectory,bool bAddOnlyInKnownFile=false,bool bOnlyGetKnownPath=false);
	void	AddFileFromNewlyCreatedCollection(const CString& path, const CString& fileName);
	void	HashFailed(UnknownFile_Struct* hashed);		// SLUGFILLER: SafeHash
	bool	FileHashingFinished(CKnownFile* file,CKnownFile** ppFileInKnownList=NULL);
	void	ClearED2KPublishInfo();
	void	ClearKadSourcePublishInfo();
	void	Process();
	void	Publish();
	void	AddKeywords(CKnownFile* pFile);
	void	RemoveKeywords(CKnownFile* pFile);
	void	DeletePartFileInstances() const;
	bool	IsUnsharedFile(const uchar* auFileHash) const;
	void	CopySharedFileMap(CMap<CCKey,const CCKey&,CKnownFile*,CKnownFile*> &Files_Map);
	CKnownFile * GetKnownFile(const CCKey & key);
	bool	HashNewFile(const CString & strFilePath,bool bAddtoWaiting=true);
	bool	AddFile(CKnownFile* pFile);
	void	Initialize();

	bool IsFileAlreadyExist(CKnownFile* pFile);

	bool	IsShared(const CString& rstrFilePath);
	bool	IsHashing(const CString& rstrDirectory, const CString& rstrName,bool* pbWaiting=NULL);
	bool    IsHashing(const CString& rstrDirectory);
	void	RemoveHashing(const CString& rstrDirectory, const CString& rstrName);
	void    RemoveHashingInUnSharedDir( bool bRemoveAll=false );

	CMutex	m_mutWriteList;
private:
	void	FindSharedFiles();
	void	HashNextFile();
	// SLUGFILLER: SafeHash
	void	RemoveFromHashing(CKnownFile* hashed);
	// SLUGFILLER: SafeHash

	CMap<CCKey,const CCKey&,CKnownFile*,CKnownFile*> m_Files_map;
	CMap<CSKey,const CSKey&, bool, bool>			 m_UnsharedFiles_map;
	CPublishKeywordList* m_keywords;
	CTypedPtrList<CPtrList, UnknownFile_Struct*> waitingforhash_list;
	CTypedPtrList<CPtrList, UnknownFile_Struct*> currentlyhashing_list;	// SLUGFILLER: SafeHash
	CCriticalSection m_hashingLocker;
	CServerConnect*		server;
	//CSharedFilesCtrl*	output;
	void	AddShareFilesFromDir(CString strDir, bool bSubDirectories,CStringList& strListAdded,bool bAddOnlyInKnownFile=false,bool bOnlyAddFileInSubDir=false,int iLevel=2);

	uint32 m_lastPublishED2K;
	bool	 m_lastPublishED2KFlag;
	int m_currFileSrc;
	int m_currFileNotes;
	int m_currFileKey;
	uint32 m_lastPublishKadSrc;
	uint32 m_lastPublishKadNotes;
};

class CAddFileThread : public CWinThread
{
	DECLARE_DYNCREATE(CAddFileThread)
protected:
	CAddFileThread();
public:
	virtual BOOL InitInstance();
	virtual int	Run();
	void	SetValues(CSharedFileList* pOwner, LPCTSTR directory, LPCTSTR filename, CPartFile* partfile = NULL);

private:
	CSharedFileList* m_pOwner;
	CString			 m_strDirectory;
	CString			 m_strFilename;
	CPartFile*		 m_partfile;
};
