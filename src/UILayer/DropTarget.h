/*
 * $Id: DropTarget.h 4483 2008-01-02 09:19:06Z soarchin $
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

struct PASTEURLDATA;

//////////////////////////////////////////////////////////////////////////////
// CMainFrameDropTarget

class CMainFrameDropTarget : public COleDropTarget
{
public:
	CMainFrameDropTarget();

	virtual DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual BOOL OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
	virtual void OnDragLeave(CWnd* pWnd);

protected:
	BOOL m_bDropDataValid;
	CLIPFORMAT m_cfHTML;
	CLIPFORMAT m_cfShellURL;

	BOOL IsSupportedDropData(COleDataObject* pDataObject);
	HRESULT PasteHTMLDocument(IHTMLDocument2* doc, PASTEURLDATA* pPaste);
	HRESULT PasteHTML(PASTEURLDATA* pPaste);
	HRESULT PasteHTML(COleDataObject &data);
	HRESULT PasteText(CLIPFORMAT cfData, COleDataObject& data);
	HRESULT PasteHDROP(COleDataObject &data);
	HRESULT AddUrlFileContents(LPCTSTR pszFileName);
};
