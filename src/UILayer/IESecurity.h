/*
 * $Id: IESecurity.h 4483 2008-01-02 09:19:06Z soarchin $
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

class CMuleBrowserControlSite : public CBrowserControlSite
{
public:
	CMuleBrowserControlSite(COleControlContainer* pCtrlCont, CDHtmlDialog *pHandler);

protected:
	URLZONE m_eUrlZone;
	void InitInternetSecurityZone();

	DECLARE_INTERFACE_MAP();

	BEGIN_INTERFACE_PART(InternetSecurityManager, IInternetSecurityManager)
		STDMETHOD(SetSecuritySite)(IInternetSecurityMgrSite*);
		STDMETHOD(GetSecuritySite)(IInternetSecurityMgrSite**);
		STDMETHOD(MapUrlToZone)(LPCWSTR,DWORD*, DWORD);
		STDMETHOD(GetSecurityId)(LPCWSTR,BYTE*, DWORD*, DWORD);
		STDMETHOD(ProcessUrlAction)(LPCWSTR pwszUrl, DWORD dwAction, BYTE __RPC_FAR *pPolicy, DWORD cbPolicy, BYTE __RPC_FAR *pContext, DWORD cbContext, DWORD dwFlags, DWORD dwReserved = 0);
		STDMETHOD(QueryCustomPolicy)(LPCWSTR, REFGUID, BYTE**, DWORD*, BYTE*, DWORD, DWORD);
		STDMETHOD(SetZoneMapping)(DWORD, LPCWSTR, DWORD);
		STDMETHOD(GetZoneMappings)(DWORD, IEnumString**, DWORD);
	END_INTERFACE_PART(InternetSecurityManager)

	BEGIN_INTERFACE_PART(ServiceProvider, IServiceProvider)
		STDMETHOD(QueryService)(REFGUID, REFIID, void**);
	END_INTERFACE_PART(ServiceProvider)
};
