/*
 * $Id: stdafx.h 4783 2008-02-02 08:17:12Z soarchin $
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
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

#ifndef WINVER
#define WINVER 0x0601			// 0x0400 == Windows 98 and Windows NT 4.0 (because of '_WIN32_WINDOWS=0x0410')
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601	// 0x0400 == Windows NT 4.0
#endif						

#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0601   // 0x0410 == Windows 98
#endif

#ifndef _WIN32_IE
//#define _WIN32_IE 0x0400		// 0x0400 == Internet Explorer 4.0 -> Comctl32.dll v4.71
#define _WIN32_IE 0x0600		// 0x0560 == Internet Explorer 5.6 -> Comctl32.dll v5.8 (same as MFC internally used)
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

#define _ATL_ALL_WARNINGS
#define _AFX_ALL_WARNINGS
// Disable some warnings which get fired with /W4 for Windows/MFC/ATL headers
#pragma warning(disable:4127) // conditional expression is constant
#pragma warning(disable:4100) 

#if _MSC_VER>=1400
#define _SECURE_ATL	0		  //TODO: resolve
#if !_SECURE_ATL
#pragma warning(disable:4996) // 'foo' was declared deprecated
#endif
#ifndef _USE_32BIT_TIME_T
#define _USE_32BIT_TIME_T
#endif
#endif

#ifdef _DEBUG
#define	_ATL_DEBUG
#define _ATL_DEBUG_QI
#endif

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC IDispatch & ClassFactory support
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT
#include <afxole.h>			// MFC OLE support

#include <winsock2.h>
#define _WINSOCKAPI_
#include <afxsock.h>		// MFC support for Windows Sockets
#include <afxdhtml.h>

#include <afxmt.h>			// MFC Multithreaded Extensions (Syncronization Objects)
#include <afxdlgs.h>		// MFC Standard dialogs
#include <atlcoll.h>
#include <afxcoll.h>
#include <afxtempl.h>
#include <math.h>


#ifndef EWX_FORCEIFHUNG
#define EWX_FORCEIFHUNG			0x00000010
#endif

#ifndef WS_EX_LAYOUTRTL
#define WS_EX_LAYOUTRTL         0x00400000L // Right to left mirroring
#endif

#ifndef LAYOUT_RTL
#define LAYOUT_RTL				0x00000001 // Right to left
#endif

#ifndef COLOR_HOTLIGHT
#define COLOR_HOTLIGHT			26
#endif

#ifndef WS_EX_LAYERED
#define WS_EX_LAYERED           0x00080000
#endif

#ifndef LWA_COLORKEY
#define LWA_COLORKEY            0x00000001
#endif

#ifndef LWA_ALPHA
#define LWA_ALPHA               0x00000002
#endif

#ifndef HDF_SORTUP
#define HDF_SORTUP              0x0400
#endif

#ifndef HDF_SORTDOWN
#define HDF_SORTDOWN            0x0200
#endif

#ifndef COLOR_GRADIENTACTIVECAPTION
#define COLOR_GRADIENTACTIVECAPTION 27
#endif



// when using warning level 4
#pragma warning(disable:4201) // nonstandard extension used : nameless struct/union (not worth to mess with, it's due to MIDL created code)
#pragma warning(disable:4238) // nonstandard extension used : class rvalue used as lvalue
#pragma warning(disable:4706)
#if _MSC_VER>=1400
#pragma warning(disable:4996) // '_swprintf' was declared deprecated
#pragma warning(disable:4127) // conditional expression is constant
#endif

#include <afxhtml.h> //Added by thilon on 2006.09.23, for CHtmlCtrl
#include <afx.h>

#include "types.h"

#define ARRSIZE(x)	(sizeof(x)/sizeof(x[0]))

#ifdef _DEBUG
#define malloc(s)         _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define calloc(c, s)      _calloc_dbg(c, s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define realloc(p, s)     _realloc_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define _expand(p, s)     _expand_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define free(p)           _free_dbg(p, _NORMAL_BLOCK)
#define _msize(p)         _msize_dbg(p, _NORMAL_BLOCK)
#endif

typedef	CArray<CStringA> CStringAArray;
typedef	CStringArray CStringWArray;

#define _TWINAPI(fname)	fname "W"

#ifndef _countof
#define _countof(array) (sizeof(array)/sizeof(array[0]))
#endif

extern "C" int __cdecl __ascii_stricmp(const char * dst, const char * src);

#define CMemDC XCMemDC

