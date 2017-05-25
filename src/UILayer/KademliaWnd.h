/*
 * $Id: KademliaWnd.h 4483 2008-01-02 09:19:06Z soarchin $
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
#include "ResizableLib\ResizableDialog.h"
#include "IconStatic.h"
#include "kademlia/routing/contact.h"
#include "resource.h"

class CKadContactListCtrl;
class CKadContactHistogramCtrl;
class CKadSearchListCtrl;
class CCustomAutoComplete;

class CKademliaWnd : public CResizableDialog
{
	DECLARE_DYNAMIC(CKademliaWnd)

public:
	CKademliaWnd(CWnd* pParent = NULL);   // standard constructor
	virtual ~CKademliaWnd();

	// Dialog Data
	enum { IDD = IDD_KADEMLIAWND };

	// Contacts
	UINT GetContactCount() const;
	void UpdateKadContactCount();
	void ShowContacts();
	void HideContacts();
	bool ContactAdd(const Kademlia::CContact* contact);
	void ContactRem(const Kademlia::CContact* contact);
	void ContactRef(const Kademlia::CContact* contact);

	// Searches
	CKadSearchListCtrl* searchList;

	void Localize();
	void UpdateControlsState();
	BOOL SaveAllSettings();

protected:
	CStatic kadContactLab;
	CStatic kadSearchLab;
	CIconStatic m_ctrlBootstrap;
	CKadContactListCtrl* m_contactListCtrl;
	CKadContactHistogramCtrl* m_contactHistogramCtrl;
	CCustomAutoComplete* m_pacONBSIPs;
	HICON icon_kadcont;
	HICON icon_kadsea;

	void SetAllIcons();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedBootstrapbutton();
	afx_msg void OnBnConnect();
	afx_msg void OnBnClickedFirewallcheckbutton();
	afx_msg void OnSysColorChange();
	afx_msg void OnEnSetfocusBootstrapip();
};
