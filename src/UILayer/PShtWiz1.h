/*
 * $Id: PShtWiz1.h 4483 2008-01-02 09:19:06Z soarchin $
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

class CDlgPageWizard : public CPropertyPageEx
{
	DECLARE_DYNCREATE(CDlgPageWizard)

	// Construction
public:
	CDlgPageWizard();
	CDlgPageWizard(UINT nIDTemplate, LPCTSTR pszCaption = NULL, LPCTSTR pszHeaderTitle = NULL, LPCTSTR pszHeaderSubTitle = NULL)
		: CPropertyPageEx(nIDTemplate)
	{
		if (pszCaption)
		{
			m_strCaption = pszCaption; // "convenience storage"
			m_psp.pszTitle = m_strCaption;
			m_psp.dwFlags |= PSP_USETITLE;
		}
		if (pszHeaderTitle && pszHeaderTitle[0] != _T('\0'))
		{
			m_strHeaderTitle = pszHeaderTitle;
			m_psp.dwSize = sizeof(m_psp);
		}
		if (pszHeaderSubTitle && pszHeaderSubTitle[0] != _T('\0'))
		{
			m_strHeaderSubTitle = pszHeaderSubTitle;
			m_psp.dwSize = sizeof(m_psp);
		}
	}
	~CDlgPageWizard(){}

	// Dialog Data


	// Overrides
public:
	virtual BOOL OnSetActive();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// Implementation
protected:
	CString m_strCaption;
	DECLARE_MESSAGE_MAP()

};
