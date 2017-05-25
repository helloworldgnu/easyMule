/*
 * $Id: MemPoolMgr.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
#include "MemPoolMgr.h"

namespace MemPool
{
	CMemPoolMgr::CMemPoolMgr(unsigned int poolsize         /* = DEFAULT_POOLSIZE */,
		unsigned int poolunitsize     /* = DEFAULT_POOLUNITSIZE */,
		unsigned int blockunitsize    /* = DEFAULT_BLOCKUNITSIZE */)
	{
		m_nPoolSize         = poolsize;
		m_nPoolUnitSize     = poolunitsize;
		m_nBlockUnitSize    = blockunitsize;

		m_nTotalMemPool     = 0;
		m_nUsingMemPool     = 0;

		m_TimeTick          = GetTickCount();

#ifdef _DEBUG
		m_nCode             = 0;
#endif
	}

	CMemPoolMgr::~CMemPoolMgr()
	{
		ClearMemPoolMgr();
	}
	
#ifdef _DEBUG
	void CMemPoolMgr::ProcessLog()
	{
		POSITION pos2, pos1 = m_MemoryUnitList.GetHeadPosition();

		for (; (pos2 = pos1) != NULL;)
		{
			m_MemoryUnitList.GetNext(pos1);
            m_MemoryUnitList.GetAt(pos2)->ProcessLog();
		}
	}
#endif

	void CMemPoolMgr::ClearMemPoolMgr()
	{
		POSITION pos2, pos1 = m_PoolList.GetHeadPosition();

		for (; (pos2 = pos1) != NULL;)
		{
			m_PoolList.GetNext(pos1);
			delete m_PoolList.GetAt(pos2);
			m_PoolList.RemoveAt(pos2);
		}

		////////////////////////////////////////////////////

        m_MemoryUnitList.RemoveAll();
	}

	TByte* CMemPoolMgr::GetMemory(CPartFile* pPartFile, unsigned int size)
	{
		TByte* pData;

		CMemoryUnit *pMemoryUnit = NULL;

		// First Check Whether the PartFile is in m_MemoryUnitList or not
		POSITION pos2, pos1 = m_MemoryUnitList.GetTailPosition();

		for (; (pos2 = pos1) != NULL;)
		{
			m_MemoryUnitList.GetPrev(pos1);

			if (m_MemoryUnitList.GetAt(pos2)->GetPartFile() == pPartFile)
			{
				pMemoryUnit = m_MemoryUnitList.GetAt(pos2);
				break;
			}
		}

		if (pMemoryUnit)
		{
			if ((pData = pMemoryUnit->GetMemory(size)) != NULL)
			{
				return pData;
			}
			else
			{
                CMemPool * pMemPool = GetMemPool(pMemoryUnit, m_nPoolUnitSize);
			    return pMemPool->GetMemory(size);
			}
		}
		else
		{
			pMemoryUnit = new CMemoryUnit(this, pPartFile);
			m_MemoryUnitList.AddTail(pMemoryUnit);
			CMemPool * pMemPool = GetMemPool(pMemoryUnit, m_nPoolUnitSize);
			return pMemPool->GetMemory(size);
		}
	}

	void CMemPoolMgr::FreeMemory(CPartFile* pPartFile)
	{
		CMemoryUnit *pMemoryUnit = NULL;

		// First find the MemoryUnit of the given Partfile
		POSITION pos2, pos1 = m_MemoryUnitList.GetTailPosition();

		for (; (pos2 = pos1) != NULL;)
		{
			m_MemoryUnitList.GetPrev(pos1);
			
			if (m_MemoryUnitList.GetAt(pos2)->GetPartFile() == pPartFile)
			{
				pMemoryUnit = m_MemoryUnitList.GetAt(pos2);
				break;
			}
		}
        // Free the Memory Pool and Remove them
		if (pMemoryUnit)
		{
			// Free All Memory Pool
			m_nUsingMemPool -= pMemoryUnit->FreeAllMemPool();
		}
	}

	CMemPool* CMemPoolMgr::GetMemPool(CMemoryUnit *pMemoryUnit, int poolsize)
	{
		CMemPool * pMemPool = NULL;

		POSITION pos2, pos1 = m_PoolList.GetHeadPosition();

		for (; (pos2 = pos1) != NULL;)
		{
			m_PoolList.GetNext(pos1);
			if (m_PoolList.GetAt(pos2)->IsFree())
			{
				pMemPool = m_PoolList.GetAt(pos2);
			    pMemoryUnit->AddTail(pMemPool);
				m_nUsingMemPool++;

				return pMemPool;
			}
		}

		try
		{
			pMemPool = NULL;
#ifdef _DEBUG
			pMemPool = new CMemPool(++m_nCode, poolsize, m_nBlockUnitSize);
#else
			pMemPool = new CMemPool(poolsize, m_nBlockUnitSize);
#endif
			m_PoolList.AddTail(pMemPool);
			m_nTotalMemPool++;

			pMemoryUnit->AddTail(pMemPool);
			m_nUsingMemPool++;

			return pMemPool;
		}
		catch(std::bad_alloc &)
		{
			return NULL; // Memory Allocation Error
		}
		catch(...)
		{
			// VC-linhai[2007-08-06]:对应warning C4701
			// 当 pMenPool 初始化失败，前面对 pMenPool 的初始化便
			if (pMemPool != NULL)
			{
				delete pMemPool;
				pMemPool = NULL;
			}

		    return NULL;
		}
	}

	unsigned int CMemPoolMgr::GetCurUsedSize(CPartFile* pPartFile) const
	{
		CMemoryUnit *pMemoryUnit = NULL;

		// First Check Whether the PartFile is in m_MemoryUnitList or not
		POSITION pos2, pos1 = m_MemoryUnitList.GetTailPosition();

		for (; (pos2 = pos1) != NULL;)
		{
			m_MemoryUnitList.GetPrev(pos1);

			if (m_MemoryUnitList.GetAt(pos2)->GetPartFile() == pPartFile)
			{
				pMemoryUnit = m_MemoryUnitList.GetAt(pos2);
				break;
			}
		}

		if (pMemoryUnit)
		{
			return pMemoryUnit->GetCurUsedSize();
		}
		else
		{
			return 0;
		}
	}

	unsigned int CMemPoolMgr::GetMemPoolUnitSize() const
	{
	    return m_nPoolUnitSize;
	}

	void CMemPoolMgr::CheckToRelease()
	{
		if ((m_nTotalMemPool - m_nUsingMemPool) > 5)
		{
			if ((GetTickCount() - m_TimeTick) > MIN2MS(30))
			{
				POSITION pos2, pos1 = m_PoolList.GetHeadPosition();

				for (; (pos2 = pos1) != NULL;)
				{
					m_PoolList.GetNext(pos1);
					if (m_PoolList.GetAt(pos2)->IsFree())
					{
						delete m_PoolList.GetAt(pos2);
						m_PoolList.RemoveAt(pos2);
						m_nTotalMemPool--;
					}
				}
			}
		}
		else
		{
			m_TimeTick = GetTickCount();
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////

	CMemoryUnit::CMemoryUnit(CMemPoolMgr * pMemPoolMgr, CPartFile* partFile)
	{
		m_pMemPoolMgr  = pMemPoolMgr; 
		m_pPartFile    = partFile;
	}

	CMemoryUnit:: ~CMemoryUnit()
	{
	}

	TByte* CMemoryUnit::GetMemory(unsigned int size)
	{
		TByte* pData;

		if (m_PoolList.IsEmpty())
		{
			return NULL; // The Pool List Should not be Empty
		}

		if ((pData = m_PoolList.GetTail()->GetMemory(size)) != NULL)
		{
			return pData;
		}
		else
		{
			return NULL;
		}
	}

	unsigned int CMemoryUnit::FreeAllMemPool()
	{
		unsigned int number = 0;

		POSITION pos2, pos1 = m_PoolList.GetTailPosition();

		for (; (pos2 = pos1) != NULL;)
		{
			m_PoolList.GetPrev(pos1);
			m_PoolList.GetAt(pos2)->FreeMemory();
		}		

		number = m_PoolList.GetCount();
#ifdef _DEBUG_MEMPOOL
		TRACE("%d Free Memory, PoolList Size is : %d\n", m_pPartFile, m_PoolList.GetCount());
#endif
		m_PoolList.RemoveAll();

		return number;
	}

	unsigned int CMemoryUnit::GetCurUsedSize() 
	{
		int CurUsedSize = 0;

		if (m_PoolList.GetCount() > 0)
		{
			CurUsedSize  = (m_PoolList.GetCount() - 1) * m_pMemPoolMgr->GetMemPoolUnitSize();
			CurUsedSize += m_PoolList.GetTail()->GetCurUsedSize();
			return CurUsedSize;
		}
		else 
		{
			return 0;
		}
	}

	CPartFile * CMemoryUnit::GetPartFile() const
	{
		return m_pPartFile;
	}

	void CMemoryUnit::AddTail(CMemPool * pMemPool)
	{
	    m_PoolList.AddTail(pMemPool);
	}

#ifdef _DEBUG
	void CMemoryUnit::ProcessLog()
	{
		POSITION pos2, pos1 = m_PoolList.GetHeadPosition();

		TRACE("PartFile %d : MemPool Code : ", m_pPartFile);

		for (; (pos2 = pos1) != NULL;)
		{
			m_PoolList.GetNext(pos1);
			TRACE(" %d", m_PoolList.GetAt(pos2)->m_nCode);
		}
	
		TRACE("\n");
	}
#endif
};
