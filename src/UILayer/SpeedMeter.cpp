/*
 * $Id: SpeedMeter.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
// SpeedMeter.cpp : 实现文件
//

#include "stdafx.h"
#include "SpeedMeter.h"
#include ".\speedmeter.h"
#include "UtilUI.h"

#define MAINCOLORVALUE	224


// CSpeedMeter

IMPLEMENT_DYNAMIC(CSpeedMeter, CWnd)
CSpeedMeter::CSpeedMeter()
{
	m_nMin		= 0;
	m_nMax		= 100;
	m_nStep		= 1;
	m_bInit		= true;

	m_crDownload	= RGB(27, 171, 89);//绿色
	m_crUpload		= RGB(232, 136, 29);//红色//RGB(0, 224, 0);
	m_pOldBMP = NULL;
}

CSpeedMeter::~CSpeedMeter()
{
	if(m_pOldBMP)
	{
		m_MemDC.SelectObject(m_pOldBMP);
	}
}


BEGIN_MESSAGE_MAP(CSpeedMeter, CWnd)
	ON_WM_PAINT()
ON_WM_SIZE()
END_MESSAGE_MAP()



// CSpeedMeter 消息处理程序


void CSpeedMeter::AddValues(UINT uUpdatarate, UINT uDownDatarate, bool bRedraw)
{
	m_UpdataList.AddTail(uUpdatarate);
	m_DownloadList.AddTail(uDownDatarate);
	
	if(GetSafeHwnd())
	{
		AddValues2Graph(uUpdatarate, uDownDatarate);

		if(bRedraw)
		{
			Invalidate();
		}
	}
}

void CSpeedMeter::AddValues2Graph(UINT uUpdatarate, UINT uDowndatarate)
{
	if(m_MemDC.GetSafeHdc())
	{
		CRect rClient;
		GetClientRect(&rClient);

		int iWidth		= rClient.Width();
		int iHeight		= rClient.Height();

		int iNewWidth	= iWidth - m_nStep;
	
		if(uUpdatarate > m_nMax)
		{
			uUpdatarate = m_nMax;	//设置上限，防止过大
		}

		if(uDowndatarate > m_nMax)
		{
			uDowndatarate = m_nMax;	//设置上限，防止过大
		}
		
		double dUpdatarate		= (double)iHeight * uUpdatarate / m_nMax;
		double dDowndatarate	= (double)iHeight * uDowndatarate / m_nMax;



		//四舍五入，这里是五入
		UINT nRndUpdatarate = (UINT)dUpdatarate;
		if(dUpdatarate - nRndUpdatarate >= 0.5)
		{
			nRndUpdatarate++;
		}
		//四舍五入，这里是五入
		UINT nRndDowndatarate = (UINT)dDowndatarate;
		if(dDowndatarate - nRndDowndatarate >= 0.5)
		{
			nRndDowndatarate++;
		}
		
		m_MemDC.BitBlt(0, 0, iNewWidth, iHeight, &m_MemDC, m_nStep, 0, SRCCOPY);
		

		CRect rect(iNewWidth, 0, iNewWidth + m_nStep, iHeight);
		DrawParentBk(GetSafeHwnd(), m_MemDC.GetSafeHdc(), &rect);

		m_MemDC.SetPixel(iWidth - 1, iHeight - nRndUpdatarate, m_crUpload);
		m_MemDC.SetPixel(iWidth - 1, iHeight - nRndDowndatarate, m_crDownload);

		while(m_UpdataList.GetCount() > rClient.Width())
		{
			m_UpdataList.RemoveHead();
			m_DownloadList.RemoveHead();
		}	
	}
}

void CSpeedMeter::SetRange(UINT nMin, UINT nMax, bool bDelete)
{
	m_nMin	= nMin;
	m_nMax	= nMax;
	
	ResetGraph(bDelete);
}

void CSpeedMeter::GetRange(UINT& nMax, UINT& nMin)
{
	nMax	= m_nMax;
	nMin	= m_nMin;
}

void CSpeedMeter::ResetGraph(bool bDelete)
{
	if(bDelete)
	{
		m_UpdataList.RemoveAll();
		m_DownloadList.RemoveAll();
	}
	m_bInit = true;
	Invalidate();
}
void CSpeedMeter::OnPaint()
{
	CPaintDC dc(this);
	
	CRect rtClient;
	GetClientRect(&rtClient);
	
	if(m_bInit)
	{		
		m_bInit = false;
		ReCreateGraph(&dc);
	}
	
	dc.BitBlt(0, 0, rtClient.Width(), rtClient.Height(), &m_MemDC, 0,0, SRCCOPY);
}

void CSpeedMeter::ReCreateGraph(CDC* pDC)
{
	CRect rClient;
	GetClientRect(&rClient);

	if(m_pOldBMP)
	{
		m_MemDC.SelectObject(m_pOldBMP);
	}

	if(m_MemBMP.GetSafeHandle())
	{
		m_MemBMP.DeleteObject();
	}

	if(m_MemDC.GetSafeHdc())
	{
		m_MemDC.DeleteDC();
	}
	
	m_MemDC.CreateCompatibleDC(pDC);
	m_MemBMP.CreateCompatibleBitmap(pDC, rClient.Width(), rClient.Height());
	m_pOldBMP = m_MemDC.SelectObject(&m_MemBMP);
	DrawParentBk(GetSafeHwnd(), m_MemDC.GetSafeHdc());
	

	POSITION pos1 = m_UpdataList.GetHeadPosition();
	POSITION pos2 = m_DownloadList.GetHeadPosition();

	while(pos1 != NULL && pos2 != NULL)
	{
		AddValues2Graph(m_UpdataList.GetNext(pos1), m_DownloadList.GetNext(pos2));
	}
}

void CSpeedMeter::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	m_bInit = true;
	Invalidate();	
}
