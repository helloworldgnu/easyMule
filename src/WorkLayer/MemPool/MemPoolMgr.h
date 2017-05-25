/*
 * $Id: MemPoolMgr.h 4483 2008-01-02 09:19:06Z soarchin $
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
#ifndef _MEMORY_POOLMGR
#define _MEMORY_POOLMGR

#include "MemPool.h"
#include "../PartFile.h"

namespace MemPool
{
	const unsigned int DEFAULT_POOLSIZE      = 1024 * 1024 * 10;
	const unsigned int DEFAULT_POOLUNITSIZE  = 1024 * 1024;
	const unsigned int DEFAULT_BLOCKUNITSIZE = 1024 * 11; // the default unit size is 11K which is bigger than most packet size

	class CMemPoolMgr;

	class CMemoryUnit
	{
	public:

		CMemoryUnit(CMemPoolMgr *pMemPoolMgr, CPartFile * partFile);
		virtual ~CMemoryUnit();

		virtual TByte* GetMemory(unsigned int size);
		unsigned int   FreeAllMemPool();
		unsigned int   GetCurUsedSize();
		CPartFile *    GetPartFile() const;
		void           AddTail(CMemPool * pMemPool);

#ifdef  _DEBUG
		void           ProcessLog();
#endif

	private:
		CMemPoolMgr * m_pMemPoolMgr;
		CPartFile   * m_pPartFile;                     // Pointer to CPartFile
		CTypedPtrList<CPtrList, CMemPool*> m_PoolList; // Memory Pool List
	};

	class CMemPoolMgr
	{
	public:
        // Constructor and Destructor
		CMemPoolMgr(unsigned int poolsize = DEFAULT_POOLSIZE,
			unsigned int poolunitsize     = DEFAULT_POOLUNITSIZE,
			unsigned int blockunitsize    = DEFAULT_BLOCKUNITSIZE);
		virtual ~CMemPoolMgr();

		// Attribute
	public:
		unsigned int   GetCurUsedSize(CPartFile* pPartFile) const;
		unsigned int   GetMemPoolUnitSize() const;

		// Operation
	public:
		virtual TByte* GetMemory(CPartFile* pPartFile, unsigned int size);     // Get Memory from the Memory Pool
		virtual void   FreeMemory(CPartFile* pPartFile);                       // Free Memory to Memory Pool
		virtual void   CheckToRelease();                                       // Release the Memory Pool Dynamic

#ifdef _DEBUG
		void           ProcessLog();
#endif
        
	protected:
        
		virtual CMemPool* GetMemPool(CMemoryUnit *pMemoryUnit, int poolsize);
		virtual void      ClearMemPoolMgr();

	private:

		CTypedPtrList<CPtrList, CMemoryUnit*> m_MemoryUnitList; // Memory Unit List
		CTypedPtrList<CPtrList, CMemPool*>    m_PoolList;       // Memory Pool List

		unsigned int m_nPoolSize;      // The total size of the Memory Pools
		unsigned int m_nPoolUnitSize;  // The Unit Size of Memory Pool
		unsigned int m_nBlockUnitSize; // The Memory Block Unit size of Each Memory Pool

		unsigned int m_nTotalMemPool;  // The Total Number of MemPool
		unsigned int m_nUsingMemPool;  // The Number of Using MemPool

		DWORD        m_TimeTick;       // The begining time of Check to Release 

#ifdef _DEBUG
		int          m_nCode;
#endif
	};
};

#endif // _MEMORY_POOLMGR
