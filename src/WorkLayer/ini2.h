/*
 * $Id: ini2.h 4483 2008-01-02 09:19:06Z soarchin $
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
// Ini.h: Schnittstelle für die Klasse CIni.

// Autor: Michael Schikora
// Mail:  schiko@schikos.de
//
// If you found this code useful,
// please let me know
//
// How to use:
//
//
//void CMyClass::UpdateFromIni(bool bFromIni)
//{
//   CIni ini(m_strFileName,m_strSection);
//   ini.SER_GET(bFromIni,m_nValueXY); 
//   ini.SER_GET(bFromIni,m_strValue);
//   ini.SER_ARR(bFromIni,m_arValue,MAX_AR); 
//   ini.SER_ARR(bFromIni,m_ar3D,3);
//   //ore with default values 
//   ini.SER_GETD(bFromIni,m_nValueXY,5); 
//   ini.SER_GETD(bFromIni,m_strValue,"Hello");
//   ini.SER_ARRD(bFromIni,m_arValue,MAX_AR,10); 
//   ini.SER_ARRD(bFromIni,m_ar3D,3,5); 
//}
#pragma once

#define SER_GET(bGet,value) SerGet(bGet,value,#value)
#define SER_ARR(bGet,value,n) SerGet(bGet,value,n,#value)
#define SER_GETD(bGet,value,default) SerGet(bGet,value,#value,NULL,default)
#define SER_ARRD(bGet,value,n,default) SerGet(bGet,value,n,#value,default)

class CIni  
{
public:

#ifdef __NEVER_DEFINED__
   // MAKRO: SerGet(bGet,value,#value)
   int SER_GET(bool bGet,int value);
   // MAKRO: SerGet(bGet,value,n,#value)
   int SER_ARR(bGet,int* value,int n);
#endif
   // If the IniFilename contains no path,
   // the module-directory will be add to the FileName,
   // to avoid storing in the windows-directory
   // bModulPath=true: ModulDir, bModulPath=false: CurrentDir
	static void AddModulPath(CString& strFileName, bool bModulPath = true);
	static CString GetDefaultSection();
	static CString GetDefaultIniFile(bool bModulPath = true);

	CIni();
	CIni(CIni const& Ini);
	CIni(CString const& strFileName);
	CIni(CString const& strFileName, CString const& strSection);
	virtual ~CIni();

	void SetFileName(const CString& strFileName);
	void SetSection(const CString& strSection);
	const CString& GetFileName() const;
	const CString& GetSection() const;

	CString		GetString(LPCTSTR lpszEntry,	LPCTSTR		lpszDefault = NULL,				LPCTSTR lpszSection = NULL);
	CString		GetStringUTF8(LPCTSTR lpszEntry,LPCTSTR		lpszDefault = NULL,				LPCTSTR lpszSection = NULL);
	CString		GetStringLong(LPCTSTR lpszEntry,LPCTSTR		lpszDefault = NULL,				LPCTSTR lpszSection = NULL);
	double		GetDouble(LPCTSTR lpszEntry,	double		fDefault = 0.0,					LPCTSTR lpszSection = NULL);
	float		GetFloat(LPCTSTR lpszEntry,		float		fDefault = 0.0F,				LPCTSTR lpszSection = NULL);
	int			GetInt(LPCTSTR lpszEntry,		int			nDefault = 0,					LPCTSTR lpszSection = NULL);
	ULONGLONG	GetUInt64(LPCTSTR lpszEntry,	ULONGLONG	nDefault = 0,					LPCTSTR lpszSection = NULL);
	WORD		GetWORD(LPCTSTR lpszEntry,		WORD		nDefault = 0,					LPCTSTR lpszSection = NULL);
	bool		GetBool(LPCTSTR lpszEntry,		bool		bDefault = false,				LPCTSTR lpszSection = NULL);
	CPoint		GetPoint(LPCTSTR lpszEntry,		CPoint		ptDefault = CPoint(0,0),		LPCTSTR lpszSection = NULL);
	CRect		GetRect(LPCTSTR lpszEntry,		CRect		rectDefault = CRect(0,0,0,0),	LPCTSTR lpszSection = NULL);
	COLORREF	GetColRef(LPCTSTR lpszEntry,	COLORREF	crDefault = RGB(128,128,128),	LPCTSTR lpszSection = NULL);
	bool		GetBinary(LPCTSTR lpszEntry,	BYTE** ppData, UINT* pBytes,				LPCTSTR lpszSection = NULL);

	void		WriteString(LPCTSTR strEntry,	LPCTSTR		s,								LPCTSTR lpszSection = NULL);
	void		WriteStringUTF8(LPCTSTR strEntry,LPCTSTR    s,								LPCTSTR lpszSection = NULL);
	void		WriteDouble(LPCTSTR lpszEntry,	double		f,								LPCTSTR lpszSection = NULL);
	void		WriteFloat(LPCTSTR lpszEntry,	float		f,								LPCTSTR lpszSection = NULL);
	void		WriteInt(LPCTSTR lpszEntry,		int			n,								LPCTSTR lpszSection = NULL);
	void		WriteUInt64(LPCTSTR lpszEntry,	ULONGLONG	n,								LPCTSTR lpszSection = NULL);
	void		WriteWORD(LPCTSTR lpszEntry,	WORD		n,								LPCTSTR lpszSection = NULL);
	void		WriteBool(LPCTSTR lpszEntry,	bool		b,								LPCTSTR lpszSection = NULL);
	void		WritePoint(LPCTSTR lpszEntry,	CPoint		pt,								LPCTSTR lpszSection = NULL);
	void		WriteRect(LPCTSTR lpszEntry,	CRect		rect,							LPCTSTR lpszSection = NULL);
	void		WriteColRef(LPCTSTR lpszEntry,	COLORREF	cr,								LPCTSTR lpszSection = NULL);
	bool		WriteBinary(LPCTSTR lpszEntry,	LPBYTE pData, UINT nBytes,					LPCTSTR lpszSection = NULL);

	void		SerGetString(	bool bGet, CString&		s,	LPCTSTR lpszEntry,	LPCTSTR lpszSection = NULL,	LPCTSTR strDefault = NULL);
	void		SerGetDouble(	bool bGet, double&		f,	LPCTSTR lpszEntry,	LPCTSTR lpszSection = NULL,	double fDefault = 0.0);
	void		SerGetFloat(	bool bGet, float&		f,	LPCTSTR lpszEntry,	LPCTSTR lpszSection = NULL,	float fDefault = 0.0);
	void		SerGetInt(		bool bGet, int&			n,	LPCTSTR lpszEntry,	LPCTSTR lpszSection = NULL,	int nDefault = 0);
	void		SerGetDWORD(	bool bGet, DWORD&		n,	LPCTSTR lpszEntry,	LPCTSTR lpszSection = NULL,	DWORD nDefault = 0);
	void		SerGetBool(		bool bGet, bool&		b,	LPCTSTR lpszEntry,	LPCTSTR lpszSection = NULL,	bool bDefault = false);
	void		SerGetPoint(	bool bGet, CPoint&		pt,	LPCTSTR lpszEntry,	LPCTSTR lpszSection = NULL,	CPoint ptDefault = CPoint(0,0));
	void		SerGetRect(		bool bGet, CRect&		rc,	LPCTSTR lpszEntry,	LPCTSTR lpszSection = NULL,	CRect rectDefault = CRect(0,0,0,0));
	void		SerGetColRef(	bool bGet, COLORREF&	cr,	LPCTSTR lpszEntry,	LPCTSTR lpszSection = NULL,	COLORREF crDefault = RGB(128,128,128));

	void		SerGet(	bool bGet, CString&	 s,	 LPCTSTR lpszEntry,	LPCTSTR lpszSection = NULL,	LPCTSTR lpszDefault = NULL);
	void		SerGet(	bool bGet, double&	 f,	 LPCTSTR lpszEntry,	LPCTSTR lpszSection = NULL,	double fDefault = 0.0);
	void		SerGet(	bool bGet, float&	 f,	 LPCTSTR lpszEntry,	LPCTSTR lpszSection = NULL,	float fDefault = 0.0F);
	void		SerGet(	bool bGet, int&		 n,	 LPCTSTR lpszEntry,	LPCTSTR lpszSection = NULL,	int nDefault = 0);
	void		SerGet(	bool bGet, short&	 n,	 LPCTSTR lpszEntry,	LPCTSTR lpszSection = NULL,	int nDefault = 0);
	void		SerGet(	bool bGet, DWORD&	 n,	 LPCTSTR lpszEntry,	LPCTSTR lpszSection = NULL,	DWORD nDefault = 0);
	void		SerGet(	bool bGet, WORD&	 n,	 LPCTSTR lpszEntry,	LPCTSTR lpszSection = NULL,	DWORD nDefault = 0);
//	void		SerGet(	bool bGet, bool&	 b,	 LPCTSTR lpszEntry,	LPCTSTR lpszSection = NULL,	bool bDefault = false);
	void		SerGet(	bool bGet, CPoint&	 pt, LPCTSTR lpszEntry,	LPCTSTR lpszSection = NULL,	CPoint ptDefault = CPoint(0,0));
	void		SerGet(	bool bGet, CRect&	 rc, LPCTSTR lpszEntry,	LPCTSTR lpszSection = NULL,	CRect rectDefault = CRect(0,0,0,0));
//	void		SerGet(	bool bGet, COLORREF& cr, LPCTSTR lpszEntry,	LPCTSTR lpszSection = NULL,	COLORREF crDefault = RGB(128,128,128));
   
//ARRAYs
	void		SerGet(	bool bGet, CString*	s,	int nCount, LPCTSTR lpszEntry, LPCTSTR lpszSection = NULL, LPCTSTR lpszDefault = NULL);
	void		SerGet(	bool bGet, double*	f,	int nCount, LPCTSTR lpszEntry, LPCTSTR lpszSection = NULL, double fDefault = 0.0);
	void		SerGet(	bool bGet, float*	f,	int nCount, LPCTSTR lpszEntry, LPCTSTR lpszSection = NULL, float fDefault = 0.0F);
	void		SerGet(	bool bGet, BYTE*	n,	int nCount, LPCTSTR lpszEntry, LPCTSTR lpszSection = NULL, BYTE nDefault = 0);
	void		SerGet(	bool bGet, int*		n,	int nCount, LPCTSTR lpszEntry, LPCTSTR lpszSection = NULL, int nDefault = 0);
	void		SerGet(	bool bGet, short*	n,	int nCount, LPCTSTR lpszEntry, LPCTSTR lpszSection = NULL, int nDefault = 0);
	void		SerGet(	bool bGet, DWORD*	n,	int nCount, LPCTSTR lpszEntry, LPCTSTR lpszSection = NULL, DWORD nDefault = 0);
	void		SerGet(	bool bGet, WORD*	n,	int nCount, LPCTSTR lpszEntry, LPCTSTR lpszSection = NULL, DWORD nDefault = 0);
	void		SerGet(	bool bGet, CPoint*	pt,	int nCount, LPCTSTR lpszEntry, LPCTSTR lpszSection = NULL, CPoint ptDefault = CPoint(0,0));
	void		SerGet(	bool bGet, CRect*	rc,	int nCount, LPCTSTR lpszEntry, LPCTSTR lpszSection = NULL, CRect rectDefault = CRect(0,0,0,0));

	int			Parse(const CString&, int nOffset, CString &strOut);
	void		DeleteKey(LPCTSTR pszKey);
   //MAKRO :
   //SERGET(bGet,value) SerGet(bGet,value,#value)

private:
	void Init(LPCTSTR lpszIniFile, LPCTSTR lpszSection = NULL);
	LPTSTR GetLPCSTR(LPCTSTR lpszEntry, LPCTSTR lpszSection, LPCTSTR lpszDefault);

	bool  m_bModulPath;  //true: Filenames without path take the Modulepath
                        //false: Filenames without path take the CurrentDirectory

#define MAX_INI_BUFFER 256
	TCHAR	m_chBuffer[MAX_INI_BUFFER];
	CString m_strFileName;
	CString m_strSection;
//////////////////////////////////////////////////////////////////////
// statische Methoden
//////////////////////////////////////////////////////////////////////
public:
	static CString	Read( LPCTSTR strFileName, LPCTSTR strSection, LPCTSTR strEntry, LPCTSTR strDefault);
	static void		Write(LPCTSTR strFileName, LPCTSTR strSection, LPCTSTR strEntry, LPCTSTR strValue);
};
