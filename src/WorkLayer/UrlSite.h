/*
 * $Id: UrlSite.h 5702 2008-05-30 09:00:18Z huby $
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

#include <vector>

enum ESiteFrom
{
	// 未知的
	sfUnknwon = 100,

	// 从metalink文件中获取到的
	sfMetalinkFile ,
	
	// 从服务器上
	sfVerycdServer ,

	sfMetaServer,

	// 手动输入的
	sfManualAdded ,

	// 起始下载的
	sfStartDown
};

class CUrlSite;
struct IPSite
{
	DWORD m_dwRetryCount;
	DWORD m_dwLastError;
	DWORD m_dwSameErrorCount;
	DWORD m_dwConnectionCount;
	DWORD m_dwMaxAllowConnCount;
	DWORD m_dwIpAddress;
	BOOL m_bBadSite;
	CUrlSite* m_pUrlSite;

	IPSite() : m_dwRetryCount(0) , m_dwLastError(0) 
		, m_dwSameErrorCount(0) , m_dwConnectionCount(0),m_dwMaxAllowConnCount(2)
		, m_dwIpAddress(0),m_bBadSite(false)
	{
	}
};

/**
* CUrlSite 用来描述一个资源点
*/
class CUrlSite
{
public:
	CUrlSite(void);
	~CUrlSite(void);

	// url 地址
	CString m_strUrl;

	// 指定该源从哪里来的
	DWORD m_dwFromWhere;
	
	// metaServer中是否已经有了
	BOOL  m_bExistInMetaServer;

	// 该站点上已经发起的连接数的个数
	// 这个节点不需要永续
	DWORD m_dwConnectionCount;

	// 这个站点上重试的次数
	// 这个节点不需要永续
	DWORD m_dwRetryCount;

	// 这个站点上已经传输完成的数据的大小,不包括协议握手包大小
	__int64 m_dwDataTransferedWithoutPayload;
	__int64 m_dwOldDataTransferedWithoutPayload;

	// 这个站点上已经传输完成的数据的大小,包括协议握手包大小
	__int64 m_dwDataTransferedWithPayload;
	__int64 m_dwOldDataTransferedWithPayload;

	// 这个站点是否是坏的站点,即文件的大小与实际的文件大小不一致,或已经更改的,
	// 但不包括无法连接的站点
	BOOL m_bBadSite;

	// 站点加入到任务中时的初始得分,这个节点需要永续
	DWORD m_dwInitPreference;

	//这个节点需要永续
	BOOL m_bNeedCommitted;

	CUrlSite* m_pRedirectFrom;

	std::vector<IPSite*> m_IPSiteList;
public:
	bool IsBadUrlSite();
	DWORD GetUrlSiteConnectNum();
	bool IsMyIP( DWORD dwIP,DWORD dwFrom);
};

class InvalidFormatException : public CException
{
};

 template<typename T>
	 T & operator << ( T & output_stream , const CUrlSite & site )
 {
	 output_stream << site.m_strUrl;
	 output_stream << site.m_dwFromWhere;
	 output_stream << site.m_dwConnectionCount;
	 output_stream << site.m_dwRetryCount;
	 output_stream << site.m_dwDataTransferedWithoutPayload;
	 output_stream << site.m_dwDataTransferedWithPayload;

	 return output_stream;
 };

 // 如果读取失败,该函数抛出 InvalidFormatException 异常出来
 template<typename T>
	 T & operator >> (T & input_stream , CUrlSite & site )
 {
	 input_stream >> site.m_strUrl;
	 input_stream >> site.m_dwFromWhere;
	 input_stream >> site.m_dwConnectionCount;
	 input_stream >> site.m_dwRetryCount;
	 input_stream >> site.m_dwDataTransferedWithoutPayload;
	 input_stream >> site.m_dwDataTransferedWithPayload;

	 return input_stream;
 };

