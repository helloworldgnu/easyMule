/*
 * $Id: HighColorTab.hpp 4483 2008-01-02 09:19:06Z soarchin $
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
// HighColorTab.hpp
//
// Author:  Yves Tkaczyk (yves@tkaczyk.net)
//
// This software is released into the public domain.  You are free to use it 
// in any way you like BUT LEAVE THIS HEADER INTACT.
//
// This software is provided "as is" with no expressed or implied warranty.  
// I accept no liability for any damage or loss of business that this software 
// may cause.
//
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include <memory>

namespace HighColorTab
{
  /*! \brief Policy class for creating image list. 

    Policy for creating a high color (32 bits) image list. The policy 
    ensure that there is a Win32 image list associated with the CImageList.
    If this is not the case, a NULL pointer shall be returned. 

    Returned image list is wrapped in an std::auto_ptr.
    
    \sa UpdateImageListFull  */
  struct CHighColorListCreator
  {
    /*! Create the image list.
        \retval std::auto_ptr<CImageList> Not null if success. */
    static std::auto_ptr<CImageList> CreateImageList()
    {
      std::auto_ptr<CImageList> apILNew( new CImageList() );
      if( NULL == apILNew.get() )
      {
        // ASSERT: The CImageList object creation failed.
        ASSERT( FALSE );
        return std::auto_ptr<CImageList>();
      }

      if( 0 == apILNew->Create( 16, 16, theApp.m_iDfltImageListColorFlags|ILC_MASK, 0, 1 ) )
      {
        // ASSERT: The image list (Win32) creation failed.
        ASSERT( FALSE );
        return std::auto_ptr<CImageList>();
      }

      return apILNew;
    }
  };



  /*! \brief Change the image list of the provided control (property sheet interface)

    This method provides full customization via policy over image list creation. The policy
    must have a method with the signature:
    <code>static std::auto_ptr<CImageList> CreateImageList()</code>

    \author Yves Tkaczyk (yves@tkaczyk.net)
    \date 02/2004 */
  template<typename TSheet,
           typename TListCreator>
    bool UpdateImageListFull(TSheet& rSheet)
  {
	  // Get the tab control...
	  CTabCtrl* pTab = rSheet.GetTabControl();
	  if (!IsWindow(pTab->GetSafeHwnd()))
	  {
      // ASSERT: Tab control could not be retrieved or it is not a valid window.
      ASSERT( FALSE );
		  return false;
	  }

    // Create the replacement image list via policy.
    std::auto_ptr<CImageList> apILNew( TListCreator::CreateImageList() );

    bool bSuccess = (NULL != apILNew.get() );

    // Reload the icons from the property pages.
    int nTotalPageCount = rSheet.GetPageCount();
    for(int nCurrentPage = 0; nCurrentPage < nTotalPageCount && bSuccess; ++nCurrentPage )
    {
      // Get the page.
      CPropertyPage* pPage = rSheet.GetPage( nCurrentPage );
      ASSERT( pPage );
      // Set the icon in the image list from the page properties.
      if( pPage && ( pPage->m_psp.dwFlags & PSP_USEHICON ) )
      {
        /*bSuccess &=*/ ( -1 != apILNew->Add( pPage->m_psp.hIcon ) );
      }

      if( pPage && ( pPage->m_psp.dwFlags & PSP_USEICONID ) )
      {
		  HICON hIcon = theApp.LoadIcon( pPage->m_psp.pszIcon, 16, 16 );
		  if (hIcon)
		  {
			/*bSuccess &=*/ ( -1 != apILNew->Add( hIcon ) );
			DestroyIcon(hIcon);
		  }
      }
    }

    if( !bSuccess )
    {
      // This ASSERT because either the image list could not be created or icon insertion failed.
      ASSERT( FALSE );
      // Cleanup what we have in the new image list.
      if( apILNew.get() )
      {
        apILNew->DeleteImageList();
      }

      return false;
    }

    // Replace the image list from the tab control.
    CImageList* pilOld = pTab->SetImageList( CImageList::FromHandle( apILNew->Detach() ) );
    // Clean the old image list if there was one.
    if( pilOld )
    {
      pilOld->DeleteImageList();
    }
       
    return true;
  };

  /*! \brief Change the image list of the provided control (property sheet)

    This method uses 32 bits image list creation default policy. */
  template<typename TSheet>
    bool UpdateImageList(TSheet& rSheet)
  {
    return UpdateImageListFull<TSheet, HighColorTab::CHighColorListCreator>( rSheet );
  };
};
