/*
 * $Id: NumericEdit.h 5130 2008-03-25 10:43:20Z fengwen $
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
#pragma once


// NumericEdit

class CNumericEdit : public CEdit
{
	DECLARE_DYNAMIC(CNumericEdit)

public:
	CNumericEdit(int iMaxWholeDigits = 5, int iMaxDecimalPlaces = 0);
	virtual ~CNumericEdit();

protected:
	int m_iMaxWholeDigits;
	int m_iMaxDecimalPlaces;
	int m_iMinValue;
	int m_iMaxValue;
	TCHAR m_cNegativeSign;
	TCHAR m_cDecimalPoint;
	CString m_strPrefix;

protected:
	DECLARE_MESSAGE_MAP()

public:
	void SetMaxWholeDigits(int nMaxWholeDigits);
	void SetMaxDecimalPlaces(int iMaxDecimalPlaces);
	void SetRange(int nMin, int nMax);

	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	CString GetText(void) const;
	bool IsNegativeAllowed() const;

	BOOL	Validate(BOOL bPrompt = TRUE, BOOL bFocus = TRUE);
protected:
	CString GetNumericText(const CString& strText, bool bUseMathSymbols = false) const;
};
