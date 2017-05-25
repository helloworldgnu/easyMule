/*
 * $Id: MemPool.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
#include "MemPool.h"

namespace MemPool
{
#ifdef _DEBUG
	CMemPool::CMemPool(int code, unsigned int poolsize, unsigned int unitsize)// Constructor
#else
	CMemPool::CMemPool(unsigned int poolsize, unsigned int unitsize)
#endif	
	{
#ifdef _DEBUG
		m_nCode = code;
#endif
		m_nPoolSize      = poolsize;
		m_nBlockUnitSize = unitsize;
		m_nCurUsedSize   = 0;

		InitMemPool(); // Init the Memory Pool

#ifdef _THREAD_SAFE
		InitializeCriticalSection(&m_CriticalSection);
#endif
	}

	CMemPool::~CMemPool()
	{
		ClearMemPool(); // Clear the node in list

		if (m_pPoolEntry)
		{
			delete[] m_pPoolEntry; // Free the memory
			m_pPoolEntry = NULL; // Free the Pointer
		}

#ifdef _THREAD_SAFE
		DeleteCriticalSection(&m_CriticalSection);
#endif

	}

	TByte* CMemPool::GetMemory(unsigned int size)
	{
		ASSERT(size > 0);
#ifdef _THREAD_SAFE
		EnterCriticalSection(&m_CriticalSection);
#endif
		PMemoryBlock pMemoryBlock = RequestBlock(size);

#ifdef _THREAD_SAFE
		LeaveCriticalSection(&m_CriticalSection);
#endif
		if (pMemoryBlock)
		{
			return pMemoryBlock->pData;
		}
		else
		{
			return NULL;
		}
	}

	PMemoryBlock CMemPool::RequestBlock(unsigned int blocksize)
	{
		// Request from m_MemoryUnitList for Memory Block
		if (m_MemoryUnitList.GetAt(m_CurUnitPos)->DataSize >= blocksize)
		{
			PMemoryBlock pMemoryBlock = m_MemoryUnitList.GetAt(m_CurUnitPos);
			unsigned int unitsize  = CalcBlockSize(blocksize);

			POSITION pos = m_CurUnitPos;
			for (unsigned int i = 0; i < unitsize; i++)
			{
				if (m_CurUnitPos == m_MemoryUnitList.GetTailPosition ())
				{
					m_CurUnitPos = pos; // This value scroll back
					return NULL;
				}

				m_MemoryUnitList.GetNext(m_CurUnitPos);
			}

			m_nCurUsedSize += unitsize * m_nBlockUnitSize; // Increase the value of m_nCurUsedSize

			return pMemoryBlock;
		}

		// return NULL if Current Memory Size can not satisfy the Request
		return NULL;
	}

	void CMemPool::FreeMemory()
	{
#ifdef _THREAD_SAFE
		EnterCriticalSection(&m_CriticalSection);
#endif

		m_CurUnitPos = m_MemoryUnitList.GetHeadPosition();

#ifdef _THREAD_SAFE
		LeaveCriticalSection(&m_CriticalSection);
#endif

		m_nCurUsedSize = 0; // Reset the value of m_nCurUsedSize
	}

	unsigned int CMemPool::CalcBlockSize(unsigned int blocksize)
	{
		unsigned int size = 0;

		size = blocksize / m_nBlockUnitSize;
		if (blocksize % m_nBlockUnitSize)
		{
			size++;
		}

		return size;
	}

	void CMemPool::InitMemPool()
	{
		try
		{
			unsigned int blocksize = CalcBlockSize(m_nPoolSize);
			PMemoryBlock pMemBlock = new MemoryBlock[blocksize];

			for (unsigned int i = 0; i < blocksize; i++)
			{
				m_MemoryUnitList.AddTail(&pMemBlock[i]);
			}

			m_pPoolEntry = (TByte*) new TByte[m_nPoolSize];
			POSITION pos2, pos1 = m_MemoryUnitList.GetHeadPosition();

			for (unsigned int i = 0; (pos2 = pos1) != NULL; i++)
			{
				m_MemoryUnitList.GetNext(pos1);
				m_MemoryUnitList.GetAt(pos2)->pData      = (TByte*)(m_pPoolEntry + i * m_nBlockUnitSize);
				m_MemoryUnitList.GetAt(pos2)->DataSize   = m_nPoolSize - i * m_nBlockUnitSize;
			}
		}
		catch (...)
		{
			// TODO : Catch the Exception of Memory allocation
		}

		m_CurUnitPos = m_MemoryUnitList.GetHeadPosition();
	}

	void CMemPool::ClearMemPool()
	{
		delete[] m_MemoryUnitList.GetHead();
		m_MemoryUnitList.RemoveAll();
	}

	//TByte* CMemPool::GetMemPoolEntry() const
	//{
	//	return m_pPoolEntry;
	//}

	unsigned int CMemPool::GetMemPoolSize() const
	{
		return m_nPoolSize;
	}

	unsigned int CMemPool::GetCurUsedSize() const
	{
	    return m_nCurUsedSize;
	}

	bool CMemPool::IsFree() const
	{
	    return m_CurUnitPos == m_MemoryUnitList.GetHeadPosition();
	}
};
