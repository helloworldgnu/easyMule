#pragma once
#include "PPgDownloadSecurity.h"

// CScanDlg 对话框

class CScanDlg : public CDialog
{
	DECLARE_DYNAMIC(CScanDlg)

public:
	CScanDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CScanDlg();

// 对话框数据
	enum { IDD = IDD_SCAN };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();

protected:
	CPPgDownloadSecurity* m_pParent;

public:
	void Localize();
protected:
	void LoadSettings();
	virtual void OnOK();
};
