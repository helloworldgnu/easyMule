/*
 * $Id: ShortcutManager.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
#include "StdAfx.h"
#include "ShortcutManager.h"

CShortcutManager::CShortcutManager(BOOL bAutoSendCmds)
{
	m_bAutoSendCmds = bAutoSendCmds;
	m_wInvalidComb = 0;
	m_wFallbackModifiers = 0;

	m_hWndHooked  = NULL;
	m_bPreMFCSubclass = FALSE;

}

CShortcutManager::~CShortcutManager(void)
{
	if (m_hWndHooked)
	{
		HookWindow((HWND)NULL);
	}

	ASSERT(m_hWndHooked==NULL);
}

BOOL CShortcutManager::AddShortcut(UINT nCmdID, DWORD dwShortcut)
{
	return AddShortcut(nCmdID, LOWORD(dwShortcut), HIWORD(dwShortcut));
}

BOOL CShortcutManager::AddShortcut(UINT nCmdID, WORD wVirtKeyCode, WORD wModifiers)
{
	// 是否无效的修改
	if (ValidateModifiers(wModifiers, wVirtKeyCode) != wModifiers)
	{
		return FALSE;
	}

	// 快捷键是否已经使用
	DWORD dwShortcut = MAKELONG(wVirtKeyCode, wModifiers);

	if (!nCmdID || !dwShortcut)
	{
		return FALSE;
	}

	UINT nOtherCmdID = 0;

	if (m_mapShortcut2ID.Lookup(dwShortcut, nOtherCmdID) && nOtherCmdID)
	{
		return FALSE;
	}

	m_mapShortcut2ID[dwShortcut] = nCmdID;

	return TRUE;
}

void CShortcutManager::SetShortcut(UINT nCmdID, WORD wVirtKeyCode, WORD wModifiers)
{
	UINT nOtherCmdID = 0;
	DWORD dwShortcut = MAKELONG(wVirtKeyCode, wModifiers);

	if (!dwShortcut)
	{
		m_mapShortcut2ID.RemoveKey(dwShortcut);
		return;
	}
	else 
	{
		if (m_mapShortcut2ID.Lookup(dwShortcut, nOtherCmdID))
		{
			m_mapShortcut2ID.RemoveKey(dwShortcut);
		}	
	}

	AddShortcut(nCmdID, wVirtKeyCode, wModifiers);
}

void CShortcutManager::SetShortcut(UINT nCmdID, DWORD dwShortcut)
{
	SetShortcut(nCmdID, LOWORD(dwShortcut), HIWORD(dwShortcut));
}

WORD CShortcutManager::ValidateModifiers(WORD wModifiers, WORD wVirtKeyCode) const
{
	if (!m_wInvalidComb)
	{
		return wModifiers;
	}

	if ((m_wInvalidComb & HKCOMB_EXFKEYS) && (wVirtKeyCode >= VK_F1 && wVirtKeyCode <= VK_F24))
	{
		return wModifiers;
	}

	// 检测无效的联合
	BOOL bCtrl = (wModifiers & HOTKEYF_CONTROL);
	BOOL bShift = (wModifiers & HOTKEYF_SHIFT);
	BOOL bAlt = (wModifiers & HOTKEYF_ALT);
	BOOL bExtended = (wModifiers & HOTKEYF_EXT);

	BOOL bFail = ((m_wInvalidComb & HKCOMB_NONE) && !bCtrl && !bShift && !bAlt);

	bFail |= ((m_wInvalidComb & HKCOMB_S) && !bCtrl && bShift && !bAlt);
	bFail |= ((m_wInvalidComb & HKCOMB_C) && bCtrl && !bShift && !bAlt);
	bFail |= ((m_wInvalidComb & HKCOMB_A) && !bCtrl && !bShift && bAlt);
	bFail |= ((m_wInvalidComb & HKCOMB_SC) && bCtrl && bShift && !bAlt);
	bFail |= ((m_wInvalidComb & HKCOMB_SA) && !bCtrl && bShift && bAlt);
	bFail |= ((m_wInvalidComb & HKCOMB_CA) && bCtrl && !bShift && bAlt);
	bFail |= ((m_wInvalidComb & HKCOMB_SCA) && bCtrl && bShift && bAlt);

	if (bFail)
	{
		return (WORD)(m_wFallbackModifiers | (bExtended ? HOTKEYF_EXT : 0x0));
	}

	return wModifiers;
}

UINT CShortcutManager::ProcessMessage(const MSG* pMsg, DWORD* pShortcut) const
{
	// 只处理可用的快捷键
	if (!IsWindowEnabled() || !IsWindowVisible())
	{
		return FALSE;
	}

	// 只处理键盘消息
	if (pMsg->message != WM_KEYDOWN && pMsg->message != WM_SYSKEYDOWN)
	{
		return FALSE;
	}

	CWnd* pWnd = CWnd::FromHandle(pMsg->hwnd);

	CWnd* pMainWnd = GetCWnd();
	CWnd* pTopParent = pWnd->GetParentOwner();

	if (pTopParent != pMainWnd)
	{
		return FALSE;
	}

	switch (pMsg->wParam)
	{
	case VK_CONTROL:
	case VK_SHIFT:
	case VK_MENU:
	case VK_NUMLOCK:
	case VK_SCROLL:
	case VK_CAPITAL:
		return FALSE;

		// 不去处理 return/cancel 键
	case VK_RETURN:
	case VK_CANCEL:
		return FALSE;

	case VK_MBUTTON:
		break;

		// 快捷键
	default: 
		{
			//不去处理发往hotkey控件的消息
			if (IsClass(pMsg->hwnd, WC_HOTKEY))
			{
				return FALSE;
			}

			// get 获取快捷方式DWORD值
			BOOL bExtKey = (pMsg->lParam & 0x01000000);
			DWORD dwShortcut = GetShortcut((WORD)pMsg->wParam, bExtKey);

			// 查找相应的命令ID
			UINT nCmdID = 0;

			if (!m_mapShortcut2ID.Lookup(dwShortcut, nCmdID) || !nCmdID)
			{
				return FALSE;
			}

			if (m_wInvalidComb & HKCOMB_EDITCTRLS)
			{
				if (IsEditControl(pMsg->hwnd))
				{
					if (IsEditShortcut(dwShortcut))
					{
						return FALSE;
					}

					WORD wModifiers = HIWORD(dwShortcut);

					if (pMsg->wParam >= VK_F1 && pMsg->wParam <= VK_F24)
					{
						// ok
					}
					// 3. else must have <ctrl> or <alt>
					else
					{
						if (!(wModifiers & (HOTKEYF_ALT | HOTKEYF_CONTROL)))
						{
							return FALSE;
						}
					}
				}
			}

			// 返回 command ID
			if (m_bAutoSendCmds)
			{
				SendMessage(NULL, WM_COMMAND, nCmdID, 0);
			}

			if (pShortcut)
			{
				*pShortcut = dwShortcut;
			}

			return nCmdID;
		}
	}

	return FALSE;
}

BOOL CShortcutManager::IsEditShortcut(DWORD dwShortcut)
{
	switch (dwShortcut)
	{
		case MAKELONG('C', HOTKEYF_CONTROL): // 复制
		case MAKELONG('V', HOTKEYF_CONTROL): // 粘贴
		case MAKELONG('X', HOTKEYF_CONTROL): // 剪切
		case MAKELONG('Z', HOTKEYF_CONTROL): // 撤销
		case MAKELONG(VK_LEFT, HOTKEYF_CONTROL | HOTKEYF_EXT): // 从左到右阅读顺序
		case MAKELONG(VK_RIGHT, HOTKEYF_CONTROL | HOTKEYF_EXT): // 从右到左阅读顺序
		case MAKELONG(VK_DELETE, 0):
		return TRUE;
	}

	return FALSE;
}

BOOL CShortcutManager::Initialize(CWnd* pOwner, WORD wInvalidComb, WORD wFallbackModifiers)
{
	if (wInvalidComb && !IsHooked())
	{
		if (pOwner && HookWindow(*pOwner))
		{
			m_wInvalidComb = wInvalidComb;
			m_wFallbackModifiers = wFallbackModifiers;
			//LoadSettings();

			return TRUE;
		}
	}

	return FALSE;
}

BOOL CShortcutManager::Release()
{
	if (!IsHooked())
		return TRUE;

	return HookWindow(NULL);
}

DWORD CShortcutManager::GetShortcut(WORD wVirtKeyCode, BOOL bExtended) const
{
	BOOL bCtrl = (GetKeyState(VK_CONTROL) & 0x8000);
	BOOL bShift = (GetKeyState(VK_SHIFT) & 0x8000);
	BOOL bAlt = (GetKeyState(VK_MENU) & 0x8000);

	WORD wModifiers = (WORD)((bCtrl ? HOTKEYF_CONTROL : 0) | (bShift ? HOTKEYF_SHIFT : 0) | (bAlt ? HOTKEYF_ALT : 0) | (bExtended ? HOTKEYF_EXT : 0));

	return MAKELONG(wVirtKeyCode, wModifiers);
}

BOOL CShortcutManager::IsEditControl(HWND hWnd)
{
	CString sClass = GetClass(hWnd);

	if (IsRichEditControl(sClass))
	{
		return TRUE;
	}

	return IsClass(sClass, WC_EDIT);
}

CString CShortcutManager::GetClass(HWND hWnd)
{
	static CString sWndClass;
	sWndClass.Empty();

	if (hWnd)
	{
		::GetClassName(hWnd, sWndClass.GetBuffer(128), 128);

		sWndClass.ReleaseBuffer();
		sWndClass.MakeLower();
	}

	return sWndClass;
}

BOOL CShortcutManager::IsRichEditControl(HWND hWnd)
{
	return IsRichEditControl(GetClass(hWnd));
}

BOOL CShortcutManager::IsRichEditControl(LPCTSTR szClass)
{
	return (IsClass(szClass, WC_RICHEDIT) || IsClass(szClass, WC_RICHEDIT20));
}

BOOL CShortcutManager::IsClass(HWND hWnd, LPCTSTR szClass)
{
	if (hWnd)
	{
		return IsClass(szClass, GetClass(hWnd));
	}

	return FALSE;
}

BOOL CShortcutManager::HookWindow(HWND hWnd)
{
	if (hWnd) 
	{
		ASSERT(m_hWndHooked == NULL);

		if (m_hWndHooked)
		{
			return FALSE;
		}

		ASSERT(::IsWindow(hWnd));

		if (!::IsWindow(hWnd))
		{
			return FALSE;
		}

		m_hWndHooked = hWnd;
		m_bPreMFCSubclass = (CWnd::FromHandlePermanent(hWnd) == NULL);
	} 
	else 
	{
		// Unhook the window
		if (m_hWndHooked) 
		{
			CWnd* pPerm = CWnd::FromHandlePermanent(m_hWndHooked);

			if (m_bPreMFCSubclass && pPerm)
			{
				pPerm->UnsubclassWindow();
			}

			if (m_bPreMFCSubclass && pPerm)
			{
				pPerm->SubclassWindow(m_hWndHooked);
			}
		}

		m_hWndHooked = NULL;
		m_bPreMFCSubclass = FALSE;
	}

	return TRUE;
}
