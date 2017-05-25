/*
 * $Id: 3DPreviewControl.h 4483 2008-01-02 09:19:06Z soarchin $
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
#pragma once

#include "barshader.h"

// C3DPreviewControl

class C3DPreviewControl : public CStatic
{
	DECLARE_DYNAMIC(C3DPreviewControl)

public:
	C3DPreviewControl();
	virtual ~C3DPreviewControl();

protected:
	DECLARE_MESSAGE_MAP()
	int m_iSliderPos;
	static CBarShader s_preview;
public:
	// Sets "slider" position for type of preview
	void SetSliderPos(int iPos);
	afx_msg void OnPaint();
};


