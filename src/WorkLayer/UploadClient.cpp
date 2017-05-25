/* 
 * $Id: UploadClient.cpp 9291 2008-12-24 09:19:05Z dgkang $
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
//#include "emule.h"
#include <zlib/zlib.h>
#include "UpDownClient.h"
#include "HttpClient.h"
#include "Opcodes.h"
#include "Packets.h"
#include "UploadQueue.h"
#include "Statistics.h"
#include "ClientList.h"
#include "ClientUDPSocket.h"
#include "SharedFileList.h"
#include "KnownFileList.h"
#include "PartFile.h"
#include "ClientCredits.h"
#include "ListenSocket.h"
#include "PeerCacheSocket.h"
#include "Sockets.h"
#include "OtherFunctions.h"
#include "SafeFile.h"
#include "DownloadQueue.h"
#include "emuledlg.h"
#include "TransferWnd.h"
#include "Log.h"
#include "Collection.h"
#include "SHAHashSet.h"
#include "GlobalVariable.h"
#include "UIMessage.h"
#include "resource.h"
#include "StatForServer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//	members of CUpDownClient
//	which are mainly used for uploading functions

CBarShader CUpDownClient::s_UpStatusBar(16);

void CUpDownClient::DrawUpStatusBar(CDC* dc, RECT* rect, bool onlygreyrect, bool  bFlat) const
{
    COLORREF crNeither;
    COLORREF crNextSending;
    COLORREF crBoth;
    COLORREF crSending;

    if (GetSlotNumber() <= CGlobalVariable::uploadqueue->GetActiveUploadsCount() ||
            (GetUploadState() != US_UPLOADING && GetUploadState() != US_CONNECTING) )
    {
        crNeither = RGB(224, 224, 224);
        crNextSending = RGB(255,208,0);
        crBoth = bFlat ? RGB(0, 0, 0) : RGB(104, 104, 104);
        crSending = RGB(0, 150, 0);
    }
    else
    {
        // grayed out
        crNeither = RGB(248, 248, 248);
        crNextSending = RGB(255,244,191);
        crBoth = bFlat ? RGB(191, 191, 191) : RGB(191, 191, 191);
        crSending = RGB(191, 229, 191);
    }

    //  Comment UI

    // wistily: UpStatusFix
    CKnownFile* currequpfile = CGlobalVariable::sharedfiles->GetFileByID(requpfileid);
    EMFileSize filesize;
    if (currequpfile)
        filesize = currequpfile->GetFileSize();
    else
        filesize = (uint64)(PARTSIZE * (uint64)m_nUpPartCount);
    // wistily: UpStatusFix

    if (filesize > (uint64)0)
    {
        s_UpStatusBar.SetFileSize(filesize);
        s_UpStatusBar.SetHeight(rect->bottom - rect->top);
        s_UpStatusBar.SetWidth(rect->right - rect->left);
        s_UpStatusBar.Fill(crNeither);
        if (!onlygreyrect && m_abyUpPartStatus)
        {
            for (UINT i = 0; i < m_nUpPartCount; i++)
                if (m_abyUpPartStatus[i])
                    s_UpStatusBar.FillRange(PARTSIZE*(uint64)(i), PARTSIZE*(uint64)(i+1), crBoth);
        }
        const Requested_Block_Struct* block;
        if (!m_BlockRequests_queue.IsEmpty())
        {
            block = m_BlockRequests_queue.GetHead();
            if (block)
            {
                uint32 start = (uint32)(block->StartOffset/PARTSIZE);
                s_UpStatusBar.FillRange((uint64)start*PARTSIZE, (uint64)(start+1)*PARTSIZE, crNextSending);
            }
        }
        if (!m_DoneBlocks_list.IsEmpty())
        {
            block = m_DoneBlocks_list.GetHead();
            if (block)
            {
                uint32 start = (uint32)(block->StartOffset/PARTSIZE);
                s_UpStatusBar.FillRange((uint64)start*PARTSIZE, (uint64)(start+1)*PARTSIZE, crNextSending);
            }
        }
        if (!m_DoneBlocks_list.IsEmpty())
        {
            for (POSITION pos=m_DoneBlocks_list.GetHeadPosition();pos!=0;)
            {
                block = m_DoneBlocks_list.GetNext(pos);
                s_UpStatusBar.FillRange(block->StartOffset, block->EndOffset + 1, crSending);
            }
        }
        s_UpStatusBar.Draw(dc, rect->left, rect->top, bFlat);
    }
}

void CUpDownClient::SetUploadState(EUploadState eNewState)
{
    if (eNewState != m_nUploadState)
    {
        if (m_nUploadState == US_UPLOADING)
        {
            // Reset upload data rate computation
            m_nUpDatarate = 0;
            m_nSumForAvgUpDataRate = 0;
            m_AvarageUDR_list.RemoveAll();
        }
        if (eNewState == US_UPLOADING)
            m_fSentOutOfPartReqs = 0;

        // don't add any final cleanups for US_NONE here
        m_nUploadState = (_EUploadState)eNewState;
        //  Comment UI
        //SendMessage(CGlobalVariable::m_hListenWnd,WM_FILE_UPDATE_PEER,0,(LPARAM)this);
        //theApp.emuledlg->transferwnd->clientlistctrl.RefreshClient(this);
        UpdateUI(UI_UPDATE_PEERLIST);
    }
}

/**
 * Gets the queue score multiplier for this client, taking into consideration client's credits
 * and the requested file's priority.
 */
float CUpDownClient::GetCombinedFilePrioAndCredit()
{
    if (credits == 0)
    {
        ASSERT ( IsKindOf(RUNTIME_CLASS(CHttpClient)) );
        return 0.0F;
    }

    return 10.0f * credits->GetScoreRatio(GetIP()) * (float)GetFilePrioAsNumber();
}

/**
 * Gets the file multiplier for the file this client has requested.
 */
int CUpDownClient::GetFilePrioAsNumber() const
{

    //  Comment UI
    CKnownFile* currequpfile = NULL;
    //  Comment UI
    currequpfile = CGlobalVariable::sharedfiles->GetFileByID(requpfileid);
    if (!currequpfile)
        return 0;

    // TODO coded by tecxx & herbert, one yet unsolved problem here:
    // sometimes a client asks for 2 files and there is no way to decide, which file the
    // client finally gets. so it could happen that he is queued first because of a
    // high prio file, but then asks for something completely different.
    int filepriority = 10; // standard
    switch (currequpfile->GetUpPriority())
    {
    case PR_VERYHIGH:
        filepriority = 18;
        break;
    case PR_HIGH:
        filepriority = 9;
        break;
    case PR_LOW:
        filepriority = 6;
        break;
    case PR_VERYLOW:
        filepriority = 2;
        break;
    case PR_NORMAL:
    default:
        filepriority = 7;
        break;
    }

    return filepriority;
}

class CSyncHelper
{
public:
    CSyncHelper()
    {
        m_pObject = NULL;
    }
    ~CSyncHelper()
    {
        if (m_pObject)
            m_pObject->Unlock();
    }
    CSyncObject* m_pObject;
};

void CUpDownClient::CreateNextBlockPackage()
{
    // See if we can do an early return. There may be no new blocks to load from disk and add to buffer, or buffer may be large enough allready.
    if (m_BlockRequests_queue.IsEmpty() || // There are no new blocks requested
            m_addedPayloadQueueSession > GetQueueSessionPayloadUp() && m_addedPayloadQueueSession-GetQueueSessionPayloadUp() > 50*1024)
    { 
		// the buffered data is large enough allready
        return;
    }

    CFile file;
    byte* filedata = 0;
    CString fullname;
    bool bFromPF = true; // Statistic to breakdown uploaded data by complete file vs. partfile.
    CSyncHelper lockFile;
    
	try
    {
        // Buffer new data if current buffer is less than 100 KBytes
        while (!m_BlockRequests_queue.IsEmpty() &&
                (m_addedPayloadQueueSession <= GetQueueSessionPayloadUp() || m_addedPayloadQueueSession-GetQueueSessionPayloadUp() < 100*1024))
        {

            Requested_Block_Struct* currentblock = m_BlockRequests_queue.GetHead();
           
			CKnownFile* srcfile = CGlobalVariable::sharedfiles->GetFileByID(currentblock->FileID);
            
            if (!srcfile)
			{
				throw GetResString(IDS_ERR_REQ_FNF);
			}

            if (srcfile->IsPartFile() && ((CPartFile*)srcfile)->GetStatus() != PS_COMPLETE)
            {
                // Do not access a part file, if it is currently moved into the incoming directory.
                // Because the moving of part file into the incoming directory may take a noticable
                // amount of time, we can not wait for 'm_FileCompleteMutex' and block the main thread.
                if (!((CPartFile*)srcfile)->m_FileCompleteMutex.Lock(0))
                { 
					// just do a quick test of the mutex's state and return if it's locked.
                    return;
                }
                lockFile.m_pObject = &((CPartFile*)srcfile)->m_FileCompleteMutex;
                // If it's a part file which we are uploading the file remains locked until we've read the
                // current block. This way the file completion thread can not (try to) "move" the file into
                // the incoming directory.

                fullname = RemoveFileExtension(((CPartFile*)srcfile)->GetFullName());
            }
            else
            {
                fullname.Format(_T("%s\\%s"),srcfile->GetPath(),srcfile->GetFileName());
            }

            uint64 i64uTogo;
            if (currentblock->StartOffset > currentblock->EndOffset)
            {
                i64uTogo = currentblock->EndOffset + (srcfile->GetFileSize() - currentblock->StartOffset);
            }
            else
            {
                i64uTogo = currentblock->EndOffset - currentblock->StartOffset;
               
#ifndef _SUPPORT_MEMPOOL
                if (srcfile->IsPartFile() && !((CPartFile*)srcfile)->IsComplete(currentblock->StartOffset,currentblock->EndOffset-1, true))
                    throw GetResString(IDS_ERR_INCOMPLETEBLOCK);
#else
                if (srcfile->IsPartFile() && !((CPartFile*)srcfile)->IsComplete(currentblock->StartOffset,currentblock->EndOffset-1, false))
                    throw GetResString(IDS_ERR_INCOMPLETEBLOCK);
#endif	       
            }

            //  Comment UI
            if ( i64uTogo > EMBLOCKSIZE*3 )
			{
				throw GetResString(IDS_ERR_LARGEREQBLOCK);
			}

            uint32 togo = (uint32)i64uTogo;

            if (!srcfile->IsPartFile())
            {
                bFromPF = false; // This is not a part file...

                if (!file.Open(fullname,CFile::modeRead|CFile::osSequentialScan|CFile::shareDenyNone))
                    throw GetResString(IDS_ERR_OPEN);
                file.Seek(currentblock->StartOffset,0);

                filedata = new byte[togo+500];
                if (uint32 done = file.Read(filedata,togo) != togo)
                {
                    file.SeekToBegin();
                    file.Read(filedata + done,togo-done);
                }
                file.Close();
            }
            else
            {
                CPartFile* partfile = (CPartFile*)srcfile;

                filedata = new byte[togo+500];

#ifndef _SUPPORT_MEMPOOL
                partfile->m_hpartfile.Seek(currentblock->StartOffset,0);

                if (uint32 done = partfile->m_hpartfile.Read(filedata,togo) != togo)
                {
                    partfile->m_hpartfile.SeekToBegin();
                    partfile->m_hpartfile.Read(filedata + done,togo-done);
                }
#else
                if (!partfile->GetDataFromBufferThenDisk(filedata, currentblock->StartOffset, 
					currentblock->StartOffset + togo - 1))
                {
                    throw CString(_T("can't get intact data."));
                }
#endif	
            }

            if (lockFile.m_pObject)
            {
                lockFile.m_pObject->Unlock(); // Unlock the (part) file as soon as we are done with accessing it.
                lockFile.m_pObject = NULL;
            }

            SetUploadFileID(srcfile);

            // check extension to decide whether to compress or not
            CString ext = srcfile->GetFileName();
            ext.MakeLower();
            int pos = ext.ReverseFind(_T('.'));
            if (pos>-1)
                ext = ext.Mid(pos);
            bool compFlag = (ext!=_T(".zip") && ext!=_T(".cbz") && ext!=_T(".rar") && ext!=_T(".cbr") && ext!=_T(".ace") && ext!=_T(".ogm"));
            if (ext==_T(".avi") && thePrefs.GetDontCompressAvi())
                compFlag=false;

            if (!IsUploadingToPeerCache() && m_byDataCompVer == 1 && compFlag)
                CreatePackedPackets(filedata,togo,currentblock,bFromPF);
            else
                CreateStandartPackets(filedata,togo,currentblock,bFromPF);

            // file statistic
            srcfile->statistic.AddTransferred(togo);

            m_addedPayloadQueueSession += togo;

            m_DoneBlocks_list.AddHead(m_BlockRequests_queue.RemoveHead());
            delete[] filedata;
            filedata = 0;
        }
    }
    catch (CString error)
    {
        if (thePrefs.GetVerbose())
            DebugLogWarning(GetResString(IDS_ERR_CLIENTERRORED), GetUserName(), error);
        CGlobalVariable::uploadqueue->RemoveFromUploadQueue(this, _T("Client error: ") + error);
		
		if (filedata) // VC-SearchDream[2007-07-04]: Add Condition
		{
			delete[] filedata;
		}
		
        return;
    }
    catch (CFileException* e)
    {
        TCHAR szError[MAX_CFEXP_ERRORMSG];
        e->GetErrorMessage(szError, ARRSIZE(szError));
        if (thePrefs.GetVerbose())
            DebugLogWarning(_T("Failed to create upload package for %s - %s"), GetUserName(), szError);
        CGlobalVariable::uploadqueue->RemoveFromUploadQueue(this, ((CString)_T("Failed to create upload package.")) + szError);
        
		if (filedata) // VC-SearchDream[2007-07-04]: Add Condition
		{
			delete[] filedata;
		}

        e->Delete();
        return;
    }
}

bool CUpDownClient::ProcessExtendedInfo(CSafeMemFile* data, CKnownFile* tempreqfile)
{
    delete[] m_abyUpPartStatus;
    m_abyUpPartStatus = NULL;
    m_nUpPartCount = 0;
    m_nUpCompleteSourcesCount= 0;
    if (GetExtendedRequestsVersion() == 0)
        return true;

    uint16 nED2KUpPartCount = data->ReadUInt16();
    if (!nED2KUpPartCount)
    {
        m_nUpPartCount = tempreqfile->GetPartCount();
        m_abyUpPartStatus = new uint8[m_nUpPartCount];
        memset(m_abyUpPartStatus, 0, m_nUpPartCount);
    }
    else
    {
        if (tempreqfile->GetED2KPartCount() != nED2KUpPartCount)
        {
            //We already checked if we are talking about the same file.. So if we get here, something really strange happened!
            m_nUpPartCount = 0;
            return false;
        }
        m_nUpPartCount = tempreqfile->GetPartCount();
        m_abyUpPartStatus = new uint8[m_nUpPartCount];
        uint16 done = 0;
        while (done != m_nUpPartCount)
        {
            uint8 toread = data->ReadUInt8();
            for (UINT i = 0; i != 8; i++)
            {
                m_abyUpPartStatus[done] = ((toread >> i) & 1) ? 1 : 0;
//				We may want to use this for another feature..
//				if (m_abyUpPartStatus[done] && !tempreqfile->IsComplete((uint64)done*PARTSIZE,((uint64)(done+1)*PARTSIZE)-1))
//					bPartsNeeded = true;
                done++;
                if (done == m_nUpPartCount)
                    break;
            }
        }
    }
    if (GetExtendedRequestsVersion() > 1)
    {
        uint16 nCompleteCountLast = GetUpCompleteSourcesCount();
        uint16 nCompleteCountNew = data->ReadUInt16();
        SetUpCompleteSourcesCount(nCompleteCountNew);
        if (nCompleteCountLast != nCompleteCountNew)
            tempreqfile->UpdatePartsInfo();
    }
    //  Comment UI
    //SendMessage(CGlobalVariable::m_hListenWnd,WM_FILE_UPDATE_PEER,0,(LPARAM)this);
    UpdateUI(UI_UPDATE_QUEUELIST);
    //theApp.emuledlg->transferwnd->queuelistctrl.RefreshClient(this);
    return true;
}

void CUpDownClient::CreateStandartPackets(byte* data,uint32 togo, Requested_Block_Struct* currentblock, bool bFromPF)
{
    uint32 nPacketSize;
    CMemFile memfile((BYTE*)data,togo);
    if (togo > 10240)
        nPacketSize = togo/(uint32)(togo/10240);
    else
        nPacketSize = togo;
    while (togo)
    {
        if (togo < nPacketSize*2)
            nPacketSize = togo;
        ASSERT( nPacketSize );
        togo -= nPacketSize;

        uint64 statpos = (currentblock->EndOffset - togo) - nPacketSize;
        uint64 endpos = (currentblock->EndOffset - togo);
        if (IsUploadingToPeerCache())
        {
            if (m_pPCUpSocket == NULL)
            {
                ASSERT(0);
                CString strError;
                strError.Format(_T("Failed to upload to PeerCache - missing socket; %s"), DbgGetClientInfo());
                throw strError;
            }
            USES_CONVERSION;
            CSafeMemFile dataHttp(10240);
            if (m_iHttpSendState == 0)
            {
                CKnownFile* srcfile = CGlobalVariable::sharedfiles->GetFileByID(GetUploadFileID());
                CStringA str;
                str.AppendFormat("HTTP/1.0 206\r\n");
                str.AppendFormat("Content-Range: bytes %I64u-%I64u/%I64u\r\n", currentblock->StartOffset, currentblock->EndOffset - 1, srcfile->GetFileSize());
                str.AppendFormat("Content-Type: application/octet-stream\r\n");
                str.AppendFormat("Content-Length: %u\r\n", (uint32)(currentblock->EndOffset - currentblock->StartOffset));

                str.AppendFormat("Server: eMule/%s\r\n", T2CA(CGlobalVariable::GetCurVersionLong()));
                str.AppendFormat("\r\n");
                dataHttp.Write((LPCSTR)str, str.GetLength());
                theStats.AddUpDataOverheadFileRequest((UINT)dataHttp.GetLength());

                m_iHttpSendState = 1;
                if (thePrefs.GetDebugClientTCPLevel() > 0)
                {
                    DebugSend("PeerCache-HTTP", this, GetUploadFileID());
                    Debug(_T("  %hs\n"), str);
                }
            }
            dataHttp.Write(data, nPacketSize);
            data += nPacketSize;

            if (thePrefs.GetDebugClientTCPLevel() > 1)
            {
                DebugSend("PeerCache-HTTP data", this, GetUploadFileID());
                Debug(_T("  Start=%I64u  End=%I64u  Size=%u\n"), statpos, endpos, nPacketSize);
            }

            UINT uRawPacketSize = (UINT)dataHttp.GetLength();
            LPBYTE pRawPacketData = dataHttp.Detach();
            CRawPacket* packet = new CRawPacket((char*)pRawPacketData, uRawPacketSize, bFromPF);
            m_pPCUpSocket->SendPacket(packet, true, false, nPacketSize);
            free(pRawPacketData);
        }
        else
        {
            Packet* packet;
            if (statpos > 0xFFFFFFFF || endpos > 0xFFFFFFFF)
            {
                packet = new Packet(OP_SENDINGPART_I64,nPacketSize+32, OP_EMULEPROT, bFromPF);
                md4cpy(&packet->pBuffer[0],GetUploadFileID());
                PokeUInt64(&packet->pBuffer[16], statpos);
                PokeUInt64(&packet->pBuffer[24], endpos);
                memfile.Read(&packet->pBuffer[32],nPacketSize);
                theStats.AddUpDataOverheadFileRequest(32);
            }
            else
            {
                packet = new Packet(OP_SENDINGPART,nPacketSize+24, OP_EDONKEYPROT, bFromPF);
                md4cpy(&packet->pBuffer[0],GetUploadFileID());
                PokeUInt32(&packet->pBuffer[16], (uint32)statpos);
                PokeUInt32(&packet->pBuffer[20], (uint32)endpos);
                memfile.Read(&packet->pBuffer[24],nPacketSize);
                theStats.AddUpDataOverheadFileRequest(24);
            }

            if (thePrefs.GetDebugClientTCPLevel() > 0)
            {
                DebugSend("OP__SendingPart", this, GetUploadFileID());
                Debug(_T("  Start=%I64u  End=%I64u  Size=%u\n"), statpos, endpos, nPacketSize);
            }
            // put packet directly on socket

            socket->SendPacket(packet,true,false, nPacketSize);
        }
    }
}

void CUpDownClient::CreatePackedPackets(byte* data, uint32 togo, Requested_Block_Struct* currentblock, bool bFromPF)
{
    BYTE* output = new BYTE[togo+300];
    uLongf newsize = togo+300;
    UINT result = compress2(output, &newsize, data, togo, 9);
    if (result != Z_OK || togo <= newsize)
    {
        delete[] output;
        CreateStandartPackets(data,togo,currentblock,bFromPF);
        return;
    }
    CMemFile memfile(output,newsize);
    uint32 oldSize = togo;
    togo = newsize;
    uint32 nPacketSize;
    if (togo > 10240)
        nPacketSize = togo/(uint32)(togo/10240);
    else
        nPacketSize = togo;

    uint32 totalPayloadSize = 0;

    while (togo)
    {
        if (togo < nPacketSize*2)
            nPacketSize = togo;
        ASSERT( nPacketSize );
        togo -= nPacketSize;
        uint64 statpos = currentblock->StartOffset;
        Packet* packet;
        if (currentblock->StartOffset > 0xFFFFFFFF || currentblock->EndOffset > 0xFFFFFFFF)
        {
            packet = new Packet(OP_COMPRESSEDPART_I64,nPacketSize+28,OP_EMULEPROT,bFromPF);
            md4cpy(&packet->pBuffer[0],GetUploadFileID());
            PokeUInt64(&packet->pBuffer[16], statpos);
            PokeUInt32(&packet->pBuffer[24], newsize);
            memfile.Read(&packet->pBuffer[28],nPacketSize);
        }
        else
        {
            packet = new Packet(OP_COMPRESSEDPART,nPacketSize+24,OP_EMULEPROT,bFromPF);
            md4cpy(&packet->pBuffer[0],GetUploadFileID());
            PokeUInt32(&packet->pBuffer[16], (uint32)statpos);
            PokeUInt32(&packet->pBuffer[20], newsize);
            memfile.Read(&packet->pBuffer[24],nPacketSize);
        }

        if (thePrefs.GetDebugClientTCPLevel() > 0)
        {
            DebugSend("OP__CompressedPart", this, GetUploadFileID());
            Debug(_T("  Start=%I64u  BlockSize=%u  Size=%u\n"), statpos, newsize, nPacketSize);
        }
        // approximate payload size
        uint32 payloadSize = nPacketSize*oldSize/newsize;

        if (togo == 0 && totalPayloadSize+payloadSize < oldSize)
        {
            payloadSize = oldSize-totalPayloadSize;
        }
        totalPayloadSize += payloadSize;

        // put packet directly on socket
        theStats.AddUpDataOverheadFileRequest(24);
        socket->SendPacket(packet,true,false, payloadSize);
    }
    delete[] output;
}

void CUpDownClient::SetUploadFileID(CKnownFile* newreqfile)
{
    CKnownFile* oldreqfile;
    //We use the knownfilelist because we may have unshared the file..
    //But we always check the download list first because that person may have decided to redownload that file.
    //Which will replace the object in the knownfilelist if completed.
    if ((oldreqfile = CGlobalVariable::downloadqueue->GetFileByID(requpfileid)) == NULL)
        oldreqfile = CGlobalVariable::knownfiles->FindKnownFileByID(requpfileid);

    if (newreqfile == oldreqfile)
        return;

    // clear old status
    delete[] m_abyUpPartStatus;
    m_abyUpPartStatus = NULL;
    m_nUpPartCount = 0;
    m_nUpCompleteSourcesCount= 0;

    if (newreqfile)
    {
        newreqfile->AddUploadingClient(this);
        md4cpy(requpfileid, newreqfile->GetFileHash());
    }
    else
        md4clr(requpfileid);

    if (oldreqfile)
        oldreqfile->RemoveUploadingClient(this);
}

void CUpDownClient::AddReqBlock(Requested_Block_Struct* reqblock)
{
    if (GetUploadState() != US_UPLOADING)
    {
        if (thePrefs.GetLogUlDlEvents())
            AddDebugLogLine(DLP_LOW, false, _T("UploadClient: Client tried to add req block when not in upload slot! Prevented req blocks from being added. %s"), DbgGetClientInfo());
        delete reqblock;
        return;
    }

    if (HasCollectionUploadSlot())
    {
        CKnownFile* pDownloadingFile = CGlobalVariable::sharedfiles->GetFileByID(reqblock->FileID);
        if (pDownloadingFile != NULL)
        {
            if ( !(CCollection::HasCollectionExtention(pDownloadingFile->GetFileName()) && pDownloadingFile->GetFileSize() < (uint64)MAXPRIORITYCOLL_SIZE) )
            {
                AddDebugLogLine(DLP_HIGH, false, _T("UploadClient: Client tried to add req block for non collection while having a collection slot! Prevented req blocks from being added. %s"), DbgGetClientInfo());
                delete reqblock;
                return;
            }
        }
        else
            ASSERT( false );
    }

    for (POSITION pos = m_DoneBlocks_list.GetHeadPosition(); pos != 0; )
    {
        const Requested_Block_Struct* cur_reqblock = m_DoneBlocks_list.GetNext(pos);
        if (reqblock->StartOffset == cur_reqblock->StartOffset && reqblock->EndOffset == cur_reqblock->EndOffset)
        {
            delete reqblock;
            return;
        }
    }
    for (POSITION pos = m_BlockRequests_queue.GetHeadPosition(); pos != 0; )
    {
        const Requested_Block_Struct* cur_reqblock = m_BlockRequests_queue.GetNext(pos);
        if (reqblock->StartOffset == cur_reqblock->StartOffset && reqblock->EndOffset == cur_reqblock->EndOffset)
        {
            delete reqblock;
            return;
        }
    }

    m_BlockRequests_queue.AddTail(reqblock);
}

uint32 CUpDownClient::SendBlockData()
{
    DWORD curTick = ::GetTickCount();

    uint64 sentBytesCompleteFile = 0;
    uint64 sentBytesPartFile = 0;
    uint64 sentBytesPayload = 0;

    if (GetFileUploadSocket() && (m_ePeerCacheUpState != PCUS_WAIT_CACHE_REPLY))
    {
        CEMSocket* s = GetFileUploadSocket();
        UINT uUpStatsPort;
        if (m_pPCUpSocket && IsUploadingToPeerCache())
        {
            uUpStatsPort = (UINT)-1;

            // Check if filedata has been sent via the normal socket since last call.
            uint64 sentBytesCompleteFileNormalSocket = socket->GetSentBytesCompleteFileSinceLastCallAndReset();
            uint64 sentBytesPartFileNormalSocket = socket->GetSentBytesPartFileSinceLastCallAndReset();

            if (thePrefs.GetVerbose() && (sentBytesCompleteFileNormalSocket + sentBytesPartFileNormalSocket > 0))
            {
                AddDebugLogLine(false, _T("Sent file data via normal socket when in PC mode. Bytes: %I64i."), sentBytesCompleteFileNormalSocket + sentBytesPartFileNormalSocket);
            }
        }
        else
            uUpStatsPort = GetUserPort();

        // Extended statistics information based on which client software and which port we sent this data to...
        // This also updates the grand total for sent bytes, etc.  And where this data came from.
        sentBytesCompleteFile = s->GetSentBytesCompleteFileSinceLastCallAndReset();
        sentBytesPartFile = s->GetSentBytesPartFileSinceLastCallAndReset();
        thePrefs.Add2SessionTransferData(GetClientSoft(), uUpStatsPort, false, true, (UINT)sentBytesCompleteFile, (IsFriend() && GetFriendSlot()));
        thePrefs.Add2SessionTransferData(GetClientSoft(), uUpStatsPort, true, true, (UINT)sentBytesPartFile, (IsFriend() && GetFriendSlot()));

        m_nTransferredUp = (UINT)(m_nTransferredUp + sentBytesCompleteFile + sentBytesPartFile);
        if (credits) credits->AddUploaded((UINT)(sentBytesCompleteFile + sentBytesPartFile), GetIP());
		
	

        sentBytesPayload = s->GetSentPayloadSinceLastCallAndReset();
        m_nCurQueueSessionPayloadUp = (UINT)(m_nCurQueueSessionPayloadUp + sentBytesPayload);

        if (CGlobalVariable::uploadqueue->CheckForTimeOver(this))
        {
            CGlobalVariable::uploadqueue->RemoveFromUploadQueue(this, _T("Completed transfer"), true);
            SendOutOfPartReqsAndAddToWaitingQueue();
        }
        else
        {
            // read blocks from file and put on socket
            CreateNextBlockPackage();
        }
    }

    if (sentBytesCompleteFile + sentBytesPartFile > 0 ||
            m_AvarageUDR_list.GetCount() == 0 || (curTick - m_AvarageUDR_list.GetTail().timestamp) > 1*1000)
    {
        // Store how much data we've transferred this round,
        // to be able to calculate average speed later
        // keep sum of all values in list up to date
        TransferredData newitem = {(UINT)(sentBytesCompleteFile + sentBytesPartFile), curTick};
        m_AvarageUDR_list.AddTail(newitem);
        m_nSumForAvgUpDataRate = (UINT)(m_nSumForAvgUpDataRate + sentBytesCompleteFile + sentBytesPartFile);
    }

    // remove to old values in list
    while (m_AvarageUDR_list.GetCount() > 0 && (curTick - m_AvarageUDR_list.GetHead().timestamp) > 10*1000)
    {
        // keep sum of all values in list up to date
        m_nSumForAvgUpDataRate -= m_AvarageUDR_list.RemoveHead().datalen;
    }

    // Calculate average speed for this slot
    if (m_AvarageUDR_list.GetCount() > 0 && (curTick - m_AvarageUDR_list.GetHead().timestamp) > 0 && GetUpStartTimeDelay() > 2*1000)
    {
        m_nUpDatarate = (UINT)(((ULONGLONG)m_nSumForAvgUpDataRate*1000) / (curTick - m_AvarageUDR_list.GetHead().timestamp));
    }
    else
    {
        // not enough values to calculate trustworthy speed. Use -1 to tell this
        m_nUpDatarate = 0; //-1;
    }

    // Check if it's time to update the display.
    if (curTick-m_lastRefreshedULDisplay > MINWAIT_BEFORE_ULDISPLAY_WINDOWUPDATE+(uint32)(rand()*800/RAND_MAX))
    {
        // Update display
        //  Comment UI begin
        //SendMessage(CGlobalVariable::m_hListenWnd,WM_FILE_UPDATE_PEER,0,(LPARAM)this);
        UpdateUI(UI_UPDATE_PEERLIST|UI_UPDATE_UPLOADLIST);
        //theApp.emuledlg->transferwnd->uploadlistctrl.RefreshClient(this);
        //theApp.emuledlg->transferwnd->clientlistctrl.RefreshClient(this);
        //Comment UI end

        m_lastRefreshedULDisplay = curTick;
    }

    return (UINT)(sentBytesCompleteFile + sentBytesPartFile);
}


void CUpDownClient::SendOutOfPartReqsAndAddToWaitingQueue()
{
    //OP_OUTOFPARTREQS will tell the downloading client to go back to OnQueue..
    //The main reason for this is that if we put the client back on queue and it goes
    //back to the upload before the socket times out... We get a situation where the
    //downloader thinks it already sent the requested blocks and the uploader thinks
    //the downloader didn't send any request blocks. Then the connection times out..
    //I did some tests with eDonkey also and it seems to work well with them also..
    if (thePrefs.GetDebugClientTCPLevel() > 0)
        DebugSend("OP__OutOfPartReqs", this);
    Packet* pPacket = new Packet(OP_OUTOFPARTREQS, 0);
    theStats.AddUpDataOverheadFileRequest(pPacket->size);
    socket->SendPacket(pPacket, true, true);
    m_fSentOutOfPartReqs = 1;
    CGlobalVariable::uploadqueue->AddClientToQueue(this, true);
}

/**
 * See description for CEMSocket::TruncateQueues().
 */
void CUpDownClient::FlushSendBlocks()
{ // call this when you stop upload, or the socket might be not able to send
    if (socket)      //socket may be NULL...
        socket->TruncateQueues();
}

void CUpDownClient::SendHashsetPacket(const uchar* forfileid)
{
    CKnownFile* file = CGlobalVariable::sharedfiles->GetFileByID(forfileid);
    if (!file)
    {
        CheckFailedFileIdReqs(forfileid);
        //  Comment UI
        throw GetResString(IDS_ERR_REQ_FNF) + _T(" (SendHashsetPacket)");
    }

    CSafeMemFile data(1024);
    data.WriteHash16(file->GetFileHash());
    UINT parts = file->GetHashCount();
    data.WriteUInt16((uint16)parts);
    for (UINT i = 0; i < parts; i++)
        data.WriteHash16(file->GetPartHash(i));
    if (thePrefs.GetDebugClientTCPLevel() > 0)
        DebugSend("OP__HashSetAnswer", this, forfileid);
    Packet* packet = new Packet(&data);
    packet->opcode = OP_HASHSETANSWER;
    theStats.AddUpDataOverheadFileRequest(packet->size);
    socket->SendPacket(packet,true,true);
}

void CUpDownClient::ClearUploadBlockRequests()
{
    FlushSendBlocks();

    for (POSITION pos = m_BlockRequests_queue.GetHeadPosition();pos != 0;)
        delete m_BlockRequests_queue.GetNext(pos);
    m_BlockRequests_queue.RemoveAll();

    for (POSITION pos = m_DoneBlocks_list.GetHeadPosition();pos != 0;)
        delete m_DoneBlocks_list.GetNext(pos);
    m_DoneBlocks_list.RemoveAll();
}

void CUpDownClient::SendRankingInfo()
{
    if (!ExtProtocolAvailable())
        return;
    UINT nRank = CGlobalVariable::uploadqueue->GetWaitingPosition(this);
    if (!nRank)
        return;
    Packet* packet = new Packet(OP_QUEUERANKING,12,OP_EMULEPROT);
    PokeUInt16(packet->pBuffer+0, (uint16)nRank);
    memset(packet->pBuffer+2, 0, 10);
    if (thePrefs.GetDebugClientTCPLevel() > 0)
        DebugSend("OP__QueueRank", this);
    theStats.AddUpDataOverheadFileRequest(packet->size);
    socket->SendPacket(packet,true,true);
}

void CUpDownClient::SendCommentInfo(/*const*/ CKnownFile *file)
{
    if (!m_bCommentDirty || file == NULL || !ExtProtocolAvailable() || m_byAcceptCommentVer < 1)
        return;
    m_bCommentDirty = false;

    UINT rating = file->GetFileRating();
    const CString& desc = file->GetFileComment();
    if (file->GetFileRating() == 0 && desc.IsEmpty())
        return;

    CSafeMemFile data(256);
    data.WriteUInt8((uint8)rating);
    data.WriteLongString(desc, GetUnicodeSupport());
    if (thePrefs.GetDebugClientTCPLevel() > 0)
        DebugSend("OP__FileDesc", this, file->GetFileHash());
    Packet *packet = new Packet(&data,OP_EMULEPROT);
    packet->opcode = OP_FILEDESC;
    theStats.AddUpDataOverheadFileRequest(packet->size);
    socket->SendPacket(packet,true);
}

void CUpDownClient::AddRequestCount(const uchar* fileid)
{
    for (POSITION pos = m_RequestedFiles_list.GetHeadPosition(); pos != 0; )
    {
        Requested_File_Struct* cur_struct = m_RequestedFiles_list.GetNext(pos);
        if (!md4cmp(cur_struct->fileid,fileid))
        {
            if (::GetTickCount() - cur_struct->lastasked < MIN_REQUESTTIME && !GetFriendSlot())
            {
                if (GetDownloadState() != DS_DOWNLOADING)
                    cur_struct->badrequests++;
                if (cur_struct->badrequests == BADCLIENTBAN)
                {
                    Ban();
                }
            }
            else
            {
                if (cur_struct->badrequests)
                    cur_struct->badrequests--;
            }
            cur_struct->lastasked = ::GetTickCount();
            return;
        }
    }
    Requested_File_Struct* new_struct = new Requested_File_Struct;
    md4cpy(new_struct->fileid,fileid);
    new_struct->lastasked = ::GetTickCount();
    new_struct->badrequests = 0;
    m_RequestedFiles_list.AddHead(new_struct);
}

void  CUpDownClient::UnBan()
{
    CGlobalVariable::clientlist->AddTrackClient(this);
    CGlobalVariable::clientlist->RemoveBannedClient(GetIP());
    SetUploadState(US_NONE);
    ClearWaitStartTime();
    //  Comment UI
    UINotify(WM_FILE_UPDATE_UPLOADRANK,0, CGlobalVariable::uploadqueue->GetWaitingUserCount());
    //theApp.emuledlg->transferwnd->ShowQueueCount(CGlobalVariable::uploadqueue->GetWaitingUserCount());

    for (POSITION pos = m_RequestedFiles_list.GetHeadPosition();pos != 0;)
    {
        Requested_File_Struct* cur_struct = m_RequestedFiles_list.GetNext(pos);
        cur_struct->badrequests = 0;
        cur_struct->lastasked = 0;
    }
}

void CUpDownClient::Ban(LPCTSTR pszReason)
{
    // VC-Huby[2006-12-21]: IP为0的不能被ban,否则把lowid都ban了（因为现在有了low2low）,不过还需要改进
    if ( GetIP()==0 )
        return;

    SetChatState(MS_NONE);
    CGlobalVariable::clientlist->AddTrackClient(this);
    if (!IsBanned())
    {
        if (thePrefs.GetLogBannedClients())
            AddDebugLogLine(false,_T("Banned: %s; %s"), pszReason==NULL ? _T("Aggressive behaviour") : pszReason, DbgGetClientInfo());
    }
#ifdef _DEBUG
    else
    {
        if (thePrefs.GetLogBannedClients())
            AddDebugLogLine(false,_T("Banned: (refreshed): %s; %s"), pszReason==NULL ? _T("Aggressive behaviour") : pszReason, DbgGetClientInfo());
    }
#endif
    CGlobalVariable::clientlist->AddBannedClient(GetIP());
    SetUploadState(US_BANNED);
    //  Comment UI
    UINotify(WM_FILE_UPDATE_UPLOADRANK,0,CGlobalVariable::uploadqueue->GetWaitingUserCount());
    //theApp.emuledlg->transferwnd->ShowQueueCount(CGlobalVariable::uploadqueue->GetWaitingUserCount());
    //SendMessage(CGlobalVariable::m_hListenWnd,WM_FILE_UPDATE_PEER,0,(LPARAM)this);
    UpdateUI(UI_UPDATE_QUEUELIST);
    //theApp.emuledlg->transferwnd->queuelistctrl.RefreshClient(this);

    if (socket != NULL && socket->IsConnected())
        socket->ShutDown(SD_RECEIVE); // let the socket timeout, since we dont want to risk to delete the client right now. This isnt acutally perfect, could be changed later
}

uint32 CUpDownClient::GetWaitStartTime() const
{
    if (credits == NULL)
    {
        ASSERT ( false );
        return 0;
    }
    uint32 dwResult = credits->GetSecureWaitStartTime(GetIP());
    if (dwResult > m_dwUploadTime && IsDownloading())
    {
        //this happens only if two clients with invalid securehash are in the queue - if at all
        dwResult = m_dwUploadTime-1;

        if (thePrefs.GetVerbose())
            DEBUG_ONLY(AddDebugLogLine(false,_T("Warning: CUpDownClient::GetWaitStartTime() waittime Collision (%s)"),GetUserName()));
    }
    return dwResult;
}

void CUpDownClient::SetWaitStartTime()
{
    if (credits == NULL)
    {
        return;
    }
    credits->SetSecWaitStartTime(GetIP());
}

void CUpDownClient::ClearWaitStartTime()
{
    if (credits == NULL)
    {
        return;
    }
    credits->ClearWaitStartTime();
}

bool CUpDownClient::GetFriendSlot() const
{
    //  Comment UI
    if (credits && CGlobalVariable::clientcredits->CryptoAvailable())
    {
        switch (credits->GetCurrentIdentState(GetIP()))
        {
        case IS_IDFAILED:
        case IS_IDNEEDED:
        case IS_IDBADGUY:
            return false;
        }
    }
    return m_bFriendSlot;
}

CEMSocket* CUpDownClient::GetFileUploadSocket(bool bLog)
{
    if (m_pPCUpSocket && (IsUploadingToPeerCache() || m_ePeerCacheUpState == PCUS_WAIT_CACHE_REPLY))
    {
        if (bLog && thePrefs.GetVerbose())
            AddDebugLogLine(false, _T("%s got peercache socket."), DbgGetClientInfo());
        return m_pPCUpSocket;
    }
    else
    {
        if (bLog && thePrefs.GetVerbose())
            AddDebugLogLine(false, _T("%s got normal socket."), DbgGetClientInfo());
        return socket;
    }
}

void CUpDownClient::SetCollectionUploadSlot(bool bValue)
{
    ASSERT( !IsDownloading() || bValue == m_bCollectionUploadSlot );
    m_bCollectionUploadSlot = bValue;
}

void CUpDownClient::UpdateDisplayedInfo(bool force)
{
#ifdef _DEBUG
    force = true;
#endif
    DWORD curTick = ::GetTickCount();
    if (force || curTick-m_lastRefreshedDLDisplay > MINWAIT_BEFORE_DLDISPLAY_WINDOWUPDATE+m_random_update_wait)
    {

        //  Comment UI begin
        //SendMessage(CGlobalVariable::m_hListenWnd,WM_FILE_UPDATE_PEER,0,(LPARAM)this);
        UpdateUI(UI_UPDATE_DOWNLOAD_PEERLIST | UI_UPDATE_DOWNLOADLIST | UI_UPDATE_PEERLIST);
        //theApp.emuledlg->transferwnd->downloadlistctrl.UpdateItem(this);
        //theApp.emuledlg->transferwnd->clientlistctrl.RefreshClient(this);
        //theApp.emuledlg->transferwnd->downloadclientsctrl.RefreshClient(this);

        //  Comment UI end

        m_lastRefreshedDLDisplay = curTick;
    }
}

void CUpDownClient::UpdateUI(int flag)
{
    UINotify(WM_FILE_UPDATE_PEER, flag, (LPARAM)this, this);
	
}
