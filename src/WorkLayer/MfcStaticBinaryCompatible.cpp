/* 
 * $Id: MfcStaticBinaryCompatible.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

// ****************************************************************************
// IMPORTANT NOTE !!!
//
// The reason for this file and the according compiler options is to solve the 
// binary compatibility problem with the (static) version of the MFC library 
// which shows up due to the defines which are specified in 'stdafx.h' of the
// eMule project.
// 
// The MFC library of VS .NET 2002 is compiled with the following SDK defines:
// 
// WINVER         = 0x0501
// _WIN32_WINNT   = 0x0501
// _WIN32_WINDOWS = 0x0410
// _WIN32_IE      = 0x0560
// 
// eMule is using different defines (see 'stdafx.h') to achieve backward 
// compatibility with older Windows systems.
// 
// Depending on the code which is taken from the MFC library or from eMule object 
// files at linking time, it may occur that we get different structure size, 
// structure member variable offsets and other ugly things than expected and used 
// within the MFC library or vice versa.
// 
// To solve this issue, some code parts (e.g. the MFC template for allocating the 
// AFX_MODULE_THREAD_STATE structure) HAVE to be compiled with the same defines 
// as used by the MFC library compilation.
// 
// This means, that this CPP file does NOT HAVE to be compiled with pre-compiled 
// headers and (more important) is NOT ALLOWED to include 'stdafx.h' from the 
// eMule project. It has to be ensured in every aspect, that this CPP file is 
// compiled with the very same compiler settings as used for the MFC library!
// ****************************************************************************
//#include "stdafx.h"	// please read the comment above!!


#include <afxwin.h>
#include <afxpriv.h>
#include <afxstat_.h>
#include <..\src\mfc\winhand_.h>

// NOTE: Although the function is currently not used any longer, the source file 
// is kept because of the above comment. Hopefully we will never need that file
// again.
/*void Mfc_IdleUpdateCmdUiTopLevelFrameList(CWnd* pMainFrame)
{
#ifdef _AFXDLL
	// Can't link this in a Release build, see KB Q316312
	AFX_MODULE_THREAD_STATE* pState = AfxGetAppModuleState()->m_thread;
#else
	AFX_MODULE_THREAD_STATE* pState = _AFX_CMDTARGET_GETSTATE()->m_thread;
#endif
	CFrameWnd* pFrameWnd = pState->m_frameList;
	while (pFrameWnd != NULL)
	{
		if (pFrameWnd->m_hWnd != NULL && pFrameWnd != pMainFrame)
		{
			if (pFrameWnd->m_nShowDelay == SW_HIDE)
				pFrameWnd->ShowWindow(pFrameWnd->m_nShowDelay);
			if (pFrameWnd->IsWindowVisible() || pFrameWnd->m_nShowDelay >= 0)
			{
				AfxCallWndProc(pFrameWnd, pFrameWnd->m_hWnd, WM_IDLEUPDATECMDUI, (WPARAM)TRUE, 0);
				pFrameWnd->SendMessageToDescendants(WM_IDLEUPDATECMDUI, (WPARAM)TRUE, 0, TRUE, TRUE);
			}
			if (pFrameWnd->m_nShowDelay > SW_HIDE)
				pFrameWnd->ShowWindow(pFrameWnd->m_nShowDelay);
			pFrameWnd->m_nShowDelay = -1;
		}
		pFrameWnd = pFrameWnd->m_pNextFrameWnd;
	}
}*/

#ifdef _DEBUG

#pragma pack(4)
class _CHandleMap
{
private:    // implementation
	CFixedAllocNoSync m_alloc;
	void (PASCAL* m_pfnConstructObject)(CObject* pObject);
	void (PASCAL* m_pfnDestructObject)(CObject* pObject);
	CMapPtrToPtr m_permanentMap;
	CMapPtrToPtr m_temporaryMap;
	CRuntimeClass* m_pClass;
	size_t m_nOffset;       // offset of handles in the object
	int m_nHandles;         // 1 or 2 (for CDC)

// Constructor/Destructor
public:
	_CHandleMap(CRuntimeClass* pClass, 
		void (PASCAL* pfnConstructObject)(CObject* pObject),
		void (PASCAL* pfnDestructObject)(CObject* pObject),
		size_t nOffset, int nHandles = 1);
#ifdef _AFXDLL
	~_CHandleMap()
#else
	virtual ~_CHandleMap()
#endif
		{ DeleteTemp(); }

// Operations
public:
	CObject* FromHandle(HANDLE h);
	void DeleteTemp();

	void SetPermanent(HANDLE h, CObject* permOb);
	void RemoveHandle(HANDLE h);

	CObject* LookupPermanent(HANDLE h);
	CObject* LookupTemporary(HANDLE h);

	friend void Mfc_IdleFreeTempMaps();
	friend class CWinThread;
};
#pragma pack()

#endif

#ifdef _DEBUG
void Mfc_IdleFreeTempMaps()
{
#if defined(_DEBUG) && !defined(_AFX_NO_DEBUG_CRT)
	// check MFC's allocator (before idle)
	if (_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) & _CRTDBG_CHECK_ALWAYS_DF)
		ASSERT(AfxCheckMemory());
#endif

#ifdef _AFXDLL
	// Can't link this in a Release build, see KB Q316312
	AFX_MODULE_THREAD_STATE* pState = AfxGetAppModuleState()->m_thread;
#else
	AFX_MODULE_THREAD_STATE* pState = _AFX_CMDTARGET_GETSTATE()->m_thread;
#endif
	if (pState->m_nTempMapLock == 0)
	{
#ifdef _DEBUG
#define _chSTR(x)		#x
#define chSTR(x)		_chSTR(x)

#define	CHECK_MAP_STATE(map, member) \
		static int s_iOld_##map##member = 0; \
		if (!bDumpMaps && ((_CHandleMap*)(pState->map))->member.GetCount() != s_iOld_##map##member) { \
			bDumpMaps = true; \
		}

#define	DUMP_MAP_STATE(map, member) \
		TRACE(chSTR(map) "->" chSTR(member) ": %d", ((_CHandleMap*)(pState->map))->member.GetCount()); \
		if (((_CHandleMap*)(pState->map))->member.GetCount() != s_iOld_##map##member) { \
			TRACE("  (%d)", ((_CHandleMap*)(pState->map))->member.GetCount() - s_iOld_##map##member); \
			s_iOld_##map##member = ((_CHandleMap*)(pState->map))->member.GetCount(); \
		} \
		TRACE("\n");

		bool bDumpMaps = false;
		CHECK_MAP_STATE(m_pmapHWND, m_permanentMap);
		CHECK_MAP_STATE(m_pmapHWND, m_temporaryMap);

		CHECK_MAP_STATE(m_pmapHMENU, m_permanentMap);
		CHECK_MAP_STATE(m_pmapHMENU, m_temporaryMap);

		CHECK_MAP_STATE(m_pmapHDC, m_permanentMap);
		CHECK_MAP_STATE(m_pmapHDC, m_temporaryMap);

		CHECK_MAP_STATE(m_pmapHGDIOBJ, m_permanentMap);
		CHECK_MAP_STATE(m_pmapHGDIOBJ, m_temporaryMap);

		CHECK_MAP_STATE(m_pmapHIMAGELIST, m_permanentMap);
		CHECK_MAP_STATE(m_pmapHIMAGELIST, m_temporaryMap);

		if (bDumpMaps)
		{
			TRACE("---Dump start\n");
			DUMP_MAP_STATE(m_pmapHWND, m_permanentMap);
			DUMP_MAP_STATE(m_pmapHWND, m_temporaryMap);

			DUMP_MAP_STATE(m_pmapHMENU, m_permanentMap);
			DUMP_MAP_STATE(m_pmapHMENU, m_temporaryMap);

			DUMP_MAP_STATE(m_pmapHDC, m_permanentMap);
			DUMP_MAP_STATE(m_pmapHDC, m_temporaryMap);

			DUMP_MAP_STATE(m_pmapHGDIOBJ, m_permanentMap);
			DUMP_MAP_STATE(m_pmapHGDIOBJ, m_temporaryMap);

			DUMP_MAP_STATE(m_pmapHIMAGELIST, m_permanentMap);
			DUMP_MAP_STATE(m_pmapHIMAGELIST, m_temporaryMap);
		}


#define SAVE_MAP_STATE(map, member) \
		((_CHandleMap*)(pState->map))->member.AssertValid(); \
		int iOld_##map##member = ((_CHandleMap*)(pState->map))->member.GetCount();

		SAVE_MAP_STATE(m_pmapHWND, m_permanentMap);
		SAVE_MAP_STATE(m_pmapHWND, m_temporaryMap);

		SAVE_MAP_STATE(m_pmapHMENU, m_permanentMap);
		SAVE_MAP_STATE(m_pmapHMENU, m_temporaryMap);

		SAVE_MAP_STATE(m_pmapHDC, m_permanentMap);
		SAVE_MAP_STATE(m_pmapHDC, m_temporaryMap);

		SAVE_MAP_STATE(m_pmapHGDIOBJ, m_permanentMap);
		SAVE_MAP_STATE(m_pmapHGDIOBJ, m_temporaryMap);

		SAVE_MAP_STATE(m_pmapHIMAGELIST, m_permanentMap);
		SAVE_MAP_STATE(m_pmapHIMAGELIST, m_temporaryMap);
#endif

		// free temp maps, OLE DLLs, etc.
		//AfxLockTempMaps();
		//AfxUnlockTempMaps();

#ifdef _DEBUG
#define CMP_MAP_STATE(map, member) \
		int iNew_##map##member = ((_CHandleMap*)(pState->map))->member.GetCount(); \
		if (iNew_##map##member != iOld_##map##member) \
			TRACE(chSTR(map) "->" chSTR(member) ": %d\n", iNew_##map##member - iOld_##map##member);

		CMP_MAP_STATE(m_pmapHWND, m_permanentMap);
		CMP_MAP_STATE(m_pmapHWND, m_temporaryMap);

		CMP_MAP_STATE(m_pmapHMENU, m_permanentMap);
		CMP_MAP_STATE(m_pmapHMENU, m_temporaryMap);

		CMP_MAP_STATE(m_pmapHDC, m_permanentMap);
		CMP_MAP_STATE(m_pmapHDC, m_temporaryMap);

		CMP_MAP_STATE(m_pmapHGDIOBJ, m_permanentMap);
		CMP_MAP_STATE(m_pmapHGDIOBJ, m_temporaryMap);

		CMP_MAP_STATE(m_pmapHIMAGELIST, m_permanentMap);
		CMP_MAP_STATE(m_pmapHIMAGELIST, m_temporaryMap);
#endif
	}

#if defined(_DEBUG) && !defined(_AFX_NO_DEBUG_CRT)
	// check MFC's allocator (after idle)
	if (_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) & _CRTDBG_CHECK_ALWAYS_DF)
		ASSERT(AfxCheckMemory());
#endif
}
#endif
