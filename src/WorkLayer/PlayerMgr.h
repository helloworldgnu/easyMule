/*
 * $Id: PlayerMgr.h 4483 2008-01-02 09:19:06Z soarchin $
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
#include "MapKey.h"

//typedef int (*PlayerDataCallback)(const CSKey & key, void * pBuf, int nBufLen);

#define _TCP_BUF_RAW	1
#define _TCP_BUF_FILE	2

#define WM_PLAYER_DATA_REQ	(WM_USER + 7789)

const int MAX_CACHE_SIZE = 1024 * 2;

typedef struct {
	uchar filehash[16];
	UINT64 pos;
	UINT64 len;
} PLAYER_DATA_REQ;

typedef struct
{
	int nType;  //  1: raw data, 2: from file
	int nLen, nOffset;
	char * pBuf;
} TCP_BUFFER;

class CListenPlayerSocket : public CAsyncSocket
{
public:
	virtual void OnAccept(int nErrorCode);
};

class CPlayerTaskSocket : public CAsyncSocket
{
	friend class CPlayerTask;
	friend class CListenPlayerSocket;
public:
	CPlayerTaskSocket()
	{
		m_pTask = NULL;
		m_uCurrentPos = 0;

		m_tmReqData = 0;
		m_nTotalBufferSize = 0;
		m_tmSendData = 0;
		m_bSentData =false;
		m_bRangeReq = false;
		m_uTailPos = 0;
		m_connected = false;
	}

	void ClearBuffer();
	bool RequestData();
	void SendBuffer(bool bKeepTail);
	void SendTinyData();
	void SaveBuffer(void * pData, int nLen);
	virtual void OnReceive(int nErrorCode);
	virtual ~CPlayerTaskSocket();
private:
	bool ParseHttpReq(CStringA strHtml, CStringA & rHashId, int & rnRange);

	CPlayerTask * m_pTask;
	UINT64 m_uCurrentPos;
	DWORD m_tmReqData, m_tmSendData;
	CList<TCP_BUFFER*> m_BufferLst;
	int m_nTotalBufferSize;
	bool m_bSentData;
	bool m_bRangeReq;
	UINT64 m_uTailPos;
	BOOL m_connected;
public:
	virtual void OnClose(int nErrorCode);
};


class CPlayerTask
{
public:
	CPlayerTask(const uchar * fh)
	{
		memcpy(m_filehash, fh, 16);

		m_uTotalFileSize = 0;
		m_bStarted = false;
		m_tmEmptyTask = 0;
	}
	~CPlayerTask()
	{
		POSITION pos = m_SockList.GetHeadPosition();
		while(pos)
		{
			CPlayerTaskSocket * sk= m_SockList.GetNext(pos);
			delete sk;
		}
	}
	CStringA GetHeader(UINT64 uPos);
	void AddSocket(CPlayerTaskSocket * sk);
	bool RequestData();
	bool SendData(UINT64 uPos, void * pData, int nLen);

	CList<CPlayerTaskSocket *, CPlayerTaskSocket*> m_SockList;

	UINT64 m_uTotalFileSize;
	bool m_bStarted;
	uchar m_filehash[16];

	time_t m_tmEmptyTask;
};
// CPlayerMgr

class CPlayerMgr : public CWinThread
{
	DECLARE_DYNCREATE(CPlayerMgr)
protected:
	CPlayerMgr();           // 动态创建所使用的受保护的构造函数
	virtual ~CPlayerMgr();

	CMap<CSKey, const CSKey&, CPlayerTask *, CPlayerTask *> m_TaskLst;
	CListenPlayerSocket	m_skListen;
	WORD	m_nListenPort;
	CMap<CPlayerTaskSocket *,CPlayerTaskSocket *,CPlayerTaskSocket *,CPlayerTaskSocket *> m_ForDelete;

	void Process();
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

	static CMutex m_Mutex;
//	static PlayerDataCallback m_PlayerCallback;

	void AddSocketToDelete(CPlayerTaskSocket * sk)
	{
		CSingleLock lock(&m_Mutex, true);
		m_ForDelete[sk]  = sk;
	}

	CPlayerTask * GetPlayerTask(const CSKey & key)
	{
		CPlayerTask * pTask=NULL;
		m_TaskLst.Lookup(key, pTask);

		return pTask;
	}
	CPlayerTask * AddPlayerTask(const CSKey & key)
	{
		CPlayerTask * pTask;
		if(m_TaskLst.Lookup(key, pTask) && pTask)
		{
			pTask->m_tmEmptyTask = 0;
			return pTask;
		}

		pTask = new CPlayerTask(key.m_key);
		m_TaskLst.SetAt(key, pTask);

		return pTask;
	}
	void RemovePlayerTask(const CSKey & key)
	{
		if(m_TaskLst.IsEmpty()) return;

		CPlayerTask * pTask;

		if(m_TaskLst.Lookup(key, pTask))
		{
			delete pTask;
			m_TaskLst.RemoveKey(key);
		}
	}
	void RemovePlayerTaskBySocket(CPlayerTaskSocket * sock)
	{
		CSingleLock slock(&m_Mutex, true);
		POSITION pos = m_TaskLst.GetStartPosition();
		while(pos)
		{
			CSKey key;
			CPlayerTask * pTask;
			m_TaskLst.GetNextAssoc(pos, key, pTask);

			if(pTask && pTask->m_SockList.Find(sock, NULL)!=NULL)
			{
				pTask->m_SockList.RemoveAt(pTask->m_SockList.Find(sock, NULL));
				if(pTask->m_SockList.IsEmpty())
				{
					m_TaskLst.RemoveKey(key);
					delete pTask;
				}

				return ;
			}
		}
	}
	WORD GetListenPort()
	{
		return m_nListenPort;
	}

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual int Run();
	// 返回值
	//  0: 成功
	//  1: 该文件已经正在播放
	//  2: 注册表查询失败
	//  3: 无法打开文件
	static int StartPlayer(const uchar * filehash, CString strFilename, UINT64 uFileSize, CString strExt);

	// VC-SoarChin[2007-08-07]: {begin} Move all external functions into CPlayerMgr as static method
	// 返回值
	//  0: 成功
	//  1: 该文件已经取消
	//  2: 参数错误
	//  3: 还没开始
	//  4: 无法获得播放器
	static int PlayerDataArrived(PLAYER_DATA_REQ * req, void * data, int len);

	//extern void SetPlayerDataCallback(PlayerDataCallback pcb);
	//extern void GetPlayerDataFromFile(const uchar * filehash, DWORD dwPos, DWORD dwLen);
	static bool IsTaskExist(const uchar * filehash);

	static void StopPlayer(const uchar * filehash);
	static void StopAllPlayer();
	// VC-SoarChin[2007-08-07]: {end} Move all external functions into CPlayerMgr as static method
};

