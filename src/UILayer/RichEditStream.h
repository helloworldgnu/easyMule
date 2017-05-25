/*
 * $Id: RichEditStream.h 4483 2008-01-02 09:19:06Z soarchin $
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

/////////////////////////////////////////////////////////////////////////////
// CRichEditStream window

class CRichEditStream : public CRichEditCtrl
{
public:
	CRichEditStream();
	virtual ~CRichEditStream();

	CRichEditStream& operator<<(LPCTSTR psz);
	CRichEditStream& operator<<(char* psz);
	CRichEditStream& operator<<(UINT uVal);
	CRichEditStream& operator<<(int iVal);
	CRichEditStream& operator<<(double fVal);

	bool IsEmpty() const {
		return GetWindowTextLength() == 0;
	}
	void AppendFormat(LPCTSTR pszFmt, ...) {
		va_list argp;
		va_start(argp, pszFmt);
		CString str;
		str.AppendFormatV(pszFmt, argp);
		va_end(argp);
		*this << str;
	}

	void InitColors();
	CHARFORMAT m_cfDef;
	CHARFORMAT m_cfBold;
	CHARFORMAT m_cfRed;

	void GetRTFText(CStringA& rstrText);

protected:
	static DWORD CALLBACK StreamOutCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb);
	DECLARE_MESSAGE_MAP()
};
