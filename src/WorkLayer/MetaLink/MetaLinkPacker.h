/*
 * $Id: MetaLinkPacker.h 7494 2008-09-27 06:22:55Z huby $
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

#include "TinyXml.h"

class CMetaLinkPacker
{
public:
	CMetaLinkPacker(void);
	~CMetaLinkPacker(void);
	void SetFileName(const char * name);
	void SetFileSize(__int64 size);
	void AddResource(const char * type, const char * ref, const char * url, int pref);
	void AddReport(const char * cTypename,const char * cReportername,const char* cInfoatt);
	const char * GetXml();
private:
	TiXmlDocument * m_doc;
	TiXmlElement * m_file;
};
