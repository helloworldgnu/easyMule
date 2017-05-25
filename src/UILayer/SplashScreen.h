/*
 * $Id: SplashScreen.h 4483 2008-01-02 09:19:06Z soarchin $
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

#include "enbitmap.h"
#include "../CxImage/xImage.h" // Added by thilon on 2006.08.01,”√”⁄º”‘ÿPNGÕº∆¨

class CSplashScreen : public CDialog
{
	DECLARE_DYNAMIC(CSplashScreen)

public:
	CSplashScreen(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSplashScreen();

// Dialog Data
	enum { IDD = IDD_SPLASH };

public:
	HBITMAP CopyScreenToBitmap(CRect& rect);// Added by thilon on 2006.08.01
protected:
	CBitmap m_imgSplash;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	BOOL OnInitDialog();
	BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
	void OnPaint(); 
	void DrawText(CDC *pDC);

protected:
	HBITMAP		m_hbm; 
	 BITMAP		m_bitmap;
	 CRect		rect;

	 CxImage*   m_image; // Added by thilon on 2006.08.01

	 CRect	m_rtImg;
	 CRect	m_rtVersion;
	 CRect	m_rtText;

public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};
