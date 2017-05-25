/*
 * $Id: ini2.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
#include "Ini2.h"
#include "StringConversion.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// If the IniFilename contains no path,
// the module-directory will be add to the FileName,
// to avoid storing in the windows-directory
void CIni::AddModulPath(CString& strFileName,bool bModulPath /*= true*/)
{
   TCHAR drive[_MAX_DRIVE];
   TCHAR dir[_MAX_DIR];
   TCHAR fname[_MAX_FNAME];
   TCHAR ext[_MAX_EXT];

   _tsplitpath( strFileName, drive, dir, fname, ext );
   if( ! drive[0]  )
   {
      //PathCanonicalize(..) doesn't work with for all Plattforms !
      CString strModule;
      if( bModulPath )
      {
         GetModuleFileName(NULL,strModule.GetBuffer(MAX_INI_BUFFER),MAX_INI_BUFFER);
         strModule.ReleaseBuffer();
      }
      else
      {
         GetCurrentDirectory(MAX_INI_BUFFER,strModule.GetBuffer(MAX_INI_BUFFER));
         strModule.ReleaseBuffer();
         // fix by "cpp@world-online.no"
         strModule.TrimRight(_T('\\'));
         strModule.TrimRight(_T('/'));
         strModule += _T("\\");
      }
      strModule.ReleaseBuffer();
      _tsplitpath( strModule, drive, dir, fname, ext );
      strModule = drive;
      strModule+= dir;
      strModule+= strFileName;
      strFileName = strModule;
   }
}

CString CIni::GetDefaultSection()
{
   return AfxGetAppName();
}

CString CIni::GetDefaultIniFile(bool bModulPath /*= true*/)
{
   TCHAR drive[_MAX_DRIVE];
   TCHAR dir[_MAX_DIR];
   TCHAR fname[_MAX_FNAME];
   TCHAR ext[_MAX_EXT];
   CString strTemp;
   CString strApplName;
   GetModuleFileName(NULL,strTemp.GetBuffer(MAX_INI_BUFFER),MAX_INI_BUFFER);
   strTemp.ReleaseBuffer();
   _tsplitpath( strTemp, drive, dir, fname, ext );
   strTemp = fname; //"ApplName"
   strTemp += _T(".ini");  //"ApplName.ini"
   if( bModulPath )
   {
      strApplName  = drive;
      strApplName += dir;
      strApplName += strTemp;
   }
   else
   {
      GetCurrentDirectory(MAX_INI_BUFFER,strApplName.GetBuffer(MAX_INI_BUFFER));
      strApplName.ReleaseBuffer();
      strApplName.TrimRight(_T('\\'));
      strApplName.TrimRight(_T('/'));
      strApplName += _T("\\");
      strApplName += strTemp;
   }
   return strApplName;
}


//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////
// Creates/Use file : "Drive:\ApplPath\ApplName.ini"
CIni::CIni():
	m_bModulPath(true)
{
	m_strFileName = GetDefaultIniFile(m_bModulPath);
	m_strSection  = GetDefaultSection();
}

CIni::CIni(CIni const& Ini):
	m_strFileName(Ini.m_strFileName),
	m_strSection(Ini.m_strSection),
	m_bModulPath(Ini.m_bModulPath)
{
   if(m_strFileName.IsEmpty())
      m_strFileName = GetDefaultIniFile(m_bModulPath);
   AddModulPath(m_strFileName,m_bModulPath);
   if(m_strSection.IsEmpty())
      m_strSection = GetDefaultSection();
}

CIni::CIni(CString const& strFileName):
	m_strFileName(strFileName),
	m_bModulPath(true)
{
	if(m_strFileName.IsEmpty())
		m_strFileName = GetDefaultIniFile(m_bModulPath);
	AddModulPath(m_strFileName,m_bModulPath);
	m_strSection = GetDefaultSection();
}

CIni::CIni(CString const& strFileName, CString const& strSection):
	m_strFileName(strFileName),
	m_strSection(strSection),
	m_bModulPath(true)
{
	if(m_strFileName.IsEmpty())
		m_strFileName = GetDefaultIniFile(m_bModulPath);
	AddModulPath(m_strFileName,m_bModulPath);
	if(m_strSection.IsEmpty())
		m_strSection = GetDefaultSection();
}

CIni::~CIni()
{
}


//////////////////////////////////////////////////////////////////////
// Zugriff auf Quelle/Ziel von IO-Operationen
//////////////////////////////////////////////////////////////////////
void CIni::SetFileName(const CString& strFileName)
{
	m_strFileName = strFileName;
	AddModulPath(m_strFileName);
}

void CIni::SetSection(const CString& strSection)
{
	m_strSection = strSection;
}

const CString& CIni::GetFileName() const
{
	return m_strFileName;
}

const CString& CIni::GetSection() const
{
	return m_strSection;
}


//////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////

void CIni::Init( LPCTSTR strFileName, LPCTSTR strSection/* = NULL*/)
{
	if(strSection != NULL)
		m_strSection = strSection;
	if(strFileName != NULL)
		m_strFileName = strFileName;
}

CString CIni::GetString(LPCTSTR strEntry, LPCTSTR strDefault/*=NULL*/, LPCTSTR strSection/* = NULL*/)
{
	if(strDefault == NULL)
		return CString(GetLPCSTR(strEntry,strSection,_T("")));
	else
		return CString(GetLPCSTR(strEntry,strSection,strDefault));
}
CString CIni::GetStringLong(LPCTSTR strEntry, LPCTSTR strDefault/*=NULL*/, LPCTSTR strSection/* = NULL*/)
{
	CString ret;
	unsigned int maxstrlen=MAX_INI_BUFFER;

	if(strSection != NULL)
		m_strSection = strSection;

	do {
		GetPrivateProfileString(m_strSection, strEntry, (strDefault==NULL)?_T(""):strDefault, 
			ret.GetBufferSetLength(maxstrlen), maxstrlen,m_strFileName);
		ret.ReleaseBuffer();
		if ((unsigned int)ret.GetLength() < maxstrlen-2)
			break;

		maxstrlen+=MAX_INI_BUFFER;

	} while(maxstrlen<32767);

	return ret;
}

CString CIni::GetStringUTF8(LPCTSTR strEntry, LPCTSTR strDefault/*=NULL*/, LPCTSTR strSection/* = NULL*/)
{
	USES_CONVERSION;
	if(strSection != NULL)
		m_strSection = strSection;

	CStringA strUTF8;
	GetPrivateProfileStringA(T2CA(m_strSection), T2CA(strEntry), T2CA(strDefault), 
							 strUTF8.GetBufferSetLength(MAX_INI_BUFFER), MAX_INI_BUFFER, T2CA(m_strFileName));
	strUTF8.ReleaseBuffer();
	return OptUtf8ToStr(strUTF8);
}

double CIni::GetDouble(LPCTSTR strEntry, double fDefault/* = 0.0*/, LPCTSTR strSection/* = NULL*/)
{
	TCHAR strDefault[MAX_PATH];
	_sntprintf(strDefault, ARRSIZE(strDefault), _T("%g"), fDefault);
	GetLPCSTR(strEntry,strSection,strDefault);
	return _tstof(m_chBuffer);
}

float CIni::GetFloat(LPCTSTR strEntry,float fDefault/* = 0.0*/, LPCTSTR strSection/* = NULL*/)
{
	TCHAR strDefault[MAX_PATH];
	_sntprintf(strDefault, ARRSIZE(strDefault), _T("%g"), fDefault);
	GetLPCSTR(strEntry,strSection,strDefault);
	return (float)_tstof(m_chBuffer);
}

int CIni::GetInt(LPCTSTR strEntry,int nDefault/* = 0*/,LPCTSTR strSection/* = NULL*/)
{
	TCHAR strDefault[MAX_PATH];
	_sntprintf(strDefault, ARRSIZE(strDefault), _T("%d"), nDefault);
	GetLPCSTR(strEntry,strSection,strDefault);
	return _tstoi(m_chBuffer);
}

ULONGLONG CIni::GetUInt64(LPCTSTR strEntry,ULONGLONG nDefault/* = 0*/,LPCTSTR strSection/* = NULL*/)
{
	TCHAR strDefault[MAX_PATH];
	_sntprintf(strDefault, ARRSIZE(strDefault), _T("%I64u"), nDefault);
	GetLPCSTR(strEntry,strSection,strDefault);
	ULONGLONG nResult;
	if (_stscanf(m_chBuffer, _T("%I64u"), &nResult) != 1)
		return 0;
	return nResult;
}

WORD CIni::GetWORD(LPCTSTR strEntry,WORD nDefault/* = 0*/,LPCTSTR strSection/* = NULL*/)
{
	TCHAR strDefault[MAX_PATH];
	_sntprintf(strDefault, ARRSIZE(strDefault), _T("%u"), nDefault);
	GetLPCSTR(strEntry,strSection,strDefault);
	return (WORD)_tstoi(m_chBuffer);
}

bool CIni::GetBool(LPCTSTR strEntry,bool bDefault/* = false*/,LPCTSTR strSection/* = NULL*/)
{
	TCHAR strDefault[MAX_PATH];
	_sntprintf(strDefault, ARRSIZE(strDefault), _T("%d"), (int)bDefault);
	GetLPCSTR(strEntry,strSection,strDefault);
	return ( _tstoi(m_chBuffer) != 0 );
}

CPoint CIni::GetPoint(LPCTSTR strEntry,	CPoint ptDefault, LPCTSTR strSection)
{
	CPoint ptReturn=ptDefault;

	CString strDefault;
	strDefault.Format(_T("(%d,%d)"),ptDefault.x, ptDefault.y);

	CString strPoint = GetString(strEntry,strDefault, strSection);
	_stscanf(strPoint,_T("(%d,%d)"), &ptReturn.x, &ptReturn.y);

	return ptReturn;
}

CRect CIni::GetRect(LPCTSTR strEntry, CRect rectDefault, LPCTSTR strSection)
{
	CRect rectReturn=rectDefault;

	CString strDefault;
	//old version :strDefault.Format("(%d,%d,%d,%d)",rectDefault.top,rectDefault.left,rectDefault.bottom,rectDefault.right);
	strDefault.Format(_T("%d,%d,%d,%d"),rectDefault.left,rectDefault.top,rectDefault.right,rectDefault.bottom);

	CString strRect = GetString(strEntry,strDefault,strSection);

	//new Version found
	if( 4==_stscanf(strRect,_T("%d,%d,%d,%d"),&rectDefault.left,&rectDefault.top,&rectDefault.right,&rectDefault.bottom))
		return rectReturn;
	//old Version found
	_stscanf(strRect,_T("(%d,%d,%d,%d)"), &rectReturn.top,&rectReturn.left,&rectReturn.bottom,&rectReturn.right);
	return rectReturn;
}

COLORREF CIni::GetColRef(LPCTSTR strEntry, COLORREF crDefault, LPCTSTR strSection)
{
	int temp[3]={	GetRValue(crDefault),
					GetGValue(crDefault),
					GetBValue(crDefault)};

	CString strDefault;
	strDefault.Format(_T("RGB(%hd,%hd,%hd)"),temp[0],temp[1],temp[2]);

	CString strColRef = GetString(strEntry,strDefault,strSection);
	_stscanf(strColRef,_T("RGB(%d,%d,%d)"), temp, temp+1, temp+2);

	return RGB(temp[0],temp[1],temp[2]);
}
	
void CIni::WriteString(LPCTSTR strEntry, LPCTSTR str, LPCTSTR strSection/* = NULL*/)
{
	if(strSection != NULL) 
		m_strSection = strSection;
	WritePrivateProfileString(m_strSection,strEntry,str,m_strFileName);
}

void CIni::WriteStringUTF8(LPCTSTR strEntry, LPCTSTR psz, LPCTSTR strSection/* = NULL*/)
{
	USES_CONVERSION;
	if(strSection != NULL) 
		m_strSection = strSection;
	CString str(psz);
	WritePrivateProfileStringA(T2CA(m_strSection), T2CA(strEntry), StrToUtf8(str), T2CA(m_strFileName));
}

void CIni::WriteDouble(LPCTSTR strEntry,double f, LPCTSTR strSection/*= NULL*/)
{
	if(strSection != NULL) 
		m_strSection = strSection;
	TCHAR strBuffer[MAX_PATH];
	_sntprintf(strBuffer, ARRSIZE(strBuffer), _T("%g"), f);
	WritePrivateProfileString(m_strSection,strEntry,strBuffer,m_strFileName);
}

void CIni::WriteFloat(LPCTSTR strEntry,float f, LPCTSTR strSection/* = NULL*/)
{
	if(strSection != NULL) 
		m_strSection = strSection;
	TCHAR strBuffer[MAX_PATH];
	_sntprintf(strBuffer, ARRSIZE(strBuffer), _T("%g"), f);
	WritePrivateProfileString(m_strSection,strEntry,strBuffer,m_strFileName);
}

void CIni::WriteInt(LPCTSTR strEntry,int n, LPCTSTR strSection/* = NULL*/)
{
	if(strSection != NULL) 
		m_strSection = strSection;
	TCHAR strBuffer[MAX_PATH];
	_itot(n, strBuffer, 10);
	WritePrivateProfileString(m_strSection,strEntry,strBuffer,m_strFileName);
}

void CIni::WriteUInt64(LPCTSTR strEntry,ULONGLONG n, LPCTSTR strSection/* = NULL*/)
{
	if(strSection != NULL) 
		m_strSection = strSection;
	TCHAR strBuffer[MAX_PATH];
	_ui64tot(n, strBuffer, 10);
	WritePrivateProfileString(m_strSection,strEntry,strBuffer,m_strFileName);
}

void CIni::WriteWORD(LPCTSTR strEntry,WORD n, LPCTSTR strSection/* = NULL*/)
{
	if(strSection != NULL) 
		m_strSection = strSection;
	TCHAR strBuffer[MAX_PATH];
	_ultot(n, strBuffer, 10);
	WritePrivateProfileString(m_strSection,strEntry,strBuffer,m_strFileName);
}

void CIni::WriteBool(LPCTSTR strEntry,bool b, LPCTSTR strSection/* = NULL*/)
{
	if(strSection != NULL) 
		m_strSection = strSection;
	TCHAR strBuffer[MAX_PATH];
	_sntprintf(strBuffer, ARRSIZE(strBuffer), _T("%d"), (int)b);
	WritePrivateProfileString(m_strSection, strEntry, strBuffer, m_strFileName);
}

void CIni::WritePoint(LPCTSTR strEntry,CPoint pt, LPCTSTR strSection)
{
	if(strSection != NULL) 
		m_strSection = strSection;
	CString strBuffer;
	strBuffer.Format(_T("(%d,%d)"),pt.x,pt.y);
	Write(m_strFileName,m_strSection,strEntry,strBuffer);
}

void CIni::WriteRect(LPCTSTR strEntry,CRect rect, LPCTSTR strSection)
{
	if(strSection != NULL) 
		m_strSection = strSection;
	CString strBuffer;
	strBuffer.Format(_T("(%d,%d,%d,%d)"),rect.top,rect.left,rect.bottom,rect.right);
	Write(m_strFileName,m_strSection,strEntry,strBuffer);
}

void CIni::WriteColRef(LPCTSTR strEntry,COLORREF cr, LPCTSTR strSection)
{
	if(strSection != NULL) 
		m_strSection = strSection;
	CString strBuffer;
	strBuffer.Format(_T("RGB(%d,%d,%d)"),GetRValue(cr), GetGValue(cr), GetBValue(cr));
	Write(m_strFileName,m_strSection,strEntry,strBuffer);
}

TCHAR* CIni::GetLPCSTR(LPCTSTR strEntry, LPCTSTR strSection, LPCTSTR strDefault)
{
	// evtl Section neu setzen
	if(strSection != NULL)
		m_strSection = strSection;

	CString temp;
	if(strDefault == NULL)
		temp = Read(m_strFileName,m_strSection,strEntry,CString());
	else
		temp = Read(m_strFileName,m_strSection,strEntry,strDefault);

	return (TCHAR*)memcpy(m_chBuffer,(LPCTSTR)temp,(temp.GetLength() + 1)*sizeof(TCHAR));// '+1' damit die Null am Ende mit kopiert wird
}

void CIni::SerGetString(	bool bGet,CString &	str,LPCTSTR strEntry,LPCTSTR strSection,LPCTSTR strDefault)
{
	if(bGet)
		str = GetString(strEntry,strDefault/*=NULL*/,strSection/* = NULL*/);
	else
		WriteString(strEntry,str, strSection/* = NULL*/);
}
void CIni::SerGetDouble(	bool bGet,double&	f,	LPCTSTR strEntry,LPCTSTR strSection/* = NULL*/,double fDefault/* = 0.0*/)
{
	if(bGet)
		f = GetDouble(strEntry,fDefault/*=NULL*/,strSection/* = NULL*/);
	else
		WriteDouble(strEntry,f, strSection/* = NULL*/);
}
void CIni::SerGetFloat(		bool bGet,float	&	f,	LPCTSTR strEntry, LPCTSTR strSection/* = NULL*/,float fDefault/* = 0.0*/)
{
	if(bGet)
		f = GetFloat(strEntry,fDefault/*=NULL*/,strSection/* = NULL*/);
	else
		WriteFloat(strEntry,f, strSection/* = NULL*/);
}
void CIni::SerGetInt(		bool bGet,int	&	n,	LPCTSTR strEntry,LPCTSTR strSection/* = NULL*/,int nDefault/* = 0*/)
{
	if(bGet)
		n = GetInt(strEntry,nDefault/*=NULL*/,strSection/* = NULL*/);
	else
		WriteInt(strEntry,n, strSection/* = NULL*/);
}
void CIni::SerGetDWORD(		bool bGet,DWORD	&	n,	LPCTSTR strEntry,LPCTSTR strSection/* = NULL*/,DWORD nDefault/* = 0*/)
{
	if(bGet)
		n = (DWORD)GetInt(strEntry,nDefault/*=NULL*/,strSection/* = NULL*/);
	else
		WriteInt(strEntry,n, strSection/* = NULL*/);
}
void CIni::SerGetBool(		bool bGet,bool	&	b,	LPCTSTR strEntry,LPCTSTR strSection/* = NULL*/,bool bDefault/* = false*/)
{
	if(bGet)
		b = GetBool(strEntry,bDefault/*=NULL*/,strSection/* = NULL*/);
	else
		WriteBool(strEntry,b, strSection/* = NULL*/);
}

void CIni::SerGetPoint(	bool bGet,CPoint	& pt,	LPCTSTR strEntry,	LPCTSTR strSection,	CPoint ptDefault)
{
	if(bGet)
		pt = GetPoint(strEntry,ptDefault,strSection);
	else
		WritePoint(strEntry,pt, strSection);
}
void CIni::SerGetRect(		bool bGet,CRect		& rect,	LPCTSTR strEntry,	LPCTSTR strSection,	CRect rectDefault)
{
	if(bGet)
		rect = GetRect(strEntry,rectDefault,strSection);
	else
		WriteRect(strEntry,rect, strSection);
}
void CIni::SerGetColRef(	bool bGet,COLORREF	& cr,	LPCTSTR strEntry,	LPCTSTR strSection,	COLORREF crDefault)
{
	if(bGet)
		cr = GetColRef(strEntry,crDefault,strSection);
	else
		WriteColRef(strEntry,cr, strSection);
}
// Überladene Methoden //////////////////////////////////////////////////////////////////////////////////////////////////77
// Einfache Typen /////////////////////////////////////////////////////////////////////////////////////////////////////////
void		CIni::SerGet(	bool bGet,CString	& str,	LPCTSTR strEntry,	LPCTSTR strSection/*= NULL*/,	LPCTSTR strDefault/*= NULL*/)
{
   SerGetString(bGet,str,strEntry,strSection,strDefault);
}
void		CIni::SerGet(	bool bGet,double	& f,	LPCTSTR strEntry,	LPCTSTR strSection/*= NULL*/,	double fDefault/* = 0.0*/)
{
   SerGetDouble(bGet,f,strEntry,strSection,fDefault);
}
void		CIni::SerGet(	bool bGet,float		& f,	LPCTSTR strEntry,	LPCTSTR strSection/*= NULL*/,	float fDefault/* = 0.0*/)
{
   SerGetFloat(bGet,f,strEntry,strSection,fDefault);
}
void		CIni::SerGet(	bool bGet,int		& n,	LPCTSTR strEntry,	LPCTSTR strSection/*= NULL*/,	int nDefault/* = 0*/)
{
   SerGetInt(bGet,n,strEntry,strSection,nDefault);
}
void		CIni::SerGet(	bool bGet,short		& n,	LPCTSTR strEntry,	LPCTSTR strSection/*= NULL*/,	int nDefault/* = 0*/)
{
   int nTemp = n;
   SerGetInt(bGet,nTemp,strEntry,strSection,nDefault);
   n = (short)nTemp;
}
void		CIni::SerGet(	bool bGet,DWORD		& n,	LPCTSTR strEntry,	LPCTSTR strSection/*= NULL*/,	DWORD nDefault/* = 0*/)
{
   SerGetDWORD(bGet,n,strEntry,strSection,nDefault);
}
void		CIni::SerGet(	bool bGet,WORD		& n,	LPCTSTR strEntry,	LPCTSTR strSection/*= NULL*/,	DWORD nDefault/* = 0*/)
{
   DWORD dwTemp = n;
   SerGetDWORD(bGet,dwTemp,strEntry,strSection,nDefault);
   n = (WORD)dwTemp;
}
void		CIni::SerGet(	bool bGet,CPoint	& pt,	LPCTSTR strEntry,	LPCTSTR strSection/*= NULL*/,	CPoint ptDefault/* = CPoint(0,0)*/)
{
   SerGetPoint(bGet,pt,strEntry,strSection,ptDefault);
}
void		CIni::SerGet(	bool bGet,CRect		& rect,	LPCTSTR strEntry,	LPCTSTR strSection/*= NULL*/,	CRect rectDefault/* = CRect(0,0,0,0)*/)
{
   SerGetRect(bGet,rect,strEntry,strSection,rectDefault);
}

void CIni::SerGet(bool bGet, CString *ar, int nCount, LPCTSTR strEntry, LPCTSTR strSection/*=NULL*/, LPCTSTR Default/*=NULL*/)
{
	if(nCount > 0) {
		CString strBuffer;
		if(bGet) {
			strBuffer = GetString(strEntry, _T(""), strSection);
			int nOffset = 0;
			for(int i = 0; i < nCount; i++) {
				nOffset = Parse(strBuffer, nOffset, ar[i]);
				if(ar[i].GetLength() == 0)
					ar[i] = Default;
			}

		} else {
			strBuffer = ar[0];
			for(int i = 1; i < nCount; i++) {
				strBuffer.AppendChar(_T(','));
				strBuffer.Append(ar[i]);
			}
			WriteString(strEntry, strBuffer, strSection);
		}
	}
}

void CIni::SerGet(bool bGet, double *ar, int nCount, LPCTSTR strEntry, LPCTSTR strSection/*=NULL*/, double Default/* = 0.0*/)
{
	if(nCount > 0) {
		CString strBuffer;
		if(bGet) {
			strBuffer = GetString(strEntry, _T(""), strSection);
			CString strTemp;
			int nOffset = 0;
			for(int i = 0; i < nCount; i++) {
				nOffset = Parse(strBuffer, nOffset, strTemp);
				if(strTemp.GetLength() == 0)
					ar[i] = Default;
				else
					ar[i] = _tstof(strTemp);
			}

		} else {
			CString strTemp;
			strBuffer.Format(_T("%g"), ar[0]);
			for(int i = 1; i < nCount; i++) {
				strTemp.Format(_T("%g"), ar[i]);
				strBuffer.AppendChar(_T(','));
				strBuffer.Append(strTemp);
			}
			WriteString(strEntry, strBuffer, strSection);
		}
	}
}
void CIni::SerGet(bool bGet, float *ar, int nCount, LPCTSTR strEntry, LPCTSTR strSection/*=NULL*/, float Default/* = 0.0*/)
{
	if(nCount > 0) {
		CString strBuffer;
		if(bGet) {
			strBuffer = GetString(strEntry, _T(""), strSection);
			CString strTemp;
			int nOffset = 0;
			for(int i = 0; i < nCount; i++) {
				nOffset = Parse(strBuffer, nOffset, strTemp);
				if(strTemp.GetLength() == 0)
					ar[i] = Default;
				else
					ar[i] = (float)_tstof(strTemp);
			}

		} else {
			CString strTemp;
			strBuffer.Format(_T("%g"), ar[0]);
			for(int i = 1; i < nCount; i++) {
				strTemp.Format(_T("%g"), ar[i]);
				strBuffer.AppendChar(_T(','));
				strBuffer.Append(strTemp);
			}
			WriteString(strEntry, strBuffer, strSection);
		}
	}
}
void CIni::SerGet(bool bGet, int *ar, int nCount, LPCTSTR strEntry, LPCTSTR strSection/*=NULL*/, int Default/* = 0*/)
{
	if(nCount > 0) {
		CString strBuffer;
		if(bGet) {
			strBuffer = GetString(strEntry, _T(""), strSection);
			CString strTemp;
			int nOffset = 0;
			for(int i = 0; i < nCount; i++) {
				nOffset = Parse(strBuffer, nOffset, strTemp);
				if(strTemp.GetLength() == 0)
					ar[i] = Default;
				else
					ar[i] = _tstoi(strTemp);
			}

		} else {
			CString strTemp;
			strBuffer.Format(_T("%d"), ar[0]);
			for(int i = 1; i < nCount; i++) {
				strTemp.Format(_T("%d"), ar[i]);
				strBuffer.AppendChar(_T(','));
				strBuffer.Append(strTemp);
			}
			WriteString(strEntry, strBuffer, strSection);
		}
	}
}
void CIni::SerGet(bool bGet, unsigned char *ar, int nCount, LPCTSTR strEntry, LPCTSTR strSection/*=NULL*/, unsigned char Default/* = 0*/)
{
	if(nCount > 0) {
		CString strBuffer;
		if(bGet) {
			strBuffer = GetString(strEntry, _T(""), strSection);
			CString strTemp;
			int nOffset = 0;
			for(int i = 0; i < nCount; i++) {
				nOffset = Parse(strBuffer, nOffset, strTemp);
				if(strTemp.GetLength() == 0)
					ar[i] = Default;
				else
					ar[i] = (unsigned char)_tstoi(strTemp);
			}

		} else {
			CString strTemp;
			strBuffer.Format(_T("%d"), ar[0]);
			for(int i = 1; i < nCount; i++) {
				strTemp.Format(_T("%d"), ar[i]);
				strBuffer.AppendChar(_T(','));
				strBuffer.Append(strTemp);
			}
			WriteString(strEntry, strBuffer, strSection);
		}
	}
}
void CIni::SerGet(bool bGet, short *ar, int nCount, LPCTSTR strEntry, LPCTSTR strSection/*=NULL*/, int Default/* = 0*/)
{
	if(nCount > 0) {
		CString strBuffer;
		if(bGet) {
			strBuffer = GetString(strEntry, _T(""), strSection);
			CString strTemp;
			int nOffset = 0;
			for(int i = 0; i < nCount; i++) {
				nOffset = Parse(strBuffer, nOffset, strTemp);
				if(strTemp.GetLength() == 0)
					ar[i] = (short)Default;
				else
					ar[i] = (short)_tstoi(strTemp);
			}

		} else {
			CString strTemp;
			strBuffer.Format(_T("%d"), ar[0]);
			for(int i = 1; i < nCount; i++) {
				strTemp.Format(_T("%d"), ar[i]);
				strBuffer.AppendChar(_T(','));
				strBuffer.Append(strTemp);
			}
			WriteString(strEntry, strBuffer, strSection);
		}
	}
}
void CIni::SerGet(bool bGet, DWORD *ar, int nCount, LPCTSTR strEntry, LPCTSTR strSection/*=NULL*/, DWORD Default/* = 0*/)
{
	if(nCount > 0) {
		CString strBuffer;
		if(bGet) {
			strBuffer = GetString(strEntry, _T(""), strSection);
			CString strTemp;
			int nOffset = 0;
			for(int i = 0; i < nCount; i++) {
				nOffset = Parse(strBuffer, nOffset, strTemp);
				if(strTemp.GetLength() == 0)
					ar[i] = Default;
				else
					ar[i] = (DWORD)_tstoi(strTemp);
			}

		} else {
			CString strTemp;
			strBuffer.Format(_T("%d"), ar[0]);
			for(int i = 1; i < nCount; i++) {
				strTemp.Format(_T("%d"), ar[i]);
				strBuffer.AppendChar(_T(','));
				strBuffer.Append(strTemp);
			}
			WriteString(strEntry, strBuffer, strSection);
		}
	}
}
void CIni::SerGet(bool bGet, WORD *ar, int nCount, LPCTSTR strEntry, LPCTSTR strSection/*=NULL*/, DWORD Default/* = 0*/)
{
	if(nCount > 0) {
		CString strBuffer;
		if(bGet) {
			strBuffer = GetString(strEntry, _T(""), strSection);
			CString strTemp;
			int nOffset = 0;
			for(int i = 0; i < nCount; i++) {
				nOffset = Parse(strBuffer, nOffset, strTemp);
				if(strTemp.GetLength() == 0)
					ar[i] = (WORD)Default;
				else
					ar[i] = (WORD)_tstoi(strTemp);
			}

		} else {
			CString strTemp;
			strBuffer.Format(_T("%d"), ar[0]);
			for(int i = 1; i < nCount; i++) {
				strTemp.Format(_T("%d"), ar[i]);
				strBuffer.AppendChar(_T(','));
				strBuffer.Append(strTemp);
			}
			WriteString(strEntry, strBuffer, strSection);
		}
	}
}
void		CIni::SerGet(	bool bGet,CPoint	* ar,	   int nCount, LPCTSTR strEntry,	LPCTSTR strSection/*=NULL*/,	CPoint Default/* = CPoint(0,0)*/)
{
   CString strBuffer;
   for( int i=0 ; i<nCount ; i++)
   {
      strBuffer.Format(_T("_%i"),i);
      strBuffer = strEntry + strBuffer;
      SerGet(bGet,ar[i],strBuffer,strSection,Default);
   }
}
void		CIni::SerGet(	bool bGet,CRect	* ar,	   int nCount, LPCTSTR strEntry,	LPCTSTR strSection/*=NULL*/,	CRect Default/* = CRect(0,0,0,0)*/)
{
   CString strBuffer;
   for( int i=0 ; i<nCount ; i++)
   {
      strBuffer.Format(_T("_%i"),i);
      strBuffer = strEntry + strBuffer;
      SerGet(bGet,ar[i],strBuffer,strSection,Default);
   }
}

int			CIni::Parse(const CString& strIn, int nOffset, CString& strOut) {

	strOut.Empty();
	int nLength = strIn.GetLength();

	if(nOffset < nLength) {
		if(nOffset != 0 && strIn[nOffset] == _T(','))
			nOffset++;

		while(nOffset < nLength) {
			if(!_istspace((_TUCHAR)strIn[nOffset]))
				break;

			nOffset++;
		}

		while(nOffset < nLength) {
			strOut += strIn[nOffset];

			if(strIn[++nOffset] == _T(','))
				break;
		}

		strOut.Trim();
	}
	return nOffset;
}

CString CIni::Read(LPCTSTR strFileName, LPCTSTR strSection, LPCTSTR strEntry, LPCTSTR strDefault)
{
	CString strReturn;
	GetPrivateProfileString(strSection,
							strEntry,
							strDefault,
							strReturn.GetBufferSetLength(MAX_INI_BUFFER),
							MAX_INI_BUFFER,
							strFileName);
	strReturn.ReleaseBuffer();
	return strReturn;
}
void CIni::Write(LPCTSTR strFileName, LPCTSTR strSection, LPCTSTR strEntry, LPCTSTR strValue)
{
	WritePrivateProfileString(strSection,
							strEntry,
							strValue,
							strFileName);
}

bool CIni::GetBinary(LPCTSTR lpszEntry, BYTE** ppData, UINT* pBytes, LPCTSTR pszSection)
{
	*ppData = NULL;
	*pBytes = 0;

	CString str = GetString(lpszEntry, NULL, pszSection);
	if (str.IsEmpty())
		return false;
	ASSERT(str.GetLength()%2 == 0);
	INT_PTR nLen = str.GetLength();
	*pBytes = UINT(nLen)/2;
	*ppData = new BYTE[*pBytes];
	for (int i=0;i<nLen;i+=2)
	{
		(*ppData)[i/2] = (BYTE)(((str[i+1] - 'A') << 4) + (str[i] - 'A'));
	}
	return true;
}

bool CIni::WriteBinary(LPCTSTR lpszEntry, LPBYTE pData, UINT nBytes, LPCTSTR pszSection)
{
	// convert to string and write out
	LPTSTR lpsz = new TCHAR[nBytes*2+1];
	UINT i;
	for (i = 0; i < nBytes; i++)
	{
		lpsz[i*2] = (TCHAR)((pData[i] & 0x0F) + 'A'); //low nibble
		lpsz[i*2+1] = (TCHAR)(((pData[i] >> 4) & 0x0F) + 'A'); //high nibble
	}
	lpsz[i*2] = 0;


	WriteString(lpszEntry, lpsz, pszSection);
	delete[] lpsz;
	return true;
}

void CIni::DeleteKey(LPCTSTR pszKey)
{
	WritePrivateProfileString(m_strSection, pszKey, NULL, m_strFileName);
}
