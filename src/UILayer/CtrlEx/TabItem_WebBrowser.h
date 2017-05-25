/*
 * $Id: TabItem_WebBrowser.h 6147 2008-07-10 03:16:33Z dgkang $
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
#include "TabItem_NormalCloseable.h"
#include "WBNotifyReceiver.h"
#include "WebBrowserWnd.h"

class CTabItem_WebBrowser : public CTabItem_NormalCloseable, public CWBNotifyReceiver
{
public:
	CTabItem_WebBrowser(void);
	~CTabItem_WebBrowser(void);

	virtual int GetDesireLength();
	void	SetWbWnd(CWebBrowserWnd *pWbw);

	CString GetUrl() const{
		if (m_pWbw)
			return m_pWbw->GetUrl();
		return _T("");
	}

	CString GetRealUrl() {
		if (m_pWbw)
			return m_pWbw->GetRealUrl();
		return _T("");
	}

	CWebBrowserWnd*	GetAssocWbw(){return m_pWbw;}
protected:

	//void	UpdateCaption();
	CWebBrowserWnd	*m_pWbw;
	//int				m_iProgressPercent;
	//CString			m_strTitle;

	//enum {ICON_COUNT = 5};
	//HICON			m_arrProgressIcons[ICON_COUNT];
};
