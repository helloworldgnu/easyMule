/*
 * $Id: TimeTick.h 4783 2008-02-02 08:17:12Z soarchin $
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
/////////////////////////////////////////////////////////////////////////////
//
// Copyright 2001, Stefan Belopotocan, http://welcome.to/BeloSoft
//
/////////////////////////////////////////////////////////////////////////////
#pragma once

/////////////////////////////////////////////////////////////////////////////
// CTimeTick

class CTimeTick
{
	CTimeTick(const CTimeTick& d);
	CTimeTick& operator=(const CTimeTick& d);

public:	
	CTimeTick();
	~CTimeTick();

	// Operations
	void Start();
	float Tick();
	bool  isPerformanceCounter() {return m_nPerformanceFrequency!=0;}

	// Implementation
protected:
	static __int64 GetPerformanceFrequency();
	static float GetTimeInMilliSeconds(__int64 nTime);

	// Data
private:
	static __int64 m_nPerformanceFrequency;

	LARGE_INTEGER m_nTimeElapsed;
	LARGE_INTEGER m_nTime;
};
