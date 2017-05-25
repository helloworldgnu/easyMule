/* 
 * $Id: Version.h 12472 2009-04-28 02:22:26Z huby $
 * 
 * this file is part of easyMule
 * Copyright (C)2002-2007 VeryCD Dev Team ( strEmail.Format("%s@%s", "devteam", "easymule.org") / http://www.easymule.org )
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

#ifndef __VERSION_H__
#define __VERSION_H__

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef _T
#define _T(x)	x
#endif

#define _chSTR(x)		_T(#x)
#define chSTR(x)		_chSTR(x)

// *) Specify the version of emule only here with the following defines.
// *) When changing any of those version nr. defines you also have to rebuild the language DLLs.
//
// General format:
//	<major>.<minor>.<update>.<build>
//
// Fields:
//	<major>		major number (e.g. 0)
//	<minor>		minor number (e.g. 30)
//	<update>	update number (e.g. 0='a'  1='b'  2='c'  3='d'  4='e'  5='f' ...)
//	<build>		build number; currently not used
//
// Currently used:
//  <major>.<minor>.<update> is used for the displayed version (GUI) and the version check number
//	<major>.<minor>			 is used for the protocol(!) version
//
#define VERSION_MJR		0
#define VERSION_MIN		49
#define VERSION_UPDATE	1
#define VERSION_BUILD	27

//VC版本号，修改后面的数值可以修改Build号
//如，70704，表示2007年07月04日
#define VC_VERSION_BUILD 90429//Added by thilon on 2006.01.10

#define EASYMULE_MJR	1		//	<major>		major number (e.g. 0)
#define EASYMULE_MIN	1		//	<minor>		minor number (e.g. 30)
#define	EASYMULE_UPDATE 5		//	<update>	update number (e.g. 0='a'  1='b'  2='c'  3='d'  4='e'  5='f' ...)

// NOTE: This version string is also used by the language DLLs!
//#define	SZ_VERSION_NAME		chSTR(VERSION_MJR) _T(".") chSTR(VERSION_MIN) _T(".") chSTR(VERSION_UPDATE)
#define	SZ_VERSION_NAME		chSTR(EASYMULE_MJR) _T(".") chSTR(EASYMULE_MIN) _T(".") chSTR(EASYMULE_UPDATE) _T(".") chSTR(VC_VERSION_BUILD)

#endif /* !__VERSION_H__ */
