/* 
 * $Id: ListenSocket.h 5075 2008-03-20 10:40:20Z huby $
 * 
 * this file is part of eMule
 * Copyright (C)2002-2006 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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
#include "EMSocket.h"
#include "EMClientReqSocket.h"

class CUpDownClient;
class CPacket;
class CTimerWnd;
enum EDebugLogPriority;



class CListenSocket : public CAsyncSocketEx
{
	friend class CClientReqSocket;

public:
	CListenSocket();
	virtual ~CListenSocket();
	CAsyncSocketEx * OnAcceptEx(int nErrorCode);
	bool	StartListening();
	void	StopListening();
	virtual void OnAccept(int nErrorCode);
	void	Process();
	void	RemoveSocket(CClientReqSocket* todel);
	void	AddSocket(CClientReqSocket* toadd);
	UINT	GetOpenSockets()		{return socket_list.GetCount();}
	void	KillAllSockets();
	bool	TooManySockets(bool bIgnoreInterval = false,bool bUseTcp=false,bool bUseNat=false);	
	uint32	GetMaxConnectionReached()	{return maxconnectionreached;}
	bool    IsValidSocket(CClientReqSocket* totest);
	void	AddConnection();
	void	RecalculateStats();
	void	ReStartListening();
	void	Debug_ClientDeleted(CUpDownClient* deleted);
	bool	Rebind();
	bool	SendPortTestReply(char result,bool disconnect=false);

	void	UpdateConnectionsStatus();
	float	GetMaxConperFiveModifier();
	uint32	GetPeakConnections()		{ return peakconnections; }
	uint32	GetTotalConnectionChecks()	{ return totalconnectionchecks; }
	float	GetAverageConnections()		{ return averageconnections; }
	uint32	GetActiveConnections()		{ return activeconnections; }
	uint16	GetConnectedPort()			{ return m_port; }
	uint32	GetTotalHalfCon()			{ return m_nHalfOpen; }
	uint32	GetTotalComp()				{ return m_nComp; }

private:
	bool bListening;
	bool bPortListening;
	CTypedPtrList<CPtrList, CClientReqSocket*> socket_list;
	uint16	m_OpenSocketsInterval;
	uint32	maxconnectionreached;
	uint16	m_ConnectionStates[3];
	int		m_nPendingConnections;
	uint32	peakconnections;
	uint32	totalconnectionchecks;
	float	averageconnections;
	uint32	activeconnections;
	uint16  m_port;
	uint32	m_nHalfOpen;
	uint32  m_nHalfOpenOfL2L;
	uint32	m_nComp;
};
