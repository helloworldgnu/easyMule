#include "StdAfx.h"
#include ".\memdump.h"

CMemDump theMemDump;

CMemDump::CMemDump(void)
{
}

CMemDump::~CMemDump(void)
{
}

void CMemDump::SetOutputFile(LPCTSTR lpszPathFile)
{
	m_strPathFile = lpszPathFile;
}

void CMemDump::Dump()
{
	int		aiOldModes[3];
	_HFILE	ahOldFiles[3];
	CFile	file;

	if (!m_strPathFile.IsEmpty())
	{
		file.Open(m_strPathFile, CFile::modeCreate | CFile::modeReadWrite);

		aiOldModes[0] = _CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE );
		ahOldFiles[0] = _CrtSetReportFile( _CRT_WARN, file.m_hFile );
		aiOldModes[1] = _CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_FILE );
		ahOldFiles[1] = _CrtSetReportFile( _CRT_ERROR, file.m_hFile );
		aiOldModes[2] = _CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_FILE );
		ahOldFiles[2] = _CrtSetReportFile( _CRT_ASSERT, file.m_hFile );
	}

	_CrtDumpMemoryLeaks();

	if (!m_strPathFile.IsEmpty())
	{
		_CrtSetReportMode( _CRT_WARN, aiOldModes[0] );
		_CrtSetReportFile( _CRT_WARN, ahOldFiles[0] );
		_CrtSetReportMode( _CRT_ERROR, aiOldModes[1] );
		_CrtSetReportFile( _CRT_ERROR, ahOldFiles[1] );
		_CrtSetReportMode( _CRT_ASSERT, aiOldModes[2] );
		_CrtSetReportFile( _CRT_ASSERT, ahOldFiles[2] );

		file.Close();
	}
}

void CMemDump::DumpIntoDifferentFile()
{
	static int s_iDumpIndex = 0;
	s_iDumpIndex++;
	CString	str;
	str.Format(_T("memdump-%d.txt"), s_iDumpIndex);
	theMemDump.SetOutputFile(str);
	theMemDump.Dump();
	theMemDump.SetOutputFile(NULL);
}
