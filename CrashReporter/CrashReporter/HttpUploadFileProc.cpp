#include "StdAfx.h"
#include ".\httpuploadfileproc.h"

IMPLEMENT_DYNCREATE(CHttpUploadFileProc, CWinThread)

CHttpUploadFileProc::CHttpUploadFileProc(void)
{
	ZeroMemory(&m_threadParam, sizeof(SHttpUploadFileThreadParam));
	m_bTerminate = FALSE;
}

CHttpUploadFileProc::~CHttpUploadFileProc(void)
{
}

BEGIN_MESSAGE_MAP(CHttpUploadFileProc, CWinThread)
END_MESSAGE_MAP()

BOOL CHttpUploadFileProc::InitInstance()
{
	// TODO: 在此执行任意逐线程初始化
	HttpUploadFile();

	return FALSE;
}

int CHttpUploadFileProc::ExitInstance()
{
	// TODO: 在此执行任意逐线程清理
	return CWinThread::ExitInstance();
}


CString CHttpUploadFileProc::Result2Str(HRESULT hr)
{
	CString		str;
	switch(hr) {
	case P_HUF_OPEN_FILE:
		str = _T("打开文件");
		break;
	case P_HUF_INTERNET_OPEN:
		str = _T("初始化网络");
		break;
	case P_HUF_INTERNET_CONNET:
		str = _T("初始化连接");
		break;
	case P_HUF_HTTP_OPEN_REQUEST:
		str = _T("开始通信");
		break;
	case P_HUF_SENDING_FILE:
		str = _T("发送文件");
		break;
	case P_HUF_WAIT_RESPONSE:
		str = _T("等待回应");
		break;
	case S_HUF_FINISHED:
		str = _T("完成");
		break;

	case E_HUF_OPEN_FILE_FAILED:
		str = _T("打开文件失败");
		break;
	case E_HUF_INTERNET_OPEN_FAILED:
		str = _T("初始化网络失败");
		break;
	case E_HUF_INTERNET_CONNECT_FAILED:
		str = _T("初始化连接失败");
		break;
	case E_HUF_HTTP_OPEN_REQUEST_FAILED:
		str = _T("无法开始通信");
		break;
	case E_HUF_SEND_FILE_FAILED:
		str = _T("文件传输过程失败");
		break;
	case E_HUF_WAIT_RESPONSE_FAILD:
		str = _T("等待回应失败");
		break;
	default:
		str.Empty();
		break;
	}

	return str;
}

BOOL CHttpUploadFileProc::HttpUploadFile()
{
	if (NULL == m_threadParam.szFileName
		|| _T('\0') == m_threadParam.szFileName[0])
		return FALSE;

	BOOL		bRet;
	LPCTSTR		lpszAgent = _T("eMule Verycd");
	LPCTSTR		lpszServerName = _T("crash.emule.org.cn");
	LPCTSTR		lpszObjectName = _T("/report");
	CFile		file;
	HINTERNET hSession = NULL;
	HINTERNET hConnect = NULL;
	HINTERNET hRequest = NULL;


	NotifyReceiver(P_HUF_OPEN_FILE, 0);

	if (!file.Open(m_threadParam.szFileName, CFile::modeRead | CFile::shareDenyWrite))
	{
		NotifyReceiver(E_HUF_OPEN_FILE_FAILED, 0);
		return FALSE;
	}

	NotifyReceiver(P_HUF_PROGRESS, 5);



	bRet = TRUE;
	do {

		if (IsTerminated())	goto Label_Quit;
		NotifyReceiver(P_HUF_INTERNET_OPEN, 0);
		hSession = InternetOpen(lpszAgent, INTERNET_OPEN_TYPE_PRECONFIG,
			NULL, NULL, 0);
		if(NULL == hSession)
		{
			NotifyReceiver(E_HUF_INTERNET_OPEN_FAILED, 0);
			bRet = FALSE;
			break;
		}

		if (IsTerminated())	goto Label_Quit;
		NotifyReceiver(P_HUF_INTERNET_CONNET, 0);
		hConnect = InternetConnect(hSession, lpszServerName, INTERNET_DEFAULT_HTTP_PORT,
									NULL, NULL, INTERNET_SERVICE_HTTP,NULL, NULL);
		if (NULL == hConnect)	
		{
			NotifyReceiver(E_HUF_INTERNET_CONNECT_FAILED, 0);
			bRet = FALSE;
			break;
		}

		if (IsTerminated())	goto Label_Quit;
		NotifyReceiver(P_HUF_HTTP_OPEN_REQUEST, 0);
		hRequest = HttpOpenRequest(hConnect, "POST", lpszObjectName, 
			NULL, NULL, NULL, INTERNET_FLAG_NO_CACHE_WRITE, 0);
		if (NULL == hRequest)
		{
			NotifyReceiver(E_HUF_HTTP_OPEN_REQUEST_FAILED, 0);
			bRet = FALSE;
			break;
		}

		NotifyReceiver(P_HUF_PROGRESS, 10);

		if (IsTerminated())	goto Label_Quit;
		if (!UseHttpSendReqEx(hRequest, file))
		{
			NotifyReceiver(E_HUF_SEND_FILE_FAILED, 0);
			bRet = FALSE;
		}
	} while(0);

	if (IsTerminated())	goto Label_Quit;
	if (bRet)
	{
		NotifyReceiver(P_HUF_PROGRESS, 100);
		NotifyReceiver(S_HUF_FINISHED, 0);
	}

Label_Quit:;

	if (NULL != hRequest)
	{
		InternetCloseHandle(hRequest);
		hRequest = NULL;
	}
	if (NULL != hConnect)
	{
		InternetCloseHandle(hConnect);
		hConnect = NULL;
	}
	if (NULL != hSession)
	{
		InternetCloseHandle(hSession);
		hSession = NULL;
	}

	file.Close();


	return bRet;
}

BOOL CHttpUploadFileProc::UseHttpSendReqEx(HINTERNET hRequest, CFile &file)
{
	//	生成form-data协议信息	<begin>
	CStringA	straHeader;
	CStringA	straContentHead;
	CStringA	straContentTail;

	straHeader = GetHttpAppendHeader();
	straContentHead = GetContentHead(file.GetFileName());
	straContentTail = GetContentTail();
	//	生成form-data协议信息	<end>

	ULONGLONG	ullFileLength = file.GetLength();
	DWORD	dwContentLength = straContentHead.GetLength() + (DWORD) ullFileLength + straContentTail.GetLength();

	INTERNET_BUFFERS BufferIn = {0};
	BufferIn.dwStructSize = sizeof( INTERNET_BUFFERS ); // Must be set or error will occur
	BufferIn.Next = NULL; 
	//BufferIn.lpcszHeader = NULL;
	//BufferIn.dwHeadersLength = 0;
	BufferIn.lpcszHeader = straHeader.LockBuffer();
	straHeader.UnlockBuffer();
	BufferIn.dwHeadersLength = (DWORD)straHeader.GetLength();
	BufferIn.dwHeadersTotal = 0;
	BufferIn.lpvBuffer = NULL;                
	BufferIn.dwBufferLength = 0;
	BufferIn.dwBufferTotal = dwContentLength;
	BufferIn.dwOffsetLow = 0;
	BufferIn.dwOffsetHigh = 0;

	if (IsTerminated())	return FALSE;
	NotifyReceiver(P_HUF_SENDING_FILE, 0);
	if(!HttpSendRequestEx( hRequest, &BufferIn, NULL, 0, 0))
		return FALSE;

	UINT	uProgressBegin = 10;
	UINT	uProgressEnd = 90;
	UINT	uProgressCur = 0;
	UINT	uReadCountSum = 0;


	const UINT uBufSize = 1024;
	BYTE pBuffer[uBufSize];
	UINT uReadCount;
	DWORD dwBytesWritten;
	BOOL bRet;
	float	fSendPercent;
	UINT	uProgressScale;

	
	//	Write Head
	bRet = InternetWriteFile( hRequest, straContentHead.LockBuffer(), straContentHead.GetLength(), &dwBytesWritten);
	straContentHead.UnlockBuffer();
	if(!bRet)
	{
		NotifyReceiver(E_HUF_SEND_FILE_FAILED, 0);
		return FALSE;
	}

	if (IsTerminated())	return FALSE;
	//	Write file contents
	uReadCountSum = 0;
	bRet = TRUE;
	file.SeekToBegin();
	do{
		if (IsTerminated())	return FALSE;
		uReadCount = file.Read(pBuffer, uBufSize);
		if (0 == uReadCount)
			break;

		if(! InternetWriteFile( hRequest, pBuffer, uReadCount, &dwBytesWritten))
		{
			NotifyReceiver(E_HUF_SEND_FILE_FAILED, 0);
			return FALSE;
		}

		uReadCountSum += uReadCount;
		fSendPercent = (float)uReadCountSum / BufferIn.dwBufferTotal;
		uProgressScale = (UINT) ((uProgressEnd - uProgressBegin) * fSendPercent);
		uProgressCur = uProgressBegin + uProgressScale;
		NotifyReceiver(P_HUF_PROGRESS, uProgressCur);

	} while (uReadCount == uBufSize);

	if (IsTerminated())	return FALSE;
	//	Write Tail
	bRet = InternetWriteFile( hRequest, straContentTail.LockBuffer(), straContentTail.GetLength(), &dwBytesWritten);
	straContentTail.UnlockBuffer();
	if(!bRet)
	{
		NotifyReceiver(E_HUF_SEND_FILE_FAILED, 0);
		return FALSE;
	}
	NotifyReceiver(P_HUF_PROGRESS, uProgressEnd);


	if (IsTerminated())	return FALSE;
	NotifyReceiver(P_HUF_WAIT_RESPONSE, 0);
	if (!HttpEndRequest(hRequest, NULL, 0, 0))
	{
		NotifyReceiver(E_HUF_WAIT_RESPONSE_FAILD, 0);
		bRet = FALSE;
	}

	return bRet;
}

void CHttpUploadFileProc::NotifyReceiver(WPARAM wParam, LPARAM lParam)
{
	if (IsTerminated())
		return;

	::PostMessage(m_threadParam.hNotifyWnd, m_threadParam.uNotifyMsg, wParam, lParam);
}

CStringA CHttpUploadFileProc::GetHttpAppendHeader()
{
	CStringA	straHeader;
	straHeader.Format("Content-Type: multipart/form-data; boundary=%s\r\n", GetBoundary());
	return straHeader;
}

CStringA CHttpUploadFileProc::GetContentHead(LPCSTR lpszFileName)
{
	CStringA	straContentHead;
	CStringA	straTemp;

	straContentHead.Empty();

	straTemp.Format("--%s\r\n", GetBoundary());						//First Boundary
	straContentHead += straTemp;

	straTemp = ValueStr("Email", CStringA(m_threadParam.szEmail));	//Email
	straContentHead += straTemp;

	straTemp = ValueStr("FileName", lpszFileName);	//FileName
	straContentHead += straTemp;

	straTemp = ValueStr("Description", CStringA(m_threadParam.szDesc));	//FileName
	straContentHead += straTemp;


	straTemp.Format("Content-Disposition: form-data; name=\"filedata\"; filename=\"%s\"\r\n"			//File
						"Content-Type: application/octet-stream\r\n"										//File Type
						"\r\n",
						lpszFileName);
	straContentHead += straTemp;

	return straContentHead;
}

CStringA CHttpUploadFileProc::GetContentTail()
{
	CStringA	straContentTail;
	straContentTail.Format("\r\n--%s--\r\n", GetBoundary());
	return straContentTail;
}

CStringA CHttpUploadFileProc::ValueStr(LPCSTR lpszName, LPCSTR lpszValue)
{
	CStringA	stra;
	stra.Format("Content-Disposition: form-data; name=\"%s\"\r\n"				//Name
		"\r\n"
		"%s"															//Value
		"\r\n--%s\r\n",													//Boundary
		lpszName, lpszValue, GetBoundary());
	return stra;
}

BOOL CHttpUploadFileProc::IsTerminated()
{
	BOOL	bRet;
	
	m_csTerminate.Lock();
	bRet = m_bTerminate;
	m_csTerminate.Unlock();
	
	return bRet;
}
void CHttpUploadFileProc::Terminate()
{
	m_csTerminate.Lock();
	m_bTerminate = TRUE;
	m_csTerminate.Unlock();
}
