/*
 * $Id: tabctrl.hpp 4483 2008-01-02 09:19:06Z soarchin $
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

class TabControl: public CTabCtrl
{
public:
  TabControl();
  virtual ~TabControl();
  UINT GetLastMovementSource()	{return m_nSrcTab;}
  UINT GetLastMovementDestionation()	{return m_nDstTab;}
  BOOL ReorderTab( unsigned int nSrcTab, unsigned int nDstTab );

  // Overriden CTabCtrl/CWnd members
//  virtual LRESULT WindowProc( UINT message, WPARAM wParam, LPARAM lParam );

  // Command/Notification Handlers
  afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
  afx_msg void OnLButtonUp( UINT nFlags, CPoint point );
  afx_msg void OnMouseMove( UINT nFlags, CPoint point );
  afx_msg void OnCaptureChanged( CWnd* );
//  afx_msg void OnPaint();
  
private:
  bool  m_bDragging;     // Specifies that whether drag 'n drop is in progress.
  UINT  m_nSrcTab;       // Specifies the source tab that is going to be moved.
  UINT  m_nDstTab;       // Specifies the destination tab (drop position).
  bool  m_bHotTracking;  // Specifies the state of whether the tab control has hot tracking enabled.
  BOOL  DragDetectPlus(CWnd* Handle, CPoint p);

  CRect m_InsertPosRect;
  CPoint m_lclickPoint;

  CSpinButtonCtrl * m_pSpinCtrl;
  
  // Utility members
  bool DrawIndicator( CPoint point );
  
  DECLARE_MESSAGE_MAP()
};
