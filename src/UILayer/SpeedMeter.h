/*
 * $Id: SpeedMeter.h 4483 2008-01-02 09:19:06Z soarchin $
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
#include "afxtempl.h"

// CSpeedMeter

class CSpeedMeter : public CWnd
{
	DECLARE_DYNAMIC(CSpeedMeter)

public:
	CSpeedMeter();
	virtual ~CSpeedMeter();

protected:
	COLORREF	m_crDownload;
	COLORREF	m_crUpload;

	CPen		m_DownloadPen;
	CPen		m_UploadPen;

	CBitmap*	m_pOldBMP;
	CBitmap		m_MemBMP;

	CList<UINT,UINT>	m_UpdataList;
	CList<UINT,UINT>	m_DownloadList;

protected:
	CDC		m_MemDC;

	bool	m_bInit;

	UINT	m_nMax;
	UINT	m_nMin;
	UINT	m_nStep;

protected:
	DECLARE_MESSAGE_MAP()
public:
	void AddValues(UINT uUpdatarate, UINT uDowndatarate, bool bRedraw = true);

	void SetRange(UINT nMin, UINT nMax, bool bDelete = false);
	void GetRange(UINT& nMax, UINT& nMin);

	void ResetGraph(bool bDelete = true);


protected:
	void AddValues2Graph(UINT uUpdatarate, UINT uDowndatarate);
public:
	afx_msg void OnPaint();
protected:
	void ReCreateGraph(CDC* pDC);
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
};
