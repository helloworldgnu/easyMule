#pragma once

class CMemDump
{
public:
	CMemDump(void);
	~CMemDump(void);

	void	SetOutputFile(LPCTSTR lpszPathFile);	//	设置输出的日志文件，NULL表示输出到默认设备。
	void	Dump();									//	输出内存泄漏信息

	void	DumpIntoDifferentFile();				//	自动输出内存泄漏信息到不同的文件。
protected:
	CString	m_strPathFile;
};

extern CMemDump theMemDump;