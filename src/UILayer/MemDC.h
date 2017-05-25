/*
 * $Id: MemDC.h 4483 2008-01-02 09:19:06Z soarchin $
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
//////////////////////////////////////////////////
// CMemDC - memory DC
//
// Author: Keith Rule
// Email:  keithr@europa.com
// Copyright 1996-2002, Keith Rule
//
// You may freely use or modify this code provided this
// Copyright is included in all derived versions.
//
// History - 10/3/97 Fixed scrolling bug.
//               Added print support. - KR
//
//       11/3/99 Fixed most common complaint. Added
//            background color fill. - KR
//
//       11/3/99 Added support for mapping modes other than
//            MM_TEXT as suggested by Lee Sang Hun. - KR
//
//       02/11/02 Added support for CScrollView as supplied
//             by Gary Kirkham. - KR
//
// This class implements a memory Device Context which allows
// flicker free drawing.
#pragma once

class CMemDC : public CDC
{
private:
    CBitmap    m_bitmap;        // Offscreen bitmap
    CBitmap*   m_oldBitmap;		// bitmap originally found in CMemDC
    CDC*       m_pDC;           // Saves CDC passed in constructor
    CRect      m_rect;          // Rectangle of drawing area.
    BOOL       m_bMemDC;        // TRUE if CDC really is a Memory DC.
	bool	   m_bFlushed;

public:
	CMemDC(CDC* pDC, LPCRECT pRect = NULL)
		: CDC()
	{
		ASSERT(pDC != NULL);

		// Some initialization
		m_pDC = pDC;
		m_oldBitmap = NULL;
		m_bMemDC = !pDC->IsPrinting();
		m_bFlushed = false;

		// Get the rectangle to draw
		if (pRect == NULL)
			pDC->GetClipBox(&m_rect);
		else
			m_rect = *pRect;

		if (m_bMemDC)
		{
			// Create a Memory DC
			CreateCompatibleDC(pDC);
			//pDC->LPtoDP(&m_rect);	// what is this needed for= -- we can't use that for RTL DCs!

			VERIFY( m_bitmap.CreateCompatibleBitmap(pDC, m_rect.Width(), m_rect.Height()) );
			m_oldBitmap = SelectObject(&m_bitmap);

			SetMapMode(pDC->GetMapMode());
			SetWindowExt(pDC->GetWindowExt());
			SetViewportExt(pDC->GetViewportExt());
            //pDC->DPtoLP(&m_rect);	// what is this needed for? -- we can't use that for RTL DCs!
			SetWindowOrg(m_rect.left, m_rect.top);
		}
		else
		{
			m_bPrinting = pDC->m_bPrinting;
			m_hDC		 = pDC->m_hDC;
			m_hAttribDC = pDC->m_hAttribDC;
		}

		// Fill background
		FillSolidRect(m_rect, pDC->GetBkColor());
	}
    
    ~CMemDC()      
    {
		Flush();
    }

	void Flush()
	{
		if (!m_bFlushed)
		{
			m_bFlushed = true;
			if (m_bMemDC)
			{
				// Copy the offscreen bitmap onto the screen.
				m_pDC->BitBlt(m_rect.left, m_rect.top, 
							  m_rect.Width(), m_rect.Height(), this, 
							  m_rect.left, m_rect.top, SRCCOPY);            
				
				//Swap back the original bitmap.
				SelectObject(m_oldBitmap);
			}
			else
				m_hDC = m_hAttribDC = NULL;
		}
	}

    // Allow usage as a pointer    
    CMemDC* operator->() 
    {
        return this;
    }       
 
    // Allow usage as a pointer    
    operator CMemDC*() 
    {
        return this;
    }
};
