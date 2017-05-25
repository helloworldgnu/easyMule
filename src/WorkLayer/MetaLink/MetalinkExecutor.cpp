/*
 * $Id: MetalinkExecutor.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
#include "StdAfx.h"
#include ".\metalinkexecutor.h"

#include "PartFile.h"
#include "MetaLinkParser.h"

CMetalinkExecutor::CMetalinkExecutor(void)
{
}

CMetalinkExecutor::~CMetalinkExecutor(void)
{
}

bool CMetalinkExecutor::execute( CPartFile * partFile )
{
	// 
	const CString & filepath = partFile->GetFilePath();
	
	if( -1 == filepath.Find( _T(".metalink") ) ) {
		// 不是 .metalink 文件，忽略
		return false;
	}

	CMetaLinkParser parser( filepath );
	if( parser.GetErrorCode() != METALINK_OK ) {
		// 不是合法的 meta link 文件
		return false;
	}

	bool ret = partFile->ChangedToMetalinkFile( &parser );

	// 
	if( !ret ) {
		return false;
	}

	// 进行处理

	return true;
}
