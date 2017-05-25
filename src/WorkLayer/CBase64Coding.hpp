/*
 * $Id: CBase64Coding.hpp 4483 2008-01-02 09:19:06Z soarchin $
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
#if ! defined( BASE_64_CODING_CLASS_HEADER )

#pragma once

/*
** Author: Samuel R. Blackburn
** Internet: wfc@pobox.com
**
** You can use it any way you like as long as you don't try to sell it.
**
** Any attempt to sell WFC in source code form must have the permission
** of the original author. You can produce commercial executables with
** WFC but you can't sell WFC.
**
** Copyright, 2000, Samuel R. Blackburn
**
** $Workfile: CBase64Coding.hpp $
** $Revision: 3 $
** $Modtime: 1/04/00 4:39a $
*/

#define BASE_64_CODING_CLASS_HEADER

class CBase64Coding
{
   private:

      // Don't allow canonical behavior (i.e. don't allow this class
      // to be passed by value)

      CBase64Coding( const CBase64Coding& ) {};
      CBase64Coding& operator=( const CBase64Coding& ) { return( *this ); };

   public:

      // Construction

      CBase64Coding();

      /*
      ** Destructor should be virtual according to MSJ article in Sept 1992
      ** "Do More with Less Code:..."
      */

      virtual ~CBase64Coding();

      virtual BOOL Encode( const char * source, int len, char * destination );
};

#endif // BASE_64_CODING_CLASS_HEADER
