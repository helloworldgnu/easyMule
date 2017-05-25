/*
 * $Id: Tools.h 4483 2008-01-02 09:19:06Z soarchin $
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
// Tools.h : header file
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

///////////////////////////////////////////////////////////////////////////////
// Usefull macros
//
#define KEYDOWN(Key) ((GetKeyState(Key)&0x8000)!=0)
#define ON_WM_MOUSEOUT() \
    { WM_MOUSELEAVE, 0, 0, 0, AfxSig_vv, \
        (AFX_PMSG)(AFX_PMSGW)(void (AFX_MSG_CALL CWnd::*)(void))&OnMouseOut },

#ifndef lengthof
  #define lengthof(a) (sizeof(a)/sizeof(a[0]))
#endif

///////////////////////////////////////////////////////////////////////////////
// Check if the specified window is child of a docked toolbar
bool ChildOfDockedToolbar (CWnd* pWnd);

/////////////////////////////////////////////////////////////////////////////
// Constants for detecting OS-Type
enum WinVer
{
    wvUndefined,
    wvWin32s,
    wvWin95,
    wvWin98,
    wvWinME,
    wvWinNT3,
    wvWinNT4,
    wvWin2000,
    wvWinXP,
};

///////////////////////////////////////////////////////////////////////////////
// Return the current OS-Type
//
WinVer WINAPI GetWinVersion ();

///////////////////////////////////////////////////////////////////////////////
inline void WINAPI ScreenToClient (HWND hWnd, LPRECT pRect)
{
    ScreenToClient (hWnd, (LPPOINT)pRect);
    ScreenToClient (hWnd, ((LPPOINT)pRect)+1);
}

///////////////////////////////////////////////////////////////////////////////
inline void WINAPI ClientToScreen (HWND hWnd, LPRECT pRect)
{
    ClientToScreen (hWnd, (LPPOINT)pRect);
    ClientToScreen (hWnd, ((LPPOINT)pRect)+1);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class CClientRect : public CRect
{
public:
    CClientRect (HWND hWnd)
    {
        ::GetClientRect (hWnd, this);
    };
    CClientRect (const CWnd* pWnd)
    {
        ::GetClientRect (pWnd->GetSafeHwnd(), this);
    };
};


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class CWindowRect : public CRect
{
public:
    CWindowRect (HWND hWnd)
    {
        ::GetWindowRect (hWnd, this);
    };
    CWindowRect (const CWnd* pWnd)
    {
        ::GetWindowRect (pWnd->GetSafeHwnd(), this);
    };
};


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class CWindowText : public CString
{
public:
    CWindowText (HWND hWnd)
    {
        CWnd::FromHandle (hWnd)->GetWindowText (*this);
    };
    CWindowText (const CWnd* pWnd)
    {
        pWnd->GetWindowText (*this);
    };
};


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#define MMS_PAINT   0x0001
#define MMS_NCPAINT 0x0002

///////////////////
class CMouseMgr
{
public:
    CMouseMgr ();

    void Init (HWND hWnd, WORD wFlags = MMS_PAINT);
    bool MouseOver () const;
    bool OnMouseMove (HWND hTrack = NULL);
    bool OnMouseOut (HWND hTrack = NULL);

protected:
    HWND m_hWnd;
    HWND m_hTrack;
    WORD m_wFlags;
    bool m_bOver;
};
