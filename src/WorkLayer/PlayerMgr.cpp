/*
 * $Id: PlayerMgr.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
// PlayerMgr.cpp : 实现文件
//

#include "stdafx.h"
//#include "emule.h"
#include "PlayerMgr.h"

#define _MAX_BUFFER_SIZE (1024*1024*10)
const int _REQ_DATA_LEN =(1024*1024);
// CPlayerMgr

const char * httphead="HTTP/1.1 200 OK\r\nServer: Microsoft-IIS/5.0\r\nX-Powered-By: ASP.NET\r\nContent-Length: %d\r\n\r\n";
//const char * httphead_part = "HTTP/1.1 206 Partial content\r\nServer: Microsoft-IIS/5.0\r\nX-Powered-By: ASP.NET\r\nContent-Length: %d\r\nContent-Range: bytes %d-%d/%d\r\n\r\n";
//const char * httphead_part = "HTTP/1.1 206 Partial content\r\nServer: Microsoft-IIS/5.0\r\nX-Powered-By: ASP.NET\r\nContent-Range: bytes %d-\r\n\r\n";
//const char * httphead_part = "HTTP/1.1 206 Partial content\r\nContent-Length: %d\r\nContent-Range: bytes %d-%d/%d\r\n\r\n";
const char * httphead_part = "HTTP/1.1 206 Partial content\r\nContent-Range: bytes %d-\r\n\r\n";

inline bool IsSocketClosed(DWORD dwError)
{
	switch(dwError)
	{
	case WSAECONNABORTED:
	case WSAENOTSOCK:
	case WSAECONNRESET:
	case WSAESHUTDOWN:
		return true;
	}

	return false;
}

CMutex CPlayerMgr::m_Mutex;
CPlayerMgr * g_PlayerMgr = NULL;
//PlayerDataCallback CPlayerMgr::m_PlayerCallback = NULL;
HWND g_hNotifyWnd = NULL;

IMPLEMENT_DYNCREATE(CPlayerMgr, CWinThread)

CPlayerMgr::CPlayerMgr()
{
	m_nListenPort = 0;
}

void CListenPlayerSocket::OnAccept(int nErrorCode)
{
	// TODO: 在此添加专用代码和/或调用基类
	CAsyncSocket * pNewSock = new CPlayerTaskSocket;
	if(!Accept(*pNewSock))
	{
		delete pNewSock;
	}
	else
	{
		TRACE("%08x new connection\n", pNewSock);
		pNewSock->AsyncSelect(FD_READ | FD_CLOSE);
		((CPlayerTaskSocket *)pNewSock)->m_connected = true;
	}
	CAsyncSocket::OnAccept(nErrorCode);
}

bool CPlayerTask::RequestData()
{
	if(m_SockList.IsEmpty())
		return false;

	POSITION pos = m_SockList.GetHeadPosition();
	while(pos)
	{
		POSITION posOld = pos;
		CPlayerTaskSocket * sk = m_SockList.GetNext(pos);
		if(! sk->RequestData())
		{
			m_SockList.RemoveAt(posOld);
			g_PlayerMgr->AddSocketToDelete(sk);
		}
	}
	return true;
}

bool CPlayerTaskSocket::RequestData()
{
	if(!m_connected)
		return false;

	DWORD tmNow = GetTickCount();

	if(m_tmSendData && (tmNow - m_tmSendData > 1000 * 5))
	{
		if(m_uCurrentPos>=m_pTask->m_uTotalFileSize)
		{
			if(m_BufferLst.IsEmpty())
			{
				//return false;
			}
			else SendBuffer(false);
		}
		else SendTinyData();
	}

	if(tmNow < m_tmReqData)
		return true;

	ASSERT(m_pTask);
	m_tmReqData = tmNow + 100;

	if(m_uCurrentPos>=m_pTask->m_uTotalFileSize)
	{
		SendBuffer(false);
		if(m_BufferLst.IsEmpty())
		{
			return false;
		}
		return true;
	}

	if(m_nTotalBufferSize>_MAX_BUFFER_SIZE)
	{
		SendBuffer(true);
		m_tmReqData = tmNow + 800;
		return true;
	}

	PLAYER_DATA_REQ * req=new PLAYER_DATA_REQ;
	memcpy(req->filehash, m_pTask->m_filehash, 16);
	req->pos = m_uCurrentPos;
	req->len = _REQ_DATA_LEN;
	TRACE("%08x current pos=%d\n", this, m_uCurrentPos);
	if(req->pos + req->len > m_pTask->m_uTotalFileSize)
	{
		req->len = m_pTask->m_uTotalFileSize - req->pos;
	}

	ASSERT(g_hNotifyWnd);
	::PostMessage(g_hNotifyWnd, WM_PLAYER_DATA_REQ, 0, (LPARAM) req);

	return true;
}

void CPlayerTaskSocket::SaveBuffer(void * pData, int nLen)
{
	if(pData==NULL || nLen<=0)
		return;

	TCP_BUFFER * pBuf = new TCP_BUFFER;
	pBuf->nType = _TCP_BUF_RAW;
	pBuf->nLen = nLen;
	pBuf->nOffset = 0;
	pBuf->pBuf = new char [nLen];
	memcpy(pBuf->pBuf, pData, nLen);

	m_BufferLst.AddTail( pBuf);
	m_nTotalBufferSize += nLen;
}

void CPlayerTaskSocket::SendBuffer(bool bKeepTail)
{
	int nRet;
	while(! m_BufferLst.IsEmpty())
	{
		if(bKeepTail && (m_nTotalBufferSize<MAX_CACHE_SIZE || m_BufferLst.GetSize()==1))
			break;

		TCP_BUFFER * pBuf = m_BufferLst.GetHead();
		if(pBuf->nType==_TCP_BUF_RAW)
		{
			nRet = Send(pBuf->pBuf + pBuf->nOffset, pBuf->nLen-pBuf->nOffset);
			if(nRet<=0)
			{
				m_tmReqData = GetTickCount() + 1000;

				DWORD dwErr=WSAGetLastError();
				if(IsSocketClosed(dwErr))
				{
					if(m_pTask)
					{
						POSITION pos = m_pTask->m_SockList.Find(this);
						if(pos) m_pTask->m_SockList.RemoveAt(pos);
						g_PlayerMgr->AddSocketToDelete(this);
					}
				}
				break;
			}

			m_tmSendData = GetTickCount();
			m_nTotalBufferSize -= nRet;
			pBuf->nOffset += nRet;
			if(pBuf->nLen <= pBuf->nOffset)
			{
				delete [] pBuf->pBuf;
				delete pBuf;

				m_BufferLst.RemoveHead();
			}
			else
			{
				break;
			}
		}
		//else if(pBuf->nType==_TCP_BUF_FILE)
		//{
		//	if(pBuf->nLen<=0)
		//	{
		//		delete pBuf;
		//		m_BufferLst.RemoveHead();
		//		continue;
		//	}

		//	m_fileToRead.Seek(pBuf->nOffset, CFile::begin);
		//	char buf[10240];
		//	int to_read = pBuf->nLen>10240? 10240 : pBuf->nLen;
		//	int ret_read = m_fileToRead.Read(buf, to_read);
		//	nRet = m_pSocket->Send(buf, ret_read);
		//	if(nRet<=0)
		//		break;
		//	else
		//	{
		//		pBuf->nOffset+=nRet;
		//		pBuf->nLen -= nRet;

		//		if(nRet<ret_read)
		//			break;
		//	}

		//	m_dwTotalSent += nRet;
		//}
	}
}

void CPlayerTaskSocket::SendTinyData()
{
	int nRet;
	if(! m_BufferLst.IsEmpty())
	{
		TCP_BUFFER * pBuf = m_BufferLst.GetHead();

		if(pBuf->nOffset==pBuf->nLen)
		{
			delete [] pBuf->pBuf;
			delete pBuf;

			m_BufferLst.RemoveHead();
			SendTinyData();
			return;
		}

		ASSERT(pBuf->nOffset<pBuf->nLen);

		nRet = Send(pBuf->pBuf + pBuf->nOffset, 1);
		if(nRet<=0) return;

		m_tmSendData = GetTickCount();
		m_nTotalBufferSize -= nRet;
		pBuf->nOffset += nRet;
		if(pBuf->nLen <= pBuf->nOffset)
		{
			delete [] pBuf->pBuf;
			delete pBuf;

			m_BufferLst.RemoveHead();
		}
	}
}

//bool CPlayerTask::SendData(void * pData, int nLen)
//{
//	if(! m_pSocket)
//	{
//		if(m_nTotalBufferSize>_MAX_BUFFER_SIZE)
//			return false;
//
//		SaveBuffer(pData, nLen);
//		return true;
//	}
//
//	SendBuffer();
//
//	if(! m_BufferLst.IsEmpty())
//	{
//		if(m_nTotalBufferSize>_MAX_BUFFER_SIZE)
//			return false;
//
//		SaveBuffer(pData, nLen);
//		return true;
//	}
//
//	int nRet=m_pSocket->Send(pData, nLen);
//	if(nRet<=0)
//	{
//		if(m_nTotalBufferSize>_MAX_BUFFER_SIZE)
//			return false;
//
//		SaveBuffer(pData, nLen);
//	}
//	else if(nRet<nLen)
//	{
//		const char * p=(char*)pData;
//
//		SaveBuffer((void*)(p+nRet), nLen-nRet);
//	}
//	return true;
//}

CPlayerTaskSocket::~CPlayerTaskSocket()
{
	if(g_PlayerMgr)
	{
		g_PlayerMgr->RemovePlayerTaskBySocket(this);
	}

	POSITION pos = m_BufferLst.GetHeadPosition();
	while(pos)
	{
		TCP_BUFFER * buf=m_BufferLst.GetNext(pos);
		delete []buf->pBuf;
		delete buf;
	}
	m_BufferLst.RemoveAll();
}

void CPlayerTaskSocket::ClearBuffer()
{
	POSITION pos = m_BufferLst.GetHeadPosition();
	while(pos)
	{
		TCP_BUFFER * pBuf= m_BufferLst.GetNext(pos);

		delete [] pBuf->pBuf;
		delete pBuf;
	}
}
void CPlayerTask::AddSocket(CPlayerTaskSocket * sk)
{
	m_bStarted = true;

	if(m_SockList.Find(sk, NULL))
		return;

	m_SockList.AddTail(sk);
	sk->m_pTask = this;
}

void CPlayerTaskSocket::OnReceive(int nErrorCode)
{
	ASSERT(g_PlayerMgr);

	char buffer[10240];
	int n=Receive(buffer, 10240);
	if(n == 0)
	{
		Close();
	}
	else if(n>0)
	{
		buffer[n] = 0;
		CStringA strHashId;
		int nRange = 0;
		if(ParseHttpReq(buffer, strHashId, nRange))
		{
			CSKey key;
			DWORD * pKey = (DWORD *)key.m_key;
			sscanf(strHashId, "%08x%08x%08x%08x", pKey, pKey+1, pKey+2, pKey+3);

			CSingleLock slock(&g_PlayerMgr->m_Mutex, true);

			CPlayerTask * pTask = g_PlayerMgr->GetPlayerTask(key);
			if(pTask)
			{
				TRACE("file size =%d\n", pTask->m_uTotalFileSize);
				//m_bRangeReq = true;
				ASSERT(m_nTotalBufferSize==0);
				pTask->AddSocket(this);
				//if(nRange > 222406523)
				//	nRange = 222406523;
				m_uCurrentPos = nRange;
				m_bSentData = true;
				if(nRange)
				{
					//if(m_uTailPos==0)
					{
						//m_uTailPos = m_uCurrentPos;
						//CStringA strHead = pTask->GetHeader(m_uTailPos);
						//SaveBuffer((void*)(const char*)strHead, strHead.GetLength());
					}
					//else
					{
					}
				}
				//else
				{
					CStringA strHead = pTask->GetHeader(nRange); 
					SaveBuffer((void*)(const char*)strHead, strHead.GetLength());
				}
			}

		}
	}

	CAsyncSocket::OnReceive(nErrorCode);
}

bool CPlayerTaskSocket::ParseHttpReq(CStringA strHtml, CStringA & rHashId, int & rnRange)
{
	rnRange = 0;
	rHashId.Empty();

	//  Not http GET command
	if(strnicmp(strHtml, "GET ", 4)!=0)
		return false;

	strHtml.Delete(0, 4);
	if(strHtml.GetAt(0)=='/')
		strHtml.Delete(0);

	int i;
	i=strHtml.Find("/");
	if(i<0) return false;

	rHashId = strHtml.Mid(0, i);
	strHtml.MakeLower();
	int n=strHtml.Find("range:");
	if(n>0)
	{
		n+=6;
		int iRangeEnd=strHtml.Find("\r\n", n);
		CStringA strRange=strHtml.Mid(n, iRangeEnd-n);
		while(! strRange.IsEmpty())
		{
			if(!isdigit(strRange.GetAt(0)))
				strRange.Delete(0);
			else break;
		}

		if(strRange.GetAt(strRange.GetLength()-1)=='-')
			strRange.Delete(strRange.GetLength()-1);

		strRange.Trim();

		rnRange = atol(strRange);
	}
	return true;
}
bool g_bWaitExit=false;
CPlayerMgr::~CPlayerMgr()
{
	g_bWaitExit = true;
}

BOOL CPlayerMgr::InitInstance()
{
	return TRUE;
}

int CPlayerMgr::ExitInstance()
{
	// TODO: 在此执行任意逐线程清理
	CSingleLock slock(&m_Mutex, true);

	POSITION pos = m_TaskLst.GetStartPosition();
	while(pos)
	{
		CSKey key;
		CPlayerTask * p;
		m_TaskLst.GetNextAssoc(pos, key, p);
		if(p) delete p;
	}
	m_TaskLst.RemoveAll();

	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(CPlayerMgr, CWinThread)
END_MESSAGE_MAP()

// CPlayerMgr 消息处理程序

CStringA CPlayerTask::GetHeader(UINT64 uPos)
{
	CStringA strTmpA;
	if(uPos==0)
		strTmpA.Format(httphead, m_uTotalFileSize);
	else
	{
		UINT64 content_len = m_uTotalFileSize - uPos;
		//DWORD stop_pos = m_uTotalFileSize - 1;
		UINT64 stop_pos;// = dwPos + 1024*1024*5;
		stop_pos = m_uTotalFileSize - 1;
		content_len = stop_pos - uPos;
		//strTmpA.Format(httphead_part, content_len, dwPos, stop_pos, m_uTotalFileSize);
		strTmpA.Format(httphead_part, uPos);
	}
	return strTmpA;
}

int CPlayerMgr::StartPlayer(const uchar * filehash, CString strFilename, UINT64 uFileSize, CString strExt)
{
	if(! g_PlayerMgr)
	{
		g_hNotifyWnd = AfxGetMainWnd()->GetSafeHwnd();
		g_PlayerMgr = (CPlayerMgr*)AfxBeginThread(RUNTIME_CLASS(CPlayerMgr));

		//  开始成功端口侦听
		while(g_PlayerMgr->GetListenPort()==0)
		{
			Sleep(100);
		}
	}

	if(strExt.IsEmpty() || filehash==NULL)
		return 1;

	CSKey key(filehash);

	CSingleLock lock(&CPlayerMgr::m_Mutex, true);

	TCHAR szBuf[_MAX_PATH], szBuf2[_MAX_PATH];
	ULONG ulSize = _MAX_PATH;

	CString strExeFile;
	CRegKey reg;
	if(reg.Open(HKEY_CLASSES_ROOT, _T(".")+strExt)==ERROR_SUCCESS)
	{
		if(reg.QueryStringValue(NULL, szBuf, &ulSize)==ERROR_SUCCESS)
		{
			CString strCmd;
			strCmd.Format(_T("%s\\shell"), szBuf);
			if(reg.Open(HKEY_CLASSES_ROOT, strCmd)==ERROR_SUCCESS)
			{
				ulSize = _MAX_PATH;
				if(reg.QueryStringValue(NULL, szBuf2, &ulSize)==ERROR_SUCCESS)
				{
					strCmd.Format(_T("%s\\shell\\%s\\command"), szBuf, szBuf2);
					if(reg.Open(HKEY_CLASSES_ROOT, strCmd)==ERROR_SUCCESS)
					{
						ulSize = _MAX_PATH;
						if(reg.QueryStringValue(NULL, szBuf, &ulSize)==ERROR_SUCCESS)
						{
							strExeFile = szBuf;
						}
					}
				}
			}
		}
	}

	//  使用配置的播放器
	if(strExeFile.IsEmpty())
	{
		// todo
		return 2;
	}

	CString strCmdLine;
	DWORD * pKey=(DWORD*)key.m_key;
	strCmdLine.Format(_T("\"http://127.0.0.1:%d/%08x%08x%08x%08x/%s\""), g_PlayerMgr->GetListenPort(),
		pKey[0], pKey[1], pKey[2], pKey[3], strFilename);

	strExeFile.Replace(_T("\"%1\""), _T(""));
	strExeFile.Replace(_T("%1"), _T(""));
	strExeFile.Trim();

	//  把参数剥出来
	int nParamPos;
	CString strTmp = strExeFile;
	while((nParamPos = strTmp.ReverseFind(_T('.'))) >= 0)
	{
		LPCTSTR szExe = strTmp;
		if(_tcsnicmp(szExe + nParamPos, _T(".exe"), 4)==0)
		{
			nParamPos += 4;
			break;
		}
		else
		{
			strTmp = strTmp.Left(nParamPos);
		}
	}

	if(nParamPos==-1)
	{
		return 2;
	}

	CString strExeParam;
	if(nParamPos<strExeFile.GetLength()-1)
	{
		if(strExeFile.GetAt(nParamPos)==_T('\"'))
			nParamPos++;

		strExeParam = strExeFile.Mid(nParamPos);
		strExeFile = strExeFile.Left(nParamPos);
	}

	CPlayerTask * pTask = g_PlayerMgr->AddPlayerTask(key);
	pTask->m_uTotalFileSize = uFileSize;
//	CStringA strTmpA;
//	strTmpA.Format(httphead, dwFileSize);
//	pTask->SendData((void*)(const char*)strTmpA, strTmpA.GetLength());

	//if(! pTask->m_fileToRead.Duplicate() Open(strFilepath, CFile::modeRead|CFile::shareDenyWrite))
	//{
	//	g_PlayerMgr->RemovePlayerTask(key);
	//	return 3;
	//}

	ShellExecute(NULL,_T("open"), strExeFile, strCmdLine + strExeParam, NULL, SW_SHOW);
	return 0;
}

int CPlayerMgr::Run()
{
	// TODO: 在此添加专用代码和/或调用基类
	ASSERT_VALID(this);
#ifndef _AFXDLL 
#define _AFX_SOCK_THREAD_STATE AFX_MODULE_THREAD_STATE 
#define _afxSockThreadState AfxGetModuleThreadState() 
	_AFX_SOCK_THREAD_STATE* pState = _afxSockThreadState; 
	if (pState->m_pmapSocketHandle == NULL) 
		pState->m_pmapSocketHandle = new CMapPtrToPtr; 
	if (pState->m_pmapDeadSockets == NULL) 
		pState->m_pmapDeadSockets = new CMapPtrToPtr; 
	if (pState->m_plistSocketNotifications == NULL) 
		pState->m_plistSocketNotifications = new CPtrList; 
#endif

	if (!m_skListen.Socket(SOCK_STREAM, FD_ACCEPT))
		return false;

	srand(time(NULL));
	WORD nListenPort;
#ifdef _DEBUG
	nListenPort = 8880;
#else
	nListenPort = 3000 + rand() % 1200;
#endif
	while(!m_skListen.Bind(nListenPort))
	{
		TRACE("bind error: %d\n", WSAGetLastError());
		Sleep(1000);
	}

	m_nListenPort = nListenPort;

	m_skListen.Listen(5);

	_AFX_THREAD_STATE* pTState = AfxGetThreadState();

	for(;;)
	{
		Process();
		if(::PeekMessage(&(pTState->m_msgCur), NULL, NULL, NULL, PM_NOREMOVE))
		{
			if (!PumpMessage())
				return ExitInstance();
		}
	}
}

bool CPlayerTask::SendData(UINT64 uPos, void * pData, int nLen)
{
	if(m_SockList.IsEmpty())
		return false;

	bool bSent = false;
	POSITION pos = m_SockList.GetHeadPosition();
	while(pos)
	{
		POSITION posOld = pos;
		CPlayerTaskSocket * sk=m_SockList.GetNext(pos);
		if(sk->m_uCurrentPos == uPos)
		{
			//  数据太少，保存下来，用来保持连接
			if(nLen<=MAX_CACHE_SIZE)
			{
				if(nLen<=0 || pData==NULL)
				{
					sk->m_tmReqData = GetTickCount() + 1000 * 5;
					continue;
				}

				//if(sk->m_nTotalBufferSize>_MAX_BUFFER_SIZE)
				//	continue;

				if(sk->m_nTotalBufferSize > MAX_CACHE_SIZE && nLen>=MAX_CACHE_SIZE)
					sk->SendBuffer(false);

				sk->SaveBuffer(pData, nLen);
				sk->m_uCurrentPos+=nLen;

				if(sk->m_uCurrentPos >= m_uTotalFileSize)
				{
					sk->SendBuffer(false);
				}

				continue;
			}
			else
			{
				//  先把老的数据发送
				if(!sk->m_BufferLst.IsEmpty())
				{
					sk->SendBuffer(false);
				}

				if(!sk->m_BufferLst.IsEmpty())
				{
					sk->SaveBuffer(pData, nLen);
					sk->m_uCurrentPos += nLen;
					sk->m_tmReqData = GetTickCount() + 1000 * 5;
					continue;
				}
			}

			bSent = true;
			int nSent=-1;
			//if(m_uTotalFileSize - dwPos > 1024*1024*4)
			//	nSent=sk->Send(pData, nLen-MAX_CACHE_SIZE);
			//else nSent = sk->Send(pData, nLen);
			if(m_uTotalFileSize - 1 <= uPos + nLen)
				nSent = sk->Send(pData, nLen);
			else nSent=sk->Send(pData, nLen-MAX_CACHE_SIZE);
			ASSERT(nLen>MAX_CACHE_SIZE);
			//nSent=sk->Send(pData, nLen-MAX_CACHE_SIZE);

			if(nSent<0)
			{
				DWORD dwErr=WSAGetLastError();

				if(IsSocketClosed(dwErr))
				{
					m_SockList.RemoveAt(posOld);
					g_PlayerMgr->AddSocketToDelete(sk);
				}
				else
				{
					sk->SaveBuffer((char*)pData, nLen);
					sk->m_uCurrentPos += nLen;
				}

				continue;
			}

			sk->m_tmSendData = GetTickCount();

			if(nSent<nLen-MAX_CACHE_SIZE)
			{
				sk->m_tmReqData = GetTickCount() + 1000;
			}

			if(nLen<_REQ_DATA_LEN)
			{
				sk->m_tmReqData = GetTickCount() + 1000 * 5;
			}

			sk->m_uCurrentPos += nLen;
			//  保存后面的一小部分，用来保持连接
			sk->SaveBuffer((char*)pData+nSent, nLen - nSent);

			//  到文件尾部，全部发送掉
			if(sk->m_uCurrentPos>=m_uTotalFileSize)
			{
				sk->SendBuffer(false);
			}
		}
	}

	return bSent;
}

void CPlayerMgr::Process()
{
	if(! m_ForDelete.IsEmpty())
	{
		CSingleLock slock(&m_Mutex, true);
		POSITION pos = m_ForDelete.GetStartPosition();
		while(pos)
		{
			CPlayerTaskSocket * sk;
			m_ForDelete.GetNextAssoc(pos, sk, sk);
			TRACE("%08x delete socket\n", sk);
			delete sk;
		}

		m_ForDelete.RemoveAll();
	}

	if(m_TaskLst.IsEmpty())
		Sleep(1000);
	else
	{
		Sleep(80);

		CSingleLock slock(&m_Mutex, true);

		POSITION pos = m_TaskLst.GetStartPosition();

		// VC-linhai[2007-08-07]: warning C4189: “dwNow” : 局部变量已初始化但不引用
		//DWORD dwNow = GetTickCount();
		while(pos)
		{
			CPlayerTask * task=NULL;
			CSKey k;
			m_TaskLst.GetNextAssoc(pos, k, task);
			if(task && task->m_SockList.IsEmpty()==FALSE)
			{
				if(! task->RequestData())
				{
					m_TaskLst.RemoveKey(k);
					pos = m_TaskLst.GetStartPosition();
				}
			}

			if(task && task->m_SockList.IsEmpty() && task->m_bStarted)
			{
				if(task->m_tmEmptyTask==0)
					task->m_tmEmptyTask = time(NULL);
				else if(time(NULL) - task->m_tmEmptyTask > 12)
				{
					m_TaskLst.RemoveKey(k);
					delete task;
				}
			}
			else
			{
				if(task) task->m_tmEmptyTask = 0;
			}
		}
	}
}

int CPlayerMgr::PlayerDataArrived(PLAYER_DATA_REQ * req, void * data, int len)
{
	if(req==NULL)
	{
		return 2;
	}
	if(!g_PlayerMgr) return 3;

	CSingleLock slock(&CPlayerMgr::m_Mutex, true);

	UINT64 pos = req->pos;
	CSKey key(req->filehash);

	CPlayerTask * pTask = g_PlayerMgr->GetPlayerTask(key);
	if(! pTask)
		return 3;

	if(!pTask->SendData(pos, data, len))
	{
		return 4;
	}



	return 0;
}

//void SetPlayerDataCallback(PlayerDataCallback pcb)
//{
//	CPlayerMgr::m_PlayerCallback = pcb;
//}
//
//void GetPlayerDataFromFile(const uchar * filehash, DWORD dwPos, DWORD dwLen)
//{
//	CSingleLock slock(&CPlayerMgr::m_Mutex, true);
//
//	CSKey key(filehash);
//	CPlayerTask * pTask = g_PlayerMgr->GetPlayerTask(key);
//	if(! pTask) return;
//
//	pTask->SaveBuffer(dwPos, dwLen);
//}

bool CPlayerMgr::IsTaskExist(const uchar * filehash)
{
	if(g_PlayerMgr==NULL) return false;

	CSingleLock slock(&CPlayerMgr::m_Mutex, true);

	CSKey key(filehash);
	if(g_PlayerMgr->m_TaskLst.IsEmpty())
		return false;

	CPlayerTask * t;
	if (g_PlayerMgr->m_TaskLst.Lookup(key, t))
	{
		return true;
	} 
	else
	{
		return false;
	}
	
}

void CPlayerMgr::StopPlayer(const uchar * filehash)
{
	if(g_PlayerMgr==NULL) return;

	CSingleLock slock(&CPlayerMgr::m_Mutex, true);

	CSKey key(filehash);
	g_PlayerMgr->RemovePlayerTask(key);
}

void CPlayerTaskSocket::OnClose(int nErrorCode)
{
	// TODO: 在此添加专用代码和/或调用基类
	m_connected = false;
	CAsyncSocket::OnClose(nErrorCode);
}

void CPlayerMgr::StopAllPlayer()
{
	if(g_PlayerMgr)
	{
		g_bWaitExit = false;

		CPlayerMgr * pOld=g_PlayerMgr;
		g_PlayerMgr = NULL;
		pOld->PostThreadMessage(WM_QUIT, 0, 0);
		while(! g_bWaitExit)
			Sleep(100);
	}
}
