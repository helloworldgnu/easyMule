/* 
 * $Id: Friend.h 4483 2008-01-02 09:19:06Z soarchin $
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


class CUpDownClient;
class CFileDataIO;

#define	FF_NAME		0x01

class CFriend
{
public:
	CFriend();
	CFriend(CUpDownClient* client);
	CFriend(const uchar* abyUserhash, uint32 dwLastSeen, uint32 dwLastUsedIP, uint16 nLastUsedPort, 
            uint32 dwLastChatted, LPCTSTR pszName, uint32 dwHasHash);
	~CFriend();

	uchar	m_abyUserhash[16];
	uint32	m_dwLastSeen;
	uint32	m_dwLastUsedIP;
	uint16	m_nLastUsedPort;
	uint32	m_dwLastChatted;
	uint32	m_dwHasHash;
	CString m_strName;

    CUpDownClient* GetLinkedClient() const { return m_LinkedClient; };
    void SetLinkedClient(CUpDownClient* linkedClient);

	void	LoadFromFile(CFileDataIO* file);
	void	WriteToFile(CFileDataIO* file);

	static const char sm_abyNullHash[16];

    void SetFriendSlot(bool newValue);
    bool GetFriendSlot() const;
private:
    bool HasUserhash();
    bool m_friendSlot;

	CUpDownClient* m_LinkedClient;
};
