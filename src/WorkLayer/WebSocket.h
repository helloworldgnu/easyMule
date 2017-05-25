/*
 * $Id: WebSocket.h 4483 2008-01-02 09:19:06Z soarchin $
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

class CWebServer;

void StartSockets(CWebServer *pThis);
void StopSockets();

class CWebSocket 
{
public:
	void SetParent(CWebServer *);
	CWebServer* m_pParent;

	class CChunk 
	{
	public:
		char* m_pData;
		char* m_pToSend;
		DWORD m_dwSize;
		CChunk* m_pNext;

		~CChunk() { delete[] m_pData; }
	};

	CChunk* m_pHead; // tails of what has to be sent
	CChunk* m_pTail;

	char* m_pBuf;
	DWORD m_dwRecv;
	DWORD m_dwBufSize;
	DWORD m_dwHttpHeaderLen;
	DWORD m_dwHttpContentLen;

	bool m_bCanRecv;
	bool m_bCanSend;
	bool m_bValid;
	SOCKET m_hSocket;

	void OnReceived(void* pData, DWORD dwDataSize, in_addr inad); // must be implemented
	void SendData(const void* pData, DWORD dwDataSize);
	void SendData(LPCSTR szText) { SendData(szText, lstrlenA(szText)); }
	void SendContent(LPCSTR szStdResponse, const void* pContent, DWORD dwContentSize);
	void SendContent(LPCSTR szStdResponse, const CString& rstr);
	void SendReply(LPCSTR szReply);
	void Disconnect();

	void OnRequestReceived(char* pHeader, DWORD dwHeaderLen, char* pData, DWORD dwDataLen , in_addr inad);
};
