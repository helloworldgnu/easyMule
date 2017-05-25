/* 
 * $Id: DLP.cpp 7586 2008-10-08 10:58:10Z dgkang $
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





#include "stdafx.h"
#include "DLP.h"
#include "otherfunctions.h"
#include "resource.h"
#include "Log.h"
#include "Preferences.h"
#include "emuleDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDLP::CDLP()
{
	dlpavailable=false;
	dlpInstance=NULL;
	Reload();
}

CDLP::~CDLP()
{
	if(dlpInstance!=NULL)
	{
		::FreeLibrary(dlpInstance);
	}
}

void CDLP::Reload()
{
	CString fullpath = thePrefs.GetMuleDirectory(EMULE_MODULEDIR);
	dlpavailable=false;
	bool waserror=false;

	CString newdll = fullpath + _T("antiLeech.dll.new");
	CString olddll = fullpath + _T("antiLeech.dll.old");
	CString currentdll = fullpath + _T("antiLeech.dll");

	BOOL bUpdateOK = FALSE;

	if(PathFileExists(newdll) && thePrefs.m_bUpdateAntiLeecher)
	{
		AddLogLine(false,GetResString(IDS_DLP_NEWVERSION));
		//new version exists, try to unload the old and load the new one
		if(dlpInstance!=NULL)
		{
			::FreeLibrary(dlpInstance);
			dlpInstance=NULL;
		}
		if(PathFileExists(currentdll))
		{
			if(PathFileExists(olddll))
			{
				if(_tremove(olddll)!=0)
					waserror=true;
			}
			if(waserror==false)
				if(_trename(currentdll,olddll)!=0)
					waserror=true;
		}
		if(waserror==false)
		{
			if(_trename(newdll,currentdll)!=0)
				waserror=true;
		}
		if(waserror)
			AddLogLine(false,GetResString(IDS_DLP_LOADOLD));
		else
			bUpdateOK = TRUE;
	}

	if(dlpInstance==NULL)
	{
		dlpInstance=::LoadLibrary(currentdll);
		if(dlpInstance!=NULL)
		{
			//testfunc = (TESTFUNC)GetProcAddress(dlpInstance,("TestFunc"));
			GetDLPVersion = (GETDLPVERSION)GetProcAddress(dlpInstance,("GetDLPVersion"));
			DLPCheckModstring_Hard = (DLPCHECKMODSTRING_HARD)GetProcAddress(dlpInstance,("DLPCheckModstring_Hard"));
			DLPCheckModstring_Soft = (DLPCHECKMODSTRING_SOFT)GetProcAddress(dlpInstance,("DLPCheckModstring_Soft"));

			DLPCheckUsername_Hard = (DLPCHECKUSERNAME_HARD)GetProcAddress(dlpInstance,("DLPCheckUsername_Hard"));
			DLPCheckUsername_Soft = (DLPCHECKUSERNAME_SOFT)GetProcAddress(dlpInstance,("DLPCheckUsername_Soft"));

			DLPCheckNameAndHashAndMod = (DLPCHECKNAMEANDHASHANDMOD)GetProcAddress(dlpInstance,("DLPCheckNameAndHashAndMod"));

			DLPCheckMessageSpam = (DLPCHECKMESSAGESPAM)GetProcAddress(dlpInstance,("DLPCheckMessageSpam"));
			DLPCheckUserhash = (DLPCHECKUSERHASH)GetProcAddress(dlpInstance,("DLPCheckUserhash"));

			DLPCheckHelloTag = (DLPCHECKHELLOTAG)GetProcAddress(dlpInstance,("DLPCheckHelloTag"));
			DLPCheckInfoTag = (DLPCHECKINFOTAG)GetProcAddress(dlpInstance,("DLPCheckInfoTag"));
			if( GetDLPVersion &&
				DLPCheckModstring_Hard &&
				DLPCheckModstring_Soft &&
				DLPCheckUsername_Hard &&
				DLPCheckUsername_Soft &&
				DLPCheckNameAndHashAndMod &&
				DLPCheckHelloTag &&
				DLPCheckInfoTag &&
				DLPCheckMessageSpam &&
				DLPCheckUserhash
				)
			{
				dlpavailable=true;
				AddLogLine(false,GetResString(IDS_DLP_DLLLOADED), GetDLPVersion());
				if (theApp.emuledlg && bUpdateOK)
				{
					CString tcs;
					tcs.Format(GetResString(IDS_DLP_DLLLOADED),GetDLPVersion());
					theApp.emuledlg->ShowNotifier(tcs,TBN_NULL);
				}
			}
			else
			{
				LogError(GetResString(IDS_DLP_LOADFAILED));
				::FreeLibrary(dlpInstance);
				dlpInstance=NULL;
			}
		}
		else
		{
			LogError(GetResString(IDS_DLP_LOADFAILED2));
			LogError(GetResString(IDS_DLP_ERRORCODE), GetLastError());
		}
	}
	else
	{
		AddDebugLogLine(false,GetResString(IDS_DLP_NO_NEWVER));
		dlpavailable=true;
	}
}
