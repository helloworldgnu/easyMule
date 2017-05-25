/*
 * $Id: IEFilter.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
// IEFilter.cpp : 实现文件
//

#include "stdafx.h"
#include "IEFilter.h"


// CIEFilter

IMPLEMENT_DYNCREATE(CIEFilter, CCmdTarget)
CIEFilter::CIEFilter()
{
	EnableAggregation();
}

CIEFilter::~CIEFilter()
{
}


BEGIN_MESSAGE_MAP(CIEFilter, CCmdTarget)
END_MESSAGE_MAP()

// {E156A047-A940-43dd-A8D9-B020B0C6DF68}
IMPLEMENT_OLECREATE(CIEFilter, "IeFilter", 0xe156a047, 0xa940, 0x43dd, 0xa8, 0xd9, 0xb0, 0x20, 0xb0, 0xc6, 0xdf, 0x68);

BEGIN_INTERFACE_MAP(CIEFilter, CCmdTarget)
	INTERFACE_PART(CIEFilter, IID_IInternetProtocol, InternetProtocolImpl)
END_INTERFACE_MAP()


STDMETHODIMP_(ULONG) CIEFilter::XInternetProtocolImpl::AddRef( )
{
	METHOD_PROLOGUE_EX(CIEFilter, InternetProtocolImpl)
	return (ULONG)pThis->ExternalAddRef();
}

STDMETHODIMP_(ULONG) CIEFilter::XInternetProtocolImpl::Release( )
{
	METHOD_PROLOGUE_EX(CIEFilter, InternetProtocolImpl)
	return (ULONG)pThis->ExternalRelease();
}

STDMETHODIMP CIEFilter::XInternetProtocolImpl::QueryInterface( REFIID iid, LPVOID FAR* ppvObj )
{
	METHOD_PROLOGUE_EX(CIEFilter, InternetProtocolImpl)
	return (HRESULT)pThis->ExternalQueryInterface(&iid, ppvObj) ;
}


// CIEFilter 消息处理程序
