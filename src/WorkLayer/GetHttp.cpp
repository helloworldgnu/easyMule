#include "StdAfx.h"
#include <Shlwapi.h>
#include "GetHttp.h"

#define  HTTP_VERB_GET		1

CGetHttp::CGetHttp(void)
{
	m_pInternetFile		= NULL;
	m_pInternetSession	= NULL;
	m_pConnection		= NULL;
	m_iTimeOut			= 3000;
	m_iRetryCount		= 0;
	m_pBuffer			= NULL;
	m_nSize				= 0;
}

CGetHttp::~CGetHttp(void)
{
	Close();
}

DWORD CGetHttp::GetServerType(CString sURL)
{
	CString sServer, sObject, sProxy, sAuthentication;
	INTERNET_PORT nPort;
	DWORD dwServiceType;

	if (PathFileExists(sURL))
	{
		dwServiceType = AFX_INET_SERVICE_FILE;
	}
	else
	{
		if (!AfxParseURL(sURL, dwServiceType, sServer, sObject, nPort))
		{
			return AFX_INET_SERVICE_FILE;
		}
	}
	return dwServiceType;
}

BOOL CGetHttp::Open(CString sURL)
{
	if(m_pInternetFile)
	{
		Close();
	}

	m_sURL = sURL;
	return EstablishConnection();
}


BOOL  CGetHttp::EstablishConnection(void)
{
	CString sServer;
	CString sObject;
	INTERNET_PORT nPort;
	DWORD dwServiceType;

	if(m_pInternetSession)
	{
		m_pInternetSession->Close();
		delete m_pInternetSession;
		return FALSE;
	}

	m_iRetryCount = 0;

	try
	{
		m_pInternetSession = new CInternetSession();
	}
	catch (CException* pEx)
	{
		pEx->Delete();
		return FALSE;
	}

	m_pInternetSession->SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, m_iTimeOut);
	m_pInternetSession->SetOption(INTERNET_OPTION_DATA_RECEIVE_TIMEOUT, m_iTimeOut);
	m_pInternetSession->SetOption(INTERNET_OPTION_CONNECT_RETRIES, 3);

	if (!AfxParseURL(m_sURL, dwServiceType, sServer, sObject, nPort))
	{
		return FALSE;
	}

	try
	{
		m_pConnection = m_pInternetSession->GetHttpConnection(sServer, nPort);

		unsigned long ulFlags = INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_RELOAD;
		if (dwServiceType == AFX_INET_SERVICE_HTTPS)
		{
			ulFlags = ulFlags | INTERNET_FLAG_SECURE;
		}

		m_pInternetFile = m_pConnection->OpenRequest(HTTP_VERB_GET, sObject, NULL, 1, NULL, NULL, ulFlags);

		ProcessHttpRequest((CHttpFile*)m_pInternetFile);
		return TRUE;

	}
	catch(CException* pEx)
	{
		pEx->Delete();
		return FALSE;
	}
	return TRUE;
}

void CGetHttp::ProcessHttpRequest(CHttpFile* pFile)
{
	BOOL bRetry = FALSE;
	// 处理Http请求
	do 
	{
		try
		{
			pFile->SendRequest();						// 发送空的请求
		}
		catch (CException* pEx)
		{
			bRetry = ProcessHttpException(pFile, pEx);	//处理Http异常
			pEx->Delete();
		}
	}
	while (bRetry && m_iRetryCount++ <= 1);
}

BOOL CGetHttp::ProcessHttpException(CHttpFile* pFile, CException* pEx)
{
	BOOL bReturn = FALSE;
	DWORD dwFlags;

	DWORD dwLastError = ((CInternetException*)pEx)->m_dwError;	//获取错误

	// 检测是否是有效的异常
	if (pEx && pEx->IsKindOf(RUNTIME_CLASS(CInternetException)))
	{
		CInternetException* pException = (CInternetException*) pEx;
		DWORD dwLastError = pException->m_dwError;		//获取错误

		switch (dwLastError)
		{
			// 忽略安全错误
		case ERROR_INTERNET_INVALID_CA:

			dwFlags = SECURITY_FLAG_IGNORE_CERT_CN_INVALID  | SECURITY_FLAG_IGNORE_CERT_DATE_INVALID | SECURITY_FLAG_IGNORE_UNKNOWN_CA | SECURITY_FLAG_IGNORE_REVOCATION | SECURITY_FLAG_IGNORE_WRONG_USAGE;
			pFile->SetOption(INTERNET_OPTION_SECURITY_FLAGS, (LPVOID)&dwFlags, sizeof(dwFlags));

			bReturn = TRUE;
			break;

		case ERROR_INTERNET_INVALID_URL:
			bReturn = FALSE;
			break;

		default:
			bReturn = TRUE;
			break;
		}
	}
	return bReturn;
}

void CGetHttp::Close()
{
	if(m_pInternetFile)
	{
		m_pInternetFile->Close();
		delete m_pInternetFile;
	}

	if(m_pInternetSession)
	{
		m_pInternetSession->Close();
		delete m_pInternetSession;
	}

	if(m_pConnection)
	{
		m_pConnection->Close();
		delete m_pConnection;
	}

	if (m_pBuffer)
	{
		free(m_pBuffer);
		m_pBuffer = NULL;
	}

	m_nSize = 0;
	m_pInternetFile = NULL;
	m_pInternetSession = NULL;
	m_pConnection = NULL;
}

BOOL CGetHttp::DownloadXML(LPCTSTR url)
{
	switch (GetServerType(url))
	{
	case AFX_INET_SERVICE_HTTP:
	case AFX_INET_SERVICE_HTTPS:
		if (Open(url))
		{
			DWORD dwStatusCode = 0;

			((CHttpFile*) m_pInternetFile)->QueryInfoStatusCode(dwStatusCode);

			if (dwStatusCode == HTTP_STATUS_OK)
			{
				int iFileBytes = 0;
				int iFileBytesCopied = 0;
				int iBytesRead = 0;

				iFileBytes = iFileBytes = static_cast<int>(m_pInternetFile->SeekToEnd());

				m_pBuffer = (char *)malloc(iFileBytes * sizeof(char));

				if (!m_pBuffer)
					return FALSE;

				m_pInternetFile->Seek(0L, CFile::begin);
				while (iBytesRead = m_pInternetFile->Read(m_pBuffer + iFileBytesCopied,iFileBytes - iFileBytesCopied))
				{
					iFileBytesCopied += iBytesRead;
				}
				ASSERT(iFileBytesCopied == iFileBytes);
				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}

BOOL CGetHttp::DownloadFile(LPCTSTR pszUrl,LPCTSTR pszFileName)
{
	switch (GetServerType(pszUrl))
	{
	case AFX_INET_SERVICE_HTTP:
	case AFX_INET_SERVICE_HTTPS:
		if (Open(pszUrl))
		{
			DWORD dwStatusCode = 0;

			((CHttpFile*) m_pInternetFile)->QueryInfoStatusCode(dwStatusCode);

			if (dwStatusCode == HTTP_STATUS_OK)
			{
				FILE * fp = NULL;
				fp = _tfopen(pszFileName,_T("wb"));
				if (!fp)
					return FALSE;

				char szBuffer[1024];
				memset(szBuffer, 0, 1024);

				int iFileBytes = 0;				//文件大小
				int iFileBytesCopied = 0;	//已经复制的文件
				int iBytesRead = 0;			//读取的字节

				iFileBytes = iFileBytes = static_cast<int>(m_pInternetFile->SeekToEnd());
				m_pInternetFile->Seek(0L, CFile::begin);
				while (iBytesRead = m_pInternetFile->Read(szBuffer,1024))
				{
					fwrite(szBuffer,sizeof(char),iBytesRead,fp);
					iFileBytesCopied += iBytesRead;
				}
				fflush(fp); fclose(fp);
				ASSERT (iFileBytesCopied == iFileBytes);
				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}