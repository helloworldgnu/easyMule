/*
 * $Id: DetailInfo.cpp 5129 2008-03-25 10:40:33Z thilon $
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
// DetailInfo.cpp : 实现文件
//

#include "stdafx.h"
#include "emule.h"
#include "DetailInfo.h"
#include "shahashset.h"
#include "MemDC.h"
#include ".\detailinfo.h"
#include "PartFile.h"
#include "GlobalVariable.h"
#include "CmdFuncs.h"
// CDetailInfo 对话框

IMPLEMENT_DYNAMIC(CDetailInfo, CDialog)
CDetailInfo::CDetailInfo(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CDetailInfo::IDD, pParent)
{
	m_pCurFile = NULL;
	m_dwCurMask = 0;
	m_pMenuXP = NULL;
}

CDetailInfo::~CDetailInfo()
{
	if(m_pMenuXP)
	{
		delete m_pMenuXP;
	}
}

void CDetailInfo::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_DETAILINFO, m_ListDetail);
}


BEGIN_MESSAGE_MAP(CDetailInfo, CResizableDialog)
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(MP_COPYSELECTED, OnCopySelected)
	ON_COMMAND(MP_SELECTALL, OnSelectAll)
	ON_WM_DRAWITEM()
	ON_WM_MEASUREITEM()
END_MESSAGE_MAP()

// CDetailInfo 消息处理程序
BOOL CDetailInfo::OnInitDialog()
{
	CResizableDialog::OnInitDialog();

	m_ListDetail.SetExtendedStyle(LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT);

	m_ListDetail.InsertColumn(0, GetResString(IDS_DETAILINFO_NAME), LVCFMT_LEFT, 120);
	m_ListDetail.InsertColumn(1, GetResString(IDS_DETAILINFO_VALUE), LVCFMT_LEFT, 800);

	//CString strItemName[11] = {_T("File Name"), _T("Size"), _T("Type"),
	//_T("Ed2k Link"), _T("Priority"), _T("File ID"), _T("Requests"), 
	//_T("Transferred Data"), _T("Folder"), _T("Accepted Requests"), 
	//_T("Complete Sources")};

	//for (int i = 0; i < 11; i++)
	//{
	//	m_ItemIndex[i] = m_ListDetail.InsertItem(i, strItemName[i]);
	//}
	AddAnchor(m_ListDetail,TOP_LEFT,BOTTOM_RIGHT);

	return TRUE;
}


CString	CDetailInfo::GetLink(CKnownFile* pFile)
{
	//ADDED by fengwen on 2007/07/12 <begin>	:	如果是url下载，则返回源url。
	if (NULL == pFile)
		return _T("");

	if (pFile->IsPartFile())
	{
		CPartFile	*pPartFile = (CPartFile	*) pFile;
		if ( pPartFile->HasNullHash() )
			return pPartFile->GetPartFileURL();
	}
	//ADDED by fengwen on 2007/07/12 <end>	:	如果是url下载，则返回源url。

	CString strLinks;
	CString strBuffer;

	bool bHashset = false;
	bool bHTML = false;
//	bool bSource = true;
//	bool bHostname  = true;
	bool bEMHash = true;

	if (!strLinks.IsEmpty())
		strLinks += _T("\r\n\r\n");

	if (bHTML)
		strLinks += _T("<a href=\"");

	//const CKnownFile* file = STATIC_DOWNCAST(CKnownFile, (*pFile)[i]);
	const CKnownFile* file = pFile;
	strLinks += CreateED2kLink(file, false);

	if (bHashset && file->GetHashCount() > 0 && file->GetHashCount() == file->GetED2KPartHashCount())
	{
		strLinks += _T("p=");
		for (UINT j = 0; j < file->GetHashCount(); j++)
		{
			if (j > 0)
				strLinks += _T(':');
			strLinks += EncodeBase16(file->GetPartHash(j), 16);
		}
		strLinks += _T('|');
	}

	if (bEMHash && file->GetAICHHashset()->HasValidMasterHash() && 
		(file->GetAICHHashset()->GetStatus() == AICH_VERIFIED || file->GetAICHHashset()->GetStatus() == AICH_HASHSETCOMPLETE))
	{
		strBuffer.Format(_T("h=%s|"), file->GetAICHHashset()->GetMasterHash().GetString() );
		strLinks += strBuffer;			
	}
    if(!strLinks.IsEmpty())
	    strLinks += _T('/');
	//if (bHostname)
	//{
	//	strBuffer.Format(_T("|sources,%s:%i|/"), thePrefs.GetYourHostname(), thePrefs.GetPort() );
	//	strLinks += strBuffer;
	//}
	//else if(bSource)
	//{
	//	uint32 dwID = CGlobalVariable::GetID();
	//	strBuffer.Format(_T("|sources,%i.%i.%i.%i:%i|/"),(uint8)dwID,(uint8)(dwID>>8),(uint8)(dwID>>16),(uint8)(dwID>>24), thePrefs.GetPort() );
	//	strLinks += strBuffer;
	//}

	if (bHTML)
		strLinks += _T("\">") + StripInvalidFilenameChars(file->GetFileName(), true) + _T("</a>");

	return strLinks;
}
void CDetailInfo::FileInfo(CFileTaskItem *pFile)
{
	try
	{
		m_ListDetail.DeleteAllItems();
		if(NULL == pFile)
			return;
		int	iItem = 0;
		CString	str;
		str = pFile->m_FileName;
		m_ListDetail.InsertItem(iItem, GetResString(IDS_DL_FILENAME));
		m_ListDetail.SetItemText(iItem, 1, str);
		iItem++;
		str = CmdFuncs::GetFileSizeDisplayStr(pFile->m_FileSize);
		m_ListDetail.InsertItem(iItem, GetResString(IDS_DL_SIZE));
		m_ListDetail.SetItemText(iItem, 1, str);
		iItem++;
		str = pFile->m_strFilePath;
		str = str.Left( str.ReverseFind(_T('\\')) );
		m_ListDetail.InsertItem(iItem, GetResString(IDS_FOLDER));
		m_ListDetail.SetItemText(iItem,1,str);
		iItem++;
		str = pFile->m_strUrl;
		if(!str.IsEmpty())
		{
		  m_ListDetail.InsertItem(iItem,GetResString(IDS_URL_LINK));
		  m_ListDetail.SetItemText(iItem,1,str);
		}
		str = pFile->m_strEd2kLink;
		if(!str.IsEmpty())
		{			
			m_ListDetail.InsertItem(iItem,GetResString(IDS_DOWNLOAD_LINK));
			m_ListDetail.SetItemText(iItem,1,str);
		}
	}
	catch(...)
	{
	}
}
void CDetailInfo::UpdateInfo(CPartFile* pFile, DWORD dwMask)
{
	try
	{
		m_ListDetail.DeleteAllItems();

		if (NULL == pFile || 0 == dwMask)
			return;

		m_pCurFile = pFile;
		m_dwCurMask = dwMask;

		int	iItem;
		CString	str;
		CPartFile	*lpPartFile = pFile;//DYNAMIC_DOWNCAST(CPartFile, pFile);

		iItem = 0;

		if (IM_FILENAME & dwMask)
		{
			str = pFile->GetFileName();
			m_ListDetail.InsertItem(iItem, GetResString(IDS_DL_FILENAME));
			m_ListDetail.SetItemText(iItem, 1, str);
			iItem++;
		}
		if (IM_FILESIZE & dwMask)
		{
			str = CastItoXBytes(pFile->GetFileSize(), false, false);
			m_ListDetail.InsertItem(iItem, GetResString(IDS_DL_SIZE));
			if(!str.IsEmpty())
			    m_ListDetail.SetItemText(iItem, 1, str);
			else
				m_ListDetail.SetItemText(iItem, 1, GetResString(IDS_UNKNOWN));
			iItem++;
		}
		if (IM_FILETYPE & dwMask)
		{
			str = pFile->GetFileTypeDisplayStr();
			m_ListDetail.InsertItem(iItem, GetResString(IDS_TYPE));
			if(!str.IsEmpty())
			    m_ListDetail.SetItemText(iItem, 1, str);
			else
				m_ListDetail.SetItemText(iItem, 1, GetResString(IDS_UNKNOWN));
			iItem++;
		}
		
		if (IM_LINK & dwMask)
		{   
			str = GetLink(pFile);
			bool bRefer = false;
			if(str.Left(7).CompareNoCase(_T("http://")) == 0)
			{  
				if(str.Find(_T('<'))>0)
				{   
					bRefer = true;
					str = str.Left(str.Find(_T('<')));
				}
				if(str.Find(_T('#'))>0)
					str = str.Left(str.Find(_T('#')));
			}
			if(!str.IsEmpty())
			{
				m_ListDetail.InsertItem(iItem, GetResString(IDS_DOWNLOAD_LINK));
				m_ListDetail.SetItemText(iItem, 1, str);
				iItem++;
			}
			if(bRefer)
			{
				
				CString refer = GetLink(pFile);
				refer = refer.Right(refer.GetLength() - 1 - refer.Find(_T('=')));
				if(refer.Find(_T('>')) > 0)
					refer.Remove(_T('>'));
				m_ListDetail.InsertItem(iItem,GetResString(IDS_REFER_LINK));
				m_ListDetail.SetItemText(iItem,1,refer);
				iItem++;
			}
		}
		if (IM_SOURCEURL & dwMask)
		{
			str = pFile->GetPartFileURL();
			bool bRefer = false;
			if(str.Find(_T('<'))>0)
			{   
				bRefer = true;
				str = str.Left(str.Find(_T('<')));
			}
			if(str.Find(_T('#'))>0)
				str = str.Left(str.Find(_T('#')));
			if(!str.IsEmpty())
			{
		       m_ListDetail.InsertItem(iItem, GetResString(IDS_URL_LINK));
		       m_ListDetail.SetItemText(iItem, 1, str);
			   iItem++;
			}
			if(bRefer)
			{   
			   CString strRefer = pFile->GetPartFileURL();
			   strRefer = strRefer.Right(strRefer.GetLength() -1 - strRefer.Find(_T('=')  ));
			   if(strRefer.Find(_T('>'))>0)
				   strRefer.Remove(_T('>'));
				m_ListDetail.InsertItem(iItem,GetResString(IDS_REFER_LINK));
				m_ListDetail.SetItemText(iItem,1,strRefer);
				iItem++;
			}
		}
		if (IM_PRIORITY & dwMask)
		{ 
			if(dwMask == CDetailInfo::IM_COMBINE_DOWNLOAD)
			      str = PriorityToString(pFile->GetDownPriority(), pFile->IsAutoDownPriority());
			if(dwMask == CDetailInfo::IM_COMBINE_SHARE)
				str = PriorityToString(pFile->GetUpPriority(),pFile->IsAutoUpPriority());
			m_ListDetail.InsertItem(iItem, GetResString(IDS_PRIORITY));
			m_ListDetail.SetItemText(iItem, 1, str);
			iItem++;
		}
		if (IM_FILEHASH & dwMask)
		{
			if (pFile->HasNullHash())
			{
				str = _T("-");
			}
			else
			{
				str = md4str(pFile->GetFileHash());
			}
			
			m_ListDetail.InsertItem(iItem, GetResString(IDS_FILEID));
			m_ListDetail.SetItemText(iItem, 1, str);
			iItem++;
		}
		if (IM_REQUEST & dwMask)
		{
			str.Format(_T("%u (%u)"), pFile->statistic.GetRequests(), pFile->statistic.GetAllTimeRequests());
			m_ListDetail.InsertItem(iItem, GetResString(IDS_SF_REQUESTS));
			m_ListDetail.SetItemText(iItem, 1, str);
			iItem++;
		}
		if (IM_TRANSFERED & dwMask)
		{
			str.Format(_T("%s (%s)"), CastItoXBytes(pFile->statistic.GetTransferred(), false, false), CastItoXBytes(pFile->statistic.GetAllTimeTransferred(), false, false));
			m_ListDetail.InsertItem(iItem, GetResString(IDS_SF_TRANSFERRED));
			m_ListDetail.SetItemText(iItem, 1, str);
			iItem++;
		}
		if (IM_FILEPATH & dwMask)
		{
			str = pFile->GetPath();
			PathRemoveBackslash(str.GetBuffer());
			str.ReleaseBuffer();
			m_ListDetail.InsertItem(iItem, GetResString(IDS_FOLDER));
			m_ListDetail.SetItemText(iItem, 1, str);
			iItem++;
		}
		if (IM_ACCEPT & dwMask)
		{
			str.Format(_T("%u (%u)"), pFile->statistic.GetAccepts(), pFile->statistic.GetAllTimeAccepts());
			m_ListDetail.InsertItem(iItem, GetResString(IDS_SF_ACCEPTS));
			m_ListDetail.SetItemText(iItem, 1, str);
			iItem++;
		}
		if (IM_SOURCE & dwMask)
		{
			str.Format(_T("%u - %u"), pFile->m_nCompleteSourcesCountLo, pFile->m_nCompleteSourcesCountHi);
			m_ListDetail.InsertItem(iItem, GetResString(IDS_COMPLSOURCES));
			m_ListDetail.SetItemText(iItem, 1, str);
			iItem++;
		}
		if (IM_REMAIN & dwMask)
		{
			if (NULL != lpPartFile)
			{
				str.Empty();
				if (lpPartFile->GetStatus() != PS_COMPLETING && lpPartFile->GetStatus() != PS_COMPLETE )
				{
					// time 
					time_t restTime;
					if (!thePrefs.UseSimpleTimeRemainingComputation())
						restTime = lpPartFile->getTimeRemaining();
					else
						restTime = lpPartFile->getTimeRemainingSimple();

					str.Format(_T("%s (%s)"), CastSecondsToHM(restTime), CastItoXBytes((lpPartFile->GetFileSize() - lpPartFile->GetCompletedSize()), false, false));
				}
				else
				{
					str.Format(_T("%s (%s)"), _T("0"), CastItoXBytes((lpPartFile->GetFileSize() - lpPartFile->GetCompletedSize()), false, false));
				}
				m_ListDetail.InsertItem(iItem, GetResString(IDS_DL_REMAINS));
				if(!str.IsEmpty())
				   m_ListDetail.SetItemText(iItem, 1, str);
				else
					m_ListDetail.SetItemText(iItem,1,GetResString(IDS_UNKNOWN));
				iItem++;
			}

		}
		if (IM_LASTCOMPLETE & dwMask)
		{
			if (NULL != lpPartFile)
			{
//				CString tempbuffer;
//				if (lpPartFile->m_nCompleteSourcesCountLo == 0)
//				{
//					tempbuffer.Format(_T("< %u"), lpPartFile->m_nCompleteSourcesCountHi);
//				}
//				else if (lpPartFile->m_nCompleteSourcesCountLo == lpPartFile->m_nCompleteSourcesCountHi)
//				{
//					tempbuffer.Format(_T("%u"), lpPartFile->m_nCompleteSourcesCountLo);
//				}
//				else
//				{
//					tempbuffer.Format(_T("%u - %u"), lpPartFile->m_nCompleteSourcesCountLo, lpPartFile->m_nCompleteSourcesCountHi);
//				}
				if (lpPartFile->lastseencomplete==NULL)
					str.Format(_T("%s" /*(%s)*/),GetResString(IDS_NEVER)/*,tempbuffer*/);
				else
					str.Format(_T("%s" /*(%s)*/),lpPartFile->lastseencomplete.Format( thePrefs.GetDateTimeFormat())/*,tempbuffer*/);

				m_ListDetail.InsertItem(iItem, GetResString(IDS_LASTSEENCOMPL));
				m_ListDetail.SetItemText(iItem, 1, str);
				iItem++;
			}

		}
		if (IM_LASTRECV & dwMask)
		{
			if (NULL != lpPartFile)
			{
				if(lpPartFile->GetFileDate()!=NULL && lpPartFile->GetCompletedSize() > (uint64)0)
					str = lpPartFile->GetCFileDate().Format( thePrefs.GetDateTimeFormat());
				else
					str.Format(_T("%s"),GetResString(IDS_NEVER));

				m_ListDetail.InsertItem(iItem, GetResString(IDS_FD_LASTCHANGE));
				m_ListDetail.SetItemText(iItem, 1, str);
				iItem++;
			}
		}
		if (IM_CATEGORY & dwMask)
		{
			if (NULL != lpPartFile)
			{
				str = (lpPartFile->GetCategory()!=0) ? thePrefs.GetCategory(lpPartFile->GetCategory())->strTitle:_T("");

				m_ListDetail.InsertItem(iItem, GetResString(IDS_CAT));
				m_ListDetail.SetItemText(iItem, 1, str);
				iItem++;
			}
		}
	}
	catch(...)
	{
	}
}


void CDetailInfo::OnSize(UINT nType, int cx, int cy)
{
	CResizableDialog::OnSize(nType, cx, cy);

	if (m_ListDetail.GetSafeHwnd())
	{
		CRect rect;
		GetClientRect(rect);
		m_ListDetail.SetColumnWidth(1, rect.right - 120);
	}
	Invalidate(FALSE);
}

void CDetailInfo::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	CMenu menu; // = new CMenu();

	menu.CreatePopupMenu();
	
	menu.AppendMenu(MF_STRING, MP_COPYSELECTED, GetResString(IDS_COPY));
	menu.AppendMenu(MF_STRING, MP_SELECTALL, GetResString(IDS_SELECTALL));

	m_pMenuXP = new CMenuXP();
	m_pMenuXP->AddMenu(&menu, TRUE);
	menu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);
	delete m_pMenuXP;
	m_pMenuXP = NULL;
}

void CDetailInfo::OnCopySelected()
{
	POSITION pos = m_ListDetail.GetFirstSelectedItemPosition();
	
	CString strText = _T("");

	if (pos == NULL)
	{
		TRACE0("No items were selected!\n");
		return; // Must Return Here 
	}
	else
	{	
		while (pos)
		{
			int nItem = m_ListDetail.GetNextSelectedItem(pos);
			TRACE1("Item %d was selected!\n", nItem);
			// you could do your own processing on nItem here

			strText += m_ListDetail.GetItemText(nItem, 1);

			strText += '\n';
		}
	}

    if (strText.GetAt(strText.GetLength() - 1) == '\n')
	{
		strText.Delete(strText.GetLength() - 1);
	}

	theApp.CopyTextToClipboard(strText);
}

void CDetailInfo::OnSelectAll()
{
	for (int i = 0; i < 11; i++)
	{
		/*m_ListDetail.SetItemState(m_ItemIndex[i], LVIS_SELECTED, LVIS_SELECTED);*/
		m_ListDetail.SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
	}
}

void CDetailInfo::Localize()
{
	if (NULL == GetSafeHwnd())
		return;

	LVCOLUMN	lc;
	CString		str;

	lc.mask = LVCF_TEXT;

	str = GetResString(IDS_DETAILINFO_NAME);
	lc.pszText = str.LockBuffer();
	m_ListDetail.SetColumn(0, &lc);
	str.UnlockBuffer();

	str = GetResString(IDS_DETAILINFO_VALUE);
	lc.pszText = str.LockBuffer();
	m_ListDetail.SetColumn(1, &lc);
	str.UnlockBuffer();

	UpdateInfo(m_pCurFile, m_dwCurMask);
}

void CDetailInfo::OnDrawItem(int /*nIDCtl*/, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	m_pMenuXP->DrawItem(lpDrawItemStruct);
}

void CDetailInfo::OnMeasureItem(int /*nIDCtl*/, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	m_pMenuXP->MeasureItem(lpMeasureItemStruct);
}
