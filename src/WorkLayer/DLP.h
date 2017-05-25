/* 
 * $Id: DLP.h 7361 2008-09-22 08:15:22Z jimmyc $
 * 
 * this file is part of eMule Xtreme-Mod (http://www.xtreme-mod.net)
 * Copyright (C)2002-2007 Xtreme-Mod (emulextreme@yahoo.de)
 */

//emule Xtreme is a modification of eMule
//Copyright (C)2002-2007 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

//
//
//	Author: Xman 
//  


#pragma once

class CDLP
{
public:
	CDLP();
	~CDLP();
	bool IsDLPavailable()		{return dlpavailable;}
	void Reload();

	typedef DWORD (__cdecl *GETDLPVERSION)();
	GETDLPVERSION GetDLPVersion;

	typedef LPCTSTR (__cdecl *DLPCHECKMODSTRING_HARD)(LPCTSTR modversion, LPCTSTR clientversion);
	DLPCHECKMODSTRING_HARD DLPCheckModstring_Hard;

	typedef LPCTSTR (__cdecl *DLPCHECKMODSTRING_SOFT)(LPCTSTR modversion, LPCTSTR clientversion);
	DLPCHECKMODSTRING_SOFT DLPCheckModstring_Soft;

	typedef LPCTSTR (__cdecl *DLPCHECKUSERNAME_HARD)(LPCTSTR username);
	DLPCHECKUSERNAME_HARD DLPCheckUsername_Hard;

	typedef LPCTSTR (__cdecl *DLPCHECKUSERNAME_SOFT)(LPCTSTR username);
	DLPCHECKUSERNAME_SOFT DLPCheckUsername_Soft;
	
	typedef LPCTSTR (__cdecl *DLPCHECKNAMEANDHASHANDMOD)(CString username, CString& userhash, CString& modversion);
	DLPCHECKNAMEANDHASHANDMOD DLPCheckNameAndHashAndMod;

	typedef LPCTSTR (__cdecl *DLPCHECKMESSAGESPAM)(LPCTSTR messagetext);
	DLPCHECKMESSAGESPAM DLPCheckMessageSpam;

	typedef LPCTSTR (__cdecl *DLPCHECKUSERHASH)(const PBYTE userhash);
	DLPCHECKUSERHASH DLPCheckUserhash;

	typedef LPCTSTR (__cdecl *DLPCHECKHELLOTAG)(UINT tagnumber);
	DLPCHECKHELLOTAG DLPCheckHelloTag;
	typedef LPCTSTR (__cdecl *DLPCHECKINFOTAG)(UINT tagnumber);
	DLPCHECKINFOTAG DLPCheckInfoTag;

	//typedef void (WINAPI*TESTFUNC)();
	//TESTFUNC testfunc;

	

private:
	HINSTANCE dlpInstance;
	bool	dlpavailable;
};
