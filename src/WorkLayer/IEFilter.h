/*
 * $Id: IEFilter.h 4483 2008-01-02 09:19:06Z soarchin $
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


// CIEFilter ÃüÁîÄ¿±ê
class CIEFilter : public CCmdTarget
{
	DECLARE_DYNCREATE(CIEFilter)

public:
	CIEFilter();
	virtual ~CIEFilter();

protected:
	DECLARE_MESSAGE_MAP()

	DECLARE_OLECREATE(CIEFilter) 
	DECLARE_INTERFACE_MAP()

	BEGIN_INTERFACE_PART(InternetProtocolImpl, IInternetProtocol)
		INIT_INTERFACE_PART(CIEFilter, InternetProtocolImpl)

		STDMETHOD(Start)(LPCWSTR /*szUrl*/, IInternetProtocolSink * /*pIProtSink*/,
						IInternetBindInfo * /*pIBindInfo*/, DWORD /*grfSTI*/, DWORD /*dwReserved*/)
		{
			return INET_E_USE_DEFAULT_PROTOCOLHANDLER;
		}
		STDMETHOD(Continue)(PROTOCOLDATA * /*pStateInfo*/)
		{
			return S_OK;
		}
		STDMETHOD(Abort)(HRESULT /*hrReason*/,DWORD /*dwOptions*/)
		{
			return E_NOTIMPL;
		}
		STDMETHOD(Terminate)(DWORD /*dwOptions*/)
		{
			return  S_OK;
		}
		STDMETHOD(Suspend)()
		{
			return E_NOTIMPL;
		}
		STDMETHOD(Resume)()
		{
			return E_NOTIMPL;
		}
		STDMETHOD(Read)(void * /*pv*/,ULONG /*cb*/,ULONG * /*pcbRead*/)
		{
			return E_NOTIMPL;
		}
		STDMETHOD(Seek)(LARGE_INTEGER /*dlibMove*/, DWORD /*dwOrigin*/, ULARGE_INTEGER * /*plibNewPosition*/)
		{
			return E_NOTIMPL;
		}
		STDMETHOD(LockRequest)(DWORD /*dwOptions*/)
		{
			return  S_OK;
		}
		STDMETHOD(UnlockRequest)()
		{
			return  S_OK;
		}

	END_INTERFACE_PART(InternetProtocolImpl)

};


