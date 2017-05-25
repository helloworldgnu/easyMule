/* 
 * $Id: SafeFile.h 4555 2008-01-11 10:13:40Z wangna $
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

namespace Kademlia{
	class CUInt128;
};

#define MAX_CFEXP_ERRORMSG	(MAX_PATH + 256)

enum EUtf8Str
{
	utf8strNone,
	utf8strOptBOM,
	utf8strRaw
};


///////////////////////////////////////////////////////////////////////////////
// CFileDataIO

class CFileDataIO
{
public:
	virtual UINT Read(void* lpBuf, UINT nCount) = 0;
	virtual void Write(const void* lpBuf, UINT nCount) = 0;
	virtual ULONGLONG Seek(LONGLONG lOff, UINT nFrom) = 0;
	virtual ULONGLONG GetPosition() const = 0;
	virtual ULONGLONG GetLength() const = 0;

	virtual uint8 ReadUInt8();
	virtual uint16 ReadUInt16();
	virtual uint32 ReadUInt32();
	virtual uint64 ReadUInt64();
	virtual void ReadUInt128(Kademlia::CUInt128 *pVal);
	virtual void ReadHash16(uchar* pVal);
	virtual CString ReadString(bool bOptUTF8);
	virtual CString ReadString(bool bOptUTF8, UINT uRawSize);
	virtual CStringW ReadStringUTF8();

	virtual void WriteUInt8(uint8 nVal);
	virtual void WriteUInt16(uint16 nVal);
	virtual void WriteUInt32(uint32 nVal);
	virtual void WriteUInt64(uint64 nVal);
	virtual void WriteUInt128(const Kademlia::CUInt128 *pVal);
	virtual void WriteHash16(const uchar* pVal);
	virtual void WriteString(const CString& rstr, EUtf8Str eEncode = utf8strNone);
	virtual void WriteString(LPCSTR psz);
	virtual void WriteLongString(const CString& rstr, EUtf8Str eEncode = utf8strNone);
	virtual void WriteLongString(LPCSTR psz);
};


///////////////////////////////////////////////////////////////////////////////
// CSafeFile

class CSafeFile : public CFile, public CFileDataIO
{
public:
	CSafeFile() {}
	CSafeFile::CSafeFile(LPCTSTR lpszFileName, UINT nOpenFlags)
		: CFile(lpszFileName, nOpenFlags) {}

	virtual UINT Read(void* lpBuf, UINT nCount);
	virtual void Write(const void* lpBuf, UINT nCount);
	virtual ULONGLONG Seek(LONGLONG lOff, UINT nFrom);
	virtual ULONGLONG GetPosition() const;
	virtual ULONGLONG GetLength() const;
};


///////////////////////////////////////////////////////////////////////////////
// CSafeMemFile

class CSafeMemFile : public CMemFile, public CFileDataIO
{
public:
	CSafeMemFile(UINT nGrowBytes = 512)
		: CMemFile(nGrowBytes) {}
	//CSafeMemFile::CSafeMemFile(BYTE* lpBuffer, UINT nBufferSize, UINT nGrowBytes = 0)
	//	: CMemFile(lpBuffer, nBufferSize, nGrowBytes) {}
	CSafeMemFile::CSafeMemFile(const BYTE* lpBuffer, UINT nBufferSize)
		: CMemFile(const_cast<BYTE*>(lpBuffer), nBufferSize, 0) {}

	const BYTE* GetBuffer() const { return m_lpBuffer; }

	virtual UINT Read(void* lpBuf, UINT nCount);
	virtual void Write(const void* lpBuf, UINT nCount);
	virtual ULONGLONG Seek(LONGLONG lOff, UINT nFrom);
	virtual ULONGLONG GetPosition() const;
	virtual ULONGLONG GetLength() const;

	virtual uint8 ReadUInt8();
	virtual uint16 ReadUInt16();
	virtual uint32 ReadUInt32();
	virtual uint64 ReadUInt64();
	virtual void ReadUInt128(Kademlia::CUInt128 *pVal);
	virtual void ReadHash16(uchar* pVal);

	virtual void WriteUInt8(uint8 nVal);
	virtual void WriteUInt16(uint16 nVal);
	virtual void WriteUInt32(uint32 nVal);
	virtual void WriteUInt64(uint64 nVal);
	virtual void WriteUInt128(const Kademlia::CUInt128 *pVal);
	virtual void WriteHash16(const uchar* pVal);
};


///////////////////////////////////////////////////////////////////////////////
// CSafeBufferedFile

class CSafeBufferedFile : public CStdioFile, public CFileDataIO
{
public:
	CSafeBufferedFile() {}
	CSafeBufferedFile::CSafeBufferedFile(LPCTSTR lpszFileName, UINT nOpenFlags)
		: CStdioFile(lpszFileName, nOpenFlags) {}

	virtual UINT Read(void* lpBuf, UINT nCount );
	virtual void Write(const void* lpBuf, UINT nCount);
	virtual ULONGLONG Seek(LONGLONG lOff, UINT nFrom);
	virtual ULONGLONG GetPosition() const;
	virtual ULONGLONG GetLength() const;

	int printf(LPCTSTR pszFmt, ...);
};

class CUnSafeBufferFile : public CSafeBufferedFile
{
public:
	CUnSafeBufferFile() {}
	CUnSafeBufferFile::CUnSafeBufferFile(LPCTSTR lpszFileName, UINT nOpenFlags)
		: CSafeBufferedFile(lpszFileName, nOpenFlags) 
	{
		m_FileIo.Open(lpszFileName, nOpenFlags);
	}
	BOOL Open(LPCTSTR lpszFileName, UINT nOpenFlags,CFileException* pException)
	{
		return this->m_FileIo.Open( lpszFileName , nOpenFlags , pException );
	}

	virtual UINT Read(void* lpBuf, UINT nCount )
	{
		return m_FileIo.Read( lpBuf , nCount );
	}

	virtual void Write(const void* lpBuf, UINT nCount)
	{
		return m_FileIo.Write( lpBuf , nCount );
	}

	virtual ULONGLONG Seek(LONGLONG lOff, UINT nFrom)
	{
		return m_FileIo.Seek( lOff , nFrom );
	}

	virtual ULONGLONG GetPosition() const
	{
		return m_FileIo.GetPosition();
	}

	virtual ULONGLONG GetLength() const
	{
		return m_FileIo.GetLength();
	}

	void Close()
	{
		return this->m_FileIo.Close();
	}

	CFile m_FileIo;
};

///////////////////////////////////////////////////////////////////////////////
// Peek - helper functions for read-accessing memory without modifying the memory pointer

__inline uint8 PeekUInt8(const void* p)
{
	return *((uint8*)p);
}

__inline uint16 PeekUInt16(const void* p)
{
	return *((uint16*)p);
}

__inline uint32 PeekUInt32(const void* p)
{
	return *((uint32*)p);
}

__inline uint64 PeekUInt64(const void* p)
{
	return *((uint64*)p);
}



///////////////////////////////////////////////////////////////////////////////
// Poke - helper functions for write-accessing memory without modifying the memory pointer

__inline void PokeUInt8(void* p, uint8 nVal)
{
	*((uint8*)p) = nVal;
}

__inline void PokeUInt16(void* p, uint16 nVal)
{
	*((uint16*)p) = nVal;
}

__inline void PokeUInt32(void* p, uint32 nVal)
{
	*((uint32*)p) = nVal;
}

__inline void PokeUInt64(void* p, uint64 nVal)
{
	*((uint64*)p) = nVal;
}


///////////////////////////////////////////////////////////////////////////////
// Array

template<class T>
class Array
{
public:
	Array(UINT nCount)
	{
		m_aT = new T[nCount];
	}
	~Array()
	{
		delete[] m_aT;
	}

	operator T* ()
	{
		return m_aT;
	}

protected:
	T* m_aT;
};
