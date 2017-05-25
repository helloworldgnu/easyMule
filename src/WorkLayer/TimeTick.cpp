/*
 * $Id: TimeTick.cpp 4783 2008-02-02 08:17:12Z soarchin $
 * 
 * this file is part of easyMule
 * Copyright (C)2002-2008 VeryCD Dev Team ( strEmail.Format("%s@%s", "emuledev", "verycd.com") / http: * www.easymule.org )
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
// TimeTick.cpp : implementation of the CTimeTick class
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright 2001, Stefan Belopotocan, http://welcome.to/BeloSoft
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TimeTick.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//////////////////////////////////////////////////////////////////////
// CTimeTick

__int64 CTimeTick::m_nPerformanceFrequency = CTimeTick::GetPerformanceFrequency();

CTimeTick::CTimeTick()
{
	m_nTimeElapsed.QuadPart = 0;
	m_nTime.QuadPart 	    = 0;
}

CTimeTick::~CTimeTick()
{
}

void CTimeTick::Start()
{	
	if (m_nPerformanceFrequency)
		QueryPerformanceCounter(&m_nTimeElapsed);
	m_nTime.QuadPart = 0;
}

float CTimeTick::Tick()
{
	LARGE_INTEGER nTime;

	if (m_nPerformanceFrequency){
		QueryPerformanceCounter(&nTime);

		float nTickTime		= GetTimeInMilliSeconds(nTime.QuadPart - m_nTimeElapsed.QuadPart);
		m_nTimeElapsed.QuadPart = nTime.QuadPart;

		return nTickTime;
	}
	return 0.0f;
}

__int64 CTimeTick::GetPerformanceFrequency()
{
	LARGE_INTEGER nPerformanceFrequency;

	if (QueryPerformanceFrequency(&nPerformanceFrequency))
		return nPerformanceFrequency.QuadPart;
	else
		return 0;
}

float CTimeTick::GetTimeInMilliSeconds(__int64 nTime)
{
	return ((float) (nTime*1000i64)) / ((float) m_nPerformanceFrequency);
}
