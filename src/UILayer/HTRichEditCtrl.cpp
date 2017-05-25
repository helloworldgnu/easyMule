/* 
 * $Id: HTRichEditCtrl.cpp 4783 2008-02-02 08:17:12Z soarchin $
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

#include "stdafx.h"
#include <share.h>
#include "emule.h"
#include "HTRichEditCtrl.h"
#include "OtherFunctions.h"
#include "Preferences.h"
#include "MenuCmds.h"
#include "Log.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNAMIC(CHTRichEditCtrl, CRichEditCtrl)

BEGIN_MESSAGE_MAP(CHTRichEditCtrl, CRichEditCtrl)
ON_WM_CONTEXTMENU()
ON_WM_KEYDOWN()
ON_CONTROL_REFLECT(EN_ERRSPACE, OnEnErrspace)
ON_CONTROL_REFLECT(EN_MAXTEXT, OnEnMaxtext)
ON_NOTIFY_REFLECT_EX(EN_LINK, OnEnLink)
ON_WM_CREATE()
ON_WM_SYSCOLORCHANGE()
ON_WM_SETCURSOR()
END_MESSAGE_MAP()

CHTRichEditCtrl::CHTRichEditCtrl()
{
	m_bRichEdit = true;
	m_bAutoScroll = true;
	m_bNoPaint = false;
	m_bEnErrSpace = false;
	m_bRestoreFormat = false;
	memset(&m_cfDefault, 0, sizeof m_cfDefault);
	m_bForceArrowCursor = false;
	m_hArrowCursor = ::LoadCursor(NULL, IDC_ARROW);
}

CHTRichEditCtrl::~CHTRichEditCtrl()
{
}

void CHTRichEditCtrl::Localize(){
}

void CHTRichEditCtrl::Init(LPCTSTR pszTitle, LPCTSTR pszSkinKey)
{
	SetProfileSkinKey(pszSkinKey);
	SetTitle(pszTitle);

	VERIFY( SendMessage(EM_SETUNDOLIMIT, 0, 0) == 0 );
	int iMaxLogBuff = thePrefs.GetMaxLogBuff();
	LimitText(iMaxLogBuff ? iMaxLogBuff : 128*1024);
	m_iLimitText = GetLimitText();

	VERIFY( GetSelectionCharFormat(m_cfDefault) );

	// prevent the RE control to change the font height within single log lines (may happen with some Unicode chars)
	DWORD dwLangOpts = SendMessage(EM_GETLANGOPTIONS);
	SendMessage(EM_SETLANGOPTIONS, 0, dwLangOpts & ~(IMF_AUTOFONT /*| IMF_AUTOFONTSIZEADJUST*/));
	//SendMessage(EM_SETEDITSTYLE, SES_EMULATESYSEDIT, SES_EMULATESYSEDIT);
}

void CHTRichEditCtrl::SetProfileSkinKey(LPCTSTR pszSkinKey)
{
	m_strSkinKey = pszSkinKey;
}

void CHTRichEditCtrl::SetTitle(LPCTSTR pszTitle)
{
	m_strTitle = pszTitle;
}

int CHTRichEditCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CRichEditCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;
	Init(NULL);
	return 0;
}

LRESULT CHTRichEditCtrl::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
		case WM_ERASEBKGND:
			if (m_bNoPaint)
				return TRUE;
		case WM_PAINT:
			if (m_bNoPaint)
				return TRUE;
	}
	return CRichEditCtrl::WindowProc(message, wParam, lParam);
}

COLORREF GetLogLineColor(UINT eMsgType)
{
	if (eMsgType == LOG_ERROR)
		return thePrefs.m_crLogError;
	if (eMsgType == LOG_WARNING)
		return thePrefs.m_crLogWarning;
	if (eMsgType == LOG_SUCCESS)
		return thePrefs.m_crLogSuccess;
	ASSERT( eMsgType == LOG_INFO );
	return CLR_DEFAULT;
}

void CHTRichEditCtrl::FlushBuffer()
{
	if (m_astrBuff.GetSize() > 0) // flush buffer
	{
		for (int i = 0; i < m_astrBuff.GetSize(); i++)
		{
			const CString& rstrLine = m_astrBuff[i];
			if (!rstrLine.IsEmpty())
			{
				if ((_TUCHAR)rstrLine[0] < 8)
					AddLine((LPCTSTR)rstrLine + 1, rstrLine.GetLength() - 1, false, GetLogLineColor((_TUCHAR)rstrLine[0]));
				else
					AddLine((LPCTSTR)rstrLine, rstrLine.GetLength());
			}
		}
		m_astrBuff.RemoveAll();
	}
}

void CHTRichEditCtrl::AddEntry(LPCTSTR pszMsg)
{
	CString strLine(pszMsg);
	strLine += _T("\n");
	if (m_hWnd == NULL){
		m_astrBuff.Add(strLine);
	}
	else{
		FlushBuffer();
		AddLine(strLine, strLine.GetLength());
	}
}

void CHTRichEditCtrl::Add(LPCTSTR pszMsg, int iLen)
{
	if (m_hWnd == NULL){
		CString strLine(pszMsg);
		m_astrBuff.Add(strLine);
	}
	else{
		FlushBuffer();
		AddLine(pszMsg, iLen);
	}
}

void CHTRichEditCtrl::AddTyped(LPCTSTR pszMsg, int iLen, UINT eMsgType)
{
	if (m_hWnd == NULL)
	{
		CString strLine;
		strLine = (TCHAR)(eMsgType & LOGMSGTYPEMASK);
		strLine += pszMsg;
		m_astrBuff.Add(strLine);
	}
	else
	{
		FlushBuffer();
		AddLine(pszMsg, iLen, false, GetLogLineColor(eMsgType & LOGMSGTYPEMASK));
	}
}

void CHTRichEditCtrl::AddLine(LPCTSTR pszMsg, int iLen, bool bLink, COLORREF cr, COLORREF bk, DWORD mask)
{
	int iMsgLen = (iLen == -1) ? _tcslen(pszMsg) : iLen;
	if (iMsgLen == 0)
		return;
#ifdef _DEBUG
	//	if (pszMsg[iMsgLen - 1] == _T('\n'))
	//		ASSERT( iMsgLen >= 2 && pszMsg[iMsgLen - 2] == _T('\r') );
#endif

	// Get Edit contents dimensions and cursor position
	long lStartChar, lEndChar;
	GetSel(lStartChar, lEndChar);
	int iSize = GetWindowTextLength();

	if (lStartChar == iSize && iSize == lEndChar)
	{
		// The cursor resides at the end of text
		SCROLLINFO si;
		si.cbSize = sizeof si;
		si.fMask = SIF_ALL;
		if (m_bAutoScroll && GetScrollInfo(SB_VERT, &si) && si.nPos >= (int)(si.nMax - si.nPage + 1))
		{
			// Not scrolled away
			SafeAddLine(iSize, pszMsg, iLen, lStartChar, lEndChar, bLink, cr, bk, mask);
			if (m_bAutoScroll && !IsWindowVisible())
				ScrollToLastLine();
		}
		else
		{
			// Reduce flicker by ignoring WM_PAINT
			m_bNoPaint = true;
			BOOL bIsVisible = IsWindowVisible();
			if (bIsVisible)
				SetRedraw(FALSE);

			// Remember where we are
			int iFirstLine = !m_bAutoScroll ? GetFirstVisibleLine() : 0;

			// Select at the end of text and replace the selection
			// This is a very fast way to add text to an edit control
			SafeAddLine(iSize, pszMsg, iLen, lStartChar, lEndChar, bLink, cr, bk, mask);
			//if (m_bAutoScroll && lStartChar == lEndChar)
			//	lStartChar = lEndChar = -1;
			SetSel(lStartChar, lEndChar); // Restore our previous selection

			if (!m_bAutoScroll)
				LineScroll(iFirstLine - GetFirstVisibleLine());
			else
				ScrollToLastLine();

			m_bNoPaint = false;
			if (bIsVisible){
				SetRedraw();
				if (m_bRichEdit)
					Invalidate();
			}
		}
	}
	else
	{
		// We should add the text anyway...

		// Reduce flicker by ignoring WM_PAINT
		m_bNoPaint = true;
		BOOL bIsVisible = IsWindowVisible();
		if (bIsVisible)
			SetRedraw(FALSE);

		// Remember where we are
		int iFirstLine = !m_bAutoScroll ? GetFirstVisibleLine() : 0;
		// Very annoying problems with EM_GETSCROLLPOS/EM_SETSCROLLPOS. Depending
		// on the amount of data in the control, the control may start to scroll up
		// by itself(!!) -- obviously because of some internal rounding errors..
		//
		// Using 'LineScroll' also gives glitches (also depending on the amount of
		// data stored in the control), but at least it doesn't start to show some 'life'
		/*POINT ptScrollPos;
		if (!m_bAutoScroll)
			SendMessage(EM_GETSCROLLPOS, 0, (LPARAM)&ptScrollPos);*/

		if (lStartChar != lEndChar)
		{
			// If we are currently selecting some text, we have to find out
			// if the caret is near the beginning of this block or near the end.
			// Note that this does not always work. Because of the EM_CHARFROMPOS
			// message returning only 16 bits this will fail if the user has selected
			// a block with a length dividable by 64k.

			// NOTE: This may cause a lot of terrible CRASHES within the RichEdit control when used for a RichEdit control!?
			// To reproduce the crash: click in the RE control while it's drawing a line and start a selection!
			if (!m_bRichEdit){
				CPoint pt;
				::GetCaretPos(&pt);
				int iCaretPos = CharFromPos(pt);
				if (abs((lStartChar % 0xffff - iCaretPos)) < abs((lEndChar % 0xffff - iCaretPos)))
				{
					iCaretPos = lStartChar;
					lStartChar = lEndChar;
					lEndChar = iCaretPos;
				}
			}
		}

		// Note: This will flicker, if someone has a good idea how to prevent this - let me know

		// Select at the end of text and replace the selection
		// This is a very fast way to add text to an edit control
		SafeAddLine(iSize, pszMsg, iLen, lStartChar, lEndChar, bLink, cr, bk, mask);
		//if (m_bAutoScroll && lStartChar == lEndChar)
		//	lStartChar = lEndChar = -1;
		SetSel(lStartChar, lEndChar); // Restore our previous selection

		if (!m_bAutoScroll){
			LineScroll(iFirstLine - GetFirstVisibleLine());
			//SendMessage(EM_SETSCROLLPOS, 0, (LPARAM)&ptScrollPos);
		}
		else
			ScrollToLastLine();

		m_bNoPaint = false;
		if (bIsVisible){
			SetRedraw();
			if (m_bRichEdit)
				Invalidate();
		}
	}
}

void CHTRichEditCtrl::OnEnErrspace()
{
	m_bEnErrSpace = true;
}

void CHTRichEditCtrl::OnEnMaxtext()
{
	m_bEnErrSpace = true;
}

void CHTRichEditCtrl::ScrollToLastLine(bool bForceLastLineAtBottom)
{
	if (bForceLastLineAtBottom)
	{
		int iFirstVisible = GetFirstVisibleLine();
		if (iFirstVisible > 0)
			LineScroll(-iFirstVisible);
	}

	// WM_VSCROLL does not work correctly under Win98 (or older version of comctl.dll)
	SendMessage(WM_VSCROLL, SB_BOTTOM);
}

void CHTRichEditCtrl::AddString(int nPos, LPCTSTR pszString, bool bLink, COLORREF cr,COLORREF bk, DWORD mask)
{
	bool bRestoreFormat = false;
	m_bEnErrSpace = false;
	SetSel(nPos, nPos);
	if (bLink)
	{
		CHARFORMAT2 cf;
		memset(&cf, 0, sizeof cf);
		GetSelectionCharFormat(cf);
		cf.dwMask |= CFM_LINK;
		cf.dwEffects |= CFE_LINK;
		SetSelectionCharFormat(cf);
	}
	else if (cr != CLR_DEFAULT || bk != CLR_DEFAULT)
	{
		CHARFORMAT2 cf;
		memset(&cf, 0, sizeof(cf));
		GetSelectionCharFormat(cf);
		if(cr != CLR_DEFAULT)
		{
			cf.dwMask |= CFM_COLOR;
			cf.dwEffects &= ~CFE_AUTOCOLOR;
			cf.crTextColor = cr;
		}
		if(bk != CLR_DEFAULT)
		{
			cf.dwMask |= CFM_BACKCOLOR;
			cf.dwEffects &= ~CFE_AUTOBACKCOLOR;
			cf.crBackColor = bk;
		}
		cf.dwMask |= mask;
		if(mask & CFM_BOLD)
			cf.dwEffects |= CFE_BOLD; //Checks if bold is set in the mask and then sets it in effects if it is
		else if(cf.dwEffects & CFE_BOLD)
			cf.dwEffects ^= CFE_BOLD; //Unset bold
		if(mask & CFM_ITALIC)
			cf.dwEffects |= CFE_ITALIC; //Checks if italic is set in the mask and then sets it in effects if it is
		else if(cf.dwEffects & CFE_ITALIC)
			cf.dwEffects ^= CFE_ITALIC; //Unset bold
		if(mask & CFM_UNDERLINE)
			cf.dwEffects |= CFE_UNDERLINE; //Checks if underlined is set in the mask and then sets it in effects if it is
		else if(cf.dwEffects & CFE_UNDERLINE)
			cf.dwEffects ^= CFE_UNDERLINE; //Unset italic
		SetSelectionCharFormat(cf);
		bRestoreFormat = true;
	}
	else if (m_bRestoreFormat)
	{
		SetSelectionCharFormat(m_cfDefault);
	}
	ReplaceSel(pszString);
	m_bRestoreFormat = bRestoreFormat;
}

void CHTRichEditCtrl::SafeAddLine(int nPos, LPCTSTR pszLine, int iLen, long& lStartChar, long& lEndChar, bool bLink, COLORREF cr, COLORREF bk, DWORD mask)
{
	// EN_ERRSPACE and EN_MAXTEXT are not working for rich edit control (at least not same as for standard control),
	// need to explicitly check the log buffer limit..
	int iCurSize = nPos;
	if (iCurSize + iLen >= m_iLimitText)
	{
		bool bOldNoPaint = m_bNoPaint;
		m_bNoPaint = true;
		BOOL bIsVisible = IsWindowVisible();
		if (bIsVisible)
			SetRedraw(FALSE);

		while (iCurSize > 0 && iCurSize + iLen > m_iLimitText)
		{
			// delete 1st line
			int iLine0Len = LineLength(0) + 1; // add NL character
			SetSel(0, iLine0Len);
			ReplaceSel(_T(""));

			// update any possible available selection
			lStartChar -= iLine0Len;
			if (lStartChar < 0)
				lStartChar = 0;
			lEndChar -= iLine0Len;
			if (lEndChar < 0)
				lEndChar = 0;

			iCurSize = GetWindowTextLength();
		}

		m_bNoPaint = bOldNoPaint;
		if (bIsVisible && !m_bNoPaint){
			SetRedraw();
			if (m_bRichEdit)
				Invalidate();
		}
	}

	AddString(nPos, pszLine, bLink, cr, bk, mask);

	if (m_bEnErrSpace)
	{
		bool bOldNoPaint = m_bNoPaint;
		m_bNoPaint = true;
		BOOL bIsVisible = IsWindowVisible();
		if (bIsVisible)
			SetRedraw(FALSE);

		// remove the first line as long as we are capable of adding the new line
		int iSafetyCounter = 0;
		while (m_bEnErrSpace && iSafetyCounter < 10)
		{
			// delete the previous partially added line
			SetSel(nPos, -1);
			ReplaceSel(_T(""));

			// delete 1st line
			int iLine0Len = LineLength(0) + 1; // add NL character
			SetSel(0, iLine0Len);
			ReplaceSel(_T(""));

			// update any possible available selection
			lStartChar -= iLine0Len;
			if (lStartChar < 0)
				lStartChar = 0;
			lEndChar -= iLine0Len;
			if (lEndChar < 0)
				lEndChar = 0;

			// add the new line again
			nPos = GetWindowTextLength();
			AddString(nPos, pszLine, bLink, cr, bk, mask);

			if (m_bEnErrSpace && nPos == 0){
				// should never happen: if we tried to add the line another time in the 1st line, there
				// will be no chance to add the line at all -> avoid endless loop!
				break;
			}
			iSafetyCounter++; // never ever create an endless loop!
		}
		m_bNoPaint = bOldNoPaint;
		if (bIsVisible && !m_bNoPaint){
			SetRedraw();
			if (m_bRichEdit)
				Invalidate();
		}
	}
}

void CHTRichEditCtrl::Reset()
{
	m_astrBuff.RemoveAll();
	SetRedraw(FALSE);
	SetWindowText(_T(""));
	SetRedraw();
	if (m_bRichEdit)
		Invalidate();
}

void CHTRichEditCtrl::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	long lSelStart, lSelEnd;
	GetSel(lSelStart, lSelEnd);

	// ugly, simulate a left click to get around the text cursor problem when right clicking.
	if (point.x != -1 && point.y != -1 && lSelStart == lSelEnd)
	{
		CPoint ptMouse(point);
		ScreenToClient(&ptMouse);
		SendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELONG(ptMouse.x, ptMouse.y));
		SendMessage(WM_LBUTTONUP, MK_LBUTTON, MAKELONG(ptMouse.x, ptMouse.y));
	}

	int iTextLen = GetWindowTextLength();

	CTitleMenu menu;
	menu.CreatePopupMenu();
	menu.AddMenuTitle(GetResString(IDS_LOGENTRY));
	menu.AppendMenu(MF_STRING | (lSelEnd > lSelStart ? MF_ENABLED : MF_GRAYED), MP_COPYSELECTED, GetResString(IDS_COPY));
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING | (iTextLen > 0 ? MF_ENABLED : MF_GRAYED), MP_SELECTALL, GetResString(IDS_SELECTALL));
	menu.AppendMenu(MF_STRING | (iTextLen > 0 ? MF_ENABLED : MF_GRAYED), MP_REMOVEALL , GetResString(IDS_PW_RESET));
	menu.AppendMenu(MF_STRING | (iTextLen > 0 ? MF_ENABLED : MF_GRAYED), MP_SAVELOG, GetResString(IDS_SAVELOG) + _T("..."));
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING | (m_bAutoScroll ? MF_CHECKED : MF_UNCHECKED), MP_AUTOSCROLL, GetResString(IDS_AUTOSCROLL));

	if (point.x == -1 && point.y == -1)
	{
		point.x = 16;
		point.y = 32;
		ClientToScreen(&point);
	}

	// Cheap workaround for the "Text cursor is showing while context menu is open" glitch. It could be solved properly
	// with the RE's COM interface, but because the according messages are not routed with a unique control ID, it's not
	// really useable (e.g. if there are more RE controls in one window). Would to envelope each RE window to get a unique ID..
	m_bForceArrowCursor = true;
	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
	m_bForceArrowCursor = false;

	VERIFY( menu.DestroyMenu() );
}

BOOL CHTRichEditCtrl::OnCommand(WPARAM wParam, LPARAM /*lParam*/)
{
	switch (wParam)
	{
		case MP_COPYSELECTED:
			CopySelectedItems();
			break;
		case MP_SELECTALL:
			SelectAllItems();
			break;
		case MP_REMOVEALL:
			Reset();
			break;
		case MP_SAVELOG:
			SaveLog();
			break;
		case MP_AUTOSCROLL:
			m_bAutoScroll = !m_bAutoScroll;
			break;
	}
	return TRUE;
}

bool CHTRichEditCtrl::SaveLog(LPCTSTR pszDefName)
{
	bool bResult = false;
	CFileDialog dlg(FALSE, _T("log"), pszDefName ? pszDefName : (LPCTSTR)m_strTitle, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("Log Files (*.log)|*.log||"), this, 0);
	if (dlg.DoModal() == IDOK)
	{
		FILE* fp = _tfsopen(dlg.GetPathName(), _T("wb"), _SH_DENYWR);
		if (fp)
		{
			// write Unicode byte-order mark 0xFEFF
			fputwc(0xFEFF, fp);

			CString strText;
			GetWindowText(strText);
			fwrite(strText, sizeof(TCHAR), strText.GetLength(), fp);
			if (ferror(fp)){
				CString strError;
				strError.Format(_T("Failed to write log file \"%s\" - %s"), dlg.GetPathName(), _tcserror(errno));
				AfxMessageBox(strError, MB_ICONERROR);
			}
			else
				bResult = true;
			fclose(fp);
		}
		else{
			CString strError;
			strError.Format(_T("Failed to create log file \"%s\" - %s"), dlg.GetPathName(), _tcserror(errno));
			AfxMessageBox(strError, MB_ICONERROR);
		}
	}
	return bResult;
}

CString CHTRichEditCtrl::GetLastLogEntry()
{
	CString strLog;
	int iLastLine = GetLineCount() - 2;
	if (iLastLine >= 0)
	{
		GetLine(iLastLine, strLog.GetBuffer(1024), 1024);
		strLog.ReleaseBuffer();
	}
	return strLog;
}

CString CHTRichEditCtrl::GetAllLogEntries()
{
	CString strLog;
	GetWindowText(strLog);
	return strLog;
}

void CHTRichEditCtrl::SelectAllItems()
{
	SetSel(0, -1);
}

void CHTRichEditCtrl::CopySelectedItems()
{
	Copy();
}

void CHTRichEditCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == 'A' && (GetKeyState(VK_CONTROL) & 0x8000))
	{
		//////////////////////////////////////////////////////////////////
		// Ctrl+A: Select all items
		SelectAllItems();
	}
	else if (nChar == 'C' && (GetKeyState(VK_CONTROL) & 0x8000))
	{
		//////////////////////////////////////////////////////////////////
		// Ctrl+C: Copy listview items to clipboard
		CopySelectedItems();
	}
	else if (nChar == VK_ESCAPE)
	{
		// dont minimize CHTRichEditCtrl
		return ;
	}

	CRichEditCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}

static const struct
{
	LPCTSTR pszScheme;
	int iLen;
} _apszSchemes[] =
    {
        { _T("ed2k://"),  7 },
        { _T("http://"),  7 },
        { _T("https://"), 8 },
        { _T("ftp://"),   6 },
        { _T("www."),     4 },
        { _T("ftp."),     4 },
        { _T("mailto:"),  7 }
    };

void CHTRichEditCtrl::AppendText(const CString& sText)
{
	LPCTSTR psz = sText;
	LPCTSTR pszStart = psz;
	while (*psz != _T('\0'))
	{
		bool bFoundScheme = false;
		for (int i = 0; i < ARRSIZE(_apszSchemes); i++)
		{
			if (_tcsncmp(psz, _apszSchemes[i].pszScheme, _apszSchemes[i].iLen) == 0)
			{
				// output everything before the URL
				if (psz - pszStart > 0){
					CString str(pszStart, psz - pszStart);
					AddLine(str, str.GetLength());
				}

				// search next space or EOL
				int iLen = _tcscspn(psz, _T(" \n\r\t"));
				if (iLen == 0){
					AddLine(psz, -1, true);
					psz += _tcslen(psz);
				}
				else{
					CString str(psz, iLen);
					AddLine(str, str.GetLength(), true);
					psz += iLen;
				}
				pszStart = psz;
				bFoundScheme = true;
				break;
			}
		}
		if (!bFoundScheme)
			psz = _tcsinc(psz);
	}

	if (*pszStart != _T('\0'))
		AddLine(pszStart, -1);
}

void CHTRichEditCtrl::AppendHyperLink(const CString& sText, const CString& sTitle, const CString& sCommand, const CString& sDirectory)
{
	UNREFERENCED_PARAMETER(sText);
	UNREFERENCED_PARAMETER(sTitle);
	UNREFERENCED_PARAMETER(sDirectory);
	ASSERT( sText.IsEmpty() );
	ASSERT( sTitle.IsEmpty() );
	ASSERT( sDirectory.IsEmpty() );
	AddLine(sCommand, sCommand.GetLength(), true);
}

void CHTRichEditCtrl::AppendColoredText(LPCTSTR pszText, COLORREF cr, COLORREF bk, DWORD mask)
{
	AddLine(pszText, -1, false, cr, bk, mask);
}

void CHTRichEditCtrl::AppendKeyWord(const CString& str, COLORREF cr)
{
	AppendColoredText(str, cr);
}

BOOL CHTRichEditCtrl::OnEnLink(NMHDR *pNMHDR, LRESULT *pResult)
{
	BOOL bMsgHandled = FALSE;
	*pResult = 0;
	ENLINK* pEnLink = reinterpret_cast<ENLINK *>(pNMHDR);
	if (pEnLink && pEnLink->msg == WM_LBUTTONDOWN)
	{
		CString strUrl;
		GetTextRange(pEnLink->chrg.cpMin, pEnLink->chrg.cpMax, strUrl);

		// check if that "URL" has a valid URL scheme. if it does not have, pass that notification up to the
		// parent window which may interpret that "URL" in some other way.
		for (int i = 0; i < ARRSIZE(_apszSchemes); i++){
			if (_tcsncmp(strUrl, _apszSchemes[i].pszScheme, _apszSchemes[i].iLen) == 0){
				ShellExecute(NULL, NULL, strUrl, NULL, NULL, SW_SHOWDEFAULT);
				*pResult = 1;
				bMsgHandled = TRUE; // do not route this message to any parent
				break;
			}
		}
	}
	return bMsgHandled;
}

CString CHTRichEditCtrl::GetText() const
{
	CString strText;
	GetWindowText(strText);
	return strText;
}

void CHTRichEditCtrl::SetFont(CFont* pFont, BOOL bRedraw)
{
	LOGFONT lf = {0};
	pFont->GetLogFont(&lf);

	CHARFORMAT cf = {0};
	cf.cbSize = sizeof cf;

	cf.dwMask |= CFM_BOLD;
	cf.dwEffects |= (lf.lfWeight == FW_BOLD) ? CFE_BOLD : 0;

	cf.dwMask |= CFM_ITALIC;
	cf.dwEffects |= (lf.lfItalic) ? CFE_ITALIC : 0;

	cf.dwMask |= CFM_UNDERLINE;
	cf.dwEffects |= (lf.lfUnderline) ? CFE_UNDERLINE : 0;

	cf.dwMask |= CFM_STRIKEOUT;
	cf.dwEffects |= (lf.lfStrikeOut) ? CFE_STRIKEOUT : 0;

	cf.dwMask |= CFM_SIZE;
	HDC hDC = ::GetDC(NULL);
	int iPointSize = -MulDiv(lf.lfHeight, 72, GetDeviceCaps(hDC, LOGPIXELSY));
	cf.yHeight = iPointSize * 20;
	::ReleaseDC(NULL, hDC);

	cf.dwMask |= CFM_FACE;
	cf.bPitchAndFamily = lf.lfPitchAndFamily;
	_tcsncpy(cf.szFaceName, lf.lfFaceName, ARRSIZE(cf.szFaceName));
	cf.szFaceName[ARRSIZE(cf.szFaceName) - 1] = _T('\0');

	// although this should work correctly (according SDK) it may give false results (e.g. the "click here..." text
	// which is shown in the server info window may not be entirely used as a hyperlink???)
	//	cf.dwMask |= CFM_CHARSET;
	//	cf.bCharSet = lf.lfCharSet;

	cf.yOffset = 0;
	VERIFY( SetDefaultCharFormat(cf) );
	VERIFY( GetSelectionCharFormat(m_cfDefault) );

	if (bRedraw){
		Invalidate();
		UpdateWindow();
	}
}

CFont* CHTRichEditCtrl::GetFont() const
{
	ASSERT(0);
	return NULL;
}

void CHTRichEditCtrl::OnSysColorChange()
{
	CRichEditCtrl::OnSysColorChange();
	ApplySkin();
}

void CHTRichEditCtrl::ApplySkin()
{
	if (!m_strSkinKey.IsEmpty())
	{
		COLORREF cr;
		if (theApp.LoadSkinColor(m_strSkinKey + _T("Bk"), cr))
		{
			SetBackgroundColor(FALSE, cr);
		}
		else
		{
			SetBackgroundColor(TRUE, GetSysColor(COLOR_WINDOW));
		}
	}
}

BOOL CHTRichEditCtrl::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// Cheap workaround for the "Text cursor is showing while context menu is open" glitch. It could be solved properly
	// with the RE's COM interface, but because the according messages are not routed with a unique control ID, it's not
	// really useable (e.g. if there are more RE controls in one window). Would to envelope each RE window to get a unique ID..
	if (m_bForceArrowCursor && m_hArrowCursor)
	{
		::SetCursor(m_hArrowCursor);
		return TRUE;
	}
	return CRichEditCtrl::OnSetCursor(pWnd, nHitTest, message);
}
