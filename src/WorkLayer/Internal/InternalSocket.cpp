/*
 * $Id: InternalSocket.cpp 4704 2008-01-26 04:04:10Z nightsuns $
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
/** @file InternalSocket.h @brief find the file you wanted in the internalnet 
<pre>
*	Copyright (c) 2007，Emule
*	All rights reserved.
*
*	当前版本：
*	作    者：kernel
*	完成日期：2007-01-11
*
*	取代版本：none
*	作    者：none
*	完成日期：none
</pre>*/

#include "stdafx.h"
#include "InternalSocket.h"
#include "GlobalVariable.h"
#include "Ed2kUpDownClient.h"


CInternalSocket::CInternalSocket(){

	int nIP_TTL = 1;
	int err;

	DWORD bufsize;
	DWORD bFlag;

	wchar_t strDestMulti[] = _T("224.1.1.1"); // 多址广播地址范围从 224 到 239 。
	u_short nDestPort = 6666;
	int addrlen = wcslen(strDestMulti) * 2;


	err = WSAStringToAddress ( strDestMulti, /* address string */
		AF_INET, /* address family */
		NULL, /* protocol info structure */
		(LPSOCKADDR)&stDestAddr, /* socket address string */
		&addrlen); /* length of socket structure */

	if(err)return;


	ASSERT(m_hSocket == INVALID_SOCKET);

	m_hSocket = WSASocket(PF_INET, SOCK_DGRAM, 0 , NULL, 0,
		WSA_FLAG_OVERLAPPED| WSA_FLAG_MULTIPOINT_C_LEAF | WSA_FLAG_MULTIPOINT_D_LEAF);
	
	if(m_hSocket == INVALID_SOCKET)return;



	//Create(6666, SOCK_DGRAM, FD_READ | FD_WRITE, theApp.GetBindAddress()/* strDestMulti thePrefs.GetBindAddrW()*/);

	bFlag = TRUE; // 设置套接字为可重用端口地址

	err = setsockopt(m_hSocket, /* socket */
		SOL_SOCKET, /* socket level */
		SO_REUSEADDR, /* socket option */
		(char *)&bFlag, /* option value */
		sizeof (bFlag)); /* size of value */

	if(err)return;
	

	if(AsyncSelect()){
		//Bind(6666, strDestMulti);
	}

	AttachHandle(m_hSocket, this, FALSE);

	err = WSAIoctl (m_hSocket, /* socket */
		SIO_MULTICAST_SCOPE, /* IP Time-To-Live */
		&nIP_TTL, /* input */
		sizeof (nIP_TTL), /* size */
		NULL, /* output */
		0, /* size */
		&bufsize, /* bytes returned */
		NULL, /* overlapped */
		NULL); /* completion routine */

	if(err)return;


	bFlag = FALSE;	//No LOOPBACK
	//bFlag = TRUE;

	err = WSAIoctl (m_hSocket, /* socket */
		SIO_MULTIPOINT_LOOPBACK, /* LoopBack on or off */
		&bFlag, /* input */
		sizeof (bFlag), /* size */
		NULL, /* output */
		0, /* size */
		&bufsize, /* bytes returned */
		NULL, /* overlapped */
		NULL); /* completion routine */

	if(err)return;


	stDestAddr.sin_family = PF_INET;
	err = WSAHtons( m_hSocket, /* socket */
		nDestPort, /* host order value */
		&(stDestAddr.sin_port)); /* network order value */

	if (err == SOCKET_ERROR)return;


	// 加入到指定多址广播组，指定为既作发送者又作接收者
	// 在 IP multicast 中，返回的套接字描述符和输入的套接字描述符相同。
	m_hSocket2 = WSAJoinLeaf (m_hSocket, /* socket */
		(PSOCKADDR)&stDestAddr, /* multicast address */
		sizeof (stDestAddr), /* length of addr struct */
		NULL, /* caller data buffer */
		NULL, /* callee data buffer */
		NULL, /* socket QOS setting */
		NULL, /* socket group QOS */
		JL_BOTH); /* do both: send *and* receive */

	if (m_hSocket2 == INVALID_SOCKET)return;


	if (err)return;

}

CInternalSocket::~CInternalSocket(){

	//closesocket(m_hSocket);
	if(m_hSocket2!=m_hSocket) closesocket(m_hSocket2);

}
void CInternalSocket::Broadcast(const uchar* filehash){


	WSABUF stWSABuf;
	CSafeMemFile packet;
	SOCKADDR_IN addr;

	DWORD bufsize;
	int err;
	int addrlen;
	
	addrlen = sizeof(addr);

	//uint32 server_ip = CGlobalVariable::serverconnect->GetLocalIP();
	//uint16 server_udpport = CGlobalVariable::serverconnect->GetLocalUDP();

	CAsyncSocket::GetSockName((SOCKADDR*)&addr, &addrlen);

	packet.WriteUInt8(OP_VC_BC_HEADER);
	packet.WriteUInt8(OP_BC_REQUESTSOURCE);

	packet.WriteHash16(filehash);
	packet.WriteUInt32((uint32)addr.sin_addr.s_addr);
	packet.WriteUInt16((uint16)addr.sin_port);

	//CKnownFile* partfile = NULL;
	//partfile = CGlobalVariable::sharedfiles->GetFileByID(filehash);

	stWSABuf.buf = (char*)packet.GetBuffer();
	stWSABuf.len = (uint32)packet.GetLength();

	err = WSASendTo(m_hSocket, /* socket */
		&stWSABuf, /* output buffer structure */
		1, /* buffer count */
		&bufsize, /* number of bytes sent */
		0, /* flags */
		(struct sockaddr*)&stDestAddr, /* destination address */
		sizeof(struct sockaddr), /* size of addr structure */
		NULL, /* overlapped structure */
		NULL); /* overlapped callback function */

	if(err)return;
}

void CInternalSocket::Process(char* buffer, uint32 buffersize){

	CSafeMemFile req((const BYTE*)buffer,(UINT)buffersize);
	CSafeMemFile res;

	CKnownFile* file = NULL;
	CPartFile* partfile = NULL;
	uchar filehash[16];
	//SOCKADDR_IN addr,ownaddr;

	WSABUF stWSABuf;
	DWORD bufsize;

	int err;
	//int addrlen;
	uint8 opcode;

	uint32 ip = CGlobalVariable::serverconnect->GetLocalIP();
	uint16 port = thePrefs.GetPort();

	if(buffersize == 24){

		if(req.ReadUInt8() != OP_VC_BC_HEADER)return;

		opcode = req.ReadUInt8();
		req.ReadHash16(filehash);

		if(opcode == OP_BC_REQUESTSOURCE){

			file = CGlobalVariable::sharedfiles->GetFileByID(filehash);
			if(file == NULL)return;

			//CAsyncSocket::GetSockName((SOCKADDR*)&ownaddr, &addrlen);

			res.WriteUInt8(OP_VC_BC_HEADER);
			res.WriteUInt8(OP_BC_ANSWERSOURCE);
			res.WriteHash16(filehash);

			res.WriteUInt32(ip);
			//res.WriteUInt16(ownaddr.sin_port);
			res.WriteUInt16(port);

			stWSABuf.buf = (char*)res.GetBuffer();
			stWSABuf.len = (u_long)res.GetLength();
			bufsize = (DWORD)res.GetLength();

			err = WSASendTo(m_hSocket, /* socket */
				&stWSABuf, /* output buffer structure */
				1, /* buffer count */
				&bufsize, /* number of bytes sent */
				0, /* flags */
				(struct sockaddr*)&stDestAddr, /* destination address */
				sizeof(struct sockaddr), /* size of addr structure */
				NULL, /* overlapped structure */
				NULL); /* overlapped callback function */
			
			if (err){
				err = WSAGetLastError();
				return;
			}
		
		}
		else if(opcode == OP_BC_ANSWERSOURCE){

			ip = req.ReadUInt32();
			port = req.ReadUInt16();

			partfile = CGlobalVariable::downloadqueue->GetFileByID(filehash);

			if(partfile){			
				CUpDownClient* toadd = new CEd2kUpDownClient(partfile, port, 0, 0, 0);
				toadd->SetSourceFrom(SF_LAN);
				toadd->m_nLanConnectIP = ip;
				CGlobalVariable::downloadqueue->CheckAndAddSource(partfile, toadd);			
			}
		}

	}

}

void CInternalSocket::OnSend(int nErrorCode){

	printf("%d",nErrorCode);

}

void CInternalSocket::OnReceive(int nErrorCode){

	printf("%d",nErrorCode);

	int err;
	int addrlen;
	DWORD dFlag;
	DWORD bufsize;

	WSABUF stWSABuf;
	char achInBuf[BUFSIZE];


	stWSABuf.buf = achInBuf;
	stWSABuf.len = BUFSIZE;

	bufsize = 0;
	dFlag = 0;
	addrlen = sizeof(stSrcAddr);

	err = WSARecvFrom (m_hSocket, /* socket */
		&stWSABuf, /* input buffer structure */
		1, /* buffer count */
		&bufsize, /* number of bytes recv'd */
		&dFlag, /* flags */
		(struct sockaddr *)&stSrcAddr, /* source address */
		&addrlen, /* size of addr structure */
		NULL, /* overlapped structure */
		NULL); /* overlapped callback function */

	if (err){
		err = WSAGetLastError();
		return;
	}

	Process(achInBuf,bufsize);

}
