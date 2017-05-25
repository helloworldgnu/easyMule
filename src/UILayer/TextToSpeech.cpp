/* 
 * $Id: TextToSpeech.cpp 4483 2008-01-02 09:19:06Z soarchin $
 * 
 * this file is part of eMule
 * Copyright (C)2002-2005 Merkur ( devs@emule-project.net / http://www.emule-project.net )
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
#if _MSC_VER < 1310	// check for 'Visual Studio .NET 2002'
#define HAVE_SAPI_H
#endif
#ifdef HAVE_SAPI_H
#include <sapi.h>
#endif
#include "emule.h"
#include "emuleDlg.h"
#include "TextToSpeech.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#ifdef HAVE_SAPI_H
///////////////////////////////////////////////////////////////////////////////
// CTextToSpeech

class CTextToSpeech
{
public:
	~CTextToSpeech();

	bool CreateTTS();
	void ReleaseTTS();
	bool IsActive() const { return m_pISpVoice != NULL; }
	bool Speak(LPCTSTR psz);

protected:
	long m_lTTSLangID;
	CComPtr<ISpVoice> m_pISpVoice;
};

CTextToSpeech::CTextToSpeech()
{
	m_lTTSLangID = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);
}

CTextToSpeech::~CTextToSpeech()
{
	ASSERT( m_pISpVoice == NULL );
	ReleaseTTS();
}

bool CTextToSpeech::CreateTTS()
{
	bool bResult = FALSE;
	if (m_pISpVoice == NULL)
	{
		HRESULT hr;
		if (SUCCEEDED(hr = m_pISpVoice.CoCreateInstance(CLSID_SpVoice)))
		{
			m_lTTSLangID = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);
			bResult = TRUE;
		}
	}
	return bResult;
}

void CTextToSpeech::ReleaseTTS()
{
	m_lTTSLangID = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);
	m_pISpVoice.Release();
}

bool CTextToSpeech::Speak(LPCTSTR pwsz)
{
	bool bResult = false;
	USES_CONVERSION;
	if (m_pISpVoice)
	{
		if (SUCCEEDED(m_pISpVoice->Speak(T2CW(pwsz), SPF_ASYNC | SPF_IS_NOT_XML, NULL)))
			bResult = true;
	}
	return bResult;
}

///////////////////////////////////////////////////////////////////////////////
CTextToSpeech theTextToSpeech;
static bool s_bTTSDisabled = false;
static bool s_bInitialized = false;
#endif

bool Speak(LPCTSTR pszSay)
{
#ifdef HAVE_SAPI_H
	if (theApp.emuledlg == NULL || !theApp.emuledlg->IsRunning())
		return false;
	if (s_bTTSDisabled)
		return false;

	if (!s_bInitialized)
	{
		s_bInitialized = true;
		if (!theTextToSpeech.CreateTTS())
			return false;
	}
	return theTextToSpeech.Speak(pszSay);
#else
	UNREFERENCED_PARAMETER(pszSay);
	return false;
#endif
}

void ReleaseTTS()
{
#ifdef HAVE_SAPI_H
	theTextToSpeech.ReleaseTTS();
	s_bInitialized = false;
#endif
}

void CloseTTS()
{
#ifdef HAVE_SAPI_H
	ReleaseTTS();
	s_bTTSDisabled = true;
#endif
}

bool IsSpeechEngineAvailable()
{
#ifdef HAVE_SAPI_H
	if (s_bTTSDisabled)
		return false;

	static bool _bIsAvailable = false;
	static bool _bCheckedAvailable = false;
	if (!_bCheckedAvailable)
	{
		_bCheckedAvailable = true;
		if (theTextToSpeech.IsActive())
		{
			_bIsAvailable = true;
		}
		else
		{
			_bIsAvailable = theTextToSpeech.CreateTTS();
			theTextToSpeech.ReleaseTTS();
		}
	}
	return _bIsAvailable;
#else
	return false;
#endif
}
