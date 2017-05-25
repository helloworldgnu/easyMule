/*
 * $Id: KadContactHistogramCtrl.h 4483 2008-01-02 09:19:06Z soarchin $
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

#define	KAD_CONTACT_HIST_NEEDED_BITS	12	// 8=256, 9=512, 10=1024, 11=2048, 12=4096, 13=8192, 14=16384, 15=32768, 16=65536
#define	KAD_CONTACT_HIST_SIZE			(1 << KAD_CONTACT_HIST_NEEDED_BITS)

class CKadContactHistogramCtrl : public CWnd
{
public:
	CKadContactHistogramCtrl();
	virtual ~CKadContactHistogramCtrl();

	void Localize();

	bool ContactAdd(const Kademlia::CContact* contact);
	void ContactRem(const Kademlia::CContact* contact);

protected:
	UINT m_aHist[KAD_CONTACT_HIST_SIZE];
	CPen m_penAxis;
	CPen m_penAux;
	CPen m_penHist;
	CFont m_fontLabel;
	int m_iMaxLabelHeight;
	int m_iMaxNumLabelWidth;
	bool m_bInitializedFontMetrics;
	CString m_strXaxis;
	CString m_strYaxis;

	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
};
