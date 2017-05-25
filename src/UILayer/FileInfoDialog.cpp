/* 
 * $Id: FileInfoDialog.cpp 4783 2008-02-02 08:17:12Z soarchin $
 * 
 * this file is part of eMule
 * Copyright (C)2002-2006 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "stdafx.h"
#include "eMule.h"
#include "FileInfoDialog.h"
#include "OtherFunctions.h"
#include "MediaInfo.h"
#include "PartFile.h"
#include "Preferences.h"
#include "UserMsgs.h"

// id3lib
#pragma warning(disable:4100) // unreferenced formal parameter
#include <id3/tag.h>
#include <id3/misc_support.h>
#pragma warning(default:4100) // unreferenced formal parameter

// MediaInfoDLL
/** @brief Kinds of Stream */
typedef enum _stream_t
{
    Stream_General,
    Stream_Video,
    Stream_Audio,
    Stream_Text,
    Stream_Chapters,
    Stream_Image,
    Stream_Max
} stream_t_C;

/** @brief Kinds of Info */
typedef enum _info_t
{
    Info_Name,
    Info_Text,
    Info_Measure,
    Info_Options,
    Info_Name_Text,
    Info_Measure_Text,
    Info_Info,
    Info_HowTo,
    Info_Max
} info_t_C;


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// SMediaInfoThreadResult

struct SMediaInfoThreadResult
{
	~SMediaInfoThreadResult() {
		delete paMediaInfo;
	}
	CArray<SMediaInfo>* paMediaInfo;
	CStringA strInfo;
};

/////////////////////////////////////////////////////////////////////////////
// CGetMediaInfoThread

class CGetMediaInfoThread : public CWinThread
{
	DECLARE_DYNCREATE(CGetMediaInfoThread)

protected:
	CGetMediaInfoThread()
	{
		m_hWndOwner = NULL;
	}

public:
	virtual BOOL InitInstance();
	virtual int	Run();
	void SetValues(HWND hWnd, const CSimpleArray<CObject*>* paFiles, HFONT hFont)
	{
		m_hWndOwner = hWnd;
		for (int i = 0; i < paFiles->GetSize(); i++)
			m_aFiles.Add(STATIC_DOWNCAST(CKnownFile, (*paFiles)[i]));
		m_hFont = hFont;
	}

private:
	bool GetMediaInfo(HWND hWndOwner, const CKnownFile* pFile, SMediaInfo* mi, bool bSingleFile);
	void WarnAboutWrongFileExtension(SMediaInfo* mi, LPCTSTR pszFileName, LPCTSTR pszExtensions);

	HWND m_hWndOwner;
	CSimpleArray<const CKnownFile*> m_aFiles;
	HFONT m_hFont;
};


/////////////////////////////////////////////////////////////////////////////
// CMediaInfoDLL

class CMediaInfoDLL
{
public:
	CMediaInfoDLL()
	{
		m_bInitialized = FALSE;
		m_hLib = NULL;
		m_ullVersion = 0;
		// MediaInfoLib - v0.4.0.1
		m_pfnMediaInfo4_Open = NULL;
		m_pfnMediaInfo4_Close = NULL;
		m_pfnMediaInfo4_Get = NULL;
		m_pfnMediaInfo4_Count_Get = NULL;
		// MediaInfoLib - v0.5 - v0.6.1
		m_pfnMediaInfo5_Open = NULL;
		// MediaInfoLib - v0.7+
		m_pfnMediaInfo_New = NULL;
		m_pfnMediaInfo_Delete = NULL;
		m_pfnMediaInfo_Open = NULL;
		m_pfnMediaInfo_Close = NULL;
		m_pfnMediaInfo_Get = NULL;
		m_pfnMediaInfo_Count_Get = NULL;
	}
	~CMediaInfoDLL()
	{
		if (m_hLib)
			FreeLibrary(m_hLib);
	}

	BOOL Initialize()
	{
		if (!m_bInitialized)
		{
			m_bInitialized = TRUE;

			CString strPath = theApp.GetProfileString(_T("eMule"), _T("MediaInfo_MediaInfoDllPath"), _T("MEDIAINFO.DLL"));
			m_hLib = LoadLibrary(strPath);
			if (m_hLib != NULL)
			{
				ULONGLONG ullVersion = GetModuleVersion(m_hLib);
				if (ullVersion == 0) // MediaInfoLib - v0.4.0.1 does not have a Win32 version info resource record
				{
					char* (__stdcall *fpMediaInfo4_Info_Version)();
					(FARPROC &)fpMediaInfo4_Info_Version = GetProcAddress(m_hLib, "MediaInfo_Info_Version");
					if (fpMediaInfo4_Info_Version)
					{
						char* pszVersion = (*fpMediaInfo4_Info_Version)();
						if (pszVersion && strcmp(pszVersion, "MediaInfoLib - v0.4.0.1 - http://mediainfo.sourceforge.net") == 0)
						{
							(FARPROC &)m_pfnMediaInfo4_Open = GetProcAddress(m_hLib, "MediaInfo_Open");
							(FARPROC &)m_pfnMediaInfo4_Close = GetProcAddress(m_hLib, "MediaInfo_Close");
							(FARPROC &)m_pfnMediaInfo4_Get = GetProcAddress(m_hLib, "MediaInfo_Get");
							(FARPROC &)m_pfnMediaInfo4_Count_Get = GetProcAddress(m_hLib, "MediaInfo_Count_Get");
							if (m_pfnMediaInfo4_Open && m_pfnMediaInfo4_Close && m_pfnMediaInfo4_Get) {
								m_ullVersion = MAKEDLLVERULL(0, 4, 0, 1);
								return TRUE;
							}
							m_pfnMediaInfo4_Open = NULL;
							m_pfnMediaInfo4_Close = NULL;
							m_pfnMediaInfo4_Get = NULL;
							m_pfnMediaInfo4_Count_Get = NULL;
						}
					}
				}
				// Note from MediaInfo developer
				// -----------------------------
				// Note : versioning method, for people who develop with LoadLibrary method
				// - if one of 2 first numbers change, there is no guaranties that the DLL is compatible with old one
				// - if one of 2 last numbers change, there is a garanty that the DLL is compatible with old one.
				// So you should test the version of the DLL, and if one of the 2 first numbers change, not load it.
				// ---
				// eMule currently handles v0.5.1.0, v0.6.0.0, v0.6.1.0
				else if (ullVersion >= MAKEDLLVERULL(0, 5, 0, 0) && ullVersion < MAKEDLLVERULL(0, 7, 0, 0))
				{
					// Don't use 'MediaInfo_Info_Version' with version v0.5+. This function is exported,
					// can be called, but does not return a valid version string..

					(FARPROC &)m_pfnMediaInfo5_Open = GetProcAddress(m_hLib, "MediaInfo_Open");
					(FARPROC &)m_pfnMediaInfo_Close = GetProcAddress(m_hLib, "MediaInfo_Close");
					(FARPROC &)m_pfnMediaInfo_Get = GetProcAddress(m_hLib, "MediaInfo_Get");
					(FARPROC &)m_pfnMediaInfo_Count_Get = GetProcAddress(m_hLib, "MediaInfo_Count_Get");
					if (m_pfnMediaInfo5_Open && m_pfnMediaInfo_Close && m_pfnMediaInfo_Get) {
						m_ullVersion = ullVersion;
						return TRUE;
					}
					m_pfnMediaInfo5_Open = NULL;
					m_pfnMediaInfo_Close = NULL;
					m_pfnMediaInfo_Get = NULL;
					m_pfnMediaInfo_Count_Get = NULL;
				}
				else if (ullVersion >= MAKEDLLVERULL(0, 7, 0, 0) && ullVersion < MAKEDLLVERULL(0, 8, 0, 0))
				{
					(FARPROC &)m_pfnMediaInfo_New = GetProcAddress(m_hLib, "MediaInfo_New");
					(FARPROC &)m_pfnMediaInfo_Delete = GetProcAddress(m_hLib, "MediaInfo_Delete");
					(FARPROC &)m_pfnMediaInfo_Open = GetProcAddress(m_hLib, "MediaInfo_Open");
					(FARPROC &)m_pfnMediaInfo_Close = GetProcAddress(m_hLib, "MediaInfo_Close");
					(FARPROC &)m_pfnMediaInfo_Get = GetProcAddress(m_hLib, "MediaInfo_Get");
					(FARPROC &)m_pfnMediaInfo_Count_Get = GetProcAddress(m_hLib, "MediaInfo_Count_Get");
					if (m_pfnMediaInfo_New && m_pfnMediaInfo_Delete && m_pfnMediaInfo_Open && m_pfnMediaInfo_Close && m_pfnMediaInfo_Get) {
						m_ullVersion = ullVersion;
						return TRUE;
					}
					m_pfnMediaInfo_New = NULL;
					m_pfnMediaInfo_Delete = NULL;
					m_pfnMediaInfo_Open = NULL;
					m_pfnMediaInfo_Close = NULL;
					m_pfnMediaInfo_Get = NULL;
					m_pfnMediaInfo_Count_Get = NULL;
				}
				FreeLibrary(m_hLib);
				m_hLib = NULL;
			}
		}
		return m_hLib != NULL;
	}

	ULONGLONG GetVersion() const
	{
		return m_ullVersion;
	}

	void* Open(LPCTSTR File)
	{
		if (m_pfnMediaInfo4_Open) {
			USES_CONVERSION;
			return (*m_pfnMediaInfo4_Open)(T2A(File));
		}
		else if (m_pfnMediaInfo5_Open)
			return (*m_pfnMediaInfo5_Open)(File);
		else if (m_pfnMediaInfo_New) {
			void* Handle = (*m_pfnMediaInfo_New)();
			if (Handle)
				(*m_pfnMediaInfo_Open)(Handle, File);
			return Handle;
		}
		return NULL;
	}

	void Close(void* Handle)
	{
		if (m_pfnMediaInfo_Delete)
			(*m_pfnMediaInfo_Delete)(Handle);	// File is automaticly closed
		else if (m_pfnMediaInfo4_Close)
			(*m_pfnMediaInfo4_Close)(Handle);
		else if (m_pfnMediaInfo_Close)
			(*m_pfnMediaInfo_Close)(Handle);
	}

	CString Get(void* Handle, stream_t_C StreamKind, int StreamNumber, LPCTSTR Parameter, info_t_C KindOfInfo, info_t_C KindOfSearch)
	{
		if (m_pfnMediaInfo4_Get) {
			USES_CONVERSION;
			return CString((*m_pfnMediaInfo4_Get)(Handle, StreamKind, StreamNumber, T2A(Parameter), KindOfInfo, KindOfSearch));
		}
		else if (m_pfnMediaInfo_Get) {
			CString strNewParameter(Parameter);
			if (m_ullVersion >= MAKEDLLVERULL(0, 7, 1, 0)) {
				// Convert old tags to new tags
				strNewParameter.Replace(_T('_'), _T('/'));

				// Workaround for a bug in MediaInfoLib
				if (strNewParameter == _T("Channels"))
					strNewParameter = _T("Channel(s)");
			}
			return (*m_pfnMediaInfo_Get)(Handle, StreamKind, StreamNumber, strNewParameter, KindOfInfo, KindOfSearch);
		}
		return _T("");
	}

	int Count_Get(void* Handle, stream_t_C StreamKind, int StreamNumber)
	{
		if (m_pfnMediaInfo4_Get)
			return (*m_pfnMediaInfo4_Count_Get)(Handle, StreamKind, StreamNumber);
		else if (m_pfnMediaInfo_Count_Get)
			return (*m_pfnMediaInfo_Count_Get)(Handle, StreamKind, StreamNumber);
		return 0;
	}

protected:
	ULONGLONG m_ullVersion;
	BOOL m_bInitialized;
	HINSTANCE m_hLib;

	// MediaInfoLib - v0.4.0.1
	void* (__stdcall *m_pfnMediaInfo4_Open)(char* File) throw(...);
	void  (__stdcall *m_pfnMediaInfo4_Close)(void* Handle) throw(...);
	char* (__stdcall *m_pfnMediaInfo4_Get)(void* Handle, stream_t_C StreamKind, int StreamNumber, char* Parameter, info_t_C KindOfInfo, info_t_C KindOfSearch) throw(...);
	int   (__stdcall *m_pfnMediaInfo4_Count_Get)(void* Handle, stream_t_C StreamKind, int StreamNumber) throw(...);

	// MediaInfoLib - v0.5+
	void*			(__stdcall *m_pfnMediaInfo5_Open)(const wchar_t* File) throw(...);
	void			(__stdcall *m_pfnMediaInfo_Close)(void* Handle) throw(...);
	const wchar_t*	(__stdcall *m_pfnMediaInfo_Get)(void* Handle, stream_t_C StreamKind, int StreamNumber, const wchar_t* Parameter, info_t_C KindOfInfo, info_t_C KindOfSearch) throw(...);
	int				(__stdcall *m_pfnMediaInfo_Count_Get)(void* Handle, stream_t_C StreamKind, int StreamNumber) throw(...);

	// MediaInfoLib - v0.7+
	int				(__stdcall *m_pfnMediaInfo_Open)(void* Handle, const wchar_t* File) throw(...);
	void*			(__stdcall *m_pfnMediaInfo_New)() throw(...);
	void			(__stdcall *m_pfnMediaInfo_Delete)(void* Handle) throw(...);
};

CMediaInfoDLL theMediaInfoDLL;


/////////////////////////////////////////////////////////////////////////////
// CFileInfoDialog dialog

IMPLEMENT_DYNAMIC(CFileInfoDialog, CResizablePage)

BEGIN_MESSAGE_MAP(CFileInfoDialog, CResizablePage)
	ON_MESSAGE(UM_MEDIA_INFO_RESULT, OnMediaInfoResult)
	ON_MESSAGE(UM_DATA_CHANGED, OnDataChanged)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

CFileInfoDialog::CFileInfoDialog()
	: CResizablePage(CFileInfoDialog::IDD, 0)
{
	m_paFiles = NULL;
	m_bDataChanged = false;
	m_strCaption = GetResString(IDS_CONTENT_INFO);
	m_psp.pszTitle = m_strCaption;
	m_psp.dwFlags |= PSP_USETITLE;
}

CFileInfoDialog::~CFileInfoDialog()
{
}

BOOL CFileInfoDialog::OnInitDialog()
{
	CWaitCursor curWait; // we may get quite busy here..
	ReplaceRichEditCtrl(GetDlgItem(IDC_FULL_FILE_INFO), this, GetDlgItem(IDC_FD_XI1)->GetFont());
	CResizablePage::OnInitDialog();
	InitWindowStyles(this);
	AddAnchor(IDC_FULL_FILE_INFO, TOP_LEFT, BOTTOM_RIGHT);

	m_fi.LimitText(0x7FFFFFFF);
	m_fi.SendMessage(EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELONG(3, 3));
	m_fi.SetAutoURLDetect();
	m_fi.SetEventMask(m_fi.GetEventMask() | ENM_LINK);

	CResizablePage::UpdateData(FALSE);
	Localize();
	return TRUE;
}

void CFileInfoDialog::OnDestroy()
{
	// This property sheet's window may get destroyed and re-created several times although
	// the corresponding C++ class is kept -> explicitly reset ResizeableLib state
	RemoveAllAnchors();

	CResizablePage::OnDestroy();
}

BOOL CFileInfoDialog::OnSetActive()
{
	if (!CResizablePage::OnSetActive())
		return FALSE;
	if (m_bDataChanged)
	{
		CString strWait = GetResString(IDS_FSTAT_WAITING);
		SetDlgItemText(IDC_FORMAT, strWait);
		SetDlgItemText(IDC_FILESIZE, strWait);
		SetDlgItemText(IDC_LENGTH, strWait);
		SetDlgItemText(IDC_VCODEC, strWait);
		SetDlgItemText(IDC_VBITRATE, strWait);
		SetDlgItemText(IDC_VWIDTH, strWait);
		SetDlgItemText(IDC_VASPECT, strWait);
		SetDlgItemText(IDC_VFPS, strWait);
		SetDlgItemText(IDC_ACODEC, strWait);
		SetDlgItemText(IDC_ACHANNEL, strWait);
		SetDlgItemText(IDC_ASAMPLERATE, strWait);
		SetDlgItemText(IDC_ABITRATE, strWait);
		SetDlgItemText(IDC_ALANGUAGE, strWait);
		SetDlgItemText(IDC_FULL_FILE_INFO, strWait);

		CGetMediaInfoThread* pThread = (CGetMediaInfoThread*)AfxBeginThread(RUNTIME_CLASS(CGetMediaInfoThread), THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
		if (pThread)
		{
			pThread->SetValues(m_hWnd, m_paFiles, (HFONT)GetDlgItem(IDC_FD_XI1)->GetFont()->m_hObject);
			pThread->ResumeThread();
		}
		m_bDataChanged = false;
	}
	return TRUE;
}

LRESULT CFileInfoDialog::OnDataChanged(WPARAM, LPARAM)
{
	m_bDataChanged = true;
	return 1;
}

IMPLEMENT_DYNCREATE(CGetMediaInfoThread, CWinThread)

BOOL CGetMediaInfoThread::InitInstance()
{
	DbgSetThreadName("GetMediaInfo");
	InitThreadLocale();
	return TRUE;
}

int CGetMediaInfoThread::Run()
{
	CoInitialize(NULL);

	HWND hwndRE = CreateWindow(RICHEDIT_CLASS, _T(""), ES_MULTILINE | ES_READONLY | WS_DISABLED, 0, 0, 200, 200, NULL, NULL, NULL, NULL);
	ASSERT( hwndRE );
	if (hwndRE && m_hFont)
		::SendMessage(hwndRE, WM_SETFONT, (WPARAM)m_hFont, 0);

	CArray<SMediaInfo>* paMediaInfo = new CArray<SMediaInfo>;
	try
	{
		CRichEditStream re;
		re.Attach(hwndRE);
		re.LimitText(0x7FFFFFFF);
		PARAFORMAT pf = {0};
		pf.cbSize = sizeof pf;
		if (re.GetParaFormat(pf)) {
			pf.dwMask |= PFM_TABSTOPS;
			pf.cTabCount = 1;
			pf.rgxTabs[0] = 3000;
			re.SetParaFormat(pf);
		}
		re.Detach();

		for (int i = 0; i < m_aFiles.GetSize(); i++)
		{
			SMediaInfo mi;
			mi.bOutputFileName = m_aFiles.GetSize() > 1;
			mi.strFileName = m_aFiles[i]->GetFileName();
			mi.strInfo.Attach(hwndRE);
			mi.strInfo.InitColors();
			if (IsWindow(m_hWndOwner) && GetMediaInfo(m_hWndOwner, m_aFiles[i], &mi, m_aFiles.GetSize() == 1)) {
				mi.strInfo.Detach();
				paMediaInfo->Add(mi);
			}
			else {
				mi.strInfo.Detach();
				delete paMediaInfo;
				paMediaInfo = NULL;
				break;
			}
		}
	}
	catch(...)
	{
		ASSERT(0);
	}

	SMediaInfoThreadResult* pThreadRes = new SMediaInfoThreadResult;
	pThreadRes->paMediaInfo = paMediaInfo;
	CRichEditStream re;
	re.Attach(hwndRE);
	re.GetRTFText(pThreadRes->strInfo);
	re.Detach();
	VERIFY( DestroyWindow(hwndRE) );

	// Usage of 'PostMessage': The idea is to post a message to the window in that other
	// thread and never deadlock (because of the post). This is safe, but leads to the problem
	// that we may create memory leaks in case the target window is currently in the process
	// of getting destroyed! E.g. if the target window gets destroyed after we put the message
	// into the queue, we have no chance of determining that and the memory wouldn't get freed.
	//if (!IsWindow(m_hWndOwner) || !PostMessage(m_hWndOwner, UM_MEDIA_INFO_RESULT, 0, (LPARAM)pThreadRes))
	//	delete pThreadRes;

	// Usage of 'SendMessage': Using 'SendMessage' seems to be dangerous because of potential
	// deadlocks. Basically it depends on what the target thread/window is currently doing 
	// whether there is a risk for a deadlock. However, even with extensive stress testing
	// there does not show any problem. The worse thing which can happen, is that we call
	// 'SendMessage', then the target window gets destroyed (while we are still waiting in
	// 'SendMessage') and would get blocked. Though, this does not happen, it seems that Windows
	// is catching that case internally and lets our 'SendMessage' call return (with a result
	// of '0'). If that happened, the 'IsWindow(m_hWndOwner)' returns FALSE, which positively
	// indicates that the target window was destroyed while we were waiting in 'SendMessage'.
	// So, everything should be fine (with that special scenario) with using 'SendMessage'.
	// Let's be brave. :)
	if (!IsWindow(m_hWndOwner) || !SendMessage(m_hWndOwner, UM_MEDIA_INFO_RESULT, 0, (LPARAM)pThreadRes))
		delete pThreadRes;

	CoUninitialize();
	return 0;
}

LRESULT CFileInfoDialog::OnMediaInfoResult(WPARAM, LPARAM lParam)
{
	SetDlgItemText(IDC_FD_XI3, GetResString(IDS_VIDEO));
	SetDlgItemText(IDC_FD_XI4, GetResString(IDS_AUDIO));
	SetDlgItemText(IDC_FORMAT, _T("-"));
	SetDlgItemText(IDC_FILESIZE, _T("-"));
	SetDlgItemText(IDC_LENGTH, _T("-"));
	SetDlgItemText(IDC_VCODEC, _T("-"));
	SetDlgItemText(IDC_VBITRATE, _T("-"));
	SetDlgItemText(IDC_VWIDTH, _T("-"));
	SetDlgItemText(IDC_VASPECT, _T("-"));
	SetDlgItemText(IDC_VFPS, _T("-"));
	SetDlgItemText(IDC_ACODEC, _T("-"));
	SetDlgItemText(IDC_ACHANNEL, _T("-"));
	SetDlgItemText(IDC_ASAMPLERATE, _T("-"));
	SetDlgItemText(IDC_ABITRATE, _T("-"));
	SetDlgItemText(IDC_ALANGUAGE, _T("-"));
	SetDlgItemText(IDC_FULL_FILE_INFO, _T(""));

	SMediaInfoThreadResult* pThreadRes = (SMediaInfoThreadResult*)lParam;
	if (pThreadRes == NULL)
		return 1;
	CArray<SMediaInfo>* paMediaInfo = pThreadRes->paMediaInfo;
	if (paMediaInfo == NULL) {
		delete pThreadRes;
		return 1;
	}

	if (paMediaInfo->GetSize() != m_paFiles->GetSize())
	{
		SetDlgItemText(IDC_FORMAT, _T(""));
		SetDlgItemText(IDC_FILESIZE, _T(""));
		SetDlgItemText(IDC_LENGTH, _T(""));
		SetDlgItemText(IDC_VCODEC, _T(""));
		SetDlgItemText(IDC_VBITRATE, _T(""));
		SetDlgItemText(IDC_VWIDTH, _T(""));
		SetDlgItemText(IDC_VASPECT, _T(""));
		SetDlgItemText(IDC_VFPS, _T(""));
		SetDlgItemText(IDC_ACODEC, _T(""));
		SetDlgItemText(IDC_ACHANNEL, _T(""));
		SetDlgItemText(IDC_ASAMPLERATE, _T(""));
		SetDlgItemText(IDC_ABITRATE, _T(""));
		SetDlgItemText(IDC_ALANGUAGE, _T(""));
		SetDlgItemText(IDC_FULL_FILE_INFO, _T(""));
		delete pThreadRes;
		return 1;
	}

	uint64 uTotalFileSize = 0;
	SMediaInfo ami;
	bool bDiffVideoStreamCount = false;
	bool bDiffVideoCompression = false;
	bool bDiffVideoWidth = false;
	bool bDiffVideoHeight = false;
	bool bDiffVideoFrameRate = false;
	bool bDiffVideoBitRate = false;
	bool bDiffVideoAspectRatio = false;
	bool bDiffAudioStreamCount = false;
	bool bDiffAudioCompression = false;
	bool bDiffAudioChannels = false;
	bool bDiffAudioSamplesPerSec = false;
	bool bDiffAudioAvgBytesPerSec = false;
	bool bDiffAudioLanguage = false;
	for (int i = 0; i < paMediaInfo->GetSize(); i++)
	{
		const SMediaInfo& mi = paMediaInfo->GetAt(i);

		uTotalFileSize += (uint64)mi.ulFileSize;
		if (i == 0)
		{
			ami = mi;
		}
		else
		{
			if (ami.strFileFormat != mi.strFileFormat)
				ami.strFileFormat.Empty();

			if (ami.strMimeType != mi.strMimeType)
				ami.strMimeType.Empty();

			ami.fVideoLengthSec += mi.fVideoLengthSec;
			if (mi.bVideoLengthEstimated)
				ami.bVideoLengthEstimated = true;
			if (ami.iVideoStreams == 0 && mi.iVideoStreams > 0 || ami.iVideoStreams > 0 && mi.iVideoStreams == 0)
			{
				if (ami.iVideoStreams == 0)
					ami.iVideoStreams = mi.iVideoStreams;
				bDiffVideoStreamCount = true;
				bDiffVideoCompression = true;
				bDiffVideoWidth = true;
				bDiffVideoHeight = true;
				bDiffVideoFrameRate = true;
				bDiffVideoBitRate = true;
				bDiffVideoAspectRatio = true;
			}
			else
			{
				if (ami.iVideoStreams != mi.iVideoStreams)
					bDiffVideoStreamCount = true;
				if (ami.strVideoFormat != mi.strVideoFormat)
					bDiffVideoCompression = true;
				if (ami.video.bmiHeader.biWidth != mi.video.bmiHeader.biWidth)
					bDiffVideoWidth = true;
				if (ami.video.bmiHeader.biHeight != mi.video.bmiHeader.biHeight)
					bDiffVideoHeight = true;
				if (ami.fVideoFrameRate != mi.fVideoFrameRate)
					bDiffVideoFrameRate = true;
				if (ami.video.dwBitRate != mi.video.dwBitRate)
					bDiffVideoBitRate = true;
				if (ami.fVideoAspectRatio != mi.fVideoAspectRatio)
					bDiffVideoAspectRatio = true;
			}

			ami.fAudioLengthSec += mi.fAudioLengthSec;
			if (mi.bAudioLengthEstimated)
				ami.bAudioLengthEstimated = true;
			if (ami.iAudioStreams == 0 && mi.iAudioStreams > 0 || ami.iAudioStreams > 0 && mi.iAudioStreams == 0)
			{
				if (ami.iAudioStreams == 0)
					ami.iAudioStreams = mi.iAudioStreams;
				bDiffAudioStreamCount = true;
				bDiffAudioCompression = true;
				bDiffAudioChannels = true;
				bDiffAudioSamplesPerSec = true;
				bDiffAudioAvgBytesPerSec = true;
				bDiffAudioLanguage = true;
			}
			else
			{
				if (ami.iAudioStreams != mi.iAudioStreams)
					bDiffAudioStreamCount = true;
				if (ami.strAudioFormat != mi.strAudioFormat)
					bDiffAudioCompression = true;
				if (ami.audio.nChannels != mi.audio.nChannels)
					bDiffAudioChannels = true;
				if (ami.audio.nSamplesPerSec != mi.audio.nSamplesPerSec)
					bDiffAudioSamplesPerSec = true;
				if (ami.audio.nAvgBytesPerSec != mi.audio.nAvgBytesPerSec)
					bDiffAudioAvgBytesPerSec = true;
				if (ami.strAudioLanguage.CompareNoCase(mi.strAudioLanguage) != 0)
					bDiffAudioLanguage = true;
			}
		}
	}

	CString buffer;

	buffer = ami.strFileFormat;
	if (!ami.strMimeType.IsEmpty())
	{
		if (!buffer.IsEmpty())
			buffer += _T("; MIME type=");
		buffer.AppendFormat(_T("%s"), ami.strMimeType);
	}
	SetDlgItemText(IDC_FORMAT, buffer);

	if (uTotalFileSize)
		SetDlgItemText(IDC_FILESIZE, CastItoXBytes(uTotalFileSize, false, false));
	if (ami.fVideoLengthSec) {
		CString strLength(CastSecondsToHM((time_t)ami.fVideoLengthSec));
		if (ami.bVideoLengthEstimated)
			strLength += _T(" (")+GetResString(IDS_ESTIMATED)+ _T(")");
		SetDlgItemText(IDC_LENGTH, strLength);
	}
	else if (ami.fAudioLengthSec) {
		CString strLength(CastSecondsToHM((time_t)ami.fAudioLengthSec));
		if (ami.bAudioLengthEstimated)
			strLength += _T(" (")+GetResString(IDS_ESTIMATED)+ _T(")");
		SetDlgItemText(IDC_LENGTH, strLength);
	}

	if (ami.iVideoStreams)
	{
		if (!bDiffVideoStreamCount && ami.iVideoStreams > 1)
			SetDlgItemText(IDC_FD_XI3, GetResString(IDS_VIDEO) + _T(" #1"));
		else
			SetDlgItemText(IDC_FD_XI3, GetResString(IDS_VIDEO));

		if (!bDiffVideoCompression && !ami.strVideoFormat.IsEmpty())
			SetDlgItemText(IDC_VCODEC, ami.strVideoFormat);
		else
			SetDlgItemText(IDC_VCODEC, _T(""));

		if (!bDiffVideoBitRate && ami.video.dwBitRate)
		{
			if (ami.video.dwBitRate == (DWORD)-1)
				buffer = _T("Variable");
			else
				buffer.Format(_T("%u %s"), (ami.video.dwBitRate + 500) / 1000, GetResString(IDS_KBITSSEC));
			SetDlgItemText(IDC_VBITRATE, buffer);
		}
		else
			SetDlgItemText(IDC_VBITRATE, _T(""));

		buffer.Empty();
		if (!bDiffVideoWidth && ami.video.bmiHeader.biWidth && !bDiffVideoHeight && ami.video.bmiHeader.biHeight)
		{
			buffer.AppendFormat(_T("%u x %u"), abs(ami.video.bmiHeader.biWidth), abs(ami.video.bmiHeader.biHeight));
			SetDlgItemText(IDC_VWIDTH, buffer);
		}
		else
			SetDlgItemText(IDC_VWIDTH, _T(""));

		if (!bDiffVideoAspectRatio && ami.fVideoAspectRatio)
		{
			buffer.Format(_T("%.3f"), ami.fVideoAspectRatio);
			CString strAR = GetKnownAspectRatioDisplayString((float)ami.fVideoAspectRatio);
			if (!strAR.IsEmpty())
				buffer.AppendFormat(_T("  (%s)"), strAR);
			SetDlgItemText(IDC_VASPECT, buffer);
		}
		else
			SetDlgItemText(IDC_VASPECT, _T(""));

		if (!bDiffVideoFrameRate && ami.fVideoFrameRate)
		{
			buffer.Format(_T("%.2f"), ami.fVideoFrameRate);
			SetDlgItemText(IDC_VFPS, buffer);
		}
		else
			SetDlgItemText(IDC_VFPS, _T(""));
	}

	if (ami.iAudioStreams)
	{
		if (!bDiffAudioStreamCount && ami.iAudioStreams > 1)
			SetDlgItemText(IDC_FD_XI4, GetResString(IDS_AUDIO) + _T(" #1"));
		else
			SetDlgItemText(IDC_FD_XI4, GetResString(IDS_AUDIO));

		if (!bDiffAudioCompression && !ami.strAudioFormat.IsEmpty())
			SetDlgItemText(IDC_ACODEC, ami.strAudioFormat);
		else
			SetDlgItemText(IDC_ACODEC, _T(""));

		if (!bDiffAudioChannels && ami.audio.nChannels)
		{
			switch (ami.audio.nChannels)
			{
				case 1:
					SetDlgItemText(IDC_ACHANNEL, _T("1 (Mono)"));
					break;
				case 2:
					SetDlgItemText(IDC_ACHANNEL, _T("2 (Stereo)"));
					break;
				case 5:
					SetDlgItemText(IDC_ACHANNEL, _T("5.1 (Surround)"));
					break;
				default:
					SetDlgItemInt(IDC_ACHANNEL, ami.audio.nChannels, FALSE);
					break;
			}
		}
		else
			SetDlgItemText(IDC_ACHANNEL, _T(""));

		if (!bDiffAudioSamplesPerSec && ami.audio.nSamplesPerSec)
		{
			buffer.Format(_T("%.3f kHz"), ami.audio.nSamplesPerSec / 1000.0);
			SetDlgItemText(IDC_ASAMPLERATE, buffer);
		}
		else
			SetDlgItemText(IDC_ASAMPLERATE, _T(""));

		if (!bDiffAudioAvgBytesPerSec && ami.audio.nAvgBytesPerSec)
		{
			if (ami.audio.nAvgBytesPerSec == (DWORD)-1)
				buffer = _T("Variable");
			else
				buffer.Format(_T("%u %s"), (UINT)((ami.audio.nAvgBytesPerSec * 8) / 1000.0 + 0.5), GetResString(IDS_KBITSSEC));
			SetDlgItemText(IDC_ABITRATE, buffer);
		}
		else
			SetDlgItemText(IDC_ABITRATE, _T(""));

		if (!bDiffAudioLanguage && !ami.strAudioLanguage.IsEmpty())
			SetDlgItemText(IDC_ALANGUAGE, ami.strAudioLanguage);
		else
			SetDlgItemText(IDC_ALANGUAGE, _T(""));
	}

	m_fi.SetRTFText(pThreadRes->strInfo);
	m_fi.SetSel(0, 0);

	delete pThreadRes;
	return 1;
}

void CGetMediaInfoThread::WarnAboutWrongFileExtension(SMediaInfo* mi, LPCTSTR pszFileName, LPCTSTR pszExtensions)
{
	if (!mi->strInfo.IsEmpty())
		mi->strInfo << _T("\r\n");
	mi->strInfo.SetSelectionCharFormat(mi->strInfo.m_cfRed);
	mi->strInfo.AppendFormat(GetResString(IDS_WARNING_WRONGFILEEXTENTION), pszFileName, pszExtensions);
	mi->strInfo.SetSelectionCharFormat(mi->strInfo.m_cfDef);
}

bool CGetMediaInfoThread::GetMediaInfo(HWND hWndOwner, const CKnownFile* pFile, SMediaInfo* mi, bool bSingleFile)
{
	if (!pFile)
		return false;
	ASSERT( !pFile->GetFilePath().IsEmpty() );

	bool bHasDRM = false;
	if (!pFile->IsPartFile() || ((CPartFile*)pFile)->IsComplete(0, 1024, true)) {
		GetMimeType(pFile->GetFilePath(), mi->strMimeType);
		bHasDRM = GetDRM(pFile->GetFilePath());
		if (bHasDRM) {
			if (!mi->strInfo.IsEmpty())
				mi->strInfo << _T("\r\n");
			mi->strInfo.SetSelectionCharFormat(mi->strInfo.m_cfRed);
			mi->strInfo.AppendFormat(GetResString(IDS_MEDIAINFO_DRMWARNING), pFile->GetFileName());
			mi->strInfo.SetSelectionCharFormat(mi->strInfo.m_cfDef);
		}
	}

	mi->ulFileSize = pFile->GetFileSize();

	bool bFoundHeader = false;
	if (pFile->IsPartFile())
	{
		// Do *not* pass a part file which does not have the beginning of file to the following code.
		//	- The MP3 reading code will skip all 0-bytes from the beginning of the file and may block
		//	  the main thread for a long time.
		//
		//	- The RIFF reading code will not work without the file header.
		//
		//	- Most (if not all) other code will also not work without the beginning of the file available.
		if (!((CPartFile*)pFile)->IsComplete(0, 16*1024, true))
			return bFoundHeader || !mi->strMimeType.IsEmpty();
	}
	else
	{
		if (pFile->GetFileSize() < (uint64)16*1024)
			return bFoundHeader || !mi->strMimeType.IsEmpty();
	}

	TCHAR szExt[_MAX_EXT];
	_tsplitpath(pFile->GetFileName(), NULL, NULL, NULL, szExt);
	_tcslwr(szExt);

	////////////////////////////////////////////////////////////////////////////
	// Check for AVI file
	//
	bool bIsAVI = false;
	if (theApp.GetProfileInt(_T("eMule"), _T("MediaInfo_RIFF"), 1))
	{
		try
		{
			if (GetRIFFHeaders(pFile->GetFilePath(), mi, bIsAVI, true))
			{
				if (bIsAVI && _tcscmp(szExt, _T(".avi")) != 0)
					WarnAboutWrongFileExtension(mi, pFile->GetFileName(), _T("avi"));
				return true;
			}
		}
		catch(...)
		{
			ASSERT(0);
		}
	}

	if (!IsWindow(hWndOwner))
		return false;

	////////////////////////////////////////////////////////////////////////////
	// Check for MPEG Audio file
	//
	if (theApp.GetProfileInt(_T("eMule"), _T("MediaInfo_ID3LIB"), 1) &&
		(_tcscmp(szExt, _T(".mp3"))==0 || _tcscmp(szExt, _T(".mp2"))==0 || _tcscmp(szExt, _T(".mp1"))==0 || _tcscmp(szExt, _T(".mpa"))==0))
	{
		try
		{
			USES_CONVERSION;
			ID3_Tag myTag;
			myTag.Link(T2CA(pFile->GetFilePath()));

			const Mp3_Headerinfo* mp3info;
			mp3info = myTag.GetMp3HeaderInfo();
			if (mp3info)
			{
				mi->strFileFormat = _T("MPEG audio");

				mi->OutputFileName();
				if (!bSingleFile) {
					mi->strInfo.SetSelectionCharFormat(mi->strInfo.m_cfBold);
					mi->strInfo << _T("MP3 Header Info\n");
				}

				switch (mp3info->version)
				{
				case MPEGVERSION_2_5:
					mi->strAudioFormat = _T("MPEG-2.5,");
					mi->audio.wFormatTag = 0x0055;
					break;
				case MPEGVERSION_2:
					mi->strAudioFormat = _T("MPEG-2,");
					mi->audio.wFormatTag = 0x0055;
					break;
				case MPEGVERSION_1:
					mi->strAudioFormat = _T("MPEG-1,");
					mi->audio.wFormatTag = 0x0055;
					break;
				default:
					break;
				}
				mi->strAudioFormat += _T(" ");

				switch (mp3info->layer)
				{
				case MPEGLAYER_III:
					mi->strAudioFormat += _T("Layer 3");
					break;
				case MPEGLAYER_II:
					mi->strAudioFormat += _T("Layer 2");
					break;
				case MPEGLAYER_I:
					mi->strAudioFormat += _T("Layer 1");
					break;
				default:
					break;
				}
				if (!bSingleFile)
				{
					mi->strInfo << _T("   ") << GetResString(IDS_CODEC) << _T(":\t") << mi->strAudioFormat << _T("\n");
					mi->strInfo << _T("   ") << GetResString(IDS_BITRATE) << _T(":\t") << ((mp3info->vbr_bitrate ? mp3info->vbr_bitrate : mp3info->bitrate) + 500) / 1000 << _T(" ") << GetResString(IDS_KBITSSEC) << _T("\n");
					mi->strInfo << _T("   ") << GetResString(IDS_SAMPLERATE) << _T(":\t") << mp3info->frequency / 1000.0 << _T(" kHz\n");
				}

				mi->iAudioStreams++;
				mi->audio.nAvgBytesPerSec = mp3info->vbr_bitrate ? mp3info->vbr_bitrate/8 : mp3info->bitrate/8;
				mi->audio.nSamplesPerSec = mp3info->frequency;

				if (!bSingleFile)
					mi->strInfo << _T("   Mode:\t");
				switch (mp3info->channelmode){
				case MP3CHANNELMODE_STEREO:
					if (!bSingleFile)
						mi->strInfo << _T("Stereo");
					mi->audio.nChannels = 2;
					break;
				case MP3CHANNELMODE_JOINT_STEREO:
					if (!bSingleFile)
						mi->strInfo << _T("Joint Stereo");
					mi->audio.nChannels = 2;
					break;
				case MP3CHANNELMODE_DUAL_CHANNEL:
					if (!bSingleFile)
						mi->strInfo << _T("Dual Channel");
					mi->audio.nChannels = 2;
					break;
				case MP3CHANNELMODE_SINGLE_CHANNEL:
					if (!bSingleFile)
						mi->strInfo << _T("Mono");
					mi->audio.nChannels = 1;
					break;
				}
				if (!bSingleFile)
					mi->strInfo << _T("\n");

				// length
				if (mp3info->time)
				{
					if (!bSingleFile)
					{
						CString strLength;
						SecToTimeLength(mp3info->time, strLength);
						mi->strInfo << _T("   ") << GetResString(IDS_LENGTH) << _T(":\t") << strLength;
						if (pFile->IsPartFile()){
							mi->strInfo.SetSelectionCharFormat(mi->strInfo.m_cfRed);
							mi->strInfo << _T(" (estimated)");
							mi->strInfo.SetSelectionCharFormat(mi->strInfo.m_cfDef);
						}
						mi->strInfo << "\n";
					}
					mi->fAudioLengthSec = mp3info->time;
					if (pFile->IsPartFile())
						mi->bAudioLengthEstimated = true;
				}

				bFoundHeader = true;
				if (mi->strMimeType.IsEmpty())
					mi->strMimeType = _T("audio/mpeg");
			}

			int iTag = 0;
			ID3_Tag::Iterator* iter = myTag.CreateIterator();
			const ID3_Frame* frame;
			while ((frame = iter->GetNext()) != NULL)
			{
				if (iTag == 0)
				{
					if (mp3info && !bSingleFile)
						mi->strInfo << _T("\n");
					mi->strInfo.SetSelectionCharFormat(mi->strInfo.m_cfBold);
					mi->strInfo << _T("MP3 Tags\n");
				}
				iTag++;

				LPCSTR desc = frame->GetDescription();
				if (!desc)
					desc = frame->GetTextID();

				CStringStream strFidInfo;
				ID3_FrameID eFrameID = frame->GetID();
				switch (eFrameID)
				{
				case ID3FID_ALBUM:
				case ID3FID_COMPOSER:
				case ID3FID_CONTENTTYPE:
				case ID3FID_COPYRIGHT:
				case ID3FID_DATE:
				case ID3FID_PLAYLISTDELAY:
				case ID3FID_ENCODEDBY:
				case ID3FID_LYRICIST:
				case ID3FID_FILETYPE:
				case ID3FID_TIME:
				case ID3FID_CONTENTGROUP:
				case ID3FID_TITLE:
				case ID3FID_SUBTITLE:
				case ID3FID_INITIALKEY:
				case ID3FID_LANGUAGE:
				case ID3FID_MEDIATYPE:
				case ID3FID_ORIGALBUM:
				case ID3FID_ORIGFILENAME:
				case ID3FID_ORIGLYRICIST:
				case ID3FID_ORIGARTIST:
				case ID3FID_ORIGYEAR:
				case ID3FID_FILEOWNER:
				case ID3FID_LEADARTIST:
				case ID3FID_BAND:
				case ID3FID_CONDUCTOR:
				case ID3FID_MIXARTIST:
				case ID3FID_PARTINSET:
				case ID3FID_PUBLISHER:
				case ID3FID_TRACKNUM:
				case ID3FID_RECORDINGDATES:
				case ID3FID_NETRADIOSTATION:
				case ID3FID_NETRADIOOWNER:
				case ID3FID_SIZE:
				case ID3FID_ISRC:
				case ID3FID_ENCODERSETTINGS:
				case ID3FID_YEAR:
				{
					char *sText = ID3_GetString(frame, ID3FN_TEXT);
					CString strText(sText);
					strText.Trim();
					strFidInfo << strText;
					delete [] sText;
					break;
				}
				case ID3FID_BPM:
				{
					char *sText = ID3_GetString(frame, ID3FN_TEXT);
					long lLength = atol(sText);
					if (lLength) // check for != "0"
						strFidInfo << sText;
					delete [] sText;
					break;
				}
				case ID3FID_SONGLEN:
				{
					char *sText = ID3_GetString(frame, ID3FN_TEXT);
					long lLength = atol(sText) / 1000;
					if (lLength)
					{
						CString strLength;
						SecToTimeLength(lLength, strLength);
						strFidInfo << strLength;
					}
					delete [] sText;
					break;
				}
				case ID3FID_USERTEXT:
				{
					char
					*sText = ID3_GetString(frame, ID3FN_TEXT),
					*sDesc = ID3_GetString(frame, ID3FN_DESCRIPTION);
					CString strText(sText);
					strText.Trim();
					if (!strText.IsEmpty())
					{
						CString strDesc(sDesc);
						strDesc.Trim();
						if (!strDesc.IsEmpty())
							strFidInfo << _T("(") << strDesc << _T(")");

						if (!strDesc.IsEmpty())
							strFidInfo << _T(": ");
						strFidInfo << strText;
					}
					delete [] sText;
				delete [] sDesc;
					break;
				}
				case ID3FID_COMMENT:
				case ID3FID_UNSYNCEDLYRICS:
				{
					char
					*sText = ID3_GetString(frame, ID3FN_TEXT),
					*sDesc = ID3_GetString(frame, ID3FN_DESCRIPTION),
					*sLang = ID3_GetString(frame, ID3FN_LANGUAGE);
					CString strText(sText);
					strText.Trim();
					if (!strText.IsEmpty())
					{
						CString strDesc(sDesc);
						strDesc.Trim();
						if (strDesc == _T("ID3v1 Comment"))
							strDesc.Empty();
						if (!strDesc.IsEmpty())
							strFidInfo << _T("(") << strDesc << _T(")");

						CString strLang(sLang);
						strLang.Trim();
						if (!strLang.IsEmpty())
							strFidInfo << _T("[") << strLang << _T("]");

						if (!strDesc.IsEmpty() || !strLang.IsEmpty())
							strFidInfo << _T(": ");
						strFidInfo << strText;
					}
					delete [] sText;
					delete [] sDesc;
					delete [] sLang;
					break;
				}
				case ID3FID_WWWAUDIOFILE:
				case ID3FID_WWWARTIST:
				case ID3FID_WWWAUDIOSOURCE:
				case ID3FID_WWWCOMMERCIALINFO:
				case ID3FID_WWWCOPYRIGHT:
				case ID3FID_WWWPUBLISHER:
				case ID3FID_WWWPAYMENT:
				case ID3FID_WWWRADIOPAGE:
				{
					char *sURL = ID3_GetString(frame, ID3FN_URL);
					CString strURL(sURL);
					strURL.Trim();
					strFidInfo << strURL;
					delete [] sURL;
					break;
				}
				case ID3FID_WWWUSER:
				{
					char
					*sURL = ID3_GetString(frame, ID3FN_URL),
					*sDesc = ID3_GetString(frame, ID3FN_DESCRIPTION);
					CString strURL(sURL);
					strURL.Trim();
					if (!strURL.IsEmpty())
					{
						CString strDesc(sDesc);
						strDesc.Trim();
						if (!strDesc.IsEmpty())
							strFidInfo << _T("(") << strDesc << _T(")");

						if (!strDesc.IsEmpty())
							strFidInfo << _T(": ");
						strFidInfo << strURL;
					}
					delete [] sURL;
					delete [] sDesc;
					break;
				}
				case ID3FID_INVOLVEDPEOPLE:
				{
					size_t nItems = frame->GetField(ID3FN_TEXT)->GetNumTextItems();
					for (size_t nIndex = 0; nIndex < nItems; nIndex++)
					{
						char *sPeople = ID3_GetString(frame, ID3FN_TEXT, nIndex);
						strFidInfo << sPeople;
						delete [] sPeople;
						if (nIndex + 1 < nItems)
							strFidInfo << _T(", ");
					}
					break;
				}
				case ID3FID_PICTURE:
				{
					char
					*sMimeType = ID3_GetString(frame, ID3FN_MIMETYPE),
					*sDesc	   = ID3_GetString(frame, ID3FN_DESCRIPTION),
					*sFormat   = ID3_GetString(frame, ID3FN_IMAGEFORMAT);
					size_t
					nPicType   = frame->GetField(ID3FN_PICTURETYPE)->Get(),
					nDataSize  = frame->GetField(ID3FN_DATA)->Size();
					strFidInfo << _T("(") << sDesc << _T(")[") << sFormat << _T(", ")
							   << nPicType << _T("]: ") << sMimeType << _T(", ") << nDataSize << _T(" bytes");
					delete [] sMimeType;
					delete [] sDesc;
					delete [] sFormat;
					break;
				}
				case ID3FID_GENERALOBJECT:
				{
					char
					*sMimeType = ID3_GetString(frame, ID3FN_MIMETYPE),
					*sDesc = ID3_GetString(frame, ID3FN_DESCRIPTION),
					*sFileName = ID3_GetString(frame, ID3FN_FILENAME);
					size_t
					nDataSize = frame->GetField(ID3FN_DATA)->Size();
					strFidInfo << _T("(") << sDesc << _T(")[")
						<< sFileName << _T("]: ") << sMimeType << _T(", ") << nDataSize << _T(" bytes");
					delete [] sMimeType;
					delete [] sDesc;
					delete [] sFileName;
					break;
				}
				case ID3FID_UNIQUEFILEID:
				{
					char *sOwner = ID3_GetString(frame, ID3FN_OWNER);
					size_t nDataSize = frame->GetField(ID3FN_DATA)->Size();
					strFidInfo << sOwner << _T(", ") << nDataSize << _T(" bytes");
					delete [] sOwner;
					break;
				}
				case ID3FID_PLAYCOUNTER:
				{
					size_t nCounter = frame->GetField(ID3FN_COUNTER)->Get();
					strFidInfo << nCounter;
					break;
				}
				case ID3FID_POPULARIMETER:
				{
					char *sEmail = ID3_GetString(frame, ID3FN_EMAIL);
					size_t
					nCounter = frame->GetField(ID3FN_COUNTER)->Get(),
					nRating = frame->GetField(ID3FN_RATING)->Get();
					strFidInfo << sEmail << _T(", counter=") << nCounter << _T(" rating=") << nRating;
					delete [] sEmail;
					break;
				}
				case ID3FID_CRYPTOREG:
				case ID3FID_GROUPINGREG:
				{
					char *sOwner = ID3_GetString(frame, ID3FN_OWNER);
					size_t
					nSymbol = frame->GetField(ID3FN_ID)->Get(),
					nDataSize = frame->GetField(ID3FN_DATA)->Size();
					strFidInfo << _T("(") << nSymbol << _T("): ") << sOwner << _T(", ") << nDataSize << _T(" bytes");
					break;
				}
				case ID3FID_SYNCEDLYRICS:
				{
					char
					*sDesc = ID3_GetString(frame, ID3FN_DESCRIPTION),
					*sLang = ID3_GetString(frame, ID3FN_LANGUAGE);
					size_t
					//nTimestamp = frame->GetField(ID3FN_TIMESTAMPFORMAT)->Get(),
					nRating = frame->GetField(ID3FN_CONTENTTYPE)->Get();
					//const char* format = (2 == nTimestamp) ? "ms" : "frames";
					strFidInfo << _T("(") << sDesc << _T(")[") << sLang << _T("]: ");
					switch (nRating)
					{
					case ID3CT_OTHER:    strFidInfo << _T("Other"); break;
					case ID3CT_LYRICS:   strFidInfo << _T("Lyrics"); break;
					case ID3CT_TEXTTRANSCRIPTION:     strFidInfo << _T("Text transcription"); break;
					case ID3CT_MOVEMENT: strFidInfo << _T("Movement/part name"); break;
					case ID3CT_EVENTS:   strFidInfo << _T("Events"); break;
					case ID3CT_CHORD:    strFidInfo << _T("Chord"); break;
					case ID3CT_TRIVIA:   strFidInfo << _T("Trivia/'pop up' information"); break;
					}
					/*ID3_Field* fld = frame->GetField(ID3FN_DATA);
					if (fld)
					{
						ID3_MemoryReader mr(fld->GetRawBinary(), fld->BinSize());
						while (!mr.atEnd())
						{
							strFidInfo << io::readString(mr).c_str();
							strFidInfo << " [" << io::readBENumber(mr, sizeof(uint32)) << " "
								<< format << "] ";
						}
					}*/
					delete [] sDesc;
					delete [] sLang;
					break;
				}
				case ID3FID_AUDIOCRYPTO:
				case ID3FID_EQUALIZATION:
				case ID3FID_EVENTTIMING:
				case ID3FID_CDID:
				case ID3FID_MPEGLOOKUP:
				case ID3FID_OWNERSHIP:
				case ID3FID_PRIVATE:
				case ID3FID_POSITIONSYNC:
				case ID3FID_BUFFERSIZE:
				case ID3FID_VOLUMEADJ:
				case ID3FID_REVERB:
				case ID3FID_SYNCEDTEMPO:
				case ID3FID_METACRYPTO:
					//strFidInfo << _T(" (unimplemented)");
					break;
				default:
					//strFidInfo << _T(" frame");
					break;
				}

				if (!strFidInfo.IsEmpty())
				{
					mi->strInfo << _T("   ") << A2CT(desc) << _T(":\t") << strFidInfo.GetText() << _T("\n");
				}
			}
			delete iter;
		}
		catch(...)
		{
			ASSERT(0);
		}

		if (bFoundHeader)
			return true;
	}

	if (!IsWindow(hWndOwner))
		return false;

	// starting the MediaDet object takes a noticeable amount of time.. avoid starting that object
	// for files which are not expected to contain any Audio/Video data.
	// note also: MediaDet does not work well for too short files (e.g. 16K)
	//
	// same applies for MediaInfoLib, its even slower than MediaDet -> avoid calling for non AV files.
	//
	// since we have a thread here, this should not be a performance problem any longer.

	bool bGiveMediaInfoLibHint = false;
	// check again for AV type; MediaDet object has trouble with RAR files (?)
	EED2KFileType eFileType = GetED2KFileTypeID(pFile->GetFileName());
	if (thePrefs.GetInspectAllFileTypes() 
		|| (eFileType == ED2KFT_AUDIO || eFileType == ED2KFT_VIDEO))
	{
		/////////////////////////////////////////////////////////////////////////////
		// Try MediaInfo lib
		//
		// Use MediaInfo only for non AVI files.. Reading potentially broken AVI files 
		// with the VfW API (as MediaInfo is doing) is rather dangerous.
		//
		if (!bIsAVI)
		{
			try
			{
				if (theMediaInfoDLL.Initialize())
				{
					void* Handle = theMediaInfoDLL.Open(pFile->GetFilePath());
					if (Handle)
					{
						CString str;

						mi->strFileFormat = theMediaInfoDLL.Get(Handle, Stream_General, 0, _T("Format"), Info_Text, Info_Name);
						str = theMediaInfoDLL.Get(Handle, Stream_General, 0, _T("Format_String"), Info_Text, Info_Name);
						if (!str.IsEmpty() && str.Compare(mi->strFileFormat) != 0)
							mi->strFileFormat += _T(" (") + str + _T(")");

						if (szExt[0] == _T('.') && szExt[1] != _T('\0'))
						{
							str = theMediaInfoDLL.Get(Handle, Stream_General, 0, _T("Format_Extensions"), Info_Text, Info_Name);
							if (!str.IsEmpty())
							{
								// minor bug in MediaInfo lib.. some file extension lists have a ')' character in there..
								str.Replace(_T(")"), _T(""));
								str.Replace(_T("("), _T(""));

								str.MakeLower();
								bool bFoundExt = false;
								int iPos = 0;
								CString strFmtExt(str.Tokenize(_T(" "), iPos));
								while (!strFmtExt.IsEmpty())
								{
									if (_tcscmp(strFmtExt, szExt + 1) == 0)
									{
										bFoundExt = true;
										break;
									}
									strFmtExt = str.Tokenize(_T(" "), iPos);
								}

								if (!bFoundExt)
									WarnAboutWrongFileExtension(mi, pFile->GetFileName(), str);
							}
						}

						CString strTitle = theMediaInfoDLL.Get(Handle, Stream_General, 0, _T("Title"), Info_Text, Info_Name);
						CString strTitleMore = theMediaInfoDLL.Get(Handle, Stream_General, 0, _T("Title_More"), Info_Text, Info_Name);
						if (!strTitleMore.IsEmpty() && !strTitle.IsEmpty() && strTitleMore != strTitle)
							strTitle += _T("; ") + strTitleMore;
						CString strAuthor = theMediaInfoDLL.Get(Handle, Stream_General, 0, _T("Author"), Info_Text, Info_Name);
						CString strCopyright = theMediaInfoDLL.Get(Handle, Stream_General, 0, _T("Copyright"), Info_Text, Info_Name);
						CString strComments = theMediaInfoDLL.Get(Handle, Stream_General, 0, _T("Comments"), Info_Text, Info_Name);

						if (strComments.IsEmpty())
						{
							strComments = theMediaInfoDLL.Get(Handle, Stream_General, 0, _T("Comment"), Info_Text, Info_Name);
						}

						CString strDate = theMediaInfoDLL.Get(Handle, Stream_General, 0, _T("Date"), Info_Text, Info_Name);
						struct tm tmUtc = {0};
						if (_stscanf(strDate, _T("UTC %u-%u-%u %u:%u:%u"), &tmUtc.tm_year, &tmUtc.tm_mon, &tmUtc.tm_mday, &tmUtc.tm_hour, &tmUtc.tm_min, &tmUtc.tm_sec) == 6)
						{
							tmUtc.tm_mon -= 1;
							tmUtc.tm_year -= 1900;
							tmUtc.tm_isdst = -1;
							time_t tLocal = mktime(&tmUtc);	// convert UTC to local time
							if (tLocal != -1)
							{
								// convert local time to UTC
								time_t tUtc = tLocal - _timezone;
								if (tmUtc.tm_isdst == 1)
									tUtc -= _dstbias;

								// output 'date+time' or just 'date'
								struct tm* tmLoc = localtime(&tUtc);
								if (tmLoc && tmLoc->tm_hour == 0 && tmLoc->tm_min == 0 && tmLoc->tm_sec == 0)
									strDate = CTime(tUtc).Format(_T("%x"));
								else
									strDate = CTime(tUtc).Format(_T("%c"));
							}
						}
						if (!strTitle.IsEmpty() || !strAuthor.IsEmpty() || !strCopyright.IsEmpty() || !strComments.IsEmpty() || !strDate.IsEmpty())
						{
							if (!mi->strInfo.IsEmpty())
								mi->strInfo << _T("\n");
							mi->OutputFileName();
							mi->strInfo.SetSelectionCharFormat(mi->strInfo.m_cfBold);
							mi->strInfo << GetResString(IDS_FD_GENERAL) << _T("\n");
							if (!strTitle.IsEmpty())
								mi->strInfo << _T("   ") << GetResString(IDS_TITLE) << _T(":\t") << strTitle << _T("\n");
							if (!strAuthor.IsEmpty())
								mi->strInfo << _T("   ") << GetResString(IDS_AUTHOR) << _T(":\t") << strAuthor << _T("\n");
							if (!strCopyright.IsEmpty())
								mi->strInfo << _T("   Copyright:\t") << strCopyright << _T("\n");
							if (!strComments.IsEmpty())
								mi->strInfo << _T("   ") << GetResString(IDS_COMMENT) << _T(":\t") << strComments << _T("\n");
							if (!strDate.IsEmpty())
								mi->strInfo << _T("   ") << GetResString(IDS_DATE) << _T(":\t") << strDate << _T("\n");
						}

						str = theMediaInfoDLL.Get(Handle, Stream_General, 0, _T("PlayTime"), Info_Text, Info_Name);
						float fFileLengthSec = _tstoi(str) / 1000.0F;
						UINT uAllBitrates = 0;

						str = theMediaInfoDLL.Get(Handle, Stream_General, 0, _T("VideoCount"), Info_Text, Info_Name);
						int iVideoStreams = _tstoi(str);
						if (iVideoStreams > 0)
						{
							mi->iVideoStreams = iVideoStreams;
							mi->fVideoLengthSec = fFileLengthSec;

							CString strCodec = theMediaInfoDLL.Get(Handle, Stream_Video, 0, _T("Codec"), Info_Text, Info_Name);
							mi->strVideoFormat = strCodec;
							if (!strCodec.IsEmpty()) {
								CStringA strCodecA(strCodec);
								if (!strCodecA.IsEmpty())
									mi->video.bmiHeader.biCompression = *(LPDWORD)(LPCSTR)strCodecA;
							}
							str = theMediaInfoDLL.Get(Handle, Stream_Video, 0, _T("Codec_String"), Info_Text, Info_Name);
							if (!str.IsEmpty() && str.Compare(mi->strVideoFormat) != 0)
								mi->strVideoFormat += _T(" (") + str + _T(")");

							str = theMediaInfoDLL.Get(Handle, Stream_Video, 0, _T("Width"), Info_Text, Info_Name);
							mi->video.bmiHeader.biWidth = _tstoi(str);

							str = theMediaInfoDLL.Get(Handle, Stream_Video, 0, _T("Height"), Info_Text, Info_Name);
							mi->video.bmiHeader.biHeight = _tstoi(str);

							str = theMediaInfoDLL.Get(Handle, Stream_Video, 0, _T("FrameRate"), Info_Text, Info_Name);
							mi->fVideoFrameRate = _tstof(str);

							str = theMediaInfoDLL.Get(Handle, Stream_Video, 0, _T("BitRate_Mode"), Info_Text, Info_Name);
							if (str.CompareNoCase(_T("VBR")) == 0) {
								mi->video.dwBitRate = (DWORD)-1;
								uAllBitrates = (UINT)-1;
							}
							else {
								str = theMediaInfoDLL.Get(Handle, Stream_Video, 0, _T("BitRate"), Info_Text, Info_Name);
								int iBitrate = _tstoi(str);
								mi->video.dwBitRate = iBitrate == -1 ? -1 : iBitrate;
								if (iBitrate == -1)
									uAllBitrates = (UINT)-1;
								else if (uAllBitrates != (UINT)-1)
									uAllBitrates += iBitrate;
							}

							str = theMediaInfoDLL.Get(Handle, Stream_Video, 0, _T("AspectRatio"), Info_Text, Info_Name);
							mi->fVideoAspectRatio = _tstof(str);

							for (int s = 1; s < iVideoStreams; s++)
							{
								if (!mi->strInfo.IsEmpty())
									mi->strInfo << _T("\n");
								mi->OutputFileName();
								mi->strInfo.SetSelectionCharFormat(mi->strInfo.m_cfBold);
								mi->strInfo << GetResString(IDS_VIDEO) << _T(" #") << s+1 << _T("\n");

								CString strCodec = theMediaInfoDLL.Get(Handle, Stream_Video, s, _T("Codec"), Info_Text, Info_Name);
								CString strVideoFormat = strCodec;
								str = theMediaInfoDLL.Get(Handle, Stream_Video, s, _T("Codec_String"), Info_Text, Info_Name);
								if (!str.IsEmpty() && str.Compare(strVideoFormat) != 0)
									strVideoFormat += _T(" (") + str + _T(")");
								if (!strVideoFormat.IsEmpty())
									mi->strInfo << _T("   ") << GetResString(IDS_CODEC) << _T(":\t") << strVideoFormat << _T("\n");

								CString strBitrate;
								str = theMediaInfoDLL.Get(Handle, Stream_Video, s, _T("BitRate_Mode"), Info_Text, Info_Name);
								if (str.CompareNoCase(_T("VBR")) == 0) {
									strBitrate = _T("Variable");
									uAllBitrates = (UINT)-1;
								}
								else {
									str = theMediaInfoDLL.Get(Handle, Stream_Video, s, _T("BitRate"), Info_Text, Info_Name);
									int iBitrate = _tstoi(str);
									if (iBitrate != 0) {
										if (iBitrate == -1) {
											strBitrate = _T("Variable");
											uAllBitrates = (UINT)-1;
										}
										else {
											strBitrate.Format(_T("%u %s"), (iBitrate + 500) / 1000, GetResString(IDS_KBITSSEC));
											if (uAllBitrates != (UINT)-1)
												uAllBitrates += iBitrate;
										}
									}
								}
								if (!strBitrate.IsEmpty())
									mi->strInfo << _T("   ") << GetResString(IDS_BITRATE) << _T(":\t") << strBitrate << _T("\n");

								CString strWidth = theMediaInfoDLL.Get(Handle, Stream_Video, s, _T("Width"), Info_Text, Info_Name);
								CString strHeight = theMediaInfoDLL.Get(Handle, Stream_Video, s, _T("Height"), Info_Text, Info_Name);
								if (!strWidth.IsEmpty() && !strHeight.IsEmpty())
									mi->strInfo << _T("   ") << GetResString(IDS_WIDTH) << _T(" x ") << GetResString(IDS_HEIGHT) << _T(":\t") << strWidth << _T(" x ") << strHeight << _T("\n");

								str = theMediaInfoDLL.Get(Handle, Stream_Video, s, _T("AspectRatio"), Info_Text, Info_Name);
								if (!str.IsEmpty())
									mi->strInfo << _T("   ") << GetResString(IDS_ASPECTRATIO) << _T(":\t") << str << _T("  (") << GetKnownAspectRatioDisplayString((float)_tstof(str)) << _T(")\n");

								str = theMediaInfoDLL.Get(Handle, Stream_Video, s, _T("FrameRate"), Info_Text, Info_Name);
								if (!str.IsEmpty())
									mi->strInfo << _T("   ") << GetResString(IDS_FPS) << _T(":\t") << str << _T("\n");
							}

							bFoundHeader = true;
						}

						str = theMediaInfoDLL.Get(Handle, Stream_General, 0, _T("AudioCount"), Info_Text, Info_Name);
						int iAudioStreams = _tstoi(str);
						if (iAudioStreams > 0)
						{
							mi->iAudioStreams = iAudioStreams;
							mi->fAudioLengthSec = fFileLengthSec;

							str = theMediaInfoDLL.Get(Handle, Stream_Audio, 0, _T("Codec"), Info_Text, Info_Name);
							if (_stscanf(str, _T("%hx"), &mi->audio.wFormatTag) != 1) {
								mi->strAudioFormat = str;
								str = theMediaInfoDLL.Get(Handle, Stream_Audio, 0, _T("Codec_String"), Info_Text, Info_Name);
								if (!str.IsEmpty() && str.Compare(mi->strAudioFormat) != 0)
									mi->strAudioFormat += _T(" (") + str + _T(")");
							}
							else {
								mi->strAudioFormat = theMediaInfoDLL.Get(Handle, Stream_Audio, 0, _T("Codec_String"), Info_Text, Info_Name);
								str = theMediaInfoDLL.Get(Handle, Stream_Audio, 0, _T("Codec_Info"), Info_Text, Info_Name);
								if (!str.IsEmpty() && str.Compare(mi->strAudioFormat) != 0)
									mi->strAudioFormat += _T(" (") + str + _T(")");
							}

							str = theMediaInfoDLL.Get(Handle, Stream_Audio, 0, _T("Channels"), Info_Text, Info_Name);
							mi->audio.nChannels = (WORD)_tstoi(str);

							str = theMediaInfoDLL.Get(Handle, Stream_Audio, 0, _T("SamplingRate"), Info_Text, Info_Name);
							mi->audio.nSamplesPerSec = _tstoi(str);

							str = theMediaInfoDLL.Get(Handle, Stream_Audio, 0, _T("BitRate_Mode"), Info_Text, Info_Name);
							if (str.CompareNoCase(_T("VBR")) == 0) {
								mi->audio.nAvgBytesPerSec = (DWORD)-1;
								uAllBitrates = (UINT)-1;
							}
							else {
								str = theMediaInfoDLL.Get(Handle, Stream_Audio, 0, _T("BitRate"), Info_Text, Info_Name);
								int iBitrate = _tstoi(str);
								mi->audio.nAvgBytesPerSec = iBitrate == -1 ? -1 : iBitrate / 8;
								if (iBitrate == -1)
									uAllBitrates = (UINT)-1;
								else if (uAllBitrates != (UINT)-1)
									uAllBitrates += iBitrate;
							}

							mi->strAudioLanguage = theMediaInfoDLL.Get(Handle, Stream_Audio, 0, _T("Language_String"), Info_Text, Info_Name);
							if (mi->strAudioLanguage.IsEmpty())
								mi->strAudioLanguage = theMediaInfoDLL.Get(Handle, Stream_Audio, 0, _T("Language"), Info_Text, Info_Name);

							for (int s = 1; s < iAudioStreams; s++)
							{
								if (!mi->strInfo.IsEmpty())
									mi->strInfo << _T("\n");
								mi->OutputFileName();
								mi->strInfo.SetSelectionCharFormat(mi->strInfo.m_cfBold);
								mi->strInfo << GetResString(IDS_AUDIO) << _T(" #") << s+1 << _T("\n");

								str = theMediaInfoDLL.Get(Handle, Stream_Audio, s, _T("Codec"), Info_Text, Info_Name);
								WORD wFormatTag;
								CString strAudioFormat;
								if (_stscanf(str, _T("%hx"), &wFormatTag) != 1) {
									strAudioFormat = str;
									str = theMediaInfoDLL.Get(Handle, Stream_Audio, s, _T("Codec_String"), Info_Text, Info_Name);
									if (!str.IsEmpty() && str.Compare(strAudioFormat) != 0)
										strAudioFormat += _T(" (") + str + _T(")");
								}
								else {
									strAudioFormat = theMediaInfoDLL.Get(Handle, Stream_Audio, s, _T("Codec_String"), Info_Text, Info_Name);
									str = theMediaInfoDLL.Get(Handle, Stream_Audio, s, _T("Codec_Info"), Info_Text, Info_Name);
									if (!str.IsEmpty() && str.Compare(strAudioFormat) != 0)
										strAudioFormat += _T(" (") + str + _T(")");
								}
								if (!strAudioFormat.IsEmpty())
									mi->strInfo << _T("   ") << GetResString(IDS_CODEC) << _T(":\t") << strAudioFormat << _T("\n");

								CString strBitrate;
								str = theMediaInfoDLL.Get(Handle, Stream_Audio, s, _T("BitRate_Mode"), Info_Text, Info_Name);
								if (str.CompareNoCase(_T("VBR")) == 0) {
									strBitrate = _T("Variable");
									uAllBitrates = (UINT)-1;
								}
								else {
									str = theMediaInfoDLL.Get(Handle, Stream_Audio, s, _T("BitRate"), Info_Text, Info_Name);
									int iBitrate = _tstoi(str);
									if (iBitrate != 0) {
										if (iBitrate == -1) {
											strBitrate = _T("Variable");
											uAllBitrates = (UINT)-1;
										}
										else {
											strBitrate.Format(_T("%u %s"), (iBitrate + 500) / 1000, GetResString(IDS_KBITSSEC));
											if (uAllBitrates != (UINT)-1)
												uAllBitrates += iBitrate;
										}
									}
								}
								if (!strBitrate.IsEmpty())
									mi->strInfo << _T("   ") << GetResString(IDS_BITRATE) << _T(":\t") << strBitrate << _T("\n");

								str = theMediaInfoDLL.Get(Handle, Stream_Audio, s, _T("Channels"), Info_Text, Info_Name);
								if (!str.IsEmpty())
								{
									int iChannels = _tstoi(str);
									mi->strInfo << _T("   ") << GetResString(IDS_CHANNELS) << _T(":\t");
									if (iChannels == 1)
										mi->strInfo << _T("1 (Mono)");
									else if (iChannels == 2)
										mi->strInfo << _T("2 (Stereo)");
									else if (iChannels == 5)
										mi->strInfo << _T("5.1 (Surround)");
									else
										mi->strInfo << iChannels;
									mi->strInfo << _T("\n");
								}

								str = theMediaInfoDLL.Get(Handle, Stream_Audio, s, _T("SamplingRate"), Info_Text, Info_Name);
								if (!str.IsEmpty())
									mi->strInfo << _T("   ") << GetResString(IDS_SAMPLERATE) << _T(":\t") << _tstoi(str) / 1000.0 << _T(" kHz\n");

								str = theMediaInfoDLL.Get(Handle, Stream_Audio, s, _T("Language_String"), Info_Text, Info_Name);
								if (str.IsEmpty())
									str = theMediaInfoDLL.Get(Handle, Stream_Audio, s, _T("Language"), Info_Text, Info_Name);
								if (!str.IsEmpty())
									mi->strInfo << _T("   ") << GetResString(IDS_PW_LANG) << _T(":\t") << str << _T("\n");

								str = theMediaInfoDLL.Get(Handle, Stream_Audio, s, _T("Language_Info"), Info_Text, Info_Name);
								if (!str.IsEmpty())
									mi->strInfo << _T("   ") << GetResString(IDS_PW_LANG) << _T(":\t") << str << _T("\n");
							}

							bFoundHeader = true;
						}

						str = theMediaInfoDLL.Get(Handle, Stream_General, 0, _T("TextCount"), Info_Text, Info_Name);
						int iTextStreams = _tstoi(str);
						for (int s = 0; s < iTextStreams; s++)
						{
							if (!mi->strInfo.IsEmpty())
								mi->strInfo << _T("\n");
							mi->OutputFileName();
							mi->strInfo.SetSelectionCharFormat(mi->strInfo.m_cfBold);
							mi->strInfo << _T("Subtitle") << _T(" #") << s+1 << _T("\n");

							str = theMediaInfoDLL.Get(Handle, Stream_Text, s, _T("Codec"), Info_Text, Info_Name);
							if (!str.IsEmpty())
								mi->strInfo << _T("   ") << GetResString(IDS_CODEC) << _T(":\t") << str << _T("\n");

							str = theMediaInfoDLL.Get(Handle, Stream_Text, s, _T("Language_String"), Info_Text, Info_Name);
							if (str.IsEmpty())
								str = theMediaInfoDLL.Get(Handle, Stream_Text, s, _T("Language"), Info_Text, Info_Name);
							if (!str.IsEmpty())
								mi->strInfo << _T("   ") << GetResString(IDS_PW_LANG) << _T(":\t") << str << _T("\n");

							str = theMediaInfoDLL.Get(Handle, Stream_Text, s, _T("Language_Info"), Info_Text, Info_Name);
							if (!str.IsEmpty())
								mi->strInfo << _T("   ") << GetResString(IDS_PW_LANG) << _T(":\t") << str << _T("\n");
						}

						str = theMediaInfoDLL.Get(Handle, Stream_General, 0, _T("ChaptersCount"), Info_Text, Info_Name);
						int iChapterStreams = _tstoi(str);
						for (int s = 0; s < iChapterStreams; s++)
						{
							if (!mi->strInfo.IsEmpty())
								mi->strInfo << _T("\n");
							mi->OutputFileName();
							mi->strInfo.SetSelectionCharFormat(mi->strInfo.m_cfBold);
							mi->strInfo << _T("Chapter") << _T(" #") << s+1 << _T("\n");

							str = theMediaInfoDLL.Get(Handle, Stream_Chapters, s, _T("Codec"), Info_Text, Info_Name);
							if (!str.IsEmpty())
								mi->strInfo << _T("   ") << GetResString(IDS_CODEC) << _T(":\t") << str << _T("\n");

							str = theMediaInfoDLL.Get(Handle, Stream_Chapters, s, _T("Language_String"), Info_Text, Info_Name);
							if (!str.IsEmpty())
								mi->strInfo << _T("   ") << GetResString(IDS_PW_LANG) << _T(":\t") << str << _T("\n");

							str = theMediaInfoDLL.Get(Handle, Stream_Chapters, s, _T("Language_Info"), Info_Text, Info_Name);
							if (!str.IsEmpty())
								mi->strInfo << _T("   ") << GetResString(IDS_PW_LANG) << _T(":\t") << str << _T("\n");
						}

						theMediaInfoDLL.Close(Handle);

						// MediaInfoLib does not handle MPEG files correctly in regards of
						// play length property -- even for completed files (applies also for
						// v0.7.2.1). So, we try to calculate the play length by using the
						// various bitrates. We could do this only for part files which are
						// still not having the final file length, but MediaInfoLib also
						// fails to determine play length for completed files (Hint: one can
						// not use GOPs to determine the play length (properly)).
						//
						//"MPEG 1"		v0.7.0.0
						//"MPEG-1 PS"	v0.7.2.1
						if (mi->strFileFormat.Find(_T("MPEG")) == 0) /* MPEG container? */
						{
							if (   uAllBitrates != 0					/* do we have any bitrates? */
							    && uAllBitrates != (UINT)-1				/* do we have CBR only? */
							   ) {
								// Though, its not that easy to calculate the real play length
								// without including the container's overhead. The value we
								// calculate with this simple formular is slightly too large!
								// But, its still better than using GOP-derived values which are
								// sometimes completely wrong.
								fFileLengthSec = (float)((double)pFile->GetFileSize() * 8.0 / uAllBitrates);

								if (mi->iVideoStreams > 0) {
									// Try to compensate the error from above by estimating the overhead
									if (mi->fVideoFrameRate > 0) {
										ULONGLONG uFrames = (ULONGLONG)(fFileLengthSec * mi->fVideoFrameRate);
										fFileLengthSec = (float)((double)(pFile->GetFileSize() - uFrames*24) * 8.0 / uAllBitrates);
									}
									mi->fVideoLengthSec = fFileLengthSec;
									mi->bVideoLengthEstimated = true;
								}
								if (mi->iAudioStreams > 0) {
									mi->fAudioLengthSec = fFileLengthSec;
									mi->bAudioLengthEstimated = true;
								}
							}
							else {
								// set the 'estimated' flags in case we any VBR stream
								if (mi->iVideoStreams > 0)
									mi->bVideoLengthEstimated = true;
								if (mi->iAudioStreams > 0)
									mi->bAudioLengthEstimated = true;
							}
						}

						if (bFoundHeader)
							return true;
					}
				}
				else {
					bGiveMediaInfoLibHint = true;
				}
			}
			catch(...)
			{
				ASSERT(0);
			}
		}

		if (!IsWindow(hWndOwner))
			return false;

		/////////////////////////////////////////////////////////////////////////////
		// Try MediaDet object
		//
		// Avoid processing of some file types which are known to crash due to bugged DirectShow filters.
#if _MSC_VER<1400
		if (thePrefs.GetInspectAllFileTypes() 
			|| (_tcscmp(szExt, _T(".ogm"))!=0 && _tcscmp(szExt, _T(".ogg"))!=0 && _tcscmp(szExt, _T(".mkv"))!=0))
		{
			try
			{
				CComPtr<IMediaDet> pMediaDet;
				HRESULT hr = pMediaDet.CoCreateInstance(__uuidof(MediaDet));
				if (SUCCEEDED(hr))
				{
					USES_CONVERSION;
					if (SUCCEEDED(hr = pMediaDet->put_Filename(CComBSTR(T2CW(pFile->GetFilePath())))))
					{
						long lStreams;
						if (SUCCEEDED(hr = pMediaDet->get_OutputStreams(&lStreams)))
						{
							for (long i = 0; i < lStreams; i++)
							{
								if (SUCCEEDED(hr = pMediaDet->put_CurrentStream(i)))
								{
									GUID major_type;
									if (SUCCEEDED(hr = pMediaDet->get_StreamType(&major_type)))
									{
										if (major_type == MEDIATYPE_Video)
										{
											mi->iVideoStreams++;

											if (mi->iVideoStreams > 1)
											{
												if (!mi->strInfo.IsEmpty())
													mi->strInfo << _T("\n");
												mi->OutputFileName();
												mi->strInfo.SetSelectionCharFormat(mi->strInfo.m_cfBold);
												mi->strInfo << GetResString(IDS_VIDEO) << _T(" #") << mi->iVideoStreams << _T("\n");
											}

											AM_MEDIA_TYPE mt = {0};
											if (SUCCEEDED(hr = pMediaDet->get_StreamMediaType(&mt)))
											{
												if (mt.formattype == FORMAT_VideoInfo)
												{
													VIDEOINFOHEADER* pVIH = (VIDEOINFOHEADER*)mt.pbFormat;

													if (mi->iVideoStreams == 1)
													{
														mi->video = *pVIH;
														if (mi->video.bmiHeader.biWidth && mi->video.bmiHeader.biHeight)
															mi->fVideoAspectRatio = (float)abs(mi->video.bmiHeader.biWidth) / (float)abs(mi->video.bmiHeader.biHeight);
														mi->video.dwBitRate = 0; // don't use this value
														mi->strVideoFormat = GetVideoFormatName(mi->video.bmiHeader.biCompression);
														pMediaDet->get_FrameRate(&mi->fVideoFrameRate);
														bFoundHeader = true;
													}
													else
													{
														mi->strInfo << _T("   ") << GetResString(IDS_CODEC) << _T(":\t") << (LPCTSTR)GetVideoFormatName(pVIH->bmiHeader.biCompression) << _T("\n");
														mi->strInfo << _T("   ") << GetResString(IDS_WIDTH) << _T(" x ") << GetResString(IDS_HEIGHT) << _T(":\t") << abs(pVIH->bmiHeader.biWidth) << _T(" x ") << abs(pVIH->bmiHeader.biHeight) << _T("\n");
														// do not use that 'dwBitRate', whatever this number is, it's not
														// the bitrate of the *encoded* video stream. seems to be the bitrate
														// of the *decoded* stream
														//if (pVIH->dwBitRate)
														//	mi->strInfo << "   Bitrate:\t" << (UINT)(pVIH->dwBitRate / 1000) << " " << GetResString(IDS_KBITSSEC) << "\n";

														double fFrameRate = 0.0;
														if (SUCCEEDED(pMediaDet->get_FrameRate(&fFrameRate)) && fFrameRate)
															mi->strInfo << _T("   ") << GetResString(IDS_FPS) << _T(":\t") << fFrameRate << _T("\n");
													}
												}
											}

											double fLength = 0.0;
											if (SUCCEEDED(pMediaDet->get_StreamLength(&fLength)) && fLength)
											{
												if (mi->iVideoStreams == 1)
													mi->fVideoLengthSec = fLength;
												else
												{
													CString strLength;
													SecToTimeLength((ULONG)fLength, strLength);
													mi->strInfo << _T("   ") << GetResString(IDS_LENGTH) << _T(":\t") << strLength;
													if (pFile->IsPartFile()){
														mi->strInfo.SetSelectionCharFormat(mi->strInfo.m_cfRed);
														mi->strInfo << _T(" (estimated)");
														mi->strInfo.SetSelectionCharFormat(mi->strInfo.m_cfDef);
													}
													mi->strInfo << _T("\n");
												}
											}

											if (mt.pUnk != NULL)
												mt.pUnk->Release();
											if (mt.pbFormat != NULL)
												CoTaskMemFree(mt.pbFormat);
										}
										else if (major_type == MEDIATYPE_Audio)
										{
											mi->iAudioStreams++;

											if (mi->iAudioStreams > 1)
											{
												if (!mi->strInfo.IsEmpty())
													mi->strInfo << _T("\n");
												mi->OutputFileName();
												mi->strInfo.SetSelectionCharFormat(mi->strInfo.m_cfBold);
												mi->strInfo << GetResString(IDS_AUDIO) << _T(" #") << mi->iAudioStreams << _T("\n");
											}

											AM_MEDIA_TYPE mt = {0};
											if (SUCCEEDED(hr = pMediaDet->get_StreamMediaType(&mt)))
											{
												if (mt.formattype == FORMAT_WaveFormatEx)
												{
													WAVEFORMATEX* wfx = (WAVEFORMATEX*)mt.pbFormat;

													// Try to determine if the stream is VBR.
													//
													// MediaDet seems to only look at the AVI stream headers to get a hint 
													// about CBR/VBR. If the stream headers are looking "odd", MediaDet
													// reports "VBR". Typically, files muxed with Nandub get identified as 
													// VBR (just because of the stream headers) even if they are CBR. Also,
													// real VBR MP3 files still get reported as CBR. Though, basically it's
													// better to report VBR even if it's CBR. The other way round is even
													// more ugly.
													if (!mt.bFixedSizeSamples)
														wfx->nAvgBytesPerSec = (DWORD)-1;

													if (mi->iAudioStreams == 1)
													{
														memcpy(&mi->audio, wfx, sizeof mi->audio);
														mi->strAudioFormat = GetWaveFormatTagName(wfx->wFormatTag);
													}
													else
													{
														mi->strInfo << _T("   ") << GetResString(IDS_CODEC) << _T(":\t") << GetWaveFormatTagName(wfx->wFormatTag) << _T("\n");

														if (wfx->nAvgBytesPerSec)
														{
															CString strBitrate;
															if (wfx->nAvgBytesPerSec == (DWORD)-1)
																strBitrate = _T("Variable");
															else
																strBitrate.Format(_T("%u %s"), (UINT)(((wfx->nAvgBytesPerSec * 8.0) + 500.0) / 1000.0), GetResString(IDS_KBITSSEC));
															mi->strInfo << _T("   ") << GetResString(IDS_BITRATE) << _T(":\t") << strBitrate << _T("\n");
														}

														if (wfx->nChannels)
														{
															mi->strInfo << _T("   ") << GetResString(IDS_CHANNELS) << _T(":\t");
															if (wfx->nChannels == 1)
																mi->strInfo << _T("1 (Mono)");
															else if (wfx->nChannels == 2)
																mi->strInfo << _T("2 (Stereo)");
															else if (wfx->nChannels == 5)
																mi->strInfo << _T("5.1 (Surround)");
															else
																mi->strInfo << wfx->nChannels;
															mi->strInfo << _T("\n");
														}

														if (wfx->nSamplesPerSec)
															mi->strInfo << _T("   ") << GetResString(IDS_SAMPLERATE) << _T(":\t") << wfx->nSamplesPerSec / 1000.0 << _T(" kHz\n");
														
														if (wfx->wBitsPerSample)
															mi->strInfo << _T("   Bit/sample:\t") << wfx->wBitsPerSample << _T(" Bit\n");
													}
													bFoundHeader = true;
												}
											}

											double fLength = 0.0;
											if (SUCCEEDED(pMediaDet->get_StreamLength(&fLength)) && fLength)
											{
												if (mi->iAudioStreams == 1)
													mi->fAudioLengthSec = fLength;
												else
												{
													CString strLength;
													SecToTimeLength((ULONG)fLength, strLength);
													mi->strInfo << _T("   ") << GetResString(IDS_LENGTH) << _T(":\t") << strLength;
													if (pFile->IsPartFile()){
														mi->strInfo.SetSelectionCharFormat(mi->strInfo.m_cfRed);
														mi->strInfo << _T(" (estimated)");
														mi->strInfo.SetSelectionCharFormat(mi->strInfo.m_cfDef);
													}
													mi->strInfo << _T("\n");
												}
											}

											if (mt.pUnk != NULL)
												mt.pUnk->Release();
											if (mt.pbFormat != NULL)
												CoTaskMemFree(mt.pbFormat);
										}
										else
										{
											if (!mi->strInfo.IsEmpty())
												mi->strInfo << _T("\n");
											mi->OutputFileName();
											mi->strInfo.SetSelectionCharFormat(mi->strInfo.m_cfBold);
											mi->strInfo << GetResString(IDS_UNKNOWN) << _T(" Stream #") << i+1 << _T("\n");

											double fLength = 0.0;
											if (SUCCEEDED(pMediaDet->get_StreamLength(&fLength)) && fLength)
											{
												CString strLength;
												SecToTimeLength((ULONG)fLength, strLength);
												mi->strInfo << _T("   ") << GetResString(IDS_LENGTH) << _T(":\t") << strLength;
												if (pFile->IsPartFile()) {
													mi->strInfo.SetSelectionCharFormat(mi->strInfo.m_cfRed);
													mi->strInfo << _T(" (estimated)");
													mi->strInfo.SetSelectionCharFormat(mi->strInfo.m_cfDef);
												}
												mi->strInfo << _T("\n");
											}
											mi->strInfo << _T("\n");
										}
									}
								}
							}
						}
					}
					else{
						TRACE(_T("Failed to open \"%s\" - %s\n"), pFile->GetFilePath(), GetErrorMessage(hr, 1));
					}
				}
			}
			catch(...){
				ASSERT(0);
			}
		}
#endif
	}

	if (!bFoundHeader && bGiveMediaInfoLibHint)
	{
		TCHAR szBuff[_MAX_PATH];
		DWORD dwRet = ::GetModuleFileName(theApp.m_hInstance, szBuff, _MAX_PATH);
		if (dwRet == 0 || dwRet == _MAX_PATH)
			szBuff[0] = _T('\0');
		CString strInstFolder(szBuff);
		PathRemoveFileSpec(strInstFolder.GetBuffer(strInstFolder.GetLength()));
		strInstFolder.ReleaseBuffer();
		CString strHint;
		strHint.Format( GetResString(IDS_MEDIAINFO_DLLMISSING), strInstFolder);
		if (!mi->strInfo.IsEmpty())
			mi->strInfo << _T("\r\n");
		mi->strInfo << strHint;
	}

	return bFoundHeader || !mi->strMimeType.IsEmpty() || bHasDRM;
}

void CFileInfoDialog::DoDataExchange(CDataExchange* pDX)
{
	CResizablePage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FULL_FILE_INFO, m_fi);
}

void CFileInfoDialog::Localize()
{
	GetDlgItem(IDC_FD_XI1)->SetWindowText(GetResString(IDS_FD_SIZE));
	GetDlgItem(IDC_FD_XI2)->SetWindowText(GetResString(IDS_LENGTH)+_T(":"));
	GetDlgItem(IDC_FD_XI3)->SetWindowText(GetResString(IDS_VIDEO));
	GetDlgItem(IDC_FD_XI4)->SetWindowText(GetResString(IDS_AUDIO));

	GetDlgItem(IDC_FD_XI5)->SetWindowText(GetResString(IDS_CODEC)+_T(":"));
	GetDlgItem(IDC_FD_XI6)->SetWindowText(GetResString(IDS_CODEC)+_T(":"));

	GetDlgItem(IDC_FD_XI7)->SetWindowText(GetResString(IDS_BITRATE)+_T(":"));
	GetDlgItem(IDC_FD_XI8)->SetWindowText(GetResString(IDS_BITRATE)+_T(":"));

	GetDlgItem(IDC_FD_XI9)->SetWindowText(GetResString(IDS_WIDTH)+_T(" x ")+GetResString(IDS_HEIGHT)+_T(":"));
	GetDlgItem(IDC_FD_XI13)->SetWindowText(GetResString(IDS_FPS)+_T(":"));
	GetDlgItem(IDC_FD_XI10)->SetWindowText(GetResString(IDS_CHANNELS)+_T(":"));
	GetDlgItem(IDC_FD_XI12)->SetWindowText(GetResString(IDS_SAMPLERATE)+_T(":"));

	GetDlgItem(IDC_STATICFI)->SetWindowText(GetResString(IDS_FILEFORMAT)+_T(":"));
	GetDlgItem(IDC_FD_XI14)->SetWindowText(GetResString(IDS_ASPECTRATIO)+_T(":"));

	GetDlgItem(IDC_STATIC_LANGUAGE)->SetWindowText(GetResString(IDS_PW_LANG)+_T(":"));	
}

void CFileInfoDialog::AddFileInfo(LPCTSTR pszFmt, ...)
{
	va_list pArgp;
	va_start(pArgp, pszFmt);
	CString strInfo;
	strInfo.FormatV(pszFmt, pArgp);
	va_end(pArgp);

	m_fi.SetSel(m_fi.GetWindowTextLength(), m_fi.GetWindowTextLength());
	m_fi.ReplaceSel(strInfo);
}
