/*
 * $Id: NumericEdit.cpp 5130 2008-03-25 10:43:20Z fengwen $
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
// NumericEdit.cpp : 实现文件
//

#include "stdafx.h"
#include "emule.h"
#include "NumericEdit.h"
#include ".\numericedit.h"



// NumericEdit

IMPLEMENT_DYNAMIC(CNumericEdit, CEdit)
CNumericEdit::CNumericEdit(int iMaxWholeDigits /*= 5*/, int iMaxDecimalPlaces /*= 0*/)
{
	m_iMaxWholeDigits = iMaxWholeDigits >= 0 ? iMaxWholeDigits : -iMaxWholeDigits;
	m_iMaxDecimalPlaces = iMaxDecimalPlaces;

	m_cDecimalPoint = _T('.');
	m_iMaxDecimalPlaces = 0;

	m_iMinValue = 0;
	m_iMaxValue = 0;
}

CNumericEdit::~CNumericEdit()
{
}

 
BEGIN_MESSAGE_MAP(CNumericEdit, CEdit)
//	ON_WM_CHAR()
//	ON_WM_KILLFOCUS()
END_MESSAGE_MAP()

void CNumericEdit::SetMaxWholeDigits(int iMaxWholeDigits)
{
	ASSERT(iMaxWholeDigits);

	//bool bAllowNegative = (iMaxWholeDigits >= 0);

	if (iMaxWholeDigits < 0)
	{
		iMaxWholeDigits = -iMaxWholeDigits;
	}

	if (m_iMaxWholeDigits == iMaxWholeDigits)
	{
		return;
	}

	m_iMaxWholeDigits = iMaxWholeDigits;
}

void CNumericEdit::SetMaxDecimalPlaces(int iMaxDecimalPlaces)
{
	ASSERT(iMaxDecimalPlaces >= 0);
	if (m_iMaxDecimalPlaces == iMaxDecimalPlaces)
	{
		return;
	}

	m_iMaxDecimalPlaces = iMaxDecimalPlaces;
}

void CNumericEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{	
	TCHAR tChar = static_cast<TCHAR>(nChar);

	int iStart, iEnd;
	GetSel(iStart, iEnd);	//获取输入位置

	
	CString strText = GetText();	//得到编辑框中的内容(输入前)
	CString strNumericText = GetNumericText(strText); //得到编辑框内容里的数字(输入前)
	int iDecimalPos = strText.Find(m_cDecimalPoint); //小数点位置

	int iNumericDecimalPos = strNumericText.Find(m_cDecimalPoint);
	int iLen = strText.GetLength();	//内容长度
	int iNumericLen = strNumericText.GetLength(); //内容中数字长度
	int iPrefixLen = m_strPrefix.GetLength();

	bool bNeedAdjustment = false;

	if (iStart < iPrefixLen && _istprint(tChar))
	{
		TCHAR cPrefix = m_strPrefix[iStart];

		if (cPrefix == tChar)
		{
			if (iLen > iStart)
			{
				iEnd = (iEnd == iLen ? iEnd : (iStart + 1));
				SetSel(iStart, iEnd);
				ReplaceSel(CString(tChar), TRUE);
			}
			else
			{
				CEdit::OnChar(nChar, nRepCnt, nFlags);
			}
		}
		else 
		{
			if ((_istdigit(tChar) || tChar == m_cNegativeSign || tChar == m_cDecimalPoint))
			{
				iEnd = (iEnd == iLen ? iEnd : (iPrefixLen));
				SetSel(iStart, iEnd);
				ReplaceSel(m_strPrefix.Mid(iStart), TRUE);

				CEdit::OnChar(nChar, nRepCnt, nFlags);
			}
		}
		
		return;
	}

	//检测是否输入减(负)号
	if (tChar == m_cNegativeSign && IsNegativeAllowed())
	{
		if (iStart == iPrefixLen)
		{
			if (!strNumericText.IsEmpty() && strNumericText[0] == m_cNegativeSign)
			{
				iEnd = (iEnd == iLen ? iEnd : (iStart + 1));
				SetSel(iStart, iEnd);
				ReplaceSel(CString(m_cNegativeSign), TRUE);
				return;
			}
		}
		else
		{
			if (strNumericText[0] == m_cNegativeSign)
			{
				SetSel(iPrefixLen, iPrefixLen + 1);
				ReplaceSel(_T(""), TRUE);
				SetSel(iStart - 1, iEnd - 1);
			}
			else
			{
				SetSel(iPrefixLen, iPrefixLen);
				ReplaceSel(CString(m_cNegativeSign), TRUE);
				SetSel(iStart + 1, iEnd + 1);
			}

			return;
		}
	}
	else
	{
		if (tChar == m_cDecimalPoint && m_iMaxDecimalPlaces > 0)
		{
			if (iDecimalPos >= 0)
			{
				if (iDecimalPos >= iStart && iDecimalPos < iEnd)
				{
					bNeedAdjustment = true;
				}
				else
				{
					SetSel(iDecimalPos + 1, iDecimalPos + 1);
				}
				return;
			}
			else
				bNeedAdjustment = true;
		}
	}

	if (_istdigit(tChar))
	{
		//判断是否超过范围
		bool bLimit = true;
		if (m_iMaxValue == 0 || m_iMaxValue - m_iMinValue == 0)
		{
			bLimit = false;
		}

		if (bLimit)
		{
			CString strNewNumericText = CString(strNumericText);

			if (strNewNumericText.IsEmpty())
			{
				strNewNumericText = _T("0");
			}

			strNewNumericText.Insert(iStart, tChar);

			int NewNumber = _wtoi(strNewNumericText);

			if (NewNumber > m_iMaxValue)
			{
				CString text;
				text.Format(GetResString(IDS_EDIT_RANGER), m_iMinValue, m_iMaxValue);
				MessageBox(text, _T("easyMule"), MB_OK);
				SetSel(0, -1);
				ReplaceSel(strNumericText, TRUE);
				return;
			}

			if (NewNumber < m_iMinValue)
			{
				CString text;
				text.Format(GetResString(IDS_EDIT_RANGER),  m_iMinValue, m_iMaxValue);
				MessageBox(text, _T("easyMule"), MB_OK);

				SetSel(0, -1);
				ReplaceSel(strNumericText, TRUE);
				return;
			}
		}

		if (iDecimalPos >= 0 && iDecimalPos < iStart)
		{
			if (strNumericText.Mid(iNumericDecimalPos + 1).GetLength() == m_iMaxDecimalPlaces)
			{
				if (iStart <= iDecimalPos + m_iMaxDecimalPlaces)
				{
					iEnd = (iEnd == iLen ? iEnd : (iStart + 1));
					SetSel(iStart, iEnd);
					ReplaceSel(CString(tChar), TRUE);
				}
				return;
			}
		}
		else
		{
			bool bIsNegative = (!strNumericText.IsEmpty() && strNumericText[0] == m_cNegativeSign);

			if (iStart == m_iMaxWholeDigits + bIsNegative + 0 + iPrefixLen)
			{
				if (/*m_uFlags & AddDecimalAfterMaxWholeDigits &&*/ m_iMaxDecimalPlaces > 0)
				{
					iEnd = (iEnd == iLen ? iEnd : (iStart + 2));
					SetSel(iStart, iEnd);
					ReplaceSel(CString(m_cDecimalPoint) + tChar, TRUE);
				}
				return;
			}

			if (strNumericText.Mid(0, iNumericDecimalPos >= 0 ? iNumericDecimalPos : iNumericLen).GetLength() == m_iMaxWholeDigits + bIsNegative)
			{
				/*if (strText[iStart] == m_cGroupSeparator)
				{
					nStart++;
				}*/

				iEnd = (iEnd == iLen ? iEnd : (iStart + 1));
				SetSel(iStart, iEnd);
				ReplaceSel(CString(tChar), TRUE);

				return;
			}

			bNeedAdjustment = true;
		}
	}
	else if (!_istprint(tChar))
	{
		bNeedAdjustment = true;

		if (tChar == 0x08)
		{
			CString strNewNumericText = CString(strNumericText);

			if (iEnd > 0)
			{
				strNewNumericText.Delete(iEnd - 1);
			}

			if (strNewNumericText.IsEmpty())
			{
				SetWindowText(strNewNumericText);
				MessageBox(_T("Please enter a Integer!"), _T("easyMue"), MB_OK);
				return;
			}

		}
	}
	else
	{
		return;
	}

	CEdit::OnChar(nChar, nRepCnt, nFlags);
}

CString CNumericEdit::GetText(void) const
{
	CString strText;
	GetWindowText(strText);
	return strText;
}

CString CNumericEdit::GetNumericText(const CString& strText, bool bUseMathSymbols) const
{
	CString strNewText;
	bool bIsNegative = false;
	bool bHasDecimalPoint = false;

	for (int iPos = 0, nLen = strText.GetLength(); iPos < nLen; iPos++)
	{
		TCHAR c = strText[iPos];
		if (_istdigit(c))
		{
			strNewText += c;
		}
		else if (c == m_cNegativeSign)
		{
			bIsNegative = true;
		}
		else if (c == m_cDecimalPoint && !bHasDecimalPoint)
		{
			bHasDecimalPoint = true;
			strNewText += (bUseMathSymbols ? _T('.') : m_cDecimalPoint);
		}
	}

	if (bIsNegative)
	{
		;//strNewText.Insert(0, bUseMathSymbols ? '-' : m_cNegativeSign);
	}

	return strNewText;
}

bool CNumericEdit::IsNegativeAllowed() const
{
	return false;
}

void CNumericEdit::SetRange(int nMin, int nMax)
{
	if (nMin < 0)
	{
		nMin = -nMin;
	}

	if (nMax < 0)
	{
		nMax = -nMax;
	}

	if (nMin >= nMax)
	{
		nMax = 0;
		nMin = 0;
	}

	m_iMinValue = nMin;
	m_iMaxValue = nMax;
}

BOOL CNumericEdit::Validate(BOOL bPrompt, BOOL bFocus)
{
	if (m_iMinValue == 0 && m_iMaxValue == 0)
		return TRUE;

	CString		strText;
	GetWindowText(strText);
	int iNum = _tstoi(strText);
	if (iNum < m_iMinValue || iNum > m_iMaxValue)
	{
		if (bPrompt)
		{
			CString strPrompt;
			strPrompt.Format(GetResString(IDS_EDIT_RANGER), m_iMinValue, m_iMaxValue);
			MessageBox(strPrompt, _T("easyMule"), MB_OK);
		}
		if (bFocus)
		{
			SetFocus();
			SetSel(0, -1);
		}
		return FALSE;
	}
	return TRUE;
}

