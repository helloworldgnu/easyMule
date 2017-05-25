/*
 * $Id: WebServices.h 4483 2008-01-02 09:19:06Z soarchin $
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

#define	WEBSVC_GEN_URLS		0x0001
#define	WEBSVC_FILE_URLS	0x0002

class CTitleMenu;

class CWebServices
{
public:
	CWebServices();

	CString GetDefaultServicesFile() const;
	int ReadAllServices();
	void RemoveAllServices();

	int GetFileMenuEntries(CTitleMenu* pMenu) { return GetAllMenuEntries(pMenu, WEBSVC_FILE_URLS); }
	int GetGeneralMenuEntries(CTitleMenu* pMenu) { return GetAllMenuEntries(pMenu, WEBSVC_GEN_URLS); }
	int GetAllMenuEntries(CTitleMenu* pMenu, DWORD dwFlags = WEBSVC_GEN_URLS | WEBSVC_FILE_URLS);
	bool RunURL(const CAbstractFile* file, UINT uMenuID);
	void Edit();

protected:
	struct SEd2kLinkService
	{
		UINT uMenuID;
		CString strMenuLabel;
		CString strUrl;
		BOOL bFileMacros;
	};
	CArray<SEd2kLinkService> m_aServices;
	time_t m_tDefServicesFileLastModified;
};

extern CWebServices theWebServices;
