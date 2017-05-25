/*
 * $Id: Tools.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
///////////////////////////////////////////////////////////////////////////////
//
// Tools.cpp : implementation file
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Tools.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////
// Check if the specified window is child of a docked toolbar
bool ChildOfDockedToolbar (CWnd* pWnd)
{
    CWnd* pParent = pWnd->GetParent();
    
    return pParent->IsKindOf (RUNTIME_CLASS(CControlBar)) &&
           ((CControlBar*)pParent)->m_pDockBar != NULL;
}

///////////////////////////////////////////////////////////////////////////////
// Return the current system version
//
WinVer WINAPI GetWinVersion ()
{
    static WinVer s_wvVal = wvUndefined;

    if ( s_wvVal != wvUndefined )
    {
        return s_wvVal;
    }
    OSVERSIONINFO osvi;

    ZeroMemory (&osvi, sizeof OSVERSIONINFO);
    osvi.dwOSVersionInfoSize = sizeof OSVERSIONINFO;

    if ( !GetVersionEx (&osvi) )
    {
        return s_wvVal = wvUndefined;
    }
    if ( osvi.dwPlatformId == VER_PLATFORM_WIN32s )
    {
        return s_wvVal = wvWin32s;
    }
    if ( osvi.dwPlatformId == VER_PLATFORM_WIN32_NT )
    {
        if ( osvi.dwMajorVersion == 4L )
        {
            return s_wvVal = wvWinNT4;
        }
        if ( osvi.dwMajorVersion == 5L && osvi.dwMinorVersion == 0L )
        {
            return s_wvVal = wvWin2000;
        }
        if ( osvi.dwMajorVersion == 5L && osvi.dwMinorVersion == 1L )
        {
            return s_wvVal = wvWinXP;
        }
        return s_wvVal = wvWinNT3;
    }
    ASSERT(osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS);

    if ( osvi.dwMajorVersion == 4L && osvi.dwMinorVersion == 10L )
    {
        return s_wvVal = wvWin98;
    }
    if ( osvi.dwMajorVersion == 4L && osvi.dwMinorVersion == 90L )
    {
        return s_wvVal = wvWinME;
    }
    return s_wvVal = wvWin95;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
CMouseMgr::CMouseMgr () : m_bOver (false)
{
}

///////////////////////////////////////////////////////////////////////////////
void CMouseMgr::Init (HWND hWnd, WORD wFlags)
{
    m_hWnd = hWnd;
    m_wFlags = wFlags;
}

///////////////////////////////////////////////////////////////////////////////
bool CMouseMgr::MouseOver () const
{
    return m_bOver;
}

extern "C" WINUSERAPI BOOL WINAPI TrackMouseEvent (LPTRACKMOUSEEVENT lpEventTrack);
///////////////////////////////////////////////////////////////////////////////
bool CMouseMgr::OnMouseMove (HWND hTrack)
{
    ASSERT(m_hWnd != NULL);
    if ( hTrack == NULL ) hTrack = m_hWnd;

    if ( !m_bOver )
    {
        m_bOver = true;

        if ( m_wFlags & MMS_PAINT )
        {
            ::InvalidateRect (m_hWnd, NULL, false);
        }
        if ( m_wFlags & MMS_NCPAINT )
        {
            ::SetWindowPos (m_hWnd, NULL, 0, 0, 0, 0, SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);
        }
        // Prepare for tracking MouseOut
        TRACKMOUSEEVENT tme = { sizeof TRACKMOUSEEVENT, TME_LEAVE, m_hTrack = hTrack, 0 };
        ::TrackMouseEvent (&tme);

        return true;
    }
    if ( hTrack != m_hTrack )
    {
        // Cancel tracking MouseOut for the previous window (main or child)
        TRACKMOUSEEVENT tme = { sizeof TRACKMOUSEEVENT, TME_CANCEL, m_hTrack, 0 };
        ::TrackMouseEvent (&tme);

        // Track MouseOut
        tme.dwFlags = TME_LEAVE;
        tme.hwndTrack = m_hTrack = hTrack;
        ::TrackMouseEvent (&tme);
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////
bool CMouseMgr::OnMouseOut (HWND hTrack)
{
    ASSERT(m_hWnd != NULL);
    if ( hTrack == NULL ) hTrack = m_hWnd;

    if ( hTrack != m_hTrack )
    {
        return false;
    }
    m_bOver = false;

    if ( m_wFlags & MMS_PAINT )
    {
        ::InvalidateRect (m_hWnd, NULL, false);
    }
    if ( m_wFlags & MMS_NCPAINT )
    {
        ::SetWindowPos (m_hWnd, NULL, 0, 0, 0, 0, SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);
    }
    return true;
}
