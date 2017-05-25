/* 
 * $Id: KnownFileList.h 9073 2008-12-18 04:38:51Z dgkang $
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

class CKnownFile;
typedef CMap<CCKey,const CCKey&,CKnownFile*,CKnownFile*> CKnownFilesMap;
typedef CMap<CSKey,const CSKey&,int,int> CancelledFilesMap;

class CKnownFileList 
{
	friend class CSharedFilesWnd;
	friend class CStatisticFile;
public:
	CKnownFileList();
	~CKnownFileList();

	bool	SafeAddKFile(CKnownFile* toadd,bool bLoadMetFile = false,CKnownFile** ppFileInMap=NULL);
	bool	Init();
	void	Save();
	void	Clear();
	void	Process();

	CKnownFile* FindKnownFile(LPCTSTR filename, uint32 date, uint64 size) const;
	CKnownFile* FindKnownFileByID(const uchar* hash) const;
	CKnownFile* FindKnownFileByPath(const CString& sFilePath,bool bCheckFileDateandSize=false) const;	
	bool	IsKnownFile(const CKnownFile* file) const;
	bool	IsFilePtrInList(const CKnownFile* file) const;

	void	AddCancelledFileID(const uchar* hash);
	bool	IsCancelledFileByID(const uchar* hash) const;

	const CKnownFilesMap& GetKnownFiles() const { return m_Files_map; }
	void	CopyKnownFileMap(CMap<CCKey,const CCKey&,CKnownFile*,CKnownFile*> &Files_Map);

private:
	bool	LoadKnownFiles();
	bool	LoadCancelledFiles();

	uint16 	requested;
	uint16 	accepted;
	uint64 	transferred;
	uint32 	m_nLastSaved;
	CKnownFilesMap		m_Files_map;
	CancelledFilesMap	m_mapCancelledFiles;
};
