// CrashReporterDlg.cpp : 实现文件
//

#include "stdafx.h"
#include <io.h>
#include "CrashReporter.h"
#include "CrashReporterDlg.h"
#include ".\crashreporterdlg.h"
#include "HttpUploadFileProc.h"
#include "WinMsg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CCrashReporterDlg 对话框



CCrashReporterDlg::CCrashReporterDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCrashReporterDlg::IDD, pParent)
	, m_bAutoRestart(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	//m_hProcThread = INVALID_HANDLE_VALUE;
	m_pUploadProc = NULL;
	m_bTinyWndSize = TRUE;
	m_uAutoCloseTimer = 0;
}

void CCrashReporterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_EMAIL, m_editEmail);
	DDX_Control(pDX, IDC_STATIC_PROGRESS_STATUS, m_sttcProgrsStatus);
	DDX_Control(pDX, IDC_PROGRESS_SEND, m_progressSend);
	DDX_Control(pDX, ID_BNCANCEL, m_bnCancel);
	DDX_Control(pDX, IDOK, m_bnOk);
	DDX_Control(pDX, IDC_EDIT_DESC, m_editDesc);
	DDX_Control(pDX, IDC_STATIC_TINY_BOUNDARY, m_sttcTinyBoundary);
	DDX_Control(pDX, IDC_STATIC_LARGE_BOUNDARY, m_sttcLargeBoundary);
	DDX_Check(pDX, IDC_CHECK_AUTO_RESTART, m_bAutoRestart);
}

BEGIN_MESSAGE_MAP(CCrashReporterDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_MESSAGE(WM_UPLOAD_PROGRESS, OnUploadProgress)

	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_BN_CLICKED(ID_BNCANCEL, OnBnClickedBncancel)
	ON_EN_MAXTEXT(IDC_EDIT_DESC, OnEnMaxtextEditDesc)
	ON_EN_MAXTEXT(IDC_EDIT_EMAIL, OnEnMaxtextEditEmail)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_MORE_INFO, OnBnClickedMoreInfo)
	ON_WM_DESTROY()
	ON_WM_TIMER()
END_MESSAGE_MAP()


void CCrashReporterDlg::SetProcStatus(int iStatus)
{
	switch(iStatus)
	{
	case PS_NOT_SEND:
		m_sttcProgrsStatus.SetWindowText(_T(""));
		m_sttcProgrsStatus.ShowWindow(SW_HIDE);
		//m_progressSend.ShowWindow(SW_HIDE);
		m_progressSend.SetPos(0);
		m_bnOk.EnableWindow(TRUE);
		m_bnCancel.SetWindowText(_T("不发送"));
		break;
	case PS_SENDING:
		m_sttcProgrsStatus.ShowWindow(SW_SHOW);
		//m_progressSend.ShowWindow(SW_SHOW);
		m_bnOk.EnableWindow(FALSE);
		m_bnCancel.SetWindowText(_T("取消"));
		break;
	case PS_SUCCEEDED:
		m_sttcProgrsStatus.ShowWindow(SW_SHOW);
		//m_progressSend.ShowWindow(SW_SHOW);
		m_bnOk.EnableWindow(FALSE);
		m_bnCancel.SetWindowText(_T("关闭"));
		break;
	default:
		break;
	}

	m_iProcStatus = iStatus;
}

void CCrashReporterDlg::LimitPrompt(UINT uLimit)
{
	CString	str;
	str.Format(_T("最多只能输入%d个英文字符或%d个中文字符。"), uLimit, uLimit/2);
	::AfxMessageBox(str);
}

void CCrashReporterDlg::ChangeWndSize(BOOL bTiny)
{
	CRect	rtBoundary;
	if (bTiny)
		m_sttcTinyBoundary.GetWindowRect(&rtBoundary);
	else
		m_sttcLargeBoundary.GetWindowRect(&rtBoundary);

	CRect	rtWnd;
	GetWindowRect(&rtWnd);

	SetWindowPos(NULL, 0, 0, rtWnd.Width(), rtBoundary.top - rtWnd.top, SWP_NOZORDER | SWP_NOMOVE);

	//	如果是tiny模式，则禁示编辑框，以避免Tab键跳到这些框上。
	m_editDesc.EnableWindow(!bTiny);	
	m_editEmail.EnableWindow(!bTiny);
}

void CCrashReporterDlg::TriggerAutoClose()
{
	CString	strIniPathFile;
	strIniPathFile = _T("config\\preferences.ini");

	if (-1 == _taccess(strIniPathFile, 04))
		return;

	m_uRestartCount = GetPrivateProfileInt(_T("CrashReporter"), _T("RestartCount"), 0, strIniPathFile);
	time_t timeLastRestart = GetPrivateProfileInt(_T("CrashReporter"), _T("LastRestartTime"), 0, strIniPathFile);
	time_t timeCur = time(NULL);

	if (timeCur - timeLastRestart > 24*60*60)
		m_uRestartCount = 0;

	if (m_uRestartCount < 5)
	{
		m_uAutoCloseTimer = 1;
		SetTimer(m_uAutoCloseTimer, 3*60*1000, NULL);	//	5分钟后自动关闭程序并重启eMule（防止挂eMule时crash没人管）
	}
}

void CCrashReporterDlg::SaveAutoCloseIni()
{
	CString	strIniPathFile;
	strIniPathFile = _T("config\\preferences.ini");

	CString strValue;
	strValue.Format(_T("%d"), m_uRestartCount);
	WritePrivateProfileString(_T("CrashReporter"), _T("RestartCount"), strValue, strIniPathFile);
	strValue.Format(_T("%d"), time(NULL));
	WritePrivateProfileString(_T("CrashReporter"), _T("LastRestartTime"), strValue, strIniPathFile);
}

void CCrashReporterDlg::StopAutoCloseTimer()
{
	if (0 != m_uAutoCloseTimer)
	{
		KillTimer(m_uAutoCloseTimer);
		m_uAutoCloseTimer = 0;
	}
}


// CCrashReporterDlg 消息处理程序

BOOL CCrashReporterDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将\“关于...\”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	m_bAutoRestart = TRUE;
	UpdateData(FALSE);

	ChangeWndSize(m_bTinyWndSize);
	m_editDesc.SetLimitText(MAX_DESC - 10);
	m_editEmail.SetLimitText(MAX_PATH - 10);

	SetProcStatus(PS_NOT_SEND);

	TriggerAutoClose();	
	
	return TRUE;  // 除非设置了控件的焦点，否则返回 TRUE
}

void CCrashReporterDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CCrashReporterDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标显示。
HCURSOR CCrashReporterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CCrashReporterDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	//OnOK();

	if (PS_NOT_SEND != GetProcStatus())
		return;

	SetProcStatus(PS_SENDING);

	CString strEmail;
	m_editEmail.GetWindowText(strEmail);
	CString strDesc;
	m_editDesc.GetWindowText(strDesc);


	m_pUploadProc = (CHttpUploadFileProc*) ::AfxBeginThread(RUNTIME_CLASS(CHttpUploadFileProc),
																		THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);

	ZeroMemory(&m_pUploadProc->m_threadParam, sizeof(SHttpUploadFileThreadParam));
	_tcsncpy(m_pUploadProc->m_threadParam.szFileName, m_strFileName, MAX_PATH - 1);
	m_pUploadProc->m_threadParam.szFileName[MAX_PATH - 1] = _T('\0');
	m_pUploadProc->m_threadParam.hNotifyWnd = GetSafeHwnd();
	m_pUploadProc->m_threadParam.uNotifyMsg = WM_UPLOAD_PROGRESS;
	_tcsncpy(m_pUploadProc->m_threadParam.szEmail, strEmail, MAX_PATH - 1);
	m_pUploadProc->m_threadParam.szEmail[MAX_PATH - 1] = _T('\0');
	_tcsncpy(m_pUploadProc->m_threadParam.szDesc, strDesc, MAX_DESC - 1);
	m_pUploadProc->m_threadParam.szDesc[MAX_DESC - 1] = _T('\0');

	m_pUploadProc->ResumeThread();
}

LRESULT CCrashReporterDlg::OnUploadProgress(WPARAM wParam, LPARAM lParam)
{
	if (PS_SENDING != GetProcStatus())
		return 0;

	HRESULT hr = (HRESULT) wParam;

	if (SUCCEEDED(hr))
	{
		if (P_HUF_PROGRESS == hr)
		{
			m_progressSend.SetPos((int) lParam);
		}
		else
		{
			m_sttcProgrsStatus.SetWindowText(CHttpUploadFileProc::Result2Str(hr));
		}
	}

	if (S_HUF_FINISHED == hr)
	{
		SetProcStatus(PS_SUCCEEDED);
		//m_hProcThread = NULL;
		m_pUploadProc = NULL;
		::AfxMessageBox(_T("已成功发送错误报告。感谢您的支持！"));
		OnCancel();
	}

	if (FAILED(hr))
	{
		//m_hProcThread = NULL;
		m_pUploadProc = NULL;
		SetProcStatus(PS_NOT_SEND);
		CString	strErrorInfo;
		strErrorInfo = CHttpUploadFileProc::Result2Str(hr);
		m_sttcProgrsStatus.SetWindowText(strErrorInfo);
		::AfxMessageBox(strErrorInfo);
	}

	return 0;
}

void CCrashReporterDlg::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	//OnCancel();
}

void CCrashReporterDlg::OnBnClickedBncancel()
{
	// TODO: 在此添加控件通知处理程序代码
	if (PS_SENDING == GetProcStatus())
	{
		if (!::IsBadReadPtr(m_pUploadProc, sizeof(CHttpUploadFileProc)))
			m_pUploadProc->SuspendThread();

		if (IDYES == ::AfxMessageBox(_T("取消发送？"), MB_YESNO))
		{
			if (!::IsBadReadPtr(m_pUploadProc, sizeof(CHttpUploadFileProc)))
			{
				m_pUploadProc->Terminate();
			}
			SetProcStatus(PS_NOT_SEND);
		}

		if (!::IsBadReadPtr(m_pUploadProc, sizeof(CHttpUploadFileProc)))
			m_pUploadProc->ResumeThread();

	}
	else
	{
		OnCancel();
	}
}

void CCrashReporterDlg::OnEnMaxtextEditDesc()
{
	// TODO: 在此添加控件通知处理程序代码
	LimitPrompt(MAX_DESC - 10);
}

void CCrashReporterDlg::OnEnMaxtextEditEmail()
{
	// TODO: 在此添加控件通知处理程序代码
	LimitPrompt(MAX_PATH - 10);
}



void CCrashReporterDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	OnBnClickedBncancel();
	//CDialog::OnClose();
}

void CCrashReporterDlg::OnBnClickedMoreInfo()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bTinyWndSize = !m_bTinyWndSize;
	ChangeWndSize(m_bTinyWndSize);
}

void CCrashReporterDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: 在此处添加消息处理程序代码
	StopAutoCloseTimer();
	UpdateData();
}

void CCrashReporterDlg::OnTimer(UINT nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (m_uAutoCloseTimer == nIDEvent)
	{
		StopAutoCloseTimer();
		PostQuitMessage(0);

		m_uRestartCount++;
		SaveAutoCloseIni();
	}

	CDialog::OnTimer(nIDEvent);
}

BOOL CCrashReporterDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	// TODO: 在此添加专用代码和/或调用基类
	StopAutoCloseTimer();

	return CDialog::OnCommand(wParam, lParam);
}
