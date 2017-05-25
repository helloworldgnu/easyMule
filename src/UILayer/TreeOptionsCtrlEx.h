/*
 * $Id: TreeOptionsCtrlEx.h 4894 2008-03-05 08:02:01Z wangna $
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
#include "TreeOptionsCtrl.h"

typedef struct
{
	NMHDR nmhdr;
	HTREEITEM hItem;
} TREEOPTSCTRLNOTIFY;


// pre defined treeview image list indices
#define	TREEOPTSCTRLIMG_EDIT	11


///////////////////////////////////////////////////////////////////////////////
// CTreeOptionsCtrlEx
class CPPgTweaks;
class CTreeOptionsCtrlEx :
	public CTreeOptionsCtrl
{
public:
	CTreeOptionsCtrlEx(CPropertyPage * ppgTweaks , UINT uImageListColorFlags = ILC_COLOR);
	virtual ~CTreeOptionsCtrlEx(void);

	void SetEditLabel(HTREEITEM hItem, const CString& rstrLabel);
	void UpdateCheckBoxGroup(HTREEITEM hItem);
	void SetImageListColorFlags(UINT uImageListColorFlags);

	virtual void OnCreateImageList();
	virtual void HandleChildControlLosingFocus(bool bLosingFocus = FALSE);
	BOOL NotifyParent(UINT uCode, HTREEITEM hItem);

	virtual void UpdateTreeControlValueFromChildControl(HTREEITEM hItem);

protected:
	UINT m_uImageListColorFlags;

	virtual void HandleCheckBox(HTREEITEM hItem, BOOL bCheck);
	virtual BOOL SetRadioButton(HTREEITEM hParent, int nIndex);
	virtual BOOL SetRadioButton(HTREEITEM hItem);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();

public:
	CPPgTweaks *pPPgTweaks;
	void MinMaxInt(HTREEITEM hItem,int minVal,int maxVal,CString orgVal);
	void MinMaxFloat(HTREEITEM hItem,CString minVal,CString maxVal,CString orgVal);
};

//Dialog Data exchange support

void DDX_TreeCheck(CDataExchange* pDX, int nIDC, HTREEITEM hItem, bool& bCheck);
void DDX_Text(CDataExchange* pDX, int nIDC, HTREEITEM hItem, CString& sText);
void DDX_Text(CDataExchange* pDX, int nIDC, HTREEITEM hItem, int& value);
void DDX_Text(CDataExchange* pDX, int nIDC, HTREEITEM hItem, UINT& value);
void DDX_Text(CDataExchange* pDX, int nIDC, HTREEITEM hItem, long& value);
void DDX_Text(CDataExchange* pDX, int nIDC, HTREEITEM hItem, DWORD& value);
void DDX_Text(CDataExchange* pDX, int nIDC, HTREEITEM hItem, float& value);
void DDX_Text(CDataExchange* pDX, int nIDC, HTREEITEM hItem, double& value);


///////////////////////////////////////////////////////////////////////////////
// CNumTreeOptionsEdit

class CNumTreeOptionsEdit : public CTreeOptionsEdit
{
	DECLARE_DYNCREATE(CNumTreeOptionsEdit)

public:
	CNumTreeOptionsEdit(){}
	virtual ~CNumTreeOptionsEdit(){}

	virtual DWORD GetWindowStyle() { return CTreeOptionsEdit::GetWindowStyle() | ES_NUMBER; }

protected:
	bool m_bSelf;

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnEnChange();

	DECLARE_MESSAGE_MAP()
};


///////////////////////////////////////////////////////////////////////////////
// CTreeOptionsEditEx

class CTreeOptionsEditEx : public CTreeOptionsEdit
{
	DECLARE_DYNCREATE(CTreeOptionsEditEx)

public:
	CTreeOptionsEditEx(){}
	virtual ~CTreeOptionsEditEx(){}

protected:
	bool m_bSelf;

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnEnChange();

	DECLARE_MESSAGE_MAP()
};
