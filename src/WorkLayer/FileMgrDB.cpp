/*
 * $Id: FileMgrDB.cpp 9297 2008-12-24 09:55:04Z dgkang $
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

#include "StringConversion.h"
#include "otherfunctions.h"
#include "UrlSite.h"
#include "FileMgr.h"
#include "FileMgr.h"
#include "FileMgrDB.h"

#ifdef _DEBUG
#define SQL_EXEC(cmd) TRACE("Executing SQL: %s\n",cmd);ASSERT(m_db->Exec(cmd));
#else
#define SQL_EXEC(cmd) m_db->Exec(cmd);
#endif

CFileMgrDB::CFileMgrDB(CString strFilename)
{
	try
	{
		m_db = new CDatabase(StrToUtf8(strFilename));
	}
	catch(...)
	{
		m_db = NULL;
		throw;
	}
	if(!m_db->InitSucc())
	{
		delete m_db;
		m_db = NULL;
		throw;
	}
	m_db->Exec("CREATE TABLE IF NOT EXISTS files (id INTEGER PRIMARY KEY AUTOINCREMENT, path TEXT UNIQUE ON CONFLICT REPLACE)");
	m_db->Exec("ALTER TABLE files ADD hash TEXT");
	m_db->Exec("ALTER TABLE files ADD state INTEGER");
	m_db->Exec("ALTER TABLE files ADD url TEXT");
	m_db->Exec("ALTER TABLE files ADD size INTEGER");
	m_db->Exec("ALTER TABLE files ADD name TEXT");
	m_db->Exec("ALTER TABLE files ADD ed2klink TEXT");
	m_db->Exec("ALTER TABLE files ADD filetime INTEGER");
	m_db->Exec("ALTER TABLE files ADD metbakid INTEGER");
	m_db->Exec("CREATE TABLE IF NOT EXISTS urls (id INTEGER PRIMARY KEY AUTOINCREMENT)");
	m_db->Exec("ALTER TABLE urls ADD fid INTEGER");
	m_db->Exec("ALTER TABLE urls ADD url TEXT");
	m_db->Exec("ALTER TABLE urls ADD fromwhere INTEGER");
	m_db->Exec("ALTER TABLE urls ADD transnopay INTEGER");
	m_db->Exec("ALTER TABLE urls ADD transpay INTEGER");
	m_db->Exec("ALTER TABLE urls ADD badsite INTEGER");
	m_db->Exec("ALTER TABLE urls ADD pref INTEGER");
	m_db->Exec("ALTER TABLE urls ADD needci INTEGER");
	m_db->Exec("CREATE TABLE IF NOT EXISTS hashing (id INTEGER PRIMARY KEY AUTOINCREMENT, path TEXT UNIQUE ON CONFLICT REPLACE)");
	m_db->Exec("ALTER TABLE hashing ADD state INTEGER");
}

CFileMgrDB::~CFileMgrDB()
{
	if(m_db != NULL)
		delete m_db;
}

void CFileMgrDB::UpdateFiles(CRBMap<FILEKEY,CFileTaskItem*> & FileList)
{
	CStringA strCmd, strCmd2;
	CHAR strHash[33];

	m_db->Exec("begin transaction");
	POSITION pos = FileList.GetHeadPosition();
	while (pos)
	{		
		FILEKEY fk = FileList.GetKeyAt(pos);
		CFileTaskItem *item = FileList.GetNextValue(pos);
		md4strA(fk.key, strHash);

		CStringA strPath = StrToUtf8(item->m_strFilePath), strUrl = StrToUtf8(item->m_strUrl), strName = StrToUtf8(item->m_FileName), strLink = StrToUtf8(item->m_strEd2kLink);
		strPath.Replace("'", "''");
		strUrl.Replace("'", "''");
		strName.Replace("'", "''");
		strLink.Replace("'", "''");
	
		strCmd2.Format("INSERT INTO files (hash, state, path, url, size, name, ed2klink, filetime, metbakid) "
			"VALUES ('%s',%u,'%s','%s',%I64u,'%s','%s',%I64u,%u)", strHash, item->m_nFileState, strPath, strUrl, item->m_FileSize, strName,
			strLink, item->m_tFiletime.GetTime(), item->m_metBakId);
		SQL_EXEC(strCmd2);
	}   
	m_db->Exec("end transaction");	
}


void CFileMgrDB::UpdateFile(CFileTaskItem * item, CONST BYTE * lpHash, BOOL bNew, BOOL bUpdateUrl)
{
	CStringA strCmd, strCmd2;
	CHAR strHash[33];
	UINT id = 0;
	if(lpHash != NULL && !isnulmd4(lpHash))
	{
		md4strA(lpHash, strHash);
		strCmd.Format("SELECT id FROM files WHERE hash='%s'", strHash);
		if(m_db->Prepare(strCmd))
		{
			if(m_db->Step())
			{
				bNew = FALSE; 
				id = m_db->GetColInt(0);
			}
			m_db->Finalize();
		}
	}
	else if(item->m_strUrl != "")
	{
		strHash[0] = 0;
		strCmd.Format("SELECT id FROM files WHERE url='%s'", StrToUtf8(item->m_strUrl));
		if(m_db->Prepare(strCmd))
		{
			if(m_db->Step())
			{
				bNew = FALSE;
				id = m_db->GetColInt(0);
			}
			m_db->Finalize();
		}
	}
	else
		return;
	CStringA strPath = StrToUtf8(item->m_strFilePath), strUrl = StrToUtf8(item->m_strUrl), strName = StrToUtf8(item->m_FileName), strLink = StrToUtf8(item->m_strEd2kLink);
	strPath.Replace("'", "''");
	strUrl.Replace("'", "''");
	strName.Replace("'", "''");
	strLink.Replace("'", "''");
	if(bNew)
		strCmd2.Format("INSERT INTO files (hash, state, path, url, size, name, ed2klink, filetime, metbakid) "
		"VALUES ('%s',%u,'%s','%s',%I64u,'%s','%s',%I64u,%u)", strHash, item->m_nFileState, strPath, strUrl, item->m_FileSize, strName,
		strLink, item->m_tFiletime.GetTime(), item->m_metBakId);
	else
		strCmd2.Format("UPDATE files SET hash='%s', state=%u, path='%s', url='%s', size=%I64u, name='%s', ed2klink='%s', "
		"filetime=%I64u, metbakid=%u WHERE id=%u", strHash, item->m_nFileState, strPath, strUrl, item->m_FileSize,strName, 
		strLink, item->m_tFiletime.GetTime(), item->m_metBakId, id);
	SQL_EXEC(strCmd2);
	if(bNew)
	{
		if(!m_db->Prepare(strCmd))
			return;
		if(!m_db->Step())
		{
			m_db->Finalize();
			return;
		}
		id = m_db->GetColInt(0);
	}

	if(!bUpdateUrl)
		return;

	strCmd.Format("DELETE FROM urls WHERE fid=%d", id);
	SQL_EXEC(strCmd);
	POSITION pos = item->m_lMetaLinkURLList.GetHeadPosition();
	while (pos)
	{
		CUrlSite pSite = item->m_lMetaLinkURLList.GetNext(pos);
		strUrl = StrToUtf8(pSite.m_strUrl);
		strUrl.Replace("'", "''");
		strCmd.Format("INSERT INTO urls (fid, url, fromwhere, transnopay, transpay, badsite, pref, needci) VALUES (%u,'%s',%u,%I64u,%I64u,%d,%d,%u)", id, strUrl, pSite.m_dwFromWhere, pSite.m_dwDataTransferedWithoutPayload, pSite.m_dwDataTransferedWithPayload, pSite.m_bBadSite ? 1 : 0, pSite.m_dwInitPreference, pSite.m_bNeedCommitted ? 1 : 0);
		SQL_EXEC(strCmd);
	}
}

void CFileMgrDB::UpdateUrl(CFileTaskItem * item, CUrlSite * lpSite, CONST BYTE * lpHash)
{
	CStringA strCmd, strUrl;
	if(lpHash != NULL && !isnulmd4(lpHash))
	{
		CHAR strHash[33];
		md4strA(lpHash, strHash);
		strCmd.Format("SELECT id FROM files WHERE hash='%s'", strHash);
	}
	else if(item->m_strUrl != "")
	{
		strUrl = StrToUtf8(item->m_strUrl);
		strUrl.Replace("'", "''");
		strCmd.Format("SELECT id FROM files WHERE url='%s'", StrToUtf8(item->m_strUrl));
	}
	else
		return;

	if(!m_db->Prepare(strCmd))
		return;
	if(!m_db->Step())
	{
		m_db->Finalize();
		return;
	}
	UINT id = m_db->GetColInt(0);

	strUrl = StrToUtf8(lpSite->m_strUrl);
	strUrl.Replace("'", "''");
	strCmd.Format("SELECT id FROM urls WHERE fid=%u AND url='%s'", id, strUrl);
	BOOL bReplace = FALSE;
	UINT uid = 0;
	if(m_db->Prepare(strCmd))
	{
		if(m_db->Step())
		{
			bReplace = TRUE;
			uid = m_db->GetColInt(0);
		}
		m_db->Finalize();
	}
	if(bReplace)
		strCmd.Format("UPDATE urls SET fid=%u, url='%s', fromwhere=%u, transnopay=%I64u, transpay=%I64u, badsite=%d, pref=%d, needci=%u WHERE id=%u", id, strUrl, lpSite->m_dwFromWhere, lpSite->m_dwDataTransferedWithoutPayload, lpSite->m_dwDataTransferedWithPayload, lpSite->m_bBadSite ? 1 : 0, lpSite->m_dwInitPreference, lpSite->m_bNeedCommitted ? 1 : 0, uid);
	else
		strCmd.Format("INSERT INTO urls (fid, url, fromwhere, transnopay, transpay, badsite, pref, needci) VALUES (%u,'%s',%u,%I64u,%I64u,%d,%d,%u)", id, strUrl, lpSite->m_dwFromWhere, lpSite->m_dwDataTransferedWithoutPayload, lpSite->m_dwDataTransferedWithPayload, lpSite->m_bBadSite ? 1 : 0, lpSite->m_dwInitPreference, lpSite->m_bNeedCommitted ? 1 : 0);
	SQL_EXEC(strCmd);
}

void CFileMgrDB::UpdateHashing(CString strPath, INT state,BOOL bNew)
{
	CStringA strCmd;
	
	strPath.Replace( _T("'"), _T("''") );
	
	if(bNew)
		strCmd.Format("INSERT INTO hashing (path, state) VALUES ('%s', %u)", StrToUtf8(strPath), state);
	else
		strCmd.Format("REPLACE INTO hashing (path, state) VALUES ('%s', %u)", StrToUtf8(strPath), state);

	SQL_EXEC(strCmd);
}

void CFileMgrDB::RemoveFile(CFileTaskItem * item, CONST BYTE * lpHash)
{
	CStringA strCmd, strCmd2;
	if(lpHash != NULL && !isnulmd4(lpHash))
	{
		CHAR strHash[33];
		md4strA(lpHash, strHash);
		strCmd.Format("SELECT id FROM files WHERE hash='%s'", strHash);
		strCmd2.Format("DELETE FROM files WHERE hash='%s'", strHash);
	}
	else
	{
		CStringA strUrl = StrToUtf8(item->m_strUrl);
		strUrl.Replace("'", "''");
		strCmd.Format("SELECT id FROM files WHERE url='%s'", strUrl);
		strCmd2.Format("DELETE FROM files WHERE url='%s'", strUrl);
	}

	if(!m_db->Prepare(strCmd))
		return;
	if(!m_db->Step())
	{
		m_db->Finalize();
		return;
	}
	UINT id = m_db->GetColInt(0);

	strCmd.Format("DELETE FROM urls WHERE fid=%d", id);
	SQL_EXEC(strCmd);
	SQL_EXEC(strCmd2);
}

void CFileMgrDB::RemoveHashing(CString strPath)
{
	CStringA strCmd, strFPath = StrToUtf8(strPath);
	strFPath.Replace("'", "''");
	strCmd.Format("DELETE FROM hashing WHERE path='%s'", strFPath);
	SQL_EXEC(strCmd);
}

void CFileMgrDB::RemoveAll()
{
	SQL_EXEC("DELETE FROM files");
	SQL_EXEC("DELETE FROM urls");
	SQL_EXEC("DELETE FROM hashing");
}

bool CFileMgrDB::LoadAll(CFileMgr * mgr)
{
	if(!m_db->Prepare("SELECT id, hash, state, path, url, size, name, ed2klink, filetime, metbakid FROM files"))
		return false;
	while(m_db->Step())
	{
		CFileTaskItem * pNewItem = new CFileTaskItem;
		FILEKEY key;
		INT id = m_db->GetColInt(0);
		LPCSTR strHash = (LPCSTR)m_db->GetColText(1);
		if(strHash[0] != 0)
			strmd4(strHash, key.key);
		pNewItem->m_nFileState = m_db->GetColInt(2);
		pNewItem->m_strFilePath = OptUtf8ToStr((LPCSTR)m_db->GetColText(3));
		pNewItem->m_strUrl = OptUtf8ToStr((LPCSTR)m_db->GetColText(4));
		pNewItem->m_FileSize = m_db->GetColInt64(5);
		pNewItem->m_FileName = OptUtf8ToStr((LPCSTR)m_db->GetColText(6));
		pNewItem->m_strEd2kLink = OptUtf8ToStr((LPCSTR)m_db->GetColText(7));
		pNewItem->m_tFiletime = m_db->GetColInt64(8);
		pNewItem->m_metBakId = m_db->GetColInt(9);
		CDatabase uqdb(m_db);
		CStringA strCmd;
		strCmd.Format("SELECT url, fromwhere, transnopay, transpay, badsite, pref, needci FROM urls WHERE fid=%d", id);
		if(uqdb.Prepare(strCmd))
		{
			while(uqdb.Step())
			{
				CUrlSite urlSite;
				urlSite.m_strUrl = OptUtf8ToStr((LPCSTR)uqdb.GetColText(0));
				urlSite.m_dwFromWhere = uqdb.GetColInt(1);
				urlSite.m_dwDataTransferedWithoutPayload = uqdb.GetColInt64(2);
				urlSite.m_dwDataTransferedWithPayload = uqdb.GetColInt64(3);
				urlSite.m_bBadSite = uqdb.GetColInt(4) > 0;
				urlSite.m_dwInitPreference = uqdb.GetColInt(5);
				urlSite.m_bNeedCommitted = uqdb.GetColInt(6) > 0;
				pNewItem->m_lMetaLinkURLList.AddTail(urlSite);
			}
			uqdb.Finalize();
		}
		if(strHash[0] != 0)
			mgr->m_FileList.SetAt(key, pNewItem);
		else
			mgr->m_UrlList.SetAt(pNewItem->m_strUrl, pNewItem);
	}
	m_db->Finalize();

	if(!m_db->Prepare("SELECT path, state FROM hashing"))
		return false;
	while(m_db->Step())
	{
		mgr->m_WaitforHashList.SetAt(OptUtf8ToStr((LPCSTR)m_db->GetColText(0)), m_db->GetColInt(1));
	}
	m_db->Finalize();

	return true;
}
