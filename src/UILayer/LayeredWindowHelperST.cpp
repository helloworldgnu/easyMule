/*
 * $Id: LayeredWindowHelperST.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
#include "LayeredWindowHelperST.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CLayeredWindowHelperST::CLayeredWindowHelperST()
{
	// Load DLL.
	m_hDll = ::LoadLibrary(_T("USER32.dll"));
}

CLayeredWindowHelperST::~CLayeredWindowHelperST()
{
	// Unload DLL (if any)
	if (m_hDll)	::FreeLibrary(m_hDll);
	m_hDll = NULL;
}

// This function adds the WS_EX_LAYERED style to the specified window.
//
// Parameters:
//		[IN]	Handle to the window and, indirectly, the class to which the window belongs. 
//				Windows 95/98/Me: The SetWindowLong function may fail if the window 
//				specified by the hWnd parameter does not belong to the same process 
//				as the calling thread. 
//
// Return value:
//		Non zero
//			Function executed successfully.
//		Zero
//			Function failed. To get extended error information, call ::GetLastError().
//
LONG CLayeredWindowHelperST::AddLayeredStyle(HWND hWnd)
{
	return ::SetWindowLong(hWnd, GWL_EXSTYLE, ::GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
} // End of AddLayeredStyle

// This function removes the WS_EX_LAYERED style from the specified window.
//
// Parameters:
//		[IN]	Handle to the window and, indirectly, the class to which the window belongs. 
//				Windows 95/98/Me: The SetWindowLong function may fail if the window 
//				specified by the hWnd parameter does not belong to the same process 
//				as the calling thread. 
//
// Return value:
//		Non zero
//			Function executed successfully.
//		Zero
//			Function failed. To get extended error information, call ::GetLastError().
//
LONG CLayeredWindowHelperST::RemoveLayeredStyle(HWND hWnd)
{
	return ::SetWindowLong(hWnd, GWL_EXSTYLE, ::GetWindowLong(hWnd, GWL_EXSTYLE) & ~WS_EX_LAYERED);
} // End of RemoveLayeredStyle

// This function sets the opacity and transparency color key of a layered window.
//
// Parameters:
//		[IN]	hWnd
//				Handle to the layered window.
//		[IN]	crKey
//				A COLORREF value that specifies the transparency color key to be used when
//				composing the layered window. All pixels painted by the window in this color will be transparent.
//				To generate a COLORREF, use the RGB() macro.
//		[IN]	bAlpha
//				Alpha value used to describe the opacity of the layered window.
//				When bAlpha is 0, the window is completely transparent.
//				When bAlpha is 255, the window is opaque. 
//		[IN]	dwFlags 
//				Specifies an action to take. This parameter can be one or more of the following values:
//					LWA_COLORKEY	Use crKey as the transparency color.  
//					LWA_ALPHA		Use bAlpha to determine the opacity of the layered window.
//
// Return value:
//		TRUE
//			Function executed successfully.
//		FALSE
//			Function failed. To get extended error information, call ::GetLastError().
//
BOOL CLayeredWindowHelperST::SetLayeredWindowAttributes(HWND hWnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags)
{
	BOOL	bRetValue = TRUE;

	if (m_hDll)
	{
		lpfnSetLayeredWindowAttributes pFn = NULL;
		pFn = (lpfnSetLayeredWindowAttributes)GetProcAddress(m_hDll, "SetLayeredWindowAttributes");
		if (pFn)
		{
			bRetValue = pFn(hWnd, crKey, bAlpha, dwFlags);
		} // if
		else bRetValue = FALSE;
	} // if

	return bRetValue;
} // End of SetLayeredWindowAttributes

// This function sets the percentage of opacity or transparency of a layered window.
//
// Parameters:
//		[IN]	hWnd
//				Handle to the layered window.
//		[IN]	byPercentage
//				Percentage (from 0 to 100)
//
// Return value:
//		TRUE
//			Function executed successfully.
//		FALSE
//			Function failed. To get extended error information, call ::GetLastError().
//
BOOL CLayeredWindowHelperST::SetTransparentPercentage(HWND hWnd, UINT byPercentage)
{
	// Do not accept values greater than 100%
	if (byPercentage > 100)	byPercentage = 100;

	return SetLayeredWindowAttributes(hWnd, 0, (BYTE)(255 * byPercentage/100), LWA_ALPHA);
} // End of SetTransparentPercentage
