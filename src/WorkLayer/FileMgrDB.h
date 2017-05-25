/*
 * $Id: FileMgrDB.h 9297 2008-12-24 09:55:04Z dgkang $
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

#include "Database.h"

class CFileMgr;
class CFileTaskItem;
class CUrlSite;
class FILEKEY;

class CFileMgrDB
{
public:
	CFileMgrDB(CString strFilename);
	~CFileMgrDB();
	void UpdateFile (CFileTaskItem * item, CONST BYTE * lpHash = NULL, BOOL bNew = TRUE, BOOL bUpdateUrl = TRUE);

	void UpdateFiles(CRBMap<FILEKEY,CFileTaskItem*> & FileList);
	void UpdateUrl(CFileTaskItem * item, CUrlSite * lpSite, CONST BYTE * lpHash = NULL);
	void UpdateHashing(CString strPath, INT state,BOOL bNew=false);
	void RemoveFile(CFileTaskItem * item, CONST BYTE * lpHash = NULL);
	void RemoveHashing(CString strPath);
	void RemoveAll();
	bool LoadAll(CFileMgr * mgr);
protected:
	CDatabase * m_db;
};
