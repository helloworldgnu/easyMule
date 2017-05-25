/*
 * $Id: UpdateInfo.cpp 5279 2008-04-10 04:35:49Z huby $
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
// UpdateInfo.cpp : 实现文件
//

#include "stdafx.h"
#include "emule.h"
#include "emuleDlg.h"
#include "UpdateInfo.h"
#include "SharedFilesCtrl.h"
#include "SharedFilesWnd.h"


// CUpdateInfo

CUpdateInfo::CUpdateInfo()
{

}

CUpdateInfo::~CUpdateInfo()
{
}


// CUpdateInfo 成员函数

void CUpdateInfo::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar<<m_Hash;
	}
	else
	{
		ar>>m_Hash;
	}
}

BOOL CUpdateInfo::OpenUpdateInfo(void)
{
	CFile file;

	if (file.Open(GetUpdateInfoFile(), CFile::modeRead))
	{
		try
		{
			CArchive ar(&file, CArchive::load);
			Serialize(ar);
		}
		catch (CException* e)
		{
			e->Delete();
			ASSERT(0);
		}

		file.Close();
		return TRUE;
	}
	
	return FALSE;
}

void CUpdateInfo::SaveUpdateInfo(void)
{
	CFile file;
	
	if (file.Open(GetUpdateInfoFile(), CFile::modeCreate | CFile::modeReadWrite ))
	{
		try
		{
			CArchive ar(&file, CArchive::store);
			Serialize(ar);
		}
		catch (CException* e)
		{
			e->Delete();
			ASSERT(0);
		}
	}	

	file.Close();
}

void CUpdateInfo::SetUpdateHash(CString hash)
{
	m_Hash = hash;
	SaveUpdateInfo();
}

CString CUpdateInfo::GetUpdateHash(void)
{
	if(OpenUpdateInfo())
	{
		return m_Hash;
	}

	return NULL;
}

CString CUpdateInfo::GetUpdateInfoFile(void)
{
	return thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("update.dat");
}


BOOL CUpdateInfo::isUpdateFile(CString hash)
{
	CString oldhash = GetUpdateHash();

	if(oldhash.IsEmpty())
	{
		return FALSE;
	}

	if(oldhash.Compare(hash) == 0)
	{
		return TRUE;
	}

	return FALSE;
}


int CUpdateInfo::GetUpdateState(const uchar* hash)
{
	//共享文件列表
	const CKnownFile* file = CGlobalVariable::sharedfiles->GetFileByID(hash);

	if(file)
	{
		if (file->IsPartFile())
		{
			return UPDATE_DOWNLOADING;	//在共享列表中，但是没有下载完成。
		}
		else
		{
			return UPDATE_DOWNLOADED;	//在共享列表中有，已经下载完。
		}
	}
	else
	{	
		//下载列表
		if ((file = CGlobalVariable::downloadqueue->GetFileByID(hash)) != NULL)
		{
			return UPDATE_DOWNLOADING;	//在下载列表中，肯定没有下载完成
		}
		else
		{
			return UPDATE_NODOWNLOAD;	//尚未下载
		}
	}
}

BOOL CUpdateInfo::UpdateInfoFileExists(void)
{
	return PathFileExists(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("update.dat"));
}

int CUpdateInfo::Compare(CString hash)
{
	CString oldhash = GetUpdateHash();
	return oldhash.Compare(hash);
}

BOOL CUpdateInfo::DeleteUpdate(CString hash)
{
	uchar UpdateHash[16];
	CPartFile* pPartFile;

	if(!strmd4(hash,UpdateHash))
	{
		return FALSE;
	}

	CKnownFile* file = CGlobalVariable::sharedfiles->GetFileByID(UpdateHash);

	try
	{
		//共享列表中是否有
		if(file)
		{
			if(file->IsPartFile())
			{
				//共享列表有，但未下载完，移除
				pPartFile = DYNAMIC_DOWNCAST(CPartFile,file);
				if( pPartFile )
				{
					pPartFile->DeleteFile();
				}
			}
			else
			{
				//共享列表有，已经下载完成未安装，移除
				DeleteFile(file->GetFilePath());
				theApp.emuledlg->sharedfileswnd->sharedfilesctrl.RemoveFile(file);
				CGlobalVariable::sharedfiles->RemoveFile(file);
			}
		}

		if ((pPartFile = CGlobalVariable::downloadqueue->GetFileByID(UpdateHash)) != NULL)
		{
			//共享列表没有，但未下载完，移除
			pPartFile->DeleteFile();
		}
	}
	catch (CException* e)
	{
		e->Delete();
		return FALSE;
	}

	return FALSE;
}
