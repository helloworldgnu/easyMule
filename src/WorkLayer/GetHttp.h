#pragma once
#include <afxinet.h>

class CGetHttp
{
public:
	CGetHttp(void);
	~CGetHttp(void);
public:
	void	Close();
	BOOL	DownloadXML(LPCTSTR url);
	BOOL	DownloadFile(LPCTSTR pszUrl,LPCTSTR pszFileName);
	PCHAR   GetBuffer(){return m_pBuffer;}

protected:
	 CInternetSession*	m_pInternetSession;
	 CInternetFile*		m_pInternetFile;
	 CHttpConnection*	m_pConnection;
	 int				m_iTimeOut;
	 CString			m_sURL;
	 int				m_iRetryCount;

	 CHAR				*m_pBuffer;
	 int				m_nSize;

protected:
	DWORD		GetServerType(CString sURL);
	BOOL		Open(CString sURL);
	BOOL		EstablishConnection(void);
	void		ProcessHttpRequest(CHttpFile* pFile);
	BOOL		ProcessHttpException(CHttpFile* pFile, CException* pEx);
};
