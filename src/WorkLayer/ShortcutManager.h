/*
 * $Id: ShortcutManager.h 4483 2008-01-02 09:19:06Z soarchin $
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

////////////////////////////////////////////////////////////////////////////
#define WC_HOTKEYA              "msctls_hotkey32"	// ansi
#define WC_HOTKEYW              L"msctls_hotkey32"	// wide

#ifdef UNICODE
#define WC_HOTKEY               WC_HOTKEYW
#else
#define WC_HOTKEY               WC_HOTKEYA
#endif

////////////////////////////////////////////////////////////////////////////
#define WC_RICHEDITA              "Richedit"	// ansi
#define WC_RICHEDITW              L"Richedit"	// wide

#ifdef UNICODE
#define WC_RICHEDIT               WC_RICHEDITW
#else
#define WC_RICHEDIT               WC_RICHEDITA
#endif

/////////////////////////////////////////////////////////////////////////////
#define WC_RICHEDIT20A              "RichEdit20A"	// ansi
#define WC_RICHEDIT20W              L"RichEdit20W"	// wide

#ifdef UNICODE
#define WC_RICHEDIT20               WC_RICHEDIT20W
#else
#define WC_RICHEDIT20               WC_RICHEDIT20A
#endif
enum 
{
	HKCOMB_EXFKEYS		= 0x0100,
	HKCOMB_EDITCTRLS	= 0x0200, // 防止在编辑框中快捷键冲突
};

class CShortcutManager
{
public:
	CShortcutManager(BOOL bAutoSendCmds = 1);
	virtual ~CShortcutManager(void);

//Class
protected:
	HWND			m_hWndHooked;
	BOOL			m_bPreMFCSubclass;

protected:
	static CString GetClass(HWND hWnd);
	static BOOL IsEditControl(HWND hWnd);
	static BOOL IsRichEditControl(HWND hWnd);

	virtual BOOL HookWindow(HWND hRealWnd);
	virtual BOOL IsHooked() const { return m_hWndHooked != NULL; }

	inline CWnd* GetCWnd() const { return CWnd::FromHandle(m_hWndHooked); }


	inline BOOL IsWindowEnabled() const { return ::IsWindowEnabled(m_hWndHooked); }
	inline BOOL IsWindowVisible() const { return ::IsWindowVisible(m_hWndHooked); }

	static BOOL IsRichEditControl(LPCTSTR szClass);
	static BOOL IsEditShortcut(DWORD dwShortcut);

	static BOOL IsClass(HWND hWnd, LPCTSTR szClass);
	static BOOL IsClass(LPCTSTR szClass, LPCTSTR szWndClass) { return (lstrcmpi(szClass, szWndClass) == 0); }

public:
	BOOL Initialize(CWnd* pOwner, WORD wInvalidComb = HKCOMB_EDITCTRLS, WORD wFallbackModifiers = 0);
	BOOL Release();

	BOOL AddShortcut(UINT nCmdID, WORD wVirtKeyCode, WORD wModifiers = HOTKEYF_CONTROL); 
	BOOL AddShortcut(UINT nCmdID, DWORD dwShortcut);

	void SetShortcut(UINT nCmdID, WORD wVirtKeyCode, WORD wModifiers = HOTKEYF_CONTROL); 
	void SetShortcut(UINT nCmdID, DWORD dwShortcut); 

	WORD ValidateModifiers(WORD wModifiers, WORD wVirtKeyCode) const;

	//在PreTranslateMessage函数中调用，返回CmdID，或者0
	UINT ProcessMessage(const MSG* pMsg, DWORD* pShortcut) const;

protected:

	DWORD GetShortcut(WORD wVirtKeyCode, BOOL bExtended) const;
protected:
	CMap<DWORD, DWORD, UINT, UINT&> m_mapShortcut2ID;
	WORD m_wInvalidComb;
	WORD m_wFallbackModifiers;

	BOOL m_bAutoSendCmds;
};
