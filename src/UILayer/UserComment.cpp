/*
 * $Id: UserComment.cpp 7232 2008-09-11 10:39:39Z huby $
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
// UserComment.cpp : 实现文件
//

#include "stdafx.h"
#include "emule.h"
#include "UserComment.h"
#include ".\usercomment.h"
#include "Version.h"


// CUserComment 对话框

IMPLEMENT_DYNAMIC(CUserComment, CDialog)
CUserComment::CUserComment(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CUserComment::IDD, pParent)
{
	m_pwebUserComment = 0;
	m_strLastCommentWeb = _T("");
}

CUserComment::~CUserComment()
{
}

void CUserComment::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CUserComment, CResizableDialog)
	ON_WM_SIZE()
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////
// Added by thilon on 2007.02.14, for Resize


// CUserComment 消息处理程序

BOOL CUserComment::OnInitDialog()
{
	CResizableDialog::OnInitDialog();

	if(! m_pwebUserComment)
	{
		//  don't delete it, auto-deleted
		m_pwebUserComment = new CHtmlCtrl;
		CRect rect;  
		GetClientRect(&rect);
		m_pwebUserComment->Create(NULL, NULL ,WS_CHILD | WS_VISIBLE &~WS_BORDER, rect,this, 34346,NULL);
		m_pwebUserComment->SetSilent(true);
	}

	AddAnchor(m_pwebUserComment->m_hWnd,TOP_LEFT, BOTTOM_RIGHT);
	InitControlContainer();
	return TRUE;
}

void CUserComment::Refresh(const CKnownFile * file)
{
/*
	if(m_pwebUserComment)
	{
		CString strURL = _T("http://www.verycd.com/files/");

		strURL += CreateED2kLink(file, false);
		strURL.Replace(_T("|"), _T("%7C"));
		m_pwebUserComment->Navigate2(strURL, 0, NULL);
	}*/
	
	if( !m_pwebUserComment || !file )
		return;
	CString strFileEd2k = CreateED2kLink(file, false);
	//{begin} VC-dgkang 2008年9月5日
	if( strFileEd2k.IsEmpty() && m_strLastCommentWeb != _T("about:blank"))
	{
		m_pwebUserComment->Navigate2(_T("about:blank"), 0, NULL);
		m_strLastCommentWeb = _T("about:blank");
		return;
	}
	//{end}
	bool bFileisFinished = true;
	if( file->IsKindOf(RUNTIME_CLASS(CPartFile)) )
	{
		if( ((CPartFile*)file)->getPartfileStatus()!=PS_COMPLETE )
			bFileisFinished = false;
	}

	CString strCommentUrl = bFileisFinished ? thePrefs.m_strFinishedFileCommentUrl : thePrefs.m_strPartFileCommentUrl;
	strCommentUrl.Replace(_T("[ed2k]"),strFileEd2k);
	strCommentUrl.Replace(_T("|"), _T("%7C"));

	CString sVersion;
	sVersion.Format(_T("&v=%u"),VC_VERSION_BUILD);
	strCommentUrl += sVersion;

	//{begin} VC-dgkang 2008年9月5日
	if (m_strLastCommentWeb != strCommentUrl)
	{
		m_pwebUserComment->Navigate2(strCommentUrl, 0, NULL);
		m_strLastCommentWeb = strCommentUrl;
	}
	//{end}
}

void CUserComment::OnSize(UINT nType, int cx, int cy)
{
	CResizableDialog::OnSize(nType, cx, cy);
	Invalidate(FALSE);
}
