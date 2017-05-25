/*
 * $Id: EmuleUpdater.cpp 4483 2008-01-02 09:19:06Z soarchin $
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
#include "StdAfx.h"
#include ".\emuleupdater.h"

#include "emule.h"
#include "OtherFunctions.h"
#include "Log.h"
#include "emuledlg.h"
#include "TaskbarNotifier.h"

#define HAS_ZLIB

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const UINT WM_HTTPDOWNLOAD_THREAD_FINISHED = WM_APP + 1;


////////////////////////////////////// gzip ///////////////////////////////////
//in the spirit of zlib, lets do something horrible with defines ;)
#ifdef HAS_ZLIB

#include <zlib/zlib.h>

static int gz_magic[2] = {0x1f, 0x8b}; /* gzip magic header */

/* gzip flag byte */
#define ASCII_FLAG   0x01 /* bit 0 set: file probably ascii text */
#define HEAD_CRC     0x02 /* bit 1 set: header CRC present */
#define EXTRA_FIELD  0x04 /* bit 2 set: extra field present */
#define ORIG_NAME    0x08 /* bit 3 set: original file name present */
#define COMMENT      0x10 /* bit 4 set: file comment present */
#define RESERVED     0xE0 /* bits 5..7: reserved */

static int get_byte(HINTERNET m_hHttpFile) {
	unsigned char c;
	DWORD dwBytesRead;
	BOOL b = ::InternetReadFile(m_hHttpFile, &c, 1, &dwBytesRead);
	if(!b)
		return EOF;
	else
		return c;
}

static int check_header(z_stream *stream, HINTERNET m_hHttpFile) {
	int method; /* method byte */
	int flags;  /* flags byte */
	uInt len;
	int c;

	/* Check the gzip magic header */
	for(len = 0; len < 2; len++) {
		c = get_byte(m_hHttpFile);
		if(c != gz_magic[len]) {
			if(len != 0) stream->avail_in++, stream->next_in--;
			if(c != EOF) {
				stream->avail_in++, stream->next_in--;
				//do not support transparent streams
				return stream->avail_in != 0 ? Z_DATA_ERROR : Z_STREAM_END;
			}
			return stream->avail_in != 0 ? Z_OK : Z_STREAM_END;
		}
	}
	method = get_byte(m_hHttpFile);
	flags = get_byte(m_hHttpFile);
	if(method != Z_DEFLATED || (flags & RESERVED) != 0)
		return Z_DATA_ERROR;

	/* Discard time, xflags and OS code: */
	for(len = 0; len < 6; len++) (void)get_byte(m_hHttpFile);

	if((flags & EXTRA_FIELD) != 0) { /* skip the extra field */
		len  =  (uInt)get_byte(m_hHttpFile);
		len += ((uInt)get_byte(m_hHttpFile))<<8;
		/* len is garbage if EOF but the loop below will quit anyway */
		while(len-- != 0 && get_byte(m_hHttpFile) != EOF) ;
	}
	if((flags & ORIG_NAME) != 0) { /* skip the original file name */
		while((c = get_byte(m_hHttpFile)) != 0 && c != EOF) ;
	}
	if((flags & COMMENT) != 0) {   /* skip the .gz file comment */
		while((c = get_byte(m_hHttpFile)) != 0 && c != EOF) ;
	}
	if((flags & HEAD_CRC) != 0) {  /* skip the header crc */
		for(len = 0; len < 2; len++) (void)get_byte(m_hHttpFile);
	}
	//return Z_DATA_ERROR if we hit EOF?
	return Z_OK;
}

#define ACCEPT_ENCODING_HEADER _T("Accept-Encoding: gzip, x-gzip, identity, *;q=0\r\n")

#define ENCODING_CLEAN_UP      if(bEncodedWithGZIP) inflateEnd(&zs)

#define ENCODING_INIT          BOOL bEncodedWithGZIP = FALSE;               \
                               z_stream zs;                                 \
                               unsigned char cBufferGZIP[1024 * 8]

#define ENCODING_QUERY {                                                    \
  /*check for gzip or x-gzip stream*/                                       \
  TCHAR szContentEncoding[32];                                              \
  DWORD dwEncodeStringSize = 32;                                            \
  if(::HttpQueryInfo(m_hHttpFile, HTTP_QUERY_CONTENT_ENCODING,              \
       szContentEncoding, &dwEncodeStringSize, NULL)) {                     \
    if(szContentEncoding[0] == 'x' && szContentEncoding[1] == '-')          \
      szContentEncoding += 2;                                               \
    if(!stricmp(szContentEncoding, "gzip")                                  \
      bEncodedWithGZIP = TRUE;                                              \
   }                                                                        \
  }

#define PREPARE_DECODER                                                     \
  if(bEncodedWithGZIP) {                                                    \
    zs.next_out = cBufferGZIP;                                              \
    zs.zalloc = (alloc_func)0;                                              \
    zs.zfree = (free_func)0;                                                \
    zs.opaque = (voidpf)0;                                                  \
    zs.next_in = (unsigned char*)szReadBuf;                                 \
    zs.next_out = Z_NULL;                                                   \
    zs.avail_in = 0;                                                        \
	zs.avail_out = sizeof(szReadBuf);                                       \
                                                                            \
    VERIFY(inflateInit2(&zs, -MAX_WBITS) == Z_OK);                          \
    int result = check_header(&zs, m_hHttpFile);                            \
    if(result != Z_OK) {                                                    \
      TRACE(_T("An exception occured while decoding the download file\n")); \
      HandleThreadErrorWithLastError(GetResString(IDS_HTTPDOWNLOAD_ERROR_READFILE));\
      inflateEnd(&zs);                                                      \
    }                                                                       \
  }

#define DECODE_DATA(CFILE, DATA, LEN)                                       \
  if(bEncodedWithGZIP) {                                                    \
    zs.next_in = (unsigned char*)DATA;                                      \
    zs.avail_in = LEN;                                                      \
    int iResult;                                                            \
    do {                                                                    \
      zs.total_out = 0;                                                     \
      zs.next_out = cBufferGZIP;                                            \
      zs.avail_out = 1024;                                                  \
      iResult = inflate(&zs, Z_SYNC_FLUSH);                                 \
      CFILE.Write(cBufferGZIP, zs.total_out);                               \
      if(iResult == Z_STREAM_ERROR || iResult == Z_DATA_ERROR) {            \
        TRACE(_T("An exception occured while decoding the download file\n"));\
        HandleThreadErrorWithLastError(GetResString(IDS_HTTPDOWNLOAD_ERROR_READFILE));\
        ENCODING_CLEAN_UP;                                                  \
        return;                                                             \
      }                                                                     \
      /*if(iResult == Z_STREAM_END) {*/                                     \
      /*}*/                                                                 \
    } while(iResult == Z_OK && zs.avail_out == 0);                          \
  } else                                                                    \
    CFILE.Write(DATA, LEN)

#else

#define ACCEPT_ENCODING_HEADER _T("Accept-Encoding: identity, *;q=0\r\n")

#define ENCODING_CLEAN_UP ((void)0)

#define ENCODING_INIT ((void)0)

#define ENCODING_QUERY ((void)0)

#define PREPARE_DECODER ((void)0)

#define DECODE_DATA(CFILE, DATA, LEN) CFILE.Write(DATA, LEN)

#endif

BEGIN_MESSAGE_MAP(CEmuleUpdater, CWnd)
	//{{AFX_MSG_MAP(CEmuleUpdater)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_HTTPDOWNLOAD_THREAD_FINISHED, OnThreadFinished)
	ON_WM_DESTROY()
	ON_WM_CLOSE()
END_MESSAGE_MAP()

CEmuleUpdater::CEmuleUpdater(void)
{
	m_hInternetSession = NULL;
	m_hHttpConnection = NULL;
	m_hHttpFile = NULL;
	m_bAbort = FALSE;
	m_bSafeToClose = FALSE;
	m_pThread = NULL;
}

CEmuleUpdater::~CEmuleUpdater(void)
{
	m_hInternetSession = NULL;
	m_hHttpConnection = NULL;
	m_hHttpFile = NULL;
	m_bAbort = FALSE;
	m_bSafeToClose = FALSE;
	m_pThread = NULL;
}

IMPLEMENT_DYNAMIC(CEmuleUpdater, CWnd);
/****************************************************************************
                          
函数名:
       GetCurrentVersion()

函数功能:
      	获取eMule当前版本号

被本函数调用的函数清单:
      		
调用本函数的函数清单:
		void CEmuleUpdater::Check(const CString& strURL);
      
参数:

返回值:

描述:
  
****************************************************************************/
void CEmuleUpdater::GetCurrentVersion(void)
{
	m_iMajor	= CGlobalVariable::m_nVersionMjr;
	m_iMinor	= CGlobalVariable::m_nVersionMin;
	m_iUpdate	= CGlobalVariable::m_nVersionUpd;
	m_iBuild	= CGlobalVariable::m_nVCVersionBld;
}

/****************************************************************************
                          
函数名:
       Check()

函数功能:
      	根据传入的URL参数连接到服务器，获取服务器上的版本信息，并判断是否进行更新

被本函数调用的函数清单:
      		void CEmuleUpdater::GetCurrentVersion(void);
			void CEmuleUpdater::UpdateAvailable(void);
			void CEmuleUpdater::UpdateNotAvailable(void);
			void CEmuleUpdater::UpdateNoCheck(void);
==================================================================
			HINTERNET InternetOpen(
						LPCTSTR lpszAgent,
						DWORD dwAccessType,
						LPCTSTR lpszProxyName,
						LPCTSTR lpszProxyBypass,
						DWORD dwFlags
			);
==================================================================
			HINTERNET InternetOpenUrl( 
						HINTERNET hInternetSession, 
						LPCTSTR lpszUrl, 
						LPCTSTR lpszHeaders, 
						DWORD dwHeadersLength, 
						DWORD dwFlags, 
						DWORD dwContext
			);
==================================================================
			BOOL WINAPI InternetReadFile(
						HINTERNET hFile, 
						LPVOID lpBuffer, 
						DWORD dwNumberOfBytesToRead, 
						LPDWORD lpdwNumberOfBytesRead
			);
==================================================================
			int MultiByteToWideChar(
						UINT CodePage,         // code page
						DWORD dwFlags,         // character-type options
						LPCSTR lpMultiByteStr, // string to map
						int cbMultiByte,       // number of bytes in string
						LPWSTR lpWideCharStr,  // wide-character buffer
						int cchWideChar        // size of buffer
			);
==================================================================
			AfxExtractSubString
			BOOL WINAPI InternetCloseHandle(
						HINTERNET hInternet
			);
==================================================================
调用本函数的函数清单:
      
参数:
		const CString& strURL
返回值:

描述:
     该函数是CEmuleUpdater的框架函数。该对象的整个流程由该函数确定。
	 该函数(Check)首先获取当前软件的版本信息，随后初始化Wininet，然
	 后根据，传入的参数读取服务器端的版本信息，通过和当前的版本信息
	 来判断是否需要更新，最后关闭Wininet。
****************************************************************************/

void CEmuleUpdater::Check(const CString& strURL)
{
	GetCurrentVersion();//获取eMule当前版本号

	HINTERNET hInet = InternetOpen(UPDATECHECK_BROWSER_STRING, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, NULL);
	HINTERNET hUrl = InternetOpenUrl(hInet, strURL, NULL, (DWORD)-1L,INTERNET_FLAG_RELOAD | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_NO_CACHE_WRITE |WININET_API_FLAG_ASYNC, NULL);

	if(hUrl)
	{
		char szBuffer[512];
		TCHAR szString[512];
		DWORD dwRead;

		if (InternetReadFile(hUrl, szBuffer, sizeof(szBuffer), &dwRead))
		{
			if (dwRead > 0)
			{
				szBuffer[dwRead-1] = 0;
				MultiByteToWideChar(CP_ACP,0,szBuffer, strlen(szBuffer)+1, szString, sizeof(szString)/2);


				CString		strSubMajor;		//主版本号
				CString		strSubMinor;		//副版本号
				CString		strSubUpdate;		//更新号
				CString		strSubBuild;		//建立日期
				CString		strSubURL;			//下载地址

					UINT     iMajor;			//主版本号
					UINT     iMinor;			//副版本号
					UINT	 iUpdate;
					UINT     iBuild;			//建立日期


				AfxExtractSubString(strSubMajor,szString, 0, '|'); //分析字符串，得到主版本号
				AfxExtractSubString(strSubMinor, szString, 1, '|');//分析字符串，得到副版本号
				AfxExtractSubString(strSubUpdate, szString, 2, '|');//分析字符串，得到更新号
				AfxExtractSubString(strSubBuild, szString, 3, '|');//分析字符串，得到建立日期
				AfxExtractSubString(strSubURL, szString, 4, '|');//分析字符串，得到下载地址

				

				iMajor		= _ttoi((LPCTSTR)strSubMajor);
				 iMinor		= _ttoi(strSubMinor);
				 iUpdate	= *(char*)strSubUpdate.GetBuffer() - 'a';
				 iBuild		= _ttoi(strSubBuild);

				 int iCurrent = m_iMajor*10000000 + m_iMinor*100000 + m_iUpdate*10000 + m_iBuild;
				 int iNewVersion = iMajor*10000000 + iMinor*100000 + iUpdate*10000 + iBuild;

				if(iNewVersion - iCurrent > 0)
				{
					UpdateAvailable(strSubURL);  //获得新版本信息，进行下载
				}
				else
				{
					UpdateNotAvailable();	//当前为最新版本，不需要下载
				}
			}
			else
			{
				UpdateNoCheck();  //与服务器连接失败
			}
		}
		else
		{
			UpdateNoCheck();//InternetCloseHandle(hUrl);//与服务器连接失败
		}
	}
	else
	{
		UpdateNoCheck();
	}
	InternetCloseHandle(hInet);
}

/****************************************************************************
                          
函数名:
       UpdateAvailable(void)

函数功能:
      	下载服务器端的升级文件

被本函数调用的函数清单:
_DownloadThread
      		
调用本函数的函数清单:
	void CEmuleUpdater::Check(const CString& strURL);
      
参数:

返回值:

描述:
     
****************************************************************************/
void CEmuleUpdater::UpdateAvailable(const CString& strURL)
{
	m_sURLToDownload = strURL;
	//验证URLDid
	ASSERT(m_sURLToDownload.GetLength());
	if (!AfxParseURL(m_sURLToDownload, m_dwServiceType, m_sServer, m_sObject, m_nPort))
	{
		m_sURLToDownload = _T("http://") + m_sURLToDownload;
		if (!AfxParseURL(m_sURLToDownload, m_dwServiceType, m_sServer, m_sObject, m_nPort))
		{
			TRACE(_T("Failed to parse the URL: %s\n"), m_sURLToDownload);
			CancelUpdater();
		}
	}

	((CemuleDlg*)theApp.GetMainWnd())->OnUpdateAvailable();
	//打开文件准备下载
	if (!m_FileToWrite.Open(m_sFileToDownloadInto, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyWrite))
	{
		TRACE(_T("Failed to open the file to download into, Error:%d\n"), GetLastError());
		CancelUpdater();
	}

	int nSlash = m_sObject.ReverseFind(_T('/'));
	if (nSlash == -1)
		nSlash = m_sObject.ReverseFind(_T('\\'));
	if (nSlash != -1 && m_sObject.GetLength() > 1)
	{
		m_sFilename = m_sObject.Right(m_sObject.GetLength() - nSlash - 1);
	}
	else
	{
		m_sFilename = m_sObject;
	}

	m_pThread = AfxBeginThread(_DownloadThread, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
	if (m_pThread == NULL)
	{
		TRACE(_T("Failed to create download thread, dialog is aborting\n"));
		CancelUpdater();
	}
	m_pThread->m_bAutoDelete = FALSE;
	m_pThread->ResumeThread();
}

/****************************************************************************
                          
函数名:
       UpdateNotAvailable(void)

函数功能:
      	提示用户，当前版本为最新版本

被本函数调用的函数清单:
      		
调用本函数的函数清单:
	void CEmuleUpdater::Check(const CString& strURL);
      
参数:

返回值:

描述:
     
****************************************************************************/
void CEmuleUpdater::UpdateNotAvailable(void)
{
	((CemuleDlg*)theApp.GetMainWnd())->OnUpdateNotAvailable();
}

/****************************************************************************
                          
函数名:
       UpdateNoCheck(void)

函数功能:
      	提示用户，连接服务器失败

被本函数调用的函数清单:
      		
调用本函数的函数清单:
	void CEmuleUpdater::Check(const CString& strURL);
      
参数:

返回值:

描述:
     
****************************************************************************/
void CEmuleUpdater::UpdateNoCheck(void)
{
	((CemuleDlg*)theApp.GetMainWnd())->OnUpdateNoCheck();
}

/****************************************************************************
                          
函数名:
       _DownloadThread(LPVOID pParam)

函数功能:
	  创建一个线程用于下载
被本函数调用的函数清单:
		DownloadThread();
      		
调用本函数的函数清单:
	void CEmuleUpdater::UpdateAvailable(void)
      
参数:

返回值:

描述:
     
****************************************************************************/
UINT AFX_CDECL CEmuleUpdater::_DownloadThread(LPVOID pParam)
{
	DbgSetThreadName("HttpDownload");
	InitThreadLocale();
	CEmuleUpdater* pEmuleUpdater = (CEmuleUpdater*) pParam;
	ASSERT(pEmuleUpdater);
	ASSERT(pEmuleUpdater->IsKindOf(RUNTIME_CLASS(CEmuleUpdater)));
	pEmuleUpdater->DownloadThread();
	return 0;
}

/****************************************************************************
                          
函数名:
       DownloadThread(void)

函数功能:
      	创建一个线程，用于下载

被本函数调用的函数清单:
      		
调用本函数的函数清单:
		UINT AFX_CDECL CEmuleUpdater::_DownloadThread(LPVOID pParam)
      
参数:

返回值:

描述:
  
****************************************************************************/
void CEmuleUpdater::DownloadThread(void)
{
	ENCODING_INIT;
	
	ASSERT(m_hInternetSession == NULL);//创建Internet session句柄
	m_hInternetSession = ::InternetOpen(AfxGetAppName(), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (m_hInternetSession == NULL)
	{
		TRACE(_T("Failed in call to InternetOpen, Error:%d\n"), ::GetLastError());
		HandleThreadErrorWithLastError(GetResString(IDS_HTTPDOWNLOAD_GENERIC_ERROR));
		return;
	}

	//退出线程
	if (m_bAbort)
	{
		PostMessage(WM_HTTPDOWNLOAD_THREAD_FINISHED);
		return;
	}  

	//Setup the status callback function
	if (::InternetSetStatusCallback(m_hInternetSession, _OnStatusCallBack) == INTERNET_INVALID_STATUS_CALLBACK)
	{
		TRACE(_T("Failed in call to InternetSetStatusCallback, Error:%d\n"), ::GetLastError());
		HandleThreadErrorWithLastError(GetResString(IDS_HTTPDOWNLOAD_GENERIC_ERROR));
		return;
	}

	//Should we exit the thread
	if (m_bAbort)
	{
		PostMessage(WM_HTTPDOWNLOAD_THREAD_FINISHED);
		return;
	}

	//连接到HTTP服务器
	ASSERT(m_hHttpConnection == NULL);
	if (m_sUserName.GetLength())
	{
		// Elandal: Assumes sizeof(void*) == sizeof(unsigned long)
		m_hHttpConnection = ::InternetConnect(m_hInternetSession, m_sServer, m_nPort, m_sUserName, 
                                          m_sPassword, m_dwServiceType, 0, (DWORD) this);
	}
	else
	{
		// Elandal: Assumes sizeof(void*) == sizeof(unsigned long)
		m_hHttpConnection = ::InternetConnect(m_hInternetSession, m_sServer, m_nPort, NULL, 
                                          NULL, m_dwServiceType, 0, (DWORD) this);
	}

	if (m_hHttpConnection == NULL)
	{
		TRACE(_T("Failed in call to InternetConnect, Error:%d\n"), ::GetLastError());
		HandleThreadErrorWithLastError(GetResString(IDS_HTTPDOWNLOAD_FAIL_CONNECT_SERVER));
		return;
	}

	//退出线程
	if (m_bAbort)
	{
		PostMessage(WM_HTTPDOWNLOAD_THREAD_FINISHED);
		return;
	}

	//Issue the request to read the file
	LPCTSTR ppszAcceptTypes[2];
	ppszAcceptTypes[0] = _T("*/*");  //We support accepting any mime file type since this is a simple download of a file
	ppszAcceptTypes[1] = NULL;
	ASSERT(m_hHttpFile == NULL);
	// Elandal: Assumes sizeof(void*) == sizeof(unsigned long)
	m_hHttpFile = HttpOpenRequest(m_hHttpConnection, NULL, m_sObject, NULL, NULL, ppszAcceptTypes, INTERNET_FLAG_RELOAD | 
								  INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_KEEP_CONNECTION, (DWORD)this);
	if (m_hHttpFile == NULL)
	{
		TRACE(_T("Failed in call to HttpOpenRequest, Error:%d\n"), ::GetLastError());
		HandleThreadErrorWithLastError(GetResString(IDS_HTTPDOWNLOAD_FAIL_CONNECT_SERVER));
		return;
	}

	//退出线程
	if (m_bAbort)
	{
		PostMessage(WM_HTTPDOWNLOAD_THREAD_FINISHED);
		return;
	}

	//fill in what encoding we support
	HttpAddRequestHeaders(m_hHttpFile, ACCEPT_ENCODING_HEADER, (DWORD)-1L, HTTP_ADDREQ_FLAG_ADD);

//label used to jump to if we need to resend the request
resend:

	//Issue the request
	BOOL bSend = ::HttpSendRequest(m_hHttpFile, NULL, 0, NULL, 0);
	if (!bSend)
	{
		TRACE(_T("Failed in call to HttpSendRequest, Error:%d\n"), ::GetLastError());
		HandleThreadErrorWithLastError(GetResString(IDS_HTTPDOWNLOAD_FAIL_CONNECT_SERVER));
		return;
	}

	//Check the HTTP status code
	TCHAR szStatusCode[32];
	DWORD dwInfoSize = 32;
	if (!HttpQueryInfo(m_hHttpFile, HTTP_QUERY_STATUS_CODE, szStatusCode, &dwInfoSize, NULL))
	{
		TRACE(_T("Failed in call to HttpQueryInfo for HTTP query status code, Error:%d\n"), ::GetLastError());
		HandleThreadError(GetResString(IDS_HTTPDOWNLOAD_INVALID_SERVER_RESPONSE));
		return;
	}
	else
	{
		long nStatusCode = _ttol(szStatusCode);

		//Handle any authentication errors
		if (nStatusCode == HTTP_STATUS_PROXY_AUTH_REQ || nStatusCode == HTTP_STATUS_DENIED)
		{
			// We have to read all outstanding data on the Internet handle
			// before we can resubmit request. Just discard the data.
			char szData[51];
			DWORD dwSize;
			do
			{
				::InternetReadFile(m_hHttpFile, (LPVOID)szData, 50, &dwSize);

			}
			while (dwSize != 0);

			//Bring up the standard authentication dialog
			if (::InternetErrorDlg(GetSafeHwnd(), m_hHttpFile, ERROR_INTERNET_INCORRECT_PASSWORD, FLAGS_ERROR_UI_FILTER_FOR_ERRORS |
                             FLAGS_ERROR_UI_FLAGS_GENERATE_DATA | FLAGS_ERROR_UI_FLAGS_CHANGE_OPTIONS, NULL) == ERROR_INTERNET_FORCE_RETRY)
				goto resend;
		}
		else if (nStatusCode != HTTP_STATUS_OK)
		{
			TRACE(_T("Failed to retrieve a HTTP 200 status, Status Code:%d\n"), nStatusCode);
			HandleThreadErrorWithLastError(GetResString(IDS_HTTPDOWNLOAD_INVALID_HTTP_RESPONSE), nStatusCode);
			return;
		}
	}

	//Check to see if any encodings are supported
	//  ENCODING_QUERY;
	TCHAR szContentEncoding[32];
	DWORD dwEncodeStringSize = 32;
	if(::HttpQueryInfo(m_hHttpFile, HTTP_QUERY_CONTENT_ENCODING, szContentEncoding, &dwEncodeStringSize, NULL))
	{
		if(!_tcsicmp(szContentEncoding, _T("gzip")) || !_tcsicmp(szContentEncoding, _T("x-gzip")))
			bEncodedWithGZIP = TRUE;
	}

	//Update the status control to reflect that we are getting the file information
//	SetStatus(GetResString(IDS_HTTPDOWNLOAD_GETTING_FILE_INFORMATION));

	// Get the length of the file.
	TCHAR szContentLength[32];
	dwInfoSize = 32;
	DWORD dwFileSize = 0;
	BOOL bGotFileSize = FALSE;
	if (::HttpQueryInfo(m_hHttpFile, HTTP_QUERY_CONTENT_LENGTH, szContentLength, &dwInfoSize, NULL))
	{
		//Set the progress control range
		bGotFileSize = TRUE;
		dwFileSize = (DWORD) _ttol(szContentLength);
//		SetProgressRange(dwFileSize);
	}

	//Update the status to say that we are now downloading the file
	//SetStatus(GetResString(IDS_HTTPDOWNLOAD_RETREIVEING_FILE));

	//Now do the actual read of the file
	//DWORD dwStartTicks = ::GetTickCount();
	//DWORD dwCurrentTicks = dwStartTicks;
	DWORD dwBytesRead = 0;
	char szReadBuf[1024];
	DWORD dwBytesToRead = 1024;
	//DWORD dwTotalBytesRead = 0;
	//DWORD dwLastTotalBytes = 0;
	//DWORD dwLastPercentage = 0;

	PREPARE_DECODER;
	do
	{
		if (!::InternetReadFile(m_hHttpFile, szReadBuf, dwBytesToRead, &dwBytesRead))
		{
			TRACE(_T("Failed in call to InternetReadFile, Error:%d\n"), ::GetLastError());
			HandleThreadErrorWithLastError(GetResString(IDS_HTTPDOWNLOAD_ERROR_READFILE));
			return;
		}
		else if (dwBytesRead && !m_bAbort)
		{
			//Write the data to file
			TRY
			{
				DECODE_DATA(m_FileToWrite, szReadBuf, dwBytesRead);
			}
			CATCH(CFileException, e);
			{
				TRACE(_T("An exception occured while writing to the download file\n"));
				HandleThreadErrorWithLastError(GetResString(IDS_HTTPDOWNLOAD_ERROR_READFILE), e->m_lOsError);
				e->Delete();
				//clean up any encoding data before we return
				ENCODING_CLEAN_UP;
				return;
			}
			END_CATCH

			//Increment the total number of bytes read
			//dwTotalBytesRead += dwBytesRead;  

			//UpdateControlsDuringTransfer(dwStartTicks, dwCurrentTicks, dwTotalBytesRead, dwLastTotalBytes, 
                                     //dwLastPercentage, bGotFileSize, dwFileSize);
		}

	}
	while (dwBytesRead && !m_bAbort);

	//Delete the file being downloaded to if it is present and the download was aborted
	m_FileToWrite.Close();
	if (m_bAbort)
		::DeleteFile(m_sFileToDownloadInto);

	//clean up any encoding data before we return
	ENCODING_CLEAN_UP;;

	//We're finished
	if (!m_bAbort)
	{
		DownloadSuceed();
		m_bSafeToClose = TRUE;
	}
	//PostMessage(WM_HTTPDOWNLOAD_THREAD_FINISHED);
}

/****************************************************************************
                          
函数名:
       HandleThreadErrorWithLastError(CString strIDError, DWORD dwLastError)

函数功能:
      	获得最后一个线程错误

被本函数调用的函数清单:
      		
调用本函数的函数清单:
      
参数:

返回值:

描述:
  
****************************************************************************/
void CEmuleUpdater::HandleThreadErrorWithLastError(CString strIDError, DWORD dwLastError)
{
	//Form the error string to report
	CString sError;
	if (dwLastError)
		sError.Format(_T("%d"), dwLastError);
	else
		sError.Format(_T("%d"), ::GetLastError());
	m_sError.Format(strIDError, sError);

	//Delete the file being downloaded to if it is present
	m_FileToWrite.Close();
	::DeleteFile(m_sFileToDownloadInto);

	//PostMessage(WM_HTTPDOWNLOAD_THREAD_FINISHED, 1);
}

/****************************************************************************
                          
函数名:
       HandleThreadError(CString strIDError)

函数功能:
      	获得一个线程错误

被本函数调用的函数清单:
      		
调用本函数的函数清单:
      
参数:

返回值:

描述:
  
****************************************************************************/
void CEmuleUpdater::HandleThreadError(CString strIDError)
{
	m_sError = strIDError;
	PostMessage(WM_HTTPDOWNLOAD_THREAD_FINISHED, 1);
}

void CALLBACK CEmuleUpdater::_OnStatusCallBack(HINTERNET hInternet, DWORD dwContext, DWORD dwInternetStatus, 
                                                  LPVOID lpvStatusInformation, DWORD dwStatusInformationLength)
{
	//Convert from the SDK C world to the C++ world
	// Elandal: Assumes sizeof(void*) == sizeof(unsigned long)
	CEmuleUpdater* pEmuleUpdater = (CEmuleUpdater*) dwContext;
	ASSERT(pEmuleUpdater);
	ASSERT(pEmuleUpdater->IsKindOf(RUNTIME_CLASS(CEmuleUpdater)));
	pEmuleUpdater->OnStatusCallBack(hInternet, dwInternetStatus, lpvStatusInformation, dwStatusInformationLength);
}

/****************************************************************************
                          
函数名:
       OnThreadFinished(WPARAM wParam, LPARAM lParam)

函数功能:
      	获得一个线程错误

被本函数调用的函数清单:
      		
调用本函数的函数清单:
      
参数:

返回值:

描述:
  
****************************************************************************/
LRESULT CEmuleUpdater::OnThreadFinished(WPARAM wParam, LPARAM /*lParam*/)
{
	//It's now safe to close since the thread has signaled us
	m_bSafeToClose = TRUE;

	if (m_bAbort)
	{
		CancelUpdater();//EndDialog(IDCANCEL);
	
	}
	else if (wParam)
	{
		if (!m_sError.IsEmpty())
		{
			LogError(LOG_STATUSBAR, _T("%s"), m_sError);
		}
		CancelUpdater();//EndDialog(IDCANCEL);
	}
	else
	{
		//CancelUpdater();//EndDialog(IDOK);
	}
	return 0L;
}

/****************************************************************************
                          
函数名:
       DownloadSuceed(void)

函数功能:
      	获得一个线程错误

被本函数调用的函数清单:
      		
调用本函数的函数清单:
      
参数:

返回值:

描述:
  
****************************************************************************/
void CEmuleUpdater::DownloadSuceed(void) // 处理成功下载后的事项
{
	theApp.m_bUpdateDownloaded = true;
	if(PathFileExists(m_sFileToDownloadInto))
	{
		ShellExecute( NULL , _T("open"),m_sFileToDownloadInto,NULL,NULL,SW_SHOWNORMAL);
}
}

/****************************************************************************
                          
函数名:
       OnStatusCallBack(
			HINTERNET hInternet,
			DWORD dwInternetStatus,
            LPVOID lpvStatusInformation,
			DWORD dwStatusInformationLength
	   )

函数功能:
      	获得一个线程错误

被本函数调用的函数清单:
      		
调用本函数的函数清单:
      
参数:

返回值:

描述:
  
****************************************************************************/
void CEmuleUpdater::OnStatusCallBack(HINTERNET /*hInternet*/, DWORD dwInternetStatus, 
                                         LPVOID /*lpvStatusInformation*/, DWORD /*dwStatusInformationLength*/)
{
	switch (dwInternetStatus)
	{
		case INTERNET_STATUS_RESOLVING_NAME:
		{
			//SetStatus(GetResString(IDS_HTTPDOWNLOAD_RESOLVING_NAME), (LPCTSTR) lpvStatusInformation);
			break;
		}
		case INTERNET_STATUS_NAME_RESOLVED:
		{
			//SetStatus(GetResString(IDS_HTTPDOWNLOAD_RESOLVED_NAME), (LPCTSTR) lpvStatusInformation);
			break;
		}
		case INTERNET_STATUS_CONNECTING_TO_SERVER:
		{
			//SetStatus(GetResString(IDS_HTTPDOWNLOAD_CONNECTING), (LPCTSTR) lpvStatusInformation);
			break;
		}
		case INTERNET_STATUS_CONNECTED_TO_SERVER:
		{
			//SetStatus(GetResString(IDS_HTTPDOWNLOAD_CONNECTED), (LPCTSTR) lpvStatusInformation);
			break;
		}
		case INTERNET_STATUS_REDIRECT:
		{
			//SetStatus(GetResString(IDS_HTTPDOWNLOAD_REDIRECTING), (LPCTSTR) lpvStatusInformation);
			break;
		}
		default:
		{
			break;
		}
	}
}
void CEmuleUpdater::DestroyUpdater(void)
{
//	CEmuleUpdater::OnDestroy();
}

void CEmuleUpdater::OnDestroy()
{
	if (m_pThread)
	{
		WaitForSingleObject(m_pThread->m_hThread, INFINITE);
		delete m_pThread;
		m_pThread = NULL;
	}

	//Free up the internet handles we may be using
	if (m_hHttpFile)
	{
		::InternetCloseHandle(m_hHttpFile);
		m_hHttpFile = NULL;
	}
	if (m_hHttpConnection)
	{
		::InternetCloseHandle(m_hHttpConnection);
		m_hHttpConnection = NULL;
	}
	if (m_hInternetSession)
	{
		::InternetCloseHandle(m_hInternetSession);
		m_hInternetSession = NULL;
	}

	CWnd::OnDestroy();

	// TODO: 在此处添加消息处理程序代码
}

void CEmuleUpdater::OnClose()
{
	if (m_bSafeToClose)	
		CWnd::OnClose();
	else
	{
		m_bAbort = TRUE;	
	}
}

void CEmuleUpdater::CancelUpdater(void)
{
	// Asynchronously free up the internet handles we may be using.
	// Otherwise we may get some kind of deadlock situation, because 'InternetConnect'
	// may not return for a very long time...
	if (m_hHttpFile)
	{
		::InternetCloseHandle(m_hHttpFile);
		m_hHttpFile = NULL;
	}
	if (m_hHttpConnection)
	{
		::InternetCloseHandle(m_hHttpConnection);
		m_hHttpConnection = NULL;
	}
	if (m_hInternetSession)
	{
		::InternetCloseHandle(m_hInternetSession);
		m_hInternetSession = NULL;
	}

	//Just set the abort flag to TRUE and
	//disable the cancel button
	m_bAbort = TRUE;
}
