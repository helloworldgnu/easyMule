/*
 * $Id: IP2Country.h 4483 2008-01-02 09:19:06Z soarchin $
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
//EastShare Start - added by AndCycle, IP to Country

// by Superlexx, based on IPFilter by Bouc7
#pragma once
//#include <atlcoll.h>
#include "Log.h"

struct Country_Struct {
	CString			ShortCountryName;
	CString			MidCountryName;
	CString			LongCountryName;
	WORD			FlagIndex;
};

struct IPRange_Struct2{
	uint32          IPstart;
	uint32          IPend;
	Country_Struct*	country;
	~IPRange_Struct2() {  }
};

//	此类现在有可能在创建时不在主线程运行，如要访问共享资源，应先通过判断CIP2CountryOld::IsInMainThread()判断是否在主线程中。
class CIP2CountryOld
{
	public:
		CIP2CountryOld(void);
		~CIP2CountryOld(void);
		
		void	Load();
		void	Unload();

		//reset ip2country referense
		void	Reset();

		//refresh passive windows
		void	Refresh();

		bool	IsIP2Country()			{return EnableIP2Country;}
		bool	ShowCountryFlag();

		Country_Struct*	GetDefaultIP2Country() {return &defaultCountry;}

		bool	LoadFromFile();
		bool	LoadCountryFlagLib();
		void	RemoveAllCountries();
		void	RemoveAllIPs();
		void	RemoveAllFlags();

		bool	AddIPRange(uint32 IPfrom,uint32 IPto, CString shortCountryName, CString midCountryName, CString longCountryName);
		bool	AddCountry(CString shortCountryName, CString midCountryName, CString longCountryName);

		Country_Struct*	GetCountryFromIP(uint32 IP);
		WORD	GetFlagResIDfromCountryCode(CString shortCountryName);

		CImageList* GetFlagImageList() {return &CountryFlagImageList;}
	private:

		//check is program current running, if it's under init or shutdown, set to false
		bool	m_bRunning;

		HINSTANCE _hCountryFlagDll;
		CImageList	CountryFlagImageList;

		bool	EnableIP2Country;
		bool	EnableCountryFlag;
		Country_Struct defaultCountry;

		CRBMap<uint32, IPRange_Struct2*> iplist;
		CRBMap<CString, Country_Struct*> countryList;
		CRBMap<CString, uint16>	CountryIDtoFlagIndex;

		bool	m_bInMainThread;	//ADDED by fengwen on 2007/01/16	: 是否在主线程中？ （只有在主线程，有些对全局变量的访问才能进行。）
		//ADDED by fengwen on 2007/01/16	<begin> : 当此类不在主线程中，不能访问全局变量时，一些访问，初始成员变量传入。
		bool	m_tp_bIsIP2CountryShowFlag;
		int		m_tp_iIP2CountryNameMode;
		int		m_tp_iDfltImageListColorFlags;
		CString	m_tp_strConfigDir;
		//ADDED by fengwen on 2007/01/16	<end> : 当此类不在主线程中，不能访问全局变量时，一些访问，初始成员变量传入。
		void AddDebugLogLine(bool bAddToStatusBar, LPCTSTR pszLine)
		{
			if (m_bInMainThread)
				::AddDebugLogLine(bAddToStatusBar, pszLine);
		}
		friend class CIP2Country;	//ADDED by fengwen on 2007/01/16	: 
};

//EastShare End - added by AndCycle, IP to Country

//ADDED by fengwen on 2007/01/16	<begin> :	为提高IP2Country的反应速度，把原来的IP2Country放在线程里执行。
class CIP2Country : public CWnd
{
	DECLARE_DYNAMIC(CIP2Country)
public:
	CIP2Country(void);
	~CIP2Country(void);

public:
	bool				ShowCountryFlag();
	CImageList*			GetFlagImageList();
	Country_Struct*		GetCountryFromIP(uint32 IP);
	bool				IsIP2Country();
	void				Load();
	void				Refresh();
	Country_Struct*		GetDefaultIP2Country();


protected:
	void				CreateOldI2CAsync(void);

	CIP2CountryOld	*m_pOldIP2Country;
	CWinThread		*m_pInitOldThread;

	typedef struct _OldInitThreadParam
	{
		HWND	hNotifyWnd;
		bool	bIsIP2CountryShowFlag;
		int		iIP2CountryNameMode;
		int		iDfltImageListColorFlags;
		CString	strConfigDir;
	} OLD_INIT_THREAD_PARAM;

	friend static UINT	CIP2Country::OldProcInitThread(LPVOID lpParam);
	static UINT	OldProcInitThread(LPVOID lpParam);
protected:
	DECLARE_MESSAGE_MAP()
	
	afx_msg LRESULT OnLoadCompleted(WPARAM wParam, LPARAM lParam);
};
//ADDED by fengwen on 2007/01/16	<end> :	为提高IP2Country的反应速度，把原来的IP2Country放在线程里执行。
