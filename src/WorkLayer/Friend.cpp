/* 
 * $Id: Friend.cpp 7701 2008-10-15 07:34:41Z huby $
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

#include "stdafx.h"
#include "emule.h"
#include "Friend.h"
#include "FriendList.h"
#include "OtherFunctions.h"
#include "UpDownClient.h"
#include "Packets.h"
#include "SafeFile.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


const char CFriend::sm_abyNullHash[16] = {0};

CFriend::CFriend(void)
{
	m_dwLastSeen = 0;
	m_dwLastUsedIP = 0;
	m_nLastUsedPort = 0;
	m_dwLastChatted = 0;
	(void)m_strName;
	m_LinkedClient = 0;
	md4cpy(m_abyUserhash, sm_abyNullHash);
	m_dwHasHash = 0;

    m_friendSlot = false;
}

//Added this to work with the IRC.. Probably a better way to do it.. But wanted this in the release..
CFriend::CFriend(const uchar* abyUserhash, uint32 dwLastSeen, uint32 dwLastUsedIP, uint16 nLastUsedPort, 
				 uint32 dwLastChatted, LPCTSTR pszName, uint32 dwHasHash){
	m_dwLastSeen = dwLastSeen;
	m_dwLastUsedIP = dwLastUsedIP;
	m_nLastUsedPort = nLastUsedPort;
	m_dwLastChatted = dwLastChatted;
	if( dwHasHash && abyUserhash){
		md4cpy(m_abyUserhash,abyUserhash);
		m_dwHasHash = md4cmp(m_abyUserhash, sm_abyNullHash) ? 1 : 0;
	}
	else{
		md4cpy(m_abyUserhash, sm_abyNullHash);
		m_dwHasHash = 0;
	}
	m_strName = pszName;
	m_LinkedClient = 0;
    m_friendSlot = false;
}

CFriend::CFriend(CUpDownClient* client){
	ASSERT ( client );
	m_dwLastSeen = time(NULL);
	m_dwLastUsedIP = client->GetIP();
	m_nLastUsedPort = client->GetUserPort();
	m_dwLastChatted = 0;
    m_LinkedClient = NULL;
    m_friendSlot = false;
    SetLinkedClient(client);
}

CFriend::~CFriend(void)
{
    if(m_LinkedClient != NULL) {
        m_LinkedClient->SetFriendSlot(false);
        m_LinkedClient->m_Friend = NULL;
        m_LinkedClient = NULL;
    }
}

void CFriend::LoadFromFile(CFileDataIO* file)
{
	file->ReadHash16(m_abyUserhash);
	m_dwHasHash = md4cmp(m_abyUserhash, sm_abyNullHash) ? 1 : 0;
	m_dwLastUsedIP = file->ReadUInt32();
	m_nLastUsedPort = file->ReadUInt16();
	m_dwLastSeen = file->ReadUInt32();
	m_dwLastChatted = file->ReadUInt32();

	UINT tagcount = file->ReadUInt32();
	for (UINT j = 0; j < tagcount; j++){
		CTag* newtag = new CTag(file, false);
		switch (newtag->GetNameID()){
			case FF_NAME:{
				ASSERT( newtag->IsStr() );
				if (newtag->IsStr()){
					if (m_strName.IsEmpty())
						m_strName = newtag->GetStr();
				}
				break;
			}
		}
		delete newtag;
	}
}

void CFriend::WriteToFile(CFileDataIO* file)
{
	if (!m_dwHasHash)
		md4cpy(m_abyUserhash, sm_abyNullHash);
	file->WriteHash16(m_abyUserhash);
	file->WriteUInt32(m_dwLastUsedIP);
	file->WriteUInt16(m_nLastUsedPort);
	file->WriteUInt32(m_dwLastSeen);
	file->WriteUInt32(m_dwLastChatted);

	uint32 uTagCount = 0;
	ULONG uTagCountFilePos = (ULONG)file->GetPosition();
	file->WriteUInt32(uTagCount);

	if (!m_strName.IsEmpty()){
		if (WriteOptED2KUTF8Tag(file, m_strName, FF_NAME))
			uTagCount++;
		CTag nametag(FF_NAME, m_strName);
		nametag.WriteTagToFile(file);
		uTagCount++;
	}

	file->Seek(uTagCountFilePos, CFile::begin);
	file->WriteUInt32(uTagCount);
	file->Seek(0, CFile::end);
}

bool CFriend::HasUserhash() {
    for(int counter = 0; counter < 16; counter++) {
        if(m_abyUserhash[counter] != 0) {
            return true;
        }
    }

    return false;
}

void CFriend::SetFriendSlot(bool newValue) {
    if(m_LinkedClient != NULL) {
        m_LinkedClient->SetFriendSlot(newValue);
    }

    m_friendSlot = newValue;
}

bool CFriend::GetFriendSlot() const {
    if(m_LinkedClient != NULL) {
        return m_LinkedClient->GetFriendSlot();
    } else {
        return m_friendSlot;
    }
}

void CFriend::SetLinkedClient(CUpDownClient* linkedClient) {
    if(linkedClient != m_LinkedClient) {
        if(linkedClient != NULL) {
            if(m_LinkedClient == NULL) {
                linkedClient->SetFriendSlot(m_friendSlot);
            } else {
                linkedClient->SetFriendSlot(m_LinkedClient->GetFriendSlot());
            }

            m_dwLastSeen = time(NULL);
            m_dwLastUsedIP = linkedClient->GetIP();
            m_nLastUsedPort = linkedClient->GetUserPort();
            m_strName = linkedClient->GetUserName();
            md4cpy(m_abyUserhash,linkedClient->GetUserHash());
            m_dwHasHash = md4cmp(m_abyUserhash, sm_abyNullHash) ? 1 : 0;

            linkedClient->m_Friend = this;
        } else if(m_LinkedClient != NULL) {
            m_friendSlot = m_LinkedClient->GetFriendSlot();
        }

        if(m_LinkedClient != NULL) {
            // the old client is no longer friend, since it is no longer the linked client
            m_LinkedClient->SetFriendSlot(false);
            m_LinkedClient->m_Friend = NULL;
        }

        m_LinkedClient = linkedClient;
    }
#if _ENABLE_NOUSE
    theApp.friendlist->RefreshFriend(this);
#endif
}
