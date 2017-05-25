/*
 * $Id: MetaLinkPacker.cpp 7524 2008-09-28 04:58:08Z huby $
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
#include "stdafx.h"
#include "MetaLinkPacker.h"

CMetaLinkPacker::CMetaLinkPacker(void)
{
	m_doc = new TiXmlDocument();
	m_file = new TiXmlElement("file");
}

CMetaLinkPacker::~CMetaLinkPacker(void)
{
	delete m_file;
	delete m_doc;
}

void CMetaLinkPacker::SetFileName(const char * name)
{
	m_file->SetAttribute("name", name);
}

void CMetaLinkPacker::SetFileSize(__int64 size)
{
	TiXmlElement es("size");
	char p[256] = {0};
	_i64toa(size, p, 10);
	es.InsertEndChild(TiXmlText(p));
	m_file->InsertEndChild(es);
}

void CMetaLinkPacker::AddResource(const char * type, const char * ref, const char * url, int pref)
{
	TiXmlElement * rese = m_file->FirstChildElement("resources");
	if(rese == NULL)
		rese = m_file->InsertEndChild(TiXmlElement("resources"))->ToElement();
	TiXmlElement resf("url");
	resf.SetAttribute("type", type);
	if(ref != 0 && ref[0] != 0)
		resf.SetAttribute("referrer", ref);
	if(pref > -2)
		resf.SetAttribute("preference", pref);
	resf.InsertEndChild(TiXmlText(url));
	rese->InsertEndChild(resf);
}

void CMetaLinkPacker::AddReport(const char * cTypename,const char * cReportername,const char* cInfoatt)
{
	TiXmlElement * repts = m_file->FirstChildElement("vc:reports");
	if( repts == NULL)
		repts = m_file->InsertEndChild(TiXmlElement("vc:reports"))->ToElement();

	TiXmlElement rept("vc:report");

	TiXmlElement type("vc:type");
	type.InsertEndChild(TiXmlText(cTypename));
	rept.InsertEndChild(type);

	TiXmlElement reporter("vc:reporter");
	reporter.InsertEndChild(TiXmlText(cReportername));
	rept.InsertEndChild(reporter);

	if(cInfoatt!=NULL && cInfoatt[0]!=0 )
	{
		TiXmlElement info("vc:info");
		info.InsertEndChild(TiXmlText(cInfoatt));		
		rept.InsertEndChild(info);
	}

	repts->InsertEndChild(rept);
/*
	TiXmlElement * verycd = m_file->FirstChildElement("verycd");
	if( verycd == NULL)
		verycd = m_file->InsertEndChild(TiXmlElement("verycd"))->ToElement();

	TiXmlElement * repts = verycd->FirstChildElement("reports");
	if( repts == NULL)
		repts = verycd->InsertEndChild(TiXmlElement("reports"))->ToElement();

	TiXmlElement rept("report");
	
	TiXmlElement type("type");
	type.InsertEndChild(TiXmlText(cTypename));
	rept.InsertEndChild(type);

	TiXmlElement reporter("reporter");
	reporter.InsertEndChild(TiXmlText(cReportername));
	rept.InsertEndChild(reporter);

	if(cInfoatt!=NULL && cInfoatt[0]!=0 )
	{
		TiXmlElement info("info");
		info.InsertEndChild(TiXmlText(cInfoatt));		
		rept.InsertEndChild(info);
	}

	repts->InsertEndChild(rept);
*/
}

const char * CMetaLinkPacker::GetXml()
{
	m_doc->Clear();
	TiXmlElement ele1("metalink");
	ele1.SetAttribute("version", "3.0");
	ele1.SetAttribute("generator", "easyMule");
	ele1.SetAttribute("xmlns", "http://www.metalinker.org/");
	ele1.SetAttribute("xmlns:vc", "http://www.verycd.com/specs/metalinker.dtd");
	TiXmlElement ele2("files");
	ele2.InsertEndChild(* m_file);
	ele1.InsertEndChild(ele2);
	m_doc->InsertEndChild(TiXmlDeclaration("1.0", "utf-8", ""));
	m_doc->InsertEndChild(ele1);
	TiXmlPrinter printer;

	printer.SetStreamPrinting();
	
//	printer.SetIndent("  ");
	
//	printer.SetLineBreak("\n");
	m_doc->Accept( &printer );
	const char * p = printer.CStr();
	size_t l = strlen(p) + 1;
	char * r = new char[l];
	strcpy(r, p);
	return r;
}
