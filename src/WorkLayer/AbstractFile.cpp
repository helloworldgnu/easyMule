/* 
 * $Id: AbstractFile.cpp 9297 2008-12-24 09:55:04Z dgkang $
 * 
 *  parts of this file are based on work from pan One (http://home-3.tiscali.nl/~meost/pms/)
 * this file is part of eMule
 * Copyright (C)2002-2006 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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
#include "AbstractFile.h"
#include "OtherFunctions.h"
#include "Kademlia/Kademlia/Entry.h"
#include "ini2.h"
#include "Preferences.h"
#include "opcodes.h"
#include "Packets.h"
#ifdef _DEBUG
#include "DebugHelpers.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CAbstractFile, CObject)

CAbstractFile::CAbstractFile()
{
	md4clr(m_abyFileHash);
	m_nFileSize = (uint64)0;
	m_uRating = 0;
	m_bCommentLoaded = false;
	m_uUserRating = 0;
	m_bHasComment = false;
	m_bKadCommentSearchRunning = false;
}

CAbstractFile::CAbstractFile(const CAbstractFile* pAbstractFile)
{
	m_strFileName = pAbstractFile->m_strFileName;
	md4cpy(m_abyFileHash, pAbstractFile->GetFileHash());
	m_nFileSize = pAbstractFile->m_nFileSize;
	m_strComment = pAbstractFile->m_strComment;
	m_uRating = pAbstractFile->m_uRating;
	m_bCommentLoaded = pAbstractFile->m_bCommentLoaded;
	m_uUserRating = pAbstractFile->m_uUserRating;
	m_bHasComment = pAbstractFile->m_bHasComment;
	m_strFileType = pAbstractFile->m_strFileType;
	m_bKadCommentSearchRunning = pAbstractFile->m_bKadCommentSearchRunning;

	const CTypedPtrList<CPtrList, Kademlia::CEntry*>& list = pAbstractFile->getNotes();
	for(POSITION pos = list.GetHeadPosition(); pos != NULL; )
	{
			Kademlia::CEntry* entry = list.GetNext(pos);
			m_kadNotes.AddTail(entry->Copy());
	}

	CopyTags(pAbstractFile->GetTags());
}

CAbstractFile::~CAbstractFile()
{
	try
	{
		ClearTags();
		for(POSITION pos = m_kadNotes.GetHeadPosition(); pos != NULL; )
		{
			Kademlia::CEntry* entry = m_kadNotes.GetNext(pos);
			delete entry;
		}
	}
	catch(...)
	{
	}
}

#ifdef _DEBUG
void CAbstractFile::AssertValid() const
{
	CObject::AssertValid();
	(void)m_strFileName;
	(void)m_abyFileHash;
	(void)m_nFileSize;
	(void)m_strComment;
	(void)m_uRating;
	(void)m_strFileType;
	(void)m_uUserRating;
	CHECK_BOOL(m_bHasComment);
	CHECK_BOOL(m_bCommentLoaded);
	taglist.AssertValid();
}

void CAbstractFile::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
}
#endif

bool CAbstractFile::AddNote(Kademlia::CEntry* pEntry)
{
	for(POSITION pos = m_kadNotes.GetHeadPosition(); pos != NULL; )
	{
		Kademlia::CEntry* entry = m_kadNotes.GetNext(pos);
		if(entry->m_uSourceID == pEntry->m_uSourceID)
		{
			ASSERT(entry != pEntry);
			return false;
		}
	}
	m_kadNotes.AddHead(pEntry);
	UpdateFileRatingCommentAvail();
	return true;
}

UINT CAbstractFile::GetFileRating() /*const*/
{
	if (!m_bCommentLoaded)
		LoadComment();
	return m_uRating;
}

const CString& CAbstractFile::GetFileComment() /*const*/
{
	if (!m_bCommentLoaded)
		LoadComment();
	return m_strComment;
}

void CAbstractFile::LoadComment()
{
	CIni ini(thePrefs.GetFileCommentsFilePath(), md4str(GetFileHash()));
	m_strComment = ini.GetStringUTF8(_T("Comment")).Left(MAXFILECOMMENTLEN);
	m_uRating = ini.GetInt(_T("Rate"), 0);
	m_bCommentLoaded = true;
}

void CAbstractFile::CopyTags(const CArray<CTag*, CTag*>& tags)
{
	for (int i = 0; i < tags.GetSize(); i++)
		taglist.Add(new CTag(*tags.GetAt(i)));
}

void CAbstractFile::ClearTags()
{
	for (int i = 0; i < taglist.GetSize(); i++)
		delete taglist[i];
	taglist.RemoveAll();
}

void CAbstractFile::AddTagUnique(CTag* pTag)
{
	for (int i = 0; i < taglist.GetSize(); i++){
		const CTag* pCurTag = taglist[i];
		if ( (   (pCurTag->GetNameID()!=0 && pCurTag->GetNameID()==pTag->GetNameID())
			  || (pCurTag->GetName()!=NULL && pTag->GetName()!=NULL && CmpED2KTagName(pCurTag->GetName(), pTag->GetName())==0)
			 )
			 && pCurTag->GetType() == pTag->GetType()){
			delete pCurTag;
			taglist.SetAt(i, pTag);
			return;
		}
	}
	taglist.Add(pTag);
}

void CAbstractFile::SetFileName(LPCTSTR pszFileName, bool bReplaceInvalidFileSystemChars, bool bAutoSetFileType, bool bRemoveControlChars)
{ 
	m_strFileName = pszFileName;
	if (bReplaceInvalidFileSystemChars){
		m_strFileName.Replace(_T('/'), _T('-'));
		m_strFileName.Replace(_T('>'), _T('-'));
		m_strFileName.Replace(_T('<'), _T('-'));
		m_strFileName.Replace(_T('*'), _T('-'));
		m_strFileName.Replace(_T(':'), _T('-'));
		m_strFileName.Replace(_T('?'), _T('-'));
		m_strFileName.Replace(_T('\"'), _T('-'));
		m_strFileName.Replace(_T('\\'), _T('-'));
		m_strFileName.Replace(_T('|'), _T('-'));
	}
	if (bAutoSetFileType)
		SetFileType(GetFileTypeByName(m_strFileName));
	
	if (bRemoveControlChars){
		for (int i = 0; i < m_strFileName.GetLength(); )
			if (m_strFileName.GetAt(i) <= '\x1F')
				m_strFileName.Delete(i);
			else
				i++;
	}
} 
      
void CAbstractFile::SetFileType(LPCTSTR pszFileType)
{ 
	m_strFileType = pszFileType;
}

CString CAbstractFile::GetFileTypeDisplayStr() const
{
	CString strFileTypeDisplayStr(GetFileTypeDisplayStrFromED2KFileType(GetFileType()));
	if (strFileTypeDisplayStr.IsEmpty())
		strFileTypeDisplayStr = GetFileType();
	return strFileTypeDisplayStr;
}


void CAbstractFile::SetFileHash(const uchar* pucFileHash)
{
	md4cpy(m_abyFileHash, pucFileHash);
}

bool CAbstractFile::HasNullHash() const
{
	return isnulmd4(m_abyFileHash);
}

uint32 CAbstractFile::GetIntTagValue(uint8 tagname) const
{
	for (int i = 0; i < taglist.GetSize(); i++){
		const CTag* pTag = taglist[i];
		if (pTag->GetNameID()==tagname && pTag->IsInt())
			return pTag->GetInt();
	}
	return 0;
}

bool CAbstractFile::GetIntTagValue(uint8 tagname, uint32& ruValue) const
{
	for (int i = 0; i < taglist.GetSize(); i++){
		const CTag* pTag = taglist[i];
		if (pTag->GetNameID()==tagname && pTag->IsInt()){
			ruValue = pTag->GetInt();
			return true;
		}
	}
	return false;
}

uint64 CAbstractFile::GetInt64TagValue(LPCSTR tagname) const
{
	for (int i = 0; i < taglist.GetSize(); i++){
		const CTag* pTag = taglist[i];
		if (pTag->GetNameID()==0 && pTag->IsInt64(true) && CmpED2KTagName(pTag->GetName(), tagname)==0)
			return pTag->GetInt64();
	}
	return 0;
}

uint64 CAbstractFile::GetInt64TagValue(uint8 tagname) const
{
	for (int i = 0; i < taglist.GetSize(); i++){
		const CTag* pTag = taglist[i];
		if (pTag->GetNameID()==tagname && pTag->IsInt64(true))
			return pTag->GetInt64();
	}
	return 0;
}

bool CAbstractFile::GetInt64TagValue(uint8 tagname, uint64& ruValue) const
{
	for (int i = 0; i < taglist.GetSize(); i++){
		const CTag* pTag = taglist[i];
		if (pTag->GetNameID()==tagname && pTag->IsInt64(true)){
			ruValue = pTag->GetInt64();
			return true;
		}
	}
	return false;
}

uint32 CAbstractFile::GetIntTagValue(LPCSTR tagname) const
{
	for (int i = 0; i < taglist.GetSize(); i++){
		const CTag* pTag = taglist[i];
		if (pTag->GetNameID()==0 && pTag->IsInt() && CmpED2KTagName(pTag->GetName(), tagname)==0)
			return pTag->GetInt();
	}
	return 0;
}

void CAbstractFile::SetIntTagValue(uint8 tagname, uint32 uValue)
{
	for (int i = 0; i < taglist.GetSize(); i++){
		CTag* pTag = taglist[i];
		if (pTag->GetNameID()==tagname && pTag->IsInt()){
			pTag->SetInt(uValue);
			return;
		}
	}
	CTag* pTag = new CTag(tagname, uValue);
	taglist.Add(pTag);
}

void CAbstractFile::SetInt64TagValue(uint8 tagname, uint64 uValue)
{
	for (int i = 0; i < taglist.GetSize(); i++){
		CTag* pTag = taglist[i];
		if (pTag->GetNameID()==tagname && pTag->IsInt64(true)){
			pTag->SetInt64(uValue);
			return;
		}
	}
	CTag* pTag = new CTag(tagname, uValue);
	taglist.Add(pTag);
}

const CString& CAbstractFile::GetStrTagValue(uint8 tagname) const
{
	for (int i = 0; i < taglist.GetSize(); i++){
		const CTag* pTag = taglist[i];
		if (pTag->GetNameID()==tagname && pTag->IsStr())
			return pTag->GetStr();
	}
	static const CString _strEmpty;
	return _strEmpty;
}

const CString& CAbstractFile::GetStrTagValue(LPCSTR tagname) const
{
	for (int i = 0; i < taglist.GetSize(); i++){
		const CTag* pTag = taglist[i];
		if (pTag->GetNameID()==0 && pTag->IsStr() && CmpED2KTagName(pTag->GetName(), tagname)==0)
			return pTag->GetStr();
	}
	static const CString _strEmpty;
	return _strEmpty;
}

void CAbstractFile::SetStrTagValue(uint8 tagname, LPCTSTR pszValue)
{
	for (int i = 0; i < taglist.GetSize(); i++){
		CTag* pTag = taglist[i];
		if (pTag->GetNameID()==tagname && pTag->IsStr()){
			pTag->SetStr(pszValue);
			return;
		}
	}
	CTag* pTag = new CTag(tagname, pszValue);
	taglist.Add(pTag);
}

CTag* CAbstractFile::GetTag(uint8 tagname, uint8 tagtype) const
{
	for (int i = 0; i < taglist.GetSize(); i++){
		CTag* pTag = taglist[i];
		if (pTag->GetNameID()==tagname && pTag->GetType()==tagtype)
			return pTag;
	}
	return NULL;
}

CTag* CAbstractFile::GetTag(LPCSTR tagname, uint8 tagtype) const
{
	for (int i = 0; i < taglist.GetSize(); i++){
		CTag* pTag = taglist[i];
		if (pTag->GetNameID()==0 && pTag->GetType()==tagtype && CmpED2KTagName(pTag->GetName(), tagname)==0)
			return pTag;
	}
	return NULL;
}

CTag* CAbstractFile::GetTag(uint8 tagname) const
{
	for (int i = 0; i < taglist.GetSize(); i++){
		CTag* pTag = taglist[i];
		if (pTag->GetNameID()==tagname)
			return pTag;
	}
	return NULL;
}

CTag* CAbstractFile::GetTag(LPCSTR tagname) const
{
	for (int i = 0; i < taglist.GetSize(); i++){
		CTag* pTag = taglist[i];
		if (pTag->GetNameID()==0 && CmpED2KTagName(pTag->GetName(), tagname)==0)
			return pTag;
	}
	return NULL;
}

void CAbstractFile::DeleteTag(CTag* pTag)
{
	for (int i = 0; i < taglist.GetSize(); i++){
		if (taglist[i] == pTag){
			taglist.RemoveAt(i);
			delete pTag;
			return;
		}
	}
}

void CAbstractFile::DeleteTag(uint8 tagname)
{
	for (int i = 0; i < taglist.GetSize(); i++){
		CTag* pTag = taglist[i];
		if (pTag->GetNameID()==tagname){
			taglist.RemoveAt(i);
			delete pTag;
			return;
		}
	}
}

void CAbstractFile::SetKadCommentSearchRunning(bool bVal){
	if (bVal != m_bKadCommentSearchRunning){
		m_bKadCommentSearchRunning = bVal;
		UpdateFileRatingCommentAvail(true);
	}
}

void CAbstractFile::RefilterKadNotes(bool bUpdate){
	// check all availabe comments against our filter again
	if (thePrefs.GetCommentFilter().IsEmpty())
		return;
	POSITION pos1, pos2;
	for (pos1 = m_kadNotes.GetHeadPosition();( pos2 = pos1 ) != NULL;)
	{
		m_kadNotes.GetNext(pos1);
		Kademlia::CEntry* entry = m_kadNotes.GetAt(pos2);
		if (!entry->GetStrTagValue(TAG_DESCRIPTION).IsEmpty()){
			CString strCommentLower(entry->GetStrTagValue(TAG_DESCRIPTION));
			strCommentLower.MakeLower();

			int iPos = 0;
			CString strFilter(thePrefs.GetCommentFilter().Tokenize(_T("|"), iPos));
			while (!strFilter.IsEmpty())
			{
				// comment filters are already in lowercase, compare with temp. lowercased received comment
				if (strCommentLower.Find(strFilter) >= 0)
				{
					m_kadNotes.RemoveAt(pos2);
					delete entry;
					break;
				}
				strFilter = thePrefs.GetCommentFilter().Tokenize(_T("|"), iPos);
			}		
		}
	}
	if (bUpdate) // untill updated rating and m_bHasComment might be wrong
		UpdateFileRatingCommentAvail();
}