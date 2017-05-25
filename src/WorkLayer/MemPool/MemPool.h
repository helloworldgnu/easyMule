/*
 * $Id: MemPool.h 4483 2008-01-02 09:19:06Z soarchin $
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
#ifndef _MEMORY_POOL
#define _MEMORY_POOL

namespace MemPool
{
	typedef BYTE TByte;

	typedef struct MemoryBlock
	{
		TByte*       pData;			// Pointer to the actual Data
		unsigned int DataSize;	    // Size of the Data Block
	}
	MemoryBlock, *PMemoryBlock;

	class CMemPool
	{
	public:
        // the default unit size is 11K which is bigger than most packet size
#ifdef _DEBUG
		CMemPool(int code, unsigned int poolsize = 1024 * 1024 * 5, unsigned int unitsize = 1024 * 11); // Constructor
#else
		CMemPool(unsigned int poolsize = 1024 * 1024 * 5, unsigned int unitsize = 1024 * 11); // Constructor
#endif
		
		virtual ~CMemPool(); // Destructor
	
		// Attribute
	public:

		//TByte*       GetMemPoolEntry() const;
		unsigned int GetMemPoolSize() const;
		unsigned int GetCurUsedSize() const;
		bool         IsFree() const; 

#ifdef _DEBUG
		int      m_nCode;
#endif

		// Operation
	public:

		virtual TByte* GetMemory(unsigned int size);      // Get Memory from the Memory Pool
		virtual void   FreeMemory();                      // Free Memory to Memory Pool

	protected:

		virtual void         InitMemPool(); // Init the Memory Pool
		virtual void         ClearMemPool();
		virtual PMemoryBlock RequestBlock(unsigned int blocksize);
		virtual unsigned int CalcBlockSize(unsigned int blocksize);

	private:

		CTypedPtrList<CPtrList, MemoryBlock*> m_MemoryUnitList;     //
		POSITION     m_CurUnitPos;       // The Current Position of the Memory Unit List
		TByte*       m_pPoolEntry;       // The Pool Entry

		unsigned int m_nBlockUnitSize;   // Unit block size
		unsigned int m_nPoolSize;        // The Memory Pool size

		unsigned int m_nCurUsedSize;     // Current Used Size

#ifdef _THREAD_SAFE
		CRITICAL_SECTION m_CriticalSection;
#endif
	};
};

#endif  // _MEMORY_POOL
