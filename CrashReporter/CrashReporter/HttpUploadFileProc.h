#pragma once

#include <AfxMt.h>
#include <WinInet.h>
#pragma comment (lib, "WinInet.lib")

#define MAX_DESC	1024

struct SHttpUploadFileThreadParam
{
	TCHAR	szFileName[MAX_PATH];
	HWND	hNotifyWnd;
	UINT	uNotifyMsg;
	TCHAR	szEmail[MAX_PATH];
	TCHAR	szDesc[MAX_DESC];
};

#define P_HUF_PROGRESS									MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_ITF, 0x200)

#define P_HUF_OPEN_FILE									MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_ITF, 0x201)
#define P_HUF_INTERNET_OPEN								MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_ITF, 0x202)
#define P_HUF_INTERNET_CONNET							MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_ITF, 0x203)
#define P_HUF_HTTP_OPEN_REQUEST							MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_ITF, 0x204)
#define P_HUF_SENDING_FILE								MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_ITF, 0x205)
#define P_HUF_WAIT_RESPONSE								MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_ITF, 0x206)

#define S_HUF_FINISHED									MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_ITF, 0x300)


#define E_HUF_OPEN_FILE_FAILED							MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x201)
#define E_HUF_INTERNET_OPEN_FAILED						MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x202)
#define E_HUF_INTERNET_CONNECT_FAILED					MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x203)
#define E_HUF_HTTP_OPEN_REQUEST_FAILED					MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x204)
#define E_HUF_SEND_FILE_FAILED							MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x205)
#define E_HUF_WAIT_RESPONSE_FAILD						MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x206)

class CHttpUploadFileProc : public CWinThread
{
	DECLARE_DYNCREATE(CHttpUploadFileProc)

protected:
	CHttpUploadFileProc(void);		// 动态创建所使用的受保护的构造函数
	virtual ~CHttpUploadFileProc(void);

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

	SHttpUploadFileThreadParam		m_threadParam;
public:
	static CString Result2Str(HRESULT hr);

	BOOL	IsTerminated();
	void	Terminate();
protected:
	BOOL HttpUploadFile();
	BOOL UseHttpSendReqEx(HINTERNET hRequest, CFile &file);
	void	NotifyReceiver(WPARAM wParam, LPARAM lParam);

	CStringA	GetBoundary(){return "---------------------------7d629031603ca";}
	CStringA	GetHttpAppendHeader();
	CStringA	GetContentHead(LPCSTR lpszFileName);
	CStringA	GetContentTail();
	CStringA	ValueStr(LPCSTR lpszName, LPCSTR lpszValue);

protected:
	BOOL							m_bTerminate;
	CCriticalSection				m_csTerminate;

protected:
	DECLARE_MESSAGE_MAP()

};
