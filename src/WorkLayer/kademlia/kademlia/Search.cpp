/* 
 * $Id: Search.cpp 11398 2009-03-17 11:00:27Z huby $
 * 
 * Copyright (C)2003 Barry Dunne (http://www.emule-project.net)
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

// Note To Mods //
/*
Please do not change anything here and release it..
There is going to be a new forum created just for the Kademlia side of the client..
If you feel there is an error or a way to improve something, please
post it in the forum first and let us look at it.. If it is a real improvement,
it will be added to the offical client.. Changing something without knowing
what all it does can cause great harm to the network if released in mass form..
Any mod that changes anything within the Kademlia side will not be allowed to advertise
there client on the eMule forum..
*/

#include "stdafx.h"
#include "./Search.h"
#include "./Kademlia.h"
#include "./Entry.h"
#include "./Defines.h"
#include "./Prefs.h"
#include "./Indexed.h"
#include "./UDPFirewallTester.h"
#include "./SearchManager.h"
#include "../io/IOException.h"
#include "../io/ByteIO.h"
#include "../routing/RoutingZone.h"
#include "../net/KademliaUDPListener.h"
//#include "../../emule.h"
#include "../../sharedfilelist.h"
#include "../../Packets.h"
#include "../../partfile.h"
//#include "../../emuledlg.h"
//#include "../../KadSearchListCtrl.h"
//#include "../../kademliawnd.h"
#include "../../DownloadQueue.h"
#include "../../SearchList.h"
#include "../../ClientList.h"
#include "../../UpDownClient.h"
#include "../../Log.h"
#include "../../KnownFileList.h"
#include "../utils/KadClientSearcher.h"
#include "../routing/Contact.h"
#include "../routing/Maps.h"
#include "../../GlobalVariable.h"
#include "../../UIMessage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace Kademlia;

void DebugSend(LPCTSTR pszMsg, uint32 uIP, uint16 uUDPPort);

CSearch::CSearch()
{
	m_tCreated = time(NULL);
	m_uType = (uint32)-1;
	m_uAnswers = 0;
	m_uTotalRequestAnswers = 0;
	m_uKadPacketSent = 0;
	m_uSearchID = (uint32)-1;
	(void)m_sFileName;
	m_bStoping = false;
	m_uTotalLoad = 0;
	m_uTotalLoadResponses = 0;

	//  Comment UI
	//theApp.emuledlg->kademliawnd->searchList->SearchAdd(this);
	SendMessage( CGlobalVariable::m_hListenWnd,WM_KAD_SEARCHADD,0,(LPARAM)this ); 

	m_uLastResponse = time(NULL);
	m_pucSearchTermsData = NULL;
	m_uSearchTermsDataSize = 0;
	pNodeSpecialSearchRequester = NULL;
	m_uClosestDistantFound = 0;
}

CSearch::~CSearch()
{
	
	// remember the closest node we found and tried to contact (if any) during this search
	// for statistical caluclations, but only if its a certain type
	switch(m_uType)
	{
		case NODECOMPLETE:
		case FILE:
		case KEYWORD:
		case NOTES:
		case STOREFILE:
		case STOREKEYWORD:
		case STORENOTES:
		case FINDSOURCE: // maybe also exclude
			if (m_uClosestDistantFound != 0)
				CKademlia::StatsAddClosestDistance(m_uClosestDistantFound);
			break;
		default: // NODE, NODESPECIAL, NODEFWCHECKUDP, FINDBUDDY
			break;
	}

	if (pNodeSpecialSearchRequester != NULL){
		// inform requester that our search failed
		pNodeSpecialSearchRequester->KadSearchIPByNodeIDResult(KCSR_NOTFOUND, 0, 0);
		pNodeSpecialSearchRequester = NULL;
	}	

	// Remove search from GUI
	//  Comment UI
	//theApp.emuledlg->kademliawnd->searchList->SearchRem(this);
	SendMessage( CGlobalVariable::m_hListenWnd,WM_KAD_SEARCHREM,0,(LPARAM)this ); 

	// Check if a source search is currently being done.
	CPartFile* pPartFile = CGlobalVariable::downloadqueue->GetFileByKadFileSearchID(GetSearchID());

	// Reset the searchID if a source search is currently being done.
	if(pPartFile)
	{
		pPartFile->SetKadFileSearchID(0);
#ifdef _DEBUG_KAD_
		Debug( _T("Delete a Kad SourceSearch(%u) for Partfile(%s)\r\n"),GetSearchID(),pPartFile->GetFileName() );
#endif
	}

	if (m_uType == NOTES){
		CAbstractFile* pAbstractFile = CGlobalVariable::knownfiles->FindKnownFileByID(CUInt128(GetTarget().GetData()).GetData());
		if (pAbstractFile != NULL)
			pAbstractFile->SetKadCommentSearchRunning(false);

		pAbstractFile = CGlobalVariable::downloadqueue->GetFileByID(CUInt128(GetTarget().GetData()).GetData());
		if (pAbstractFile != NULL)
			pAbstractFile->SetKadCommentSearchRunning(false);

		//  Comment UI
		CGlobalVariable::searchlist->SetNotesSearchStatus(CUInt128(GetTarget().GetData()).GetData(), false);
	}

	// Decrease the use count for any contacts that are in your contact list.
	for (ContactMap::iterator itContactMap = m_mapInUse.begin(); itContactMap != m_mapInUse.end(); ++itContactMap)
		((CContact*)itContactMap->second)->DecUse();

	// Delete any temp contacts..
	for (ContactList::const_iterator itContactList = m_listDelete.begin(); itContactList != m_listDelete.end(); ++itContactList)
		delete *itContactList;

	// Check if this search was contacting a overload node and adjust time of next time we use that node.
	if(CKademlia::IsRunning() && GetNodeLoad() > 20)
	{
		switch(GetSearchTypes())
		{
			case CSearch::STOREKEYWORD:
				Kademlia::CKademlia::GetIndexed()->AddLoad(GetTarget(), ((uint32)(DAY2S(7)*((double)GetNodeLoad()/100.0))+(uint32)time(NULL)));
				break;
		}
	}
	if(m_pucSearchTermsData)
		delete[] m_pucSearchTermsData;
}

void CSearch::Go()
{
	// Start with a lot of possible contacts, this is a fallback in case search stalls due to dead contacts
	if (m_mapPossible.empty())
	{
		CUInt128 uDistance(CKademlia::GetPrefs()->GetKadID());
		uDistance.Xor(m_uTarget);
		CKademlia::GetRoutingZone()->GetClosestTo(3, m_uTarget, uDistance, 50, &m_mapPossible, true, true);
	}
	if (!m_mapPossible.empty())
	{
		//Lets keep our contact list entries in mind to dec the inUse flag.
		for (ContactMap::iterator itContactMap = m_mapPossible.begin(); itContactMap != m_mapPossible.end(); ++itContactMap)
			m_mapInUse[itContactMap->first] = itContactMap->second;

		ASSERT(m_mapPossible.size() == m_mapInUse.size());

		// Take top ALPHA_QUERY to start search with.
		int iCount;
		
		if(m_uType == NODE)
			iCount = 1;
		else
			iCount = min(ALPHA_QUERY, (int)m_mapPossible.size());

		ContactMap::iterator itContactMap2 = m_mapPossible.begin();
		// Send initial packets to start the search.
		for (int iIndex=0; iIndex<iCount; iIndex++)
		{
			CContact* pContact = itContactMap2->second;
			// Move to tried
			m_mapTried[itContactMap2->first] = pContact;
			// Send the KadID so other side can check if I think it has the right KadID. (Saftey net)
			// Send request
			SendFindValue(pContact);
			++itContactMap2;
		}
	}
	// Update search for the GUI

	//  Comment UI
	//theApp.emuledlg->kademliawnd->searchList->SearchRef(this);
	SendMessage( CGlobalVariable::m_hListenWnd,WM_KAD_SEARCHREF,0,(LPARAM)this ); 
}

//If we allow about a 15 sec delay before deleting, we won't miss a lot of delayed returning packets.
void CSearch::PrepareToStop()
{
	// Check if already stoping..
	if( m_bStoping )
		return;

	// Set basetime by search type.
	uint32 uBaseTime;
	switch(m_uType)
	{
		case NODE:
		case NODECOMPLETE:
		case NODESPECIAL:
		case NODEFWCHECKUDP:
			uBaseTime = SEARCHNODE_LIFETIME;
			break;
		case FILE:
			uBaseTime = SEARCHFILE_LIFETIME;
			break;
		case KEYWORD:
			uBaseTime = SEARCHKEYWORD_LIFETIME;
			break;
		case NOTES:
			uBaseTime = SEARCHNOTES_LIFETIME;
			break;
		case STOREFILE:
			uBaseTime = SEARCHSTOREFILE_LIFETIME;
			break;
		case STOREKEYWORD:
			uBaseTime = SEARCHSTOREKEYWORD_LIFETIME;
			break;
		case STORENOTES:
			uBaseTime = SEARCHSTORENOTES_LIFETIME;
			break;
		case FINDBUDDY:
			uBaseTime = SEARCHFINDBUDDY_LIFETIME;
			break;
		case FINDSOURCE:
			uBaseTime = SEARCHFINDSOURCE_LIFETIME;
			break;
		default:
			uBaseTime = SEARCH_LIFETIME;
	}

	// Adjust created time so that search will delete within 15 seconds.
	// This gives late results time to be processed.
	m_tCreated = time(NULL) - uBaseTime + SEC(15);
	m_bStoping = true;

	//Update search within GUI.
	SendMessage( CGlobalVariable::m_hListenWnd,WM_KAD_SEARCHREF,0,(LPARAM)this ); 
}

void CSearch::JumpStart()
{
	// If we ran out of contacts, stop search.
	if (m_mapPossible.empty())
	{
		PrepareToStop();
		return;
	}

	// If we had a response within the last 3 seconds, no need to jumpstart the search.
	if (m_uLastResponse + SEC(3) > (uint32)time(NULL))
		return;

	// Search for contacts that can be used to jumpstart a stalled search.
	while(!m_mapPossible.empty())
	{
		// Get a contact closest to our target.
		CContact* pContact = m_mapPossible.begin()->second;

		// Have we already tried to contact this node.
		if (m_mapTried.count(m_mapPossible.begin()->first) > 0)
		{
			// Did we get a response from this node, if so, try to store or get info.
			if(m_mapResponded.count(m_mapPossible.begin()->first) > 0)
				StorePacket();
			// Remove from possible list.
			m_mapPossible.erase(m_mapPossible.begin());
		}
		else
		{
			// Add to tried list.
			m_mapTried[m_mapPossible.begin()->first] = pContact;
			// Send the KadID so other side can check if I think it has the right KadID. (Saftey net)
			// Send request
			SendFindValue(pContact);
			break;
		}
	}
}

void CSearch::ProcessResponse(uint32 uFromIP, uint16 uFromPort, ContactList *plistResults)
{
	if(plistResults)
		m_uLastResponse = time(NULL);

	// Remember the contacts to be deleted when finished
	for (ContactList::iterator itContactList = plistResults->begin(); itContactList != plistResults->end(); ++itContactList)
		m_listDelete.push_back(*itContactList);

	if (m_uType == NODEFWCHECKUDP){
		m_uAnswers++;
		delete plistResults;
		// Update search on the GUI.
		SendMessage( CGlobalVariable::m_hListenWnd,WM_KAD_SEARCHREF,0,(LPARAM)this ); 
		return;
	}
	// Not interested in responses for FIND_NODE.
	// Once we get a results we stop the search.
	// These contacts are added to contacts by UDPListener.
	if (m_uType == NODE)
	{
		// Note we got an answer
		m_uAnswers++;
		// We clear the possible list to force the search to stop.
		// We do this so the user has time to visually see the results.
		m_mapPossible.clear();
		delete plistResults;
		// Update search on the GUI.

		//  Comment UI
		//theApp.emuledlg->kademliawnd->searchList->SearchRef(this);
		SendMessage( CGlobalVariable::m_hListenWnd,WM_KAD_SEARCHREF,0,(LPARAM)this ); 
		return;
	}

	try
	{
		//Find contact that is responding.
		for (ContactMap::const_iterator itContactMap = m_mapTried.begin(); itContactMap != m_mapTried.end(); ++itContactMap)
		{
			CUInt128 uFromDistance(itContactMap->first);
			CContact* pFromContact = itContactMap->second;

			if ((pFromContact->GetIPAddress() == uFromIP) && (pFromContact->GetUDPPort() == uFromPort))
			{
				// Add to list of people who responded
				m_mapResponded[uFromDistance] = pFromContact;

				std::map<uint32, uint32> mapReceivedIPs;
				// Loop through their responses
				for (ContactList::iterator itContactList = plistResults->begin(); itContactList != plistResults->end(); ++itContactList)
				{
					// Get next result
					CContact* pContact = *itContactList;

					// Calc distance this result is to the target.
					CUInt128 uDistance(pContact->GetClientID());
					uDistance.Xor(m_uTarget);

					// Ignore this contact if already know or tried it.
					if (m_mapPossible.count(uDistance) > 0)
						continue;
					if (m_mapTried.count(uDistance) > 0)
						continue;
					// we only accept unique IPs in the answer, having multiple IDs pointing to one IP in the routing tables
					// is no longer allowed since 0.49a anyway
					if (mapReceivedIPs.count(pContact->GetIPAddress()) > 0){
						DebugLogWarning(_T("Multiple KadIDs pointing to same IP (%s) in KADEMLIA(2)_RES answer - ignored, sent by %s")
							, ipstr(ntohl(pContact->GetIPAddress())), ipstr(ntohl(pFromContact->GetIPAddress())));
						continue;
					}
					else
						mapReceivedIPs[pContact->GetIPAddress()] = 1;
					// Add to possible
					m_mapPossible[uDistance] = pContact;

					// Verify if the result is closer to the target then the one we just checked.
					if (uDistance < uFromDistance)
					{
						// The top APLPHA_QUERY of results are used to determine if we send a request.
						bool bTop = false;
						if (m_mapBest.size() < ALPHA_QUERY)
						{
							bTop = true;
							m_mapBest[uDistance] = pContact;
						}
						else
						{
							ContactMap::iterator itContactMapBest = m_mapBest.end();
							itContactMapBest--;
							if (uDistance < itContactMapBest->first)
							{
								// Prevent having more then ALPHA_QUERY within the Best list.
								m_mapBest.erase(itContactMapBest);
								m_mapBest[uDistance] = pContact;
								bTop = true;
							}
						}

						if(bTop)
						{
							// We determined this contact is a canditate for a request.
							// Add to the tried list.
							m_mapTried[uDistance] = pContact;
							// Send the KadID so other side can check if I think it has the right KadID. (Saftey net)
							// Send request
							SendFindValue(pContact);
						}
					}
				}
				// Complete node search, just increase the answers and update the GUI
				if( m_uType == NODECOMPLETE || m_uType == NODESPECIAL)
				{
					m_uAnswers++;
					//  Comment UI
					//theApp.emuledlg->kademliawnd->searchList->SearchRef(this);
					SendMessage( CGlobalVariable::m_hListenWnd,WM_KAD_SEARCHREF,0,(LPARAM)this ); 
				}
				break;
			}
		}
	}
	catch (...)
	{
		AddDebugLogLine(false, _T("Exception in CSearch::ProcessResponse"));
	}
	delete plistResults;
}

void CSearch::StorePacket()
{
	ASSERT(!m_mapPossible.empty());

	// This method is currently only called by jumpstart so only use best possible.
	ContactMap::const_iterator itContactMap = m_mapPossible.begin();
	CUInt128 uFromDistance(itContactMap->first);
	CContact* pFromContact = itContactMap->second;

	if (uFromDistance < m_uClosestDistantFound || m_uClosestDistantFound == 0)
		m_uClosestDistantFound = uFromDistance;
	// Make sure this is a valid Node to store too.
	// Shouldn't LAN IPs already be filtered?
	if(thePrefs.FilterLANIPs() && uFromDistance.Get32BitChunk(0) > SEARCHTOLERANCE)
		return;

	// What kind of search are we doing?
	switch(m_uType)
	{
		case FILE:
			{
				CSafeMemFile m_pfileSearchTerms;
				m_pfileSearchTerms.WriteUInt128(&m_uTarget);
				if (pFromContact->GetVersion() >= 3/*47b*/)
				{
					// Find file we are storing info about.
					uchar ucharFileid[16];
					m_uTarget.ToByteArray(ucharFileid);
					CKnownFile* pFile = CGlobalVariable::downloadqueue->GetFileByID(ucharFileid);
					if(pFile)
					{
						// JOHNTODO -- Add start position
						// Start Position range (0x0 to 0x7FFF)
						m_pfileSearchTerms.WriteUInt16(0);
						m_pfileSearchTerms.WriteUInt64(pFile->GetFileSize());
						if (thePrefs.GetDebugClientKadUDPLevel() > 0)
							DebugSend("KADEMLIA2_SEARCH_SOURCE_REQ", pFromContact->GetIPAddress(), pFromContact->GetUDPPort());
						if (pFromContact->GetVersion() >= 6){ /*48b*/
							CUInt128 uClientID = pFromContact->GetClientID();
							CKademlia::GetUDPListener()->SendPacket(&m_pfileSearchTerms, KADEMLIA2_SEARCH_SOURCE_REQ, pFromContact->GetIPAddress(), pFromContact->GetUDPPort(), pFromContact->GetUDPKey(), &uClientID);
						}
						else {
							CKademlia::GetUDPListener()->SendPacket(&m_pfileSearchTerms, KADEMLIA2_SEARCH_SOURCE_REQ, pFromContact->GetIPAddress(), pFromContact->GetUDPPort(), 0, NULL);
							ASSERT( CKadUDPKey(0) == pFromContact->GetUDPKey() );
						}
					}
					else
					{
						PrepareToStop();
						break;
					}
				}
				else
				{
					m_pfileSearchTerms.WriteUInt8(1);
					if (thePrefs.GetDebugClientKadUDPLevel() > 0)
						DebugSend("KADEMLIA_SEARCH_REQ(File)", pFromContact->GetIPAddress(), pFromContact->GetUDPPort());
					CKademlia::GetUDPListener()->SendPacket(&m_pfileSearchTerms, KADEMLIA_SEARCH_REQ, pFromContact->GetIPAddress(), pFromContact->GetUDPPort(), 0, NULL);
				}
				// Inc total request answers
				m_uTotalRequestAnswers++;
				// Update search in the GUI
				//  Comment UI
				//theApp.emuledlg->kademliawnd->searchList->SearchRef(this);
				SendMessage( CGlobalVariable::m_hListenWnd,WM_KAD_SEARCHREF,0,(LPARAM)this ); 
				break;
			}
		case KEYWORD:
			{
				//JOHNTODO -- We cannot precreate these packets as we do not know
				// before hand if we are talking to Kad1.0 or Kad2.0..
				CSafeMemFile m_pfileSearchTerms;
				m_pfileSearchTerms.WriteUInt128(&m_uTarget);
				if (pFromContact->GetVersion() >= 3/*47b*/)
				{
					if (m_uSearchTermsDataSize == 0)
					{
						// JOHNTODO - Need to add ability to change start position.
						// Start position range (0x0 to 0x7FFF)
						m_pfileSearchTerms.WriteUInt16((uint16)0x0000);
					}
					else
					{
						// JOHNTODO - Need to add ability to change start position.
						// Start position range (0x8000 to 0xFFFF)
						m_pfileSearchTerms.WriteUInt16((uint16)0x8000);
						m_pfileSearchTerms.Write(m_pucSearchTermsData, m_uSearchTermsDataSize);
					}
				}
				else
				{
					if (m_uSearchTermsDataSize == 0)
					{
						m_pfileSearchTerms.WriteUInt8(0);
						// We send this extra byte to flag we handle large files.
						m_pfileSearchTerms.WriteUInt8(0);
					}
					else
					{
						// Set to 2 to flag we handle handle large files.
						m_pfileSearchTerms.WriteUInt8(2);
						m_pfileSearchTerms.Write(m_pucSearchTermsData, m_uSearchTermsDataSize);
					}
				}

				if (pFromContact->GetVersion() >= 6){ /*48b*/
					if (thePrefs.GetDebugClientKadUDPLevel() > 0)
						DebugSend("KADEMLIA2_SEARCH_KEY_REQ", pFromContact->GetIPAddress(), pFromContact->GetUDPPort());
					CUInt128 uClientID = pFromContact->GetClientID();
					CKademlia::GetUDPListener()->SendPacket(&m_pfileSearchTerms, KADEMLIA2_SEARCH_KEY_REQ, pFromContact->GetIPAddress(), pFromContact->GetUDPPort(), pFromContact->GetUDPKey(), &uClientID);

				}
				else if (pFromContact->GetVersion() >= 3/*47b*/)
				{
					if (thePrefs.GetDebugClientKadUDPLevel() > 0)
						DebugSend("KADEMLIA2_SEARCH_KEY_REQ", pFromContact->GetIPAddress(), pFromContact->GetUDPPort());
					CKademlia::GetUDPListener()->SendPacket(&m_pfileSearchTerms, KADEMLIA2_SEARCH_KEY_REQ, pFromContact->GetIPAddress(), pFromContact->GetUDPPort(), 0, NULL);
					ASSERT( CKadUDPKey(0) == pFromContact->GetUDPKey() );
				}
				else
				{
					if (thePrefs.GetDebugClientKadUDPLevel() > 0)
						DebugSend("KADEMLIA_SEARCH_REQ(KEYWORD)", pFromContact->GetIPAddress(), pFromContact->GetUDPPort());
					CKademlia::GetUDPListener()->SendPacket(&m_pfileSearchTerms, KADEMLIA_SEARCH_REQ, pFromContact->GetIPAddress(), pFromContact->GetUDPPort(), 0, NULL);
				}
				// Inc total request answers
				m_uTotalRequestAnswers++;
				// Update search in the GUI

				//  Comment UI
				//theApp.emuledlg->kademliawnd->searchList->SearchRef(this);
				SendMessage( CGlobalVariable::m_hListenWnd,WM_KAD_SEARCHREF,0,(LPARAM)this ); 
				break;
			}
		case NOTES:
			{
				// Write complete packet
				CSafeMemFile m_pfileSearchTerms;
				m_pfileSearchTerms.WriteUInt128(&m_uTarget);

				if (pFromContact->GetVersion() >= 3/*47b*/)
				{
					// Find file we are storing info about.
					uchar ucharFileid[16];
					m_uTarget.ToByteArray(ucharFileid);
					CKnownFile* pFile = CGlobalVariable::sharedfiles->GetFileByID(ucharFileid);
					if(pFile)
					{
						m_pfileSearchTerms.WriteUInt64(pFile->GetFileSize());
						if (thePrefs.GetDebugClientKadUDPLevel() > 0)
							DebugSend("KADEMLIA2_SEARCH_NOTES_REQ", pFromContact->GetIPAddress(), pFromContact->GetUDPPort());
						if (pFromContact->GetVersion() >= 6){ /*48b*/
							CUInt128 uClientID = pFromContact->GetClientID();
							CKademlia::GetUDPListener()->SendPacket(&m_pfileSearchTerms, KADEMLIA2_SEARCH_NOTES_REQ, pFromContact->GetIPAddress(), pFromContact->GetUDPPort(), pFromContact->GetUDPKey(), &uClientID);
						}
						else {
							CKademlia::GetUDPListener()->SendPacket(&m_pfileSearchTerms, KADEMLIA2_SEARCH_NOTES_REQ, pFromContact->GetIPAddress(), pFromContact->GetUDPPort(), 0, NULL);
							ASSERT( CKadUDPKey(0) == pFromContact->GetUDPKey() );
						}
					}
					else
					{
						PrepareToStop();
						break;
					}
				}
				else
				{
					m_pfileSearchTerms.WriteUInt128(&CKademlia::GetPrefs()->GetKadID());
					if (thePrefs.GetDebugClientKadUDPLevel() > 0)
						DebugSend("KADEMLIA_SEARCH_NOTES_REQ", pFromContact->GetIPAddress(), pFromContact->GetUDPPort());
					CKademlia::GetUDPListener()->SendPacket(&m_pfileSearchTerms, KADEMLIA_SEARCH_NOTES_REQ, pFromContact->GetIPAddress(), pFromContact->GetUDPPort(), 0, NULL);
				}
				// Inc total request answers
				m_uTotalRequestAnswers++;
				// Update search in the GUI

				//  Comment UI
				//theApp.emuledlg->kademliawnd->searchList->SearchRef(this);
				SendMessage( CGlobalVariable::m_hListenWnd,WM_KAD_SEARCHREF,0,(LPARAM)this ); 
				break;
			}
		case STOREFILE:
			{
				// Try to store yourself as a source to a Node.
				// As a safe guard, check to see if we already stored to the Max Nodes
				if( m_uAnswers > SEARCHSTOREFILE_TOTAL )
				{
					PrepareToStop();
					break;
				}

				// Find the file we are trying to store as a source too.
				uchar ucharFileid[16];
				m_uTarget.ToByteArray(ucharFileid);
				CKnownFile* pFile = CGlobalVariable::sharedfiles->GetFileByID(ucharFileid);

				if (pFile)
				{
					// We set this mostly for GUI resonse.
					m_sFileName = pFile->GetFileName();

					// Get our clientID for the packet.
					CUInt128 uID(CKademlia::GetPrefs()->GetClientHash());

					//We can use type for different types of sources.
					//1 HighID Sources..
					//2 cannot be used as older clients will not work.
					//3 Firewalled Kad Source.
					//4 >4GB file HighID Source.
					//5 >4GB file Firewalled Kad source.
					//6 Firewalled Source with Direct Callback (supports >4GB)

					bool bDirectCallback = false;
					TagList listTag;
					if( CGlobalVariable::IsFirewalled() )
					{
						bDirectCallback = (Kademlia::CKademlia::IsRunning() && !Kademlia::CUDPFirewallTester::IsFirewalledUDP(true) && Kademlia::CUDPFirewallTester::IsVerified());
						if (bDirectCallback){
							// firewalled, but direct udp callback is possible so no need for buddies
							// We are not firewalled..
							listTag.push_back(new CKadTagUInt(TAG_SOURCETYPE, 6));
							listTag.push_back(new CKadTagUInt(TAG_SOURCEPORT, thePrefs.GetPort()));
							if (!CKademlia::GetPrefs()->GetUseExternKadPort())
								listTag.push_back(new CKadTagUInt16(TAG_SOURCEUPORT, CKademlia::GetPrefs()->GetInternKadPort()));
							if (pFromContact->GetVersion() >= 2/*47a*/)
							{
								listTag.push_back(new CKadTagUInt(TAG_FILESIZE, pFile->GetFileSize()));
							}							
						}
						else if( CGlobalVariable::clientlist->GetBuddy() ) // We are firewalled, make sure we have a buddy.
						{
							// We send the ID to our buddy so they can do a callback.
							CUInt128 uBuddyID(true);
							uBuddyID.Xor(CKademlia::GetPrefs()->GetKadID());
							if(pFile->GetFileSize() > OLD_MAX_EMULE_FILE_SIZE)
								listTag.push_back(new CKadTagUInt8(TAG_SOURCETYPE, 5));
							else
								listTag.push_back(new CKadTagUInt8(TAG_SOURCETYPE, 3));
							listTag.push_back(new CKadTagUInt(TAG_SERVERIP, CGlobalVariable::clientlist->GetBuddy()->GetIP()));
							listTag.push_back(new CKadTagUInt(TAG_SERVERPORT, CGlobalVariable::clientlist->GetBuddy()->GetUDPPort()));
							listTag.push_back(new CKadTagStr(TAG_BUDDYHASH, CStringW(md4str(uBuddyID.GetData()))));
							listTag.push_back(new CKadTagUInt(TAG_SOURCEPORT, thePrefs.GetPort()));
							if (!CKademlia::GetPrefs()->GetUseExternKadPort())
								listTag.push_back(new CKadTagUInt16(TAG_SOURCEUPORT, CKademlia::GetPrefs()->GetInternKadPort()));

							//  changed by yunchenn 2006/12/13
#ifdef _ENABLE_NATTRAVERSE
							listTag.push_back(new CKadTagUInt(TAG_SOURCEPORT, ULONG(-1)));
#else
							listTag.push_back(new CKadTagUInt(TAG_SOURCEPORT, thePrefs.GetPort()));
#endif

							if (pFromContact->GetVersion() >= 2/*47a*/)
							{
								listTag.push_back(new CKadTagUInt(TAG_FILESIZE, pFile->GetFileSize()));
							}
						}
						else
						{
							// We are firewalled, but lost our buddy.. Stop everything.
							PrepareToStop();
							break;
						}
					}
					else
					{
						// We are not firewalled..
						if(pFile->GetFileSize() > OLD_MAX_EMULE_FILE_SIZE)
							listTag.push_back(new CKadTagUInt(TAG_SOURCETYPE, 4));
						else
							listTag.push_back(new CKadTagUInt(TAG_SOURCETYPE, 1));
						listTag.push_back(new CKadTagUInt(TAG_SOURCEPORT, thePrefs.GetPort()));
						if (!CKademlia::GetPrefs()->GetUseExternKadPort())
							listTag.push_back(new CKadTagUInt16(TAG_SOURCEUPORT, CKademlia::GetPrefs()->GetInternKadPort()));

						if (pFromContact->GetVersion() >= 2/*47a*/)
						{
							listTag.push_back(new CKadTagUInt(TAG_FILESIZE, pFile->GetFileSize()));
						}
					}

					listTag.push_back(new CKadTagUInt8(TAG_ENCRYPTION, CKademlia::GetPrefs()->GetMyConnectOptions(true, true)));
					

					// Send packet
					CKademlia::GetUDPListener()->SendPublishSourcePacket(pFromContact, m_uTarget, uID, listTag);
					// Inc total request answers
					m_uTotalRequestAnswers++;
					// Update search in the GUI

					//  Comment UI
					//theApp.emuledlg->kademliawnd->searchList->SearchRef(this);
					SendMessage( CGlobalVariable::m_hListenWnd,WM_KAD_SEARCHREF,0,(LPARAM)this ); 
					// Delete all tags.
					for (TagList::const_iterator itTagList = listTag.begin(); itTagList != listTag.end(); ++itTagList)
						delete *itTagList;
				}
				else
					PrepareToStop();
				break;
			}
		case STOREKEYWORD:
			{
				// Try to store keywords to a Node.
				// As a safe guard, check to see if we already stored to the Max Nodes
				if( m_uAnswers > SEARCHSTOREKEYWORD_TOTAL )
				{
					PrepareToStop();
					break;
				}

				uint16 iCount = (uint16)m_listFileIDs.size();

				if(iCount == 0)
				{
					PrepareToStop();
					break;
				}
				else if(iCount > 150)
					iCount = 150;

				UIntList::const_iterator itListFileID = m_listFileIDs.begin();
				uchar ucharFileid[16];

				while(iCount && (itListFileID != m_listFileIDs.end()))
				{
					uint16 iPacketCount = 0;
					byte byPacket[1024*50];
					CByteIO byIO(byPacket,sizeof(byPacket));
					byIO.WriteUInt128(m_uTarget);
					byIO.WriteUInt16(0); // Will be corrected before sending.
					while((iPacketCount < 50) && (itListFileID != m_listFileIDs.end()))
					{
						CUInt128 iID = *itListFileID;
						iID.ToByteArray(ucharFileid);
						CKnownFile* pFile = CGlobalVariable::sharedfiles->GetFileByID(ucharFileid);
						if(pFile)
						{
							iCount--;
							iPacketCount++;
							byIO.WriteUInt128(iID);
							PreparePacketForTags( &byIO, pFile );
						}
						++itListFileID;
					}
					
					// Correct file count.
					uint32 current_pos = byIO.GetUsed();
					byIO.Seek(16);
					byIO.WriteUInt16(iPacketCount);
					byIO.Seek(current_pos);
					
					// Send packet
					if (pFromContact->GetVersion() >= 6){ /*48b*/
						if (thePrefs.GetDebugClientKadUDPLevel() > 0)
							DebugSend("KADEMLIA2_PUBLISH_KEY_REQ", pFromContact->GetIPAddress(), pFromContact->GetUDPPort());
						CUInt128 uClientID = pFromContact->GetClientID();
						CKademlia::GetUDPListener()->SendPacket( byPacket, sizeof(byPacket)-byIO.GetAvailable(), KADEMLIA2_PUBLISH_KEY_REQ, pFromContact->GetIPAddress(), pFromContact->GetUDPPort(), pFromContact->GetUDPKey(), &uClientID);

					}	
					else if (pFromContact->GetVersion() >= 2/*47a*/)
					{
						if (thePrefs.GetDebugClientKadUDPLevel() > 0)
							DebugSend("KADEMLIA2_PUBLISH_KEY_REQ", pFromContact->GetIPAddress(), pFromContact->GetUDPPort());
						CKademlia::GetUDPListener()->SendPacket( byPacket, sizeof(byPacket)-byIO.GetAvailable(), KADEMLIA2_PUBLISH_KEY_REQ, pFromContact->GetIPAddress(), pFromContact->GetUDPPort(), 0, NULL);
						ASSERT( CKadUDPKey(0) == pFromContact->GetUDPKey() );
					}
					else
					{
						if (thePrefs.GetDebugClientKadUDPLevel() > 0)
							DebugSend("KADEMLIA_PUBLISH_REQ", pFromContact->GetIPAddress(), pFromContact->GetUDPPort());
						CKademlia::GetUDPListener()->SendPacket( byPacket, sizeof(byPacket)-byIO.GetAvailable(), KADEMLIA_PUBLISH_REQ, pFromContact->GetIPAddress(), pFromContact->GetUDPPort(), 0, NULL);
					}
				}
				// Inc total request answers
				m_uTotalRequestAnswers++;
				// Update search in the GUI

				//  Comment UI
				//theApp.emuledlg->kademliawnd->searchList->SearchRef(this);
				SendMessage( CGlobalVariable::m_hListenWnd,WM_KAD_SEARCHREF,0,(LPARAM)this ); 
				break;
			}
		case STORENOTES:
			{
				// Find file we are storing info about.
				uchar ucharFileid[16];
				m_uTarget.ToByteArray(ucharFileid);
				CKnownFile* pFile = CGlobalVariable::sharedfiles->GetFileByID(ucharFileid);

				if (pFile)
				{
					byte byPacket[1024*2];
					CByteIO byIO(byPacket,sizeof(byPacket));

					// Send the Hash of the file we are storing info about.
					byIO.WriteUInt128(m_uTarget);
					// Send our ID with the info.
					byIO.WriteUInt128(CKademlia::GetPrefs()->GetKadID());

					// Create our taglist
					TagList listTag;
					listTag.push_back(new CKadTagStr(TAG_FILENAME, pFile->GetFileName()));
					if(pFile->GetFileRating() != 0)
						listTag.push_back(new CKadTagUInt(TAG_FILERATING, pFile->GetFileRating()));
					if(pFile->GetFileComment() != _T(""))
						listTag.push_back(new CKadTagStr(TAG_DESCRIPTION, pFile->GetFileComment()));
					if (pFromContact->GetVersion() >= 2/*47a*/)
						listTag.push_back(new CKadTagUInt(TAG_FILESIZE, pFile->GetFileSize()));
					byIO.WriteTagList(listTag);

					// Send packet
					if (pFromContact->GetVersion() >= 6){ /*48b*/
						if (thePrefs.GetDebugClientKadUDPLevel() > 0)
							DebugSend("KADEMLIA2_PUBLISH_NOTES_REQ", pFromContact->GetIPAddress(), pFromContact->GetUDPPort());
						CUInt128 uClientID = pFromContact->GetClientID();
						CKademlia::GetUDPListener()->SendPacket( byPacket, sizeof(byPacket)-byIO.GetAvailable(), KADEMLIA2_PUBLISH_NOTES_REQ, pFromContact->GetIPAddress(), pFromContact->GetUDPPort(), pFromContact->GetUDPKey(), &uClientID);
					}
					else if (pFromContact->GetVersion() >= 2/*47a*/)
					{
						if (thePrefs.GetDebugClientKadUDPLevel() > 0)
							DebugSend("KADEMLIA2_PUBLISH_NOTES_REQ", pFromContact->GetIPAddress(), pFromContact->GetUDPPort());
						CKademlia::GetUDPListener()->SendPacket( byPacket, sizeof(byPacket)-byIO.GetAvailable(), KADEMLIA2_PUBLISH_NOTES_REQ, pFromContact->GetIPAddress(), pFromContact->GetUDPPort(), 0, NULL);
						ASSERT( CKadUDPKey(0) == pFromContact->GetUDPKey() );
					}
					else
					{
						if (thePrefs.GetDebugClientKadUDPLevel() > 0)
							DebugSend("KADEMLIA_PUBLISH_NOTES_REQ", pFromContact->GetIPAddress(), pFromContact->GetUDPPort());
						CKademlia::GetUDPListener()->SendPacket( byPacket, sizeof(byPacket)-byIO.GetAvailable(), KADEMLIA_PUBLISH_NOTES_REQ, pFromContact->GetIPAddress(), pFromContact->GetUDPPort(), 0, NULL);
					}
					// Inc total request answers
					m_uTotalRequestAnswers++;
					// Update search in the GUI

					//  Comment UI
					//theApp.emuledlg->kademliawnd->searchList->SearchRef(this);
					SendMessage( CGlobalVariable::m_hListenWnd,WM_KAD_SEARCHREF,0,(LPARAM)this ); 
					// Delete all tags.
					for (TagList::const_iterator itTagList = listTag.begin(); itTagList != listTag.end(); ++itTagList)
						delete *itTagList;
				}
				else
					PrepareToStop();
				break;
			}
		case FINDBUDDY:
			{
				// Send a buddy request as we are firewalled.
				// As a safe guard, check to see if we already requested the Max Nodes
				if( m_uAnswers > SEARCHFINDBUDDY_TOTAL )
				{
					PrepareToStop();
					break;
				}

				CSafeMemFile m_pfileSearchTerms;
				// Send the ID we used to find our buddy. Used for checks later and allows users to callback someone if they change buddies.
				m_pfileSearchTerms.WriteUInt128(&m_uTarget);
				// Send client hash so they can do a callback.
				m_pfileSearchTerms.WriteUInt128(&CKademlia::GetPrefs()->GetClientHash());
				// Send client port so they can do a callback
				m_pfileSearchTerms.WriteUInt16(thePrefs.GetPort());

				// Do a keyword/source search request to this Node.
				// Send packet
#ifdef _DEBUG_KAD_
				Debug( _T("StorePacket:KADEMLIA_FINDBUDDY_REQ. \r\n") );
#endif
				if (thePrefs.GetDebugClientKadUDPLevel() > 0)
					DebugSend("KADEMLIA_FINDBUDDY_REQ", pFromContact->GetIPAddress(), pFromContact->GetUDPPort());
				if (pFromContact->GetVersion() >= 6){ /*48b*/
					CUInt128 uClientID = pFromContact->GetClientID();
					CKademlia::GetUDPListener()->SendPacket(&m_pfileSearchTerms, KADEMLIA_FINDBUDDY_REQ, pFromContact->GetIPAddress(), pFromContact->GetUDPPort(), pFromContact->GetUDPKey(), &uClientID);
				}
				else {
					CKademlia::GetUDPListener()->SendPacket(&m_pfileSearchTerms, KADEMLIA_FINDBUDDY_REQ, pFromContact->GetIPAddress(), pFromContact->GetUDPPort(), 0, NULL);
					ASSERT( CKadUDPKey(0) == pFromContact->GetUDPKey() );
				}
				// Inc total request answers
				m_uAnswers++;
				// Update search in the GUI

				//  Comment UI
				//theApp.emuledlg->kademliawnd->searchList->SearchRef(this);
				SendMessage( CGlobalVariable::m_hListenWnd,WM_KAD_SEARCHREF,0,(LPARAM)this ); 
				break;
			}
		case FINDSOURCE:
			{
				// Try to find if this is a buddy to someone we want to contact.
				// As a safe guard, check to see if we already requested the Max Nodes
				if( m_uAnswers > SEARCHFINDSOURCE_TOTAL )
				{
					PrepareToStop();
					break;
				}

				CSafeMemFile fileIO(34);
				// This is the ID the the person we want to contact used to find a buddy.
				fileIO.WriteUInt128(&m_uTarget);
				if( m_listFileIDs.size() != 1)
					throw CString(_T("Kademlia.CSearch.ProcessResponse: m_listFileIDs.size() != 1"));
				// Currently, we limit they type of callbacks for sources.. We must know a file it person has for it to work.
				fileIO.WriteUInt128(&m_listFileIDs.front());
				// Send our port so the callback works.
				fileIO.WriteUInt16(thePrefs.GetPort());
				// Send packet
				if (thePrefs.GetDebugClientKadUDPLevel() > 0)
					DebugSend("KADEMLIA_CALLBACK_REQ", pFromContact->GetIPAddress(), pFromContact->GetUDPPort());
				if (pFromContact->GetVersion() >= 6){ /*48b*/
					CUInt128 uClientID = pFromContact->GetClientID();
					CKademlia::GetUDPListener()->SendPacket( &fileIO, KADEMLIA_CALLBACK_REQ, pFromContact->GetIPAddress(), pFromContact->GetUDPPort(), pFromContact->GetUDPKey(), &uClientID);
				}
				else {
					CKademlia::GetUDPListener()->SendPacket( &fileIO, KADEMLIA_CALLBACK_REQ, pFromContact->GetIPAddress(), pFromContact->GetUDPPort(), 0, NULL);
					ASSERT( CKadUDPKey(0) == pFromContact->GetUDPKey() );
				}
				// Inc total request answers
				m_uAnswers++;
				// Update search in the GUI

				//  Comment UI
				//theApp.emuledlg->kademliawnd->searchList->SearchRef(this);
				SendMessage( CGlobalVariable::m_hListenWnd,WM_KAD_SEARCHREF,0,(LPARAM)this ); 
				break;
			}
		case NODESPECIAL:
			{
				// we are looking for the IP of a given nodeid, so we just check if we 0 distance and if so, report the
				// tip to the requester
				if (uFromDistance == CUInt128((ULONG)0)){
					pNodeSpecialSearchRequester->KadSearchIPByNodeIDResult(KCSR_SUCCEEDED, ntohl(pFromContact->GetIPAddress()), pFromContact->GetTCPPort());
					pNodeSpecialSearchRequester = NULL;
					PrepareToStop();
				}
				break;
			}
	}
}

void CSearch::ProcessResult(const CUInt128 &uAnswer, TagList *plistInfo)
{
	// We received a result, process it based on type.
	switch(m_uType)
	{
		case FILE:
			ProcessResultFile(uAnswer, plistInfo);
			break;
		case KEYWORD:
			ProcessResultKeyword(uAnswer, plistInfo);
			break;
		case NOTES:
			ProcessResultNotes(uAnswer, plistInfo);
			break;
	}
	// Update search for the GUI

	//  Comment UI
	//theApp.emuledlg->kademliawnd->searchList->SearchRef(this);
	SendMessage( CGlobalVariable::m_hListenWnd,WM_KAD_SEARCHREF,0,(LPARAM)this ); 
}

void CSearch::ProcessResultFile(const CUInt128 &uAnswer, TagList *plistInfo)
{
	// Process a possible source to a file.
	// Set of data we could receive from the result.
	uint8 uType = 0;
	uint32 uIP = 0;
	uint16 uTCPPort = 0;
	uint16 uUDPPort = 0;
	uint32 uBuddyIP = 0;
	uint16 uBuddyPort = 0;
	//    uint32 uClientID = 0;
	CUInt128 uBuddy;
	uint8 byCryptOptions = 0; // 0 = not supported

	for (TagList::const_iterator itTagList = plistInfo->begin(); itTagList != plistInfo->end(); ++itTagList)
	{
		CKadTag* pTag = *itTagList;
		if (!pTag->m_name.Compare(TAG_SOURCETYPE))
			uType = (uint8)pTag->GetInt();
		else if (!pTag->m_name.Compare(TAG_SOURCEIP))
			uIP = (uint32)pTag->GetInt();
		else if (!pTag->m_name.Compare(TAG_SOURCEPORT))
			uTCPPort = (uint16)pTag->GetInt();
		else if (!pTag->m_name.Compare(TAG_SOURCEUPORT))
			uUDPPort = (uint16)pTag->GetInt();
		else if (!pTag->m_name.Compare(TAG_SERVERIP))
			uBuddyIP = (uint32)pTag->GetInt();
		else if (!pTag->m_name.Compare(TAG_SERVERPORT))
			uBuddyPort = (uint16)pTag->GetInt();
		//        else if (!pTag->m_name.Compare(TAG_CLIENTLOWID))
		//            uClientID = pTag->GetInt();
		else if (!pTag->m_name.Compare(TAG_BUDDYHASH))
		{
			uchar ucharBuddyHash[16];
			if (pTag->IsStr() && strmd4(pTag->GetStr(), ucharBuddyHash))
			md4cpy(uBuddy.GetDataPtr(), ucharBuddyHash);
			else
				TRACE("+++ Invalid TAG_BUDDYHASH tag\n");
		}
		else if (!pTag->m_name.Compare(TAG_ENCRYPTION))
			byCryptOptions = (uint8)pTag->GetInt();

		delete pTag;
	}
	delete plistInfo;

	// Process source based on it's type. Currently only one method is needed to process all types.
	switch( uType )
	{
		case 1:
		case 3:
		case 4:
		case 5:
		case 6:
			m_uAnswers++;
			//  Comment UI
			//theApp.emuledlg->kademliawnd->searchList->SearchRef(this);
			SendMessage( CGlobalVariable::m_hListenWnd,WM_KAD_SEARCHREF,0,(LPARAM)this ); 
			CGlobalVariable::downloadqueue->KademliaSearchFile(m_uSearchID, &uAnswer, &uBuddy, uType, uIP, uTCPPort, uUDPPort, uBuddyIP, uBuddyPort, byCryptOptions);
			break;
	}
}

void CSearch::ProcessResultNotes(const CUInt128 &uAnswer, TagList *plistInfo)
{
	// Process a received Note to a file.
	// Create a Note and set the ID's.
	CEntry* pEntry = new CEntry();
	pEntry->m_uKeyID.SetValue(m_uTarget);
	pEntry->m_uSourceID.SetValue(uAnswer);
	// Create flag to determine if we keep this note.
	bool bFilterComment = false;

	// Loops through tags and pull wanted into. Currently we only keep Filename, Rating, Comment.
	for (TagList::const_iterator itTagList = plistInfo->begin(); itTagList != plistInfo->end(); ++itTagList)
	{
		CKadTag* pTag = *itTagList;
		if (!pTag->m_name.Compare(TAG_SOURCEIP))
		{
			pEntry->m_uIP = (uint32)pTag->GetInt();
			delete pTag;
		}
		else if (!pTag->m_name.Compare(TAG_SOURCEPORT))
		{
			pEntry->m_uTCPPort = (uint16)pTag->GetInt();
			delete pTag;
		}
		else if (!pTag->m_name.Compare(TAG_FILENAME))
		{
			pEntry->SetFileName(pTag->GetStr());
			delete pTag;
		}
		else if (!pTag->m_name.Compare(TAG_DESCRIPTION))
		{
			pEntry->AddTag(pTag);

			// Test if comment sould be filtered
			if (!thePrefs.GetCommentFilter().IsEmpty())
			{
				CString strCommentLower(pTag->GetStr());
				strCommentLower.MakeLower();

				int iPos = 0;
				CString strFilter(thePrefs.GetCommentFilter().Tokenize(_T("|"), iPos));
				while (!strFilter.IsEmpty())
				{
					// comment filters are already in lowercase, compare with temp. lowercased received comment
					if (strCommentLower.Find(strFilter) >= 0)
					{
						bFilterComment = true;
						break;
					}
					strFilter = thePrefs.GetCommentFilter().Tokenize(_T("|"), iPos);
				}
			}
		}
		else if (!pTag->m_name.Compare(TAG_FILERATING))
			pEntry->AddTag(pTag);
		else
			delete pTag;
	}
	delete plistInfo;

	// If we think this should be filtered, delete the note.
	if(bFilterComment)
	{
		delete pEntry;
		return;
	}

	uchar ucharFileid[16];
	m_uTarget.ToByteArray(ucharFileid);

	// Add notes to any searches we have done.
	// The returned entry object will never be attached
	// to anything. So you can delete the entry object
	// at any time after this call..
	bool bFlag = false;

	//  Comment UI
	bFlag=CGlobalVariable::searchlist->AddNotes(pEntry, ucharFileid);

	// Check if this hash is in our shared files..
	CAbstractFile* pFile = (CAbstractFile*)CGlobalVariable::sharedfiles->GetFileByID(ucharFileid);

	// If we didn't find a file in the shares check if it's in our download queue.
	if(!pFile)
		pFile = (CAbstractFile*)CGlobalVariable::downloadqueue->GetFileByID(ucharFileid);

	// If we found a file try to add the Note to the file.
	if( pFile && pFile->AddNote(pEntry) )
	{
		// Inc the number of answers.
		m_uAnswers++;
		// Update the search in the GUI

		//  Comment UI
		//theApp.emuledlg->kademliawnd->searchList->SearchRef(this);
		SendMessage( CGlobalVariable::m_hListenWnd,WM_KAD_SEARCHREF,0,(LPARAM)this ); 
		// We do note delete the NOTE in this case.
		return;
	}

	// It is possible that pFile->AddNote can fail even if we found a File.
	if (bFlag)
	{
		// Inc the number of answers.
		m_uAnswers++;
		// Update the search in the GUI

		//  Comment UI
		//theApp.emuledlg->kademliawnd->searchList->SearchRef(this);
		SendMessage( CGlobalVariable::m_hListenWnd,WM_KAD_SEARCHREF,0,(LPARAM)this ); 
	}

	// We always delete the entry object if pFile->AddNote fails..
	delete pEntry;
}

void CSearch::ProcessResultKeyword(const CUInt128 &uAnswer, TagList *plistInfo)
{
	// Process a keyword that we received.
	// Set of data we can use for a keyword result
	CString sName;
	uint64 uSize = 0;
	CString sType;
	CString sFormat;
	CString sArtist;
	CString sAlbum;
	CString sTitle;
	uint32 uLength = 0;
	CString sCodec;
	uint32 uBitrate = 0;
	uint32 uAvailability = 0;
	uint32 uPublishInfo = 0;
	// Flag that is set if we want this keyword.
	bool bFileName = false;
	bool bFileSize = false;

	for (TagList::const_iterator itTagList = plistInfo->begin(); itTagList != plistInfo->end(); ++itTagList)
	{
		CKadTag* pTag = *itTagList;

		if (!pTag->m_name.Compare(TAG_FILENAME))
		{
			// Set flag based on last tag we saw.
			sName = pTag->GetStr();
			if( sName != _T("") )
				bFileName = true;
			else
				bFileName = false;
		}
		else if (!pTag->m_name.Compare(TAG_FILESIZE))
		{
			if(pTag->IsBsob() && pTag->GetBsobSize() == 8)
				uSize = *((uint64*)pTag->GetBsob());
			else
				uSize = pTag->GetInt();

			// Set flag based on last tag we saw.
			if(uSize)
				bFileSize = true;
			else
				bFileSize = false;
		}
		else if (!pTag->m_name.Compare(TAG_FILETYPE))
			sType = pTag->GetStr();
		else if (!pTag->m_name.Compare(TAG_FILEFORMAT))
			sFormat = pTag->GetStr();
		else if (!pTag->m_name.Compare(TAG_MEDIA_ARTIST))
			sArtist = pTag->GetStr();
		else if (!pTag->m_name.Compare(TAG_MEDIA_ALBUM))
			sAlbum = pTag->GetStr();
		else if (!pTag->m_name.Compare(TAG_MEDIA_TITLE))
			sTitle = pTag->GetStr();
		else if (!pTag->m_name.Compare(TAG_MEDIA_LENGTH))
			uLength = (uint32)pTag->GetInt();
		else if (!pTag->m_name.Compare(TAG_MEDIA_BITRATE))
			uBitrate = (uint32)pTag->GetInt();
		else if (!pTag->m_name.Compare(TAG_MEDIA_CODEC))
			sCodec = pTag->GetStr();
		else if (!pTag->m_name.Compare(TAG_SOURCES))
		{
			// Some rouge client was setting a invalid availability, just set it to 0
			uAvailability = (uint32)pTag->GetInt();
			if( uAvailability > 65500 )
				uAvailability = 0;
		}
		else if (!pTag->m_name.Compare(TAG_PUBLISHINFO))
		{
			// we don't keep this as tag, but as a member property of the searchfile, as we only need its informations
			// in the search list and don't want to carry the tag over when downloading the file (and maybe even wrongly publishing it)
			uPublishInfo = (uint32)pTag->GetInt();
#ifdef _DEBUG
			uint32 byDifferentNames = (uPublishInfo & 0xFF000000) >> 24;
			uint32 byPublishersKnown = (uPublishInfo & 0x00FF0000) >> 16;
			uint32 wTrustValue = uPublishInfo & 0x0000FFFF;
			DebugLog(_T("Received PublishInfoTag: %u different names, %u Publishers, %.2f Trustvalue"), byDifferentNames, byPublishersKnown, (float)wTrustValue / 100.0f);  
#endif
		}
		delete pTag;
	}
	delete plistInfo;

	// If we don't have a valid filename or filesize, drop this keyword.
	if( !bFileName || !bFileSize )
		return;

	// Check that this result matches original criteria
	WordList listTestWords;
	CSearchManager::GetWords(sName, &listTestWords);
	CStringW keyword;
	for (WordList::const_iterator itWordListWords = m_listWords.begin(); itWordListWords != m_listWords.end(); ++itWordListWords)
	{
		keyword = *itWordListWords;
		bool bInterested = false;
		for (WordList::const_iterator itWordListTestWords = listTestWords.begin(); itWordListTestWords != listTestWords.end(); ++itWordListTestWords)
		{
			if (!keyword.CompareNoCase(*itWordListTestWords))
			{
				bInterested = true;
				break;
			}
		}
		if (!bInterested)
			return;
	}

	// Inc the number of answers.
	m_uAnswers++;
	// Update the search in the GUI
	
	//  Comment UI
	//theApp.emuledlg->kademliawnd->searchList->SearchRef(this);
	SendMessage( CGlobalVariable::m_hListenWnd,WM_KAD_SEARCHREF,0,(LPARAM)this );

	// Send we keyword to searchlist to be processed.
	// This method is still legacy from the multithreaded Kad, maybe this can be changed for better handling.
	CGlobalVariable::searchlist->KademliaSearchKeyword(m_uSearchID, &uAnswer, sName, uSize, sType, uPublishInfo, 8,
		    2, TAG_FILEFORMAT, (LPCTSTR)sFormat,
		    2, TAG_MEDIA_ARTIST, (LPCTSTR)sArtist,
		    2, TAG_MEDIA_ALBUM, (LPCTSTR)sAlbum,
		    2, TAG_MEDIA_TITLE, (LPCTSTR)sTitle,
		    3, TAG_MEDIA_LENGTH, uLength,
		    3, TAG_MEDIA_BITRATE, uBitrate,
		    2, TAG_MEDIA_CODEC, (LPCTSTR)sCodec,
		    3, TAG_SOURCES, uAvailability);
}

void CSearch::SendFindValue(CContact* pContact)
{
	// Found a Node that we think has contacts closer to our target.
	try
	{
		// Make sure we are not in the process of stopping.
		if(m_bStoping)
			return;
		CSafeMemFile fileIO(33);
		// The number of returned contacts is based on the type of search.
		switch(m_uType)
		{
			case NODE:
			case NODECOMPLETE:
			case NODESPECIAL:
			case NODEFWCHECKUDP:
				fileIO.WriteUInt8(KADEMLIA_FIND_NODE);
				break;
			case FILE:
			case KEYWORD:
			case FINDSOURCE:
			case NOTES:
				fileIO.WriteUInt8(KADEMLIA_FIND_VALUE);
				break;
			case FINDBUDDY:
			case STOREFILE:
			case STOREKEYWORD:
			case STORENOTES:
				fileIO.WriteUInt8(KADEMLIA_STORE);
				break;
			default:
				AddDebugLogLine(false, _T("Invalid search type. (CSearch::SendFindValue)"));
				return;
		}
		// Put the target we want into the packet.
		fileIO.WriteUInt128(&m_uTarget);
		// Add the ID of the contact we are contacting for sanity checks on the other end.
		fileIO.WriteUInt128(&pContact->GetClientID());
		// Inc the number of packets sent.
		m_uKadPacketSent++;
		// Update the search for the GUI.

		//  Comment UI
		//theApp.emuledlg->kademliawnd->searchList->SearchRef(this);
		SendMessage( CGlobalVariable::m_hListenWnd,WM_KAD_SEARCHREF,0,(LPARAM)this );

		if (pContact->GetVersion() >= 2/*47a*/)
		{
			if (pContact->GetVersion() >= 6){ /*48b*/
				CUInt128 uClientID = pContact->GetClientID();
				CKademlia::GetUDPListener()->SendPacket(&fileIO, KADEMLIA2_REQ, pContact->GetIPAddress(), pContact->GetUDPPort(), pContact->GetUDPKey(), &uClientID);
			}
			else {
				CKademlia::GetUDPListener()->SendPacket(&fileIO, KADEMLIA2_REQ, pContact->GetIPAddress(), pContact->GetUDPPort(), 0, NULL);
				ASSERT( CKadUDPKey(0) == pContact->GetUDPKey() );
			}
			if (thePrefs.GetDebugClientKadUDPLevel() > 0)
			{
				switch(m_uType)
				{
					case NODE:
						DebugSend("KADEMLIA2_REQ(NODE)", pContact->GetIPAddress(), pContact->GetUDPPort());
						break;
					case NODECOMPLETE:
						DebugSend("KADEMLIA2_REQ(NODECOMPLETE)", pContact->GetIPAddress(), pContact->GetUDPPort());
						break;
					case NODESPECIAL:
						DebugSend("KADEMLIA2_REQ(NODESPECIAL)", pContact->GetIPAddress(), pContact->GetUDPPort());
						break;
					case NODEFWCHECKUDP:
						DebugSend("KADEMLIA2_REQ(NODEFWCHECKUDP)", pContact->GetIPAddress(), pContact->GetUDPPort());
						break;
					case FILE:
						DebugSend("KADEMLIA2_REQ(FILE)", pContact->GetIPAddress(), pContact->GetUDPPort());
						break;
					case KEYWORD:
						DebugSend("KADEMLIA2_REQ(KEYWORD)", pContact->GetIPAddress(), pContact->GetUDPPort());
						break;
					case STOREFILE:
						DebugSend("KADEMLIA2_REQ(STOREFILE)", pContact->GetIPAddress(), pContact->GetUDPPort());
						break;
					case STOREKEYWORD:
						DebugSend("KADEMLIA2_REQ(STOREKEYWORD)", pContact->GetIPAddress(), pContact->GetUDPPort());
						break;
					case STORENOTES:
						DebugSend("KADEMLIA2_REQ(STORENOTES)", pContact->GetIPAddress(), pContact->GetUDPPort());
						break;
					case NOTES:
						DebugSend("KADEMLIA2_REQ(NOTES)", pContact->GetIPAddress(), pContact->GetUDPPort());
						break;
					default:
						DebugSend("KADEMLIA2_REQ()", pContact->GetIPAddress(), pContact->GetUDPPort());
				}
			}
		}
		else
		{
			CKademlia::GetUDPListener()->SendPacket(&fileIO, KADEMLIA_REQ, pContact->GetIPAddress(), pContact->GetUDPPort(), 0, NULL);
			if (thePrefs.GetDebugClientKadUDPLevel() > 0)
			{
				switch(m_uType)
				{
					case NODE:
						DebugSend("KADEMLIA_REQ(NODE)", pContact->GetIPAddress(), pContact->GetUDPPort());
						break;
					case NODECOMPLETE:
						DebugSend("KADEMLIA_REQ(NODECOMPLETE)", pContact->GetIPAddress(), pContact->GetUDPPort());
						break;
					case NODESPECIAL:
						DebugSend("KADEMLIA_REQ(NODECOMPLETE)", pContact->GetIPAddress(), pContact->GetUDPPort());
						break;
					case NODEFWCHECKUDP:
						DebugSend("KADEMLIA_REQ(NODEFWCHECKUDP)", pContact->GetIPAddress(), pContact->GetUDPPort());
						break;
					case FILE:
						DebugSend("KADEMLIA_REQ(FILE)", pContact->GetIPAddress(), pContact->GetUDPPort());
						break;
					case KEYWORD:
						DebugSend("KADEMLIA_REQ(KEYWORD)", pContact->GetIPAddress(), pContact->GetUDPPort());
						break;
					case STOREFILE:
						DebugSend("KADEMLIA_REQ(STOREFILE)", pContact->GetIPAddress(), pContact->GetUDPPort());
						break;
					case STOREKEYWORD:
						DebugSend("KADEMLIA_REQ(STOREKEYWORD)", pContact->GetIPAddress(), pContact->GetUDPPort());
						break;
					case STORENOTES:
						DebugSend("KADEMLIA_REQ(STORENOTES)", pContact->GetIPAddress(), pContact->GetUDPPort());
						break;
					case NOTES:
						DebugSend("KADEMLIA_REQ(NOTES)", pContact->GetIPAddress(), pContact->GetUDPPort());
						break;
					default:
						DebugSend("KADEMLIA_REQ()", pContact->GetIPAddress(), pContact->GetUDPPort());
				}
			}
		}
	}
	catch ( CIOException *ioe )
	{
		AddDebugLogLine( false, _T("Exception in CSearch::SendFindValue (IO error(%i))"), ioe->m_iCause);
		ioe->Delete();
	}
	catch (...)
	{
		AddDebugLogLine(false, _T("Exception in CSearch::SendFindValue"));
	}
}

void CSearch::AddFileID(const CUInt128& uID)
{
	// Add a file hash to the search list.
	// This is used mainly for storing keywords, but was also reused for storing notes.
	m_listFileIDs.push_back(uID);
}

static int GetMetaDataWords(CStringArray& rastrWords, const CString& rstrData)
{
	// Create a list of the 'words' found in 'data'. This is similar but though not equal
	// to the 'CSearchManager::GetWords' function which needs to follow some other rules.
	int iPos = 0;
	CString strWord = rstrData.Tokenize(_aszInvKadKeywordChars, iPos);
	while (!strWord.IsEmpty())
	{
		rastrWords.Add(strWord);
		strWord = rstrData.Tokenize(_aszInvKadKeywordChars, iPos);
	}
	return rastrWords.GetSize();
}

static bool IsRedundantMetaData(const CStringArray& rastrFileNameWords, const CString& rstrMetaData)
{
	// Verify if the meta data string 'rstrMetaData' is already contained within the filename.
	if (rstrMetaData.IsEmpty())
		return true;

	int iMetaDataWords = 0;
	int iFoundInFileName = 0;
	int iPos = 0;
	CString strMetaDataWord(rstrMetaData.Tokenize(_aszInvKadKeywordChars, iPos));
	while (!strMetaDataWord.IsEmpty())
	{
		iMetaDataWords++;
		for (int i = 0; i < rastrFileNameWords.GetSize(); i++)
		{
			if (rastrFileNameWords.GetAt(i).CompareNoCase(strMetaDataWord) == 0)
			{
				iFoundInFileName++;
				break;
			}
		}
		if (iFoundInFileName < iMetaDataWords)
			return false;
		strMetaDataWord = rstrMetaData.Tokenize(_aszInvKadKeywordChars, iPos);
	}

	if (iMetaDataWords == 0)
		return true;
	if (iFoundInFileName == iMetaDataWords)
		return true;
	return false;
}

void CSearch::PreparePacketForTags( CByteIO *byIO, CKnownFile *pFile)
{
	// We are going to publish a keyword, setup the tag list.
	TagList listTag;
	try
	{
		if( pFile && byIO )
		{
			// Name, Size
			listTag.push_back(new CKadTagStr(TAG_FILENAME, pFile->GetFileName()));
			if(pFile->GetFileSize() > OLD_MAX_EMULE_FILE_SIZE)
			{
				// TODO: As soon as we drop Kad1 support, we should switch to Int64 tags (we could do now already for kad2 nodes only but no advantage in that)
				byte byValue[8];
				*((uint64*)byValue) = pFile->GetFileSize();
				listTag.push_back(new CKadTagBsob(TAG_FILESIZE, byValue, sizeof(byValue)));
			}
			else
				listTag.push_back(new CKadTagUInt(TAG_FILESIZE, pFile->GetFileSize()));

			listTag.push_back(new CKadTagUInt(TAG_SOURCES, pFile->m_nCompleteSourcesCount));

			// eD2K file type (Audio, Video, ...)
			// NOTE: Archives and CD-Images are published with file type "Pro"
			CString strED2KFileType(GetED2KFileTypeSearchTerm(GetED2KFileTypeID(pFile->GetFileName())));
			if (!strED2KFileType.IsEmpty())
				listTag.push_back(new CKadTagStr(TAG_FILETYPE, strED2KFileType));

			// file format (filename extension)
			// 21-Sep-2006 []: TAG_FILEFORMAT is no longer explicitly published nor stored as
			// it is already part of the filename.
			//int iExt = pFile->GetFileName().ReverseFind(_T('.'));
			//if (iExt != -1)
			//{
			//	CString strExt(pFile->GetFileName().Mid(iExt));
			//	if (!strExt.IsEmpty())
			//	{
			//		strExt = strExt.Mid(1);
			//		if (!strExt.IsEmpty())
			//			listTag.push_back(new CKadTagStr(TAG_FILEFORMAT, strExt));
			//	}
			//}

			// additional meta data (Artist, Album, Codec, Length, ...)
			// only send verified meta data to nodes
			if (pFile->GetMetaDataVer() > 0)
			{
				static const struct
				{
					uint8 uName;
					uint8 uType;
				}
				_aMetaTags[] =
				    {
				        { FT_MEDIA_ARTIST,  2 },
				        { FT_MEDIA_ALBUM,   2 },
				        { FT_MEDIA_TITLE,   2 },
				        { FT_MEDIA_LENGTH,  3 },
				        { FT_MEDIA_BITRATE, 3 },
				        { FT_MEDIA_CODEC,   2 }
				    };
				CStringArray astrFileNameWords;
				for (int iIndex = 0; iIndex < ARRSIZE(_aMetaTags); iIndex++)
				{
					const ::CTag* pTag = pFile->GetTag(_aMetaTags[iIndex].uName, _aMetaTags[iIndex].uType);
					if (pTag)
					{
						// skip string tags with empty string values
						if (pTag->IsStr() && pTag->GetStr().IsEmpty())
							continue;
						// skip integer tags with '0' values
						if (pTag->IsInt() && pTag->GetInt() == 0)
							continue;
						char szKadTagName[2];
						szKadTagName[0] = (char)pTag->GetNameID();
						szKadTagName[1] = '\0';
						if (pTag->IsStr())
						{
							bool bIsRedundant = false;
							if (   pTag->GetNameID() == FT_MEDIA_ARTIST
								|| pTag->GetNameID() == FT_MEDIA_ALBUM
								|| pTag->GetNameID() == FT_MEDIA_TITLE)
							{
								if (astrFileNameWords.GetSize() == 0)
									GetMetaDataWords(astrFileNameWords, pFile->GetFileName());
								bIsRedundant = IsRedundantMetaData(astrFileNameWords, pTag->GetStr());
								//if (bIsRedundant)
								//	TRACE(_T("Skipping meta data tag \"%s\" for file \"%s\"\n"), pTag->GetStr(), pFile->GetFileName());
							}
							if (!bIsRedundant)
							listTag.push_back(new CKadTagStr(szKadTagName, pTag->GetStr()));
						}
						else
							listTag.push_back(new CKadTagUInt(szKadTagName, pTag->GetInt()));
					}
				}
			}
			byIO->WriteTagList(listTag);
		}
		else
		{
			//If we get here.. Bad things happen.. Will fix this later if it is a real issue.
			ASSERT(0);
		}
	}
	catch ( CIOException *ioe )
	{
		AddDebugLogLine( false, _T("Exception in CSearch::PreparePacketForTags (IO error(%i))"), ioe->m_iCause);
		ioe->Delete();
	}
	catch (...)
	{
		AddDebugLogLine(false, _T("Exception in CSearch::PreparePacketForTags"));
	}
	for (TagList::const_iterator itTagList = listTag.begin(); itTagList != listTag.end(); ++itTagList)
		delete *itTagList;
}

uint32 CSearch::GetNodeLoad() const
{
	// Node load is the average of all node load responses.
	if( m_uTotalLoadResponses == 0 )
	{
		return 0;
	}
	return m_uTotalLoad/m_uTotalLoadResponses;
}

uint32 CSearch::GetSearchID() const
{
	return m_uSearchID;
}
uint32 CSearch::GetSearchTypes() const
{
	return m_uType;
}
void CSearch::SetSearchTypes( uint32 uVal )
{
	m_uType = uVal;
}
void CSearch::SetTargetID( CUInt128 uVal )
{
	m_uTarget = uVal;
}
uint32 CSearch::GetAnswers() const
{
	if(m_listFileIDs.size() == 0)
		return m_uAnswers;
	// If we sent more then one packet per node, we have to average the answers for the real count.
	return m_uAnswers/((m_listFileIDs.size()+49)/50);
}
uint32 CSearch::GetKadPacketSent() const
{
	return m_uKadPacketSent;
}
uint32 CSearch::GetRequestAnswer() const
{
	return m_uTotalRequestAnswers;
}

const CString& CSearch::GetFileName() const
{
	return m_sFileName;
}
void CSearch::SetFileName(const CString& sFileName)
{
	m_sFileName = sFileName;
}
CUInt128 CSearch::GetTarget() const
{
	return m_uTarget;
}
bool CSearch::Stoping() const
{
	return m_bStoping;
}
uint32 CSearch::GetNodeLoadResonse() const
{
	return m_uTotalLoadResponses;
}
uint32 CSearch::GetNodeLoadTotal() const
{
	return m_uTotalLoad;
}
void CSearch::UpdateNodeLoad( uint8 uLoad )
{
	// Since all nodes do not return a load value, keep track of total responses and total load.
	m_uTotalLoad += uLoad;
	m_uTotalLoadResponses++;
}

void CSearch::SetSearchTermData( uint32 uSearchTermDataSize, LPBYTE pucSearchTermsData )
{
	m_uSearchTermsDataSize = uSearchTermDataSize;
	m_pucSearchTermsData = new BYTE[uSearchTermDataSize];
	memcpy(m_pucSearchTermsData, pucSearchTermsData, uSearchTermDataSize);
}
