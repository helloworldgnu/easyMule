/*
 * $Id: Database.h 4701 2008-01-25 10:34:25Z soarchin $
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

class CDatabase
{
public:
	CDatabase(LPCSTR filename);
	CDatabase(CDatabase * _db);
	~CDatabase();
	BOOL InitSucc();
	BOOL Exec(CONST CHAR * cmd);
	BOOL Prepare(CONST CHAR * cmd);
	BOOL Step();
	VOID Finalize();
	INT GetCols();
	INT GetColInt(INT col);
	INT64 GetColInt64(INT col);
	DOUBLE GetColFloat(INT col);
	CONST BYTE * GetColText(INT col);
	CONST VOID * GetColBlob(INT col);
protected:
	BOOL m_bIsCopy;
	VOID * m_db, * m_stmt;
};
