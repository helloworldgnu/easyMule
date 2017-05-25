/*
 * $Id: MeterIcon.h 4483 2008-01-02 09:19:06Z soarchin $
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
// MeterIcon.h: interface for the CMeterIcon class.
//
// Created: 04/02/2001 {mm/dm/yyyyy}
// Written by: Anish Mistry http://am-productions.yi.org/
/* This code is licensed under the GNU GPL.  See License.txt or (http://www.gnu.org/copyleft/gpl.html). */
//////////////////////////////////////////////////////////////////////
#pragma once

class CMeterIcon  
{
public:
	CMeterIcon();
	virtual ~CMeterIcon();

	bool SetColorLevels(const int* pLimits, const COLORREF* pColors, int nEntries);
	COLORREF SetBorderColor(COLORREF crColor);
	int SetNumBars(int nNum);
	int SetMaxValue(int nVal);
	int SetWidth(int nWidth);
	SIZE SetDimensions(int nWidth, int nHeight);
	bool Init(HICON hFrame, int nMaxVal, int nNumBars, int nSpacingWidth, int nWidth, int nHeight, COLORREF crColor);
	HICON Create(const int* pBarData);
	HICON SetFrame(HICON hIcon);

protected:
	int m_nEntries;
	bool m_bInit;
	HICON m_hFrame;
	int m_nSpacingWidth;
	int m_nMaxVal;
	SIZE m_sDimensions;
	int m_nNumBars;
	COLORREF m_crBorderColor;
	int* m_pLimits;
	COLORREF* m_pColors;

	bool DrawIconMeter(HDC destDC, HDC destDCMask, int nLevel, int nPos);
	HICON CreateMeterIcon(const int* pBarData);
	COLORREF GetMeterColor(int nLevel) const;
};
