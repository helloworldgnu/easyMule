/*
 * $Id: BarShader.h 4483 2008-01-02 09:19:06Z soarchin $
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

class CBarShader
{
public:
	CBarShader(uint32 height = 1, uint32 width = 1);
	~CBarShader(void);

	//set the width of the bar
	void SetWidth(int width);

	//set the height of the bar
	void SetHeight(int height);

	//returns the width of the bar
	int GetWidth() {
		return m_iWidth;
	}

	//returns the height of the bar
	int GetHeight() {
		return m_iHeight;
	}

	//call this to blank the shaderwithout changing file size
	void Reset();

	//sets new file size and resets the shader
	void SetFileSize(EMFileSize fileSize);

	//fills in a range with a certain color, new ranges overwrite old
	void FillRange(uint64 start, uint64 end, COLORREF color);

	//fills in entire range with a certain color
	void Fill(COLORREF color);

	//draws the bar
	void Draw(CDC* dc, int iLeft, int iTop, bool bFlat);
	void DrawPreview(CDC* dc, int iLeft, int iTop, UINT previewLevel);		//Cax2 aqua bar

protected:
	void BuildModifiers();
	void FillRect(CDC *dc, LPRECT rectSpan, float fRed, float fGreen, float fBlue, bool bFlat);
	void FillRect(CDC *dc, LPRECT rectSpan, COLORREF color, bool bFlat);

	int    m_iWidth;
	int    m_iHeight;
	double m_dPixelsPerByte;
	double m_dBytesPerPixel;
	EMFileSize m_uFileSize;
	bool	m_bIsPreview;

private:
	CRBMap<uint64, COLORREF> m_Spans;	// SLUGFILLER: speedBarShader
	float *m_Modifiers;
	UINT m_used3dlevel;
};
