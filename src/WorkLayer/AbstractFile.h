/* 
 * $Id: AbstractFile.h 9297 2008-12-24 09:55:04Z dgkang $
 * 
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#pragma once
#include <list>
#include "opcodes.h"

/*
					   CPartFile
					 /
		  CKnownFile
		/
CAbstractFile - CCollectionFile
		\
		  CSearchFile
*/

namespace Kademlia
{
	class CUInt128;
	class CEntry;
	typedef std::list<CStringW> WordList;
};

class CTag;

typedef CTypedPtrList<CPtrList, Kademlia::CEntry*> CKadEntryPtrList;

class CAbstractFile: public CObject
{
	DECLARE_DYNAMIC(CAbstractFile)

public:
	CAbstractFile();
	CAbstractFile(const CAbstractFile* pAbstractFile);
	virtual ~CAbstractFile();

	const CString& GetFileName() const { return m_strFileName; }
	virtual void SetFileName(LPCTSTR pszFileName, bool bReplaceInvalidFileSystemChars = false, bool bAutoSetFileType = true, bool bRemoveControlChars = false); // 'bReplaceInvalidFileSystemChars' is set to 'false' for backward compatibility!
	virtual bool    GetFileNameConflicted() const { return false; }

	// returns the ED2K file type (an ASCII string)
	const CString& GetFileType() const { return m_strFileType; }
	virtual void SetFileType(LPCTSTR pszFileType);

	// returns the file type which is used to be shown in the GUI
	CString GetFileTypeDisplayStr() const;

	const uchar* GetFileHash() const { return m_abyFileHash; }
	void SetFileHash(const uchar* pucFileHash);
	bool HasNullHash() const;

	EMFileSize		GetFileSize() const					{ return m_nFileSize; }
	virtual void	SetFileSize(EMFileSize nFileSize)	{ m_nFileSize = nFileSize; }
	bool			IsLargeFile() const					{ return m_nFileSize > (uint64)OLD_MAX_EMULE_FILE_SIZE; }

	uint32 GetIntTagValue(uint8 tagname) const;
	uint32 GetIntTagValue(LPCSTR tagname) const;
	bool GetIntTagValue(uint8 tagname, uint32& ruValue) const;
	uint64 GetInt64TagValue(uint8 tagname) const;
	uint64 GetInt64TagValue(LPCSTR tagname) const;
	bool GetInt64TagValue(uint8 tagname, uint64& ruValue) const;
	void SetIntTagValue(uint8 tagname, uint32 uValue);
	void SetInt64TagValue(uint8 tagname, uint64 uValue);
	const CString& GetStrTagValue(uint8 tagname) const;
	const CString& GetStrTagValue(LPCSTR tagname) const;
	void SetStrTagValue(uint8 tagname, LPCTSTR);
	CTag* GetTag(uint8 tagname, uint8 tagtype) const;
	CTag* GetTag(LPCSTR tagname, uint8 tagtype) const;
	CTag* GetTag(uint8 tagname) const;
	CTag* GetTag(LPCSTR tagname) const;
	const CArray<CTag*, CTag*>& GetTags() const { return taglist; }
	void AddTagUnique(CTag* pTag);
	void DeleteTag(uint8 tagname);
	void DeleteTag(CTag* pTag);
	void ClearTags();
	void CopyTags(const CArray<CTag*, CTag*>& tags);
	virtual bool IsPartFile() const { return false; }

	bool	HasComment() const { return m_bHasComment; }
	void	SetHasComment(bool in) { m_bHasComment = in; }
	UINT	UserRating(bool bKadSearchIndicator = false) const { return (bKadSearchIndicator && m_bKadCommentSearchRunning)  ? 6 : m_uUserRating; }
	bool	HasRating()	const { return m_uUserRating > 0; }
	bool	HasBadRating()	const { return ( HasRating() && (m_uUserRating < 2)); }
	void	SetUserRating(UINT in) { m_uUserRating = in; }
	const CString& GetFileComment() /*const*/;
	UINT	GetFileRating() /*const*/;
	void	LoadComment();
	virtual void	UpdateFileRatingCommentAvail(bool bForceUpdate = true) = 0;

	bool	AddNote(Kademlia::CEntry* pEntry);
	void	RefilterKadNotes(bool bUpdate = true);
	const	CKadEntryPtrList& getNotes() const { return m_kadNotes; }

	bool			IsKadCommentSearchRunning() const						{ return m_bKadCommentSearchRunning; }
	void			SetKadCommentSearchRunning(bool bVal);

#ifdef _DEBUG
	// Diagnostic Support
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CString m_strFileName;
	uchar	m_abyFileHash[16];
	EMFileSize	m_nFileSize;
	CString m_strComment;
	UINT	m_uRating;
	bool	m_bCommentLoaded;
	UINT	m_uUserRating;
	bool	m_bHasComment;
	bool	m_bKadCommentSearchRunning;
	CString m_strFileType;
	CArray<CTag*, CTag*> taglist;
	CKadEntryPtrList m_kadNotes;
};
