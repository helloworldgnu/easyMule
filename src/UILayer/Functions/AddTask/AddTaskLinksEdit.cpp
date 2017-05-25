/*
 * $Id: AddTaskLinksEdit.cpp 7586 2008-10-08 10:58:10Z dgkang $
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
// AddTaskLinksEdit.cpp : 实现文件
//

#include "stdafx.h"
#include "AddTaskLinksEdit.h"
#include ".\addtasklinksedit.h"
#include "UserMsgs.h"
#include "ED2KLink.h"
#include "Util.h"
#include "otherfunctions.h"

// CAddTaskLinksEdit

IMPLEMENT_DYNAMIC(CAddTaskLinksEdit, CEdit)
CAddTaskLinksEdit::CAddTaskLinksEdit()
{
	m_pDoc = NULL;
}

CAddTaskLinksEdit::~CAddTaskLinksEdit()
{
}


BEGIN_MESSAGE_MAP(CAddTaskLinksEdit, CEdit)
	ON_CONTROL_REFLECT(EN_CHANGE, OnEnChange)
	
	ON_MESSAGE(UM_ADDTASK_DOC_ADDED, OnDocAdded)
	ON_MESSAGE(UM_ADDTASK_DOC_MODIFIED, OnDocModified)
	ON_MESSAGE(UM_ADDTASK_DOC_URL_ADDED, OnDocUrlAdded)
	ON_MESSAGE(UM_ADDTASK_DOC_URL_MODIFIED, OnDocUrlModified)
	ON_MESSAGE(UM_ADDTASK_DOC_URL_REMOVED, OnDocUrlRemoved)
	
END_MESSAGE_MAP()


void CAddTaskLinksEdit::UpdateLinksByWindowText()
{
	if (NULL == m_pDoc)
		return;

	//	取现在Doc里的所有ed2k的fileHash，组成set。
	set<CFileHashKey>	setKeysInDoc;
	setKeysInDoc = m_pDoc->GetAllKeysInDoc();

	//	把EditCtrl里的内容变成一行一行的字符串，并把每一行放入列表中。
	CString	strText;
	GetWindowText(strText);
	CList<CString>		lstLinks;
	::ConvertStrToStrList(&lstLinks, strText);


	CFileHashKey			key;
	CAddTaskDoc::SItem		docItem;
	CString					strLine;
	CED2KFileLink			*pLink = NULL;

	//CList<CString>			lstUrls;
	CMapStringToPtr			setUrls;
	CString					strPrefix;

	// 分析每一行，并做相应的处理。
	POSITION	pos = lstLinks.GetHeadPosition();
	while (NULL != pos)
	{
		strLine = lstLinks.GetNext(pos);

		strLine = Decode3URL(strLine);
		
		strPrefix = strLine.Left(strLine.Find(_T(':')));

		if (0 == strPrefix.CompareNoCase(_T("ed2k")))
		{
			try
			{
				pLink = NULL;
				pLink = (CED2KFileLink*) CED2KFileLink::CreateLinkFromUrl(strLine);
			}
			catch (...)
			{
				SAFE_DELETE(pLink);
			}

			if (NULL != pLink)
			{
				key = pLink->GetHashKey();
				SAFE_DELETE(pLink);

				setKeysInDoc.erase(key);

				docItem.strLinkText = strLine;
				docItem.bCheck = TRUE;
				m_pDoc->SetItem(key, docItem, CAddTaskDoc::IM_TEXT | CAddTaskDoc::IM_CHECK, GetSafeHwnd());
			}
		}
		else if (0 == strPrefix.CompareNoCase(_T("http"))
			|| 0 == strPrefix.CompareNoCase(_T("ftp")))
		{
			setUrls.SetAt(strLine, NULL);
			//lstUrls.AddTail(strLine);
		}

	}

	// EditCtrl里没有的ed2k链接，则在doc里把它删除。
	set<CFileHashKey>::iterator		it;
	for (it = setKeysInDoc.begin();
		setKeysInDoc.end() != it;
		it++)
	{
		//docItem.bCheck = FALSE;
		//m_pDoc->SetItem(*it, docItem, CAddTaskDoc::IM_CHECK, GetSafeHwnd());
		m_pDoc->RemoveItem(*it);
	}


	m_pDoc->UpdateUrlItems(&setUrls, GetSafeHwnd());
}

void CAddTaskLinksEdit::AddText(LPCTSTR lpszText)
{
	CString	str;
	CString	strText;
	GetWindowText(strText);

	if (strText.IsEmpty())
	{
		strText.Append(lpszText);
		strText.Append(_T("\r\n"));
	}
	else
	{
		str = strText.Right(2);
		if (_T("\r\n") == str)
		{
			strText.Append(lpszText);
			strText.Append(_T("\r\n"));
		}
		else
		{
			strText.Append(_T("\r\n"));
			strText.Append(lpszText);
			strText.Append(_T("\r\n"));
		}
	}

	SetWindowText(strText);
}

void CAddTaskLinksEdit::RemoveLine(LPCTSTR lpszText)
{
	if (NULL == lpszText
		|| lpszText[0] == _T('\0'))
		return;

	CString	strOldText;
	GetWindowText(strOldText);
	CONST TCHAR * pszURL = strOldText;

	CString url;
	CString result = _T("");
	INT len = _tcslen(pszURL), i = 0, j = 0;
	for(i = 0, j = _tcscspn(pszURL + i, _T("\r\n")); i < len; i += j + 1, j = _tcscspn(pszURL + i, _T("\r\n")))
	{
		if(j > 10)
		{
			url.SetString(pszURL + i, j);
			url = url.Trim();
			if(url != _T("") && url != lpszText)
				result = result + url + _T("\r\n");
		}
	}

	SetWindowText(result);
}

void CAddTaskLinksEdit::SetText(const CFileHashKey &key, LPCTSTR lpszText)
{
	int	iStartPos, iEndPos;
	if ( !FindLineByKey(key, iStartPos, iEndPos) )
	{
		AddText(lpszText);
	}
	else
	{
		RemoveText(key);
		AddText(lpszText);
	}
}

void CAddTaskLinksEdit::RemoveText(const CFileHashKey &key)
{
	int	iStartPos, iEndPos;
	if ( !FindLineByKey(key, iStartPos, iEndPos) )
		return;

	CString	strOldText;
	CString	strNewText;

	GetWindowText(strOldText);
	strNewText = RemoveLine(strOldText, iStartPos, iEndPos);
	SetWindowText(strNewText);
}

CString	CAddTaskLinksEdit::RemoveLine(const CString &str, int iStart, int iEnd)
{
	CString strTemp = str.Mid(iEnd + 1, 2);
	if (_T("\r\n") == strTemp)
		iEnd += 2;
	
	
	CString	strNew;

	strNew = str.Left(iStart);
	if (iEnd < str.GetLength() - 1)
		strNew += str.Mid(iEnd + 1);

	return strNew;
}

BOOL CAddTaskLinksEdit::FindLineByKey(const CFileHashKey &key, int &iStartPos, int &iEndPos)
{
	BOOL				bDone;
	int					iTmpStartPos;
	int					iTmpEndPos;
	CString				strLine;
	CED2KLink			*pLink = NULL;
	CFileHashKey		keyCmp;

	CString	str;
	GetWindowText(str);

	bDone = FALSE;
	iTmpStartPos = iTmpEndPos = 0;

	do {
		iStartPos = iTmpStartPos;

		iTmpEndPos = str.Find(_T("\r\n"), iTmpStartPos);
		if (-1 == iTmpEndPos)
		{
			iEndPos = str.GetLength() - 1;

			strLine = str.Mid(iTmpStartPos);
			bDone = TRUE;
		}
		else
		{
			iEndPos = iTmpEndPos - 1;

			strLine = str.Mid(iTmpStartPos, iTmpEndPos - iTmpStartPos);
			iTmpStartPos = iTmpEndPos + 2;
		}

		try
		{
			if (!strLine.IsEmpty())
			{
				pLink = CED2KLink::CreateLinkFromUrl(strLine);
				if (CED2KLink::kFile == pLink->GetKind())
					keyCmp = ((CED2KFileLink*)pLink)->GetHashKey();
				SAFE_DELETE(pLink);

				if (key == keyCmp)
				{
					return TRUE;
				}
			}
		}
		catch(...)
		{
		}

	} while(!bDone);

	return FALSE;
}

// CAddTaskLinksEdit 消息处理程序


void CAddTaskLinksEdit::OnEnChange()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 CEdit::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	UpdateLinksByWindowText();
}

LRESULT CAddTaskLinksEdit::OnDocAdded(WPARAM wParam, LPARAM lParam)
{
	if (NULL == m_pDoc)			return 0;
	if (0 == lParam)			return 0;

	CFileHashKey			*pKey = (CFileHashKey*) lParam;
	CAddTaskDoc::SItem		item;

	if (!m_pDoc->GetItem(*pKey, item))
		return 0;

	AddText(item.strLinkText);

	return 0;
}
LRESULT CAddTaskLinksEdit::OnDocModified(WPARAM wParam, LPARAM lParam)
{
	if (NULL == m_pDoc)						return 0;
	if (0 == wParam || 0 == lParam)			return 0;

	DWORD					dwModifiedMask	= wParam;
	CFileHashKey			*pKey = (CFileHashKey*) lParam;
	CAddTaskDoc::SItem		item;

	if (! m_pDoc->GetItem(*pKey, item))
		return 0;

	if (CAddTaskDoc::IM_CHECK & dwModifiedMask)
	{
		if (item.bCheck)
			SetText(*pKey, item.strLinkText);
		else
			RemoveText(*pKey);
	}
	else if (CAddTaskDoc::IM_TEXT & dwModifiedMask)
	{
		SetText(*pKey, item.strLinkText);
	}

	return 0;
}

LRESULT CAddTaskLinksEdit::OnDocUrlAdded(WPARAM wParam, LPARAM lParam)
{
	if (0 == lParam)
		return 0;
	
	CString	strUrl = (LPCTSTR) lParam;

	AddText(strUrl);

	return 0;
}

LRESULT CAddTaskLinksEdit::OnDocUrlModified(WPARAM wParam, LPARAM lParam)
{
	if (0 == lParam)
		return 0;

	BOOL	bCheck = (BOOL) wParam;
	CString	strUrl = (LPCTSTR) lParam;

	if (bCheck)
		AddText(strUrl);
	else
		RemoveLine(strUrl);

	return 0;
}

LRESULT CAddTaskLinksEdit::OnDocUrlRemoved(WPARAM wParam, LPARAM lParam)
{
	if (0 == lParam)
		return 0;

	RemoveLine((LPCTSTR) lParam);

	return 0;
}
