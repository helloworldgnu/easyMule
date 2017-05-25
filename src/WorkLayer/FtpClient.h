/*
 * $Id: FtpClient.h 9297 2008-12-24 09:55:04Z dgkang $
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
// ***************************************************************
//  FtpClient   version:  1.0   ·  date: 08/09/2007
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) 2007 - All Rights Reserved
// ***************************************************************
// Ftp Peer 定义,从ParFile获取下载数据任务,并把下载后的数据交给PartFile
// Ftp Peer 需要管理两个Socket通道( 命令Socket和数据Socket)
//                                         vc-huby
// ***************************************************************

#pragma once
#include "INetClient.h"

#include "FTPClientReqSocket.h"


class CFtpClient : public CINetClient
{
	DECLARE_DYNAMIC(CFtpClient)
public:
	CFtpClient(IPSite *pIPSite);
	virtual ~CFtpClient(void);

	bool SetUrl(LPCTSTR pszUrl, uint32 nIP = 0);

	virtual void SetRequestFile(CPartFile* pReqFile);	

	virtual bool TryToConnect(bool bIgnoreMaxCon = false, bool bNoCallbacks = false, CRuntimeClass* pClassSocket = NULL);
	virtual bool Connect();
	
	virtual void OnSocketConnected(int nErrorCode);
	virtual bool Disconnected(LPCTSTR pszReason, bool bFromSocket = false,CClientReqSocket* pSocket=NULL);

	virtual void Pause( );

	bool		 OnFileSizeKnown( uint64 uiFileSize );
	void		 OnCmdSocketErr(CStringA strError);
	virtual void ProcessRawData(const BYTE * pucData, UINT uSize);

	virtual bool SendFtpBlockRequests();
	virtual void SendCancelTransfer(Packet* packet);
	
	CFtpClientDataSocket* GetClientDataSocket( bool bCreateIfNull=true );
	CFtpClientReqSocket*  GetClientReqSocket( );

	bool		 GetUrlStartPosForReq(  uint64& uiUrlStartPos ); //< 计算此ftp peer应该从数据的哪个位置开始发请求


	void				CloseDataSocket();
	int					PendingBlockReqCount(){ return m_PendingBlocks_list.GetCount();}
private:

	CFtpClientDataSocket *	m_pFtpDataSocket; //< 发PASV命令成功后才会创建数据通道传文件数据
};
