/* 
 * $Id: PartFile.h 9297 2008-12-24 09:55:04Z dgkang $
 * 
 * Copyright (C)2002-2006 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#pragma once
#include "KnownFile.h"
#include "DeadSourceList.h"
#include "CorruptionBlackBox.h"
#include "UrlSrcFromSvrMgr.h"
#include "updownclient.h"

#include "HttpClient.h"
#include "UIMessage.h"

#include "ClientReqSocket.h"
#include "UrlSite.h"

#include <string>
#include <vector>

enum PartFileSizeStatus
{
    FS_UNKNOWN = 0,		//FS_:FileStatus
    FS_KNOWN,
	FS_NOSIZE,
	FS_KNOWN_FROM_ORIGINAL
};

enum EPartFileStatus{
    PS_READY			= 0,	//PS_:PartStatus
    PS_EMPTY			= 1,
    PS_WAITINGFORHASH	= 2,
    PS_HASHING			= 3,
    PS_ERROR			= 4,
    PS_INSUFFICIENT		= 5,
    PS_UNKNOWN			= 6,
    PS_PAUSED			= 7,
    PS_COMPLETING		= 8,
    PS_COMPLETE			= 9
};

#define PR_VERYLOW			4 // I Had to change this because it didn't save negative number correctly.. Had to modify the sort function for this change..
#define PR_LOW				0 //*
#define PR_NORMAL			1 // Don't change this - needed for edonkey clients and server!
#define PR_HIGH				2 //*
#define PR_VERYHIGH			3
#define PR_AUTO				5 //UAP Hunter

//#define BUFFER_SIZE_LIMIT 500000 // Max bytes before forcing a flush
#ifdef _SUPPORT_MEMPOOL
#define BUFFER_TIME_LIMIT	300000 // Max milliseconds before forcing a flush
#else
#define BUFFER_TIME_LIMIT	60000 // Max milliseconds before forcing a flush
#endif


#define	PARTMET_BAK_EXT	_T(".bak")
#define	PARTMET_TMP_EXT	_T(".backup")

#define STATES_COUNT		17

enum EPartFileFormat{
    PMT_UNKNOWN			= 0,
    PMT_DEFAULTOLD,
    PMT_SPLITTED,
    PMT_NEWOLD,
    PMT_SHAREAZA,
    PMT_BADFORMAT
};

#define	FILE_COMPLETION_THREAD_FAILED	0x0000
#define	FILE_COMPLETION_THREAD_SUCCESS	0x0001
#define	FILE_COMPLETION_THREAD_RENAMED	0x0002

enum EPartFileOp
{
    PFOP_NONE = 0,
    PFOP_HASHING,
    PFOP_COPYING,
    PFOP_UNCOMPRESSING
};
enum {NOMAL_FILE = 0,METALINK_FILE = 1,REMETALINK_FILE = 2,METALINKPART_FILE = 3,STOPPEDMETALINK_FILE = 4};//VC-wangna[2007-12-18]:文件类型

class CSearchFile;
class CUpDownClient;
enum EDownloadState;
class CxImage;
class CSafeMemFile;
class CED2KFileLink;
class CMetaLinkParser;

#pragma pack(1)
struct Requested_Block_Struct
{
	Requested_Block_Struct()
	{
		bBlockReqHelpRobed = FALSE;
	}
	uint64	StartOffset;
    uint64	EndOffset;
    uchar	FileID[16];
    uint64  transferred; // Barry - This counts bytes completed
	uint32  BlockIdx;
	BOOL    bBlockReqHelpRobed;
};
#pragma pack()

struct Gap_Struct
{
    uint64 start;
    uint64 end;
};

struct PartFileBufferedData
{
    BYTE *data;						// Barry - This is the data to be written
    uint64 start;					// Barry - This is the start offset of the data
    uint64 end;						// Barry - This is the end offset of the data
    Requested_Block_Struct *block;	// Barry - This is the requested block that this data relates to
};

class CFileTaskItem;

typedef CTypedPtrList<CPtrList, CUpDownClient*> CUpDownClientPtrList;

///////////////////////////////////////////////////////////////////////////
//
// 事件列表(EventList)
//

typedef CTypedPtrList<CObList, CTraceEvent*> EventList;

class CPartFile : public CKnownFile
{
    DECLARE_DYNAMIC(CPartFile)

    friend class CPartFileConvert;
public:
    CPartFile(UINT cat = 0);
    CPartFile(CSearchFile* searchresult, const CString & filepath, UINT cat = 0);
    CPartFile(CString edonkeylink, UINT cat = 0);
    CPartFile(class CED2KFileLink* fileLink, UINT cat = 0);
    virtual ~CPartFile();

    bool	IsPartFile() const
    {
        return !(status == PS_COMPLETE);
    }

    // VC-SearchDream[2007-05-16]: See the movie while downloading Begin
	bool    IsSeeOnDownloading() const; 
	void    SetSeeOnDownloading(CWnd* pParent, bool bSee);
	bool	IsSeeReady();
	bool    IsCompleteforPlayer(uint64 start, uint64 &end, bool bIgnoreBufferedData);
	// VC-SearchDream[2007-05-16]: See the movie while downloading End

	// eD2K filename
	virtual void SetFileName(LPCTSTR pszFileName, bool bReplaceInvalidFileSystemChars = false); // 'bReplaceInvalidFileSystemChars' is set to 'false' for backward compatibility!
	bool    GetFileNameConflicted() const { return m_bFileNameConflicted; }

    PartFileSizeStatus GetPartFileSizeStatus()
    {
        return m_PartFileSizeStatus;
    }

    void SetPartFileSizeStatus(PartFileSizeStatus status)
    {
        m_PartFileSizeStatus = status;
	}
	// VC-SearchDream[2007-04-10]: for HTTP and FTP Direct DownLoad End

    // part.met filename (without path!)
    const CString& GetPartMetFileName() const
    {
        return m_partmetfilename;
    }

    // full path to part.met file or completed file
    const CString& GetFullName() const
    {
        return m_fullname;
    }
    void	SetFullName(CString name)
    {
        m_fullname = name;
    }
    CString	GetTempPath() const;

    // local file system related properties
    bool	IsNormalFile() const
    {
        return (m_dwFileAttributes & (FILE_ATTRIBUTE_COMPRESSED | FILE_ATTRIBUTE_SPARSE_FILE)) == 0;
    }
    const bool	IsAllocating() const
    {
        return m_AllocateThread != NULL;
    }
    EMFileSize	GetRealFileSize() const;
    void	GetSizeToTransferAndNeededSpace(uint64& pui64SizeToTransfer, uint64& pui32NeededSpace) const;
    uint64	GetNeededSpace() const;

    // last file modification time (NT's version of UTC), to be used for stats only!
    CTime	GetCFileDate() const
    {
        return CTime(m_tLastModified);
    }
    uint32	GetFileDate() const
    {
        return m_tLastModified;
    }

    // file creation time (NT's version of UTC), to be used for stats only!
    CTime	GetCrCFileDate() const
    {
        return CTime(m_tCreated);
    }
    uint32	GetCrFileDate() const
    {
        return m_tCreated;
    }

    void	InitializeFromLink(CED2KFileLink* fileLink, UINT cat = 0,bool bFromMetaServer=false);
	void InitFromOriginalURL(void);

    virtual uint32	Process(uint32 reducedownload, UINT icounter);
    uint8	LoadPartFile(LPCTSTR in_directory, LPCTSTR filename, bool getsizeonly = false,bool bFromMetBakDir = false); //filename = *.part.met
    uint8	LoadPartFile(LPCTSTR in_filepath, bool bFromBakup=false);
    uint8	ImportShareazaTempfile(LPCTSTR in_directory,LPCTSTR in_filename , bool getsizeonly);

    bool	SavePartFile();
    void	PartFileHashFinished(CKnownFile* result);
    bool	HashSinglePart(UINT partnumber); // true = ok , false = corrupted
	void    OnSinglePartHashFailed(UINT uPartNumber,uint32 partRange, bool bNoAICH=false);

    void	AddGap(uint64 start, uint64 end);
    void	FillGap(uint64 start, uint64 end);
    void	DrawStatusBar(CDC* dc, LPCRECT rect, bool bFlat) /*const*/;
    virtual void	DrawShareStatusBar(CDC* dc, LPCRECT rect, bool onlygreyrect, bool	 bFlat) const;
    bool	IsComplete(uint64 start, uint64 end, bool bIgnoreBufferedData) const;
    bool	IsPureGap(uint64 start, uint64 end) const;
    bool	IsAlreadyRequested(uint64 start, uint64 end) const;
    bool    ShrinkToAvoidAlreadyRequested(uint64& start, uint64& end) const;
    bool	IsCorruptedPart(UINT partnumber) const;
    uint64	GetTotalGapSizeInRange(uint64 uRangeStart, uint64 uRangeEnd) const;
    uint64	GetTotalGapSizeInPart(UINT uPart) const;
	uint64  GetTotalGapSizeInBlockRange(UINT iBlockStart,UINT iBlockEnd) const;
	void	GetFilePosOfBlock( int iBlockIdx,uint64* piBlockPosStart,uint64* piBlockPosEnd=NULL ) const;
    void	UpdateCompletedInfos();
    void	UpdateCompletedInfos(uint64 uTotalGaps);
    virtual void	UpdatePartsInfo();

	EPartFileStatus	GetStatus(bool ignorepause = false) const;
	void	SetStatus(EPartFileStatus eStatus);		// set status and update GUI
	void	_SetStatus(EPartFileStatus eStatus);	// set status and do *not* update GUI
	void	NotifyStatusChange();
	bool	IsStopped() const { return stopped; }
	bool	IsPaused() const {return paused;}
	bool	GetCompletionError() const { return m_bCompletionError; }
	EMFileSize  GetCompletedSize() const { return completedsize; }
	CString getPartfileStatus() const;
	int		getPartfileStatusRang() const;
	void	SetActive(bool bActive);
	// VC-SearchDream[2007-05-16]: for see movie while downloading
	bool    GetNextRequestedBlockOnSee(CUpDownClient* sender, Requested_Block_Struct** newblocks, uint16* count);
    bool	GetNextRequestedBlock(CUpDownClient* sender, Requested_Block_Struct** newblocks, uint16* count) /*const*/;
	void    BlockReqHelped( Requested_Block_Struct* pReqBlockOld,Requested_Block_Struct* pReqBlockNew);
    void	WritePartStatus(CSafeMemFile* file) /*const*/;
    void	WriteCompleteSourcesCount(CSafeMemFile* file) const;
    void	AddSources(CSafeMemFile* sources,uint32 serverip, uint16 serverport, bool bWithObfuscationAndHash);
	//MODIFIED by fengwen on 2006/09/29 <begin> : 传递Refer参数，以在请求url时填写。{22555E64-6826-4f57-B027-49617DA784F6}
	//void	AddSource(LPCTSTR pszURL, uint32 nIP);
	void	AddSource(LPCTSTR pszURL, uint32 nIP, LPCTSTR lpszRefer = NULL);
	//MODIFIED by fengwen on 2006/09/29 <end> : 传递Refer参数，以在请求url时填写。{22555E64-6826-4f57-B027-49617DA784F6}

	bool ChangedToMetalinkFile( CMetaLinkParser * parser );
	bool AddMetalinkSource( CMetaLinkParser * parser,ESiteFrom siteFrom=sfMetaServer );

    static bool CanAddSource(uint32 userid, uint16 port, uint32 serverip, uint16 serverport, UINT* pdebug_lowiddropped = NULL, bool Ed2kID = true);

	bool RemoveTag( int tag_name );

    uint8	GetDownPriority() const
    {
        return m_iDownPriority;
    }
    void	SetDownPriority(uint8 iNewDownPriority, bool resort = true);
    bool	IsAutoDownPriority(void) const
    {
        return m_bAutoDownPriority;
    }
    void	SetAutoDownPriority(bool NewAutoDownPriority)
    {
        m_bAutoDownPriority = NewAutoDownPriority;
    }
    void	UpdateAutoDownPriority();

    UINT	GetSourceCount() const
    {
        return srclist.GetCount();
    }
    UINT	GetSrcA4AFCount() const
    {
        return A4AFsrclist.GetCount();
    }
    UINT	GetSrcStatisticsValue(EDownloadState nDLState) const;
    UINT	GetTransferringSrcCount() const;
    uint64	GetTransferred() const
    {
        return m_uTransferred;
    }
    uint32	GetDatarate() const
    {
        return datarate;
    }
    float	GetPercentCompleted() const
    {
        return percentcompleted;
    }
    UINT	GetNotCurrentSourcesCount() const;
    int		GetValidSourcesCount() const;
    bool	IsArchive(bool onlyPreviewable = false) const; // Barry - Also want to preview archives
    bool    IsPreviewableFileType() const;
    time_t	getTimeRemaining() const;
    time_t	getTimeRemainingSimple() const;
    uint32	GetDlActiveTime() const;

    // Barry - Added as replacement for BlockReceived to buffer data before writing to disk
    virtual uint32	WriteToBuffer(uint64 transize, const BYTE *data, uint64 start, uint64 end, Requested_Block_Struct *block, const CUpDownClient* client);
    virtual void	FlushBuffer(bool forcewait=false, bool bForceICH = false, bool bNoAICH = false);
    // Barry - This will invert the gap list, up to caller to delete gaps when done
    // 'Gaps' returned are really the filled areas, and guaranteed to be in order
    void	GetFilledList(CTypedPtrList<CPtrList, Gap_Struct*> *filled) const;

    // Barry - Added to prevent list containing deleted blocks on shutdown
    void	RemoveAllRequestedBlocks(void);
    bool	RemoveBlockFromList(uint64 start, uint64 end);
    bool	IsInRequestedBlockList(const Requested_Block_Struct* block) const;
    void	RemoveAllSources(bool bTryToSwap);

    bool	CanOpenFile() const;
    bool	IsReadyForPreview() const;
    bool	CanStopFile() const;
    bool	CanPauseFile() const;
    bool	CanResumeFile() const;

    void	OpenFile() const;
    void	PreviewFile();
    void	DeleteFile();
    void	StopFile(bool bCancel = false, bool resort = true,bool bFinished=false,bool bFailed = false);
    void	PauseFile(bool bInsufficient = false, bool resort = true ,bool bStop=false);
    void	StopPausedFile();
    void	ResumeFile(bool resort = true);
    void	ResumeFileInsufficient();

	virtual Packet* CreateSrcInfoPacket(const CUpDownClient* forClient, uint8 byRequestedVersion, uint16 nRequestedOptions) const;
	void	AddClientSources(CSafeMemFile* sources, uint8 sourceexchangeversion, bool bSourceExchange2, /*const*/ CUpDownClient* pClient = NULL);

	void AddFileLog(CTraceEvent* Event);    
    UINT	GetAvailablePartCount() const
    {
        return availablePartsCount;
    }
    void	UpdateAvailablePartsCount();

    uint32	GetLastAnsweredTime() const
    {
        return m_ClientSrcAnswered;
    }
    void	SetLastAnsweredTime()
    {
        m_ClientSrcAnswered = ::GetTickCount();
    }
    void	SetLastAnsweredTimeTimeout();

    uint64	GetCorruptionLoss() const
    {
        return m_uCorruptionLoss;
    }
    uint64	GetCompressionGain() const
    {
        return m_uCompressionGain;
    }
    uint32	GetRecoveredPartsByICH() const
    {
        return m_uPartsSavedDueICH;
    }

    virtual void	UpdateFileRatingCommentAvail(bool bForceUpdate = false);

    void	AddDownloadingSource(CUpDownClient* client);
    void	RemoveDownloadingSource(CUpDownClient* client);

    CString GetProgressString(uint16 size) const;
    CString GetInfoSummary() const;

//	int		GetCommonFilePenalty() const;
    void	UpdateDisplayedInfo(bool force = false);

    UINT	GetCategory() /*const*/;
    void	SetCategory(UINT cat);
    bool	CheckShowItemInGivenCat(int inCategory) /*const*/;

    uint8*	MMCreatePartStatus();

    //preview
    virtual bool GrabImage(uint8 nFramesToGrab, double dStartTime, bool bReduceColor, uint16 nMaxWidth,void* pSender);
    virtual void GrabbingFinished(CxImage** imgResults, uint8 nFramesGrabbed, void* pSender);

    void	FlushBuffersExceptionHandler(CFileException* error);
    void	FlushBuffersExceptionHandler();

    void	PerformFileCompleteEnd(DWORD dwResult,bool& bDeleted);

    void	SetFileOp(EPartFileOp eFileOp);
    EPartFileOp GetFileOp() const
    {
        return m_eFileOp;
    }
    void	SetFileOpProgress(UINT uProgress);
    UINT	GetFileOpProgress() const
    {
        return m_uFileOpProgress;
    }

    void	RequestAICHRecovery(UINT nPart);
    void	AICHRecoveryDataAvailable(UINT nPart);

    uint32	m_LastSearchTime;
    uint32	m_LastSearchTimeKad;
    uint8	m_TotalSearchesKad;
    uint64	m_iAllocinfo;
    CUpDownClientPtrList srclist;
    CUpDownClientPtrList A4AFsrclist; //<<-- enkeyDEV(Ottavio84) -A4AF-
    CTime	lastseencomplete;
    CFile	m_hpartfile;				// permanent opened handle to avoid write conflicts
    CMutex 	m_FileCompleteMutex;		// Lord KiRon - Mutex for file completion
    uint16	src_stats[4];
    uint16  net_stats[3];
    volatile bool m_bPreviewing;
    volatile bool m_bRecoveringArchive; // Is archive recovery in progress
    bool	m_bLocalSrcReqQueued;
    bool	srcarevisible;				// used for downloadlistctrl
    bool	hashsetneeded;
    bool    AllowSwapForSourceExchange()
    {
        return ::GetTickCount()-lastSwapForSourceExchangeTick > 30*1000;
    } // ZZ:DownloadManager
    void    SetSwapForSourceExchangeTick()
    {
        lastSwapForSourceExchangeTick = ::GetTickCount();
    } // ZZ:DownloadManager

    UINT	SetPrivateMaxSources(uint32 in)
    {
        return m_uMaxSources = in;
    }
    UINT	GetPrivateMaxSources() const
    {
        return m_uMaxSources;
    }
    UINT	GetMaxSources() const;
    UINT	GetMaxSourcePerFileSoft() const;
    UINT	GetMaxSourcePerFileUDP() const;

    bool    GetPreviewPrio() const
    {
        return m_bpreviewprio;
    }
    void    SetPreviewPrio(bool in)
    {
        m_bpreviewprio=in;
    }

	bool OnCannotAllocateMission( CUpDownClient * client/*, Requested_Block_Struct** newblocks , uint16 *max */);	

    static bool RightFileHasHigherPrio(CPartFile* left, CPartFile* right);

    CDeadSourceList	m_DeadSourceList;

#ifdef _DEBUG
    // Diagnostic Support
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif
	
	UINT	GetMetBakId(){ return m_metBakId; }
	void	SetMetBakId(UINT id){ m_metBakId = id;}
protected:
    bool	GetNextEmptyBlockInPart(UINT partnumber, Requested_Block_Struct* result) const;
    void	CompleteFile(bool hashingdone);
    void	CreatePartFile(UINT cat = 0);
    void	CreatePartFile(const CString strPathfile, UINT cat = 0);
    void	Init();

protected:

	static UINT AFX_CDECL AllocateSpaceThread(LPVOID lpParam);

	uint64	m_uTransferred;
	CCorruptionBlackBox	m_CorruptionBlackBox;
	// Barry - Buffered data to be written
	CTypedPtrList<CPtrList, PartFileBufferedData*> m_BufferedData_list;
	uint64	m_nTotalBufferData;
	uint32	m_nLastBufferFlushTime;
	CWinThread* m_AllocateThread;

private:

    BOOL 		PerformFileComplete(); // Lord KiRon
    static UINT CompleteThreadProc(LPVOID pvParams); // Lord KiRon - Used as separate thread to complete file
    void		CharFillRange(CString* buffer,uint32 start, uint32 end, char color) const;

    static CBarShader s_LoadBar;
    static CBarShader s_ChunkBar;
    uint32	m_iLastPausePurge;
    uint16	count;
    UINT	m_anStates[STATES_COUNT];
    EMFileSize	completedsize;
    uint64	m_uCorruptionLoss;
    uint64	m_uCompressionGain;
    uint32	m_uPartsSavedDueICH;
    uint32	datarate;
    CString m_fullname;
    CString m_partmetfilename;
	bool    m_bFileNameConflicted;
	UINT	m_metBakId;// [10/22/2007 huby]: to save the met.bak to AppDown default dir
    UINT	m_uMaxSources;
    bool	paused;
    bool	stopped;
    bool	insufficient;
    bool	m_bCompletionError;
    uint8	m_iDownPriority;
    bool	m_bAutoDownPriority;
    EPartFileStatus	status;
    bool	newdate;	// indicates if there was a writeaccess to the .part file
    uint32	lastpurgetime;
    uint32	m_LastNoNeededCheck;
    CTypedPtrList<CPtrList, Gap_Struct*> gaplist;
    CTypedPtrList<CPtrList, Requested_Block_Struct*> requestedblocks_list;
    CArray<uint16,uint16> m_SrcpartFrequency;
    float	percentcompleted;
    CList<uint16, uint16> corrupted_list;
    uint32	m_ClientSrcAnswered;
    UINT	availablePartsCount;
    DWORD	m_lastRefreshedDLDisplay;
    CUpDownClientPtrList m_downloadingSourceList;
    bool	m_bDeleteAfterAlloc;
    bool	m_bpreviewprio;
    UINT	m_category;
    DWORD	m_dwFileAttributes;
    time_t	m_tActivated;
    uint32	m_nDlActiveTime;
    uint32	m_tLastModified;	// last file modification time (NT's version of UTC), to be used for stats only!
    uint32	m_tCreated;			// file creation time (NT's version of UTC), to be used for stats only!
    uint32	m_random_update_wait;
    volatile EPartFileOp m_eFileOp;
    volatile UINT m_uFileOpProgress;

    DWORD   lastSwapForSourceExchangeTick; // ZZ:DownloadManaager

	bool		m_bDownloadFromOriginal;    /// Added by thilon on 2008.04.29
	uint16		m_iPartToValidFromStartUrl; /// [VC-Huby-080527] 需要从起始站点验证整个完整数据的Part
	uint16      m_iPartToValidFromUrlSite;
	CUrlSite*	m_pUrlSitetoValidBadorNot;  /// 单独验证这个UrlSite是不是一个坏站点

    //ADDED by fengwen on 2006/09/01 <begin> : Record Url sources.
public:
//    CList <CUrlSite> m_strlstUrlSources;
private:
    BOOL	IsExistInUrlSourcesList(LPCTSTR lpszUrl,CUrlSite** ppUrlSiteExist=NULL);
public:
    void	AddUrlSrcToDownloadQueue(LPCTSTR lpszUrl);
private:
    void	ReAddAllUrlSrcToDownloadQueue(void);
    UINT	SaveUrlSourcesToFile(CFileDataIO *pFile);
public:
	CUrlSite* RecordUrlSource(LPCTSTR lpszUrl,bool bAddToDownloaddQueue=true,DWORD dwPref=0,ESiteFrom siteFrom=sfMetaServer);
    //ADDED by fengwen on 2006/09/01 <end> : Record Url sources.

    //ADDED by fengwen on 2006/09/13 <begin> : Get url source from server

    void	OnUrlSrcFromSvrFetched(const CStringList *pUrlList);
    void	ParseUrlSourceRecord(LPCTSTR lpszUrlRec, CString &strUrl, CString &strRefer);

    CUrlSrcFromSvrMgr		m_urlSrcFromSvrMgr;
    BOOL					m_bAlreadyFetchUrlSrc;		//标识是否已经从服务器上取过url源。
    //ADDED by fengwen on 2006/09/13 <end> : Get url source from server

    //ADDED by fengwen on 2006/12/14	<begin> :	先从内存里找，再到文件里找。
    BOOL		GetDataFromBufferThenDisk(BYTE *data, uint64 start, uint64 end);

    enum _PartCheckStatus
    {
        PCS_NOTCHECK, PCS_INTACT, PCS_CORRUPTED
    };
    CArray<int, int>		m_arrPartCheckStatus;

    BOOL		VerifyPartFromBufferAndDisk(UINT uPartIndex);
    virtual void SetFileSize(EMFileSize nFileSize);

	BOOL	NeedMoreSourceFromKad( );
	//ADDED by fengwen on 2006/12/14	<end> :	先从内存里找，再到文件里找。

#ifdef _SUPPORT_MEMPOOL
    int         m_nCounter;
#endif

    //ADDED by fengwen on 2006/12/14	<end> :	先从内存里找，再到文件里找。

    // VC-SearchDream[2007-04-11]: For HTTP and FTP Direct DownLoad
    PartFileSizeStatus	m_PartFileSizeStatus;

	// VC-SearchDream[2007-05-16]: See the movie while downloading Begin 
	bool				m_bSeeOnDownloading; 
	bool				m_bNotifytoPlay;
	uint16				m_nCurrentSeeingPart;
	uint64              m_nCurrentSeeingPosition;
	// VC-SearchDream[2007-05-18]: See the movie while downloading End
public:

#ifdef _DEBUG
	int   GetReqedBlockCount() { return requestedblocks_list.GetCount(); }             
#endif

	bool	CloseToFinish( ); //<[VC-Huby-20080423]: 判断Partfile是否接近下载完成

	void	OnGetFileSizeFromInetPeer( uint64 uiFileSize,bool bOriginal=false ); //< 从http/ftp peer获取到下载任务文件大小

	void    ZeroSize_CompleteDownLoad(); ///VC-Huby[2007-09-03]: 大小确实为零的下载文件结束

	//ADDED by VC-fengwen 2007/08/01 <begin> : 处理未知文件大小的下载

	void	NoSize_CompleteDownLoad(); 	
	void	InitNosizePartFile();
	void    NoSize_FlushBuffer(bool forcewait=false, bool bForceICH = false, bool bNoAICH = false);
	uint32	NoSize_WriteToBuffer(uint64 transize, const BYTE *data, uint64 start, uint64 end, Requested_Block_Struct *block, const CUpDownClient* client);
protected:
	uint64			m_nFileTransferSize;
	//ADDED by VC-fengwen 2007/08/01 <end> : 处理未知文件大小的下载

private:
	void	DeleteSourceByPeerType(CPeerType peerTypeToDel);
	BOOL	IsSafeDrmFile( const CString &strFileFullName );

public:
	BOOL   m_bIsSafeDrmFile;

	DWORD	m_dwTickGetFileSize;

	bool					SplitFileToBlockRange( );					
	CTypedPtrList<CPtrList, INetBlockRange_Struct*> m_BlockRangeList;
	bool GetBlockRequestFromBlockRange( CUpDownClient* sender,Requested_Block_Struct** newblocks,uint16* pCount ); 
	bool GetNextEmptyBlockInBlock( uint32 iBlockIdx, Requested_Block_Struct *pBlock,uint64& iContinuousEnd ); 
	CRBMap<CString,URLEncodeType> m_UrlEncodeTypeMap;

	CFileTaskItem *				  m_pFileTaskItem;
protected:
	void PlayPartFile(CPartFile* pPartFile);

public:
	EventList* GetEventList(void);
	CList<CUrlSite*> * GetUrlSources(void);
	CString& GetINetDownLoadURL(void);
protected:
	EventList	m_EventList;
public:
	void RetryManage(UINT nRetryTimes);
	UINT m_TotalRetryTimes;

private:
#ifdef UNICODE
	std::vector<std::wstring> m_strURLsAdded;
#else
	std::vector<std::string> m_strURLsAdded;
#endif

public:
	CList<CUrlSite *> m_UrlSiteList;
	void		LoadUrlSiteList( CList<CUrlSite>& urlSiteList );
	CUrlSite*   FindDownloadMostUrlSite();
	bool		NeedPostUrlSiteToMetaServer();
	uint32      GetTotalDownFromEd2k();
	bool        HavePartNeedValid() { return (m_iPartToValidFromUrlSite!=(uint16)-1 || m_iPartToValidFromStartUrl!=(uint16)-1); }
	bool        DownloadFromOriginal() {return m_bDownloadFromOriginal;}
	DWORD		GetTaskConnectCount();
	UINT		GetCanConnectMaxNumber();
	UINT		m_uCurrentMainConnectNum;
	UINT		GetMirrorSiteNumber();
	UINT		GetOtherConnectNum(DWORD dwCurrentConnectCount);

	bool NeedCommitted(uint64 uFileSize);
	CArray<CString> m_PostFixArray;

	//VC-dgkang 2008年6月14日
	//控制分配磁盘线程及时的退出
	BOOL m_AllocateThreadQuit; 
};