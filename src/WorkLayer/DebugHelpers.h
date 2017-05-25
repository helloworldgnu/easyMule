/*
 * $Id: DebugHelpers.h 4483 2008-01-02 09:19:06Z soarchin $
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

#define CHECK_OBJ(pObj)		if (pObj != NULL) ASSERT_VALID(pObj)
#define CHECK_PTR(ptr)		ASSERT( ptr == NULL || AfxIsValidAddress(ptr, sizeof(*ptr)) );
#define CHECK_ARR(ptr, len)	ASSERT( (ptr == NULL && len == 0) || (ptr != NULL && len != 0 && AfxIsValidAddress(ptr, len)) );
#define	CHECK_BOOL(bVal)	ASSERT( (UINT)(bVal) == 0 || (UINT)(bVal) == 1 );

#define	CRASH_HERE()		(*((int*)NULL) = 0)

#ifdef _DEBUG
#ifndef NO_USE_CLIENT_TCP_CATCH_ALL_HANDLER
#define NO_USE_CLIENT_TCP_CATCH_ALL_HANDLER	1
#endif
#endif
