/* 
 * $Id: IEMonitor.h 5178 2008-03-28 09:41:48Z soarchin $
 * 
 * this file is part of easyMule
 * Copyright (C)2002-2007 VeryCD Dev Team ( strEmail.Format("%s@%s", "devteam", "easymule.org") / http://www.easymule.org )
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

// IEMonitor.h Added by Soar Chin (8/31/2007)

#pragma once

class CIEMonitor
{
public:
	CIEMonitor(void);
	~CIEMonitor(void);
	static void ApplyChanges(void);
	static void RegisterAll(void);
	static BOOL IsRegistered(void);
	static BOOL IsPathOk(void);
private:
	static bool m_bFirstRun, m_bIEMenu, m_bMonitor, m_bAlt, m_bEd2k;
	static WORD m_wLangID;
	static BOOL RegisterLibrary(LPCTSTR szName);
	static BOOL UnregisterLibrary(LPCTSTR szName);
protected:
	static BOOL CheckForUpdate( CString realpath );
};
