/*
 * $Id: JavaScriptEscape.h 4483 2008-01-02 09:19:06Z soarchin $
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
// JavaScriptEscape.h: interface for the JavaScriptEscape class.
// Added by thilon on 2006.08.28
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_JAVASCRIPTESCAPE_H__93ED2E50_F5D4_4B07_8D06_9B16B4CAD05D__INCLUDED_)
#define AFX_JAVASCRIPTESCAPE_H__93ED2E50_F5D4_4B07_8D06_9B16B4CAD05D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class JavaScriptEscape  
{
public:
	JavaScriptEscape();
	virtual ~JavaScriptEscape();

	CString Escape(CString strValue);
	CString UnEscape(CString strValue);

protected:
	CString javaEscape(CString s);
};

#endif // !defined(AFX_JAVASCRIPTESCAPE_H__93ED2E50_F5D4_4B07_8D06_9B16B4CAD05D__INCLUDED_)
