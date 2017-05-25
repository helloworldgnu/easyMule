/*
 * $Id: Database.cpp 4711 2008-01-26 08:37:28Z soarchin $
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

#include "Database.h"
#include "sqlite3.h"

CDatabase::CDatabase(LPCSTR filename)
{
	m_stmt = NULL;
	sqlite3 *db;
	INT rc;

	rc = sqlite3_open(filename, &db);
	if( rc )
	{
		m_db = NULL;
		sqlite3_close(db);
		throw;
	}
	m_db = db;
	Exec("PRAGMA page_size = 1024");
	Exec("PRAGMA default_cache_size = 256");
	m_bIsCopy = FALSE;
	return;
}

CDatabase::CDatabase( CDatabase * _db )
{
	m_db = _db->m_db;
	m_stmt = NULL;
	m_bIsCopy = TRUE;
}

CDatabase::~CDatabase()
{
	Finalize();
	if(!m_bIsCopy && m_db != NULL)
	{
		Exec("VACUUM");
		sqlite3 *db = (sqlite3 *)m_db;
		sqlite3_close(db);
		m_db = NULL;
	}
}

BOOL CDatabase::InitSucc()
{
	return (m_db != NULL);
}

BOOL CDatabase::Exec(CONST CHAR * cmd)
{
	if(m_db == NULL)
		return FALSE;
	CHAR *zErrMsg = 0;
	sqlite3 *db = (sqlite3 *)m_db;
	INT rc;
	rc = sqlite3_exec(db, cmd, 0, 0, &zErrMsg);
	if( rc != SQLITE_OK )
	{
		sqlite3_free(zErrMsg);
		return FALSE;
	}
	return TRUE;
}

BOOL CDatabase::Prepare(CONST CHAR * cmd)
{
	if(m_db == NULL)
		return FALSE;
	Finalize();
	sqlite3 *db = (sqlite3 *)m_db;
	sqlite3_stmt *stmt;
	INT rc;
	rc = sqlite3_prepare(db, cmd, -1, &stmt, NULL); 
	if (rc == SQLITE_OK)
	{
		m_stmt = stmt;
		return TRUE;
	}
	return FALSE;
}

BOOL CDatabase::Step()
{
	if(m_stmt == NULL)
		return FALSE;
	sqlite3_stmt *stmt = (sqlite3_stmt *)m_stmt;
	INT rc;
	rc = sqlite3_step(stmt);
	if(rc == SQLITE_ROW)
	{
		return TRUE;
	}
	Finalize();
	return FALSE;
}

VOID CDatabase::Finalize()
{
	if(m_stmt != NULL)
	{
		sqlite3_stmt *stmt = (sqlite3_stmt *)m_stmt;
		sqlite3_finalize(stmt);
		m_stmt = NULL;
	}
}

INT CDatabase::GetCols()
{
	if(m_stmt == NULL)
		return 0;
	sqlite3_stmt *stmt = (sqlite3_stmt *)m_stmt;
	return sqlite3_column_count(stmt);
}

INT CDatabase::GetColInt(INT col)
{
	if(m_stmt == NULL)
		return 0;
	sqlite3_stmt *stmt = (sqlite3_stmt *)m_stmt;
	return sqlite3_column_int(stmt, col);
}

DOUBLE CDatabase::GetColFloat(INT col)
{
	if(m_stmt == NULL)
		return 0.0;
	sqlite3_stmt *stmt = (sqlite3_stmt *)m_stmt;
	return sqlite3_column_double(stmt, col);
}

CONST BYTE * CDatabase::GetColText(INT col)
{
	if(m_stmt == NULL)
		return NULL;
	sqlite3_stmt *stmt = (sqlite3_stmt *)m_stmt;
	return sqlite3_column_text(stmt, col);
}

CONST VOID * CDatabase::GetColBlob(INT col)
{
	if(m_stmt == NULL)
		return NULL;
	sqlite3_stmt *stmt = (sqlite3_stmt *)m_stmt;
	return sqlite3_column_blob(stmt, col);
}

INT64 CDatabase::GetColInt64(INT col)
{
	if(m_stmt == NULL)
		return 0;
	sqlite3_stmt *stmt = (sqlite3_stmt *)m_stmt;
	return sqlite3_column_int64(stmt, col);
}