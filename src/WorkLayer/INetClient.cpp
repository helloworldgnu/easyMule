#include "StdAfx.h"
#include ".\inetclient.h"

#include "PartFile.h"
#include "Preferences.h"
#include "ClientCredits.h"
#include "globalvariable.h"
#include "emule.h"

IMPLEMENT_DYNAMIC(CINetClient,CUpDownClient)

CINetClient::CINetClient(IPSite * pIpSite)
{
	m_uiFileSize = 0;
	m_bAddOtherSources = true;
	m_pIpSite = 0;
	m_dwErrorCount = 0;
	m_dwLastErrorCount = 0;
	m_bAllowedAddMoreConn = false;
	m_dwLastAddOtherSources = 0;
	m_pIpSite = pIpSite;
	if( NULL!=pIpSite )
		m_pIpSite->m_dwConnectionCount++;
	CGlobalVariable::m_uCurrentPublicConnectNum++;	
}

CINetClient::~CINetClient(void)
{
	if( NULL!=m_pIpSite )
		m_pIpSite->m_dwConnectionCount--;
	CGlobalVariable::m_uCurrentPublicConnectNum--;
}
void CINetClient::ConnectionEstablished()
{
	SetDownloadState(DS_CONNECTED);
}
uint32  CINetClient::GetTimeUntilReask() const
{	
	ASSERT( reqfile );

	DWORD dwTickNow = GetTickCount();
	DWORD dwLastAskedtime = GetLastAskedTime(reqfile);

	if( (dwTickNow-dwLastAskedtime)<INetFileAskTimeInterval )
		return INetFileAskTimeInterval-(dwTickNow-dwLastAskedtime);
	else
		return 0; //OK,已超时INetFileAskTimeInterval,看看是否有需要下载的任务..		
}

int CINetClient::CreateBlockRequests_Before(int iMaxBlocks)
{
	if( m_pBlockRangeToDo==NULL )
	{
		GetBlockRange();
	}

	if( m_pBlockRangeToDo==NULL )
		return 0;//return CUpDownClient::CreateBlockRequests( iMaxBlocks );

	return min( iMaxBlocks,int(m_pBlockRangeToDo->m_iBlockIndexE-m_pBlockRangeToDo->m_iBlockCurrentDoing+1) ); /// m_iBlockCurrentDoing 还没完成的;
}

void CINetClient::CreateBlockRequests_After(int /*iMaxBlocks*/)
{
	if( m_PendingBlocks_list.IsEmpty() ) /// 领不到活干了,说明当前的BlockRange做完了
	{
#ifdef _DEBUG_PEER
		Debug( _T("Peer(%d) Finished BlockRange(%d-%d-%d)-can note get PendingBlocks from it. \n"),m_iPeerIndex,m_pBlockRangeToDo->m_iBlockIndexS,m_pBlockRangeToDo->m_iBlockCurrentDoing,m_pBlockRangeToDo->m_iBlockIndexE);
#endif
		m_pBlockRangeToDo->m_bRangeIsFinished = true;
		m_pBlockRangeToDo->m_pClient	= NULL;
		if( reqfile->GetTotalGapSizeInBlockRange(m_pBlockRangeToDo->m_iBlockIndexS,m_pBlockRangeToDo->m_iBlockIndexE) )
			m_pBlockRangeToDo->m_bDataFinished = true;
		m_pBlockRangeToDo = NULL;		
	}

#ifdef _DEBUG_PEER
	TRACE( "%s-Peer(%d)-speed(%d),PendingBlocksCount=%d,DownloadBlocksCount=%d.",__FUNCTION__,m_iPeerIndex,GetDownloadDatarate(),m_PendingBlocks_list.GetCount(),m_DownloadBlocks_list.GetCount());
	if( m_pBlockRangeToDo )
		Debug( _T("from BlockRange(%d-%d-%d) "),m_pBlockRangeToDo->m_iBlockIndexS,m_pBlockRangeToDo->m_iBlockCurrentDoing,m_pBlockRangeToDo->m_iBlockIndexE);
	TRACE( "\n" );
#endif
}

void CINetClient::SetSiteBadOrGood()
{
	m_pIpSite->m_bBadSite = true;
	if (m_pIpSite->m_pUrlSite->IsBadUrlSite())
	{
		m_pIpSite->m_pUrlSite->m_bBadSite = true;
	}
	else
		m_pIpSite->m_pUrlSite->m_bBadSite = false;
}

bool CINetClient::TryToGetBlockRequests(int iMaxBlocks)
{
	SetCompletePartStatus( );
	
	if( reqfile && reqfile->HavePartNeedValid() )
	{
		CreateBlockRequestsOrg(iMaxBlocks);
	}
	else
	{
		if (m_PendingBlocks_list.IsEmpty())
		{
			while( GetBlockRange() )
			{
				ASSERT( m_pBlockRangeToDo->m_iBlockIndexE!=(UINT)-1 && m_pBlockRangeToDo->m_iBlockIndexE>=m_pBlockRangeToDo->m_iBlockIndexS );
				CreateBlockRequests(iMaxBlocks);	//
				if( !m_PendingBlocks_list.IsEmpty() )
					break;
			}		
		}

		if( m_PendingBlocks_list.IsEmpty() )
		{
			CreateBlockRequestsOrg(iMaxBlocks);
		}
	}


	if (m_PendingBlocks_list.IsEmpty())
	{
		SetDownloadState(DS_NONEEDEDPARTS);		
		return false;
	}

	return true;
}

INetBlockRange_Struct* CINetClient::GetBlockRange()
{
	if( m_pBlockRangeToDo )
		return m_pBlockRangeToDo; //考虑pause后resume,该Peer可以接着原来的BlockRange继续做..

	//ASSERT(m_pBlockRangeToDo==NULL);

	if( reqfile->m_BlockRangeList.IsEmpty() )
	{
		reqfile->SplitFileToBlockRange();
	}

	/// 先找到没有Peer在做的BlockRange
	for( POSITION pos=reqfile->m_BlockRangeList.GetHeadPosition(); pos!=0; )
	{
		INetBlockRange_Struct* pBlockRange = reqfile->m_BlockRangeList.GetNext(pos);
		if( pBlockRange->m_bDataFinished || pBlockRange->m_bRangeIsFinished )
			continue;

		if( pBlockRange->m_pClient==NULL )
		{
			pBlockRange->m_pClient = this;
			pBlockRange->m_dwTakeOverTime = GetTickCount();
			pBlockRange->m_iBlockLastReqed = pBlockRange->m_iBlockCurrentDoing;
			m_pBlockRangeToDo = pBlockRange;
#ifdef _DEBUG_PEER
			Debug( _T("Peer(%d) Get the BlockRange(%d,%d,%d) because no peer do it.\n"),m_iPeerIndex,pBlockRange->m_iBlockIndexS
				,pBlockRange->m_iBlockCurrentDoing,pBlockRange->m_iBlockIndexE );
#endif
			CString sLogOut;
			sLogOut.Format(GetResString(IDS_BLOCK_RANGE),pBlockRange->m_iBlockIndexS
				,pBlockRange->m_iBlockCurrentDoing,pBlockRange->m_iBlockIndexE );
			AddPeerLog(new CTraceInformation(sLogOut));
			return pBlockRange;
		}
	}

	/// 找一个最慢的或还有最多Block需要完成的BlockRange,然后帮忙完成
	INetBlockRange_Struct *pBlockRangeMostTimeToFinish=NULL,*pBlockRangeMostCountToFinish=NULL;
	float fMostTimeToFinish = 10.0; //先设定至少还有10s多时间才能做完的才有必要被打劫..
	float fTimeToFinish;
	UINT iMostCountToFinish = 0;
	UINT iToFinish;	
	UINT iRemainBlockRangeStart;
	for( POSITION pos=reqfile->m_BlockRangeList.GetHeadPosition(); pos!=0; )
	{
		INetBlockRange_Struct* pBlockRange = reqfile->m_BlockRangeList.GetNext(pos);
		if( pBlockRange->m_bRangeIsFinished /* ||pBlockRange->m_iBlockIndexE==pBlockRange->m_iBlockLastReqed */ )
			continue;		

		iRemainBlockRangeStart = pBlockRange->m_iBlockCurrentDoing;//max(pBlockRange->m_iBlockLastReqed,pBlockRange->m_iBlockCurrentDoing);
		iToFinish= pBlockRange->m_iBlockIndexE-iRemainBlockRangeStart+1;
		fTimeToFinish=0;
		if( pBlockRange->m_pClient->GetDownloadDatarate()!=0 )
		{
			fTimeToFinish = iToFinish*EMBLOCKSIZE*(float)1.0/pBlockRange->m_pClient->GetDownloadDatarate();
			if( fTimeToFinish<10.0 )
				continue;
		}

		if( iToFinish>iMostCountToFinish )
		{
			iMostCountToFinish = iToFinish;
			pBlockRangeMostCountToFinish = pBlockRange;
		}

		if( (GetTickCount()-pBlockRange->m_dwTakeOverTime)<10*1000 )
			continue; /// 此BlockRange刚刚才被领走,看不出真实速度		

		if( pBlockRange->m_pClient->GetDownloadDatarate()==0 )
		{
#ifdef _DEBUG_PEER
			Debug( _T("Peer(%d)-speed(%d) hijack all of the BlockRange(%d,%d,%d,%d) because peer(%d) no speed after 10s. \n"),m_iPeerIndex,GetDownloadDatarate()
				,pBlockRange->m_iBlockIndexS,pBlockRange->m_iBlockCurrentDoing,pBlockRange->m_iBlockLastReqed,pBlockRange->m_iBlockIndexE,pBlockRange->m_pClient->m_iPeerIndex );
#endif			
			pBlockRange->m_pClient->SetDownloadState( DS_NONEEDEDPARTS );
			pBlockRange->m_pClient->m_pBlockRangeToDo = NULL;
			pBlockRange->m_dwTakeOverTime = GetTickCount(); 
			pBlockRange->m_iBlockLastReqed = pBlockRange->m_iBlockCurrentDoing; 
			pBlockRange->m_pClient = this; /// 被这个新Source全部打劫
			m_pBlockRangeToDo = pBlockRange;
			CString sLogOut;
			sLogOut.Format(GetResString(IDS_HIJACK_BLOCK),pBlockRange->m_iBlockIndexS,pBlockRange->m_iBlockCurrentDoing,pBlockRange->m_iBlockIndexE);
			AddPeerLog(new CTraceInformation(sLogOut));
			return pBlockRange;
		}
		else if( fTimeToFinish>fMostTimeToFinish )
		{
			fMostTimeToFinish = fTimeToFinish;
			pBlockRangeMostTimeToFinish = pBlockRange; 
		}			

	}//for

	INetBlockRange_Struct* pBlockRangeToHijack = pBlockRangeMostTimeToFinish ? pBlockRangeMostTimeToFinish : (pBlockRangeMostCountToFinish?pBlockRangeMostCountToFinish:NULL);

	if( pBlockRangeToHijack==NULL )
		return NULL;
	ASSERT( pBlockRangeToHijack->m_iBlockCurrentDoing>=pBlockRangeToHijack->m_iBlockIndexS 
		    && pBlockRangeToHijack->m_iBlockCurrentDoing<=pBlockRangeToHijack->m_iBlockIndexE);

	/// 找到了要最长时间才能完成或有最多任务要完成的BlockRange,开始按平分打劫或速度比例打劫
	UINT iBlockCountLeft=0,iHijackCount=0;	
	iBlockCountLeft = pBlockRangeToHijack->m_iBlockIndexE - pBlockRangeToHijack->m_iBlockCurrentDoing + 1;
	if( GetDownloadDatarate()==0 )
		iHijackCount = iBlockCountLeft / 2;
	else
		iHijackCount =  iBlockCountLeft*GetDownloadDatarate()/(pBlockRangeToHijack->m_pClient->GetDownloadDatarate() + GetDownloadDatarate());
	if( iHijackCount<1 )
	{
		//TODO,180K内再打劫,处理比较麻烦,将来再说
	}
	else
	{
		//if( pBlockRangeToHijack->m_GetDownloadDatarate()==0 && iBlockCountLeft>1 ) //打劫速度为零的Peer
		ASSERT( iHijackCount<=iBlockCountLeft );

		INetBlockRange_Struct* pNewBlockRange = new INetBlockRange_Struct;
		pNewBlockRange->m_iBlockIndexE = pBlockRangeToHijack->m_iBlockIndexE;
		pNewBlockRange->m_iBlockIndexS = pNewBlockRange->m_iBlockIndexE - iHijackCount + 1;
		pNewBlockRange->m_iBlockCurrentDoing = pNewBlockRange->m_iBlockIndexS;
		pNewBlockRange->m_dwTakeOverTime = GetTickCount();
		pNewBlockRange->m_iBlockLastReqed = pNewBlockRange->m_iBlockCurrentDoing;
		pNewBlockRange->m_pClient = this;
		pBlockRangeToHijack->m_iBlockIndexE = max((int)pNewBlockRange->m_iBlockIndexS-1,pBlockRangeToHijack->m_iBlockCurrentDoing); //被截尾

		ASSERT( pNewBlockRange->m_iBlockIndexS<=pNewBlockRange->m_iBlockIndexE);
		ASSERT( pBlockRangeToHijack->m_iBlockIndexS<=pNewBlockRange->m_iBlockIndexE );

		if( iHijackCount==iBlockCountLeft )
		{
			pBlockRangeToHijack->m_bRangeIsFinished = true; //全部被打劫了.. :-)
			pBlockRangeToHijack->m_pClient->SetDownloadState(DS_NONEEDEDPARTS);
			pBlockRangeToHijack->m_pClient->m_pBlockRangeToDo = NULL;
			if( reqfile->GetTotalGapSizeInBlockRange(pBlockRangeToHijack->m_iBlockIndexS,pBlockRangeToHijack->m_iBlockIndexE)==0 )
				pBlockRangeToHijack->m_bDataFinished = true;
		}
		reqfile->m_BlockRangeList.AddTail( pNewBlockRange );
		m_pBlockRangeToDo = pNewBlockRange;
#ifdef _DEBUG_PEER
		Debug( _T("Peer(%d)-speed(%d) hijack the BlockRange(%d,%d,%d) from Peer(%d)-speed(%d)-blockRange(%d,%d,%d,%d) \n"),m_iPeerIndex,GetDownloadDatarate(),
			pNewBlockRange->m_iBlockIndexS,pNewBlockRange->m_iBlockCurrentDoing,pNewBlockRange->m_iBlockIndexE,pBlockRangeToHijack->m_pClient->m_iPeerIndex,pBlockRangeToHijack->m_pClient->GetDownloadDatarate(),
			pBlockRangeToHijack->m_iBlockIndexS,pBlockRangeToHijack->m_iBlockCurrentDoing,pBlockRangeToHijack->m_iBlockLastReqed,pBlockRangeToHijack->m_iBlockIndexE);
#endif
		CString sLogOut;
		sLogOut.Format(GetResString(IDS_HIJACK_FROM),pNewBlockRange->m_iBlockIndexS
			,pNewBlockRange->m_iBlockCurrentDoing,pNewBlockRange->m_iBlockIndexE,pBlockRangeToHijack->m_iBlockIndexS,pBlockRangeToHijack->m_iBlockCurrentDoing,pBlockRangeToHijack->m_iBlockIndexE );
		AddPeerLog(new CTraceInformation(sLogOut));
		if( pBlockRangeToHijack->m_iBlockLastReqed>=pNewBlockRange->m_iBlockIndexS )
		{
			//已请求的也被打劫了,先吐出来
			pBlockRangeToHijack->m_pClient->RemoveDownloadBlockRequests(pNewBlockRange->m_iBlockIndexS,pBlockRangeToHijack->m_iBlockLastReqed);
		}
		return pNewBlockRange;
	}

#ifdef _DEBUG_PEER
	Debug( _T("Peer(%d)-speed(%d) can not get BlockRange \n") );
#endif

	return NULL;
}

bool CINetClient::RequestBlock( Requested_Block_Struct** newblocks,uint16* pCount, bool bUseParent )
{
	if(bUseParent)
		return reqfile->GetNextRequestedBlock(this, newblocks, pCount);
	else
		return reqfile->GetBlockRequestFromBlockRange(this, newblocks, pCount);
}
void CINetClient::UpdateTransData(const CString& strURL)
{
 	POSITION pos = CGlobalVariable::filemgr.m_UrlList.GetHeadPosition();
 	while(pos)
	{
		CFileTaskItem *pItem = CGlobalVariable::filemgr.m_UrlList.GetValueAt(pos);
		if (pItem->m_strUrl == reqfile->GetPartFileURL())
	    {
			POSITION position = pItem->m_lMetaLinkURLList.GetHeadPosition();
			while (position)
			{
				CUrlSite &pSite = const_cast<CUrlSite &>( pItem->m_lMetaLinkURLList.GetNext(position) );
				if (pSite.m_strUrl == strURL)
				{
					pSite.m_dwDataTransferedWithoutPayload =  getIpSite()->m_pUrlSite->m_dwDataTransferedWithoutPayload;
					break;
				}
			}
			break;
		}
		CGlobalVariable::filemgr.m_UrlList.GetNext(pos);
	}

}

bool CINetClient::IsNeedAvoidInitalizeConnection()
{
	POSITION position = this->reqfile->m_UrlSiteList.GetHeadPosition();
	while (position)
	{
		CUrlSite *pUrlSite = this->reqfile->m_UrlSiteList.GetNext(position);
		for( size_t i = 0; i < pUrlSite->m_IPSiteList.size(); i++ ) {
			if( this->m_pIpSite == pUrlSite->m_IPSiteList[i] )
				continue;

			if( this->m_pIpSite->m_dwIpAddress == pUrlSite->m_IPSiteList[i]->m_dwIpAddress ) {
				if( pUrlSite->m_IPSiteList[i]->m_dwConnectionCount > 1 )
					// 已经发起了
					return true;
			}
		}
	}

	return false;
}

bool CINetClient::IsOriginalUrlSite( )
{
	if( m_pIpSite && m_pIpSite->m_pUrlSite )
		return (sfStartDown==m_pIpSite->m_pUrlSite->m_dwFromWhere);

	return false;

/*
	if( m_pIpSite && m_pIpSite->m_pUrlSite && reqfile )
	{
		CString strUrl = m_pIpSite->m_pUrlSite->m_strUrl;
		ParseRef( strUrl );
		CString strUrlOfPartfile = reqfile->GetINetDownLoadURL();
		ParseRef(strUrlOfPartfile);
		if( strUrlOfPartfile.CompareNoCase(strUrl)==0 )
			return true;
	}

	return false;
*/
}

DWORD CINetClient::GetIPFrom( ) const
{
	if(m_pIpSite)
	{
		CUrlSite* pUrlSite = m_pIpSite->m_pUrlSite;
		if( pUrlSite )
		{
			return pUrlSite->m_dwFromWhere;
		}
	}

	return sfUnknwon;
}
