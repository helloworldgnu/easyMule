/*
 * $Id: InternalSocket.h 4483 2008-01-02 09:19:06Z soarchin $
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

#pragma once

//#include "emule.h"
#include "SharedFileList.h"
#include "PartFile.h"
#include "DownloadQueue.h"
#include "sockets.h"
#include "SafeFile.h"
#include "ClientUDPSocket.h"
#include "updownclient.h"
#include "Preferences.h"


#define BUFSIZE 1024
#define MAXADDRSTR 16
#define LOOPCOUNT 100

#define OP_VC_BC_HEADER		0xf1	//Hex "0xBx"

#define OP_BC_REQUESTSOURCE			0xB1
#define OP_BC_ANSWERSOURCE			0xB2

class CInternalSocket : public CAsyncSocket{

	public:

		CInternalSocket();
		~CInternalSocket();

		void Broadcast(const uchar* filehash);
        void Process(char* buffer, uint32 buffersize);

	protected:

		virtual void	OnSend(int nErrorCode);	
		virtual void	OnReceive(int nErrorCode);
		
	private:
		SOCKET m_hSocket2;
		SOCKADDR_IN stSrcAddr, stDestAddr;

};

