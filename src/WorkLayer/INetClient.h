#pragma once

#include "updownclient.h"
#include "SourceURL.h"
#include "UrlSite.h"

/**
* class CINetClient 
* CINetClient派生于 CUpDownClient 用来描述基于 C/S 架构的一个客户端,
*
*/
class CINetClient : public CUpDownClient
{
	DECLARE_DYNAMIC(CINetClient)
public:
	CINetClient(IPSite * pIpSite);
	~CINetClient(void);

	// 得到得设置 IpSite
	bool setIpSite( IPSite * pIpSite )
	{
		this->m_pIpSite = pIpSite;
		return true;
	}

	IPSite * getIpSite()
	{
		return this->m_pIpSite;
	}

	// 指定是否是ED2K的客户端
	virtual bool IsEd2kClient() const 
	{ 
		return false; 
	}

	bool IsOriginalUrlSite( );

	// INet 类型永远不需要发送 HELLO 包
	virtual bool SendHelloPacket()
	{
		return true;
	}

	//
	// 得到远程文件的大小
	//
	// 返回值:
	// -1: 如果文件长度为未知
	//
	virtual uint64 GetFileSize( )
	{ 
		return m_uiFileSize;
	}

	// 为该 Client 创建新的任务,
	// 通过 iMaxBlocks 来指定最大分配的 任务长度
	virtual int CreateBlockRequests_Before(int iMaxBlocks);
	virtual void CreateBlockRequests_After(int iMaxBlocks);
	virtual void ConnectionEstablished(); //< handshake finished.. just override the CUpDownClient::ConnectionEstablished
	virtual uint32  GetTimeUntilReask() const; ///< http/ftp Peer如果没在下载,可以10s后再询问一次.
	
	bool TryToGetBlockRequests(int iMaxBlocks);

	//设置是否是坏站点
	void SetSiteBadOrGood();

	virtual	DWORD	GetIPFrom( ) const;

public:
	// 
	CSourceURL m_SourceURL;

protected:  
	//
	// 指向该站点指向的 IPSite 结构
	//
	IPSite * m_pIpSite;

	//
	// 文件的长度,
	// -1: 文件大小还未知
	//
	uint64 m_uiFileSize;
  
	//
	//是否添加下载源
	//
	bool m_bAddOtherSources;

	/// [VC-Huby-080513]: 最近一次检查添加其它下载源的时间
	DWORD m_dwLastAddOtherSources;

	virtual INetBlockRange_Struct* GetBlockRange();
	virtual bool RequestBlock( Requested_Block_Struct** newblocks, uint16* pCount, bool bUseParent );
	/// 是否已允许添加更多连接
	bool m_bAllowedAddMoreConn;

public:
	void UpdateTransData(const CString& strURL);

	// 该函数判断是否需要避免发起同一个IP的不同refer的连接
	bool IsNeedAvoidInitalizeConnection();
};
