/*
 * $Id: Resizee.h 4483 2008-01-02 09:19:06Z soarchin $
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

namespace TabWnd{

class CResizer;
class CResizee
{
	friend class CResizer;
public:
	CResizee();
	~CResizee(void);

	enum {ATTR_FIXLEN = 0x1, ATTR_TAIL = 0x2, ATTR_FILL = 0x4};

	void			AttachToResizer(CResizer *pResizer);
	void			DetachFromResizer();
	const CRect&	GetRect(void);
	void			SetRect(const CRect &rect);

	UINT			GetAttribute(){return m_uAttribute;}
	void			SetAttribute(UINT uAttribute){m_uAttribute = uAttribute;}

	BOOL			IsFixedLength(){return m_uAttribute & ATTR_FIXLEN;}
	void			SetDesireLength(int iLength){m_iDesireLength = iLength;}
	virtual	int		GetDesireLength(void){return m_iDesireLength;}
	void			SetDynDesireLength(BOOL bDynDesireLength){m_bDynDesireLength = bDynDesireLength;}
	BOOL			IsDynDesireLength(){return m_bDynDesireLength;}


	BOOL			IsStickTail(){return m_uAttribute & ATTR_TAIL;}
	BOOL			IsFill(){return m_uAttribute & ATTR_FILL;}

protected:
	virtual	void OnSize() = 0;
private:
	//void	SetRect(const CRect &rect);
	
	CResizer	*m_pResizer;
	POSITION	m_posInResizer;
	CRect		m_rect;
	UINT		m_uAttribute;
	int			m_iDesireLength;
	BOOL		m_bDynDesireLength;
};

}//namespace TabWnd{
