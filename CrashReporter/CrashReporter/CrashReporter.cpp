// CrashReporter.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include "CrashReporter.h"
#include "CrashReporterDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCrashReporterApp

BEGIN_MESSAGE_MAP(CCrashReporterApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()


// CCrashReporterApp 构造

CCrashReporterApp::CCrashReporterApp()
{
	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的一个 CCrashReporterApp 对象

CCrashReporterApp theApp;


// CCrashReporterApp 初始化

BOOL CCrashReporterApp::InitInstance()
{
	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControls()。否则，将无法创建窗口。
	InitCommonControls();

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO: 应适当修改该字符串，
	// 例如修改为公司或组织名
	SetRegistryKey(_T("应用程序向导生成的本地应用程序"));

	
	if (NULL == m_lpCmdLine
		|| _T('\0') == m_lpCmdLine[0])
		return FALSE;

	CCrashReporterDlg dlg;
	dlg.m_strFileName = m_lpCmdLine;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: 在此放置处理何时用“确定”来关闭
		//对话框的代码
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: 在此放置处理何时用“取消”来关闭
		//对话框的代码
	}

	if (dlg.m_bAutoRestart)
		ShellExecute(NULL, "open", "eMule.exe", NULL, NULL, SW_SHOW);

	if (!::IsBadReadPtr(dlg.m_pUploadProc, sizeof(CHttpUploadFileProc)))
		WaitForSingleObject(dlg.m_pUploadProc->m_hThread, 30000);

	//	删除错误报告
	DeleteFile(dlg.m_strFileName);

	// 由于对话框已关闭，所以将返回 FALSE 以便退出应用程序，
	// 而不是启动应用程序的消息泵。
	return FALSE;
}
