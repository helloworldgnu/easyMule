// CrashReporterDlg.h : 头文件
//

#pragma once
#include "afxwin.h"

#include "HttpUploadFileProc.h"

// CCrashReporterDlg 对话框
class CCrashReporterDlg : public CDialog
{
// 构造
public:
	CCrashReporterDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_CRASHREPORTER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

public:
	CEdit m_editEmail;
	CStatic m_sttcProgrsStatus;
	CProgressCtrl m_progressSend;
	CButton m_bnCancel;
	CButton m_bnOk;

	CString			m_strFileName;
	
	CHttpUploadFileProc	*m_pUploadProc;
protected:
	enum _ProcStatus
	{
		PS_NOT_SEND,
		PS_SENDING,
		PS_SUCCEEDED
	};
	int		m_iProcStatus;
	void	SetProcStatus(int iStatus);
	int		GetProcStatus(void){return m_iProcStatus;}

	void	LimitPrompt(UINT uLimit);
	void	ChangeWndSize(BOOL bTiny);

	BOOL	m_bTinyWndSize;
	
	UINT_PTR	m_uAutoCloseTimer;
	UINT		m_uRestartCount;
	void	TriggerAutoClose();
	void	SaveAutoCloseIni();

	void	StopAutoCloseTimer();

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg LRESULT OnUploadProgress(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedCancel();
	CEdit m_editDesc;
	afx_msg void OnEnMaxtextEditDesc();
	afx_msg void OnEnMaxtextEditEmail();
	afx_msg void OnBnClickedBncancel();
	afx_msg void OnClose();
	afx_msg void OnBnClickedMoreInfo();
	CStatic m_sttcTinyBoundary;
	CStatic m_sttcLargeBoundary;
	BOOL m_bAutoRestart;
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT nIDEvent);
protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
};
